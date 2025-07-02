/**
 * @file ModoAP.h
 * @brief Gestión del modo Punto de Acceso (AP) y portal cautivo para configuración WiFi en ESP32.
 *
 * Este archivo contiene las definiciones, estructuras y funciones necesarias para iniciar el dispositivo
 * en modo Punto de Acceso (AP), mostrar un portal cautivo de configuración WiFi y guardar los datos
 * introducidos por el usuario en la EEPROM. Incluye la gestión del servidor DNS para redirigir todas
 * las peticiones al portal de configuración, así como el manejo de la configuración WiFi persistente.
 *
 * Funcionalidades principales:
 * - Definición de la estructura ConfigWiFi para almacenar los parámetros de red y acceso.
 * - Inicio del modo AP con SSID y contraseña definidos.
 * - Portal cautivo DNS que redirige cualquier dominio a la página de configuración.
 * - Servidor web asíncrono para mostrar el formulario y guardar la configuración.
 * - Funciones para guardar y cargar la configuración WiFi en la EEPROM.
 * - Función para procesar solicitudes DNS pendientes.
 *
 * Uso típico:
 * 1. Llamar a iniciarModoAP() para activar el modo AP y el portal cautivo.
 * 2. El usuario se conecta al AP, accede al portal y guarda la configuración.
 * 3. Los datos se almacenan en EEPROM para su uso posterior.
 *
 * @author  Julian Salas Baertolome
 * @version 1.0 
 * @date    22/06/2025
 * @note Este fichero necesita de las librerías WiFi, DNSServer, ESPAsyncWebServer y EEPROM para su funcionamiento.
 * @note Es necesario declarar server como externo
 */
#ifndef MODO_AP_H
  #define MODO_AP_H

  #include <WiFi.h>
  #include <DNSServer.h>
  #include <ESPAsyncWebServer.h>
  #include <EEPROM.h>

  #define DebugAP

  #define EEPROM_SIZE 256                                   // Tamaño de la EEPROM para almacenar la configuración WiFi

    struct ConfigWiFi {
      char ssid[32];
      char password[32];
      char ip[16];
      char dominio[32];
      char usuario[32];
      char clave[32];
    };

    ConfigWiFi configWiFi;

  

const char* ap_ssid = "magaru";
const char* ap_password = "24081960";

  DNSServer dnsServer;
  extern AsyncWebServer server;

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
    #ifdef DebugAP
      Serial.println("Guardando configuración WiFi...");
      Serial.print("SSID: ");
      Serial.println(configWiFi.ssid);
      Serial.print("IP: ");
      Serial.println(configWiFi.ip);
      Serial.print("Dominio: ");
      Serial.println(configWiFi.dominio);
      Serial.print("Usuario: ");
      Serial.println(configWiFi.usuario);
      Serial.print("Clave: ");
      Serial.println(configWiFi.clave);
    #endif    
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
    #ifdef DebugAP
      Serial.println("Cargando configuración WiFi...");
      Serial.print("SSID: ");
      Serial.println(configWiFi.ssid);
      Serial.print("IP: ");
      Serial.println(configWiFi.ip);
      Serial.print("Dominio: ");
      Serial.println(configWiFi.dominio);
      Serial.print("Usuario: ");
      Serial.println(configWiFi.usuario);
      Serial.print("Clave: ");
      Serial.println(configWiFi.clave);
    #endif    
}

const char* htmlForm = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Configuración WiFi</title>
</head>
<body>
  <h2>Configuración WiFi</h2>
  <form action="/guardar" method="POST">
    <label>Nombre WiFi (SSID):</label><br>
    <input type="text" name="ssid" maxlength="31" required><br>
    <label>Password WiFi:</label><br>
    <input type="password" name="password" maxlength="31" required><br>
    <label>IP deseada:</label><br>
    <input type="text" name="ip" maxlength="15" required><br>
    <label>Dominio:</label><br>
    <input type="text" name="dominio" maxlength="31" required><br><br>
    <label>Usuario:</label><br>
    <input type="text" name="usuario" maxlength="31" required><br><br>
    <label>Clave:</label><br>
    <input type="text" name="clave" maxlength="31" required><br><br>
    
    <input type="submit" value="Guardar">
  </form>
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
        if (request->hasParam("ssid", true) && request->hasParam("password", true) &&
            request->hasParam("ip", true) && request->hasParam("dominio", true)) {
            strncpy(configWiFi.ssid, request->getParam("ssid", true)->value().c_str(), sizeof(configWiFi.ssid));
            strncpy(configWiFi.password, request->getParam("password", true)->value().c_str(), sizeof(configWiFi.password));
            strncpy(configWiFi.ip, request->getParam("ip", true)->value().c_str(), sizeof(configWiFi.ip));
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

#endif