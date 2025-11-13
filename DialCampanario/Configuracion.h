#ifndef CONFIGURACION_H
#define CONFIGURACION_H
    #include <Arduino.h>  
    #include <stdint.h>

namespace Config {
    
    // ============================================================================
    // I2C - Configuración de comunicación con ESP32 Campanarios
    // ============================================================================
    namespace I2C {
        constexpr uint8_t SLAVE_ADDR = 0x12;            // Dirección I2C del ESP32 Campanarios
        constexpr int FREQUENCY = 100000;               // Frecuencia I2C (100kHz)
        constexpr int TIMEOUT_MS = 1000;                // Timeout comunicación I2C
    }
    
    // ============================================================================
    // ESTADOS - Sincronizados con Config::States de ESP32 Campanarios
    // ============================================================================
    namespace States {

        enum I2CState : uint8_t {
            INICIO                      = 0,
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
    // ============================================================================
    // DISPLAY - Configuración de pantalla M5Dial
    // ============================================================================
    namespace Display {
        // Colores
        constexpr uint16_t COLOR_FONDO = 0x0000;        // Negro
        constexpr uint16_t COLOR_INFO = 0xFFFF;         // Blanco
        constexpr uint16_t COLOR_VOLTIOS = 0xFFFF;      // Blanco
        constexpr uint16_t COLOR_SELECCION = 0xFD20;    // Naranja (TFT_ORANGE)
        constexpr uint16_t COLOR_TRANSPARENTE = 0x0002; // Transparente
        constexpr uint16_t COLOR_NOINTERNET = 0xF800;   // Rojo (TFT_RED)
        
        // Brillo
        constexpr uint8_t BRILLO_MAXIMO = 127;          // Brillo máximo pantalla
        constexpr uint8_t BRILLO_MINIMO = 0;            // Brillo mínimo pantalla
        constexpr uint8_t DELAY_BRILLO_MS = 20;         // Delay entre cambios de brillo
        
        // Dimensiones sprites
        constexpr int SPRITE_WIDTH = 120;               // Ancho sprites menú
        constexpr int SPRITE_HEIGHT = 120;              // Alto sprites menú
        constexpr int MAX_SPRITES = 9;                 // Máximo número de sprites

        constexpr int CICLOS_PARA_SLEEP = 30;           // Ciclos para poner el display en modo sleep 
    }
    
    // ============================================================================
    // INDICES - Posiciones en array de sprites
    // ============================================================================
    namespace SpriteIndex {
        constexpr int STOP = 0;                         // Posición sprite Stop
        constexpr int DIFUNTOS = 1;                     // Posición sprite Difuntos
        constexpr int MISA = 2;                         // Posición sprite Misa
        constexpr int FIESTA = 3;                       // Posición sprite Fiesta
        constexpr int CALEFACCION_ON = 4;               // Posición sprite Calefacción On
        constexpr int CALEFACCION_OFF = 5;              // Posición sprite Calefacción Off
        constexpr int CAMPANARIO = 6;                   // Posición sprite Campanario
        constexpr int NO_INTERNET = 7;                  // Posición sprite No Internet
        constexpr int CALEFACCION_TEMP = 8;             // Posición sprite Calefacción Temporizada
        // Espacio para nuevas opciones:
        constexpr int NUEVA_OPCION = 9;                 // Próxima opción a añadir
    }
    
    // ============================================================================
    // TIMING - Configuración de temporización
    // ============================================================================
    namespace Timing {
        constexpr unsigned long DELAY_INICIO_MS = 5000;                 // Tiempo de espera para que se inicie el modulo del campanario
        constexpr unsigned long UPDATE_INTERVAL_MS = 1000;              // Intervalo actualización pantalla
        constexpr unsigned long I2C_REQUEST_INTERVAL_MS = 500;          // Intervalo solicitudes I2C
        constexpr unsigned long BUTTON_DEBOUNCE_MS = 50;                // Debounce botones
        constexpr unsigned long IDLE_TIMEOUT_MS = 30000;                // Timeout para volver a pantalla inicial
        constexpr unsigned long I2C_COMMAND_PROCESSING_DELAY_MS = 300;  // Tiempo para que el esclavo procese comandos
    }
}

#endif