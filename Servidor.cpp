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
      
        if (!SPIFFS.begin(true)) {
            DBG_SRV("❌ Error al montar SPIFFS");
        } else {
            DBG_SRV("✅ SPIFFS montado correctamente");
            String idiomaServidor = cargarIdiomaDesdeConfig();
            DBG_SRV_PRINTF("🌍 Idioma del servidor: %s", idiomaServidor.c_str());
            
            // ✅ CARGAR CONFIGURACIÓN DE TELEGRAM AL INICIO
            cargarConfigTelegramDesdeSPIFFS();
            DBG_SRV_PRINTF("📱 Configuración de Telegram cargada: %s (%s)", 
                          Config::Telegram::CAMPANARIO_NOMBRE.c_str(),
                          Config::Telegram::CAMPANARIO_UBICACION.c_str());
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

            server.on("/alarmas.html", HTTP_GET, [usuario, clave](AsyncWebServerRequest *request){
              if(!request->authenticate(usuario, clave)) {
                return request->requestAuthentication();
              }
              request->send(SPIFFS, "/alarmas.html", "text/html");
            });

            // ✅ ENDPOINT PARA DESCARGAR ARCHIVOS DE SPIFFS
            server.on("/download", HTTP_GET, [usuario, clave](AsyncWebServerRequest *request){
              // Verificar autenticación
              if(!request->authenticate(usuario, clave)) {
                return request->requestAuthentication();
              }
              
              if (!request->hasParam("file")) {
                DBG_SRV("❌ Parámetro 'file' no encontrado en la petición");
                request->send(400, "text/plain", "Parametro 'file' requerido");
                return;
              }
              
              String filename = request->getParam("file")->value();
              DBG_SRV_PRINTF("📥 Solicitud de descarga: %s", filename.c_str());
              
              // Validar que el archivo existe y está en la lista permitida
              if (filename != "alarmas_personalizadas.json" && 
                  filename != "telegram_config.json" && 
                  filename != "config.json" &&
                  filename != "Secuencias.json") {
                request->send(403, "text/plain", "Archivo no permitido");
                DBG_SRV_PRINTF("❌ Intento de descarga de archivo no permitido: %s", filename.c_str());
                return;
              }
              
              String filepath = "/" + filename;
              
              if (!SPIFFS.exists(filepath)) {
                request->send(404, "text/plain", "Archivo no encontrado en SPIFFS");
                DBG_SRV_PRINTF("❌ Archivo no encontrado: %s", filepath.c_str());
                return;
              }
              
              DBG_SRV_PRINTF("✅ Descargando archivo: %s", filename.c_str());
              
              // Enviar archivo con headers apropiados
              AsyncWebServerResponse *response = request->beginResponse(SPIFFS, filepath, "application/json", true);
              response->addHeader("Content-Disposition", "attachment; filename=\"" + filename + "\"");
              request->send(response);
            });

            // ✅ ENDPOINT PARA SUBIR/RESTAURAR ARCHIVOS A SPIFFS
            server.on("/upload", HTTP_POST, [usuario, clave](AsyncWebServerRequest *request){
              if(!request->authenticate(usuario, clave)) {
                return request->requestAuthentication();
              }
              request->send(200);
            }, [usuario, clave](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
              // Verificar autenticación en cada chunk
              if(!request->authenticate(usuario, clave)) {
                return;
              }
              
              static File uploadFile;
              
              if (index == 0) {
                // Cerrar archivo anterior si quedó abierto
                if (uploadFile) {
                  uploadFile.close();
                }
                
                // Inicio de upload
                DBG_SRV_PRINTF("📤 Iniciando upload: %s", filename.c_str());
                
                // Validar que el archivo está en la lista permitida
                if (filename != "alarmas_personalizadas.json" && 
                    filename != "telegram_config.json" && 
                    filename != "config.json" &&
                    filename != "Secuencias.json") {
                  DBG_SRV_PRINTF("❌ Archivo no permitido para upload: %s", filename.c_str());
                  return;
                }
                
                String filepath = "/" + filename;
                uploadFile = SPIFFS.open(filepath, "w");
                if (!uploadFile) {
                  DBG_SRV_PRINTF("❌ Error abriendo archivo para escritura: %s", filepath.c_str());
                  return;
                }
              }
              
              // Escribir chunk de datos
              if (uploadFile) {
                size_t written = uploadFile.write(data, len);
                if (written != len) {
                  DBG_SRV_PRINTF("⚠️  Escritura parcial: %d de %d bytes", written, len);
                }
              }
              
              if (final) {
                // Fin de upload
                if (uploadFile) {
                  uploadFile.flush();  // Forzar escritura a SPIFFS
                  uploadFile.close();
                  DBG_SRV_PRINTF("✅ Upload completado: %s (%d bytes)", filename.c_str(), index + len);
                  
                  // Si es telegram_config.json, recargar configuración
                  if (filename == "telegram_config.json") {
                    cargarConfigTelegramDesdeSPIFFS();
                    DBG_SRV("🔄 Configuración de Telegram recargada desde archivo subido");
                  }
                  
                  // Si es alarmas_personalizadas.json, recargar alarmas
                  if (filename == "alarmas_personalizadas.json") {
                    // Aquí podrías llamar a una función para recargar alarmas si existe
                    DBG_SRV("🔄 Alarmas recargadas desde archivo subido");
                  }
                  
                  // Si es Secuencias.json, recargar secuencias
                  if (filename == "Secuencias.json") {
                    Campanario.CargarSecuencias();
                    DBG_SRV("🔄 Secuencias de campanadas recargadas desde archivo subido");
                  }
                }
              }
            });

            // 🔍 ENDPOINT DEBUG: Listar archivos en SPIFFS
            server.on("/list-spiffs", HTTP_GET, [usuario, clave](AsyncWebServerRequest *request){
              if(!request->authenticate(usuario, clave)) {
                return request->requestAuthentication();
              }
              
              String output = "Archivos en SPIFFS:\n\n";
              File root = SPIFFS.open("/");
              File file = root.openNextFile();
              
              while(file) {
                output += "- ";
                output += file.name();
                output += " (";
                output += String(file.size());
                output += " bytes)\n";
                file = root.openNextFile();
              }
              
              request->send(200, "text/plain", output);
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
/*
            if (telegramBot.isEnabled() && Config::Telegram::NOTIFICACION_DIFUNTOS) {
                telegramBot.sendSequenceNotification("Difuntos", Config::Telegram::METODO_ACTIVACION_WEB);
            }
*/
        } else if (mensaje == "Misa") {                                     // Si el mensaje es "Misa"
            nToque = Config::States::MISA;                                  // Establece la secuencia a Misa para tocar misa
            ws.textAll("REDIRECT:/Campanas.html");                          // Indica a los clientes que deben redirigir a la pantalla de presentacion de las campanas
            DBG_SRV("Procesando mensaje: TocaMisa");
/*
            if (telegramBot.isEnabled() && Config::Telegram::NOTIFICACION_MISA) {
                telegramBot.sendSequenceNotification("Misa", Config::Telegram::METODO_ACTIVACION_WEB);
            }   
*/
        } else if (mensaje == "Fiesta") {                                   // Si el mensaje es "Fiesta"
            nToque = Config::States::FIESTA;                                // Establece la secuencia a Fiesta para tocar fiesta
            ws.textAll("REDIRECT:/Campanas.html");                          // Indica a los clientes que deben redirigir a la pantalla de presentacion de las campanas
            DBG_SRV("Procesando mensaje: TocaFiesta");
/*

                telegramBot.sendSequenceNotification("Fiesta", Config::Telegram::METODO_ACTIVACION_WEB);
            }
*/
        } else if (mensaje == "PARAR") {                                    // Si el mensaje es "PARAR"  
            nToque = 0;                                                     // Parada la secuencia de toques
            Campanario.ParaSecuencia();                                     // Detiene la secuencia de campanadas
            ws.textAll("REDIRECT:/index.html");                             // Indica a los clientes que deben redirigir
            DBG_SRV("Procesando mensaje: Parar");
            if (telegramBot.isEnabled() && Config::Telegram::NOTIFICACION_STOP) {
                telegramBot.sendStopNotification(Config::Telegram::METODO_ACTIVACION_WEB);
            }
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
            if (telegramBot.isEnabled() && Config::Telegram::NOTIFICACION_CALEFACCION_ON) {
                telegramBot.sendCalefaccionOnNotification(Config::Telegram::METODO_ACTIVACION_WEB);
            }

            
        } else if (mensaje == "CALEFACCION_OFF") {                          // Si el mensaje es "CALEFACCION_OFF"
            Campanario.ApagaCalefaccion();                                  // Apaga la calefacción
            ws.textAll("CALEFACCION:OFF");                                  // Envía el estado de la calefacción a todos los clientes conectados
            DBG_SRV("Procesando mensaje: Calefacción OFF");
            if (telegramBot.isEnabled() && Config::Telegram::NOTIFICACION_CALEFACCION_OFF) {
                telegramBot.sendCalefaccionOffNotification(Config::Telegram::METODO_ACTIVACION_WEB);
            }
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
        }else if (mensaje== "GET_SECUENCIA_ACTIVA") {                       // Si el mensaje es "GET_SECUENCIA"  
            String secuenciaActiva = String(Campanario.GetSecuenciaActiva());  // Obtiene la secuencia actual
            ws.textAll("SECUENCIAACTIVA:" + secuenciaActiva);               // Envía la secuencia al cliente que lo pidió    ---- NO UTILIZADO EN ESTA VERSION ----
        }else if (mensaje.startsWith("ADD_ALARMA_WEB:") || 
         mensaje.startsWith("EDIT_ALARMA_WEB:") || 
         mensaje.startsWith("DELETE_ALARMA_WEB:") || 
         mensaje.startsWith("TOGGLE_ALARMA_WEB:") || 
         mensaje == "GET_ALARMAS_WEB" || 
         mensaje == "GET_STATS_ALARMAS_WEB") {
        // Separar comando y datos
            int separador = mensaje.indexOf(':');
            String comando = (separador > 0) ? mensaje.substring(0, separador) : mensaje;
            String datos = (separador > 0) ? mensaje.substring(separador + 1) : "{}";
            procesarComandoAlarma(nullptr, comando, datos);
        } else if (mensaje.startsWith("SET_IDIOMA:")) {
            String nuevoIdioma = mensaje.substring(11);
            DBG_SRV_PRINTF("🌍 Cambiando idioma a: %s", nuevoIdioma.c_str());

            if (nuevoIdioma == "ca" || nuevoIdioma == "es") {
                if (guardarIdiomaEnConfig(nuevoIdioma)) {
                    // Notificar a todos los clientes conectados
                    ws.textAll("IDIOMA_CAMBIADO:" + nuevoIdioma);
                    DBG_SRV("✅ Idioma guardado y notificado a todos los clientes");
                } else {
                    ws.textAll("ERROR_IDIOMA:No se pudo guardar");
                }
            } else {
                ws.textAll("ERROR_IDIOMA:Idioma no soportado");
            }

        } else if (mensaje == "GET_CONFIG") {
            DBG_SRV("📋 Enviando configuración actual");
            String config = obtenerConfiguracionJSON();
            ws.textAll("CONFIG_ACTUAL:" + config);

        } else if (mensaje == "GET_IDIOMA") {
            String idioma = cargarIdiomaDesdeConfig();
            ws.textAll("IDIOMA_ACTUAL:" + idioma);
            DBG_SRV_PRINTF("📤 Enviando idioma actual: %s", idioma.c_str());
        
        } else if (mensaje.startsWith("SAVE_CONFIG_TELEGRAM:")) {
            String jsonConfig = mensaje.substring(21); // Extraer JSON después de "SAVE_CONFIG_TELEGRAM:"
            DBG_SRV_PRINTF("📱 Guardando configuración de Telegram: %s", jsonConfig.c_str());
            
            if (guardarConfigTelegramEnSPIFFS(jsonConfig)) {
                ws.textAll("CONFIG_TELEGRAM_OK");
                DBG_SRV("✅ Configuración de Telegram guardada correctamente");
            } else {
                ws.textAll("CONFIG_TELEGRAM_ERROR");
                DBG_SRV("❌ Error al guardar configuración de Telegram");
            }
        
        } else if (mensaje == "GET_CONFIG_TELEGRAM") {
            String config = cargarConfigTelegramDesdeSPIFFS();
            ws.textAll("CONFIG_TELEGRAM:" + config);
            DBG_SRV_PRINTF("📤 Enviando configuración de Telegram: %s", config.c_str());
        
        } else if (mensaje.startsWith("VERIFY_PIN:")) {
            String pin = mensaje.substring(11); // Extraer PIN después de "VERIFY_PIN:"
            DBG_SRV_PRINTF("🔐 Verificando PIN recibido: %s", pin.c_str());
            
            if (verificarPinAcceso(pin)) {
                ws.textAll("PIN_OK");
                DBG_SRV("✅ PIN correcto");
            } else {
                ws.textAll("PIN_ERROR");
                DBG_SRV("❌ PIN incorrecto");
            }
        
        } else if (mensaje == "GET_VERSION_OTA") {
            // Enviar versión actual del firmware
            ws.textAll("VERSION_OTA:" + String(Config::OTA::FIRMWARE_VERSION));
            DBG_SRV_PRINTF("📤 Versión actual enviada: %s", Config::OTA::FIRMWARE_VERSION);
        
        } else if (mensaje == "CHECK_UPDATE_OTA") {
            // Comprobar si hay actualizaciones disponibles
            DBG_SRV("🔍 Comprobando actualizaciones OTA...");
            
            VersionInfo versionInfo = OTA.checkForUpdates();
            
            if (versionInfo.newVersionAvailable) {
                // Enviar información de actualización disponible en JSON (evita parseo frágil por ":")
                JsonDocument otaDoc;
                otaDoc["version"] = versionInfo.latestVersion;
                otaDoc["firmwareUrl"] = versionInfo.firmwareUrl;
                otaDoc["spiffsUrl"] = versionInfo.spiffsUrl;

                String notes = versionInfo.releaseNotes;
                if (notes.length() > Config::OTA::MAX_RELEASE_NOTES_WS) {
                    notes = notes.substring(0, Config::OTA::MAX_RELEASE_NOTES_WS);
                    notes += "\n...";
                }
                otaDoc["releaseNotes"] = notes;

                String payload;
                serializeJson(otaDoc, payload);
                ws.textAll("UPDATE_AVAILABLE_JSON:" + payload);
                DBG_SRV_PRINTF("✅ Nueva versión disponible: %s", versionInfo.latestVersion.c_str());
            } else {
                ws.textAll("NO_UPDATE");
                DBG_SRV("ℹ️ No hay actualizaciones disponibles");
            }
        
        } else if (mensaje == "START_UPDATE_FIRMWARE" || mensaje == "START_UPDATE_SPIFFS" || mensaje == "START_UPDATE_COMPLETE") {
            // Iniciar proceso de actualización OTA
            DBG_SRV_PRINTF("🚀 Iniciando actualización OTA: %s", mensaje.c_str());
            
            // Primero comprobar qué versión hay disponible
            VersionInfo versionInfo = OTA.checkForUpdates();
            
            if (!versionInfo.newVersionAvailable) {
                ws.textAll("OTA_ERROR:No hay actualizaciones disponibles");
                DBG_SRV("❌ No hay actualizaciones para instalar");
            } else {
                // Configurar callbacks antes de actualizar
                OTA.setProgressCallback([](int progress, const char* message) {
                    String msg = "OTA_PROGRESS:" + String(progress) + ":" + String(message ? message : "");
                    ws.textAll(msg);
                    DBG_OTA_PRINTF("📊 Progreso: %d%% - %s", progress, message ? message : "");
                });
                
                OTA.setErrorCallback([](const char* error) {
                    String msg = "OTA_ERROR:" + String(error ? error : "Error desconocido");
                    ws.textAll(msg);
                    DBG_OTA_PRINTF("❌ Error: %s", error ? error : "Error desconocido");
                });
                
                OTA.setSuccessCallback([](const char* version) {
                    String msg = "OTA_SUCCESS:" + String(version ? version : "");
                    ws.textAll(msg);
                    DBG_OTA_PRINTF("✅ Actualización completada: %s", version ? version : "");
                });
                
                bool resultado = false;
                
                if (mensaje == "START_UPDATE_FIRMWARE") {
                    // Solo actualizar firmware
                    ws.textAll("OTA_PROGRESS:0:Descargando firmware...");
                    resultado = OTA.updateFirmware(versionInfo.firmwareUrl, versionInfo.firmwareSize);
                    
                    if (resultado) {
                        // Enviar éxito ANTES de reiniciar
                        ws.textAll("OTA_SUCCESS:" + versionInfo.latestVersion);
                        DBG_SRV_PRINTF("✅ Firmware actualizado a v%s. Reiniciando...", versionInfo.latestVersion.c_str());
                        
                        // Dar tiempo para que el mensaje llegue al cliente
                        delay(2000);
                        
                        // Reiniciar ESP32
                        ESP.restart();
                    }
                    
                } else if (mensaje == "START_UPDATE_SPIFFS") {
                    // Solo actualizar SPIFFS
                    ws.textAll("OTA_PROGRESS:0:Descargando SPIFFS...");
                    resultado = OTA.updateSPIFFS(versionInfo.spiffsUrl, versionInfo.spiffsSize);
                    
                    if (resultado) {
                        // Enviar éxito ANTES de reiniciar
                        ws.textAll("OTA_SUCCESS:" + versionInfo.latestVersion);
                        DBG_SRV("✅ SPIFFS actualizado. Reiniciando...");
                        
                        // Dar tiempo para que el mensaje llegue al cliente
                        delay(2000);
                        
                        // Reiniciar ESP32
                        ESP.restart();
                    }
                    
                } else if (mensaje == "START_UPDATE_COMPLETE") {
                    // Actualización completa asíncrona (no bloquea WebSocket)
                    ws.textAll("OTA_PROGRESS:0:Iniciando actualización completa...");
                    OTA.performFullUpdateAsync(versionInfo);
                    resultado = true;  // La tarea se ejecuta en segundo plano
                }
                
                if (resultado) {
                    DBG_SRV("✅ Actualización iniciada correctamente");
                } else {
                    ws.textAll("OTA_ERROR:Error al iniciar la actualización");
                    DBG_SRV("❌ Error al iniciar actualización OTA");
                }
            }
        
        } else if (mensaje == "RESET_SYSTEM") {
            // Reiniciar el sistema
            DBG_SRV("🔄 Reinicio del sistema solicitado por el usuario");
            ws.textAll("RESET_OK");
            
            // Dar tiempo para enviar la respuesta
            delay(500);
            
            // Reiniciar ESP32
            ESP.restart();
        
        } else {
            nToque = 0; // Resetea la secuencia si el mensaje no es reconocido
            DBG_SRV("Mensaje no reconocido, reseteando secuencia.");
        }
    }    
    /**
    * @brief Procesa comandos específicos de gestión de alarmas personalizables
    * 
    * @details Función especializada que maneja todos los comandos relacionados
    *          con alarmas personalizables recibidos a través del WebSocket.
    *          
    *          **COMANDOS PROCESADOS:**
    *          - ADD_ALARMA_WEB: Crea nueva alarma personalizable con callback automático
    *          - EDIT_ALARMA_WEB: Modifica alarma existente manteniendo callback original
    *          - DELETE_ALARMA_WEB: Elimina alarma personalizable del sistema
    *          - TOGGLE_ALARMA_WEB: Habilita/deshabilita alarma específica
    *          - GET_ALARMAS_WEB: Obtiene listado completo de alarmas personalizables
    *          - GET_STATS_ALARMAS_WEB: Obtiene estadísticas del sistema de alarmas
    *          
    *          **TIPOS DE ACCIÓN SOPORTADOS:**
    *          - MISA: Configura callback accionSecuencia con parámetro MISA
    *          - DIFUNTOS: Configura callback accionSecuencia con parámetro DIFUNTOS  
    *          - FIESTA: Configura callback accionSecuencia con parámetro personalizado
    *          
    *          **PROCESO DE CREACIÓN:**
    *          1. Deserializa datos JSON del comando
    *          2. Determina callback y parámetro según tipo de acción
    *          3. Convierte día web (0-7) a máscara de días del sistema
    *          4. Llama a Alarmas.addPersonalizable() con parámetros procesados
    *          5. Envía confirmación o error a todos los clientes WebSocket
    * 
    * @param client Puntero al cliente WebSocket que envió el comando (puede ser nullptr)
    * @param comando String con el comando a procesar (ADD_ALARMA_WEB, EDIT_ALARMA_WEB, etc.)
    * @param datos String con datos JSON del comando a procesar
    * 
    * @note Llamada automáticamente por procesaMensajeWebSocket() para comandos de alarmas
    * @note Utiliza deserializeJson() para procesar datos de entrada
    * @note Configura callbacks automáticamente según el tipo de acción
    * @note Envía respuestas a TODOS los clientes conectados vía ws.textAll()
    * 
    * @warning Valida todos los datos JSON antes de procesar comandos
    * @warning Los callbacks son configurados automáticamente, no personalizables por comando
    * @warning Comando EDIT no modifica callbacks existentes por seguridad
    * 
    * @see procesaMensajeWebSocket() - Función que delega comandos de alarmas
    * @see AlarmScheduler::addPersonalizable() - Método para crear alarmas
    * @see AlarmScheduler::modificarPersonalizable() - Método para modificar alarmas
    * @see AlarmScheduler::eliminarPersonalizable() - Método para eliminar alarmas
    * @see convertirDiaAMascara() - Función auxiliar para conversión de días
    * @see accionSecuencia() - Callback configurado automáticamente
    * 
    * @since v2.1 - Sistema de alarmas personalizables vía web
    * @author Julian Salas Bartolomé
    */
    void procesarComandoAlarma(AsyncWebSocketClient *client, const String& comando, const String& datos) {
        JsonDocument doc;
        deserializeJson(doc, datos);
           
            if (comando == "ADD_ALARMA_WEB") {
               // Determinar callback y parámetro según el tipo
               String tipoAccion = doc["accion"] | "MISA";
               void (*callback)(uint16_t) = nullptr;
               uint16_t parametro = 0;

               if (tipoAccion == "MISA") {
                   callback = accionSecuencia;
                   parametro = Config::States::I2CState::MISA;
                   DBG_SRV("🔔 Configurando callback MISA");
               } else if (tipoAccion == "DIFUNTOS") {
                   callback = accionSecuencia;
                   parametro = Config::States::I2CState::DIFUNTOS;
                   DBG_SRV("🔔 Configurando callback DIFUNTOS");
               } else if (tipoAccion == "FIESTA") {
                   callback = accionSecuencia;
                   parametro = Config::States::I2CState::FIESTA;
                   DBG_SRV("🔔 Configurando callback FIESTA");
               } else if (tipoAccion == "CALEFACCION") {
                   callback = accionEnciendeCalefaccion;
                   parametro = doc["duracion"] | 30;  // Duración en minutos (por defecto 30)
                   DBG_SRV_PRINTF("🔥 Configurando callback CALEFACCION con %d minutos", parametro);
               }

               if (callback) {
                   uint8_t idx = Alarmas.addPersonalizable(
                       doc["nombre"] | "",
                       doc["descripcion"] | "",
                       convertirDiaAMascara(doc["dia"] | 0),
                       doc["hora"] | 0,
                       doc["minuto"] | 0,
                       tipoAccion.c_str(),
                       parametro,
                       callback,
                       doc["habilitada"] | true
                   );

                   if (idx < AlarmScheduler::MAX_ALARMAS) {
                       ws.textAll("ALARMA_CREADA_WEB:" + String(Alarmas.get(idx)->idWeb));
                   } else {
                       ws.textAll("ERROR_ALARMA_WEB:Máximo de alarmas alcanzado");
                   }
               }   
            } else if (comando == "EDIT_ALARMA_WEB") {
                
                String tipoAccion = doc["accion"] | "MISA";
                void (*callback)(uint16_t) = nullptr;
                uint16_t parametro = 0;
            
                if (tipoAccion == "MISA") {
                    callback = accionSecuencia;
                    parametro = Config::States::I2CState::MISA;
                    DBG_SRV("🔧 Configurando callback MISA para edición");
                } else if (tipoAccion == "DIFUNTOS") {
                    callback = accionSecuencia;
                    parametro = Config::States::I2CState::DIFUNTOS;
                    DBG_SRV("🔧 Configurando callback DIFUNTOS para edición");
                } else if (tipoAccion == "FIESTA") {
                    callback = accionSecuencia;
                    parametro = Config::States::I2CState::FIESTA;
                    DBG_SRV("🔧 Configurando callback FIESTA para edición");
                } else if (tipoAccion == "CALEFACCION") {
                    callback = accionEnciendeCalefaccion;
                    parametro = doc["duracion"] | 30;  // Duración en minutos (por defecto 30)
                    DBG_SRV_PRINTF("🔥 Configurando callback CALEFACCION para edición con %d minutos", parametro);
                }
            
                // ✅ VERIFICAR callback válido
                if (callback == nullptr) {
                    DBG_SRV_PRINTF("❌ ERROR: Callback es NULL para tipo '%s'", tipoAccion.c_str());
                    ws.textAll("ERROR_ALARMA_WEB:Tipo de acción no válido");
                    return;
                }
            
                // ✅ LLAMAR con callback y parámetro (igual que ADD_ALARMA_WEB)
                bool resultado = Alarmas.modificarPersonalizable(
                    doc["id"] | -1,
                    doc["nombre"] | "",
                    doc["descripcion"] | "",
                    convertirDiaAMascara(doc["dia"] | 0),
                    doc["hora"] | 0,
                    doc["minuto"] | 0,
                    tipoAccion.c_str(),
                    doc["habilitada"] | true,
                    callback,      // ✅ PASAR CALLBACK
                    parametro      // ✅ PASAR PARÁMETRO
                );
            
                if (resultado) {
                    ws.textAll("ALARMA_MODIFICADA_WEB:" + String(doc["id"] | -1));
                    DBG_SRV("✅ Alarma modificada con callback reasignado");
                } else {
                    ws.textAll("ERROR_ALARMA_WEB:No se pudo modificar");
                }
            } else if (comando == "DELETE_ALARMA_WEB") {
               int id = doc["id"] | -1;
               if (Alarmas.eliminarPersonalizable(id)) {
                   ws.textAll("ALARMA_ELIMINADA_WEB:" + String(id));
               } else {
                   ws.textAll("ERROR_ALARMA_WEB:No se pudo eliminar");
               }
           } else if (comando == "TOGGLE_ALARMA_WEB") {
               int id = doc["id"] | -1;
               bool estado = doc["habilitada"] | false;
               if (Alarmas.habilitarPersonalizable(id, estado)) {
                   ws.textAll("ALARMA_TOGGLED_WEB:" + String(id) + ":" + (estado ? "true" : "false"));
               }
           } else if (comando == "GET_ALARMAS_WEB") {
               String jsonAlarmas = Alarmas.obtenerPersonalizablesJSON();
               ws.textAll("ALARMAS_WEB:" + jsonAlarmas);
           } else if (comando == "GET_STATS_ALARMAS_WEB") {
               String jsonStats = Alarmas.obtenerEstadisticasJSON();
               ws.textAll("STATS_ALARMAS_WEB:" + jsonStats);
           }
    }
    /**
     * @brief Convierte día web (0-7) a máscara de días del sistema de alarmas
     * 
     * @details Función auxiliar que traduce la numeración de días utilizada
     *          en la interfaz web a las máscaras de bits utilizadas internamente
     *          por el sistema de alarmas del campanario.
     *          
     *          **CONVERSIÓN DE DÍAS:**
     *          - 0: Todos los días (DOW_TODOS = 0x7F)
     *          - 1: Domingo (bit 0 = 0x01)
     *          - 2: Lunes (bit 1 = 0x02)
     *          - 3: Martes (bit 2 = 0x04)
     *          - 4: Miércoles (bit 3 = 0x08)
     *          - 5: Jueves (bit 4 = 0x10)
     *          - 6: Viernes (bit 5 = 0x20)
     *          - 7: Sábado (bit 6 = 0x40)
     *          
     *          **VALIDACIÓN:**
     *          - Valores fuera del rango 0-7 se convierten a DOW_TODOS
     *          - Garantiza compatibilidad con sistema interno de máscaras
     * 
     * @param dia Día en formato web (0=todos, 1=domingo, 2=lunes, ..., 7=sábado)
     * 
     * @retval uint8_t Máscara de bits compatible con sistema de alarmas
     * @retval DOW_TODOS Si día está fuera del rango válido (0-7)
     * 
     * @note Utilizada internamente por procesarComandoAlarma()
     * @note Compatible con constantes DOW_* definidas en el sistema
     * @note Domingo se considera día 1 (no 0) en la numeración web
     * 
     * @warning Valores inválidos se convierten automáticamente a DOW_TODOS
     * @warning La numeración web difiere del estándar ISO (lunes = día 1)
     * 
     * @see procesarComandoAlarma() - Función principal que utiliza esta conversión
     * @see DOW_TODOS, DOW_DOMINGO, DOW_LUNES, etc. - Constantes del sistema
     * @see AlarmScheduler::addPersonalizable() - Recibe máscara convertida
     * 
     * @since v2.1 - Sistema de alarmas personalizables vía web
     * @author Julian Salas Bartolomé
     */
    uint8_t convertirDiaAMascara(int dia) {
        if (dia == 0) {
            DBG_SRV("🔄 Convertiendo día 0 (todos los días) a DOW_TODOS");
            return DOW_TODOS;
        }

        if (dia >= 1 && dia <= 7) {
            uint8_t mascara = 1 << (dia - 1);
            DBG_SRV_PRINTF("🔄 Convertiendo día %d a máscara 0x%02X", dia, mascara);
            return mascara;
        }

        DBG_SRV_PRINTF("⚠️ Día inválido %d, usando DOW_TODOS", dia);
        return DOW_TODOS;
    }

    // ============================================================================
    // GESTIÓN DE IDIOMA PERSISTENTE
    // ============================================================================

/**
 * @brief Carga la configuración de idioma persistente desde SPIFFS
 * 
 * @details Función que recupera el idioma configurado por el usuario desde
 *          el archivo de configuración almacenado en SPIFFS, garantizando
 *          persistencia entre reinicios del dispositivo.
 *          
 *          **PROCESO DE CARGA:**
 *          1. Verifica existencia del archivo /config.json en SPIFFS
 *          2. Si no existe, crea configuración por defecto (catalán)
 *          3. Lee y parsea contenido JSON del archivo de configuración
 *          4. Extrae campo "idioma" con fallback a catalán si hay errores
 *          5. Valida y retorna código de idioma configurado
 *          
 *          **IDIOMAS SOPORTADOS:**
 *          - "ca": Catalán (idioma por defecto)
 *          - "es": Español (castellano)
 *          
 *          **MANEJO DE ERRORES:**
 *          - Archivo inexistente: Crea configuración por defecto
 *          - Error de lectura: Retorna catalán por defecto
 *          - JSON malformado: Retorna catalán por defecto
 *          - Campo faltante: Utiliza catalán como fallback
 * 
 * @retval String Código de idioma configurado ("ca" o "es")
 * @retval "ca" Si hay errores o no existe configuración previa
 * 
 * @note Llamada durante inicialización del servidor web
 * @note Crea archivo de configuración automáticamente si no existe
 * @note Utiliza logging de depuración si DEBUGSERVIDOR está definido
 * @note Garantiza que siempre retorna un idioma válido
 * 
 * @warning Requiere SPIFFS montado correctamente antes de la llamada
 * @warning Función síncrona, puede bloquear brevemente durante lectura de archivo
 * 
 * @see guardarIdiomaEnConfig() - Función complementaria para guardar idioma
 * @see ServidorOn() - Función que utiliza idioma cargado durante inicialización
 * @see obtenerConfiguracionJSON() - Función relacionada para configuración completa
 * 
 * @since v3.0 - Sistema de configuración persistente de idioma
 * @author Julian Salas Bartolomé
 */
String cargarIdiomaDesdeConfig() {
    DBG_SRV("📂 Cargando idioma desde configuración...");
    
    if (!SPIFFS.exists("/config.json")) {
        DBG_SRV("⚠️ Archivo config.json no existe, creando con idioma por defecto");
        guardarIdiomaEnConfig("ca"); // Catalán por defecto
        return "ca";
    }
    
    File file = SPIFFS.open("/config.json", "r");
    if (!file) {
        DBG_SRV("❌ Error al abrir config.json");
        return "ca";
    }
    
    String contenido = file.readString();
    file.close();
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, contenido);
    
    if (error) {
        DBG_SRV_PRINTF("❌ Error al parsear config.json: %s", error.c_str());
        return "ca";
    }
    
    String idioma = doc["idioma"] | "ca";
    DBG_SRV_PRINTF("✅ Idioma cargado: %s", idioma.c_str());
    
    return idioma;
}

