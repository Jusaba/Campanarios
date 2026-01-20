#include "DNSServicio.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "Servidor.h"

    // Variable estática para cachear el dominio
    static String dominioCache = "";

    const char* serverUrl1 = "http://serverpic1.jusaba.es:8001"; 
    const char* serverUrl2 = "http://serverpic2.jusaba.es:8001";
    const char* serverUrl3 = "http://serverpic.cloud:8001"; 
    const char* serverUrl4 = "http://serverpic.com:8001";
    const char* userPassword = "JUSABA2408";      

    HTTPClient http1;
    HTTPClient http2;
    HTTPClient http3;
    HTTPClient http4;


    
  /**
   * @brief Actualiza DNS en los 4 servidores para el dominio especificado
   * 
   * @details Función principal que realiza actualización inmediata de DNS
   *          en los cuatro servidores configurados (2 viejos + 2 nuevos durante migración).
   *          Envía la IP pública actual del dispositivo a todos los proveedores DNS mediante
   *          peticiones HTTP autenticadas independientes con tolerancia a fallos.
   *          
   *          **PROCESO DE ACTUALIZACIÓN:**
   *          1. Obtiene IP pública actual del dispositivo (WiFi.localIP())
   *          2. Construye URLs de actualización para los 4 servidores
   *          3. Configura autenticación HTTP Basic con credenciales
   *          4. Envía petición HTTP GET a cada servidor (serverUrl1-4)
   *          5. Procesa respuesta de cada servidor independientemente
   *          6. Si un servidor falla, continúa con los siguientes
   *          7. Muestra resultado por debug para cada servidor
   *          8. Actualiza cache tras completar todos los intentos
   * 
   * @param cDominio Nombre del subdominio a actualizar (ej: "campanario" para campanario.jusaba.es)
   * 
   * @note **MIGRACIÓN:** Durante transición, actualiza servidores viejos (1-2) y nuevos (3-4)
   * @note **TOLERANCIA A FALLOS:** Continúa actualizando aunque fallen servidores individuales
   * @note **AUTENTICACIÓN:** Utiliza username y userPassword globales
   * @note **TIMEOUT:** Utiliza timeout por defecto de HTTPClient (5000ms típicamente)
   * @note **DEBUG:** Logging detallado si DEBUGDNS está habilitado
   * @note **CACHE:** Actualiza lastUpdatedIP tras completar todos los servidores
   * 
   * @warning **FUNCIÓN BLOQUEANTE:** Puede tardar varios segundos en completar (4 servidores)
   * @warning **RED REQUERIDA:** Falla si no hay conexión WiFi activa
   * @warning **CREDENCIALES:** Utiliza variables globales username/userPassword
   * 
   * @see serverUrl1 - URL del servidor DNS primario viejo
   * @see serverUrl2 - URL del servidor DNS secundario viejo
   * @see serverUrl3 - URL del servidor DNS primario nuevo
   * @see serverUrl4 - URL del servidor DNS secundario nuevo
   * @see username - Usuario para autenticación HTTP Basic
   * @see userPassword - Contraseña para autenticación HTTP Basic
   * @see ActualizaDNSSiNecesario() - Versión con cache inteligente
   * 
   * @example
   * @code
   * // Actualizar DNS inmediatamente para dominio específico en los 4 servidores
   * ActualizaDNS("campanario");   => campanario.jusaba.es
   * @endcode
   * 
   * @since v1.0 (actualizado v2.0 para soportar 4 servidores)
   * @author Julian Salas Bartolomé
   */
    void ActualizaDNS (const char* cDominio)
    {
        // Actualizar cache del dominio
        if (dominioCache != String(cDominio)) {
            dominioCache = String(cDominio);                        // Actualiza el dominio cacheado
        }
        
        // Servidor DNS 1 (viejo)
        #ifdef DEBUGDNS
          DBG_DNS_PRINT("Actualizando servidor DNS 1...");
        #endif
        http1.begin(serverUrl1);                                    // Iniciar conexión al servidor DNS 1
        http1.setAuthorization(cDominio, userPassword);             // Configurar autenticación 
        int httRespuesta1 = http1.GET();                            // Enviar petición GET
        #ifdef DEBUGDNS
          if (httRespuesta1 > 0) {                                  // Verificar éxito de la respuesta
            DBG_DNS_PRINT("[DNS1] HTTP Codigo: ");
            DBG_DNS(httRespuesta1);
            String cRespuesta = http1.getString();                  // Obtener texto de respuesta
            DBG_DNS_PRINT("[DNS1] Respuesta: ");
            DBG_DNS(cRespuesta);
          } else {
            DBG_DNS_PRINT("[DNS1] Error HTTP: ");
            DBG_DNS(httRespuesta1);
          }
        #endif
        http1.end();                                                // Finalizar conexión
        
        // Servidor DNS 2 (viejo)
        #ifdef DEBUGDNS
          DBG_DNS_PRINT("Actualizando servidor DNS 2...");
        #endif
        http2.begin(serverUrl2);                                    // Iniciar conexión al servidor DNS 2
        http2.setAuthorization(cDominio, userPassword);             // Configurar autenticación  
        int httRespuesta2 = http2.GET();
        #ifdef DEBUGDNS
          if (httRespuesta2 > 0) {
            DBG_DNS_PRINT("[DNS2] HTTP Codigo: ");
            DBG_DNS(httRespuesta2);
            String cRespuesta = http2.getString();                  // Obtener texto de respuesta
            DBG_DNS_PRINT("[DNS2] Respuesta: ");
            DBG_DNS(cRespuesta);
          } else {
            DBG_DNS_PRINT("[DNS2] Error HTTP: ");
            DBG_DNS(httRespuesta2);
          }
        #endif
        http2.end();                                                // Finalizar conexión
        
        // Servidor DNS 3 (nuevo)
        #ifdef DEBUGDNS
          DBG_DNS_PRINT("Actualizando servidor DNS 3...");
        #endif
        http3.begin(serverUrl3);                                    // Iniciar conexión al servidor DNS 3
        http3.setAuthorization(cDominio, userPassword);             // Configurar autenticación  
        int httRespuesta3 = http3.GET();
        #ifdef DEBUGDNS
          if (httRespuesta3 > 0) {
            DBG_DNS_PRINT("[DNS3] HTTP Codigo: ");
            DBG_DNS(httRespuesta3);
            String cRespuesta = http3.getString();                  // Obtener texto de respuesta
            DBG_DNS_PRINT("[DNS3] Respuesta: ");
            DBG_DNS(cRespuesta);
          } else {
            DBG_DNS_PRINT("[DNS3] Error HTTP: ");
            DBG_DNS(httRespuesta3);
          }
        #endif
        http3.end();                                                // Finalizar conexión
        
        // Servidor DNS 4 (nuevo)
        #ifdef DEBUGDNS
          DBG_DNS_PRINT("Actualizando servidor DNS 4...");
        #endif
        http4.begin(serverUrl4);                                    // Iniciar conexión al servidor DNS 4
        http4.setAuthorization(cDominio, userPassword);             // Configurar autenticación  
        int httRespuesta4 = http4.GET();
        #ifdef DEBUGDNS
          if (httRespuesta4 > 0) {
            DBG_DNS_PRINT("[DNS4] HTTP Codigo: ");
            DBG_DNS(httRespuesta4);
            String cRespuesta = http4.getString();                  // Obtener texto de respuesta
            DBG_DNS_PRINT("[DNS4] Respuesta: ");
            DBG_DNS(cRespuesta);
          } else {
            DBG_DNS_PRINT("[DNS4] Error HTTP: ");
            DBG_DNS(httRespuesta4);
          }
        #endif
        http4.end();                                                // Finalizar conexión
        
        #ifdef DEBUGDNS
          DBG_DNS_PRINT("Actualización DNS completada en 4 servidores");
        #endif
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
                  
                  // Notificar actualización DNS si está habilitada
                  extern TelegramServicio telegramBot;
                  if (telegramBot.isEnabled() && Config::Telegram::NOTIFICACION_DNS_UPDATE) {
                      telegramBot.sendDnsUpdateNotification(dominioCache + " (" + currentIP + ")");
                  }
              }
        }