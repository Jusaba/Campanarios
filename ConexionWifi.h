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


bool ConectarWifi(const ConfigWiFi& ConfiguracionWiFi, unsigned long timeout_ms);


/**
 * @brief Establece la conexión Wi-Fi utilizando una configuración específica y un tiempo de espera.
 * 
 * Configura la IP estática del dispositivo basada en el último octeto de la dirección IP proporcionada en la configuración.
 * Intenta conectar a la red Wi-Fi especificada por el SSID y la contraseña. Si la conexión es exitosa dentro del tiempo de espera,
 * actualiza la configuración DNS y sincroniza el reloj RTC.
 * 
 * @param ConfiguracionWiFi Estructura que contiene los parámetros de configuración Wi-Fi (SSID, contraseña, IP, dominio, etc.).
 * @param timeout_ms Tiempo máximo de espera para la conexión en milisegundos (por defecto 10,000 ms).
 * @return true Si la conexión Wi-Fi se establece correctamente.
 * @return false Si ocurre un error o se supera el tiempo de espera sin conectar.
 */
bool ConectarWifi(const ConfigWiFi& ConfiguracionWiFi, unsigned long timeout_ms = 10000) // timeout por defecto: 10 segundos
{
    // Parseo de IP completa a.b.c.d desde ConfiguracionWiFi.ip
    int a=-1,b=-1,c=-1,d=-1;
    bool ipValida = (sscanf(ConfiguracionWiFi.ip, "%d.%d.%d.%d", &a,&b,&c,&d) == 4) &&
                    (a>=0 && a<=255 && b>=0 && b<=255 && c>=0 && c<=255 && d>=0 && d<=255);

    IPAddress local_IP;      // Se rellenará si ipValida
    IPAddress gateway;       // Derivamos gateway como a.b.c.1 (heurística) si no se dispone de otro dato
    IPAddress subnet(255, 255, 255, 0); // Mantener /24 por defecto
    IPAddress primaryDNS(8, 8, 8, 8);
    IPAddress secondaryDNS(8, 8, 4, 4);

    if(ipValida) {
        local_IP = IPAddress((uint8_t)a,(uint8_t)b,(uint8_t)c,(uint8_t)d);
        gateway  = IPAddress((uint8_t)a,(uint8_t)b,(uint8_t)c,1); // suposición habitual
        if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
          #ifdef DEBUGWIFI
            Serial.println("Configuración de IP estática fallida");
          #endif
        } else {
          #ifdef DEBUGWIFI
            Serial.print("IP estática configurada: "); Serial.println(local_IP);
            Serial.print("Gateway: "); Serial.println(gateway);
          #endif
        }
    } else {
        #ifdef DEBUGWIFI
          Serial.print("IP inválida en configuración ('"); Serial.print(ConfiguracionWiFi.ip); Serial.println("'). Usando DHCP.");
        #endif
    }

    #ifdef DEBUGWIFI
      Serial.println("Iniciando conexión Wi-Fi...");
      Serial.print("Conectando a ");
      Serial.println(ConfiguracionWiFi.ssid);
    #endif

    WiFi.begin(ConfiguracionWiFi.ssid, ConfiguracionWiFi.password);

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startAttemptTime) < timeout_ms) {
      delay(500);
      #ifdef DEBUGWIFI
        Serial.print(".");
      #endif
    }

  if (WiFi.status() == WL_CONNECTED) {
      #ifdef DEBUGWIFI
        Serial.println();
        Serial.println("\nConexión Wi-Fi establecida.");
        Serial.print("Direccion IP: ");
        Serial.println(WiFi.localIP());
      #endif
      ActualizaDNS(ConfiguracionWiFi.dominio);
      RTC::begin();
      return true;
    } else {
      #ifdef DEBUGWIFI
        Serial.println();
        Serial.println("Error al conectar a la red Wi-Fi (timeout).");
      #endif
      return false;
    }
}