/**
 * @brief Guarda la configuración de idioma de forma persistente en SPIFFS
 * 
 * @details Función que almacena el idioma seleccionado por el usuario en
 *          el archivo de configuración SPIFFS, preservando configuración
 *          existente y garantizando persistencia entre reinicios.
 *          
 *          **PROCESO DE GUARDADO:**
 *          1. Carga configuración existente de /config.json si existe
 *          2. Preserva campos de configuración no relacionados con idioma
 *          3. Actualiza campo "idioma" con nuevo valor especificado
 *          4. Añade metadatos de configuración completa (zona horaria, etc.)
 *          5. Serializa y guarda JSON actualizado en SPIFFS
 *          6. Valida operación de escritura antes de confirmar éxito
 *          
 *          **ESTRUCTURA DEL ARCHIVO CONFIG.JSON:**
 *          ```json
 *          {
 *            "version": "1.0",
 *            "idioma": "ca|es",
 *            "configuracion": {
 *              "idioma_defecto": "ca|es",
 *              "zona_horaria": "Europe/Madrid",
 *              "formato_hora": "24h"
 *            }
 *          }
 *          ```
 *          
 *          **VALIDACIONES:**
 *          - Preserva configuración existente no relacionada con idioma
 *          - Crea estructura completa si archivo no existía previamente
 *          - Verifica escritura exitosa antes de confirmar operación
 * 
 * @param idioma Código de idioma a guardar ("ca" para catalán, "es" para español)
 * 
 * @retval true Idioma guardado correctamente en SPIFFS
 * @retval false Error durante escritura o creación del archivo
 * 
 * @note Llamada automáticamente cuando usuario cambia idioma vía web
 * @note Preserva configuración existente no relacionada con idioma
 * @note Utiliza logging de depuración si DEBUGSERVIDOR está definido
 * @note Crea archivo completo con metadatos si no existía previamente
 * 
 * @warning Requiere SPIFFS montado y con espacio suficiente disponible
 * @warning Función síncrona, puede bloquear brevemente durante escritura
 * @warning No valida códigos de idioma, acepta cualquier string
 * 
 * @see cargarIdiomaDesdeConfig() - Función complementaria para cargar idioma
 * @see procesaMensajeWebSocket() - Procesa comandos SET_IDIOMA que llaman esta función
 * @see obtenerConfiguracionJSON() - Función relacionada para configuración completa
 * 
 * @since v3.0 - Sistema de configuración persistente de idioma
 * @author Julian Salas Bartolomé
 */
