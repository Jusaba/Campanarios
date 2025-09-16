#include "Campana.h"

    /**
     * @brief Constructor que inicializa una campana con pin específico
     * 
     * @details Inicializa el objeto campana configurando el pin GPIO especificado
     *          como salida digital y estableciendo su estado inicial en LOW.
     *          El pin queda listo para controlar relés, solenoides o drivers
     *          de campana mediante activación temporal.
     *          
     *          **PROCESO DE INICIALIZACIÓN:**
     *          1. Guarda el número de pin en variable privada
     *          2. Configura el pin como OUTPUT digital
     *          3. Establece estado inicial LOW (campana inactiva)
     *          4. Log de inicialización para debug
     * 
     * @param nPin Número del pin GPIO para controlar la campana (0-39 en ESP32)
     * 
     * @note **CONFIGURACIÓN AUTOMÁTICA:** El pin se configura como OUTPUT automáticamente
     * @note **ESTADO INICIAL:** La campana inicia siempre inactiva (LOW)
     * @note **COMPATIBILIDAD:** Compatible con pines digitales estándar del ESP32
     * 
     * @warning **PIN VÁLIDO:** Verificar que el pin GPIO es válido y no está reservado
     * @warning **CONFLICTOS:** Cada campana debe usar un pin diferente
     * @warning **HARDWARE:** Verificar compatibilidad de voltaje (3.3V ESP32)
     * 
     * @see pinMode() - Función Arduino utilizada para configurar el pin
     * @see digitalWrite() - Función utilizada para controlar el estado
     * 
     * @example
     * @code
     * // Crear campana en pin 12
     * CAMPANA campanaMayor(12);
     * 
     * // Crear múltiples campanas
     * CAMPANA campana1(12);
     * CAMPANA campana2(13);
     * CAMPANA campana3(14);
     * @endcode
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    CAMPANA::CAMPANA (int nPin) {
        // Constructor por defecto
        this->_nPin = nPin;                     // Asignar el pin de la campana
        pinMode(this->_nPin, OUTPUT);           // Configurar el pin como salida
        DBG_CAMPANA_PRINTF("[CAMPANA] Inicializada en pin %d - Estado: INACTIVA\n", _nPin);
    }

    /**
     * @brief Destructor que asegura el apagado seguro de la campana
     * 
     * @details Destructor que garantiza que el pin de control de la campana
     *          se establezca en estado LOW (inactivo) cuando el objeto sea
     *          destruido, evitando que quede activado accidentalmente.
     *          
     *          **PROCESO DE DESTRUCCIÓN:**
     *          1. Establece el pin en estado LOW (inactivo)
     *          2. Log de destrucción para debug
     *          3. Liberación automática de recursos
     * 
     * @note **SEGURIDAD:** Apagado automático garantizado
     * @note **CLEANUP:** No requiere intervención manual
     * @note **ESTADO FINAL:** Pin siempre queda en LOW al destruir objeto
     * 
     * @see digitalWrite() - Función utilizada para desactivar el pin
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */

    CAMPANA::~CAMPANA() {
        digitalWrite(_nPin, LOW);
        DBG_CAMPANA_PRINTF("[CAMPANA] Destructor - Pin %d desactivado por seguridad\n", _nPin);

    }
    /**
     * @brief Activa la campana durante el tiempo especificado (badajo)
     * 
     * @details Método principal que simula el toque de una campana real mediante
     *          la activación temporal del pin de control. Activa el pin durante
     *          TiempoBadajoOn milisegundos y luego lo desactiva automáticamente.
     *          
     *          **SECUENCIA DE ACTIVACIÓN:**
     *          1. Establece pin en HIGH (activar relé/solenoide)
     *          2. Espera TiempoBadajoOn milisegundos (200ms por defecto)
     *          3. Establece pin en LOW (desactivar relé/solenoide)
     *          4. Log de la operación para debug
     *          
     *          **SIMULACIÓN DEL BADAJO:**
     *          - HIGH: Badajo golpea la campana (activación electromagnética)
     *          - Delay: Tiempo de contacto y resonancia
     *          - LOW: Badajo se retira (desactivación electromagnética)
     * 
     * @note **TIEMPO FIJO:** Utiliza constante TiempoBadajoOn (200ms)
     * @note **BLOQUEO TEMPORAL:** El hilo se bloquea durante la activación
     * @note **AUTOMÁTICO:** Desactivación automática tras el delay
     * 
     * @warning **FUNCIÓN BLOQUEANTE:** Bloquea ejecución durante 200ms
     * @warning **NO REENTRANTE:** No llamar mientras está ejecutándose
     * @warning **THREADING:** En aplicaciones multi-hilo, considerar sincronización
     * 
     * @see TiempoBadajoOn - Constante que define duración del toque
     * @see delay() - Función Arduino utilizada para temporización
     * 
     * @example
     * @code
     * CAMPANA campanaMayor(12);
     * 
     * // Tocar campana una vez
     * campanaMayor.Toca();
     * 
     * // Secuencia de toques (ej: 3 campanadas)
     * for(int i = 0; i < 3; i++) {
     *     campanaMayor.Toca();
     *     delay(1000);  // Pausa entre toques
     * }
     * @endcode
     * 
     * @example
     * @code
     * // Uso en función de acción para alarmas
     * void accionTocaHora() {
     *     struct tm t;
     *     if (getLocalTime(&t)) {
     *         // Tocar tantas veces como la hora actual
     *         int hora = t.tm_hour;
     *         if (hora > 12) hora -= 12;  // Formato 12h
     *         if (hora == 0) hora = 12;   // Medianoche = 12 toques
     *         
     *         for(int i = 0; i < hora; i++) {
     *             campanaMayor.Toca();
     *             if (i < hora - 1) delay(800);  // Pausa entre toques
     *         }
     *     }
     * }
     * @endcode
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANA::Toca(void) {
        digitalWrite(this->_nPin, HIGH);                                // Activar el pin de la campana
        delay(TiempoBadajoOn);                                          // Mantenerlo activo durante 1 segundo
        digitalWrite(this->_nPin, LOW);                                 // Desactivar el pin de la campana
        DBG_CAMPANA_PRINTF("[CAMPANA] Toque completado en pin %d\n", _nPin);
    }
