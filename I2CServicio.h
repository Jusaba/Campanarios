/**
 * @file I2CServicio.h
 * @brief Implementación del servicio de comunicación I2C esclavo para proyecto Campanarios
 * 
 * @details Este archivo implementa un sistema completo de comunicación I2C que permite
 *          al ESP32 funcionar como dispositivo esclavo para intercambiar información
 *          con un dispositivo maestro (típicamente Arduino o ESP32 maestro):
 *          
 *          **FUNCIONALIDADES IMPLEMENTADAS:**
 *          - Comunicación I2C bidireccional como dispositivo esclavo
 *          - Recepción de comandos y parámetros del dispositivo maestro
 *          - Envío de estado del campanario, fecha/hora y temporizaciones
 *          - Protocolo diferenciado entre comandos y solicitudes de información
 *          - Manejo inteligente de calefacción (envía temporización vs fecha/hora)
 *          - Integración completa con el sistema RTC para datos temporales
 *          
 *          **PROTOCOLO I2C IMPLEMENTADO:**
 *          1. **Comandos directos:** Secuencias que ejecutan acciones inmediatamente
 *          2. **Solicitudes de información:** Requieren respuesta en siguiente petición
 *          3. **Parámetros opcionales:** Algunos comandos incluyen byte de parámetro
 *          4. **Respuestas estructuradas:** Datos organizados según tipo de solicitud
 *          
 *          **TIPOS DE RESPUESTA:**
 *          - Estado simple: 1 byte con estado actual del campanario
 *          - Hora: Estado + hora + minuto + segundo (4 bytes)
 *          - Fecha completa: Estado + día + mes + año + hora + minuto + segundo (7 bytes)
 *          - Temporización: Estado + 00 + 00 + 00 + horas + minutos + segundos (7 bytes)
 * 
 * @note **DIRECCIONAMIENTO I2C:**
 *       - Dirección esclavo: Config::I2C::SLAVE_ADDR (definida en Configuracion.h)
 *       - Protocolo estándar I2C con callbacks Wire.onReceive() y Wire.onRequest()
 *       - Soporte para comunicación full-duplex (envío y recepción)
 * 
 * @note **INTEGRACIÓN SISTEMA:**
 *       - Acceso directo al objeto Campanario para estado y control
 *       - Integración con RTC para datos temporales precisos
 *       - Variable global nSegundosTemporizacion para calefacción temporizada
 *       - Debug modular activable por diferentes flags de desarrollo
 * 
 * @warning **SINCRONIZACIÓN:**
 *          - Variables volatile para evitar problemas de concurrencia
 *          - Callbacks I2C ejecutan en contexto de interrupción
 *          - Procesamiento mínimo en callbacks para evitar bloqueos
 * 
 * @warning **DEPENDENCIAS CRÍTICAS:**
 *          - Wire.h: Biblioteca I2C del ESP32 fundamental
 *          - Configuracion.h: Definición de direcciones y estados
 *          - Auxiliar.h: Acceso a objeto Campanario y variables globales
 *          - RTC.h: Funciones de tiempo y sincronización NTP
 *          - Debug.h: Sistema de logging modular
 * 
 * @author Julian Salas Bartolomé
 * @date 2025-09-14
 * @version 2.0
 * 
 * @since v1.0 - Comunicación I2C básica con comandos simples
 * @since v2.0 - Protocolo avanzado, temporización calefacción y debug modular
 * 
 * @see I2CServicio.h - Definiciones y prototipos del servicio I2C
 * @see Configuracion.h - Estados y direcciones I2C utilizadas
 * @see Auxiliar.h - Objeto Campanario y variables globales accedidas
 * @see RTC.h - Funciones de tiempo utilizadas para respuestas
 * @see Debug.h - Macros de debug utilizadas en el módulo
 * 
 * @todo Implementar checksum para validación de integridad de datos
 * @todo Añadir timeout para detectar maestro desconectado
 * @todo Implementar buffer circular para comandos múltiples
 * @todo Añadir autenticación para comandos críticos
 */
#ifndef I2CSERVICIO_H
	#define I2CSERVICIO_H

    #include <Arduino.h>
    #include <Wire.h>
    #include <stdint.h>



    // Variables I2C globales (extern para usar desde otros módulos)
    extern volatile uint8_t secuenciaI2C;               // Secuencia de comando recibida por I2C (volatile por interrupciones)      
    extern uint8_t requestI2C;                          // Solicitud de información 
    extern uint8_t ParametroI2C;                        // Parámetro adicional para ciertos comandos (opcional)

    // Funciones de inicialización I2C
    void initI2C();

    // Funciones de comunicación I2C
    void recibirSecuencia(int numBytes);
    void enviarRequest();

    // Funciones de envío específicas I2C
    void enviarEstadoI2C();
    void enviarHoraI2C();
    void enviarFechaHoraI2C();
    void enviarEstadoTemporizacionI2C();
    void enviarFechaoTemporizacionI2C();

#endif