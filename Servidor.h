#ifndef SERVIDOR_H
	#define SERVIDOR_H
    #include <WiFi.h>
    #include <AsyncTCP.h>
    #include <ESPAsyncWebServer.h>
     #include <SPIFFS.h>

    #include "Campanario.h"
    
    #define DEBUGSERVIDOR

    //AsyncWebSocket ws;                                                     // WebSocket  
    int nToque = 0;                                                    // Variable para almacenar la secuencia de botones pulsados

    const int Rele0 = 26;
    const int Rele1 = 27;
    const int Rele2 = 25;

    AsyncWebSocket ws("/ws");
    AsyncWebServer server(80);
    WiFiClient client;

    extern CAMPANARIO Campanario;

    void ServidorOn(void);
    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    void listSPIFFS(void);
    void printFileContent(const char* filename);
    void procesaMensajeWebSocket(void *arg, uint8_t *data, size_t len);


    void ServidorOn(void)
    {
        if(!SPIFFS.begin(true)){
            #ifdef DEBUGSERVIDOR
                Serial.println("ha ocurrido un error montando SPIFFS");
            #endif    
        }
/*
if (SPIFFS.format()) {
    Serial.println("SPIFFS formateado correctamente.");
} else {
    Serial.println("Error al formatear SPIFFS.");
}                
*/        
//        listSPIFFS(); // Lista los archivos en SPIFFS al iniciar el servidor
//        printFileContent("/index.html"); // Imprime el contenido del archivo index.html al iniciar el servidor  
//        printFileContent("/Campanas.js"); // Imprime el contenido del archivo Misa.js al iniciar el servidor
//        printFileContent ("/Campanario.js)"); // Imprime el contenido del archivo Campanario.js al iniciar el servidor

        ws.onEvent(onEvent);
        server.addHandler(&ws);

        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(SPIFFS, "/index.html", "text/html");
            nToque = 0; // Resetea la secuencia a 0 al cargar la página principal
        });
        server.on("/Campanas.html", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(SPIFFS, "/Campanas.html", "text/html");
        });
        server.serveStatic("/", SPIFFS, "/");
        // Iniciar el servidor
        server.begin();
        #ifdef DEBUGSERVIDOR
            Serial.println("Servidor HTTP iniciado en el puerto 80.");
            Serial.println("Servidor HTTP iniciado. Esperando conexiones...");
        #endif
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

    void procesaMensajeWebSocket(void *arg, uint8_t *data, size_t len)
    {
        String mensaje = String((const char*)data).substring(0, len);
        #ifdef DEBUGSERVIDOR
          Serial.println(" ");
          Serial.printf("procesaMensajeWebSocket -> Mensaje recibido: %s\n", mensaje);
          Serial.println(" ");
        #endif
        //switch para procesar el mensaje recibido
        if (mensaje == "Difuntos") {
            nToque = 1;                               // Establece la secuencia a 1 para tocar difuntos
            ws.textAll("REDIRECT:/Campanas.html");    // Indica a los clientes que deben redirigir a la pantalla de presentacion de las campanas
            #ifdef DEBUGSERVIDOR
              Serial.println("Procesando mensaje: TocaDifuntos");
            #endif
        } else if (mensaje == "Fiesta") {
            nToque = 2;                               // Establece la secuencia a 2 para tocar fiesta
            ws.textAll("REDIRECT:/Campanas.html");    // Indica a los clientes que deben redirigir a la pantalla de presentacion de las campanas
            #ifdef DEBUGSERVIDOR
              Serial.println("Procesando mensaje: TocaFiesta");
            #endif
        } else if (mensaje == "Misa") {
            nToque = 3; // Establece la secuencia a 3 para tocar misa
            ws.textAll("REDIRECT:/Campanas.html");    // Indica a los clientes que deben redirigir a la pantalla de presentacion de las campanas
             #ifdef DEBUGSERVIDOR
              Serial.println("Procesando mensaje: TocaMisa");
            #endif
        } else if (mensaje == "PARAR") {
            nToque = 0; // Parada la secuencia de toques
            Campanario.ParaSecuencia(); // Detiene la secuencia de campanadas
            ws.textAll("REDIRECT:/index.html"); // Indica a los clientes que deben redirigir            
            #ifdef DEBUGSERVIDOR
              Serial.println("Procesando mensaje: Parar");
            #endif   
        } else if (mensaje == "CALEFACCION_ON") {
            Campanario.EnciendeCalefaccion(); // Enciende la calefacción
            #ifdef DEBUGSERVIDOR
              Serial.println("Procesando mensaje: Calefacción ON");
            #endif
        } else if (mensaje == "CALEFACCION_OFF") {
            Campanario.ApagaCalefaccion(); // Apaga la calefacción
            #ifdef DEBUGSERVIDOR
              Serial.println("Procesando mensaje: Calefacción OFF");
            #endif
        } else if (mensaje == "GET_CALEFACCION") {
            // Enviar el estado de la calefacción a todos los clientes conectados
            String estadoCalefaccion = Campanario.GetEstadoCalefaccion() ? "ON" : "OFF";
            ws.textAll("ESTADO_CALEFACCION:" + estadoCalefaccion);
            #ifdef DEBUGSERVIDOR
              Serial.printf("Estado de la calefacción enviado: %s\n", estadoCalefaccion.c_str());
            #endif
        } else {
            nToque = 0; // Resetea la secuencia si el mensaje no es reconocido
            #ifdef DEBUGSERVIDOR
              Serial.println("Mensaje no reconocido, reseteando secuencia.");
            #endif
        }

       // ws.textAll("Hola caracola"); // Enviar los datos a todos los clientes
    }    

    void listSPIFFS() {
    /*
    if (!SPIFFS.begin(true)) { // Montar el sistema de archivos SPIFFS
        Serial.println("Error al montar SPIFFS");
        return;
    }
    */
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

    //SPIFFS.end(); // Desmontar SPIFFS
}  

void printFileContent(const char* filename) {
  /*
  if (!SPIFFS.begin(true)) { // Montar el sistema de archivos SPIFFS
      Serial.println("Error al montar SPIFFS");
      return;
  }
  */
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
  //SPIFFS.end(); // Desmontar SPIFFS
}

#endif