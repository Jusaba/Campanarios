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
  #include <Wire.h>
  #include "Auxiliar.h"
  #include "ModoAp.h"
  
  
  #define DEBUG
  
  
  
  void setup() {
    
      Serial.begin(9600);                                           // Iniciar la comunicación serie a 9600 baudios
  
      pinMode(PinConfiguracion, INPUT_PULLUP);                      // Comprueba el estado del pin PinConfiguracion para iniciar el modo AP si está en LOW para configurar el dispositivo
      if (digitalRead(PinConfiguracion) == LOW) {
        #ifdef DEBUG
          Serial.println("Pin PinConfiguracion a 0: Iniciando modo AP..."); 
        #endif  
        iniciarModoAP();
        while(1)                                                    // Bucle infinito para esperar a que se configure el Wifi                              
        {
          delay(100);
        }
      } else {
      
        cargarConfigWiFi();                                         // Carga la configuración guardada
        #ifdef DEBUG
          Serial.println("Iniciando Campanario...");
        #endif
      
        Wire.begin(I2C_SLAVE_ADDR);                                 // Iniciar el bus I2C como esclavo con la dirección definida
        Wire.setClock(100000);
        Wire.onReceive(recibirSecuencia);
        Wire.onRequest(enviarRequest);
      
        CAMPANA* campana1 = new CAMPANA(PinCampana1);               // Crea una nueva instancia de la clase CAMPANA para la campana 1
        CAMPANA* campana2 = new CAMPANA(PinCampana2);               // Crea una nueva instancia de la clase CAMPANA para la campana 2
      
        CALEFACCION* calefaccion = new CALEFACCION(CalefaccionPin); // Crea una nueva instancia de la clase CALEFACCION   
      
        Campanario.AddCampana(campana1);                            // Añade la campana 1 al campanario
        Campanario.AddCampana(campana2);                            // Añade la campana 2 al campanario
        Campanario.AddCalefaccion(calefaccion);                     // Añade la calefacción al campanario  
      
        lConexionInternet = ConectarWifi(configWiFi);               // Llama a la función para conectar a la red Wi-Fi con la configuración cargada
        if (lConexionInternet)                                      // Llama a la función para conectar a la red Wi-Fi
        {                                                           // Si la conexión es exitosa
            ServidorOn(configWiFi.usuario, configWiFi.clave);       // Llama a la función para iniciar el servidor
            Campanario.SetInternetConectado();                     // Notifica al campanario que hay conexión a Internet
            #ifdef DEBUG
              Serial.println("Conexión Wi-Fi exitosa.");
            #endif
        } else {
          Campanario.ClearInternetConectado(); // Notifica al campanario que no hay conexión a Internet
          #ifdef DEBUG
            Serial.println("Error al conectar a la red Wi-Fi.");
          #endif
        }
      
    }  
    }
  
  void loop() {

    if (!Campanario.GetEstadoSecuencia()) {
      if (RTC::isNtpSync()) {
        ChekearCuartos();                                             // Llama a la función para chequear los cuartos y las horas y tocar las campanas correspondientes
      }
      EsPeriodoToqueCampanas();                                       // Llama a la función para comprobar si estamos en el período de proteccion de toque de campanas
      if (millis() - ultimoCheckInternet > intervaloCheckInternet) {  // Comprueba si ha pasado el intervalo de tiempo para verificar la conexión a Internet
          ultimoCheckInternet = millis();
          TestInternet();                                            // Llama a la función para comprobar la conexión a Internet y actualizar el DNS si es necesario
      }
    }  
  
    if (secuenciaI2C > 0) {                                         // Si se ha recibido orden por I2C
      EjecutaSecuencia(secuenciaI2C);                               // Llama a la función para ejecutar la orden recibida 
      secuenciaI2C = 0;                                             // Resetea para esperar la siguiente orden
      nToque = 0;                                                   // Resetea el numero de la secuencia a tocar
    }
    
    if (nToque > 0) {                                               // Si la orden se ha recibido por websocket
      EjecutaSecuencia(nToque);                                     // Llama a la función para ejecutar la orden recibida de inernet
      nToque = 0;                                                   // Resetea el numero de la secuencia a tocar  
    }
  
    TestCampanadas();                                               // Llama a la función para probar las campanadas y enviar el número de campana tocada a los clientes conectados
  
    if ( Campanario.GetEstadoCalefaccion())
    {  
      double segundos = Campanario.TestTemporizacionCalefaccion(); // Verifica el estado de la calefacción y obtiene el tiempo restante
      
      if (segundos == 0) {                                        // Verifica si la calefacción debe apagarse automáticamente
        nToque = EstadoCalefaccionOff;                            // Establece el estado de la calefacción a apagada
        } else {
        #ifdef DEBUGSERVIDOR
          Serial.print("Calefacción aún activa, quedan.");
          Serial.print(segundos);
          Serial.println(" segundos para apagarse.");
        #endif
      }

    }


  }

