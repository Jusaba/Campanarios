#include "Campanario.h"

/**
 * @brief Constructor de la clase CAMPANARIO
 * 
 * Este constructor inicializa una instancia de CAMPANARIO sin parámetros.
 * Actualmente no realiza ninguna operación específica de inicialización.
 */
CAMPANARIO::CAMPANARIO() {
    // Constructor por defecto
}
/**
 * @brief Destructor de la clase CAMPANARIO
 * 
 * Destructor por defecto que limpia los recursos utilizados por la instancia de CAMPANARIO
 * cuando esta es eliminada. Al ser el destructor por defecto, no realiza ninguna
 * operación específica de limpieza.
 */CAMPANARIO::~CAMPANARIO() {
    // Destructor por defecto   
}
/**
 * @brief Añade una campana al campanario
 * 
 * Este método permite agregar una instancia de CAMPANA al campanario.
 * 
 * @param pCampana Puntero a la instancia de CAMPANA que se desea añadir al campanario
 */ 
void CAMPANARIO::AddCampana(CAMPANA* pCampana) {
    if (_nNumCampanas < 5) {                    // Verifica que no se exceda el número máximo de campanas
        _pCampanas[_nNumCampanas++] = pCampana; // Añade la campana al array y aumenta el contador
    }
    else {
        #ifdef DEBUGCAMPANARIO
            Serial.println("No se pueden añadir más campanas al campanario.");
        #endif
    }   
    
}
/**
 * @brief Toca la campana para difuntos
 * 
 * Este método hace tocar las campanas del campanario para el evento de difuntos.
 */
void CAMPANARIO::TocaDifuntos(void) {
   
    #ifdef DEBUGCAMPANARIO
        Serial.println("Tocando campanas para difuntos...");
    #endif
    //Priemer paso de la secuencia de toques
    Campana2->Repique(4, 1000);             // Hace que la segunda campana toque 4 veces con un intervalo de 1000 ms entre toques
    Campana1->Repique(1, 1000);             // Hace que la primera campana toque 1 vez con un intervalo de 1000 ms
    Campana2->Repique(3, 1000);             // Hace que la segunda campana toque 4 veces con un intervalo de 1000 ms entre toques
    Campana1->Repique(1, 1000);             // Hace que la primera campana toque 1 vez con un intervalo de 1000 ms
    Campana2->Repique(2, 1000);             // Hace que la segunda campana toque 4 veces con un intervalo de 1000 ms entre toques
    Campana1->Repique(1, 1000);             // Hace que la primera campana toque 1 vez con un intervalo de 1000 ms
    Campana2->Repique(3, 1000);             // Hace que la segunda campana toque 4 veces con un intervalo de 1000 ms entre toques
    Campana1->Repique(1, 1000);             // Hace que la primera campana toque 1 vez con un intervalo de 1000 ms
    //Segundo paso de la secuencia de toques
    Campana2->Repique(4, 1000);             // Hace que la segunda campana toque 4 veces con un intervalo de 1000 ms entre toques
    Campana1->Repique(1, 1000);             // Hace que la primera campana toque 1 vez con un intervalo de 1000 ms
    Campana2->Repique(3, 1000);             // Hace que la segunda campana toque 4 veces con un intervalo de 1000 ms entre toques
    Campana1->Repique(1, 1000);             // Hace que la primera campana toque 1 vez con un intervalo de 1000 ms
    Campana2->Repique(2, 1000);             // Hace que la segunda campana toque 4 veces con un intervalo de 1000 ms entre toques
    Campana1->Repique(1, 1000);             // Hace que la primera campana toque 1 vez con un intervalo de 1000 ms
    Campana2->Repique(3, 1000);             // Hace que la segunda campana toque 4 veces con un intervalo de 1000 ms entre toques
    Campana1->Repique(1, 1000);             // Hace que la primera campana toque 1 vez con un intervalo de 1000 ms
    #ifdef DEBUGCAMPANARIO
        Serial.println("Secuencia de difuntos completada.");
    #endif

}
/**
 * @brief Toca la secuencia de campanas para fiesta
 * 
 * Este método hace tocar las campanas del campanario para el evento de fiesta.
 */
void CAMPANARIO::TocaFiesta(void) {
    #ifdef DEBUGCAMPANARIO
        Serial.println("Tocando campanas para fiesta...");
    #endif
    int nRepeticiones = 12; 
    while (nRepeticiones > 0) {          // Repite la secuencia hasta que nRepeticiones sea 0
        this->_TocaSecuencia1_2_1000();  // Llama al método privado para tocar la secuencia de campanas
        nRepeticiones--;                 // Decrementa el contador de repeticiones
    }   
    Campana1->Repique(2, 1000);          // Hace que la primera campana toque 2 veces con un intervalo de 1000 ms
    nRepeticiones = 12;                  // Reinicia el contador de repeticiones
    while (nRepeticiones > 0) {          // Repite la secuencia hasta que nRepeticiones sea 0
        this->_TocaSecuencia1_2_1000();  // Llama al método privado para tocar la secuencia de campanas
        nRepeticiones--;                 // Decrementa el contador de repeticiones
    }
    Campana1->Repique(7, 1000);          // Hace que la primera campana toque 2 veces con un intervalo de 1000 ms
    nRepeticiones = 6;                   // Reinicia el contador de repeticiones
    while (nRepeticiones > 0) {          // Repite la secuencia hasta que nRepeticiones sea 0
        this->_TocaSecuencia1_2_1000();  // Llama al método privado para tocar la secuencia de campanas
        nRepeticiones--;                 // Decrementa el contador de repeticiones
    }
    Campana1->Repique(1, 1000);          // Hace que la primera campana toque 1 vez con un intervalo de 1000 ms

    #ifdef DEBUGCAMPANARIO
        Serial.println("Secuencia de fiesta completada.");
    #endif
}
/**
 * @brief Toca la secuencia de campanas 1 y 2 con un intervalo de 1000 ms
 * 
 */

 void CAMPANARIO::_TocaSecuencia1_2_1000 (void) {
    Campana1->Repique(1, 1000);             // Hace que la primera campana toque 1 vez con un intervalo de 1000 ms
    Campana2->Repique(1, 1000);             // Hace que la segunda campana toque 1 vez con un intervalo de 1000 ms
 }