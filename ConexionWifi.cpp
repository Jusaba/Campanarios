

#include "ConexionWifi.h"
#include "DNSServicio.h"
#include "Servidor.h"
#include "RTC.h"
#include "ModoAp.h"


/**
 * @brief Establece conexión Wi-Fi con configuración IP estática/DHCP y timeout
 * 
 * @details Función principal para conectar el dispositivo a una red Wi-Fi específica.
 *          Implementa un sistema robusto con las siguientes características:
 *          
 *          **PROCESO DE CONEXIÓN:**
 *          1. **Parsing de IP:** Analiza y valida la IP estática en formato "a.b.c.d"
 *          2. **Configuración de Red:** Establece IP estática o fallback a DHCP
 *          3. **Conexión Wi-Fi:** Inicia conexión con credenciales proporcionadas
 *          4. **Timeout Control:** Espera conexión hasta timeout_ms milisegundos
 *          5. **Servicios Post-Conexión:** Activa DNS personalizado y sincroniza RTC
 *          
 *          **CONFIGURACIÓN IP ESTÁTICA:**
 *          - Validación automática de formato y rangos (0-255 por octeto)
 *          - Gateway inferido como "a.b.c.1" (convención de redes domésticas)
 *          - Subnet mask /24 (255.255.255.0) estándar
 *          - DNS Google (8.8.8.8 primario, 8.8.4.4 secundario)
 *          - Fallback automático a DHCP si IP inválida
 *          
 *          **GESTIÓN DE ERRORES:**
 *          - Timeout configurable evita bloqueos indefinidos
 *          - Validación de IP antes de aplicar configuración
 *          - Logging detallado del proceso (condicional con DEBUGWIFI)
 *          - Retorno seguro sin crash ante fallos
 * 
 * @param ConfiguracionWiFi Estructura de configuración Wi-Fi que debe contener:
 *                          - ssid: Nombre de la red Wi-Fi (string)
 *                          - password: Contraseña de la red (string)
 *                          - ip: Dirección IP estática en formato "a.b.c.d" (string)
 *                          - dominio: Dominio personalizado para DNS (string)
 * @param timeout_ms Tiempo máximo de espera para establecer conexión en milisegundos
 *                   - Valor por defecto: 10,000ms (10 segundos)
 *                   - Rango recomendado: 5,000-30,000ms
 *                   - 0 = espera indefinida (NO recomendado)
 * 
 * @retval true Conexión Wi-Fi establecida exitosamente y servicios activados
 * @retval false Error en conexión, timeout superado o configuración inválida
 * 
 * @note **SERVICIOS ACTIVADOS TRAS CONEXIÓN:**
 *       - ActualizaDNS(dominio): Configura DNS personalizado del proyecto
 *       - RTC::beginConMultiplesServidores(): Sincroniza reloj con servidores NTP
 * 
 * 
 * @warning **IP ESTÁTICA:**
 *          - Formato obligatorio: "a.b.c.d" (ej: "192.168.1.100")
 *          - Rangos válidos: cada octeto 0-255
 *          - Gateway se infiere como "a.b.c.1" automáticamente
 * 
 * @warning **CONFIGURACIÓN:**
 *          - ConfiguracionWiFi.ssid no debe estar vacío
 *          - ConfiguracionWiFi.password puede estar vacío (redes abiertas)
 *          - timeout_ms > 0 para evitar esperas indefinidas
 * 
 * @see ConfigWiFi - Estructura de configuración utilizada
 * @see ActualizaDNS() - Función de DNS llamada tras conexión
 * @see RTC::beginConMultiplesServidores() - Sincronización temporal
 * @see WiFi.config() - Configuración IP estática de ESP32
 * @see WiFi.begin() - Inicio de conexión Wi-Fi
 * 
 * @example
 * @code
 * ConfigWiFi config = {"MiRed", "MiPassword", "192.168.1.100", "midominio.local"};
 * if (ConectarWifi(config, 15000)) {
 *     Serial.println("Wi-Fi conectado exitosamente");
 *     // Sistema listo para usar servicios de red
 * } else {
 *     Serial.println("Error de conexión Wi-Fi");
 *     // Activar modo AP o reintentar
 * }
 * @endcode
 * 
 * @since v1.0 - Implementación básica con IP estática
 * @since v2.0 - Añadido parsing automático y validación
 * @since v2.1 - Timeout configurable y mejor manejo de errores
 * 
 * @author Julian Salas Bartolomé
 * @date 2025-09-14
 */
