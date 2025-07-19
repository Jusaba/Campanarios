/**
 * @file Servidor.h
 * @brief Definiciones y funciones para el servidor web y WebSocket del sistema de campanario.
 *
 * Este archivo contiene las definiciones, variables globales y funciones necesarias
 * para la gestión del servidor HTTP y WebSocket en el sistema de domótica del campanario.
 * Permite la interacción remota con el sistema mediante una interfaz web, el control
 * de las campanas y la calefacción, así como la gestión de archivos en SPIFFS.
 *
 * Funcionalidades principales:
 * - Inicialización y configuración del servidor HTTP y WebSocket.
 * - Autenticación básica de usuarios para el acceso web.
 * - Procesamiento de mensajes recibidos por WebSocket para controlar el campanario y la calefacción.
 * - Notificación en tiempo real a los clientes web mediante WebSocket.
 * Como funciones auxiliares:
 * - Listado e impresión de archivos almacenados en SPIFFS.
 *
 * @author [Tu Nombre]
 * @date [Fecha de creación o última modificación]
 * @version 1.0
 */
#ifndef SERVIDOR_H
	#define SERVIDOR_H
    #include <WiFi.h>
    #include <AsyncTCP.h>
    #include <ESPAsyncWebServer.h>
    #include <SPIFFS.h>

    #include "Campanario.h"
    
    // Definiciones de estados del sistema
    // Estos estados se utilizan para controlar la secuencia de toques de las campanas
    // y el estado de la calefacción.
    #define EstadoInicio            0
    #define EstadoDifuntos          1
    #define EstadoMisa              2
    #define EstadoStop              3
    #define EstadoCalefaccionOn     4
    #define EstadoCalefaccionOff    5


    #define DEBUGSERVIDOR                                 // Descomentar para activar el modo de depuración del servidor

    bool servidorIniciado = false;                        // Variable para indicar si el servidor ha sido iniciado
    //AsyncWebSocket ws;                                  // WebSocket  
    int nToque = 0;                                       // Variable para almacenar la secuencia de botones pulsados
    
    // Pines de los relés para controlar las campanas
    const int Rele0 = 26;                           
    const int Rele1 = 27;
    const int Rele2 = 25;

    AsyncWebSocket ws("/ws");                                           // WebSocket en la ruta /ws 
    AsyncWebServer server(80);                                          // Servidor HTTP en el puerto 80
    WiFiClient client;                                                  // Cliente WiFi para manejar las conexiones

    // Usuario y contraseña
    //const char* http_username = "usuario";
    //const char* http_password = "clave";


    extern CAMPANARIO Campanario;

    void ServidorOn(const char* usuario, const char* clave);             // Función para iniciar el servidor HTTP y WebSocket
    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);  // Callback para manejar eventos del WebSocket
    void procesaMensajeWebSocket(void *arg, uint8_t *data, size_t len);  // Procesa los mensajes recibidos por WebSocket
    bool hayInternet(void);                                             // Comprueba si hay conexión a Internet

    // Funciones auxiliares
    // Estas funciones se utilizan para listar los archivos en SPIFFS y para imprimir el contenido de un archivo específico.
    // Se pueden utilizar para depurar o verificar el contenido de los archivos almacenados en el sistema de archivos SPIFFS.
    void listSPIFFS(void);
    void printFileContent(const char* filename);


  /**
   * @brief Inicializa y configura el servidor HTTP asíncrono.
   * 
   * Monta el sistema de archivos SPIFFS, configura los manejadores de eventos WebSocket y las rutas HTTP.
   * Protege la ruta principal ("/") con autenticación básica utilizando el usuario y clave proporcionados.
   * Sirve archivos estáticos desde SPIFFS y resetea la secuencia nToque al cargar la página principal.
   * 
   * @param usuario Nombre de usuario para autenticación básica.
   * @param clave   Contraseña para autenticación básica.
   */
    void ServidorOn(const char* usuario, const char* clave)
    {
      
        if(!SPIFFS.begin(true)){
            #ifdef DEBUGSERVIDOR
                Serial.println("ha ocurrido un error montando SPIFFS");
            #endif    
        }
//SPIFFS.format();        
//listSPIFFS();
//printFileContent("/index.html");         // Imprime el contenido del archivo index.html en la consola para verificar que se ha cargado correctamente

        if (!servidorIniciado) {
          ws.onEvent(onEvent);                                            // Configura el manejador de eventos del WebSocket          
          server.addHandler(&ws);                                         // Añade el manejador de WebSocket al servidor HTTP

          server.on("/", HTTP_GET, [usuario, clave](AsyncWebServerRequest *request){
            if(!request->authenticate(usuario, clave)) {
              return request->requestAuthentication();
            }
            request->send(SPIFFS, "/index.html", "text/html");
            nToque = 0; // Resetea la secuencia a 0 al cargar la página principal
          });

          server.on("/Campanas.html", HTTP_GET, [usuario, clave](AsyncWebServerRequest *request){
            if(!request->authenticate(usuario, clave)) {
              return request->requestAuthentication();
            }
            request->send(SPIFFS, "/Campanas.html", "text/html");
          });
          server.serveStatic("/", SPIFFS, "/");
          // Iniciar el servidor
          server.begin();
          #ifdef DEBUGSERVIDOR
              Serial.println("Servidor HTTP iniciado en el puerto 80.");
              Serial.println("Servidor HTTP iniciado. Esperando conexiones...");
          #endif
          servidorIniciado = true;                                       // Marca el servidor como iniciado
        }
    }


  /**
   * @brief Manejador de eventos del WebSocket.
   * 
   * Esta función estática actúa como callback para los eventos del WebSocket.
   * Redirige los eventos al manejador de eventos de la instancia (_Eventos)
   * si existe una instancia válida.
   * 
   * @param server Puntero al servidor WebSocket
   * @param client Puntero al cliente WebSocket que generó el evento
   * @param type Tipo de evento WebSocket recibido
   * @param arg Argumentos adicionales del evento
   * @param data Datos recibidos en el evento
   * @param len Longitud de los datos recibidos
   */
    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    
        switch (type) {
          case WS_EVT_CONNECT:
              #ifdef DEBUGSERVIDOR
                Serial.println(" ");
                Serial.print("OnEvent->WS_EVT_CONNECT: ");
                Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
                Serial.println(" ");
              #endif
            break;
          case WS_EVT_DISCONNECT:
            #ifdef DEBUGSERVIDOR
              Serial.println(" ");
              Serial.print("OnEvent->WS_EVT_DISCONNECT: ");
              Serial.printf("WebSocket client #%u disconnected\n", client->id());
              Serial.println(" ");
            #endif      
            break;
          case WS_EVT_DATA:
            #ifdef DEBUGSERVIDOR
              Serial.println(" ");
              Serial.print("OnEvent->WS_EVT_DATA: ");
              Serial.printf("Message received: %s\n", (char*)data);
              Serial.println(" ");
            #endif
            procesaMensajeWebSocket(arg, data, len);
            break;
          case WS_EVT_PONG:
          case WS_EVT_ERROR:
            break;
        }
      
    }

  /**
   * @brief Procesa los mensajes recibidos a través de WebSocket y ejecuta acciones según el contenido del mensaje.
   *
   * Esta función interpreta el mensaje recibido desde un cliente WebSocket y realiza diferentes acciones en el sistema,
   * como iniciar secuencias de campanadas, controlar la calefacción, detener secuencias o informar estados.
   * Además, envía respuestas o comandos a todos los clientes conectados según corresponda.
   *
   * @param arg Puntero a argumentos adicionales (no utilizado en esta implementación).
   * @param data Puntero al buffer que contiene los datos del mensaje recibido.
   * @param len Longitud del mensaje recibido en bytes.
   *
   * Mensajes reconocidos y acciones asociadas:
   * - "Difuntos": Inicia la secuencia de campanadas para difuntos y redirige a los clientes a la pantalla de campanas.
   * - "Misa": Inicia la secuencia de campanadas para misa y redirige a los clientes a la pantalla de campanas.
   * - "PARAR": Detiene cualquier secuencia de campanadas en curso y redirige a los clientes a la pantalla principal.
   * - "CALEFACCION_ON": Enciende la calefacción y notifica a los clientes el nuevo estado.
   * - "CALEFACCION_OFF": Apaga la calefacción y notifica a los clientes el nuevo estado.
   * - "GET_CALEFACCION": Envía a los clientes el estado actual de la calefacción.
   * - "GET_CAMPANARIO": Envía a los clientes el estado actual del campanario.
   * - Cualquier otro mensaje: Resetea la secuencia de campanadas.
   *
   * @note Si está definido DEBUGSERVIDOR, se imprimen mensajes de depuración por el puerto serie.
   */
    void procesaMensajeWebSocket(void *arg, uint8_t *data, size_t len)
    {
        String mensaje = String((const char*)data).substring(0, len);
        #ifdef DEBUGSERVIDOR
          Serial.println(" ");
          Serial.printf("procesaMensajeWebSocket -> Mensaje recibido: %s\n", mensaje);
          Serial.println(" ");
        #endif
        //switch para procesar el mensaje recibido
        if (mensaje == "Difuntos") {                               // 
            nToque = EstadoDifuntos;                               // Establece la secuencia a EstadoDifuntos para tocar difuntos
            ws.textAll("REDIRECT:/Campanas.html");                 // Indica a los clientes que deben redirigir a la pantalla de presentacion de las campanas
            #ifdef DEBUGSERVIDOR
              Serial.println("Procesando mensaje: TocaDifuntos");
            #endif
        } else if (mensaje == "Misa") {                            // Si el mensaje es "Misa"
            nToque = EstadoMisa;                                   // Establece la secuencia a Misa para tocar misa
            ws.textAll("REDIRECT:/Campanas.html");                 // Indica a los clientes que deben redirigir a la pantalla de presentacion de las campanas
             #ifdef DEBUGSERVIDOR
              Serial.println("Procesando mensaje: TocaMisa");
            #endif
        } else if (mensaje == "PARAR") {                          // Si el mensaje es "PARAR"  
            nToque = 0;                                           // Parada la secuencia de toques
            Campanario.ParaSecuencia();                           // Detiene la secuencia de campanadas
            ws.textAll("REDIRECT:/index.html");                   // Indica a los clientes que deben redirigir            
            #ifdef DEBUGSERVIDOR
              Serial.println("Procesando mensaje: Parar");
            #endif   
        } else if (mensaje == "CALEFACCION_ON") {                 // Si el mensaje es "CALEFACCION_ON"  
            Campanario.EnciendeCalefaccion();                     // Enciende la calefacción
            ws.textAll("CALEFACCION:ON");                         // Envía el estado de la calefacción a todos los clientes conectados
            #ifdef DEBUGSERVIDOR
              Serial.println("Procesando mensaje: Calefacción ON");
            #endif
        } else if (mensaje == "CALEFACCION_OFF") {                // Si el mensaje es "CALEFACCION_OFF"
            Campanario.ApagaCalefaccion();                        // Apaga la calefacción
            ws.textAll("CALEFACCION:OFF");                        // Envía el estado de la calefacción a todos los clientes conectados
            #ifdef DEBUGSERVIDOR
              Serial.println("Procesando mensaje: Calefacción OFF");
            #endif
        } else if (mensaje == "GET_CALEFACCION") {                // Si el mensaje es "GET_CALEFACCION"
            String estadoCalefaccion = Campanario.GetEstadoCalefaccion() ? "ON" : "OFF"; // Enviar el estado de la calefacción a todos los clientes conectados
            ws.textAll("ESTADO_CALEFACCION:" + estadoCalefaccion);// Envía el estado de la calefacción a todos los clientes conectados
            #ifdef DEBUGSERVIDOR
              Serial.printf("Estado de la calefacción enviado: %s\n", estadoCalefaccion.c_str());
            #endif
        } else if (mensaje == "GET_CAMPANARIO") {                 // Si el mensaje es "GET_CAMPANARIO"  
            String EstadoCampanario = String(Campanario.GetEstadoCampanario()); // Obtiene el estado del campanario 
            ws.textAll("ESTADO_CAMPANARIO:" + EstadoCampanario);  // Envía el estado al cliente que lo pidió
        } else {
            nToque = 0; // Resetea la secuencia si el mensaje no es reconocido
            #ifdef DEBUGSERVIDOR
              Serial.println("Mensaje no reconocido, reseteando secuencia.");
            #endif
        }
    }    


    bool hayInternet() {
      HTTPClient http;
      http.begin("http://clients3.google.com/generate_204"); // URL ligera y rápida de Google
      int httpCode = http.GET();
      http.end();
      return (httpCode == 204); // Google responde 204 si hay Internet
  }

  /**
   * @brief Lista los archivos almacenados en el sistema de archivos SPIFFS y muestra su nombre y tamaño por el puerto serie.
   *
   * Esta función recorre todos los archivos presentes en la raíz del sistema de archivos SPIFFS,
   * imprimiendo el nombre y el tamaño de cada archivo utilizando la interfaz Serial.
   * Es útil para depuración y verificación del contenido almacenado en la memoria SPIFFS del dispositivo.
   */
    void listSPIFFS() {

      Serial.println("Archivos en SPIFFS:");
      File root = SPIFFS.open("/");
      File file = root.openNextFile();

      while (file) {
          Serial.print("Nombre: ");
          Serial.println(file.name());
          Serial.print("Tamaño: ");
          Serial.println(file.size());
          file = root.openNextFile(); // Abrir el siguiente archivo
      }
}  

  /**
   * @brief Imprime el contenido de un archivo en el monitor serie línea por línea.
   * 
   * Esta función abre el archivo especificado por el nombre de archivo en modo lectura,
   * imprime su contenido línea por línea en el monitor serie y luego cierra el archivo.
   * Si ocurre un error al abrir el archivo, se muestra un mensaje de error.
   * 
   * @param filename Nombre del archivo a leer e imprimir.
   */
   void printFileContent(const char* filename) {
     File file = SPIFFS.open(filename, "r"); // Abrir el archivo en modo lectura
     if (!file) {
         Serial.printf("Error al abrir el archivo: %s\n", filename);
         return;
     }
     Serial.printf("Contenido del archivo %s:\n", filename);
     while (file.available()) {
         String line = file.readStringUntil('\n'); // Leer línea por línea
         Serial.println(line); // Imprimir la línea en el monitor serie
     }
     file.close(); // Cerrar el archivo
   }


#endif