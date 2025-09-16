
/**
 * @file Acciones.h
 * @brief Declaraciones de funciones de acción para el sistema de campanario automatizado por Alarmas o manual
 * 
 * @details Este archivo define las funciones de acción que pueden ser ejecutadas
 *          por el sistema de alarmas programadas, el servidor web, o manualmente.
 *          Cada función implementa una acción específica del campanario con
 *          verificaciones inteligentes de estado y condiciones.
 *          
 *          **TIPOS DE ACCIONES DEFINIDAS:**
 *          - Secuencias de campanadas (con verificación de campanario ocupado)
 *          - Toques de hora y media hora (con protección horario nocturno)
 *          - Mantenimiento del sistema (sincronización NTP, DNS)
 *          - Acciones que siempre se ejecutan (sin verificaciones de campanario)
 *          
 *          **ARQUITECTURA INTELIGENTE:**
 *          - Solo las acciones de campanadas verifican si el campanario está ocupado
 *          - Las acciones de mantenimiento se ejecutan siempre
 *          - Verificación automática de horarios nocturnos para protección
 *          - Integración completa con sistema de alarmas programadas
 *          
 *          **INTEGRACIÓN SISTEMA:**
 *          - Compatible con sistema de alarmas (función de acción como parámetro)
 *          - Utilizables desde servidor web para control manual
 *          - Funciones de mantenimiento automático (NTP, DNS)
 *          - Debug integrado para monitorización
 * 
 * @note **MODULAR:** Cada acción es independiente y puede usarse por separado
 * @note **INTELIGENTE:** Verificaciones automáticas según el tipo de acción
 * @note **SEGURO:** Protection contra interferencias entre acciones de campanadas
 * 
 * @warning **CAMPANARIO:** Las acciones de campanadas requieren objeto Campanario inicializado
 * @warning **RTC:** Las acciones de tiempo requieren RTC sincronizado
 * @warning **WIFI:** Las acciones de mantenimiento requieren conexión WiFi válida
 * 
 * @author Julian Salas Bartolomé
 * @date 2025-09-16
 * @version 1.0
 * 
 * @see Acciones.cpp - Implementación de todas las funciones declaradas
 * @see Alarmas.h - Sistema que utiliza estas funciones como callbacks
 * @see Campanario.h - Sistema de campanario controlado por estas acciones
 * @see Servidor.h - Servidor web que puede invocar estas acciones
 * 
 * @todo Implementar acciones adicionales para eventos especiales
 * @todo Añadir acciones de diagnóstico y mantenimiento avanzado
 */
#ifndef ACCIONES_H
    #define ACCIONES_H
    #include <Arduino.h>  
    #include <stdint.h>
    #include "Debug.h"

    #define DebugAcciones

    // Funciones específicas del campanario para usar con Alarmas
    void accionSecuencia(uint16_t seqId);                                               // Ejecuta secuencia específica
    void accionTocaHora(void);                                                          // Ejecuta toque de hora
    void accionTocaMedia(void);                                                         // Ejecuta toque de media hora


    void SincronizaNTP( void );                                                         // Sincroniza reloj con servidor NTP    
    


#endif