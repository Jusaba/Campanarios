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
        constexpr int SDA_PIN = 21;                     // Pin SDA del M5Dial
        constexpr int SCL_PIN = 22;                     // Pin SCL del M5Dial
        constexpr int FREQUENCY = 100000;               // Frecuencia I2C (100kHz)
        constexpr int TIMEOUT_MS = 1000;                // Timeout comunicación I2C
    }
    
    // ============================================================================
    // ESTADOS - Sincronizados con Config::States de ESP32 Campanarios
    // ============================================================================
    namespace States {
        // Estados I2C (comandos que envía M5Dial → ESP32)
        constexpr uint8_t DIFUNTOS = 3;                 // Toque de difuntos
        constexpr uint8_t MISA = 4;                     // Toque de misa
        constexpr uint8_t STOP = 5;                     // Parar campanadas
        constexpr uint8_t CALEFACCION_ON = 6;           // Encender calefacción
        constexpr uint8_t CALEFACCION_OFF = 7;          // Apagar calefacción
        constexpr uint8_t SET_TEMPORIZADOR = 14;        // Configurar temporizador
        
        // Estados de solicitud de información (M5Dial solicita → ESP32)
        constexpr uint8_t CAMPANARIO = 8;               // Solicitar estado campanario
        constexpr uint8_t HORA = 9;                     // Solicitar hora actual
        constexpr uint8_t FECHA_HORA = 10;              // Solicitar fecha y hora
        constexpr uint8_t FECHA_HORA_O_TEMPORIZACION = 11;  // Solicitar fecha/hora o temporizador
        constexpr uint8_t TEMPORIZACION = 12;           // Solicitar tiempo restante temporizador
        constexpr uint8_t FIN = 13;                     // Fin de transmisión
        
        // Bits de estado campanario (respuestas ESP32 → M5Dial)
        constexpr uint8_t BIT_DIFUNTOS = 0x01;          // bit 0 - Difuntos activo
        constexpr uint8_t BIT_MISA = 0x02;              // bit 1 - Misa activa
        constexpr uint8_t BIT_HORA = 0x04;              // bit 2 - Toque de hora
        constexpr uint8_t BIT_CUARTOS = 0x08;           // bit 3 - Toque de cuartos
        constexpr uint8_t BIT_CALEFACCION = 0x10;       // bit 4 - Calefacción activa
        constexpr uint8_t BIT_SIN_INTERNET = 0x20;      // bit 5 - Sin internet
        constexpr uint8_t BIT_PROTECCION_CAMPANADAS = 0x40;  // bit 6 - Protección activa
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
        
        // Brillo
        constexpr uint8_t BRILLO_MAXIMO = 127;          // Brillo máximo pantalla
        constexpr uint8_t BRILLO_MINIMO = 0;            // Brillo mínimo pantalla
        constexpr uint8_t DELAY_BRILLO_MS = 20;         // Delay entre cambios de brillo
        
        // Dimensiones sprites
        constexpr int SPRITE_WIDTH = 120;               // Ancho sprites menú
        constexpr int SPRITE_HEIGHT = 120;              // Alto sprites menú
        constexpr int MAX_SPRITES = 10;                 // Máximo número de sprites
    }
    
    // ============================================================================
    // INDICES - Posiciones en array de sprites
    // ============================================================================
    namespace SpriteIndex {
        constexpr int STOP = 0;                         // Posición sprite Stop
        constexpr int DIFUNTOS = 1;                     // Posición sprite Difuntos
        constexpr int MISA = 2;                         // Posición sprite Misa
        constexpr int CALEFACCION_ON = 3;               // Posición sprite Calefacción On
        constexpr int CALEFACCION_OFF = 4;              // Posición sprite Calefacción Off
        constexpr int CAMPANARIO = 5;                   // Posición sprite Campanario
        constexpr int NO_INTERNET = 6;                  // Posición sprite No Internet
        constexpr int CALEFACCION_TEMP = 7;             // Posición sprite Calefacción Temporizada
        // Espacio para nuevas opciones:
        constexpr int NUEVA_OPCION = 8;                 // Próxima opción a añadir
    }
    
    // ============================================================================
    // TIMING - Configuración de temporización
    // ============================================================================
    namespace Timing {
        constexpr unsigned long UPDATE_INTERVAL_MS = 1000;     // Intervalo actualización pantalla
        constexpr unsigned long I2C_REQUEST_INTERVAL_MS = 5000; // Intervalo solicitudes I2C
        constexpr unsigned long BUTTON_DEBOUNCE_MS = 50;       // Debounce botones
        constexpr unsigned long IDLE_TIMEOUT_MS = 30000;       // Timeout para volver a pantalla inicial
    }
}

#endif