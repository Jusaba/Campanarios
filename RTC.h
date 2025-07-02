/**
 * @file RTC.h
 * @brief Clase utilitaria para la gestión del reloj en tiempo real (RTC) del ESP32 usando NTP.
 *
 * Esta clase permite sincronizar la hora del sistema del ESP32 con un servidor NTP y obtener la hora actual
 * en formato de cadena. Utiliza la funcionalidad RTC interna del ESP32, no requiere hardware adicional.
 *
 * - Sincroniza la hora con un servidor NTP configurable.
 * - Permite obtener la hora actual como String.
 * - Incluye opciones para zona horaria y horario de verano.
 *
 * Ejemplo de uso:
 * @code
 *   RTC::begin(); // Sincroniza la hora con NTP
 *   String hora = RTC::getTimeStr(); // Obtiene la hora actual como String
 * @endcode
 *
 * @author  Julian Salas Baertolome
 * @date    Junio/2025
 */
#ifndef RTC_H
    #define RTC_H

    #define DEBUGRTC

    #include <WiFi.h>
    #include <time.h>

    // Configuración por defecto para servidores NTP
    #define NTP_SERVER "pool.ntp.org"
    #define GMT_OFFSET_SEC 3600                                 // Offset horario en segundos (por ejemplo, -5*3600 para GMT-5)
    #define DAYLIGHT_OFFSET_SEC 3600                            // Horario de verano, si aplica

class RTC {
    public:
        static void begin(const char* ntpServer = NTP_SERVER, long gmtOffsetSec = GMT_OFFSET_SEC, int daylightOffsetSec = DAYLIGHT_OFFSET_SEC, unsigned long timeout_ms = 10000) {
            configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
            #ifdef DEBUGRTC
                Serial.println("Sincronizando hora con NTP...");
            #endif
            struct tm timeinfo;
            unsigned long start = millis();
            bool syncOk = false;
            while (!getLocalTime(&timeinfo)) {
                if (millis() - start > timeout_ms) {
                    #ifdef DEBUGRTC
                        Serial.println("Timeout esperando sincronización NTP.");
                    #endif
                    break;
                }
                #ifdef DEBUGRTC
                    Serial.println("Esperando sincronización NTP...");
                #endif
                delay(1000);
            }
            syncOk = getLocalTime(&timeinfo);
            RTC::ntpSyncOk = syncOk;
            #ifdef DEBUGRTC
                if (syncOk) {
                    Serial.println("Hora sincronizada correctamente:");
                    Serial.println(timeToString(timeinfo).c_str());
                }
            #endif
        }

        static bool isNtpSync() {
            return ntpSyncOk; // Devuelve el estado de sincronización NTP
        }

        static String getTimeStr() {
            struct tm timeinfo;
            if (!getLocalTime(&timeinfo)) {
                return "Error obteniendo hora";
            }
            return timeToString(timeinfo);
        }

    private:
        static String timeToString(const struct tm& timeinfo) {
            char buffer[30];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
            return String(buffer);
        }
        static bool ntpSyncOk; // Indica si la sincronización NTP fue exitosa
};

// Definición del miembro estático fuera de la clase
bool RTC::ntpSyncOk = false;

#endif // RTC_H
