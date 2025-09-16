/**
 * @file ModoAP.h
 * @brief Implementación del sistema de Access Point para configuración inicial del proyecto Campanarios
 * 
 * @details Este archivo implementa un sistema completo de Access Point (AP) con portal cautivo
 *          para la configuración inicial del dispositivo cuando no hay conexión WiFi disponible:
 *          
 *          **FUNCIONALIDADES IMPLEMENTADAS:**
 *          - Access Point WiFi con credenciales configurables
 *          - Formulario web HTML para configuración completa del sistema
 *          - Validación de entrada de datos (IP, rangos, campos requeridos)
 *          - Persistencia de configuración en EEPROM para supervivencia a reinicios
 *          - Servidor web asíncrono para interfaz de configuración
 *          - Debug condicional con DebugAP para desarrollo
 *          
 *          **FLUJO DE CONFIGURACIÓN:**
 *          1. Dispositivo inicia en modo AP si no hay configuración WiFi
 *          2. Usuario se conecta a red "magaru" con password "24081960"
 *          3. Usuario abre navegador y accede a ip 192.168.4.1
 *          4. Usuario completa formulario (SSID, IP, credenciales, etc.)
 *          5. Datos se validan y guardan en EEPROM
 *          6. Usuario reinicia dispositivo para aplicar configuración
 * 
 * 
 * @note **PERSISTENCIA EEPROM:**
 *       - Configuración se guarda en posición 0 de EEPROM
 *       - Estructura ConfigWiFi completa preservada entre reinicios
 *       - EEPROM se inicializa/finaliza correctamente en cada operación
 * 
 * @warning **CREDENCIALES AP:**
 *          - SSID: "magaru" (hardcoded)
 *          - Password: "24081960" (hardcoded)
 *          - Cambiar en producción por seguridad
 * 
 * @warning **VALIDACIÓN:**
 *          - IP se valida rango 0-255 por octeto
 *          - Strings se truncan automáticamente si exceden buffer
 *          - Todos los campos son obligatorios (required)
 * 
 * @author Julian Salas Bartolomé
 * @date 2025-09-14
 * @version 2.0
 * 
 * @since v1.0 - Implementación básica de AP con formulario
 * @since v2.0 - Portal cautivo, validación mejorada y debug
 * 
 * @see ModoAP.h - Definiciones y estructuras utilizadas
 * @see Configuracion.h - Estructura ConfigWiFi
 * @see ConexionWifi.cpp - Sistema que usa la configuración guardada
 * 
 * @todo Implementar configuración dinámica de credenciales AP
 * @todo Añadir validación de SSID existente (scan de redes)
 * @todo Implementar timeout de AP (volver a modo normal)
 * @todo Añadir interfaz para reset de configuración
 */
#ifndef MODO_AP_H
  #define MODO_AP_H

  #include <WiFi.h>
  #include <DNSServer.h>
  #include <ESPAsyncWebServer.h>
  #include <EEPROM.h>
  #include "Debug.h"

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
    extern ConfigWiFi configWiFi;                                                                     // Estructura para almacenar la configuración WiFi 
    extern DNSServer dnsServer;                                                                       // Servidor DNS para portal cautivo                  
    extern AsyncWebServer server;                                                                     // Servidor web asíncrono          
    extern const char* ap_ssid;                                                                       // SSID del Access Point
    extern const char* ap_password;                                                                   // Contraseña del Access Point
    extern const char* htmlForm;                                                                      // Formulario HTML para configuración

    // Prototipos
    void guardarConfigWiFi();
    void cargarConfigWiFi();
    void iniciarModoAP();
    void handleDNS();


#endif