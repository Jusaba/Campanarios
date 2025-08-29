

#include "ConexionWifi.h"
#include "DNSServicio.h"
#include "Servidor.h"
#include "RTC.h"
#include "ModoAp.h"
#include "Configuracion.h"

template <typename T>
void DBG(T x) {
  if constexpr (Config::Debug::WIFI_DEBUG) {
    Serial.println(x);
  }
}

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
bool ConectarWifi(const ConfigWiFi& ConfiguracionWiFi, unsigned long timeout_ms ) 
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
            DBG("Configuración de IP estática fallida");
        } else {
            DBG("IP estática configurada: " + String(local_IP));
            DBG("Gateway: " + String(gateway));
        }
    } else {
          DBG("IP inválida en configuración ('" + String(ConfiguracionWiFi.ip) + "'). Usando DHCP.");
    }

    DBG("Iniciando conexión Wi-Fi...");
    DBG("Conectando a " + String(ConfiguracionWiFi.ssid));

    WiFi.begin(ConfiguracionWiFi.ssid, ConfiguracionWiFi.password);

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startAttemptTime) < timeout_ms) {
      delay(500);
      if constexpr (Config::Debug::WIFI_DEBUG) {
        Serial.print(".");
      }
    }

  if (WiFi.status() == WL_CONNECTED) {
      if constexpr (Config::Debug::WIFI_DEBUG) {
        Serial.println();
        Serial.println("\nConexión Wi-Fi establecida.");
        Serial.print("Direccion IP: ");
        Serial.println(WiFi.localIP());
      }
      ActualizaDNS(ConfiguracionWiFi.dominio);
      RTC::begin();
      return true;
    } else {
      if constexpr (Config::Debug::WIFI_DEBUG) {
        Serial.println();
        Serial.println("Error al conectar a la red Wi-Fi (timeout).");
      }
      return false;
    }
}