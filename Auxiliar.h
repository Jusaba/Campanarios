/**
 * @file Auxiliar.h
 * @brief Declaraciones de funciones auxiliares para el sistema de campanario
 * 
 * @details Este archivo contiene las declaraciones de las funciones auxiliares y de utilidad
 *          del sistema de campanario automatizado que incluyen:
 *          
 *          **FUNCIONALIDADES PRINCIPALES:**
 *          - Ejecución de secuencias de campanadas por ID
 *          - Pruebas y diagnósticos del sistema de campanadas
 *          - Verificación y monitorización de conexión a internet
 *          - Gestión de protección de campanadas por horarios
 *          - Control de períodos de toque permitidos
 *          - Coordinación entre subsistemas del campanario
 *          
 *          **FUNCIONES DE SECUENCIAS:**
 *          - EjecutaSecuencia(): Mapeo de IDs a secuencias específicas
 *          - TestCampanadas(): Diagnóstico y pruebas del sistema
 *          - Soporte para parámetros adicionales en secuencias
 *          
 *          **GESTIÓN DE CONECTIVIDAD:**
 *          - TestInternet(): Verificación periódica de conexión
 *          - Actualización automática de estado de conectividad
 *          - Integración con DNS dinámico
 *          
 *          **PROTECCIÓN DE CAMPANADAS:**
 *          - Control de horarios nocturnos automático
 *          - Períodos de silencio configurables
 *          - Integración con estado del campanario
 * 
 * @note **COORDINACIÓN:** Este módulo actúa como pegamento entre subsistemas
 * @note **FLEXIBILIDAD:** Permite extensión fácil de nuevas secuencias
 * @note **MONITORIZACIÓN:** Funciones de diagnóstico integradas
 * 
 * @warning **VARIABLES GLOBALES:** Depende de variables globales inicializadas
 * @warning **CAMPANARIO:** Requiere objeto Campanario inicializado
 * @warning **RTC:** Funciones de tiempo requieren RTC sincronizado
 * 
 * @author Julian Salas Bartolomé
 * @date 2025-09-16
 * @version 1.0
 * 
 * @see Auxiliar.h - Declaraciones e interfaces utilizadas
 * @see Campanario.h - Sistema de campanario controlado
 * @see Acciones.h - Funciones de acción que utilizan estas utilidades
 * @see ConexionWifi.h - Gestión de conectividad utilizada
 * 
 * @todo Implementar estadísticas de uso de secuencias
 * @todo Añadir diagnósticos avanzados de hardware
 * @todo Integrar log de eventos para análisis posterior
 */
#ifndef AUXILIAR_H
#define AUXILIAR_H

#include "RTC.h"
//#include <Wire.h>
#include "Servidor.h"
#include "ConexionWifi.h"  // Para ConfigWiFi
#include "Configuracion.h"
#include "Debug.h"
#include "Campanario.h"   // Para CAMPANARIO
#include <time.h>
#include "Alarmas.h"

// Variables globales 

extern CAMPANARIO Campanario;                           //Campanario

extern struct tm timeinfo;                              // Estructura de tiempo global   
extern int nCampanaTocada;                              // Campana que se está tocando
extern bool lConexionInternet;                          // Estado de conexión a Internet
extern bool lProteccionCampanadas;                      // Estado de protección de campanadas
extern bool lProteccionCampanadasAnterior;              // Estado anterior de protección de campanadas
extern unsigned long ultimoCheckInternet;               // Última verificación de conexión a Internet
//extern const unsigned long intervaloCheckInternet;      // Intervalo de verificación de conexión a Internet
extern int nTemporizacionCalefaccion;                   // Temporización de calefacción en minutos
extern double nSegundosTemporizacion;                   // Temporización de calefacción en segundos

extern ConfigWiFi configWiFi;                           // Estructura de configuración WiFi

// SOLO prototipos (sin implementaciones)

void TestCampanadas(void);                              // Prototipo de la función para probar las campanadas
void EjecutaSecuencia(int nSecuencia);                  // Prototipo de la función para ejecutar una secuencia
void EjecutaSecuencia(int nSecuencia, int nParametro);  // Prototipo de la función para ejecutar una secuencia con parámetro
void TestInternet(void);                                // Prototipo de la función para probar la conexión a Internet

bool EsPeriodoToqueCampanas(void);                      // Prototipo de la función para comprobar si estamos en el período de toque de campanas
void ActualizaEstadoProteccionCampanadas(void);         // Prototipo de la función para actualizar el estado de protección de campanadas

void IniciaAlarmas (void);                             // Prototipo de la función para iniciar las alarmas

#endif