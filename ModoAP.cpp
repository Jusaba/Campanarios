#include "ModoAP.h"



// Definiciones únicas de variables
ConfigWiFi configWiFi;
DNSServer dnsServer;
const char* ap_ssid = "magaru";
const char* ap_password = "24081960";


/**
 * @brief Persiste la configuración WiFi completa en memoria EEPROM
 *
 * @details Esta función guarda de forma permanente la estructura configWiFi
 *          en la memoria EEPROM para que la configuración sobreviva a reinicios
 *          y cortes de energía del dispositivo.
 *          
 *          **PROCESO DE GUARDADO:**
 *          1. Inicializa EEPROM con tamaño definido (EEPROM_SIZE)
 *          2. Escribe estructura ConfigWiFi completa en posición 0
 *          3. Ejecuta commit() para asegurar escritura física permanente
 *          4. Finaliza uso de EEPROM para liberar recursos
 *          5. Logging detallado de todos los valores guardados (si debug activo)
 * 
 * @note **PERSISTENCIA:** Los datos se mantienen entre reinicios y cortes de energía
 * @note **POSICIÓN EEPROM:** Siempre utiliza posición 0 para la configuración WiFi
 * @note **ATOMIC OPERATION:** EEPROM.commit() asegura escritura completa o fallo total
 * @note **DEBUG:** Logging automático de todos los campos si DBG_AP está habilitado
 * 
 * @warning **EEPROM LIFECYCLE:** Inicializa y finaliza EEPROM en cada llamada
 * @warning **WEAR LEVELING:** Uso frecuente puede degradar memoria EEPROM
 * @warning **DATOS CRÍTICOS:** Fallo en commit() puede corromper configuración
 * 
 * @see cargarConfigWiFi() - Función complementaria para restaurar configuración
 * @see configWiFi - Variable global que se persiste
 * @see EEPROM_SIZE - Tamaño de EEPROM definido en configuración
 * @see ConfigWiFi - Estructura que se guarda completa
 * 
 * @since v1.0
 * @author Julian Salas Bartolomé
 */
  void guardarConfigWiFi() {
      EEPROM.begin(EEPROM_SIZE);
      EEPROM.put(0, configWiFi);
      EEPROM.commit();
      EEPROM.end();
      DBG_AP("Guardando configuración WiFi...");
      DBG_AP_PRINT("SSID: ");
      DBG_AP(configWiFi.ssid);
      DBG_AP_PRINT("IP: ");
      DBG_AP(configWiFi.ip);
      DBG_AP_PRINT("Dominio: ");
      DBG_AP(configWiFi.dominio);
      DBG_AP_PRINT("Usuario: ");
      DBG_AP(configWiFi.usuario);
      DBG_AP_PRINT("Clave: ");
      DBG_AP(configWiFi.clave);
  }

/**
 * @brief Restaura la configuración WiFi desde memoria EEPROM
 *
 * @details Esta función recupera la configuración WiFi previamente guardada
 *          en EEPROM y la carga en la variable global configWiFi para
 *          su uso por todo el sistema.
 *          
 *          **PROCESO DE CARGA:**
 *          1. Inicializa acceso a EEPROM con tamaño definido (EEPROM_SIZE)
 *          2. Lee estructura ConfigWiFi completa desde posición 0
 *          3. Almacena datos recuperados en variable global configWiFi
 *          4. Finaliza acceso a EEPROM para liberar recursos
 *          5. Logging detallado de todos los valores cargados (si debug activo)
 * 
 * @note **INICIALIZACIÓN:** Debe llamarse durante el arranque del sistema
 * @note **POSICIÓN EEPROM:** Siempre lee desde posición 0 de la EEPROM
 * @note **VARIABLE GLOBAL:** Los datos se cargan en configWiFi global
 * @note **DEBUG:** Logging automático de todos los campos si DBG_AP está habilitado
 * 
 * @warning **DATOS CORRUPTOS:** No valida integridad de datos leídos de EEPROM
 * @warning **PRIMERA VEZ:** Si no hay configuración previa, datos pueden ser basura
 * @warning **EEPROM LIFECYCLE:** Inicializa y finaliza EEPROM en cada llamada
 * 
 * @see guardarConfigWiFi() - Función complementaria para persistir configuración
 * @see configWiFi - Variable global donde se cargan los datos
 * @see EEPROM_SIZE - Tamaño de EEPROM definido en configuración  
 * @see ConfigWiFi - Estructura que se lee completa desde EEPROM
 * 
 * @since v1.0
 * @author Julian Salas Bartolomé
 */
  void cargarConfigWiFi() {
      EEPROM.begin(EEPROM_SIZE);
      EEPROM.get(0, configWiFi);
      EEPROM.end();
      DBG_AP("Cargando configuración WiFi...");
      DBG_AP_PRINT("SSID: ");
      DBG_AP(configWiFi.ssid);
      DBG_AP_PRINT("IP: ");
      DBG_AP(configWiFi.ip);
      DBG_AP_PRINT("Dominio: ");
      DBG_AP(configWiFi.dominio);
      DBG_AP_PRINT("Usuario: ");
      DBG_AP(configWiFi.usuario);
      DBG_AP_PRINT("Clave: ");
      DBG_AP(configWiFi.clave);
  }

