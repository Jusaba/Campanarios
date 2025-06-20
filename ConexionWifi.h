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
 * - RTC.h: Para la sincronización del reloj en tiempo real.
 * - Librería WiFi estándar de ESP32.
 *
 * Definiciones:
 * - ssid: Nombre de la red Wi-Fi.
 * - password: Contraseña de la red Wi-Fi.
 * - local_IP: Dirección IP estática asignada al dispositivo.
 *
 * Macros:
 * - DEBUGWIFI: Habilita mensajes de depuración por Serial.
 *
 * Funciones:
 * - void ConectarWifi(void): Establece la conexión Wi-Fi, configura IP estática,
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

// Datos SSID y contraseña de la red Wi-Fi
const char* ssid = "D_Wifi_jsb_rma";
const char* password = "9732309093112";
//const char* ssid = "T_Wifi_jsb_rma";
//const char* password = "9776424223112";

// Configuración de IP estática
IPAddress local_IP(192, 168, 1, 173); 



/**
 * @brief Establece la conexión WiFi con configuración de IP estática
 * 
 * Esta función realiza las siguientes tareas:
 * - Configura una IP estática con los parámetros de red definidos
 * - Establece la conexión con la red WiFi usando las credenciales configuradas
 * - Espera hasta que la conexión se establezca exitosamente
 * - Si la conexión es exitosa:
 *   - Actualiza el DNS en miniDNS
 *   - Sincroniza el RTC con NTP
 * 
 * La función incluye mensajes de depuración que se muestran cuando DEBUGWIFI está definido
 * 
 * @note Requiere que las variables ssid, password y local_IP estén definidas previamente
 * @note Utiliza las librerías WiFi y RTC
 * 
 * @return void
 */
void ConectarWifi (const ConfigWiFi& ConfiguracionWiFi)
{
    uint8_t ultimoOcteto = atoi(ConfiguracionWiFi.ip); // <-- Conversión correcta
    IPAddress local_IP(192, 168, 1, ultimoOcteto);
    IPAddress gateway(192, 168, 1, 1);          // Puerta de enlace (router)
    IPAddress subnet(255, 255, 255, 0);         // Máscara de subred
    IPAddress primaryDNS(8, 8, 8, 8);           // DNS primario (Google DNS)
    IPAddress secondaryDNS(8, 8, 4, 4);         // DNS secundario (Google DNS)
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
      Serial.println(ssid);
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