/**
 * @file Alarmas.h
 * @brief Implementación del sistema de alarmas programables 
 * 
 * @details Este archivo implementa un sistema completo de programación de alarmas
 *          que permite ejecutar acciones específicas en horarios predeterminados:
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
 *          
 *          **TIPOS DE ALARMAS SOPORTADAS:**
 *          1. **Horario fijo:** Día específico + hora + minuto exactos
 *          2. **Wildcard:** Cualquier hora (*) y/o cualquier minuto (*)
 *          3. **Intervalo:** Repetición cada X minutos desde punto de anclaje
 *          4. **Días múltiples:** Máscara de bits para días de la semana
 *          
 *          **PREVENCIÓN DE DUPLICADOS:**
 *          - Cache por día del año (ultimoDiaAno) para alarmas diarias
 *          - Cache por minuto (ultimoMinuto) para alarmas en mismo día
 *          - Timestamp epoch (ultimaEjecucion) para alarmas de intervalo
 *          - Verificación de estado de secuencia para evitar solapamientos
 * 
 * @note **INTEGRACIÓN SISTEMA:**
 *       - Acceso directo a objeto Campanario para verificar secuencias activas
 *       - Utilización de funciones de Acciones.h para acciones estándar
 *       - Sincronización con RTC mediante getLocalTime() del ESP32
 *       - Debug modular activable con DebugAlarma
 * 
 * @note **CONFIGURACIÓN HORARIA:**
 *       - Formato 24 horas (0-23 para tm_hour)
 *       - Minutos 0-59 (tm_min)
 *       - Días 0-6 donde 0=Domingo, 6=Sábado (tm_wday)
 *       - Soporte para horarios nocturnos con Config::Time
 * 
 * @warning **DEPENDENCIAS CRÍTICAS:**
 *          - time.h: Funciones de tiempo del sistema (getLocalTime, time_t)
 *          - Configuracion.h: Estados, constantes temporales y configuración
 *          - Acciones.h: Funciones de acción predefinidas del sistema
 *          - Debug.h: Sistema de logging modular DBG_ALM_*
 *          - Auxiliar.h: Objeto Campanario y variables globales (implícito)
 * 
 * @warning **LIMITACIONES:**
 *          - Máximo 16 alarmas simultáneas (MAX_ALARMAS)
 *          - Resolución mínima de 1 minuto (no soporta segundos)
 *          - Cache no persistente (se pierde en reinicios)
 *          - Verificación de RTC requerida para funcionamiento
 * 
 * @author Julian Salas Bartolomé
 * @date 2025-09-14
 * @version 2.0
 * 
 * @since v1.0 - Sistema básico de alarmas con horarios fijos
 * @since v2.0 - Intervalos, wildcards, tres tipos de acciones y prevención duplicados
 * 
 * @see Alarmas.h - Definiciones de estructuras y prototipos
 * @see Acciones.h - Funciones de acción utilizadas por las alarmas
 * @see Configuracion.h - Estados y constantes temporales del sistema
 * @see RTC.h - Sistema de sincronización temporal utilizado
 * 
 * @todo Implementar persistencia de alarmas en EEPROM
 * @todo Añadir soporte para alarmas de segundos específicos
 * @todo Implementar prioridades de alarmas para conflictos
 * @todo Añadir alarmas condicionales (ej: solo si hay WiFi)
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
    bool esHorarioNocturno() const;
    void resetCache();

    // === GESTIÓN WEB DE ALARMAS PERSONALIZABLES ===
    uint8_t addPersonalizable(const char* nombre, const char* descripcion,
                         uint8_t mascaraDias, uint8_t hora, uint8_t minuto,
                         const char* tipoString, uint16_t parametro,
                         void (*callback)(uint16_t), bool habilitada = true);
    
    bool    modificarPersonalizable(int idWeb, const char* nombre, const char* descripcion,
                                   uint8_t mascaraDias, uint8_t hora, uint8_t minuto,
                                   const char* tipoAccion, bool habilitada);
    
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