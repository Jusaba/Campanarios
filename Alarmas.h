/**
 * @file Alarmas.h
 * @brief Sistema completo de alarmas programables con gestión web
 * 
 * @details Este archivo implementa un sistema avanzado de programación de alarmas
 *          que permite ejecutar acciones específicas en horarios predeterminados,
 *          con soporte completo para gestión web de alarmas personalizables:
 *          
 *          **FUNCIONALIDADES IMPLEMENTADAS:**
 *          - Sistema de alarmas programables con máscaras de días de la semana
 *          - Soporte para horarios específicos y wildcards (cualquier hora/minuto)
 *          - Alarmas de intervalo que se repiten cada X minutos
 *          - Tres tipos de acciones: métodos miembro, funciones externas con/sin parámetros
 *          - Prevención de ejecuciones duplicadas en el mismo minuto/día
 *          - Sistema de cache temporal para evitar disparos múltiples
 *          - Integración completa con RTC y sincronización NTP
 *          - Debug detallado de ejecución y estado de alarmas
 *          - **GESTIÓN WEB:** Creación, edición, eliminación vía interfaz web
 *          - **PERSISTENCIA JSON:** Almacenamiento automático en SPIFFS
 *          - **IDs ÚNICOS:** Sistema de identificación web independiente
 *          - **CALLBACKS DINÁMICOS:** Configuración de acciones desde servidor web
 *          
 *          **TIPOS DE ALARMAS SOPORTADAS:**
 *          1. **Horario fijo:** Día específico + hora + minuto exactos
 *          2. **Wildcard:** Cualquier hora (*) y/o cualquier minuto (*)
 *          3. **Intervalo:** Repetición cada X minutos desde punto de anclaje
 *          4. **Días múltiples:** Máscara de bits para días de la semana
 *          5. **PERSONALIZABLES:** Alarmas editables vía web con persistencia
 *          6. **SISTEMA:** Alarmas predefinidas no editables por usuario
 *          
 *          **GESTIÓN WEB DE ALARMAS PERSONALIZABLES:**
 *          - Creación dinámica con nombre, descripción y tipo de acción
 *          - Edición completa preservando callbacks configurados
 *          - Eliminación segura con reorganización automática del array
 *          - Habilitación/deshabilitación individual por ID web
 *          - Exportación JSON para interfaz web (lista completa + estadísticas)
 *          - Persistencia automática en /alarmas_personalizadas.json
 *          - Carga automática al inicio del sistema
 *          - IDs web únicos independientes del índice del array
 *          
 *          **TIPOS DE ACCIÓN WEB SOPORTADOS:**
 *          - "MISA": Secuencia de llamada a misa con parámetros específicos
 *          - "DIFUNTOS": Secuencia de toque de difuntos
 *          - "FIESTA": Secuencias festivas personalizables
 *          - "SISTEMA": Acciones internas del sistema (no editables)
 *          - **EXTENSIBLE:** Fácil añadir nuevos tipos de acción
 *          
 *          **PREVENCIÓN DE DUPLICADOS:**
 *          - Cache por día del año (ultimoDiaAno) para alarmas diarias
 *          - Cache por minuto (ultimoMinuto) para alarmas en mismo día
 *          - Timestamp epoch (ultimaEjecucion) para alarmas de intervalo
 *          - Verificación de estado de secuencia para evitar solapamientos
 *          
 *          **ARQUITECTURA DE PERSISTENCIA:**
 *          - Archivo JSON: /alarmas_personalizadas.json en SPIFFS
 *          - Estructura versionada con metadatos de configuración
 *          - Separación clara entre alarmas de sistema y personalizables
 *          - Backup automático y validación de integridad
 *          - Migración automática de versiones anteriores
 * 
 * @note **INTEGRACIÓN SISTEMA:**
 *       - Acceso directo a objeto Campanario para verificar secuencias activas
 *       - Utilización de funciones de Acciones.h para acciones estándar
 *       - Sincronización con RTC mediante getLocalTime() del ESP32
 *       - Debug modular activable con DebugAlarma
 *       - **INTEGRACIÓN WEB:** Comandos WebSocket desde Servidor.cpp
 *       - **CALLBACKS EXTERNOS:** Configuración de acciones desde código externo
 * 
 * @note **CONFIGURACIÓN HORARIA:**
 *       - Formato 24 horas (0-23 para tm_hour)
 *       - Minutos 0-59 (tm_min)
 *       - Días 0-6 donde 0=Domingo, 6=Sábado (tm_wday)
 *       - Soporte para horarios nocturnos con Config::Time
 *       - **MÁSCARAS WEB:** Conversión automática de formato web (1-7) a máscaras
 * 
 * @note **COMANDOS WEBSOCKET SOPORTADOS:**
 *       - ADD_ALARMA_WEB: Crear nueva alarma personalizable
 *       - EDIT_ALARMA_WEB: Modificar alarma existente
 *       - DELETE_ALARMA_WEB: Eliminar alarma por ID web
 *       - TOGGLE_ALARMA_WEB: Habilitar/deshabilitar alarma
 *       - GET_ALARMAS_WEB: Obtener lista completa JSON
 *       - GET_STATS_ALARMAS_WEB: Obtener estadísticas del sistema
 * 
 * @warning **DEPENDENCIAS CRÍTICAS:**
 *          - time.h: Funciones de tiempo del sistema (getLocalTime, time_t)
 *          - Configuracion.h: Estados, constantes temporales y configuración
 *          - Acciones.h: Funciones de acción predefinidas del sistema
 *          - Debug.h: Sistema de logging modular DBG_ALM_*
 *          - ArduinoJson.h: Serialización/deserialización JSON para persistencia
 *          - SPIFFS.h: Sistema de archivos para almacenamiento persistente
 *          - **Servidor.cpp:** Configuración de callbacks para alarmas web
 * 
 * @warning **LIMITACIONES:**
 *          - Máximo 16 alarmas simultáneas total (sistema + personalizables)
 *          - Resolución mínima de 1 minuto (no soporta segundos)
 *          - Cache no persistente (se pierde en reinicios)
 *          - Verificación de RTC requerida para funcionamiento
 *          - **SPIFFS:** Requiere espacio suficiente para archivo JSON
 *          - **CALLBACKS:** Deben ser configurados externamente antes de crear alarmas
 *          - **THREAD SAFETY:** No thread-safe, usar desde hilo principal únicamente
 * 
 * @warning **LIMITACIONES WEB:**
 *          - IDs web no reutilizan números eliminados (solo crecen)
 *          - Callbacks no son serializables (se configuran al crear)
 *          - Modificación no cambia callbacks existentes (por seguridad)
 *          - Eliminación reorganiza array (puede cambiar índices internos)
 * 
 * @author Julian Salas Bartolomé
 * @date 2025-01-06
 * @version 2.1
 * 
 * @since v1.0 - Sistema básico de alarmas con horarios fijos
 * @since v2.0 - Intervalos, wildcards, tres tipos de acciones y prevención duplicados
 * @since v2.1 - Sistema de alarmas personalizables vía web con persistencia JSON
 * 
 * @see Alarmas.cpp - Implementación completa de todas las funciones
 * @see Servidor.cpp - Procesamiento de comandos web y configuración de callbacks
 * @see Acciones.h - Funciones de acción utilizadas por las alarmas
 * @see Configuracion.h - Estados y constantes temporales del sistema
 * @see RTC.h - Sistema de sincronización temporal utilizado
 * 
 * @todo Implementar sistema de prioridades para alarmas conflictivas
 * @todo Añadir soporte para alarmas de segundos específicos
 * @todo Implementar alarmas condicionales (ej: solo si hay WiFi)
 * @todo Sistema de backup/restore completo de configuración
 * @todo Interfaz web para gestión avanzada de alarmas
 * @todo Validación de parámetros en tiempo real desde web
 * @todo Sistema de notificaciones push para cambios de estado
 * @todo Historial de ejecuciones de alarmas para análisis
 * 
 * @example **EJEMPLO DE USO - ALARMA DE SISTEMA:**
 * @code
 * // Crear alarma de sistema básica (no editable por web)
 * uint8_t idx = Alarmas.add(
 *     DOW_TODOS,                    // Todos los días
 *     ALARMA_WILDCARD, 0,          // Cada hora en punto
 *     0,                           // No intervalo
 *     &AlarmScheduler::tocarHoras, // Método de la clase
 *     0,                           // Sin parámetro
 *     true                         // Habilitada
 * );
 * @endcode
 * 
 * @example **EJEMPLO DE USO - ALARMA PERSONALIZABLE VÍA WEB:**
 * @code
 * // Desde Servidor.cpp - procesamiento de comando ADD_ALARMA_WEB
 * uint8_t idx = Alarmas.addPersonalizable(
 *     "Misa Domingo",              // Nombre descriptivo
 *     "Primera llamada dominical", // Descripción
 *     DOW_DOMINGO,                 // Solo domingos
 *     11, 5,                       // 11:05 AM
 *     "MISA",                      // Tipo de acción
 *     Config::States::MISA,        // Parámetro para callback
 *     accionSecuencia,             // Callback externo
 *     true                         // Habilitada inicialmente
 * );
 * @endcode
 * 
 * @example **EJEMPLO DE GESTIÓN JSON:**
 * @code
 * // Obtener alarmas para interfaz web
 * String json = Alarmas.obtenerPersonalizablesJSON();
 * ws.textAll("ALARMAS_WEB:" + json);
 * 
 * // Obtener estadísticas del sistema
 * String stats = Alarmas.obtenerEstadisticasJSON();
 * ws.textAll("STATS_ALARMAS_WEB:" + stats);
 * @endcode
 */

