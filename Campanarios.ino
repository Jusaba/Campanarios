//https://f1atb.fr/esp32-relay-integrated-230v-ac-power-supply-sensors/
/**
 * @file Campanarios.ino
 * @brief Programa principal para la gestión y control del campanario electrónico basado en ESP32.
 *
 * Este archivo implementa la lógica principal del sistema de campanario, incluyendo:
 * - Inicialización de hardware y periféricos (campanas, calefacción, RTC, WiFi, I2C, etc.).
 * - Gestión del modo configuración mediante portal cautivo (AP) si el pin 19 está a LOW.
 * - Conexión a red WiFi y arranque del servidor web asíncrono para control remoto.
 * - Recepción y ejecución de secuencias de campanadas tanto por WebSocket como por I2C.
 * - Control y consulta del estado de la calefacción.
 * - Ejecución de toques automáticos según la hora y eventos programados.
 *
 * Flujo principal:
 * 1. Si el pin 19 está a LOW, inicia el modo AP y portal cautivo para configuración WiFi.
 * 2. Si no, carga la configuración WiFi guardada y conecta a la red.
 * 3. Inicializa el bus I2C como esclavo y configura los manejadores de eventos.
 * 4. Crea e inicializa los objetos CAMPANA y CALEFACCION, y los añade al campanario.
 * 5. Arranca el servidor web para control remoto.
 * 6. En el loop principal, atiende órdenes recibidas por I2C o WebSocket y ejecuta las secuencias correspondientes.
 *
 *   ┌─────────────────────┐    I2C     ┌─────────────────────┐
 *   │   DialCampanario    │◄──────────►│   Campanarios.ino   │
 *   │   (M5Dial Master)   │            │   (ESP32 Slave)     │
 *   │                     │            │                     │
 *   │ • Control físico    │            │ • Control relés     │
 *   │ • Menús M5Dial      │            │ • WebSocket server  │
 *   │ • Envía comandos    │            │ • Recibe comandos   │
 *   │ • Recibe estados    │            │ • Ejecuta acciones  │
 *   └─────────────────────┘            └─────────────────────┘
 *                                                 │
 *                                       WebSocket │
 *                                                 ▼
 *                                       ┌─────────────────────┐
 *                                       │     Frontend Web    │
 *                                       │   /data/Campanas.js │
 *                                       │   /data/index.html  │
 *                                       └─────────────────────┘
 * @author  Julian Salas Baertolome
 * @date    22/06/2025
 * @version 1.0
 * @note    Este programa requiere las librerías Wire, WiFi, DNSServer, ESPAsyncWebServer y RTC para su funcionamiento.
 * @note    En este proyecto se utilizan las calases CAMPANA, CALEFACCION, Campanario y RTC para gestionar las campanas, calefacción y tiempo respectivamente.
 */
  #include "DNSServicio.h"
  #include "Servidor.h"
  #include "Campana.h"
  #include "Campanario.h"
  #include "RTC.h"
  #include "ConexionWifi.h"
  #include "OTAServicio.h"
  #include <Wire.h>
  #include "Auxiliar.h"
  #include "ModoAp.h"
  #include "Configuracion.h"
  //#include "TimeManager.h"
  #include "Alarmas.h"
  #include "Acciones.h"
  #include "I2CServicio.h"
  #include "TelegramServicio.h"
  #include "Debug.h"

  //#include "Acciones.h"
  
  

  AlarmScheduler Alarmas;
  
  void setup() {
    
      Serial.begin(9600);                                                             // Iniciar la comunicación serie a 9600 baudios

      pinMode(Config::Pins::CONFIGURACION, INPUT_PULLUP);                             // Comprueba el estado del pin PinConfiguracion para iniciar el modo AP si está en LOW para configurar el dispositivo

      if (digitalRead(Config::Pins::CONFIGURACION) == LOW) {                          // Si el pin PinConfiguracion está a LOW    
        DBG_INO("Pin PinConfiguracion a 0: Iniciando modo AP...");
        iniciarModoAP();                                                              // Inicia el modo AP para configurar el dispositivo           
        while(1)                                                                      // Bucle infinito para esperar a que se configure el Wifi                              
        {
          delay(100);
        }
      } else {
        cargarConfigWiFi();                                                           // Carga la configuración guardada
        DBG_INO("Iniciando Campanario...");
        initI2C();                                                                    // Inicializa el bus I2C como esclavo

        CAMPANA* campana1 = new CAMPANA(Config::Pins::CAMPANA1);                      // Crea una nueva instancia de la clase CAMPANA para la campana 1
        CAMPANA* campana2 = new CAMPANA(Config::Pins::CAMPANA2);                      // Crea una nueva instancia de la clase CAMPANA para la campana 2

        CALEFACCION* calefaccion = new CALEFACCION(Config::Pins::CALEFACCION);        // Crea una nueva instancia de la clase CALEFACCION

        Campanario.AddCampana(campana1);                                              // Añade la campana 1 al campanario
        Campanario.AddCampana(campana2);                                              // Añade la campana 2 al campanario
        Campanario.AddCalefaccion(calefaccion);                                       // Añade la calefacción al campanario  


        lConexionInternet = ConectarWifi(configWiFi);                                 // Llama a la función para conectar a la red Wi-Fi con la configuración cargada
        if (lConexionInternet)                                                        // Llama a la función para conectar a la red Wi-Fi
        {                                                                             // Si la conexión es exitosa
            ServidorOn(configWiFi.usuario, configWiFi.clave);                         // Llama a la función para iniciar el servidor
            Campanario.SetInternetConectado();                                        // Notifica al campanario que hay conexión a Internet
            DBG_INO("Conexión Wi-Fi exitosa.");
            
            // Inicializar servicio OTA
            OTA.begin();
            OTA.enableAutoUpdate(Config::OTA::AUTO_UPDATE_ENABLED);
            DBG_INO("Servicio OTA inicializado.");
            
            //Alarmas.begin(false);                                                      // Inicializa el sistema de alarmas sin cargar configuración por defecto
            IniciaAlarmas();                                                            // Llama a la función para iniciar las alarmas
            // Inicializar servicio Telegram (solo notificaciones)
            telegramBot.begin(Config::Telegram::BOT_TOKEN, Config::Telegram::CHAT_ID, Config::Telegram::CAMPANARIO_NOMBRE);
            DBG_INO("Servicio Telegram inicializado (solo notificaciones).");
            if (Config::Telegram::NOTIFICACION_START)
            {
              telegramBot.sendStartupNotification();
            }
        } else {
          Campanario.ClearInternetConectado();                                        // Notifica al campanario que no hay conexión a Internet
          DBG_INO("Error al conectar a la red Wi-Fi.");
        }
                                                              

        //Alarmas.add(DOW_TODOS, 8, 30, 300); // añadir más
        //Alarmas.add(DOW_TODOS, 8, 0, 0, &AlarmScheduler::accionTocaHora); // cada día a las 08:00
        //Alarmas.add(DOW_TODOS, ALARMA_WILDCARD, ALARMA_WILDCARD, 10, &AlarmScheduler::accionSecuencia, 300); // cada 10 min secuencia 300
        //Alarmas.addExternal0(DOW_TODOS, ALARMA_WILDCARD, 29, 4, accionPruebaReloj, true);         
//        Alarmas.addExternal0(DOW_TODOS, ALARMA_WILDCARD, 0, 0, accionTocaHora, true);                                      // Toca cada hora en punto con accionTocaHora()
//        Alarmas.addExternal0(DOW_TODOS, ALARMA_WILDCARD, 30, 0, accionTocaMedia, true);                                    // Toca cada media hora con accionTocaMedia()  
//        Alarmas.addExternal0(DOW_TODOS, 12, 2, 0, SincronizaNTP, true);                                                    // Sincroniza NTP al mediodía
//        Alarmas.addExternal0(DOW_TODOS, ALARMA_WILDCARD, 10, 0, ActualizaDNSSiNecesario, true);                             // Actualiza DNS si es necesario cada hora en el minuto 10
//        Alarmas.addExternal(DOW_DOMINGO, 11, 05, 0, &accionSecuencia, Config::States::I2CState::MISA);                     // Toca misa los domingos a las 11:05
//        Alarmas.addExternal(DOW_DOMINGO, 11, 25, 0, &accionSecuencia, Config::States::I2CState::MISA);                     // Toca misa los domingos a las 11:25

 //Alarmas.imprimirTodasLasAlarmas();

    }
  }
  void loop() {

    if (!Campanario.GetEstadoSecuencia()) {                                 // Si no hay secuencia de campanadas en curso
      if (RTC::isNtpSync()) {                                               // Si el RTC está sincronizado por NTP
        Alarmas.check();                                                    // Llama a la función para buscar las alarmas programadas
      }

      ActualizaEstadoProteccionCampanadas();                                // Llama a la función para comprobar si estamos en el período de proteccion de toque de campanas
      if (millis() - ultimoCheckInternet > Config::Network::INTERNET_CHECK_INTERVAL_MS) {      // Comprueba si ha pasado el intervalo de tiempo para verificar la conexión a Internet
          ultimoCheckInternet = millis();
          TestInternet();                                                   // Llama a la función para comprobar la conexión a Internet y actualizar el DNS si es necesario
      }
      
      // Comprobar actualizaciones OTA automáticas
      OTA.checkAutoUpdate();
      
      // Verificar mensajes de Telegram    
//      if (telegramBot.isEnabled()) {
//          telegramBot.checkMessages();
//      }
    }  
  
    if (secuenciaI2C > 0) {                                                 // Si se ha recibido orden por I2C
      if (secuenciaI2C == Config::States::SET_TEMPORIZADOR) {
          // Secuencias que SÍ necesitan parámetro:
          EjecutaSecuencia(secuenciaI2C, ParametroI2C, Config::Telegram::METODO_ACTIVACION_MANUAL);                     // Con parámetro I2C
          DBG_INO_PRINTF("I2C -> EjecutaSecuencia(%d, %d, %d)", secuenciaI2C, ParametroI2C, Config::Telegram::METODO_ACTIVACION_MANUAL  );
      } else {
          // Secuencias que NO necesitan parámetro:
          EjecutaSecuencia(secuenciaI2C, Config::Telegram::METODO_ACTIVACION_MANUAL);                                   // Sin parámetro
          DBG_INO_PRINTF("I2C -> EjecutaSecuencia(%d,  %d)", secuenciaI2C, Config::Telegram::METODO_ACTIVACION_MANUAL);
      }
      secuenciaI2C = 0;                                                     // Resetea para esperar la siguiente orden
      nToque = 0;                                                           // Resetea el numero de la secuencia a tocar
    }
    if (nToque > 0) {                                                       // Si la orden se ha recibido por websocket
      EjecutaSecuencia(nToque, Config::Telegram::METODO_ACTIVACION_MANUAL);                                              // Llama a la función para ejecutar la orden recibida de inernet
      nToque = 0;                                                           // Resetea el numero de la secuencia a tocar
    }
  
    TestCampanadas();                                                     // Llama a la función para probar las campanadas y enviar el número de campana tocada a los clientes conectados
  
    if ( Campanario.GetEstadoCalefaccion())
    {  
      nSegundosTemporizacion = Campanario.TestTemporizacionCalefaccion(); // Verifica el estado de la calefacción y obtiene el tiempo restante
      
      if (nSegundosTemporizacion == 0) {                                  // Verifica si la calefacción debe apagarse automáticamente
        nToque = Config::States::CALEFACCION_OFF;                         // Establece el estado de la calefacción a apagada
      } else {
        DBG_INO_PRINTF("Calefacción aún activa, quedan %.0f segundos para apagarse.", nSegundosTemporizacion);
      }

    }


  }