bool guardarIdiomaEnConfig(const String& idioma) {
    DBG_SRV_PRINTF("💾 Guardando idioma en configuración: %s", idioma.c_str());
    
    JsonDocument doc;
    
    // Cargar configuración existente si existe
    if (SPIFFS.exists("/config.json")) {
        File file = SPIFFS.open("/config.json", "r");
        if (file) {
            String contenido = file.readString();
            file.close();
            deserializeJson(doc, contenido); // Ignorar errores, usar estructura vacía si falla
        }
    }
    
    // Actualizar idioma
    doc["version"] = "1.0";
    doc["idioma"] = idioma;
    doc["configuracion"]["idioma_defecto"] = idioma;
    doc["configuracion"]["zona_horaria"] = "Europe/Madrid";
    doc["configuracion"]["formato_hora"] = "24h";
    
    // Guardar archivo
    File file = SPIFFS.open("/config.json", "w");
    if (!file) {
        DBG_SRV("❌ Error al crear config.json");
        return false;
    }
    
    serializeJson(doc, file);
    file.close();
    
    DBG_SRV("✅ Configuración guardada correctamente");
    return true;
}

/**
 * @brief Obtiene la configuración completa del sistema en formato JSON
 * 
 * @details Función que retorna toda la configuración del sistema como
 *          string JSON, incluyendo idioma, zona horaria y preferencias
 *          de formato, para ser enviada a clientes web.
 *          
 *          **CONFIGURACIÓN INCLUIDA:**
 *          - Versión del archivo de configuración
 *          - Idioma actualmente configurado
 *          - Idioma por defecto del sistema
 *          - Zona horaria configurada
 *          - Formato de hora preferido
 *          
 *          **PROCESO DE OBTENCIÓN:**
 *          1. Verifica existencia de archivo /config.json
 *          2. Si no existe, genera configuración por defecto
 *          3. Lee contenido completo del archivo
 *          4. Retorna JSON como string para envío a cliente
 *          
 *          **CONFIGURACIÓN POR DEFECTO:**
 *          ```json
 *          {
 *            "version": "1.0",
 *            "idioma": "ca",
 *            "configuracion": {
 *              "idioma_defecto": "ca",
 *              "zona_horaria": "Europe/Madrid", 
 *              "formato_hora": "24h"
 *            }
 *          }
 *          ```
 * 
 * @retval String JSON completo con toda la configuración del sistema
 * @retval "{}" Si hay error crítico leyendo el archivo
 * 
 * @note Llamada cuando cliente web solicita configuración vía GET_CONFIG
 * @note Genera configuración por defecto si archivo no existe
 * @note Retorna JSON como string listo para envío WebSocket
 * @note No modifica archivos, solo lectura de configuración
 * 
 * @warning Requiere SPIFFS montado correctamente
 * @warning Función síncrona, puede bloquear brevemente durante lectura
 * @warning Retorna "{}" en caso de error crítico de lectura
 * 
 * @see cargarIdiomaDesdeConfig() - Función relacionada para solo idioma
 * @see guardarIdiomaEnConfig() - Función relacionada para guardar configuración
 * @see procesaMensajeWebSocket() - Procesa comandos GET_CONFIG que llaman esta función
 * 
 * @since v3.0 - Sistema de gestión de configuración completa
 * @author Julian Salas Bartolomé
 */

