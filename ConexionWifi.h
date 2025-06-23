/**
 * @file ConexionWifi.h
 * @brief Funciones y definiciones para la conexión Wi-Fi con IP estática en ESP32.
 *
 * Este archivo contiene la declaración y definición de la función para conectar el ESP32
 * a una red Wi-Fi utilizando una configuración de IP estática. Incluye la actualización
 * del DNS y la sincronización del reloj RTC mediante NTP tras la conexión exitosa.
 *
 * Dependencias:
 * - DNSServicio.h: Para la actualización del DNS.
 * - Servidor.h: Para la gestión del servidor local (si aplica).
 * - ModoAp.h: Para el modo punto de acceso (si aplica).
 * - RTC.h: Para la sincronización del reloj en tiempo real.
 * - Librería WiFi estándar de ESP32.
 *
 *
 * Macros:
 * - DEBUGWIFI: Habilita mensajes de depuración por Serial.
 *
 * Funciones:
 * - void ConectarWifi (const ConfigWiFi& ConfiguracionWiFi): Establece la conexión Wi-Fi, configura IP estática con los datos pasados en la estructura ConfigWiFi,
 *   actualiza DNS y sincroniza el RTC.
 *
 * @author Julian Salas Bartolome
 * @date 2025-05-27
 * @version 1.0
 */

#include "DNSServicio.h"
#include "Servidor.h"
#include "RTC.h"
#include "ModoAp.h"

#define DEBUGWIFI


void ConectarWifi (const ConfigWiFi& ConfiguracionWiFi);


/**
 * @brief Establece la conexión Wi-Fi utilizando la configuración proporcionada.
 *
 * Esta función configura una IP estática basada en el último octeto de la dirección IP
 * especificada en la estructura ConfigWiFi. Luego, intenta conectar el dispositivo a la red Wi-Fi
 * utilizando el SSID y la contraseña proporcionados. Si la conexión es exitosa, actualiza el DNS
 * dinámico y sincroniza la hora mediante NTP.
 *
 * @param ConfiguracionWiFi Estructura que contiene los parámetros de configuración Wi-Fi:
 *        - ip: Cadena de texto con el último octeto de la IP local.
 *        - ssid: Nombre de la red Wi-Fi.
 *        - password: Contraseña de la red Wi-Fi.
 *        - dominio: Dominio para la actualización de DNS dinámico.
 *
 * @note Utiliza macros de depuración (DEBUGWIFI) para imprimir mensajes por Serial.
 * @note Llama a funciones externas para la actualización de DNS y sincronización de hora.
 */
void ConectarWifi (const ConfigWiFi& ConfiguracionWiFi)
{
    uint8_t ultimoOcteto = atoi(ConfiguracionWiFi.ip);                        // Convierte el último octeto de la IP a entero
    IPAddress local_IP(192, 168, 1, ultimoOcteto);
    IPAddress gateway(192, 168, 1, 1);                                        // Puerta de enlace (router)
    IPAddress subnet(255, 255, 255, 0);                                       // Máscara de subred
    IPAddress primaryDNS(8, 8, 8, 8);                                         // DNS primario (Google DNS)
    IPAddress secondaryDNS(8, 8, 4, 4);                                       // DNS secundario (Google DNS)
    // Configurar IP estática
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      #ifdef DEBUGWIFI
        Serial.println("Configuración de IP estática fallida");
      #endif
    }
    // Conexión a la red Wi-Fi
    #ifdef DEBUGWIFI
      Serial.println("Iniciando conexión Wi-Fi...");
      Serial.print("Conectando a ");
      Serial.println(ConfiguracionWiFi.ssid);
    #endif
    WiFi.begin(ConfiguracionWiFi.ssid, ConfiguracionWiFi.password);     // Inicia la conexión Wi-Fi 
    while (WiFi.status() != WL_CONNECTED) {                             // Espera hasta que se conecte
      delay(500);
      #ifdef DEBUGWIFI
        Serial.print(".");                                              // Imprime un punto cada medio segundo para indicar que está intentando conectar   
      #endif
    }
    #ifdef DEBUGWIFI
      Serial.println();
      Serial.println("\nConexión Wi-Fi establecida.");
      Serial.print("Direccion IP: ");
      Serial.println(WiFi.localIP());
    #endif

  
  if (WiFi.status() == WL_CONNECTED) {

    ActualizaDNS(configWiFi.dominio);                 // Llama a la función para actualizar el DNS en minidindns
    RTC::begin();                                     // Sincroniza hora con NTP
  } else {
    #ifdef DEBUGWIFI
      Serial.println("Error al conectar a la red Wi-Fi.");
    #endif
  }    
}