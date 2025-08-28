#include "DNSServicio.h"
#include <WiFi.h>
#include <HTTPClient.h>


    const char* serverUrl1 = "http://serverpic.cloud:8001"; 
    const char* serverUrl2 = "http://serverpic.com:8001";
    //const char* username = "raimat";                             //Nombre del dominio deseado
    const char* userPassword = "JUSABA2408";      

    HTTPClient http1;
    HTTPClient http2;


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