bool ConectarWifi(const ConfigWiFi& ConfiguracionWiFi, unsigned long timeout_ms ) 
{
    DBG_WIFI("=== INICIANDO CONEXIÓN WI-FI ===");
    DBG_WIFI_PRINTF("SSID: %s", ConfiguracionWiFi.ssid);
    DBG_WIFI_PRINTF("IP configurada: %s", ConfiguracionWiFi.ip);
    DBG_WIFI_PRINTF("Dominio: %s", ConfiguracionWiFi.dominio);
    DBG_WIFI_PRINTF("Timeout: %lu ms", timeout_ms);
 
    int a=-1,b=-1,c=-1,d=-1;                                                                            // Variables para parsing de IP                   
    bool ipValida = (sscanf(ConfiguracionWiFi.ip, "%d.%d.%d.%d", &a,&b,&c,&d) == 4) &&
                    (a>=0 && a<=255 && b>=0 && b<=255 && c>=0 && c<=255 && d>=0 && d<=255);

    IPAddress local_IP;                                                                                 // Se rellenará si ipValida
    IPAddress gateway;                                                                                  // Derivamos gateway como a.b.c.1 (heurística) si no se dispone de otro dato
    IPAddress subnet(255, 255, 255, 0);                                                                 // Mantener /24 por defecto
    IPAddress primaryDNS(8, 8, 8, 8);                                                                   // DNS Google primario                     
    IPAddress secondaryDNS(8, 8, 4, 4);                                                                 // DNS Google secundario 

    if(ipValida) {                                                                                      //  Si la IP es válida, configuramos IP estática
        local_IP = IPAddress((uint8_t)a,(uint8_t)b,(uint8_t)c,(uint8_t)d);                              // Convertir a IPAddress   
        gateway  = IPAddress((uint8_t)a,(uint8_t)b,(uint8_t)c,1); // suposición habitual
        if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {                        // Configurar IP estática , si no se puede configurar
            DBG_WIFI("ERROR: Configuración de IP estática fallida");
            return false;                                                                               // Retornar error                  
        } else {
            DBG_WIFI("✅ IP estática configurada: " + local_IP.toString());
            DBG_WIFI("✅ Gateway inferido: " + gateway.toString());
            DBG_WIFI("✅ Subnet: " + subnet.toString());
        }
    } else {                                                                                            // Si la IP no es válida
           DBG_WIFI("⚠️  IP inválida en configuración ('" + String(ConfiguracionWiFi.ip) + "'). Usando DHCP.");
    }

    DBG_WIFI("🔄 Iniciando conexión Wi-Fi...");
    DBG_WIFI("🔗 Conectando a: " + String(ConfiguracionWiFi.ssid));

    WiFi.begin(ConfiguracionWiFi.ssid, ConfiguracionWiFi.password);                                   // Iniciar conexión Wi-Fi

    unsigned long startAttemptTime = millis();                                                        // Tiempo de inicio para timeout          
    int dots = 0;                                                                                     // Contador de puntos para log          
    while (WiFi.status() != WL_CONNECTED && (millis() - startAttemptTime) < timeout_ms) {             // Esperar hasta conexión o timeout
      delay(500);
      DBG_WIFI_PRINT(".");
      dots++;
      if (dots >= 16) { // Evitar líneas demasiado largas en el log
          DBG_WIFI_PRINT("\n");
          dots = 0;
      }
    }

  if (WiFi.status() == WL_CONNECTED) {                                                              // Si la conexión fue exitosa
      DBG_WIFI();
      DBG_WIFI("🎉 ¡Conexión Wi-Fi establecida exitosamente!");
      DBG_WIFI_PRINT("📡 Dirección IP asignada: ");
      DBG_WIFI(WiFi.localIP());
      DBG_WIFI_PRINT("🌐 Gateway: ");
      DBG_WIFI(WiFi.gatewayIP());
      DBG_WIFI_PRINT("📶 Intensidad señal (RSSI): ");
      DBG_WIFI(WiFi.RSSI());
      DBG_WIFI_PRINT(" dBm");

      DBG_WIFI("🔧 Configurando servicios DNS...");
      ActualizaDNS(ConfiguracionWiFi.dominio);                                                      // Configurar DNS personalizado                
      //RTC::begin();
      RTC::beginConMultiplesServidores();                                                           // Sincronizar reloj con NTP
      return true;                                                                                  // Retornar éxito     
    } else {                                                                                        // Si la conexión falló (timeout)      
      DBG_WIFI("Error al conectar a la red Wi-Fi (timeout).");
      return false;                                                                                 // Retornar error          
    }
}