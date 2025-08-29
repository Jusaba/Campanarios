#include "DNSServicio.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "Servidor.h"
#include "Configuracion.h"

    // Variable estática para cachear el dominio
    static String dominioCache = "";

    const char* serverUrl1 = "http://serverpic.cloud:8001"; 
    const char* serverUrl2 = "http://serverpic.com:8001";
    const char* userPassword = "JUSABA2408";      

    HTTPClient http1;
    HTTPClient http2;


    
    /**
     * @brief Actualiza la configuración DNS enviando solicitudes HTTP a dos servidores DNS.
     *
     * Esta función realiza las siguientes acciones:
     * - Verifica si el dominio proporcionado es diferente al cacheado y lo actualiza si es necesario.
     * - Envía una solicitud HTTP GET al primer servidor DNS con autenticación.
     * - Envía una solicitud HTTP GET al segundo servidor DNS con autenticación.
     * - Si está habilitado DEBUGDNS, imprime en el puerto serie el código de respuesta HTTP y el texto de respuesta recibido de ambos servidores.
     *
     * @param cDominio Cadena de caracteres que representa el dominio a actualizar.
     */
    void ActualizaDNS (const char* cDominio)
    {
        // http al servidor DNS 1
        if (dominioCache != String(cDominio)) {
            dominioCache = String(cDominio);                        // Actualiza el dominio cacheado
        }
        http1.begin(serverUrl1);
        http1.setAuthorization(cDominio, userPassword);
        int httRespuesta = http1.GET();
        if constexpr (Config::Debug::DNS_DEBUG) {
            if (httRespuesta > 0) {
              Serial.print("HTTP Codigo de respuesta: ");
              Serial.println(httRespuesta);
              String cRespuesta = http1.getString();
              Serial.println("Texto de respuesta:");
              Serial.println(cRespuesta);
            } else {
              Serial.print("Error on HTTP request: ");
              Serial.println(httRespuesta);
            }
        }
        http1.end();
        // http al servidor DNS 2
        http2.begin(serverUrl2);
        http2.setAuthorization(cDominio, userPassword);
        httRespuesta = http2.GET();
        if constexpr (Config::Debug::DNS_DEBUG) {
            if (httRespuesta > 0) {
              Serial.print("HTTP Codigo de respuesta: ");
              Serial.println(httRespuesta);
              String cRespuesta = http2.getString();
              Serial.println("Texto de respuesta:");
              Serial.println(cRespuesta);
            } else {
              Serial.print("Error on HTTP request: ");
              Serial.println(httRespuesta);
            }
        }
        http2.end();    
       }

      /**
       * @brief Actualiza el registro DNS si la dirección IP local ha cambiado.
       *
       * Esta función verifica si existe un dominio cacheado y si hay conexión a Internet.
       * Si la dirección IP local ha cambiado desde la última actualización, se llama a
       * ActualizaDNS para actualizar el registro DNS correspondiente al dominio cacheado.
       * Utiliza una variable estática para almacenar la última IP conocida y evitar
       * actualizaciones innecesarias. Incluye mensajes de depuración si está habilitado
       * DEBUGDNS.
       */
       void ActualizaDNSSiNecesario() 
       {
              static String lastIP = "";

              if (dominioCache.isEmpty()) {
                  if constexpr (Config::Debug::DNS_DEBUG) {
                    Serial.println("DNS: No hay dominio cacheado, saltando actualización");
                  }
                  return;
              }
              if (!hayInternet()) return;

              String currentIP = WiFi.localIP().toString();

              // Actualizar si cambió la IP
              if (currentIP != lastIP) {
                  ActualizaDNS(dominioCache.c_str());
                  lastIP = currentIP;
                 if constexpr (Config::Debug::DNS_DEBUG) {
                    Serial.println("DNS actualizado - IP: " + currentIP);
                  }
              }
        }