#ifndef ALARMAS_H
#define ALARMAS_H

#include <time.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <vector>
#include "Acciones.h"
#include "Configuracion.h"
#include "Debug.h"
#include "DNSServicio.h"

//#define DebugAlarma

// Máscaras días (bit0 = Domingo ... bit6 = Sábado)
enum : uint8_t {
    DOW_DOMINGO    = 1 << 0,
    DOW_LUNES      = 1 << 1,
    DOW_MARTES     = 1 << 2,
    DOW_MIERCOLES  = 1 << 3,
    DOW_JUEVES     = 1 << 4,
    DOW_VIERNES    = 1 << 5,
    DOW_SABADO     = 1 << 6,
    DOW_TODOS      = 0x7F
};

#define ALARMA_WILDCARD 255   // wildcard (*)

class AlarmScheduler; // forward

struct Alarm {
    bool     habilitada          = false;                       // Si la alarma está habilitada
    uint8_t  mascaraDias         = DOW_TODOS;                   // Máscara de días (bit0=Domingo ... bit6=Sábado)
    uint8_t  hora                = 0;                           // Hora (0-23 o ALARMA_WILDCARD)
    uint8_t  minuto              = 0;                           // Minuto (0-59 o ALARMA_WILDCARD)
    uint16_t intervaloMin        = 0;                           // Intervalo (minutos)
    int16_t  ultimoDiaAno        = -1;                          // Último día del año
    uint8_t  ultimoMinuto        = 255;                         // Último minuto  
    uint8_t  ultimaHora          = 255;                         // Última hora ejecutada (255 inicial)
    time_t   ultimaEjecucion     = 0;                           // Acciones a ejecutar
    void     (AlarmScheduler::*accion)(uint16_t) = nullptr;     // Método miembro
    void     (*accionExt)(uint16_t) = nullptr;                  // Función externa con parámetro
    void     (*accionExt0)() = nullptr;                         // Función externa sin parámetro
    uint16_t parametro           = 0;                           // Parámetro para la acción  
    //Campos para la personalizacion via web
    char     nombre[50];                                        // Nombre descriptivo
    char     descripcion[100];                                  // Descripción opcional  
    char     tipoString[20];                                    // "MISA", "DIFUNTOS", "FIESTA", "SISTEMA"
    bool     esPersonalizable;                                  // true = editable vía web, false = sistema
    int      idWeb = -1;                                        // ID único para interfaz web (-1 si no aplica)  
    
