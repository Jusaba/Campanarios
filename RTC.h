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
    #include <WiFi.h>
    #include <time.h>

    // Configuración por defecto para servidores NTP
    #define NTP_SERVER "pool.ntp.org"
    #define GMT_OFFSET_SEC 3600                                 // Offset horario en segundos (por ejemplo, -5*3600 para GMT-5)
    #define DAYLIGHT_OFFSET_SEC 3600                            // Horario de verano, si aplica

    class RTC {
        public:
            static void begin(const char* ntpServer = NTP_SERVER, 
                          long gmtOffsetSec = GMT_OFFSET_SEC, 
                          int daylightOffsetSec = DAYLIGHT_OFFSET_SEC, 
                          unsigned long timeout_ms = 10000);
            static bool isNtpSync();
            static String getTimeStr();

        private:
            static String timeToString(const struct tm& timeinfo);
            static bool ntpSyncOk;  // SOLO declaración
    };

#endif // RTC_H
