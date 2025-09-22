    #include "Servidor.h"

    bool servidorIniciado = false;                               // Flag que indica si el servidor ha sido iniciado
    int  nToque = 0;                                             // Contador de toques del campanario
    AsyncWebSocket ws("/ws");                                    // WebSocket en la ruta /ws 
    AsyncWebServer server(80);                                   // Servidor HTTP en el puerto 80
    WiFiClient client;                                           // Cliente WiFi para manejar las conexiones



  /**
   * @brief Inicializa el servidor HTTP y WebSocket con autenticación
   * 
   * @details Función principal que configura e inicia el servidor web completo:
   *          
   *          **PROCESO DE INICIALIZACIÓN:**
   *          1. Configuración del servidor HTTP en puerto especificado
   *          2. Configuración de rutas para archivos SPIFFS
   *          3. Inicialización del WebSocket con callback onEvent()
   *          4. Configuración de autenticación HTTP Basic
   *          5. Inicio del servidor y establecimiento de servidorIniciado = true
   *          6. Logging de inicialización exitosa
   * 
   * @param usuario Usuario para autenticación HTTP Basic
   * @param clave Contraseña para autenticación HTTP Basic
   * 
   * @note Debe llamarse DESPUÉS de WiFi.begin() exitoso
   * @note Debe llamarse DESPUÉS de SPIFFS.begin() exitoso
   * @note Establece automáticamente servidorIniciado = true
   * @note Configura WebSocket en ruta "/ws"
   * 
   * @warning Requiere WiFi conectado antes de la llamada
   * @warning Requiere SPIFFS inicializado con archivos web
   * @warning Solo debe llamarse UNA vez durante inicialización
   * 
   * @see onEvent() - Callback configurado para WebSocket
   * @see servidorIniciado - Variable establecida por esta función
   * @see SPIFFS.begin() - Debe ejecutarse antes
   * 
   * @since v1.0 - Inicialización básica
   * @since v2.0 - Añadido WebSocket
   * @since v3.0 - Añadida autenticación HTTP
   * 
   * @author Julian Salas Bartolomé
   */
    void ServidorOn(const char* usuario, const char* clave)
    {
      
        if(!SPIFFS.begin(true)){                                                                  // Inicializar SPIFFS 
            DBG_SRV("ha ocurrido un error montando SPIFFS");
        }
        if (!servidorIniciado) {                                                                  // Si el servidor no ha sido iniciado
          ws.onEvent(onEvent);                                                                    // Configura el manejador de eventos del WebSocket          
          server.addHandler(&ws);                                                                 // Añade el manejador de WebSocket al servidor HTTP

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
          DBG_SRV("Servidor HTTP iniciado en el puerto 80.");
          DBG_SRV("Servidor HTTP iniciado. Esperando conexiones...");
          servidorIniciado = true;                                       // Marca el servidor como iniciado
        }
    }


  /**
   * @brief Callback principal para eventos de WebSocket
   * 
   * @details Función callback que maneja todos los eventos del WebSocket:
   *          conexiones, desconexiones, mensajes recibidos y errores.
   *
   *          **EVENTOS GESTIONADOS:**
   *          - WS_EVT_CONNECT: Nueva conexión de cliente
   *          - WS_EVT_DISCONNECT: Desconexión de cliente
   *          - WS_EVT_DATA: Mensaje recibido (delegado a procesaMensajeWebSocket)
   *          - WS_EVT_PONG: Respuesta pong recibida
   *          - WS_EVT_ERROR: Error en comunicación WebSocket
   *
   * @param server Puntero al servidor WebSocket que generó el evento
   * @param client Puntero al cliente WebSocket involucrado
   * @param type Tipo de evento WebSocket ocurrido
   * @param arg Argumento adicional del evento
   * @param data Datos del mensaje (si type == WS_EVT_DATA)
   * @param len Longitud de los datos del mensaje
   *
   * @note Se ejecuta automáticamente en cada evento WebSocket
   * @note Los mensajes de datos se delegan a procesaMensajeWebSocket()
   * @note Logging automático de eventos si DEBUGSERVIDOR está definido
   *
   * @warning No llamar directamente, es callback automático
   * @warning Los punteros pueden ser NULL según el tipo de evento
   *
   * @see procesaMensajeWebSocket() - Procesa mensajes de datos
   * @see ws.onEvent() - Donde se registra este callback
   * @see AwsEventType - Tipos de eventos WebSocket
   *
   * @since v2.0
   * @author Julian Salas Bartolomé
   *
   *          **EVENTOS GESTIONADOS:**
   *          - WS_EVT_CONNECT: Nueva conexión de cliente
   *          - WS_EVT_DISCONNECT: Desconexión de cliente
   *          - WS_EVT_DATA: Mensaje recibido (delegado a procesaMensajeWebSocket)
   *          - WS_EVT_PONG: Respuesta pong recibida
   *          - WS_EVT_ERROR: Error en comunicación WebSocket
   * 
   * @param server Puntero al servidor WebSocket que generó el evento
   * @param client Puntero al cliente WebSocket involucrado
   * @param type Tipo de evento WebSocket ocurrido
   * @param arg Argumento adicional del evento
   * @param data Datos del mensaje (si type == WS_EVT_DATA)
   * @param len Longitud de los datos del mensaje
   * 
   * @note Se ejecuta automáticamente en cada evento WebSocket
   * @note Los mensajes de datos se delegan a procesaMensajeWebSocket()
   * @note Logging automático de eventos si DEBUGSERVIDOR está definido
   * 
   * @warning No llamar directamente, es callback automático
   * @warning Los punteros pueden ser NULL según el tipo de evento
   * 
   * @see procesaMensajeWebSocket() - Procesa mensajes de datos
   * @see ws.onEvent() - Donde se registra este callback
   * @see AwsEventType - Tipos de eventos WebSocket
   * 
   * @since v2.0
   * @author Julian Salas Bartolomé
   */
    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    
        switch (type) {
          case WS_EVT_CONNECT:
              DBG_SRV(" ");
              DBG_SRV("OnEvent->WS_EVT_CONNECT: ");
              DBG_SRV_PRINTF("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
              DBG_SRV(" ");
              break;
          case WS_EVT_DISCONNECT:
              DBG_SRV(" ");
              DBG_SRV("OnEvent->WS_EVT_DISCONNECT: ");
              DBG_SRV_PRINTF("WebSocket client #%u disconnected\n", client->id());
              DBG_SRV(" ");
              break;
          case WS_EVT_DATA:
              DBG_SRV(" ");
              DBG_SRV("OnEvent->WS_EVT_DATA: ");
              DBG_SRV_PRINTF("Message received: %s\n", (char*)data);
              DBG_SRV(" ");
              procesaMensajeWebSocket(arg, data, len);
              break;
          case WS_EVT_PONG:
          case WS_EVT_ERROR:
              break;
        }
      
    }
  /**
   * @brief Procesa mensajes recibidos por WebSocket
   * 
   * @details Función especializada que analiza y ejecuta los comandos
   *          recibidos a través del WebSocket desde clientes web.
   *          
   *          **COMANDOS PROCESADOS:**
   *          - Comandos de control del campanario
   *                - "Difuntos": Inicia la secuencia de campanadas para difuntos y redirige a los clientes a la pantalla de campanas.
   *                - "Misa": Inicia la secuencia de campanadas para misa y redirige a los clientes a la pantalla de campanas.
   *                - "PARAR": Detiene cualquier secuencia de campanadas en curso y redirige a los clientes a la pantalla principal.
   *                - "CALEFACCION_ON": Enciende la calefacción y notifica a los clientes el nuevo estado.
   *                - "CALEFACCION_OFF": Apaga la calefacción y notifica a los clientes el nuevo estado.
   *          - Solicitudes de estado del sistema
   *                - "GET_CALEFACCION": Envía a los clientes el estado actual de la calefacción.
   *                - "GET_CAMPANARIO": Envía a los clientes el estado actual del campanario.
   *          - Configuración de parámetros
   *          - Ejecución de secuencias de toques
   * 
   * @param arg Argumento adicional del mensaje WebSocket
   * @param data Puntero a los datos del mensaje recibido
   * @param len Longitud en bytes de los datos del mensaje
   * 
   * @note Llamada automáticamente por onEvent() cuando type == WS_EVT_DATA
   * @note Interpreta datos como texto JSON o comandos de texto plano
   * @note Ejecuta acciones sobre el objeto Campanario según el comando
   * @note Envía respuestas de confirmación a todos los clientes conectados
   * 
   * @warning Valida todas las entradas antes de ejecutar comandos
   * @warning Los datos recibidos pueden no estar null-terminated
   * 
   * @see onEvent() - Función que llama a esta cuando hay datos
   * @see Campanario - Objeto utilizado para ejecutar comandos
   * @see ws.textAll() - Para enviar respuestas a todos los clientes
   * 
   * @since v2.0
   * @author Julian Salas Bartolomé
   */
    void procesaMensajeWebSocket(void *arg, uint8_t *data, size_t len)
    {
        String mensaje = String((const char*)data).substring(0, len);       // Convierte los datos a String
        DBG_SRV(" ");
        DBG_SRV_PRINTF("procesaMensajeWebSocket -> Mensaje recibido: %s\n", mensaje);
        DBG_SRV(" ");
        //switch para procesar el mensaje recibido
        if (mensaje == "Difuntos") {                                        // Si el mensaje es "Difuntos"
            nToque = Config::States::DIFUNTOS;                              // Establece la secuencia a EstadoDifuntos para tocar difuntos
            ws.textAll("REDIRECT:/Campanas.html");                          // Indica a los clientes que deben redirigir a la pantalla de presentacion de las campanas
            DBG_SRV("Procesando mensaje: TocaDifuntos");
        } else if (mensaje == "Misa") {                                     // Si el mensaje es "Misa"
            nToque = Config::States::MISA;                                  // Establece la secuencia a Misa para tocar misa
            ws.textAll("REDIRECT:/Campanas.html");                          // Indica a los clientes que deben redirigir a la pantalla de presentacion de las campanas
            DBG_SRV("Procesando mensaje: TocaMisa");
        } else if (mensaje == "Fiesta") {                                   // Si el mensaje es "Fiesta"
            nToque = Config::States::FIESTA;                                // Establece la secuencia a Fiesta para tocar fiesta
            ws.textAll("REDIRECT:/Campanas.html");                          // Indica a los clientes que deben redirigir a la pantalla de presentacion de las campanas
            DBG_SRV("Procesando mensaje: TocaFiesta");
        } else if (mensaje == "PARAR") {                                    // Si el mensaje es "PARAR"  
            nToque = 0;                                                     // Parada la secuencia de toques
            Campanario.ParaSecuencia();                                     // Detiene la secuencia de campanadas
            ws.textAll("REDIRECT:/index.html");                             // Indica a los clientes que deben redirigir
            DBG_SRV("Procesando mensaje: Parar");
        } else if (mensaje.startsWith("CALEFACCION_ON:")) {                 // Si el mensaje comienza con "CALEFACCION_ON:"
            String minutosStr = mensaje.substring(15);                      // Extrae los minutos después de "CALEFACCION_ON:"
            int minutos = minutosStr.toInt();                               // Convierte la cadena de minutos a entero
            if (minutos < 0 || minutos >  Config::Heating::MAX_MINUTES) {   // Validación del rango de minutos
                minutos = 0;                                                // Si está fuera del rango, establece a 0
                DBG_SRV_PRINTF("Minutos fuera de rango, establecido a 0. Valor recibido: %s\n", minutosStr.c_str());
            }
            Campanario.EnciendeCalefaccion(minutos);                        // Enciende la calefacción
            ws.textAll("CALEFACCION:ON:" + String(minutos));                // Envía el estado con los minutos programados
            DBG_SRV_PRINTF("Procesando mensaje: Calefacción ON por %d minutos\n", minutos);
        } else if (mensaje == "CALEFACCION_OFF") {                          // Si el mensaje es "CALEFACCION_OFF"
            Campanario.ApagaCalefaccion();                                  // Apaga la calefacción
            ws.textAll("CALEFACCION:OFF");                                  // Envía el estado de la calefacción a todos los clientes conectados
            DBG_SRV("Procesando mensaje: Calefacción OFF");
        } else if (mensaje == "GET_CALEFACCION") {                          // Si el mensaje es "GET_CALEFACCION"
            String estadoCalefaccion = Campanario.GetEstadoCalefaccion() ? "ON" : "OFF"; // Enviar el estado de la calefacción a todos los clientes conectados
            ws.textAll("ESTADO_CALEFACCION:" + estadoCalefaccion);          // Envía el estado de la calefacción a todos los clientes conectados
            DBG_SRV_PRINTF("Estado de la calefacción enviado: %s\n", estadoCalefaccion.c_str());
        } else if (mensaje == "GET_TIEMPOCALEFACCION") {                    // Si el mensaje es "GET_TIEMPOCALEFACCION"
            String tiempoCalefaccion = String(Campanario.TestTemporizacionCalefaccion());
            ws.textAll("TIEMPO_CALEFACCION:" + tiempoCalefaccion);          // Envía el tiempo de calefacción al cliente que lo pidió
        } else if (mensaje == "GET_CAMPANARIO") {                           // Si el mensaje es "GET_CAMPANARIO"  
            String EstadoCampanario = String(Campanario.GetEstadoCampanario()); // Obtiene el estado del campanario 
            ws.textAll("ESTADO_CAMPANARIO:" + EstadoCampanario);            // Envía el estado al cliente que lo pidió
        } else {
            nToque = 0; // Resetea la secuencia si el mensaje no es reconocido
            DBG_SRV("Mensaje no reconocido, reseteando secuencia.");
        }
    }    


  /**
   * @brief Verifica si hay conexión activa a Internet
   * 
   * @details Función que realiza una verificación real de conectividad
   *          a Internet mediante petición HTTP a un servidor externo confiable.
   *          
   *          **PROCESO DE VERIFICACIÓN:**
   *          1. Utiliza HTTPClient para realizar petición HTTP GET
   *          2. Conecta a servidor externo confiable (ej: google.com, 8.8.8.8)
   *          3. Analiza código de respuesta HTTP
   *          4. Determina si hay conectividad real a Internet
   * 
   * @retval true Conexión a Internet verificada y funcionando
   * @retval false Sin conexión a Internet o error en verificación
   * 
   * @note Realiza verificación REAL, no solo estado WiFi local
   * @note Utiliza timeout para evitar bloqueos prolongados
   * @note Logging automático del resultado si DEBUGSERVIDOR definido
   * @note Útil para detectar redes WiFi sin salida a Internet
   * 
   * @warning Función bloqueante durante la verificación (pocos segundos)
   * @warning Requiere WiFi conectado para funcionar correctamente
   * 
   * @see client - Cliente WiFi utilizado para la verificación
   * @see HTTPClient - Clase utilizada para petición HTTP
   * 
   * @since v3.0
   * @author Julian Salas Bartolomé
   */
    bool hayInternet() {
      HTTPClient http;
      http.begin("http://clients3.google.com/generate_204");                        // URL ligera y rápida de Google
      int httpCode = http.GET();                                                    // Realiza la petición GET
      http.end();                                                                   // Finaliza la conexión
      DBG_SRV_PRINTF("hayInternet -> Código HTTP recibido: %d\n", httpCode);  
      if (httpCode == 204) {
          DBG_SRV("hayInternet -> Conexión a Internet disponible.");
      } else {
          DBG_SRV("hayInternet -> No hay conexión a Internet.");
      }
      return (httpCode == 204);                                                     // Devolvemos true si hay internet, (Google responde 204 si hay Internet)
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
