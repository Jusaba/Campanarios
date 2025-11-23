// Debug.h - Sistema centralizado de debug
#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

// ============================================================================
// CONFIGURACIÓN DE DEBUG - Comentar/descomentar según necesidades
// ============================================================================

// Debug general del sistema
#define DEBUG                       // Debug general del sistema

// Debug específico por módulos
//#define DEBUGI2C                    // Debug general del módulo I2C
//#define DEBUGI2CREQUEST             // Debug de requests I2C (envío/recepción)
//#define DEBUGI2CSECUENCIA           // Debug de secuencias I2C recibidas

#define DEBUGINO                    // Debug del módulo INO

//#define DEBUGAUXILIAR             // Debug del módulo Auxiliar
//#define DEBUGPROTECCION             // Debug de protección de campanadas
//#define DEBUGACCIONES             // Debug de las acciones de alarmas
 #define DEBUGSERVIDOR              // Debug del servidor web
 #define DEBUGDNS                 // Debug del servicio DNS
 #define DEBUGWIFI                // Debug de conexión WiFi
 #define DEBUGALARMAS               // Debug del sistema de alarmas
 #define DEBUGCAMPANARIO          // Debug del campanario
// #define DEBUGRTC                 // Debug del RTC
// #define DEBUGAP                  // Debug del modo AP
// #define DEBUGCALEFACCION         // Debug del sistema de calefacción
// #define DEBUGCAMPANA             // Debug del sistema de campanas
 #define DEBUGTELEGRAM            // Debug del servicio Telegram
#define DBG_ALARMS_ENABLED          // Habilita macros de debug para alarmas personalizadas

// ============================================================================
// MACROS DE DEBUG - Simplifica el uso de debug condicional
// ============================================================================

// Macro para debug general
#ifdef DEBUG
    #define DBG(msg) Serial.println(String("[DEBUG] ") + msg)
    #define DBG_PRINT(msg) Serial.print(String("[DEBUG] ") + msg)
    #define DBG_PRINTF(fmt, ...) Serial.printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG(msg)
    #define DBG_PRINT(msg)
    #define DBG_PRINTF(fmt, ...)
#endif

// Macros para debug específico de I2C
#ifdef DEBUGI2C
    #define DBG_I2C(msg) Serial.println(String("[I2C] ") + msg)
    #define DBG_I2C_PRINTF(fmt, ...) Serial.printf("[I2C] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_I2C(msg)
    #define DBG_I2C_PRINTF(fmt, ...)
#endif

#ifdef DEBUGI2CREQUEST
    #define DBG_I2C_REQ(msg) Serial.println(String("[I2C-REQ] ") + msg)
    #define DBG_I2C_REQ_PRINTF(fmt, ...) Serial.printf("[I2C-REQ] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_I2C_REQ(msg)
    #define DBG_I2C_REQ_PRINTF(fmt, ...)
#endif

#ifdef DEBUGI2CSECUENCIA
    #define DBG_I2C_SEQ(msg) Serial.println(String("[I2C-SEQ] ") + msg)
    #define DBG_I2C_SEQ_PRINTF(fmt, ...) Serial.printf("[I2C-SEQ] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_I2C_SEQ(msg)
    #define DBG_I2C_SEQ_PRINTF(fmt, ...)
#endif

// Macros para debug de Auxiliar
#ifdef DEBUGAUXILIAR
    #define DBG_AUX(msg) Serial.println(String("[AUX] ") + msg)
    #define DBG_AUX_PRINTF(fmt, ...) Serial.printf("[AUX] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_AUX(msg)
    #define DBG_AUX_PRINTF(fmt, ...)
#endif

// Macros para debug de protección
#ifdef DEBUGPROTECCION
    #define DBG_PROT(msg) Serial.println(String("[PROT] ") + msg)
    #define DBG_PROT_PRINTF(fmt, ...) Serial.printf("[PROT] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_PROT(msg)
    #define DBG_PROT_PRINTF(fmt, ...)
