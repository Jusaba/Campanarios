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
        DBG("Sincronizando hora con NTP en " + String(ntpServer) + ".");

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


    bool RTC::beginConMultiplesServidores(unsigned long timeout_ms) 
    {
        DBG("Iniciando sincronización NTP con múltiples servidores...");
    
        // Lista de servidores NTP (3 servidores máximo soportados por ESP32)
        const char* ntpServers[] = {
            NTP_SERVER1,
            NTP_SERVER2,
            NTP_SERVER3
        };
    
        // Configurar múltiples servidores NTP
        configTime(GMT_OFFSET_SEC, 
               DAYLIGHT_OFFSET_SEC, 
               ntpServers[0], 
               ntpServers[1], 
               ntpServers[2]);
    
        DBG("Servidores NTP configurados:");
        DBG("  - Servidor 1: " + String(ntpServers[0]));
        DBG("  - Servidor 2: " + String(ntpServers[1])); 
        DBG("  - Servidor 3: " + String(ntpServers[2]));
    
        struct tm timeinfo;
        unsigned long start = millis();
        int intentos = 0;
    
        while (millis() - start < timeout_ms) {
            if (getLocalTime(&timeinfo)) {
                // Validar que la fecha sea realista (después de 2020)
                if (ValidaFecha(timeinfo)) 
                {
                    ntpSyncOk = true;
                    DBG("✅ Sincronización NTP exitosa:");
                    DBG("   Fecha/Hora: " + timeToString(timeinfo));
                    DBG("   Servidor usado: " + String(ntpServers[intentos % 3]));
                    return true;
                } else {
                    DBG("⚠️ Fecha inválida recibida, reintentando...");
                }
            }
        
        
            intentos++;
            if (intentos % 3 == 0) {
                DBG("Reintentando sincronización... (intento " + String(intentos/3 + 1) + ")");
            }

            delay(1000);
        }
    
        DBG("❌ Timeout en sincronización NTP después de " + String(timeout_ms/1000) + " segundos");
        ntpSyncOk = false;
        return false;
    }

bool RTC::ValidaFecha(const struct tm& timeinfo) {
    // Verificar que el año sea razonable (después de 2020, antes de 2050)
    int year = timeinfo.tm_year + 1900;
    if (year < 2020 || year > 2050) {
        return false;
    }
    
    // Verificar mes válido
    if (timeinfo.tm_mon < 0 || timeinfo.tm_mon > 11) {
        return false;
    }
    
    // Verificar día válido
    if (timeinfo.tm_mday < 1 || timeinfo.tm_mday > 31) {
        return false;
    }
    
    return true;
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