/**
 * @file Auxiliar.h
 * @brief Funciones auxiliares para el control del campanario y la calefacción.
 *
 * Este archivo contiene definiciones, constantes, variables globales y funciones auxiliares
 * para la gestión de las campanas, la calefacción y la comunicación I2C en el sistema de domótica
 * del campanario. Incluye la lógica para el toque automático de horas y cuartos, la ejecución
 * de secuencias especiales (difuntos, misa), el control remoto de la calefacción y la integración
 * con el servidor web mediante WebSockets.
 *
 * Funcionalidades principales:
 * - Control de campanas por horas y cuartos usando RTC.
 * - Ejecución de secuencias especiales (difuntos, misa, parada).
 * - Control de la calefacción (encendido/apagado).
 * - Comunicación I2C para recibir órdenes y enviar estados.
 * - Notificación a clientes web mediante WebSockets.
 *
 * @author Julian Salas Baertolome
 * @version 1.0
 * @date 19/06/2025
 * 
 */

#ifndef AUXILIAR_H
  #define AUXILIAR_H

    #include "RTC.h"

    #include <Wire.h>
    #include "Servidor.h"
    

    #define DEBUG
    #define DEBUGAUXILIAR
    #define DEBUGREQUEST
    
    #define PinCampana1         26                          // Definición del pin para la campana 1
    #define PinCampana2         25                          // Definición del pin para la campana 2
    #define CalefaccionPin      33                          // Definición del pin para la calefacción
    #define PinRele4            32                          // Definición del pin para el relé de la calefacción
    #define PinConfiguracion    19                          // Definición del pin para la configuración del modo AP

    #define I2C_SLAVE_ADDR      0x12                        // Dirección del esclavo I2C   
    #define SDA_PIN             21                          // Definición del pin SDA para I2C
    #define SCL_PIN             22                          // Definición del pin SCL para I2C

    //#define EstadoInicio              0
    #define EstadoDifuntos              1
    #define EstadoMisa                  2
    #define EstadoStop                  3
    #define EstadoCalefaccionOn         4
    #define EstadoCalefaccionOff        5
    #define EstadoCampanario            6
    #define EstadoHora                  7
    #define EstadoSinInternet           8
    #define EstadoProteccionCampanadas  9
    #define EstadoFechaHora              10
    #define EstadoSetTemporizador        11

    #define BitEstadoSinInternet  0x20                      // Bit para indicar que no hay conexión a Internet
    #define BitEstadoProteccionCampanadas 0x40              // Bit para indicar que la protección de campanadas está activa

    #define InicioHorarioNocturno 23                        // Hora de inicio del horario nocturno (23:00)
    #define FinHorarioNocturno     7                        // Hora de fin del horario noct

    extern ConfigWiFi configWiFi;                           // Estructura para almacenar la configuración WiFi definida en modo AP

    CAMPANARIO Campanario;                                  // Instancia del campanario 
    struct tm timeinfo;                                     // Estructura para almacenar la hora actual

    int ultimoMinuto = -1;                                  // Variable para almacenar el minuto anterior
    int nCampanaTocada = 0;                                 // Variable para almacenar el número de campana tocada
    int ultimaHora = -1;                                    // Variable para almacenar la última hora tocada

    volatile uint8_t secuenciaI2C = 0;
    uint8_t requestI2C = 0;                                 // Variable para almacenar el número de solicitud I2C
    uint8_t ParametroI2C = 0;                               // Variable para almacenar el parametro recibido por I2C ( cuando se recibe )
    bool lConexionInternet = false;                         // Variable para indicar si hay conexión a Internet
    bool lProteccionCampanadas = false;                     // Variable para indicar si la protección de campanadas está activa
    bool lProteccionCampanadasAnterior = false;             // Variable para almacenar el estado anterior de la protección de campanadas

    unsigned long ultimoCheckInternet = 0;
    const unsigned long intervaloCheckInternet = 5 * 60 * 1000; // 5 minutos en ms
    int nTemporizacionCalefaccion = 45;                     // Variable para almacenar la temporización de la calefacción

    void ChekearCuartos(void);                              // Función para chequear los cuartos y las horas y tocar las campanas correspondientes
    void TestCampanadas(void);                              // Función para temporizar las campnas y presentarlas en la pagina correspondiente  
    
    void recibirSecuencia(int numBytes);                    // Función para recibir la secuencia de campanas por I2C
    void enviarRequest();                                   // Función para enviar una solicitud al esclavo I2C
    void enviarHoraI2C();                                   // Función para enviar la hora actual por I2C
    void enviarFechaHoraI2C();                              // Función para enviar la fecha y hora completa por I2C
    void enviarEstadoI2C();                                 // Función para enviar el estado de la secuencia de campanas por I2C

    void EjecutaSecuencia (int nSecuencia);                 // Función para ejecutar la secuencia de campanas según el valor recibido por I2C
    
    void TestInternet(void);                                // Función para comprobar la conexión a Internet y actualizar el DNS si es necesario

    bool EsHorarioNocturno (void);                          // Función para comprobar si es horario nocturno (entre InicioHorarioNocturno y FinHorarioNocturno)
    bool EsPeriodoToqueCampanas (void);                     // Función para comprobar si estamos ±5 minutos de un toque de hora o media hora