#endif

// Macros para debug de acciones
#ifdef DEBUGACCIONES
    #define DBG_ACCIONES(msg) Serial.println(String("[ACCIONES] ") + msg)
    #define DBG_ACCIONES_PRINT(msg) Serial.print(String("[ACCIONES] ") + msg)
    #define DBG_ACCIONES_PRINTF(fmt, ...) Serial.printf("[ACCIONES] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_ACCIONES(msg)
    #define DBG_ACCIONES_PRINT(msg)
    #define DBG_ACCIONES_PRINTF(fmt, ...)
#endif

// Macros para debug de servidor
#ifdef DEBUGSERVIDOR
    #define DBG_SRV(msg) Serial.println(String("[SRV] ") + msg)
    #define DBG_SRV_PRINTF(fmt, ...) Serial.printf("[SRV] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_SRV(msg)
    #define DBG_SRV_PRINTF(fmt, ...)
#endif

// Macros para debug de DNS
#ifdef DEBUGDNS
    #define DBG_DNS(msg) Serial.println(String("[DNS] ") + msg)
    #define DBG_DNS_PRINT(msg) Serial.print(String("[DNS] ") + msg)
    #define DBG_DNS_PRINTF(fmt, ...) Serial.printf("[DNS] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_DNS(msg)
    #define DBG_DNS_PRINT (msg)
    #define DBG_DNS_PRINTF(fmt, ...)
#endif

// Macros para debug de WiFi
#ifdef DEBUGWIFI
    #define DBG_WIFI(msg) Serial.println(String("[WIFI] ") + msg)
    #define DBG_WIFI_PRINT(msg) Serial.print(String("[WIFI] ") + msg)
    #define DBG_WIFI_PRINTF(fmt, ...) Serial.printf("[WIFI] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_WIFI(msg)
    #define DBG_WIFI_PRINT(msg)
    #define DBG_WIFI_PRINTF(fmt, ...)
#endif

// Macros para debug de alarmas
#ifdef DEBUGALARMAS
    #define DBG_ALM(msg) Serial.println(String("[ALM] ") + msg)
    #define DBG_ALM_PRINTF(fmt, ...) Serial.printf("[ALM] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_ALM(msg)
    #define DBG_ALM_PRINTF(fmt, ...)
#endif

// Macros para debug de campanario
#ifdef DEBUGCAMPANARIO
    #define DBG_CAM(msg) Serial.println(String("[CAMPANARIO] ") + msg)
    #define DBG_CAM_PRINT(msg) Serial.print(String("[CAMPANARIO] ") + msg)
    #define DBG_CAM_PRINTF(fmt, ...) Serial.printf("[CAMPANARIO] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_CAM(msg)
    #define DBG_CAM_PRINT(msg)
    #define DBG_CAM_PRINTF(fmt, ...)
#endif

// Macros para debug de RTC
#ifdef DEBUGRTC
    #define DBG_RTC(msg) Serial.println(String("[RTC] ") + msg)
    #define DBG_RTC_PRINT(msg) Serial.print(String("[RTC] ") + msg)
    #define DBG_RTC_PRINTF(fmt, ...) Serial.printf("[RTC] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_RTC(msg)
    #define DBG_RTC_PRINT(msg)
    #define DBG_RTC_PRINTF(fmt, ...)
#endif

//Macros para debug de INO
#ifdef DEBUGINO
    #define DBG_INO(msg) Serial.println(String("[INO] ") + msg)
    #define DBG_INO_PRINT(msg) Serial.print(String("[INO] ") + msg)
    #define DBG_INO_PRINTF(fmt, ...) Serial.printf("[INO] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_INO(msg)
    #define DBG_INO_PRINT(msg)
    #define DBG_INO_PRINTF(fmt, ...)
#endif

