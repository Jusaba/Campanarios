/**
 * @file ConexionWifi.cpp
 * @brief Implementación del sistema de conexión Wi-Fi para proyecto Campanarios
 * 
 * @details Este archivo implementa la gestión completa de conexiones Wi-Fi del sistema:
 *          
 *          **FUNCIONALIDADES IMPLEMENTADAS:**
 *          - Conexión Wi-Fi con configuración IP estática o DHCP automático
 *          - Parsing y validación automática de direcciones IP
 *          - Configuración inteligente de gateway (heurística a.b.c.1)
 *          - Timeout configurable para evitar bloqueos indefinidos
 *          - Integración automática con servicios DNS y RTC tras conexión exitosa
 *          - Logging detallado del proceso de conexión (debug condicional)
 *          - Fallback automático a DHCP si IP estática es inválida
 *          
 *          **FLUJO DE CONEXIÓN:**
 *          1. Parsing y validación de IP estática desde configuración
 *          2. Configuración de parámetros de red (IP, gateway, DNS)
 *          3. Inicio de conexión Wi-Fi con credenciales
 *          4. Espera con timeout configurable
 *          5. Si exitosa: Actualización DNS + Sincronización RTC
 *          6. Si falla: Retorno de error sin bloquear sistema
 * 
 * @note **CONFIGURACIÓN IP ESTÁTICA:**
 *       - Parsing automático de formato "a.b.c.d" 
 *       - Gateway inferido como "a.b.c.1" (convención estándar)
 *       - Subnet mask /24 (255.255.255.0) por defecto
 *       - DNS primario/secundario Google (8.8.8.8 / 8.8.4.4)
 * 
 * @note **INTEGRACIÓN CON SERVICIOS:**
 *       - ActualizaDNS(): Configuración automática de DNS personalizado
 *       - RTC::beginConMultiplesServidores(): Sincronización temporal
 *       - Debug condicional con macro DEBUGWIFI
 * 
 * @warning **DEPENDENCIAS CRÍTICAS:**
 *          - WiFi.h debe estar correctamente inicializado
 *          - DNSServicio.h debe implementar ActualizaDNS()
 *          - RTC.h debe implementar beginConMultiplesServidores()
 *          - ConfigWiFi debe tener campos: ssid, password, ip, dominio
 * 
 * @author Julian Salas Bartolomé
 * @date 2025-09-14
 * @version 2.1
 * 
 * @since v1.0 - Conexión básica Wi-Fi con IP estática
 * @since v2.0 - Añadido parsing automático y fallback DHCP
 * @since v2.1 - Integración con servicios DNS y RTC, timeout configurable
 * 
 * @see ConexionWifi.h - Definiciones y estructuras de configuración
 * @see DNSServicio.h - Servicios DNS integrados
 * @see RTC.h - Sincronización temporal
 * @see Configuracion.h - Estructura ConfigWiFi
 * 
 */
#ifndef CONEXIONWIFI_H
  
  #define CONEXIONWIFI_H

  #include "ModoAP.h"

  bool ConectarWifi(const ConfigWiFi& ConfiguracionWiFi, unsigned long timeout_ms = 10000);

#endif
