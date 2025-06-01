#include "Campana.h"

/**
 * @brief Constructor de la clase CAMPANA
 * @details Inicializa una instancia de CAMPANA asignando el pin de control
 * @param nPin Número de pin GPIO al que está conectada la campana
 */
CAMPANA::CAMPANA (int nPin) {
    // Constructor por defecto
    this->_nPin = nPin;                     // Asignar el pin de la campana
    pinMode(this->_nPin, OUTPUT);           // Configurar el pin como salida
}
/**
 * @brief Destructor de la clase CAMPANA
 * 
 * Destructor por defecto que limpia los recursos utilizados por la instancia de CAMPANA
 * cuando esta es eliminada. Al ser el destructor por defecto, no realiza ninguna
 * operación específica de limpieza.
 */
CAMPANA::~CAMPANA() {
    // Destructor por defecto
}
/**
 * @brief Activa la campana durante un tiempo predeterminado
 * 
 * Este método controla el badajo de la campana activando el pin correspondiente
 * durante el tiempo especificado en TiempoBadajoOn. Realiza un ciclo completo
 * de activación y desactivación del badajo.
 * 
 * @note El tiempo de activación está definido por la constante TiempoBadajoOn
 * 
 * @warning Este método usa delay(), lo que bloquea la ejecución durante el tiempo especificado
 */
void CAMPANA::Toca(void) {
    digitalWrite(this->_nPin, HIGH);                                // Activar el pin de la campana
    delay(TiempoBadajoOn);                                          // Mantenerlo activo durante 1 segundo
    digitalWrite(this->_nPin, LOW);                                 // Desactivar el pin de la campana
Serial.println("Dong");
}
