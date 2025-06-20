/**
 * @file DNSServicio.h
 * @brief Servicio para actualización dinámica de DNS mediante peticiones HTTP.
 *
 * Este archivo define las funciones y constantes necesarias para realizar peticiones HTTP
 * a dos servidores DNS, permitiendo la actualización dinámica de la dirección IP asociada
 * a un dominio. Utiliza autenticación básica y muestra información de depuración por el puerto serie.
 *
 * Funcionalidad principal:
 *  - Actualiza la IP en dos servidores DNS mediante peticiones HTTP GET autenticadas.
 *  - Muestra por consola el código de respuesta HTTP y el texto de respuesta recibido.
 *
 * Constantes:
 *  - serverUrl1, serverUrl2: URLs de los servidores DNS.
 *  - username, userPassword: Credenciales para la autenticación HTTP básica.
 *
 * Funciones:
 *  - void ActualizaDNS(void): Realiza la actualización de DNS en ambos servidores.
 *
 * Dependencias:
 *  - WiFi.h
 *  - HTTPClient.h
 *
 * Uso:
 *  Llama a ActualizaDNS() cuando quieras actualizar la IP en los servidores DNS configurados.
 */
#ifndef DNSSERVICIO_H
	#define DNSSERVICIO_H
    #include <WiFi.h>
    #include <HTTPClient.h>
    
    #define DEBUGDNS


    const char* serverUrl1 = "http://serverpic.cloud:8001"; 
    const char* serverUrl2 = "http://serverpic.com:8001";
    //const char* username = "raimat";                             //Nombre del dominio deseado
    const char* userPassword = "JUSABA2408";      

    HTTPClient http1;
    HTTPClient http2;

    void ActualizaDNS (const char* cDominio);

    void ActualizaDNS (const char* cDominio)
    {
        // http al servidor DNS 1
        http1.begin(serverUrl1);
        http1.setAuthorization(cDominio, userPassword);
        int httRespuesta = http1.GET();
        #ifdef DEBUGDNS
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
        #endif
        http1.end();
        // http al servidor DNS 2
        http2.begin(serverUrl2);
        http2.setAuthorization(cDominio, userPassword);
        httRespuesta = http2.GET();
        #ifdef DEBUGDNS
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
        #endif
        http2.end();    
       }
#endif