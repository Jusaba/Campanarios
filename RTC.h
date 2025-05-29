#ifndef RTC_H
#define RTC_H

#include <WiFi.h>
#include <time.h>

// Configuración por defecto para servidores NTP
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 3600      // Offset horario en segundos (por ejemplo, -5*3600 para GMT-5)
#define DAYLIGHT_OFFSET_SEC 3600 // Horario de verano, si aplica

class RTC {
public:
    static void begin(const char* ntpServer = NTP_SERVER, long gmtOffsetSec = GMT_OFFSET_SEC, int daylightOffsetSec = DAYLIGHT_OFFSET_SEC) {
        configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
        Serial.println("Sincronizando hora con NTP...");

        struct tm timeinfo;
        while (!getLocalTime(&timeinfo)) {
            Serial.println("Esperando sincronización NTP...");
            delay(1000);
        }

        Serial.println("Hora sincronizada correctamente:");
        Serial.println(timeToString(timeinfo).c_str());
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
};

#endif // RTC_H
