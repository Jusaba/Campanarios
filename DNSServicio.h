/**
 * @file DNSServicio.h
 * @brief Servicio de actualización dinámica de DNS para proyecto Campanarios
 * 
 * @details Este header define un servicio completo de DNS dinámico que permite
 *          actualizar automáticamente la dirección IP pública del dispositivo
 *          en servidores DNS externos mediante peticiones HTTP autenticadas:
 *          
 *          **FUNCIONALIDADES PRINCIPALES:**
 *          - Actualización DNS en dos servidores independientes para redundancia
 *          - Autenticación HTTP Basic con credenciales configurables
 *          - Cache inteligente para evitar actualizaciones innecesarias
 *          - Detección automática de cambios de IP pública
 *          - Timeout configurable para peticiones HTTP
 *          - Manejo robusto de errores de conectividad y timeout
 *          
 *          **ARQUITECTURA DEL SISTEMA:**
 *          - Servidores duales: Redundancia automática entre dos proveedores DNS
 *          - Cache de IP: Almacena última IP actualizada para evitar spam a servidores
 *          - HTTPClient reutilizable: Una instancia para ambos servidores
 *          - Credenciales globales: Usuario y password configurables para autenticación
 * 
 * @note **SERVIDORES DNS SOPORTADOS:**
 *       - Servidor primario: Definido en serverUrl1 (típicamente proveedor principal)
 *       - Servidor secundario: Definido en serverUrl2 (backup/redundancia)
 *       - Ambos servidores se actualizan en cada llamada para consistencia
 * 
 * @note **CACHE INTELIGENTE:**
 *       - ActualizaDNSSiNecesario() solo actualiza si IP ha cambiado
 *       - Evita spam a servidores DNS con actualizaciones innecesarias
 *       - Variable global lastUpdatedIP almacena última IP procesada
 * 
 * @note **AUTENTICACIÓN:**
 *       - HTTP Basic Authentication con username y userPassword
 *       - Credenciales enviadas en cada petición a ambos servidores
 *       - Compatible con proveedores estándar de DNS dinámico
 * 
 * @warning **DEPENDENCIAS CRÍTICAS:**
 *          - WiFi.h: Conexión a Internet requerida para peticiones HTTP
 *          - HTTPClient.h: Cliente HTTP del ESP32 para peticiones autenticadas
 *          - Configuracion.h: Credenciales y URLs de servidores (implícito)
 * 
 * @warning **REQUISITOS DE RED:**
 *          - Conexión Wi-Fi estable y activa
 *          - Acceso HTTP/HTTPS a servidores DNS externos (puerto 80/443)
 *          - Resolución DNS funcional para hostnames de servidores
 * 
 * @warning **SEGURIDAD:**
 *          - Credenciales transmitidas en HTTP Basic (base64, no cifrado)
 *          - URLs y credenciales hardcoded en código fuente
 *          - Sin validación de certificados SSL en HTTPS
 *          - Considerar HTTPS y almacenamiento seguro de credenciales
 * 
 * @author Julian Salas Bartolomé
 * @date 2025-09-14
 * @version 1.5
 * 
 * @since v1.0 - Actualización DNS básica con un servidor
 * @since v1.5 - Dual servers, cache inteligente y debug mejorado
 * 
 * @see DNSServicio.cpp - Implementación de todas las funcionalidades
 * @see Configuracion.h - URLs de servidores y credenciales (implícito)
 * @see ConexionWifi.cpp - Sistema de conectividad WiFi requerido
 * 
 * @todo Añadir configuración dinámica de servidores desde EEPROM
 * @todo Implementar retry automático con backoff exponencial
 * @todo Añadir validación de respuestas DNS (no solo código HTTP)
 * @todo Implementar rotación de servidores si uno falla sistemáticamente
 */
#ifndef DNSSERVICIO_H
	#define DNSSERVICIO_H
    #include <WiFi.h>
    #include <HTTPClient.h>
    

    void ActualizaDNS (const char* cDominio);
    void ActualizaDNSSiNecesario(void);



#endif