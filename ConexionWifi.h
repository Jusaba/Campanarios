/**
 * @file ConexionWifi.h
 * @brief Funciones y definiciones para la conexión Wi-Fi con IP estática en ESP32.
 *
 * Este archivo contiene la declaración y definición de la función para conectar el ESP32
 * a una red Wi-Fi utilizando una configuración de IP estática. Incluye la actualización
 * del DNS y la sincronización del reloj RTC mediante NTP tras la conexión exitosa.
 *
 * Dependencias:
 * - DNSServicio.h: Para la actualización del DNS.
 * - Servidor.h: Para la gestión del servidor local (si aplica).
 * - ModoAp.h: Para el modo punto de acceso (si aplica).
 * - RTC.h: Para la sincronización del reloj en tiempo real.
 * - Librería WiFi estándar de ESP32.
 *
 *
 * Macros:
 * - DEBUGWIFI: Habilita mensajes de depuración por Serial.
 *
 * Funciones:
 * - void ConectarWifi (const ConfigWiFi& ConfiguracionWiFi): Establece la conexión Wi-Fi, configura IP estática con los datos pasados en la estructura ConfigWiFi,
 *   actualiza DNS y sincroniza el RTC.
 *
 * @author Julian Salas Bartolome
 * @date 2025-05-27
 * @version 1.0
 */
#ifndef CONEXIONWIFI_H
  
  #define CONEXIONWIFI_H

  #include "ModoAP.h"

  #define DEBUGWIFI

  bool ConectarWifi(const ConfigWiFi& ConfiguracionWiFi, unsigned long timeout_ms = 10000);

#endif
