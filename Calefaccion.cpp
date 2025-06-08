#include "Calefaccion.h"

CALEFACCION::CALEFACCION(int nPin) {
    this->_nPin = nPin; // Asigna el número de pin a la calefacción
    pinMode(this->_nPin, OUTPUT); // Configura el pin de la calefacción como salida
    _lCalefaccion = false; // Inicializa el estado de la calefacción como apagada
}
CALEFACCION::~CALEFACCION() {
    // Destructor, no se necesita liberar recursos específicos
}
void CALEFACCION::Enciende(void) {
    digitalWrite(this->_nPin, HIGH); // Activa el pin de la calefacción
    this->_lCalefaccion = true; // Actualiza el estado de la calefacción
}
void CALEFACCION::Apaga(void) {
    digitalWrite(this->_nPin, LOW); // Desactiva el pin de la calefacción
    this->_lCalefaccion = false; // Actualiza el estado de la calefacción
}
bool CALEFACCION::GetEstado(void) {
    return this->_lCalefaccion; // Devuelve el estado de la calefacción
}