String obtenerConfiguracionJSON() {
    if (!SPIFFS.exists("/config.json")) {
        // Crear configuración por defecto
        JsonDocument doc;
        doc["version"] = "1.0";
        doc["idioma"] = "ca";
        doc["configuracion"]["idioma_defecto"] = "ca";
        doc["configuracion"]["zona_horaria"] = "Europe/Madrid";
        doc["configuracion"]["formato_hora"] = "24h";
        
        String resultado;
        serializeJson(doc, resultado);
        return resultado;
    }
    
    File file = SPIFFS.open("/config.json", "r");
    if (!file) return "{}";
    
    String contenido = file.readString();
    file.close();
    
    return contenido;
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

/**
 * @brief Guarda la configuración de Telegram en SPIFFS
 * 
 * @details Almacena la configuración de notificaciones de Telegram en formato JSON
 *          en el archivo /telegram_config.json en SPIFFS. Incluye:
 *          - Nombre del dispositivo (identificador del campanario)
 *          - Ubicación física del campanario
 *          - Preferencias de notificaciones (qué eventos notificar)
 * 
 * @param jsonConfig String con configuración en formato JSON desde el cliente web
 * 
 * @retval true Configuración guardada correctamente en SPIFFS
 * @retval false Error al guardar (problema de escritura o JSON inválido)
 * 
 * @note El archivo se crea/sobrescribe completamente en cada guardado
 * @note Formato JSON esperado:
 *       {
 *         "nombre": "manolis",
 *         "ubicacion": "Lleida",
 *         "notificaciones": {
 *           "inicio": true,
 *           "misa": true,
 *           "difuntos": true,
 *           "fiesta": true,
 *           "calefaccion": true,
 *           "alarma": true
 *         }
 *       }
 * 
 * @warning Requiere SPIFFS montado y con espacio disponible
 * @warning No valida el contenido del JSON (responsabilidad del cliente)
 * 
 * @see cargarConfigTelegramDesdeSPIFFS() - Función complementaria para cargar
 * @see procesaMensajeWebSocket() - Procesa comando SAVE_CONFIG_TELEGRAM
 * 
 * @since v3.1 - Sistema de configuración dinámica de Telegram
 * @author Julian Salas Bartolomé
 */
bool guardarConfigTelegramEnSPIFFS(const String& jsonConfig) {
    DBG_SRV("💾 Guardando configuración de Telegram en SPIFFS...");
    
    // Validar que el JSON no esté vacío
    if (jsonConfig.length() == 0) {
        DBG_SRV("❌ JSON vacío, abortando guardado");
        return false;
    }
    
    // Guardar directamente el JSON recibido
    File file = SPIFFS.open("/telegram_config.json", "w");
    if (!file) {
        DBG_SRV("❌ Error al crear telegram_config.json");
        return false;
    }
    
    file.print(jsonConfig);
    file.close();
    
    // ✅ ACTUALIZAR VARIABLES GLOBALES inmediatamente después de guardar
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonConfig);
    
    if (!error) {
        // Actualizar nombre y ubicación
        Config::Telegram::CAMPANARIO_NOMBRE = doc["nombre"] | "";
        Config::Telegram::CAMPANARIO_UBICACION = doc["ubicacion"] | "";
        
        // Actualizar preferencias de notificaciones
        if (doc.containsKey("notificaciones")) {
            JsonObject notif = doc["notificaciones"];
            Config::Telegram::NOTIFICACION_START = notif["inicio"] | true;
            Config::Telegram::NOTIFICACION_MISA = notif["misa"] | true;
            Config::Telegram::NOTIFICACION_DIFUNTOS = notif["difuntos"] | true;
            Config::Telegram::NOTIFICACION_FIESTA = notif["fiesta"] | true;
            Config::Telegram::NOTIFICACION_STOP = notif["stop"] | false;
            Config::Telegram::NOTIFICACION_CALEFACCION_ON = notif["calefaccion"] | true;
            Config::Telegram::NOTIFICACION_CALEFACCION_OFF = notif["calefaccion_off"] | false;
            Config::Telegram::NOTIFICACION_ALARMAS = notif["alarma"] | true;
            Config::Telegram::NOTIFICACION_ERRORES = notif["errores"] | false;
            Config::Telegram::NOTIFICACION_INTERNET_RECONEXION = notif["internet"] | false;
            Config::Telegram::NOTIFICACION_HORA = notif["hora"] | false;
            Config::Telegram::NOTIFICACION_MEDIAHORA = notif["mediahora"] | false;
            Config::Telegram::NOTIFICACION_NTP_SYNC = notif["ntp"] | false;
            Config::Telegram::NOTIFICACION_DNS_UPDATE = notif["dns"] | false;
            Config::Telegram::NOTIFICACION_ALARMA_PROGRAMADA = notif["alarma_programada"] | false;
        }
        
        DBG_SRV("✅ Variables globales actualizadas con nueva configuración");
    }
    
    DBG_SRV("✅ Configuración de Telegram guardada correctamente");
    DBG_SRV_PRINTF("   Tamaño: %d bytes", jsonConfig.length());
    
    return true;
}

