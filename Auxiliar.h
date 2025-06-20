#include "RTC.h"
#include <Wire.h>
#include "Servidor.h"

    #define DEBUGAUXILIAR
    
    #define PinCampana1         26                          // Definición del pin para la campana 1
    #define PinCampana2         25                          // Definición del pin para la campana 2
    #define CalefaccionPin      33                          // Definición del pin para la calefacción
    #define PinRele4            32                          // Definición del pin para el relé de la calefacción

    #define I2C_SLAVE_ADDR      0x12
    #define SDA_PIN             21
    #define SCL_PIN             22

    #define EstadoInicio            0
    #define EstadoDifuntos          1
    #define EstadoMisa              2
    #define EstadoStop              3
    #define EstadoCalefaccionOn     4
    #define EstadoCalefaccionOff    5



    CAMPANARIO Campanario;                                  // Instancia del campanario 
    struct tm timeinfo;                                     // Estructura para almacenar la hora actual

    int ultimoMinuto = -1;                                  // Variable para almacenar el minuto anterior
    int nCampanaTocada = 0;                                 // Variable para almacenar el número de campana tocada

    volatile uint8_t secuenciaI2C = 0;

    void ChekearCuartos(void);                          // Función para chequear los cuartos y las horas y tocar las campanas correspondientes
    void TestCampanadas(void);                          // Función para temporizar las campnas y presentarlas en la pagina correspondiente  
    
    void recibirSecuencia(int numBytes);                // Función para recibir la secuencia de campanas por I2C
    void enviarEstadoI2C();                             // Función para enviar el estado de la secuencia de campanas por I2C

    void EjecutaSecuencia (int nSecuencia);             // Función para ejecutar la secuencia de campanas según el valor recibido por I2C
    
    /**
     * @brief Verifica y gestiona las campanadas de horas y cuartos
     * 
     * Esta función se ejecuta para controlar el toque de campanas según la hora actual:
     * - Detecta cambios de minuto
     * - En las horas en punto (minuto 0), toca las campanadas correspondientes a la hora
     * - En los cuartos (minuto 15, 30, 45), toca las campanadas correspondientes al cuarto
     * 
     * La función obtiene la hora del RTC (Real Time Clock) y si no puede obtenerla,
     * muestra un mensaje de error por el puerto serie.
     * 
     * @note Requiere que el RTC esté configurado y funcionando correctamente
     * @note Usa la clase Campanario para ejecutar los toques
     * @note Si DEBUGAUXILIAR está definido, imprime mensajes de depuración por Serial
     */
    void ChekearCuartos(void) {
        if (!Campanario.GetEstadoSecuencia()) {                         // Si la secuencia de campanadas no está activa ( aseguramos no tocar horas o cuartos si esta tocando una secuencia )
            if (getLocalTime(&timeinfo)) {                              // Obtiene la hora actual del RTC
                int hora = timeinfo.tm_hour;                            // Obtiene el valor de la horaº
                int minuto = timeinfo.tm_min;                           // Obtiene el valor del minuto
                if (minuto != ultimoMinuto) {                           // Si el minuto ha cambiado desde la última vez que se comprobó          
                    ultimoMinuto = minuto;
                    if (minuto % 15 == 0) {                             // Si es un cuarto (0, 15, 30, 45)
                        if (minuto == 0) {                              // Si es la hora en punto (minuto 0)
                            ws.textAll("REDIRECT:/Campanas.html");      // Redirige a la página de campanas
                            Campanario.TocaHora(hora);                  // Toca la campana correspondiente a la hora
                        } else {
                            ws.textAll("REDIRECT:/Campanas.html");      // Redirige a la página de campanas
                            Campanario.TocaCuarto(minuto / 15);         // Toca la campana correspondiente al cuarto (1, 2, 3)
                       }
                    }
                }
            } else {
                #ifdef DEBUGAUXILIAR
                    Serial.println("No se pudo obtener la hora del RTC");
                #endif
            }
        }    
    }    

    /**
     * @brief Llama al método ActualizarSecuenciaCampanadas del campanario para gestionar las campanas.
     * 
     * Esa funbcion controla las temporizaciones de cada toque y devuelve la campana tocada. si es que en la ultima
     * llamada se ha tocado alguna campana, si se ha tocado alguna se pone el flag de tocando secuencia a true, y se resetea la campana tocada
     * en el campanario para poder detectar el proximo toque de campana.
     * 
     * Esta función realiza las siguientes operaciones:
     * - Actualiza y obtiene el número de la campana que ha sido tocada mediante el método ActualizarSecuenciaCampanadas del objeto Campanario.
     * - Gestiona el estado de la secuencia de campanadas (activa/inactiva) con la variable lTocandoSecuencia.
     * - Envía notificaciones al servidor sobre las campanas tocadas para que se puedan reflejar en la interfaz web.
     * - Registra información de depuración si DEBUGAUXILIAR está definido
     * 
     * Cuando una campana es tocada:
     * - Activa el indicador de secuencia activa
     * - Envía el número de campana al servidor WebSocket
     * - Resetea el contador de campana tocada
     * 
     * Cuando la secuencia finaliza:
     * - Desactiva el indicador de secuencia activa
     * - Envía mensaje "PARAR" al servidor WebSocket para indicar que la secuencia ha finalizado y que debe volver a index.html
     * 
     * @note Utiliza comunicación WebSocket para notificar a los clientes
     * @note Requiere la clase Campanario previamente inicializada
     */
    void TestCampanadas (void) {
        nCampanaTocada = Campanario.ActualizarSecuenciaCampanadas();        // Llama al método ActualizarSecuenciaCampanadas del campanario para gestionar las campanas
        if (nCampanaTocada > 0) {                                           // Si se ha tocado una campana 
            #ifdef DEBUGAUXILIAR
                Serial.print("Campana tocada: ");
                Serial.println(nCampanaTocada);
            #endif  
            Campanario.ResetCampanaTocada();                                // Resetea el número de campana tocada
            ws.textAll("CAMPANA:"+String(nCampanaTocada));                  // Envía el número de campana tocada a todos los clientes conectados
            if (!Campanario.GetEstadoSecuencia()) {                         // Si la secuencia de campanadas no está activa ( no esta tocando ninguna secuencia )
                #ifdef DEBUGAUXILIAR
                    Serial.println("Secuencia de campanadas finalizada.");
                #endif
                Campanario.ParaSecuencia();                                 // Indica al campanario que se ha finalizado la secuencia
                ws.textAll("REDIRECT:/index.html");                         // Indica a los clientes que deben redirigir a index.html
            }
        }
    }

    /**
     * @brief Recibe una secuencia por comunicación I2C
     * 
     * Este método lee un byte de datos desde el bus I2C cuando hay datos disponibles
     * y lo almacena en la variable secuenciaI2C. Si está definido DEBUGAUXILIAR,
     * imprime por el puerto serie el valor recibido.
     * 
     * @param numBytes Número de bytes a recibir (actualmente solo se lee 1 byte)
     */
    void recibirSecuencia(int numBytes) {
        if (Wire.available()) {
            secuenciaI2C = Wire.read();
            #ifdef DEBUGAUXILIAR
                Serial.println("Secuencia recibida por I2C: " + String(secuenciaI2C));
            #endif
        }
    }

    /**
     * @brief Envía el estado actual del campanario a través de comunicación I2C
     * 
     * Esta función obtiene el estado actual del campanario y lo transmite
     * a través del bus I2C. Si está definido DEBUGAUXILIAR, también imprime
     * el estado enviado por el puerto serie.
     * 
     * @note Requiere que la comunicación I2C (Wire) esté inicializada
     * @note Utiliza la clase Campanario para obtener el estado
     */
    void enviarEstadoI2C() {
        int nEstadoCampanario = Campanario.GetEstadoCampanario(); // Obtiene el estado del campanario
        Wire.write(nEstadoCampanario); // estadoCampanario es la variable que quieres enviar
        #ifdef DEBUGAUXILIAR
            Serial.println("Estado enviado por I2C: " + String(nEstadoCampanario));
        #endif
    }

    /**
     * @brief Ejecuta diferentes secuencias de control del campanario según el parámetro recibido
     * 
     * @param nSecuencia Número entero que indica la secuencia a ejecutar:
     *                   - EstadoDifuntos: Activa secuencia de toque de difuntos
     *                   - EstadoMisa: Activa secuencia de toque de misa
     *                   - EstadoStop: Detiene cualquier secuencia en ejecución
     *                   - EstadoCalefaccionOn: Enciende el sistema de calefacción
     *                   - EstadoCalefaccionOff: Apaga el sistema de calefacción
     * 
     * @details La función maneja diferentes estados del campanario y notifica a los
     *          clientes web conectados mediante websockets sobre los cambios realizados.
     *          Para los toques de campana, redirige a los clientes a la página de campanas.
     *          Para el control de calefacción, notifica el estado actual.
     *          Si la secuencia no es reconocida, genera un mensaje de debug si DEBUGSERVIDOR está definido.
     */
    void EjecutaSecuencia (int nSecuencia) {
        switch (nSecuencia) {
            case EstadoDifuntos:
                Campanario.TocaDifuntos();                  // Toca la secuencia de difuntos
                ws.textAll("REDIRECT:/Campanas.html");      // Indica a los clientes que deben redirigir a la pantalla de presentacion de las campanas
                break;
           case EstadoMisa:
                Campanario.TocaMisa(); // Toca la secuencia de hora
                ws.textAll("REDIRECT:/Campanas.html");      // Indica a los clientes que deben redirigir a la pantalla de presentacion de las campanas
                break;
            case EstadoStop:
                Campanario.ParaSecuencia();                 // Llama a la función para detener la secuencia de campanadas
                ws.textAll("REDIRECT:/index.html");         // Indica a los clientes que deben redirigir a la página principal
                break;
            case EstadoCalefaccionOn:
                Campanario.EnciendeCalefaccion();           // Llama a la función para encender la calefacción
                ws.textAll("CALEFACCION:ON");               // Envía el estado de la calefacción a todos los clientes conectados
                break;
            case EstadoCalefaccionOff:
                Campanario.ApagaCalefaccion();              // Llama a la función para apagar la calefacción
                ws.textAll("CALEFACCION:OFF");              // Envía el estado de la calefacción a todos los clientes conectados
                break;
            default:
                #ifdef DEBUGSERVIDOR
                    Serial.println("Secuencia no reconocida.");
                #endif
                break;
        }
    }
    

