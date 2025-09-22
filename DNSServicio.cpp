#include "DNSServicio.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "Servidor.h"

    // Variable estática para cachear el dominio
    static String dominioCache = "";

    const char* serverUrl1 = "http://serverpic.cloud:8001"; 
    const char* serverUrl2 = "http://serverpic.com:8001";
    const char* userPassword = "JUSABA2408";      

    HTTPClient http1;
    HTTPClient http2;


    
  /**
   * @brief Actualiza DNS en ambos servidores para el dominio especificado
   * 
   * @details Función principal que realiza actualización inmediata de DNS
   *          en los dos servidores configurados. Envía la IP pública actual
   *          del dispositivo a ambos proveedores DNS mediante peticiones HTTP
   *          autenticadas independientes.
   *          
   *          **PROCESO DE ACTUALIZACIÓN:**
   *          1. Obtiene IP pública actual del dispositivo (WiFi.localIP())
   *          2. Construye URLs de actualización para ambos servidores
   *          3. Configura autenticación HTTP Basic con credenciales
   *          4. Envía petición HTTP GET a servidor primario (serverUrl1)
   *          5. Procesa respuesta y muestra resultado por debug
   *          6. Envía petición HTTP GET a servidor secundario (serverUrl2)
   *          7. Procesa respuesta y muestra resultado por debug
   *          8. Actualiza cache con nueva IP procesada
   * 
   * @param cDominio Nombre del subdominio a actualizar (ej: "campanario" para campanario.jusaba.es)
   * 
   * @note **SERVIDORES DUALES:** Actualiza AMBOS servidores en cada llamada
   * @note **AUTENTICACIÓN:** Utiliza username y userPassword globales
   * @note **TIMEOUT:** Utiliza timeout por defecto de HTTPClient (5000ms típicamente)
   * @note **DEBUG:** Logging detallado si DEBUGDNS está habilitado
   * @note **CACHE:** Actualiza lastUpdatedIP tras completar ambos servidores
   * 
   * @warning **FUNCIÓN BLOQUEANTE:** Puede tardar varios segundos en completar
   * @warning **RED REQUERIDA:** Falla si no hay conexión WiFi activa
   * @warning **CREDENCIALES:** Utiliza variables globales username/userPassword
   * 
   * @see serverUrl1 - URL del servidor DNS primario utilizado
   * @see serverUrl2 - URL del servidor DNS secundario utilizado  
   * @see username - Usuario para autenticación HTTP Basic
   * @see userPassword - Contraseña para autenticación HTTP Basic
   * @see ActualizaDNSSiNecesario() - Versión con cache inteligente
   * 
   * @example
   * @code
   * // Actualizar DNS inmediatamente para dominio específico
   * ActualizaDNS("campanario");   => campanario.jusaba.es
   * @endcode
   * 
   * @since v1.0
   * @author Julian Salas Bartolomé
   */
    void ActualizaDNS (const char* cDominio)
    {
        // http al servidor DNS 1
        if (dominioCache != String(cDominio)) {
            dominioCache = String(cDominio);                        // Actualiza el dominio cacheado
        }
        http1.begin(serverUrl1);                                    // Iniciar conexión al servidor DNS 1
        http1.setAuthorization(cDominio, userPassword);             // Configurar autenticación 
        int httRespuesta = http1.GET();                             // Enviar petición GET
        #ifdef DEBUGDNS
          if (httRespuesta > 0) {                                   // Verificar éxito de la respuesta, si hay éxito
            DBG_DNS_PRINT("HTTP Codigo de respuesta: ");
            DBG_DNS(httRespuesta);
            String cRespuesta = http1.getString();                  // Obtener texto de respuesta
            DBG_DNS_PRINT("Texto de respuesta:");
            DBG_DNS(cRespuesta);
          } else {
            DBG_DNS_PRINT("Error on HTTP request: ");
            DBG_DNS(httRespuesta);
          }
        #endif
        http1.end();                                                // Finalizar conexión      
        // http al servidor DNS 2
        http2.begin(serverUrl2);                                    // Iniciar conexión al servidor DNS 2
        http2.setAuthorization(cDominio, userPassword);             // Configurar autenticación  
        httRespuesta = http2.GET();
        #ifdef DEBUGDNS
            if (httRespuesta > 0) {
              DBG_DNS_PRINT("HTTP Codigo de respuesta: ");
              DBG_DNS(httRespuesta);
              String cRespuesta = http2.getString();                // Obtener texto de respuesta
              DBG_DNS_PRINT("Texto de respuesta:");
              DBG_DNS(cRespuesta);
            } else {
              DBG_DNS_PRINT("Error on HTTP request: ");
              DBG_DNS(httRespuesta);
            }
        #endif
        http2.end();                                                // Finalizar conexión
       }

    /**
    * @brief Actualiza DNS solo si la IP pública ha cambiado (con cache inteligente)
    * 
    * @details Función inteligente que verifica si la IP pública actual es diferente
    *          a la última IP actualizada en los servidores DNS. Solo realiza la
    *          actualización si detecta un cambio, evitando spam innecesario a los
    *          servidores DNS y optimizando el ancho de banda.
    *          
    *          **LÓGICA DE CACHE:**
    *          1. Obtiene IP pública actual del dispositivo (WiFi.localIP())
    *          2. Compara con lastUpdatedIP almacenada en cache
    *          3. Si son iguales: No hace nada (evita actualización innecesaria)
    *          4. Si son diferentes: Llama a ActualizaDNS() con dominio configurado
    *          5. Actualiza cache tras actualización exitosa
    * 
    * @note **DOMINIO FIJO:** Utiliza dominio predefinido desde configuración global
    * @note **CACHE PERSISTENTE:** lastUpdatedIP se mantiene entre llamadas
    * @note **OPTIMIZACIÓN:** Evita actualizaciones innecesarias a servidores DNS
    * @note **USO RECOMENDADO:** Llamar periódicamente desde loop principal
    * @note **DEBUG:** Logging de comparación de IPs si DEBUGDNS habilitado
    * 
    * @warning **DOMINIO HARDCODED:** Utiliza dominio fijo desde configuración
    * @warning **CACHE NO PERSISTENTE:** Se resetea en cada reinicio del dispositivo
    * @warning **IP LOCAL:** Compara IP local (no detecta cambios de IP pública NAT)
    * 
    * @see ActualizaDNS() - Función llamada internamente si IP ha cambiado
    * @see lastUpdatedIP - Variable de cache utilizada para comparación
    * @see WiFi.localIP() - Función utilizada para obtener IP actual
    * 
    * @example
    * @code
    * void loop() {
    *     // Verificar y actualizar DNS solo si es necesario
    *     ActualizaDNSSiNecesario();
    *     delay(60000); // Verificar cada minuto
    * }
    * @endcode
    * 
    * @since v1.5
    * @author Julian Salas Bartolomé
    */
       void ActualizaDNSSiNecesario() 
       {
              static String lastIP = "";

              if (dominioCache.isEmpty()) {
                DBG_DNS("DNS: No hay dominio cacheado, saltando actualización");
                return;
              }
              if (!hayInternet()) return;

              String currentIP = WiFi.localIP().toString();

              // Actualizar si cambió la IP
              if (currentIP != lastIP) {
                  ActualizaDNS(dominioCache.c_str());
                  lastIP = currentIP;
                  DBG_DNS("DNS actualizado - IP: " + currentIP);
              }
        }