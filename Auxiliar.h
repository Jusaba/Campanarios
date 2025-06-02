#include "RTC.h"

    #define DEBUGAUXILIAR

    CAMPANARIO Campanario;                              // Instancia del campanario 
    struct tm timeinfo;

    int ultimoMinuto = -1;                              // Variable para almacenar el último minuto detectado
    int nCampanaTocada = 0;                             // Variable para almacenar el número de campana tocada


    void ChekearCuartos(void);                          // Función para chequear los cuartos y las horas y tocar las campanas correspondientes
    void TestCampanadas(void);                          // Función para temporizar las campnas y presentarlas en la pagina correspondiente  

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
        if (!Campanario.GetEstadoSecuencia()) {                         // Si la secuencia de campanadas no está activa
            if (getLocalTime(&timeinfo)) {
                int hora = timeinfo.tm_hour;
                int minuto = timeinfo.tm_min;
                // Detectar cambio de minuto
                if (minuto != ultimoMinuto) {
                    ultimoMinuto = minuto;
                    if (minuto % 15 == 0) {
                        if (minuto == 0) {
                            // Es la hora en punto
                            ws.textAll("REDIRECT:/Campanas.html"); // Redirige a la página de campanas
                            Campanario.TocaHora(hora);
                            #ifdef DEBUGAUXILIAR
                                Serial.println("Es la hora en punto");
                            #endif
                        } else {
                            // Es un cuarto
                            ws.textAll("REDIRECT:/Campanas.html"); // Redirige a la página de campanas
                            Campanario.TocaCuarto(minuto/ 15);
                            #ifdef DEBUGAUXILIAR
                                Serial.print("Es el cuarto: ");
                                Serial.println(minuto / 15);
                            #endif
                        }
                    }
                }
            } else {
                Serial.println("No se pudo obtener la hora del RTC");
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
        nCampanaTocada = Campanario.ActualizarSecuenciaCampanadas();
        if (nCampanaTocada > 0) {                                           // Si se ha tocado una campana
            #ifdef DEBUGAUXILIAR
                Serial.print("Campana tocada: ");
                Serial.println(nCampanaTocada);
            #endif  
            Campanario.ResetCampanaTocada();                                // Resetea el número de campana tocada
            ws.textAll("CAMPANA:"+String(nCampanaTocada));                  // Envía el número de campana tocada a todos los clientes conectados
            if (!Campanario.GetEstadoSecuencia()) {                         // Si la secuencia de campanadas no está activa
                #ifdef DEBUGAUXILIAR
                    Serial.println("Secuencia de campanadas finalizada.");
                #endif
                Campanario.ParaSecuencia();                                 // Detiene la secuencia de campanadas
                ws.textAll("REDIRECT:/index.html"); // Indica a los clientes que deben redirigir   
            }
        }
    }