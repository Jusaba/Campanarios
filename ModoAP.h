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


    // Variables globales
    extern ConfigWiFi configWiFi;
    extern DNSServer dnsServer;
    extern AsyncWebServer server;
    extern const char* ap_ssid;
    extern const char* ap_password;
    extern const char* htmlForm;

    // Prototipos
    void guardarConfigWiFi();
    void cargarConfigWiFi();
    void iniciarModoAP();
    void handleDNS();


#endif