#include "Calefaccion.h"

    /**
     * @brief Constructor que inicializa el sistema de calefacción con pin específico
     * 
     * @details Inicializa el objeto calefacción configurando el pin de control
     *          y estableciendo el estado inicial. El pin se configura como salida
     *          y se inicializa en estado LOW (calefacción apagada).
     * 
     * @param nPin Pin GPIO para controlar el relé de la calefacción
     * 
     * @note **CONFIGURACIÓN PIN:** Se configura automáticamente como OUTPUT
     * @note **ESTADO INICIAL:** La calefacción inicia siempre apagada
     * @note **TEMPORIZADOR:** Se inicializa sin temporizador activo
     * 
     * 
     * @example
     * @code
     * // Crear instancia con pin 12 para controlar relé
     * CALEFACCION calefaccion(12);
     * @endcode
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    CALEFACCION::CALEFACCION(int nPin) {
        this->_nPin = nPin;                 // Asigna el número de pin a la calefacción
        pinMode(this->_nPin, OUTPUT);       // Configura el pin de la calefacción como salida
        _lCalefaccion = false;              // Inicializa el estado de la calefacción como apagada
        DBG_CALEFACCION_PRINTF("CALEFACCION->Inicializada en pin %d, estado OFF\n", this->_nPin);
    }

    /**
     * @brief Destructor que asegura el apagado seguro del sistema
     * 
     * @details Destructor que garantiza que la calefacción se apague
     *          automáticamente cuando el objeto sea destruido, evitando
     *          que quede encendida accidentalmente.
     * 
     * @note **SEGURIDAD:** Llamada automática a Apaga() para garantizar apagado
     * @note **CLEANUP:** Limpieza automática sin intervención manual
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    CALEFACCION::~CALEFACCION() {
        // Destructor, no se necesita liberar recursos específicos
        this->Apaga();
        DBG_CALEFACCION("[CALEF] Destructor - Calefacción apagada por seguridad");
    }

/**
 * @brief Enciende la calefacción con temporizador automático de apagado
 * 
 * @details Activa la calefacción durante un número específico de minutos.
 *          El sistema se apagará automáticamente cuando transcurra el tiempo
 *          especificado. Registra el momento de encendido usando RTC.
 *          
 *          **PROCESO DE ENCENDIDO:**
 *          1. Activa el pin de control (relé ON)
 *          2. Marca el estado interno como encendido
 *          3. Registra el tiempo actual de encendido (RTC)
 *          4. Configura el temporizador de apagado automático
 *          5. Log de la operación para debug
 * 
 * @param nMinutos Número de minutos que permanecerá encendida (1-1440)
 * 
 * @note **TEMPORIZADOR OBLIGATORIO:** No permite encendido indefinido
 * @note **RTC REQUERIDO:** Utiliza getLocalTime() para temporización precisa  
 * @note **SOBREESCRITURA:** Si ya está encendida, actualiza el temporizador
 * 
 * @warning **RANGO VÁLIDO:** nMinutos debe ser > 0 (se recomienda 1-120 minutos)
 * @warning **RTC SINCRONIZADO:** Requiere RTC válido para funcionamiento correcto
 * 
 * @see VerificarTemporizador() - Para verificar tiempo restante
 * @see Apaga() - Para apagado manual antes del temporizador
 * 
 * @example
 * @code
 * // Encender calefacción durante 30 minutos
 * calefaccion.Enciende(30);
 * 
 * // Verificar estado
 * if (calefaccion.GetEstado()) {
 *     Serial.println("Calefacción activada correctamente");
 * }
 * @endcode
 * 
 * @since v1.0
 * @author Julian Salas Bartolomé
 */
    void CALEFACCION::Enciende(int nMinutos) {
        
        if (getLocalTime(&this->_tiempoEncendido)) {
            this->_nMinutosOn = nMinutos;      // Establece el tiempo solicitado para la calefacción
            this->_lCalefaccion = true;         // Actualiza el estado de la calefacción
            digitalWrite(this->_nPin, HIGH);    // Activa el pin de la calefacción
            DBG_CALEFACCION_PRINTF("CALEFACCION->Encendida por %d minutos", nMinutos);
        } else {
            // Si no se puede obtener la hora del RTC, no encender por seguridad
            DBG_CALEFACCION("CALEFACCION->Error obteniendo hora del RTC, no se enciende");
        }    
    }

    /**
     * @brief Apaga inmediatamente la calefacción y limpia el temporizador
     * 
     * @details Desactiva la calefacción de forma inmediata, independientemente
     *          del estado del temporizador. Limpia todas las variables de estado
     *          y resetea el sistema para el próximo uso.
     *          
     *          **PROCESO DE APAGADO:**
     *          1. Desactiva el pin de control (relé OFF)
     *          2. Marca el estado interno como apagado
     *          3. Resetea el temporizador y tiempo de encendido
     *          4. Log de la operación para debug
     * 
     * @note **APAGADO INMEDIATO:** No espera al temporizador - apagado instantáneo
     * @note **RESET COMPLETO:** Limpia todas las variables de estado
     * @note **SEGURIDAD:** Puede llamarse en cualquier momento sin efectos secundarios
     * 
     * @see Enciende() - Para encender con temporizador
     * @see GetEstado() - Para verificar el estado actual
     * 
     * @example
     * @code
     * // Apagar calefacción manualmente
     * calefaccion.Apaga();
     * 
     * // Verificar que se apagó
     * if (!calefaccion.GetEstado()) {
     *     Serial.println("Calefacción apagada correctamente");
     * }
     * @endcode
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */

    void CALEFACCION::Apaga(void) {
        digitalWrite(this->_nPin, LOW);     // Desactiva el pin de la calefacción
        this->_lCalefaccion = false;        // Actualiza el estado de la calefacción
        this->_nMinutosOn = 0;               // Resetea el temporizador
        memset(&this->_tiempoEncendido, 0, sizeof(this->_tiempoEncendido)); // Limpia el tiempo de encendido
        DBG_CALEFACCION("CALEFACCION->Apagada");
    }

    /**
     * @brief Obtiene el estado actual de la calefacción
     * 
     * @details Devuelve el estado actual del sistema de calefacción de forma
     *          inmediata sin realizar verificaciones de temporizador.
     * 
     * @return true si la calefacción está encendida, false si está apagada
     * 
     * @note **ESTADO ACTUAL:** Refleja el estado interno, no verifica temporizador
     * @note **INMEDIATO:** No realiza operaciones de I/O ni verificaciones
     * @note **COMPLEMENTARIO:** Usar con VerificarTemporizador() para estado completo
     * 
     * @see VerificarTemporizador() - Para verificar si debe apagarse automáticamente
     * @see Enciende() - Para encender la calefacción
     * @see Apaga() - Para apagar la calefacción
     * 
     * @example
     * @code
     * if (calefaccion.GetEstado()) {
     *     Serial.println("Calefacción ON");
     *     double restante = calefaccion.VerificarTemporizador();
     *     Serial.printf("Tiempo restante: %.1f segundos\n", restante);
     * } else {
     *     Serial.println("Calefacción OFF");
     * }
     * @endcode
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    bool CALEFACCION::GetEstado(void) {
        return this->_lCalefaccion;         // Devuelve el estado de la calefacción
    }
    /**
     * @brief Verifica el temporizador y apaga automáticamente si ha expirado
     * 
     * @details Función principal de gestión del temporizador que debe ser llamada
     *          periódicamente para verificar si la calefacción debe apagarse
     *          automáticamente. Calcula el tiempo transcurrido desde el encendido
     *          y lo compara con el temporizador programado.
     *          
     *          **ALGORITMO DE VERIFICACIÓN:**
     *          1. Si no está encendida, retorna 0 inmediatamente
     *          2. Obtiene tiempo actual del RTC
     *          3. Calcula tiempo transcurrido desde encendido
     *          4. Si ha expirado el temporizador: apaga automáticamente
     *          5. Si no ha expirado: calcula y retorna tiempo restante
     *          
     *          **CÁLCULO TEMPORAL:**
     *          - Utiliza difftime() para cálculo preciso entre timestamps
     *          - Maneja correctamente cambios de día/hora
     *          - Resolución de segundos para precisión máxima
     * 
     * @return Tiempo restante en segundos (double) o 0.0 si está apagada/expirada
     * 
     * @note **LLAMADA PERIÓDICA:** Debe llamarse regularmente desde loop() o timer
     * @note **AUTO-APAGADO:** Apaga automáticamente cuando expira el temporizador
     * @note **PRECISIÓN:** Utiliza RTC para cálculos temporales precisos
     * @note **MANEJO ERRORES:** Gestiona errores de RTC graciosamente
     * 
     * @warning **RTC REQUERIDO:** Requiere RTC sincronizado para funcionamiento
     * @warning **FRECUENCIA:** Llamar cada 1-10 segundos para respuesta adecuada
     * 
     * @see Enciende() - Para establecer el temporizador inicial
     * @see Apaga() - Llamada automáticamente cuando expira
     * @see GetEstado() - Para verificar estado sin modificarlo
     * 
     * @example
     * @code
     * void loop() {
     *     // Verificar temporizador de calefacción cada segundo
     *     double restante = calefaccion.VerificarTemporizador();
     *     
     *     if (restante > 0) {
     *         Serial.printf("Calefacción ON - Restante: %.0f segundos\n", restante);
     *     } else if (calefaccion.GetEstado()) {
     *         Serial.println("Calefacción ON - Sin temporizador");
     *     } else {
     *         Serial.println("Calefacción OFF");
     *     }
     *     
     *     delay(1000);
     * }
     * @endcode
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    double CALEFACCION::VerificarTemporizador(void) {
        double seconds = 0;
        if (this->_lCalefaccion) {                                                                  // Si la calefacción está encendida
            struct tm tiempoActual;                                                                 // Estructura para obtener el tiempo actual
            if (getLocalTime(&tiempoActual)) {                                                      // Obtener el tiempo actual del RTC                
                time_t tiempoEncendido = mktime(&this->_tiempoEncendido);                           // Convertir tiempo de encendido a time_t
                time_t ahora = mktime(&tiempoActual);                                               // Convertir tiempo actual a time_t
                seconds = difftime(ahora, tiempoEncendido);                                         // Calcular segundos transcurridos desde el encendido
                if (seconds >= this->_nMinutosOn * 60) {                                            // Si ha pasado el tiempo programado
                    this->Apaga();                                                                  // Apagar la calefacción automáticamente    
                    DBG_CALEFACCION("VerificarTemporizador: Temporizador expirado, calefacción apagada");
                    return 0;                                                                       // Retorna 0 ya que se ha apagado  
                }
            } else {                                                                                // Si no se puede obtener la hora del RTC, no hacer nada por seguridad
                    DBG_CALEFACCION("VerificarTemporizador: Error obteniendo hora del RTC");
            }
            return ((this->_nMinutosOn * 60) - seconds);                                            // Retorna los segundos restantes para apagar la calefacción            
        }else{
            return 0;                                                                               // Si la calefacción está apagada, retorna 0
        }
        
    }