//Macros para debug de AP
#ifdef DEBUGAP
    #define DBG_AP(msg) Serial.println(String("[AP] ") + msg)
    #define DBG_AP_PRINT(msg) Serial.print(String("[AP] ") + msg)
    #define DBG_AP_PRINTF(fmt, ...) Serial.printf("[AP] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_AP(msg)
    #define DBG_AP_PRINT(msg)
    #define DBG_AP_PRINTF(fmt, ...)
#endif

//Macros para debug de calefacción
#ifdef DEBUGCALEFACCION
    #define DBG_CALEFACCION(msg) Serial.println(String("[CALEFACCION] ") + msg)
    #define DBG_CALEFACCION_PRINT(msg) Serial.print(String("[CALEFACCION] ") + msg)
    #define DBG_CALEFACCION_PRINTF(fmt, ...) Serial.printf("[CALEFACCION] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_CALEFACCION(msg)
    #define DBG_CALEFACCION_PRINT(msg)
    #define DBG_CALEFACCION_PRINTF(fmt, ...)
#endif

//Macros para debug de campanas
#ifdef DEBUGCAMPANA
    #define DBG_CAMPANA(msg) Serial.println(String("[CAMPANA] ") + msg)
    #define DBG_CAMPANA_PRINT(msg) Serial.print(String("[CAMPANA] ") + msg)
    #define DBG_CAMPANA_PRINTF(fmt, ...) Serial.printf("[CAMPANA] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_CAMPANA(msg)
    #define DBG_CAMPANA_PRINT(msg)
    #define DBG_CAMPANA_PRINTF(fmt, ...)
#endif

#ifdef DEBUGTELEGRAM
    #define DBG_TELEGRAM(msg) Serial.println(String("[TELEGRAM] ") + msg)
    #define DBG_TELEGRAM_PRINT(msg) Serial.print(String("[TELEGRAM] ") + msg)
    #define DBG_TELEGRAM_PRINTF(fmt, ...) Serial.printf("[TELEGRAM] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_TELEGRAM(msg)
    #define DBG_TELEGRAM_PRINT(msg)
    #define DBG_TELEGRAM_PRINTF(fmt, ...)
#endif

#ifdef DBG_ALARMS_ENABLED
    #define DBG_ALARMS(msg) Serial.println("[ALARMS] " + String(msg))
    #define DBG_ALARMS_PRINTF(format, ...) Serial.printf("[ALARMS] " format "\n", ##__VA_ARGS__)
#else
    #define DBG_ALARMS(msg)
    #define DBG_ALARMS_PRINTF(format, ...)
#endif

// ============================================================================
// FUNCIONES DE DEBUG AVANZADAS (Opcional)
// ============================================================================

class DebugHelper {
public:
    // Función para mostrar memoria libre
    static void printFreeHeap() {
        #ifdef DEBUG
        Serial.printf("[DEBUG] Memoria libre: %d bytes\n", ESP.getFreeHeap());
        #endif
    }
    
    // Función para mostrar información del sistema
    static void printSystemInfo() {
        #ifdef DEBUG
        Serial.println("[DEBUG] === INFORMACIÓN DEL SISTEMA ===");
        Serial.printf("[DEBUG] CPU Freq: %d MHz\n", ESP.getCpuFreqMHz());
        Serial.printf("[DEBUG] Flash Size: %d bytes\n", ESP.getFlashChipSize());
        Serial.printf("[DEBUG] Free Heap: %d bytes\n", ESP.getFreeHeap());
        Serial.printf("[DEBUG] Chip ID: %08X\n", (uint32_t)ESP.getEfuseMac());
        Serial.println("[DEBUG] ================================");
        #endif
    }
    
    // Función para debug de arrays
    template<typename T>
    static void printArray(const char* name, T* array, size_t size) {
        #ifdef DEBUG
        Serial.printf("[DEBUG] Array %s [%d]: ", name, size);
        for (size_t i = 0; i < size; i++) {
            Serial.printf("%d ", array[i]);
        }
        Serial.println();
        #endif
    }
};

#endif