/**
 * @brief Verifica el cambio de minuto y ejecuta acciones relacionadas con las campanadas.
 *
 * Esta función debe ser llamada periódicamente para comprobar si ha cambiado el minuto actual.
 * Si la secuencia de campanadas no está activa y el minuto ha cambiado, realiza las siguientes acciones:
 *   - Si es una hora en punto (minuto 0) y no es horario nocturno, hace sonar la campana correspondiente a la hora.
 *   - Si es mediodía (hora 12 y minuto 0) y hay conexión a Internet, sincroniza el RTC con NTP.
 *   - Si es media hora (minuto 30) y no es horario nocturno, hace sonar la campana de la media hora.
 *   - Redirige a la página de campanas en todos los casos de media hora.
 *   - Si hay conexión a Internet, actualiza el DNS con el dominio configurado.
 *   - Si no hay conexión a Internet, muestra un mensaje de depuración (si está habilitado).
 * Si no se puede obtener la hora del RTC, muestra un mensaje de depuración (si está habilitado).
 * @note Requiere que las funciones Campanario.GetEstadoSecuencia(), getLocalTime(), Campanario.TocaHoraSinCuarto(), Campanario.TocaMediaHora(), hayInternet(), RTC::begin(), ActualizaDNS(), y ws.textAll() estén implementadas.
 */
    void ChekearCuartos(void) 
    {
        if (!Campanario.GetEstadoSecuencia()) {                         // Si la secuencia de campanadas no está activa ( aseguramos no tocar horas o cuartos si esta tocando una secuencia )
            if (getLocalTime(&timeinfo)) {                              // Obtiene la hora actual del RTC
                int hora = timeinfo.tm_hour;                            // Obtiene el valor de la horaº
                int minuto = timeinfo.tm_min;                           // Obtiene el valor del minuto
                if (minuto != ultimoMinuto) {                           // Si el minuto ha cambiado desde la última vez que se comprobó          
                    ultimoMinuto = minuto;

                    if (minuto % 30 == 0) {                             // Si es una media hora (0 o 30 minutos)
                        ws.textAll("REDIRECT:/Campanas.html");          // Redirige a la página de campanas
                        if (minuto == 0) {                              // Si es la hora en punto (minuto 0)
                            if(!EsHorarioNocturno())                    // Si no es horario nocturno
                            {
                                Campanario.TocaHoraSinCuartos(hora);    // Toca la campana correspondiente a la hora
                            }
                            if (hora == 12 )
                            {
                                if(hayInternet()){
                                    RTC::begin();                       // Sincroniza el RTC con NTP si es mediodía
                                }
                            }   
                        } else {
                            if (!EsHorarioNocturno())                   // Si no es horario nocturno
                            {
                                Campanario.TocaMediaHora();             // Toca la campana correspondiente a la media hora
                            }
                        }
                        if (hayInternet()){                             // Verifica si hay conexión a Internet
                            ActualizaDNS(configWiFi.dominio);           // Actualiza el DNS con el dominio configurado
                        } else {
                            #ifdef DEBUGAUXILIAR
                                Serial.println("Sin conexión a internet. No se actualiza DNS.");
                            #endif
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
    void TestCampanadas (void) 
    {
        nCampanaTocada = Campanario.ActualizarSecuenciaCampanadas();        // Llama al método ActualizarSecuenciaCampanadas del campanario para gestionar las campanas
        if (nCampanaTocada > 0) {                                           // Si se ha tocado una campana 
            #ifdef DEBUGAUXILIAR
                Serial.print("Campana tocada: ");
                Serial.println(nCampanaTocada);
            #endif  
            Campanario.ResetCampanaTocada();                                // Resetea el número de campana tocada
            ws.textAll("CAMPANA:"+String(nCampanaTocada));                  // Envía el número de campana tocada a todos los clientes conectados
            if (!Campanario.GetEstadoSecuencia()) {                                        // Si la secuencia de campanadas no está activa ( no esta tocando ninguna secuencia )
                #ifdef DEBUGAUXILIAR
                    Serial.println("Secuencia de campanadas finalizada.");
                #endif
                Campanario.ParaSecuencia();                                 // Indica al campanario que se ha finalizado la secuencia
                ws.textAll("REDIRECT:/index.html");                         // Indica a los clientes que deben redirigir a index.html
            }
        }
    }


    /**
     * @brief Recibe una secuencia de datos por I2C y procesa los bytes recibidos.
     * 
     * Esta función lee los datos disponibles en el bus I2C utilizando la librería Wire.
     * El primer byte recibido se almacena en 'secuenciaI2C'. Si se reciben dos bytes,
     * el segundo byte se interpreta como un parámetro adicional y se almacena en 'ParametroI2C'.
     * 
     * Si la secuencia recibida corresponde a una solicitud de estado, hora o fecha/hora,
     * se guarda la solicitud en 'requestI2C' y se resetea 'secuenciaI2C'.
     * 
     * En modo depuración (DEBUGAUXILIAR), se imprime por el puerto serie la información recibida.
     * 
     * @param numBytes Número de bytes esperados en la secuencia recibida (1 o 2).
     */
    void recibirSecuencia(int numBytes) 
    {
        if (Wire.available()) { 

            secuenciaI2C = Wire.read();
            if (numBytes == 2) {
                ParametroI2C = Wire.read(); // Lee el primer byte que es el comando de solicitud
            }
            #ifdef DEBUGAUXILIAR
                Serial.println("Secuencia recibida por I2C: " + String(secuenciaI2C));
            #endif
            if (numBytes == 2 )
            {
                #ifdef DEBUGAUXILIAR
                    Serial.printf("Parametro recibido por I2C: %d\n", ParametroI2C);
                #endif
            }
            if (secuenciaI2C == EstadoCampanario || secuenciaI2C == EstadoHora || secuenciaI2C == EstadoFechaHora) {
                requestI2C = secuenciaI2C;              // Guarda la solicitud I2C para enviar la hora o el estado
                secuenciaI2C = 0;                        // Resetea la secuencia si es una solicitud de estado o hora
            }
        }
    }

/**
 * @brief Envía la hora actual y el estado del campanario a través del bus I2C.
 *
 * Esta función obtiene la hora local desde el RTC si está sincronizado con NTP.
 * Si la obtención es exitosa, envía el estado del campanario, la hora, los minutos y los segundos por I2C.
 * En caso de error al obtener la hora, se envían valores por defecto (0xFF) para hora, minuto y segundos.
 * 
 * @note Si está definido DEBUGAUXILIAR, se imprime información de depuración por el puerto serie.
 */
    void enviarHoraI2C() 
    {
        uint8_t hora = 0xFF;
        uint8_t minuto = 0xFF;
        uint8_t segundos = 0xFF; // Inicializa las variables de hora, minuto y segundos
        struct tm localTime;
        int nEstadoCampanario = Campanario.GetEstadoCampanario(); // Obtiene el estado del campanario
    
        if (RTC::isNtpSync() && getLocalTime(&localTime)) {
            hora = (uint8_t)localTime.tm_hour;
            minuto = (uint8_t)localTime.tm_min;
            segundos = (uint8_t)localTime.tm_sec; // Obtiene la hora, minuto y segundos del RTC
            #ifdef DEBUGAUXILIAR
                Serial.printf("enviarHoraI2C -> Hora enviada por I2C: %02d:%02d\n", hora, minuto);
            #endif
        } else {
            #ifdef DEBUGAUXILIAR
                Serial.println("Error obteniendo hora para enviar por I2C");
            #endif
        }
        Wire.write(nEstadoCampanario); // Envía el estado del campanario
        Wire.write(hora);
        Wire.write(minuto);
        Wire.write(segundos); // Envía la hora, minuto y segundos por I2C
    }

/**
 * @brief Envía la fecha y hora completa junto con el estado del campanario a través del bus I2C.
 *
 * Esta función obtiene la fecha y hora local desde el RTC si está sincronizado con NTP.
 * Si la obtención es exitosa, envía el estado del campanario, día, mes, año, hora, minutos y segundos por I2C.
 * En caso de error al obtener la fecha/hora, se envían valores por defecto (0xFF) para todos los campos.
 * 
 * Orden de envío por I2C:
 * 1. Estado del campanario (1 byte)
 * 2. Día (1 byte)
 * 3. Mes (1 byte) 
 * 4. Año (1 byte - solo los dos últimos dígitos)
 * 5. Hora (1 byte)
 * 6. Minutos (1 byte)
 * 7. Segundos (1 byte)
 * 
 * @note Si está definido DEBUGAUXILIAR, se imprime información de depuración por el puerto serie.
 */
    void enviarFechaHoraI2C() 
    {
        uint8_t dia = 0xFF;
        uint8_t mes = 0xFF;
        uint8_t ano = 0xFF;
        uint8_t hora = 0xFF;
        uint8_t minuto = 0xFF;
        uint8_t segundos = 0xFF; // Inicializa las variables de fecha y hora
        struct tm localTime;
        int nEstadoCampanario = Campanario.GetEstadoCampanario(); // Obtiene el estado del campanario
        if (RTC::isNtpSync() && getLocalTime(&localTime)) {
            dia = (uint8_t)localTime.tm_mday;        // Día del mes (1-31)
            mes = (uint8_t)(localTime.tm_mon + 1);   // Mes (1-12) - tm_mon va de 0-11
            ano = (uint8_t)(localTime.tm_year % 100); // Año (solo los 2 últimos dígitos)
            hora = (uint8_t)localTime.tm_hour;       // Hora (0-23)
            minuto = (uint8_t)localTime.tm_min;      // Minutos (0-59)
            segundos = (uint8_t)localTime.tm_sec;    // Segundos (0-59)
            #ifdef DEBUGAUXILIAR
                Serial.printf("enviarFechaHoraI2C -> Fecha y hora enviada por I2C: %02d/%02d/%02d %02d:%02d:%02d\n", 
                             dia, mes, ano, hora, minuto, segundos);
            #endif
        } else {
            #ifdef DEBUGAUXILIAR
                Serial.println("Error obteniendo fecha y hora para enviar por I2C");
            #endif
        }
        Wire.write(nEstadoCampanario); // Envía el estado del campanario
        Wire.write(dia);               // Envía el día
        Wire.write(mes);               // Envía el mes
        Wire.write(ano);               // Envía el año (2 dígitos)
        Wire.write(hora);              // Envía la hora
        Wire.write(minuto);            // Envía los minutos
        Wire.write(segundos);          // Envía los segundos
    }


/**
 * @brief Procesa una solicitud recibida por I2C y responde según el tipo de petición.
 *
 * Esta función verifica el valor de la variable 'requestI2C' para determinar el tipo de solicitud recibida.
 * Dependiendo del caso, envía el estado del campanario o la hora actual a través de I2C.
 * Después de procesar la solicitud, reinicia 'requestI2C' para esperar una nueva petición.
 *
 * @note Si está definido DEBUGAUXILIAR, imprime por el puerto serie información sobre la solicitud recibida.
 */
    void enviarRequest() 
    {
        #ifdef DEBUGAUXILIAR
            Serial.print("enviarRequest -> Solicitud recibida por I2C: ");
            Serial.println (requestI2C);
        #endif
        switch (requestI2C) {
            case EstadoCampanario: // Si se solicita el estado del campanario
                enviarEstadoI2C(); // Envía el estado del campanario
                break;
            case EstadoHora: // Si se solicita la hora
                enviarHoraI2C(); // Envía la hora actual
                break;
            case EstadoFechaHora:
                enviarFechaHoraI2C(); // Envía la fecha actual
                break;
        }
        requestI2C = 0; // Resetea la solicitud I2C para esperar la siguiente
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
            Serial.println("enviarEstadoI2C -> Estado enviado por I2C: " + String(nEstadoCampanario));
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
 *                   - EstadoProteccionCampanadas: Notifica el estado de la protección de campanadas
 * 
 * @details La función maneja diferentes estados del campanario y notifica a los
 *          clientes web conectados mediante websockets sobre los cambios realizados.
 *          Para los toques de campana, redirige a los clientes a la página de campanas.
 *          Para el control de calefacción, notifica el estado actual.
 *          Si la secuencia no es reconocida, genera un mensaje de debug si DEBUGSERVIDOR está definido.
 */
    void EjecutaSecuencia (int nSecuencia) 
    {
        #ifdef DEBUGSERVIDOR
            Serial.println("Ejecutando secuencia: ");
            Serial.println(nSecuencia);
        #endif
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
                Campanario.EnciendeCalefaccion(nTemporizacionCalefaccion);         // Llama a la función para encender la calefacción con temporizacion
                ws.textAll("CALEFACCION:ON");               // Envía el estado de la calefacción a todos los clientes conectados
                break;
            case EstadoCalefaccionOff:
                Campanario.ApagaCalefaccion();              // Llama a la función para apagar la calefacción
                ws.textAll("CALEFACCION:OFF");              // Envía el estado de la calefacción a todos los clientes conectados
                break;
            case EstadoProteccionCampanadas:
                // Notifica el estado de la protección de campanadas a todos los clientes conectados
                ws.textAll(lProteccionCampanadas ? "PROTECCION:ON" : "PROTECCION:OFF");
                #ifdef DEBUGAUXILIAR
                    Serial.printf("EjecutaSecuencia -> Protección campanadas notificada: %s\n", lProteccionCampanadas ? "ACTIVA" : "INACTIVA");
                #endif
                break;
            case EstadoSetTemporizador:                     //Fija el temporizador de la calefacción
                nTemporizacionCalefaccion = (int)ParametroI2C; // Asigna el valor del parámetro recibido al temporizador de calefacción
                Campanario.EnciendeCalefaccion(nTemporizacionCalefaccion); // Llama a la función para fijar el temporizador de calefacción
                ws.textAll("CALEFACCION:ON:" + String(nTemporizacionCalefaccion));
                #ifdef DEBUGAUXILIAR
                    Serial.printf("EjecutaSecuencia -> Temporizador de calefacción fijado a %d minutos.\n", nTemporizacionCalefaccion);
                #endif
                break;
            default:
                #ifdef DEBUGSERVIDOR
                    Serial.println("Secuencia no reconocida.");
                #endif
                break;
        }
    }
    void TestInternet(void) {
        bool estadoAnteriorInternet = lConexionInternet; // Guarda el estado anterior de la conexión
        if (!hayInternet()) { // hayInternet() debe comprobar acceso real a internet
          lConexionInternet = ConectarWifi(configWiFi); // Intenta reconectar
          if (lConexionInternet) {
              #ifdef DEBUG
                Serial.println("TestInternet -> Reconectado a internet correctamente.");
              #endif
              ServidorOn(configWiFi.usuario, configWiFi.clave); // Reinicia el servidor si es necesario
              if (!estadoAnteriorInternet) { // Si el estado cambió de desconectado a conectado
                  Campanario.SetInternetConectado(); // Notifica al campanario que hay internet
              }
          } else {
              #ifdef DEBUG
                Serial.println("TestInternet -> Sin conexión a internet. Funcionando en modo local.");
              #endif
              if (estadoAnteriorInternet) { // Si el estado cambió de conectado a desconectado
                  Campanario.ClearInternetConectado(); // Notifica al campanario que no hay internet
              }
          }
        }else{
            lConexionInternet = true; // Asegura que la variable esté actualizada
            if (!estadoAnteriorInternet) { // Si el estado cambió de desconectado a conectado
                Campanario.SetInternetConectado(); // Notifica al campanario que hay internet
            }
            #ifdef DEBUG
                Serial.println("TestInternet -> Conexión a internet activa.");
            #endif
        }
    }

    /**
     * @brief Comprueba si la hora actual se encuentra dentro del horario nocturno.
     *
     * Esta función obtiene la hora local actual y verifica si está dentro del rango definido
     * por las variables globales InicioHorarioNocturno y FinHorarioNocturno. El horario nocturno
     * se considera desde InicioHorarioNocturno (inclusive) hasta FinHorarioNocturno (exclusive).
     *
     * @return true si la hora actual está en el horario nocturno, false en caso contrario o si no se pudo obtener la hora.
     */
    bool EsHorarioNocturno (void) {
        if (!RTC::isNtpSync()) {
            #ifdef DEBUG
                Serial.println("EsHorarioNocturno -> RTC no sincronizado con NTP.");
            #endif
            return false; // Si el RTC no está sincronizado, no se puede determinar el horario nocturno
        }
        struct tm localTime;
        if (getLocalTime(&localTime)) { // Obtiene la hora local
            int hora = localTime.tm_hour; // Obtiene la hora actual
            return (hora >= InicioHorarioNocturno || hora < FinHorarioNocturno); // Comprueba si está en horario nocturno
        }
        return false; // Si no se pudo obtener la hora, devuelve false
    }    

    /**
     * @brief Comprueba si estamos en el período de ±5 minutos alrededor de un toque de campanas.
     *
     * Esta función determina si el momento actual está dentro de la ventana de 5 minutos antes
     * o 5 minutos después de los momentos en que suenan las campanas:
     * - Toques de hora: minuto 0 (en punto)
     * - Toques de media hora: minuto 30
     * 
     * Períodos de actividad:
     * - Para hora en punto: entre 55-59 y 0-5 minutos
     * - Para media hora: entre 25-29 y 30-35 minutos
     *
     * @return true si estamos en período de toque de campanas (±5 min), false en caso contrario o si no se pudo obtener la hora.
     * 
     * @note Requiere que el RTC esté sincronizado con NTP para funcionar correctamente.
     */
    bool EsPeriodoToqueCampanas (void) {
        
        if (!RTC::isNtpSync()) {                                                            // Verifica si el RTC está sincronizado con NTP y si no lo esta           
            #ifdef DEBUGPROTECCION
                Serial.println("EsPeriodoToqueCampanas -> RTC no sincronizado con NTP.");
            #endif
            // Detecta cambio de estado y notifica si es necesario
            if (lProteccionCampanadas != false) {                                           //Si esta la proteccion de campanadas la deshabilitamos
                lProteccionCampanadasAnterior = lProteccionCampanadas;
                lProteccionCampanadas = false;
                EjecutaSecuencia(EstadoProteccionCampanadas);                               // Notifica el cambio de estado a cliente web
                Campanario.ClearProteccionCampanadas();                                     // Desactiva la protección de campanadas
                #ifdef DEBUGPROTECCION
                    Serial.println("EsPeriodoToqueCampanas -> Protección desactivada (RTC no sincronizado)");
                #endif
            }
            return false; // Si el RTC no está sincronizado, no se puede determinar el período
        }
        
        struct tm localTime;                                                                //Si hay sincronizacion
        if (getLocalTime(&localTime)) {                                                     // Obtiene la hora local                            
            int minuto = localTime.tm_min;                                                  //Obtenemos el minuto actual
            bool periodoHoraEnPunto = (minuto >= 55) || (minuto < 5);                       // Comprueba si estamos ±5 minutos de la hora en punto (minuto 0)
            bool periodoMediaHora = (minuto >= 25 && minuto <= 35);                         // Comprueba si estamos ±5 minutos de la media hora (minuto 30)
            bool nuevoEstadoProteccion = (periodoHoraEnPunto || periodoMediaHora);          //Si esta en algun periodo de proteccion
            if (lProteccionCampanadas != nuevoEstadoProteccion) {                           // Si el estado de protección ha cambiado     
                lProteccionCampanadasAnterior = lProteccionCampanadas;                      // Guarda el estado anterior de la protección de campanadas 
                lProteccionCampanadas = nuevoEstadoProteccion;                              // Actualiza el estado de la protección de campanadas
                if (lProteccionCampanadas) {                                                // Si estamos en un período de toque de campanas
                    Campanario.SetProteccionCampanadas();                                   // Activa la protección de campanadas
                    #ifdef DEBUGPROTECCION
                        Serial.printf("EsPeriodoToqueCampanas -> Activando protección de campanadas (minuto %d)\n", minuto);
                    #endif
                } else {                                                                    // Si no estamos en un período de toque de campanas
                    Campanario.ClearProteccionCampanadas();                                 // Desactiva la protección de campanadas
                    #ifdef DEBUGPROTECCION
                        Serial.printf("EsPeriodoToqueCampanas -> Desactivando protección de campanadas (minuto %d)\n", minuto);
                    #endif
                }
                EjecutaSecuencia(EstadoProteccionCampanadas);                               // Notifica el cambio de estado Al cliete web
                #ifdef DEBUGPROTECCION
                    Serial.printf("EsPeriodoToqueCampanas -> Cambio de protección: %s -> %s (minuto %d)\n", lProteccionCampanadasAnterior ? "ACTIVA" : "INACTIVA",lProteccionCampanadas ? "ACTIVA" : "INACTIVA",minuto);
                #endif
            }
            #ifdef DEBUGPROTECCION
                if (nuevoEstadoProteccion) {
                    Serial.printf("EsPeriodoToqueCampanas -> Período activo (minuto %d): %s\n", minuto, periodoHoraEnPunto ? "Hora en punto" : "Media hora");
                }
            #endif
            
            return nuevoEstadoProteccion;
        }

        #ifdef DEBUGPROTECCION
            Serial.println("EsPeriodoToqueCampanas -> Error obteniendo hora del RTC");
        #endif
        
        if (lProteccionCampanadas != false) {                                                   // Si no se pudo obtener la hora, desactiva la protección de campanadas si estaba activa
            lProteccionCampanadasAnterior = lProteccionCampanadas;
            lProteccionCampanadas = false;
            EjecutaSecuencia(EstadoProteccionCampanadas); // Notifica el cambio de estado
            #ifdef DEBUGPROTECCION
                Serial.println("EsPeriodoToqueCampanas -> Protección desactivada (error obteniendo hora)");
            #endif
        }
        return false; // Si no se pudo obtener la hora, devuelve false
    }
#endif