/**
 * @brief Carga la configuración de Telegram desde SPIFFS
 * 
 * @details Lee el archivo /telegram_config.json de SPIFFS y retorna
 *          su contenido como string JSON. Si no existe, retorna
 *          configuración por defecto para el campanario.
 *          
 *          **CONFIGURACIÓN POR DEFECTO:**
 *          - nombre: Valor de CAMPANARIO_ID (de Configuracion.h)
 *          - ubicacion: "España"
 *          - Todas las notificaciones habilitadas
 * 
 * @retval String JSON con la configuración cargada
 * @retval String JSON con configuración por defecto si no existe archivo
 * 
 * @note Llamada al cargar la página de configuración de Telegram
 * @note Genera configuración por defecto si es primera ejecución
 * @note Utiliza CAMPANARIO_ID definido en Configuracion.h
 * 
 * @warning Requiere SPIFFS montado correctamente
 * @warning Requiere CAMPANARIO_ID definido en Configuracion.h
 * 
 * @see guardarConfigTelegramEnSPIFFS() - Función complementaria para guardar
 * @see procesaMensajeWebSocket() - Procesa comando GET_CONFIG_TELEGRAM
 * 
 * @since v3.1 - Sistema de configuración dinámica de Telegram
 * @author Julian Salas Bartolomé
 */
