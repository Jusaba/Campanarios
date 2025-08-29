#include "Calefaccion.h"
#include "Configuracion.h"

/**
 * @brief Constructor de la clase CALEFACCION
 * 
 * Inicializa una instancia del sistema de calefacción configurando el pin especificado
 * como salida y estableciendo el estado inicial como apagado.
 * 
 * @param nPin Número del pin GPIO que controla la calefacción
 */
    CALEFACCION::CALEFACCION(int nPin) {
        this->_nPin = nPin;                 // Asigna el número de pin a la calefacción
        pinMode(this->_nPin, OUTPUT);       // Configura el pin de la calefacción como salida
        _lCalefaccion = false;              // Inicializa el estado de la calefacción como apagada
    }

/**
 * @brief Destructor de la clase CALEFACCION
 * 
 * Destructor por defecto que libera los recursos asociados al objeto CALEFACCION.
 * En esta implementación no se requiere liberar recursos específicos ya que no hay
 * memoria dinámica u otros recursos que necesiten ser liberados manualmente.
 */
    CALEFACCION::~CALEFACCION() {
        // Destructor, no se necesita liberar recursos específicos
    }

/**
 * @brief Activa el sistema de calefacción
 * 
 * Establece el pin digital asociado a la calefacción en estado alto (HIGH) y 
 * actualiza la variable interna que registra el estado de la calefacción a encendido.
 * 
 * @pre El pin debe estar correctamente configurado como salida
 * @post La calefacción quedará encendida y el estado interno actualizado
 */
    void CALEFACCION::Enciende(int nMinutos) {
        
        if (getLocalTime(&_tiempoEncendido)) {
            this->_nMinutosOn = nMinutos;      // Establece el tiempo solicitado para la calefacción
            this->_lCalefaccion = true;         // Actualiza el estado de la calefacción
            digitalWrite(this->_nPin, HIGH);    // Activa el pin de la calefacción
        }    
    }

/**
 * @brief Apaga la calefacción
 * 
 * Desactiva la señal del pin asociado a la calefacción y actualiza 
 * el estado interno para reflejar que está apagada
 */
    void CALEFACCION::Apaga(void) {
        digitalWrite(this->_nPin, LOW);     // Desactiva el pin de la calefacción
        this->_lCalefaccion = false;        // Actualiza el estado de la calefacción
    }

/**
 * @brief Obtiene el estado actual de la calefacción
 * 
 * @return bool true si la calefacción está encendida, false si está apagada
 */
    bool CALEFACCION::GetEstado(void) {
        return this->_lCalefaccion;         // Devuelve el estado de la calefacción
    }

    double CALEFACCION::VerificarTemporizador(void) {
        double seconds = 0;
        if (this->_lCalefaccion) {
            struct tm tiempoActual;
            if (getLocalTime(&tiempoActual)) {
                time_t tiempoEncendido = mktime(&this->_tiempoEncendido);
                time_t ahora = mktime(&tiempoActual);
                seconds = difftime(ahora, tiempoEncendido);
                if (seconds >= this->_nMinutosOn * 60) {
                    this->Apaga();
                    return 0;
                }
            } else {
                // Si no se puede obtener la hora del RTC, no hacer nada por seguridad
                if constexpr (Config::Debug::HEATING_DEBUG) {
                    Serial.println("VerificarTemporizador: Error obteniendo hora del RTC");
                }
            }
        }
        return ((this->_nMinutosOn * 60) - seconds); // Retorna los segundos restantes para apagar la calefacción
    }