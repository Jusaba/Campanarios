// Config.h
#ifndef CONFIGURACION_H
#define CONFIGURACION_H

#include <stdint.h>
#include <Arduino.h>  

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
            constexpr int NOCHE_FIN_HORA   = 8;     // Hora Fin Horario Nocturno

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
            constexpr unsigned long INTERNET_CHECK_INTERVAL_MS = 1UL * 60UL * 1000UL;  // 1 minuto

            // Timeouts
            constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 10000;  // 10 segundos
        }

        namespace Telegram {
            // Configuración del bot (debes configurar estos valores)
            const String BOT_TOKEN = "7031269795:AAFJZVP9isGip5eICQNh_BuJ8WzHCxtOGNE";      // Token del bot de Telegram
            //const String CHAT_ID = "106299125";      
            const String CHAT_ID = "-4948304348"  ;                                        // ID del chat autorizado
            
            constexpr int METODO_ACTIVACION_MANUAL = 0;                                     
            constexpr int METODO_ACTIVACION_WEB = 1;
            constexpr int METODO_ACTIVACION_ALARMA_PROGRAMADA = 2;
            constexpr int METODO_ACTIVACION_SISTEMA = 3;
            constexpr int METODO_ACTIVACION_OFF = 9;

            
            // Intervalos de verificación
            constexpr unsigned long CHECK_MESSAGES_INTERVAL_MS = 2000;      // Verificar cada 2 segundos
            constexpr unsigned long NOTIFICATION_COOLDOWN_MS = 30000;       // Cooldown entre notificaciones
            
            // Configuración de mensajes
            constexpr int MAX_MESSAGE_LENGTH = 4096;                        // Longitud máxima mensaje Telegram
            constexpr bool ENABLE_SILENT_NOTIFICATIONS = true;              // Notificaciones silenciosas por defecto

            // ✅ Variables de configuración dinámica (cargadas desde SPIFFS)
            inline String CAMPANARIO_NOMBRE = "";              // Nombre del campanario (configurable vía web)
            inline String CAMPANARIO_ID = "";                 // ID del campanario (configurable vía web)
            inline String CAMPANARIO_UBICACION = "";           // Ubicación del campanario (configurable vía web)
 
            // ✅ Flags de notificaciones (configurables vía web)
            inline bool NOTIFICACION_START = false;                          // Notificar al iniciar el sistema
            inline bool NOTIFICACION_ERRORES = false;                       // Notificar errores críticos
            inline bool NOTIFICACION_ALARMAS = false;                        // Notificar creacion de alarmas de campanario
            inline bool NOTIFICACION_CALEFACCION_ON = false;                 // Notificar activación de calefacción On
            inline bool NOTIFICACION_CALEFACCION_OFF = false;               // Notificar desactivación de calefacción Off
            inline bool NOTIFICACION_MISA = false;                           // Notificar ejecución de secuencias Misa
            inline bool NOTIFICACION_DIFUNTOS = false;                       // Notificar ejecución de secuencias Difuntos
            inline bool NOTIFICACION_FIESTA = false;                         // Notificar ejecución de secuencias Fiesta
            inline bool NOTIFICACION_STOP = false;                           // Notificar parada de secuencias
            inline bool NOTIFICACION_INTERNET_RECONEXION = false;           // Notificar reconexión a Internet
            inline bool NOTIFICACION_HORA = false;                          // Notificar toques de hora
            inline bool NOTIFICACION_MEDIAHORA = false;                     // Notificar toques de cuartos
            inline bool NOTIFICACION_NTP_SYNC = false;                      // Notificar sincronización NTP exitosa
            inline bool NOTIFICACION_DNS_UPDATE = false;                    // Notificar actualización DNS dinámica
            inline bool NOTIFICACION_ALARMA_PROGRAMADA = false;             // Notificar ejecución de alarmas programadas
        }

        // ==================== OTA ====================
        namespace OTA {
            // Versión del firmware (debe coincidir con GitHub Release)
            inline String FIRMWARE_VERSION = "1.5.4";
            
            // Configuración GitHub
            inline String GITHUB_OWNER = "Jusaba";
            inline String GITHUB_REPO = "Campanarios";
            
            // URLs (se construyen en runtime)
            inline String getGitHubApiUrl() {
                return "https://api.github.com/repos/" + GITHUB_OWNER + "/" + GITHUB_REPO + "/releases/latest";
            }
            
            inline String getGitHubReleaseBaseUrl() {
                return "https://github.com/" + GITHUB_OWNER + "/" + GITHUB_REPO + "/releases/download/";
            }
            
            // Tamaños máximos (esquema 'default')
            constexpr size_t MAX_FIRMWARE_SIZE = 1310720;  // 1.25 MB
            constexpr size_t MAX_SPIFFS_SIZE = 1441792;    // ~1.4 MB (0x160000)
            
            // Configuración de actualización automática
            constexpr unsigned long AUTO_UPDATE_INTERVAL_HOURS = 24;  // Comprobar cada 24 horas
            constexpr unsigned long UPDATE_CHECK_TIMEOUT_MS = 10000;  // Timeout de 10 segundos
            
            // Auto-update habilitado por defecto
            inline bool AUTO_UPDATE_ENABLED = false;  // Deshabilitado por seguridad, activar vía web
        }

        // ==================== ESTADOS I2C ====================
        namespace States {
            enum I2CState : uint8_t {
                DIFUNTOS                    = 1,
                MISA                        = 2,
                FIESTA                      = 3,
                STOP                        = 4,
                CALEFACCION_ON              = 5,
                CALEFACCION_OFF             = 6,
                CAMPANARIO                  = 7,
                HORA                        = 8,
                SIN_INTERNET                = 9,
                PROTECCION_CAMPANADAS       = 10,
                FECHA_HORA                  = 11,
                SET_TEMPORIZADOR            = 12,
                FIN                         = 13,
                FECHA_HORA_O_TEMPORIZACION  = 14,
                TEMPORIZACION               = 15,
                SECUENCIA_ACTIVA            = 16
            };

            // Bits de estado
            constexpr uint8_t BIT_SECUENCIA                   = 0x01;       // bit 0
            constexpr uint8_t BIT_LIBRE_1                     = 0x02;       // bit 1
            constexpr uint8_t BIT_LIBRE_2                     = 0x04;       // bit 2
            constexpr uint8_t BIT_HORA                        = 0x08;       // bit 3
            constexpr uint8_t BIT_CUARTOS                     = 0x10;       // bit 4
            constexpr uint8_t BIT_CALEFACCION                 = 0x20;       // bit 5
            constexpr uint8_t BIT_SIN_INTERNET                = 0x40;       // bit 6
            constexpr uint8_t BIT_PROTECCION_CAMPANADAS       = 0x80;       // bit 7
        }
        // ==================== SECUENCIAS ====================
        namespace Secuencia {
            constexpr uint8_t NINGUNA       = 0;
            constexpr uint8_t DIFUNTOS      = 1;
            constexpr uint8_t MISA          = 2;
            constexpr uint8_t FIESTA        = 3;
            constexpr uint8_t HORA          = 4;
            constexpr uint8_t CUARTOS       = 5;
            constexpr uint8_t PERSONALIZADA = 6;
            // ... futuras secuencias
        }        
        // ==================== CALEFACCIÓN ====================
        namespace Heating {
            constexpr int DEFAULT_TIMER_MINUTES = 45;  // Era nTemporizacionCalefaccion por defecto
            constexpr int MAX_MINUTES           = 300; // Máximo minutos de calefacción (5 horas)
        }

        // ==================== DEBUG ====================
        namespace Debug {
            // Flags de debug (puedes activar/desactivar módulos específicos)
            constexpr bool AUXILIAR_DEBUG    = true;   // Era #ifdef DEBUGAUXILIAR
            constexpr bool PROTECTION_DEBUG  = true;   // Era #ifdef DEBUGPROTECCION  
            constexpr bool SERVER_DEBUG      = true;   // Era #ifdef DEBUGSERVIDOR
            constexpr bool GENERAL_DEBUG     = true;   // Era #ifdef DEBUG
        }
        // ==================== CAMPANARIO ====================
        namespace Campanario {
            constexpr int MAX_CAMPANAS = 2;  // Número máximo de campanas en el campanario
        }
        // ==================== ALARMAS ====================
        namespace Alarmas {
            constexpr int MAX_ALARMAS = 5;  // Número máximo de alarmas
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