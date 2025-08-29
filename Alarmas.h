#ifndef ALARMAS_H
#define ALARMAS_H

#include <time.h>
#include <Arduino.h>
#include "Acciones.h"
#include "Configuracion.h"

// La depuración de alarmas se controla con Config::Debug::ALARM_DEBUG

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
    bool     enabled        = false;
    uint8_t  dowMask        = DOW_TODOS;
    uint8_t  hour           = 0;          // 0-23 o ALARMA_WILDCARD
    uint8_t  minute         = 0;          // 0-59 o ALARMA_WILDCARD
    uint16_t intervalMin    = 0;          // >0 => modo intervalo (min)
    int16_t  lastYDay       = -1;
    uint8_t  lastMinute     = 255;
    time_t   lastExecEpoch  = 0;
    void     (AlarmScheduler::*action)(uint16_t) = nullptr; // método miembro
    void     (*extAction)(uint16_t) = nullptr;              // función externa con param
    void     (*extAction0)() = nullptr;                     // función externa sin param
    uint16_t param          = 0;
};

class AlarmScheduler {
public:
    static constexpr uint8_t MAX_ALARMAS = 16;
    struct tm t;

    bool begin(bool cargarPorDefecto = true) {
        clear();
        if (cargarPorDefecto) initDefaults();
        return true;
    }

    // Añadir alarma (método miembro)
    uint8_t add(uint8_t dowMask,
                uint8_t hour,
                uint8_t minute,
                uint16_t intervalMin,
                void (AlarmScheduler::*action)(uint16_t),
                uint16_t param = 0,
                bool enabled = true)
    {
        if (_num >= MAX_ALARMAS) return 255;
        Alarm &a = _alarmas[_num];
        a.enabled       = enabled;
        a.dowMask       = (dowMask ? dowMask : DOW_TODOS);
        a.hour          = hour;
        a.minute        = minute;
        a.intervalMin   = intervalMin;
        a.lastYDay      = -1;
        a.lastMinute    = 255;
        a.lastExecEpoch = 0;
        a.action        = action;
        a.extAction     = nullptr;
        a.extAction0    = nullptr;
        a.param         = param;
        return _num++;
    }

    // Añadir alarma (función externa con parámetro)
    uint8_t addExternal(uint8_t dowMask,
                        uint8_t hour,
                        uint8_t minute,
                        uint16_t intervalMin,
                        void (*ext)(uint16_t),
                        uint16_t param = 0,
                        bool enabled = true)
    {
        if (_num >= MAX_ALARMAS) return 255;
        Alarm &a = _alarmas[_num];
        a.enabled       = enabled;
        a.dowMask       = (dowMask ? dowMask : DOW_TODOS);
        a.hour          = hour;
        a.minute        = minute;
        a.intervalMin   = intervalMin;
        a.lastYDay      = -1;
        a.lastMinute    = 255;
        a.lastExecEpoch = 0;
        a.action        = nullptr;
        a.extAction     = ext;
        a.extAction0    = nullptr;
        a.param         = param;
        return _num++;
    }

    // Añadir alarma (función externa sin parámetro)
    uint8_t addExternal0(uint8_t dowMask,
                         uint8_t hour,
                         uint8_t minute,
                         uint16_t intervalMin,
                         void (*ext0)(),
                         bool enabled = true)
    {
        if (_num >= MAX_ALARMAS) return 255;
        Alarm &a = _alarmas[_num];
        a.enabled       = enabled;
        a.dowMask       = (dowMask ? dowMask : DOW_TODOS);
        a.hour          = hour;
        a.minute        = minute;
        a.intervalMin   = intervalMin;
        a.lastYDay      = -1;
        a.lastMinute    = 255;
        a.lastExecEpoch = 0;
        a.action        = nullptr;
        a.extAction     = nullptr;
        a.extAction0    = ext0;
        a.param         = 0;
        return _num++;
    }


    void disable(uint8_t idx) { if (idx < _num) _alarmas[idx].enabled = false; }
    void enable(uint8_t idx)  { if (idx < _num) _alarmas[idx].enabled = true;  }
    void clear() { _num = 0; }
    uint8_t count() const { return _num; }
    const Alarm* get(uint8_t idx) const { return (idx < _num) ? &_alarmas[idx] : nullptr; }