/**
 * @brief Formulario HTML para configuración WiFi del sistema
 * 
 * @details Formulario HTML estático que se sirve como portal cautivo para
 *          configurar todos los parámetros del sistema. Incluye validación
 *          HTML5 y CSS responsivo para mejor experiencia de usuario.
 *          
 *          **CAMPOS CONFIGURABLES:**
 *          - SSID: Nombre de red WiFi objetivo (maxlength: 31)
 *          - Password: Contraseña de red WiFi (maxlength: 31)
 *          - IP: Dirección IP estática en 4 campos separados (0-255 cada uno)
 *          - Dominio: Dominio personalizado del sistema (maxlength: 31)
 *          - Usuario: Usuario para autenticación del servidor (maxlength: 31)
 *          - Clave: Contraseña para autenticación del servidor (maxlength: 31)
 * 
 * @note **VALIDACIÓN HTML5:**
 *       - Campos obligatorios (required)
 *       - Rangos numéricos (0-255) para octetos IP
 *       - Límites de caracteres (maxlength)
 *       - Tipos de input específicos (password, number, text)
 * 
 * @note **DISEÑO RESPONSIVO:**
 *       - CSS embebido para styling consistente
 *       - Formulario adaptable a diferentes tamaños de pantalla
 *       - Grupos de campos organizados visualmente
 * 
 * @warning **LIMITACIONES:**
 *          - HTML estático - no se pre-rellena con valores guardados
 *          - Requiere reinicio manual tras guardar configuración
 *          - No valida existencia de SSID objetivo
 * 
 * @see server.on("/") - Ruta que sirve este formulario
 * @see server.on("/guardar") - Procesa datos enviados por este formulario
 * 
 * @since v1.0 - Formulario básico
 * @since v2.0 - Validación HTML5 y CSS mejorado
 */
const char* htmlForm = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Configuración WiFi</title>
  <style>
    body{font-family:Arial,Helvetica,sans-serif;margin:14px;}
    input{margin-bottom:6px;padding:4px;}
    .ip-group input{width:60px;display:inline-block;text-align:center;}
  </style>
</head>
<body>
  <h2>Configuración WiFi</h2>
  <form action="/guardar" method="POST">
    <label>Nombre WiFi (SSID):</label><br>
    <input type="text" name="ssid" maxlength="31" required><br>
    <label>Password WiFi:</label><br>
    <input type="password" name="password" maxlength="31" required><br>
    <label>IP deseada (4 octetos):</label><br>
    <div class="ip-group">
      <input type="number" name="ip1" min="0" max="255" placeholder="0-255" required> .
      <input type="number" name="ip2" min="0" max="255" placeholder="0-255" required> .
      <input type="number" name="ip3" min="0" max="255" placeholder="0-255" required> .
      <input type="number" name="ip4" min="0" max="255" placeholder="0-255" required>
    </div>
    <label>Dominio:</label><br>
    <input type="text" name="dominio" maxlength="31" required><br><br>
    <label>Usuario:</label><br>
    <input type="text" name="usuario" maxlength="31" required><br><br>
    <label>Clave:</label><br>
    <input type="text" name="clave" maxlength="31" required><br><br>
    <input type="submit" value="Guardar">
  </form>
  <p style="font-size:12px;color:#555;">Tras guardar, reinicie el dispositivo para aplicar la configuración. La IP se almacenará como texto en configWiFi.ip.</p>
</body>
</html>
)rawliteral";

