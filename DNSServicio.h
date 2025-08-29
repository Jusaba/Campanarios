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
    void ActualizaDNS (const char* cDominio);
    void ActualizaDNSSiNecesario(void);



#endif