    void check() {
        if (!getLocalTime(&t)) return;
        uint8_t curHour   = t.tm_hour;
        uint8_t curMinute = t.tm_min;
        uint8_t curDowM   = dowMaskFrom(t.tm_wday);
        int     curYDay   = t.tm_yday;
        time_t  nowEpoch  = time(nullptr);

        if constexpr (Config::Debug::ALARM_DEBUG) {
        static uint32_t lastDbg = 0;
        if (millis() - lastDbg > 5000) {
            Serial.printf("[ALRMCHK] %02u:%02u DOW=%d YDay=%d\n",
                          curHour, curMinute, t.tm_wday, curYDay);
            lastDbg = millis();
        }
        }

        for (uint8_t i = 0; i < _num; ++i) {
            Alarm &a = _alarmas[i];
            if (!a.enabled) continue;
            if (!(a.dowMask & curDowM)) continue;

            bool disparar = false;

            if (a.intervalMin > 0) {
                if (a.lastExecEpoch == 0) {
                    bool anclaOk = true;
                    if (a.hour   != ALARMA_WILDCARD && a.hour   != curHour)   anclaOk = false;
                    if (a.minute != ALARMA_WILDCARD && a.minute != curMinute) anclaOk = false;
                    if (anclaOk) disparar = true;
                } else if ((nowEpoch - a.lastExecEpoch) >= (time_t)(a.intervalMin * 60)) {
                    disparar = true;
                }
            } else {
                if ((a.hour == ALARMA_WILDCARD || a.hour == curHour) &&
                    (a.minute == ALARMA_WILDCARD || a.minute == curMinute) &&
                    !(a.lastYDay == curYDay && a.lastMinute == curMinute))
                {
                    disparar = true;
                }
            }

            if (!disparar) continue;
            if (Campanario.GetEstadoSecuencia()) continue;

            // Ejecutar acción
            if (a.action) {
                (this->*a.action)(a.param);
            } else if (a.extAction) {
                a.extAction(a.param);
            } else if (a.extAction0) {
                a.extAction0();
            }

            a.lastYDay      = curYDay;
            a.lastMinute    = curMinute;
            a.lastExecEpoch = nowEpoch;

            if constexpr (Config::Debug::ALARM_DEBUG) {
            Serial.printf("[ALARM] idx=%u ok param=%u\n", i, a.param);
            }
        }
    }



private:
    Alarm  _alarmas[MAX_ALARMAS];
    uint8_t _num = 0;

    static uint8_t dowMaskFrom(int wday) {
        return (wday >= 0 && wday <= 6) ? (1 << wday) : 0;
    }
    bool esHorarioNocturno() const {
        return (t.tm_hour >= InicioHorarioNocturno || 
            t.tm_hour < FinHorarioNocturno);
    }

    void initDefaults() {
    /*
        // Domingos (misa)
        addSecuencia(DOW_DOMINGO, 11, 5,  101);
        addSecuencia(DOW_DOMINGO, 11, 25, 102);
        // Ejemplos hora/media
        add(DOW_TODOS, ALARMA_WILDCARD, 0, 0, &AlarmScheduler::accionTocaHora);
        add(DOW_TODOS, ALARMA_WILDCARD, 30, 0, &AlarmScheduler::accionTocaMedia);
        // Cada minuto
        add(DOW_TODOS, ALARMA_WILDCARD, ALARMA_WILDCARD, 0, &AlarmScheduler::accionMinutos);
        // Función externa con parámetro (defínela en Acciones.cpp)
        // addExternal(DOW_TODOS, 12, 15, 0, MiFuncion, 123);
        // Función externa sin parámetro (ej: SincronizaNTP)
        addExternal0(DOW_TODOS, 18, 35, 0, SincronizaNTP);
    */

    // ===== MISAS DOMINICALES =====
    addExternal(DOW_DOMINGO, 11, 5,  0, accionSecuencia, EstadoMisa, true);
    addExternal(DOW_DOMINGO, 11, 25, 0, accionSecuencia, EstadoMisa, true);

    // ===== CUARTOS Y MEDIAS (FUNCIONALIDAD MIGRADA DE TIMEMANAGER) =====
    // Tocar horas en punto (wildcards = todas las horas)
    addExternal0(DOW_TODOS, ALARMA_WILDCARD, 0, 0, accionTocaHora, true);
    
    // Tocar medias horas
    addExternal0(DOW_TODOS, ALARMA_WILDCARD, 30, 0, accionTocaMedia, true);

    // ===== FUNCIONES EXTERNAS =====
    // Sincronización NTP al mediodía
    addExternal0(DOW_TODOS, 12, 2, 0, SincronizaNTP, true);
    addExternal0(DOW_TODOS, 12, 5, 0, ActualizaDNSSiNecesario, true);

    }
};

#endif