/**
 * @brief Inicia el sistema completo de Access Point con portal cautivo
 *
 * @details Función principal que configura el dispositivo ESP32 como punto de acceso
 *          WiFi con servidor web integrado y portal cautivo DNS para configuración
 *          inicial del sistema cuando no hay conexión WiFi disponible.
 *          
 *          **CONFIGURACIÓN COMPLETA REALIZADA:**
 *          1. **WiFi AP Mode:** Configura ESP32 como punto de acceso con credenciales
 *          2. **DNS Captive Portal:** Servidor DNS que redirige TODOS los dominios
 *          3. **Web Server Routes:** Define rutas HTTP para formulario y guardado
 *          4. **Form Validation:** Validación server-side de datos recibidos
 *          5. **EEPROM Persistence:** Guardado automático en EEPROM tras validar
 *          6. **Error Handling:** Respuestas apropiadas para datos inválidos
 *          
 *          **RUTAS HTTP CONFIGURADAS:**
 *          - GET / : Sirve formulario HTML de configuración
 *          - POST /guardar : Procesa y valida datos del formulario
 *          - onNotFound(*) : Redirige cualquier URL al formulario (portal cautivo)
 *          
 *          **VALIDACIÓN DE DATOS:**
 *          - Presencia de todos los campos obligatorios
 *          - Rangos IP válidos (0-255 por cada octeto)
 *          - Truncado automático con strncpy() si excede buffer
 *          - Construcción segura de IP con snprintf()
 * 
 * @note **CREDENCIALES AP:**
 *       - SSID: "magaru" (definido en ap_ssid)
 *       - Password: "24081960" (definido en ap_password)
 *       - IP automática: 192.168.4.1 (estándar ESP32 AP)
 * 
 * @note **PORTAL CAUTIVO:**
 *       - DNS Server en puerto 53 redirige "*" (cualquier dominio)
 *       - onNotFound() captura URLs no definidas
 *       - Cualquier petición web lleva al formulario de configuración
 * 
 * @note **PERSISTENCIA:**
 *       - Configuración válida se guarda automáticamente en EEPROM
 *       - Llamada automática a guardarConfigWiFi() tras validar datos
 *       - Usuario debe reiniciar manualmente para aplicar configuración
 * 
 * @warning **CREDENCIALES HARDCODED:**
 *          - SSID y password del AP están fijos en código
 *          - Cambiar ap_ssid y ap_password para mayor seguridad
 * 
 * @warning **VALIDACIÓN LIMITADA:**
 *          - No verifica existencia del SSID objetivo (sin WiFi scan)
 *          - No valida formato de dominio o credenciales
 *          - Acepta cualquier string que quepa en buffer
 * 
 * @warning **SEGURIDAD:**
 *          - Datos transmitidos en HTTP plano (sin cifrado)
 *          - Sin autenticación en formulario (cualquiera puede acceder)
 *          - Sin protección CSRF en POST requests
 * 
 * @see ap_ssid - SSID del Access Point utilizado
 * @see ap_password - Contraseña del Access Point utilizado
 * @see htmlForm - Formulario HTML servido en las rutas
 * @see guardarConfigWiFi() - Llamada automáticamente tras validar datos
 * @see configWiFi - Variable global modificada por el formulario
 * @see server - Servidor AsyncWebServer configurado y iniciado
 * @see dnsServer - Servidor DNS configurado para portal cautivo
 * 
 * @example
 * @code
 * // Uso típico durante arranque si no hay configuración WiFi
 * cargarConfigWiFi();  // Cargar configuración existente
 * if (strlen(configWiFi.ssid) == 0) {  // Si no hay SSID configurado
 *     iniciarModoAP();  // Iniciar modo configuración
 *     while(true) {     // Loop hasta reinicio manual
 *         handleDNS();  // Procesar peticiones DNS
 *         delay(100);
 *     }
 * }
 * @endcode
 * 
 * @since v1.0 - Implementación básica de AP y formulario
 * @since v2.0 - Portal cautivo DNS y validación mejorada
 * 
 * @author Julian Salas Bartolomé
 */

  void iniciarModoAP() {

      WiFi.mode(WIFI_AP);                                                 // Inicia el modo AP
      WiFi.softAP(ap_ssid, ap_password);                                  // Configura el SSID y la contraseña del AP 

      dnsServer.start(53, "*", WiFi.softAPIP());                          // Portal cautivo DNS

      server.onNotFound([](AsyncWebServerRequest *request){               // Redirige todas las peticiones a la página de configuración
        request->send(200, "text/html", htmlForm);
      });

      // Página principal
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){        // Redirige a la página de configuración
        request->send(200, "text/html", htmlForm);
      });

      // Guardar configuración
      server.on("/guardar", HTTP_POST, [](AsyncWebServerRequest *request){
          // Validamos presencia de todos los parámetros necesarios.
          if (request->hasParam("ssid", true) && request->hasParam("password", true) &&
            request->hasParam("ip1", true) && request->hasParam("ip2", true) &&
            request->hasParam("ip3", true) && request->hasParam("ip4", true) &&
            request->hasParam("dominio", true) && request->hasParam("usuario", true) && request->hasParam("clave", true)) {

            int ip1 = request->getParam("ip1", true)->value().toInt();
            int ip2 = request->getParam("ip2", true)->value().toInt();
            int ip3 = request->getParam("ip3", true)->value().toInt();
            int ip4 = request->getParam("ip4", true)->value().toInt();
            bool ipOk = (ip1>=0 && ip1<=255 && ip2>=0 && ip2<=255 && ip3>=0 && ip3<=255 && ip4>=0 && ip4<=255);
            if(!ipOk){
              request->send(400, "text/html", "<h2>IP inválida (cada octeto 0-255).</h2>");
              return;
            }
            // Copiamos strings (se truncarán automáticamente si exceden tamaño del buffer destino).
            strncpy(configWiFi.ssid, request->getParam("ssid", true)->value().c_str(), sizeof(configWiFi.ssid));
            strncpy(configWiFi.password, request->getParam("password", true)->value().c_str(), sizeof(configWiFi.password));
            snprintf(configWiFi.ip, sizeof(configWiFi.ip), "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
            strncpy(configWiFi.dominio, request->getParam("dominio", true)->value().c_str(), sizeof(configWiFi.dominio));
            strncpy(configWiFi.usuario, request->getParam("usuario", true)->value().c_str(), sizeof(configWiFi.usuario));
            strncpy(configWiFi.clave, request->getParam("clave", true)->value().c_str(), sizeof(configWiFi.clave));
            guardarConfigWiFi();
            request->send(200, "text/html", "<h2>Configuración guardada. Reinicie el dispositivo.</h2>");
          } else {
            request->send(400, "text/html", "<h2>Error en los datos enviados.</h2>");
          }
      });
      server.begin();
  }

