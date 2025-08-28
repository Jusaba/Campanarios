// RTC.cpp - Implementaciones
#include "RTC.h"

#ifdef DEBUGRTC
  #define DBG(x) Serial.println(x)
#else
  #define DBG(x)
#endif

    bool RTC::ntpSyncOk = false;  // Única definición

    void RTC::begin(const char* ntpServer, long gmtOffsetSec, 
                int daylightOffsetSec, unsigned long timeout_ms) 
    {
        configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
        DBG("Sincronizando hora con NTP...");
    
        struct tm timeinfo;
        unsigned long start = millis();
        while (!getLocalTime(&timeinfo)) {
            if (millis() - start > timeout_ms) {
                DBG("Timeout esperando sincronización NTP.");
                break;
            }
            DBG("Esperando sincronización NTP...");
            delay(1000);
        }
    
        ntpSyncOk = getLocalTime(&timeinfo);
        if (ntpSyncOk) {
            DBG("Hora sincronizada correctamente:");
            DBG(timeToString(timeinfo));
        }
    }

    bool RTC::isNtpSync() {
        return ntpSyncOk;
    }

    String RTC::getTimeStr() {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            return "Error obteniendo hora";
        }
        return timeToString(timeinfo);
    }

    String RTC::timeToString(const struct tm& timeinfo) {
        char buffer[30];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
        return String(buffer);
    }