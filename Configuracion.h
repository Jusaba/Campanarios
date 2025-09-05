// Config.h
#ifndef CONFIGURACION_H
#define CONFIGURACION_H

#include <stdint.h>

    namespace Config {

        // ==================== HARDWARE ====================
        namespace Pins {
            constexpr int CAMPANA1       = 26;
            constexpr int CAMPANA2       = 25;
            constexpr int CALEFACCION    = 33;
            constexpr int RELE4          = 32;
            constexpr int CONFIGURACION  = 19;
        }

        namespace I2C {
            constexpr uint8_t SLAVE_ADDR = 0x12;
            constexpr int SDA_PIN        = 21;
            constexpr int SCL_PIN        = 22;
        }

        // ==================== TIMING ====================
        namespace Time {
            // Horario nocturno (campanas silenciadas)
            constexpr int NOCHE_INICIO_HORA = 23;   // Hora Inicio Horario Nocturno
            constexpr int NOCHE_FIN_HORA   = 7;     // Hora Fin Horario Nocturno

            // Protección de campanadas (±X minutos alrededor de toques)
            constexpr int MARGEN_PROTECCION_VENTANA = 3;   // ±3 min = ventana de 6 min
            constexpr int MINUTO_HORA_CAMPANADA     = 0;   // Minuto de toque de hora
            constexpr int MINUTO_MEDIA_CAMPANADA    = 30;  // Minuto de toque de media

            // ============================================================================
            // CONSTANTES CALCULADAS AUTOMÁTICAMENTE - Se recalculan si cambias las base
            // ============================================================================
                
            // Ventana de protección para HORA EN PUNTO (con wrap-around automático)
            constexpr int INICIO_VENTANA_HORA = (MINUTO_HORA_CAMPANADA + 60 - MARGEN_PROTECCION_VENTANA) % 60;  // 57
            constexpr int FIN_VENTANA_HORA = (MINUTO_HORA_CAMPANADA + MARGEN_PROTECCION_VENTANA) % 60;           // 3
                
            // Ventana de protección para MEDIA HORA (sin wrap-around necesario)
            constexpr int INICIO_VENTANA_MEDIA = MINUTO_MEDIA_CAMPANADA - MARGEN_PROTECCION_VENTANA;             // 27
            constexpr int FIN_VENTANA_MEDIA = MINUTO_MEDIA_CAMPANADA + MARGEN_PROTECCION_VENTANA;                // 33

            
            // Sincronización NTP
            constexpr int NTP_SYNC_HORA = 12;              // Hora de sincronización automática
        }

        namespace Network {
            // Intervalos de verificación
            constexpr unsigned long INTERNET_CHECK_INTERVAL_MS = 5UL * 60UL * 1000UL;  // 5 minutos

            // Timeouts
            constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 10000;  // 10 segundos
        }

        // ==================== ESTADOS I2C ====================
        namespace States {
            enum I2CState : uint8_t {
                DIFUNTOS                    = 1,
                MISA                        = 2,
                STOP                        = 3,
                CALEFACCION_ON              = 4,
                CALEFACCION_OFF             = 5,
                CAMPANARIO                  = 6,
                HORA                        = 7,
                SIN_INTERNET                = 8,
                PROTECCION_CAMPANADAS       = 9,
                FECHA_HORA                  = 10,
                SET_TEMPORIZADOR            = 11,
                FIN                         = 12,
                FECHA_HORA_O_TEMPORIZACION  = 13,
                TEMPORIZACION               = 14
            };

            // Bits de estado
            constexpr uint8_t BIT_DIFUNTOS                    = 0x01;       // bit 0
            constexpr uint8_t BIT_MISA                        = 0x02;       // bit 1
            constexpr uint8_t BIT_HORA                        = 0x04;       // bit 2
            constexpr uint8_t BIT_CUARTOS                     = 0x08;       // bit 3
            constexpr uint8_t BIT_CALEFACCION                 = 0x10;       // bit 4
            constexpr uint8_t BIT_SIN_INTERNET                = 0x20;       // bit 5
            constexpr uint8_t BIT_PROTECCION_CAMPANADAS       = 0x40;       // bit 6

            constexpr uint8_t BIT_CALEFACCION_ACTIVA     = (1 << 4);  // Para bit 4
        }

        // ==================== CALEFACCIÓN ====================
        namespace Heating {
            constexpr int DEFAULT_TIMER_MINUTES = 45;  // Era nTemporizacionCalefaccion por defecto
            constexpr int MAX_MINUTES           = 120; // Máximo minutos de calefacción (2 horas)
        }

        // ==================== DEBUG ====================
        namespace Debug {
            // Flags de debug (puedes activar/desactivar módulos específicos)
            constexpr bool AUXILIAR_DEBUG    = true;   // Era #ifdef DEBUGAUXILIAR
            constexpr bool PROTECTION_DEBUG  = true;   // Era #ifdef DEBUGPROTECCION  
            constexpr bool SERVER_DEBUG      = true;   // Era #ifdef DEBUGSERVIDOR
            constexpr bool GENERAL_DEBUG     = true;   // Era #ifdef DEBUG
        }
    }

    // ==================== COMPATIBILITY (transición) ====================
    // Mantén estos #defines para compatibilidad mientras migras el código
/*
    #define PinCampana1         Config::Pins::CAMPANA1
    #define PinCampana2         Config::Pins::CAMPANA2
    #define CalefaccionPin      Config::Pins::CALEFACCION
    #define PinRele4            Config::Pins::RELE4
    #define PinConfiguracion    Config::Pins::CONFIGURACION
    
    #define I2C_SLAVE_ADDR      Config::I2C::SLAVE_ADDR
    #define SDA_PIN             Config::I2C::SDA_PIN
    #define SCL_PIN             Config::I2C::SCL_PIN

    #define InicioHorarioNocturno Config::Time::NOCHE_INICIO_HORA
    #define FinHorarioNocturno    Config::Time::NOCHE_FIN_HORA
*/    
/*
    #define EstadoDifuntos              Config::States::DIFUNTOS
    #define EstadoMisa                  Config::States::MISA
    #define EstadoStop                  Config::States::STOP
    #define EstadoCalefaccionOn         Config::States::CALEFACCION_ON
    #define EstadoCalefaccionOff        Config::States::CALEFACCION_OFF
    #define EstadoCampanario            Config::States::CAMPANARIO
    #define EstadoHora                  Config::States::HORA
    #define EstadoSinInternet           Config::States::SIN_INTERNET
    #define EstadoProteccionCampanadas  Config::States::PROTECCION_CAMPANADAS
    #define EstadoFechaHora             Config::States::FECHA_HORA
    #define EstadoSetTemporizador       Config::States::SET_TEMPORIZADOR
    #define EstadoFin                   Config::States::FIN
    #define EstadoFechaHoraoTemporizacion Config::States::FECHA_HORA_O_TEMPORIZACION
    #define EstadoTemporizacion         Config::States::TEMPORIZACION
    
    #define BitEstadoSinInternet        Config::States::BIT_SIN_INTERNET
    #define BitEstadoProteccionCampanadas Config::States::BIT_PROTECCION_CAMPANADAS
*/
#endif