/**
 * @brief Procesa peticiones DNS pendientes del portal cautivo
 *
 * @details Función de mantenimiento que debe ser llamada regularmente en el
 *          loop principal para procesar las peticiones DNS entrantes del
 *          servidor DNS del portal cautivo.
 *          
 *          **FUNCIONAMIENTO:**
 *          - Procesa una petición DNS pendiente por llamada
 *          - Redirige TODOS los dominios al IP del Access Point
 *          - Implementa comportamiento de portal cautivo estándar
 *          - Debe llamarse frecuentemente para respuesta rápida
 * 
 * @note **LOOP PRINCIPAL:** Debe llamarse en el loop() principal del programa
 * @note **FRECUENCIA:** Llamar cada 50-100ms para respuesta óptima
 * @note **NON-BLOCKING:** Función no bloqueante - retorna inmediatamente
 * @note **PORTAL CAUTIVO:** Esencial para redirigir navegadores al formulario
 * 
 * @warning **LLAMADA REQUERIDA:** Si no se llama, el portal cautivo no funcionará
 * @warning **FRECUENCIA:** Llamadas muy espaciadas causan timeouts DNS
 * @warning **MODO AP:** Solo funciona cuando el dispositivo está en modo AP
 * 
 * @see dnsServer - Servidor DNS que procesa esta función
 * @see iniciarModoAP() - Función que configura el servidor DNS
 * @see dnsServer.start() - Donde se inicia el servidor DNS
 * 
 * @example
 * @code
 * void loop() {
 *     handleDNS();  // Procesar peticiones DNS del portal cautivo
 *     // ... otro código del loop
 *     delay(100);   // Pequeña pausa para no saturar
 * }
 * @endcode
 * 
 * @since v2.0
 * @author Julian Salas Bartolomé
 */
  void handleDNS() {
      dnsServer.processNextRequest();
  }