String cargarConfigTelegramDesdeSPIFFS() {
    DBG_SRV("📂 Cargando configuración de Telegram desde SPIFFS...");
    
    if (!SPIFFS.exists("/telegram_config.json")) {
        DBG_SRV("⚠️ Archivo telegram_config.json no existe, creando configuración por defecto");
        
        // Configuración por defecto CON CAMPOS VACÍOS hasta primera configuración
        String configDefault = "{";
        configDefault += "\"nombre\":\"\",";  // Vacío hasta primera configuración
        configDefault += "\"ubicacion\":\"\",";  // Vacío hasta primera configuración
        configDefault += "\"notificaciones\":{";
        configDefault += "\"inicio\":true,";
        configDefault += "\"misa\":true,";
        configDefault += "\"difuntos\":true,";
        configDefault += "\"fiesta\":true,";
        configDefault += "\"stop\":false,";
        configDefault += "\"calefaccion\":true,";
        configDefault += "\"calefaccion_off\":false,";
        configDefault += "\"alarma\":true,";
        configDefault += "\"errores\":false,";
        configDefault += "\"internet\":false,";
        configDefault += "\"hora\":false,";
        configDefault += "\"mediahora\":false,";
        configDefault += "\"ntp\":false,";
        configDefault += "\"dns\":false,";
        configDefault += "\"alarma_programada\":false";
        configDefault += "}}";
        
        // Guardar configuración por defecto
        guardarConfigTelegramEnSPIFFS(configDefault);
        
        return configDefault;
    }
    
    // Leer archivo existente
    File file = SPIFFS.open("/telegram_config.json", "r");
    if (!file) {
        DBG_SRV("❌ Error al abrir telegram_config.json");
        return "{}";
    }
    
    String contenido = file.readString();
    file.close();
    
    // ✅ PARSEAR Y CARGAR EN VARIABLES GLOBALES DE Configuracion.h
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, contenido);
    
    if (!error) {
        // Cargar nombre y ubicación
        Config::Telegram::CAMPANARIO_NOMBRE = doc["nombre"] | "";
        Config::Telegram::CAMPANARIO_UBICACION = doc["ubicacion"] | "";
        
        // Si nombre está vacío, usar CAMPANARIO_ID como fallback
        if (Config::Telegram::CAMPANARIO_NOMBRE.isEmpty() && !String(Config::Telegram::CAMPANARIO_ID).isEmpty()) {
            Config::Telegram::CAMPANARIO_NOMBRE = String(Config::Telegram::CAMPANARIO_ID);
        }
        
        // Cargar preferencias de notificaciones
        if (doc.containsKey("notificaciones")) {
            JsonObject notif = doc["notificaciones"];
            Config::Telegram::NOTIFICACION_START = notif["inicio"] | true;
            Config::Telegram::NOTIFICACION_MISA = notif["misa"] | true;
            Config::Telegram::NOTIFICACION_DIFUNTOS = notif["difuntos"] | true;
            Config::Telegram::NOTIFICACION_FIESTA = notif["fiesta"] | true;
            Config::Telegram::NOTIFICACION_STOP = notif["stop"] | false;
            Config::Telegram::NOTIFICACION_CALEFACCION_ON = notif["calefaccion"] | true;
            Config::Telegram::NOTIFICACION_CALEFACCION_OFF = notif["calefaccion_off"] | false;
            Config::Telegram::NOTIFICACION_ALARMAS = notif["alarma"] | true;
            Config::Telegram::NOTIFICACION_ERRORES = notif["errores"] | false;
            Config::Telegram::NOTIFICACION_INTERNET_RECONEXION = notif["internet"] | false;
            Config::Telegram::NOTIFICACION_HORA = notif["hora"] | false;
            Config::Telegram::NOTIFICACION_MEDIAHORA = notif["mediahora"] | false;
            Config::Telegram::NOTIFICACION_NTP_SYNC = notif["ntp"] | false;
            Config::Telegram::NOTIFICACION_DNS_UPDATE = notif["dns"] | false;
            Config::Telegram::NOTIFICACION_ALARMA_PROGRAMADA = notif["alarma_programada"] | false;
        }
        
        DBG_SRV("✅ Configuración de Telegram cargada en variables globales:");
        DBG_SRV_PRINTF("   Nombre: %s", Config::Telegram::CAMPANARIO_NOMBRE.c_str());
        DBG_SRV_PRINTF("   Ubicación: %s", Config::Telegram::CAMPANARIO_UBICACION.c_str());
        DBG_SRV_PRINTF("   Notif. Inicio: %s", Config::Telegram::NOTIFICACION_START ? "SÍ" : "NO");
        DBG_SRV_PRINTF("   Notif. Misa: %s", Config::Telegram::NOTIFICACION_MISA ? "SÍ" : "NO");
        DBG_SRV_PRINTF("   Notif. Difuntos: %s", Config::Telegram::NOTIFICACION_DIFUNTOS ? "SÍ" : "NO");
        DBG_SRV_PRINTF("   Notif. Fiesta: %s", Config::Telegram::NOTIFICACION_FIESTA ? "SÍ" : "NO");
        DBG_SRV_PRINTF("   Notif. Stop: %s", Config::Telegram::NOTIFICACION_STOP ? "SÍ" : "NO");
        DBG_SRV_PRINTF("   Notif. Calef.ON: %s", Config::Telegram::NOTIFICACION_CALEFACCION_ON ? "SÍ" : "NO");
        DBG_SRV_PRINTF("   Notif. Calef.OFF: %s", Config::Telegram::NOTIFICACION_CALEFACCION_OFF ? "SÍ" : "NO");
        DBG_SRV_PRINTF("   Notif. Alarmas: %s", Config::Telegram::NOTIFICACION_ALARMAS ? "SÍ" : "NO");
        DBG_SRV_PRINTF("   Notif. Errores: %s", Config::Telegram::NOTIFICACION_ERRORES ? "SÍ" : "NO");
        DBG_SRV_PRINTF("   Notif. Internet Reconexión: %s", Config::Telegram::NOTIFICACION_INTERNET_RECONEXION ? "SÍ" : "NO");
        DBG_SRV_PRINTF("   Notif. Hora: %s", Config::Telegram::NOTIFICACION_HORA ? "SÍ" : "NO");
        DBG_SRV_PRINTF("   Notif. Mediahora: %s", Config::Telegram::NOTIFICACION_MEDIAHORA ? "SÍ" : "NO");
        DBG_SRV_PRINTF("   Notif. NTP Sync: %s", Config::Telegram::NOTIFICACION_NTP_SYNC ? "SÍ" : "NO");
        DBG_SRV_PRINTF("   Notif. DNS Update: %s", Config::Telegram::NOTIFICACION_DNS_UPDATE ? "SÍ" : "NO");
        DBG_SRV_PRINTF("   Notif. Alarma Programada: %s", Config::Telegram::NOTIFICACION_ALARMA_PROGRAMADA ? "SÍ" : "NO");  
    } else {
        DBG_SRV_PRINTF("❌ Error al parsear telegram_config.json: %s", error.c_str());
    }
    
    DBG_SRV_PRINTF("   Tamaño JSON: %d bytes", contenido.length());
    
    return contenido;
}