   // Constructor para inicializar nuevos campos
    Alarm() : esPersonalizable(false), idWeb(-1) {
        nombre[0] = '\0';
        descripcion[0] = '\0';
        strcpy(tipoString, "SISTEMA");
    }    
};

class AlarmScheduler {
public:
    static constexpr uint8_t MAX_ALARMAS = 16;
    struct tm t;

    bool begin(bool cargarPorDefecto = true);
    void check();
    uint8_t add(uint8_t mascaraDias,
                    uint8_t hora,
                    uint8_t minuto,
                    uint16_t intervaloMin,
                    void (AlarmScheduler::*accion)(uint16_t),
                    uint16_t parametro = 0,
                    bool habilitada = true);
    uint8_t addExternal(uint8_t mascaraDias,
                         uint8_t hora,
                         uint8_t minuto,
                         uint16_t intervaloMin,
                         void (*ext)(uint16_t),
                         uint16_t parametro = 0,
                         bool habilitada = true);      
    uint8_t addExternal0(uint8_t mascaraDias,
                         uint8_t hora,
                         uint8_t minuto,
                         uint16_t intervaloMin,
                         void (*ext0)(),
                         bool habilitada = true);                     
    void disable(uint8_t idx);
    void enable(uint8_t idx);
    void clear();
    uint8_t count() const;
    const Alarm* get(uint8_t idx) const;
    Alarm* getMutable(uint8_t idx);  // Permite modificar alarmas (para restaurar callbacks)
    bool esHorarioNocturno() const;
    void resetCache();

    // === GESTIÓN WEB DE ALARMAS PERSONALIZABLES ===
    uint8_t addPersonalizable(const char* nombre, const char* descripcion,
                         uint8_t mascaraDias, uint8_t hora, uint8_t minuto,
                         const char* tipoString, uint16_t parametro,
                         void (*callback)(uint16_t), bool habilitada = true);
    
    bool modificarPersonalizable(int idWeb, const char* nombre, const char* descripcion,
                           uint8_t mascaraDias, uint8_t hora, uint8_t minuto,
                           const char* tipoString, bool habilitada,
                           void (*callback)(uint16_t), uint16_t parametro); 
    
    bool    eliminarPersonalizable(int idWeb);
    bool    habilitarPersonalizable(int idWeb, bool estado);
    
    String  obtenerPersonalizablesJSON();
    String  obtenerEstadisticasJSON();
    
    bool    cargarPersonalizablesDesdeJSON();
    bool    guardarPersonalizablesEnJSON();
    
     void imprimirTodasLasAlarmas();


private:
    Alarm  _alarmas[MAX_ALARMAS];
    uint8_t _num = 0;

    static uint8_t mascaraDesdeDiaSemana(int diaSemana);
    void initDefaults(); 

    // === VARIABLES PARA GESTIÓN WEB ===
    int     _siguienteIdWeb;
    
    // === MÉTODOS AUXILIARES ===
    uint8_t _buscarIndicePorIdWeb(int idWeb);
    int     _generarNuevoIdWeb();
     
    String  _diaToString(int dia);
    void    _crearAlarmasPersonalizablesPorDefecto();
};

#endif