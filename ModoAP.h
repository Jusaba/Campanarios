#ifndef MODO_AP_H
#define MODO_AP_H

#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>

#define EEPROM_SIZE 256

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
const char* ap_password = "tremp2025";

DNSServer dnsServer;
extern AsyncWebServer server;

void guardarConfigWiFi() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(0, configWiFi);
    EEPROM.commit();
    EEPROM.end();
}

void cargarConfigWiFi() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(0, configWiFi);
    EEPROM.end();
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

void iniciarModoAP() {
    // Inicia el modo AP
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid, ap_password);

    // Portal cautivo DNS
    dnsServer.start(53, "*", WiFi.softAPIP());

    // Redirige todas las peticiones a la página de configuración
    server.onNotFound([](AsyncWebServerRequest *request){
        request->send(200, "text/html", htmlForm);
    });

    // Página principal
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
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

void handleDNS() {
    dnsServer.processNextRequest();
}

#endif