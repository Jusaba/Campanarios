#include "ModoAP.h"
#include "Configuracion.h"

template <typename T>
void DBGln(T x) {
  if constexpr (Config::Debug::AP_DEBUG) {
    Serial.println(x);
  }
}

template <typename T>
void DBG(T x) {
  if constexpr (Config::Debug::AP_DEBUG) {
    Serial.print(x);
  }
}


// Definiciones únicas de variables
ConfigWiFi configWiFi;
DNSServer dnsServer;
const char* ap_ssid = "magaru";
const char* ap_password = "24081960";


/**
 * @brief Guarda la configuración WiFi en la memoria EEPROM.
 *
 * Esta función inicia la EEPROM, almacena la estructura configWiFi en la posición 0,
 * realiza el commit para asegurar que los datos se escriban correctamente y finaliza el uso de la EEPROM.
 * Es útil para preservar la configuración WiFi entre reinicios del dispositivo.
 */
void guardarConfigWiFi() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(0, configWiFi);
    EEPROM.commit();
    EEPROM.end();
    DBGln("Guardando configuración WiFi...");
    DBG("SSID: ");
    DBGln(configWiFi.ssid);
    DBG("IP: ");
    DBGln(configWiFi.ip);
    DBG("Dominio: ");
    DBGln(configWiFi.dominio);
    DBG("Usuario: ");
    DBGln(configWiFi.usuario);
    DBG("Clave: ");
    DBGln(configWiFi.clave);
}

/**
 * @brief Carga la configuración WiFi almacenada en la EEPROM.
 *
 * Esta función inicializa la EEPROM, recupera la configuración WiFi guardada 
 * en la posición 0 y la almacena en la variable global configWiFi. 
 * Finalmente, finaliza el acceso a la EEPROM.
 */
void cargarConfigWiFi() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(0, configWiFi);
    EEPROM.end();
    DBGln("Cargando configuración WiFi...");
    DBG("SSID: ");
    DBGln(configWiFi.ssid);
    DBG("IP: ");
    DBGln(configWiFi.ip);
    DBG("Dominio: ");
    DBGln(configWiFi.dominio);
    DBG("Usuario: ");
    DBGln(configWiFi.usuario);
    DBG("Clave: ");
    DBGln(configWiFi.clave);
}

// Formulario estático. La IP se introduce en 4 campos separados (ip1..ip4) para evitar asumir subred fija.
// Nota: No se pre-rellena con la IP guardada porque mantenemos html estático (sin generación dinámica).
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
 * @brief Inicia el modo Punto de Acceso (AP) en el dispositivo.
 *
 * Configura el dispositivo para funcionar como un punto de acceso WiFi, permitiendo que otros dispositivos se conecten a él.
 * - Establece el modo WiFi en AP.
 * - Configura el SSID y la contraseña del AP.
 * - Inicia un servidor DNS para portal cautivo, redirigiendo todas las peticiones al portal de configuración.
 * - Define rutas HTTP para mostrar el formulario de configuración y guardar los datos enviados por el usuario.
 * - Almacena la configuración WiFi recibida y responde con mensajes de éxito o error.
 * - Inicia el servidor web para atender las solicitudes entrantes.
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
 * @brief Procesa la siguiente solicitud DNS entrante.
 *
 * Esta función llama al método processNextRequest() del objeto dnsServer
 * para manejar las solicitudes DNS pendientes. Es útil en aplicaciones
 * donde se implementa un servidor DNS local, por ejemplo, en modo AP
 * para redireccionar dominios.
 */
void handleDNS() {
    dnsServer.processNextRequest();
}