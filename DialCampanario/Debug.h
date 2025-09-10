#ifndef DEBUG_H
    #define DEBUG_H

#include <Arduino.h>

// ============================================================================
// CONFIGURACIÓN DE DEBUG - Comentar/descomentar según necesidades
// ============================================================================

// Debug general del sistema
#define DEBUG                       // Debug general del sistema

// Debug específico por módulos
//#define DEBUGI2C                    // Debug comunicación I2C con ESP32
//#define DEBUGDISPLAY                // Debug funciones de pantalla y sprites
#define DEBUGBUTTONS                // Debug eventos de botones y encoder
#define DEBUGMENU                   // Debug navegación de menús
//#define DEBUGAUXILIAR               // Debug funciones auxiliares
#define DEBUGENCODER                 // Debug manejo del encoder
//#define DEBUGTIMING               // Debug temporizaciones y delays
//#define DEBUGSTARTUP              // Debug proceso de inicialización
//#define DEBUGSPRITES              // Debug carga y gestión de sprites
//#define DEBUGSTATES               // Debug cambios de estado del sistema

// ============================================================================
// MACROS DE DEBUG - Simplifica el uso de debug condicional
// ============================================================================

#ifdef DEBUG
    #define DBG(msg) Serial.println(String("[DEBUG] ") + msg)
    #define DBG_PRINT(msg) Serial.print(String("[DEBUG] ") + msg)
    #define DBG_PRINTF(fmt, ...) Serial.printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG(msg)
    #define DBG_PRINT(msg)
    #define DBG_PRINTF(fmt, ...)
#endif

#ifdef DEBUGI2C
    #define DBG_I2C(msg) Serial.println(String("[I2C] ") + msg)
    #define DBG_I2C_PRINT(msg) Serial.print(String("[I2C] ") + msg)
    #define DBG_I2C_PRINTF(fmt, ...) Serial.printf("[I2C] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_I2C(msg)
    #define DBG_I2C_PRINT(msg)
    #define DBG_I2C_PRINTF(fmt, ...)
#endif

#ifdef DEBUGDISPLAY
    #define DBG_DISPLAY(msg) Serial.println(String("[DISPLAY] ") + msg)
    #define DBG_DISPLAY_PRINT(msg) Serial.print(String("[DISPLAY] ") + msg)
    #define DBG_DISPLAY_PRINTF(fmt, ...) Serial.printf("[DISPLAY] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_DISPLAY(msg)
    #define DBG_DISPLAY_PRINT(msg)
    #define DBG_DISPLAY_PRINTF(fmt, ...)
#endif

#ifdef DEBUGBUTTONS
    #define DBG_BUTTONS(msg) Serial.println(String("[BUTTONS] ") + msg)
    #define DBG_BUTTONS_PRINT(msg) Serial.print(String("[BUTTONS] ") + msg)
    #define DBG_BUTTONS_PRINTF(fmt, ...) Serial.printf("[BUTTONS] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_BUTTONS(msg)
    #define DBG_BUTTONS_PRINT(msg)
    #define DBG_BUTTONS_PRINTF(fmt, ...)
#endif

#ifdef DEBUGMENU
    #define DBG_MENU(msg) Serial.println(String("[MENU] ") + msg)
    #define DBG_MENU_PRINT(msg) Serial.print(String("[MENU] ") + msg)
    #define DBG_MENU_PRINTF(fmt, ...) Serial.printf("[MENU] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_MENU(msg)
    #define DBG_MENU_PRINT(msg)
    #define DBG_MENU_PRINTF(fmt, ...)
#endif

#ifdef DEBUGTIMING
    #define DBG_TIMING(msg) Serial.println(String("[TIMING] ") + msg)
    #define DBG_TIMING_PRINT(msg) Serial.print(String("[TIMING] ") + msg)
    #define DBG_TIMING_PRINTF(fmt, ...) Serial.printf("[TIMING] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_TIMING(msg)
    #define DBG_TIMING_PRINT(msg)
    #define DBG_TIMING_PRINTF(fmt, ...)
#endif

#ifdef DEBUGSTARTUP
    #define DBG_STARTUP(msg) Serial.println(String("[STARTUP] ") + msg)
    #define DBG_STARTUP_PRINT(msg) Serial.print(String("[STARTUP] ") + msg)
    #define DBG_STARTUP_PRINTF(fmt, ...) Serial.printf("[STARTUP] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_STARTUP(msg)
    #define DBG_STARTUP_PRINT(msg)
    #define DBG_STARTUP_PRINTF(fmt, ...)
#endif

#ifdef DEBUGSPRITES
    #define DBG_SPRITES(msg) Serial.println(String("[SPRITES] ") + msg)
    #define DBG_SPRITES_PRINT(msg) Serial.print(String("[SPRITES] ") + msg)
    #define DBG_SPRITES_PRINTF(fmt, ...) Serial.printf("[SPRITES] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_SPRITES(msg)
    #define DBG_SPRITES_PRINT(msg)
    #define DBG_SPRITES_PRINTF(fmt, ...)
#endif

#ifdef DEBUGSTATES
    #define DBG_STATES(msg) Serial.println(String("[STATES] ") + msg)
    #define DBG_STATES_PRINT(msg) Serial.print(String("[STATES] ") + msg)
    #define DBG_STATES_PRINTF(fmt, ...) Serial.printf("[STATES] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_STATES(msg)
    #define DBG_STATES_PRINT(msg)
    #define DBG_STATES_PRINTF(fmt, ...)
#endif
#ifdef DEBUGAUXILIAR
    #define DBG_AUXILIAR(msg) Serial.println(String("[AUXILIAR] ") + msg)
    #define DBG_AUXILIAR_PRINT(msg) Serial.print(String("[AUXILIAR] ") + msg)
    #define DBG_AUXILIAR_PRINTF(fmt, ...) Serial.printf("[AUXILIAR] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_AUXILIAR(msg)
    #define DBG_AUXILIAR_PRINT(msg)
    #define DBG_AUXILIAR_PRINTF(fmt, ...)
#endif
#ifdef DEBUGENCODER
    #define DBG_ENCODER(msg) Serial.println(String("[ENCODER] ") + msg)
    #define DBG_ENCODER_PRINT(msg) Serial.print(String("[ENCODER] ") + msg)
    #define DBG_ENCODER_PRINTF(fmt, ...) Serial.printf("[ENCODER] " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG_ENCODER(msg)
    #define DBG_ENCODER_PRINT(msg)
    #define DBG_ENCODER_PRINTF(fmt, ...)
#endif    
#endif