/**
 * @brief Verifica el PIN de acceso a configuración
 * 
 * @details Valida el PIN introducido por el usuario para acceder a la
 *          configuración avanzada del sistema. El PIN se almacena en
 *          /pin_config.json en SPIFFS. Si no existe, se crea con PIN
 *          por defecto "1234".
 *          
 *          **SEGURIDAD:**
 *          - PIN almacenado en SPIFFS (no hardcoded)
 *          - PIN por defecto: "1234" (debe cambiarse en producción)
 *          - Persistencia entre reinicios
 *          - Modificable desde interfaz web (futuro)
 * 
 * @param pin String con el PIN a verificar (4 dígitos)
 * 
 * @retval true PIN correcto
 * @retval false PIN incorrecto
 * 
 * @note El PIN por defecto es "1234" si no existe archivo
 * @note Se recomienda cambiar el PIN después de primera configuración
 * 
 * @warning En producción, implementar límite de intentos fallidos
 * @warning Considerar hash del PIN en lugar de texto plano
 * 
 * @see procesaMensajeWebSocket() - Procesa comando VERIFY_PIN
 * 
 * @since v3.1 - Sistema de configuración con PIN
 * @author Julian Salas Bartolomé
 */
bool verificarPinAcceso(const String& pin) {
    DBG_SRV_PRINTF("🔐 Verificando PIN: %s", pin.c_str());
    
    String pinCorrecto = "2408"; // PIN por defecto
    
    // Cargar PIN desde SPIFFS si existe
    if (SPIFFS.exists("/pin_config.json")) {
        File file = SPIFFS.open("/pin_config.json", "r");
        if (file) {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, file);
            file.close();
            
            if (!error && doc.containsKey("pin")) {
                pinCorrecto = doc["pin"].as<String>();
                DBG_SRV("📂 PIN cargado desde SPIFFS");
            }
        }
    } else {
        // Crear archivo con PIN por defecto
        DBG_SRV("⚠️ Archivo pin_config.json no existe, creando con PIN por defecto");
        
        File file = SPIFFS.open("/pin_config.json", "w");
        if (file) {
            JsonDocument doc;
            doc["pin"] = "2408";
            doc["info"] = "Cambiar PIN desde interfaz web";
            serializeJson(doc, file);
            file.close();
            DBG_SRV("✅ Archivo pin_config.json creado con PIN por defecto");
        }
    }
    
    return (pin == pinCorrecto);
}


