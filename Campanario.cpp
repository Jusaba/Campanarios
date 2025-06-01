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
    if (this->_nNumCampanas < 5) {                    // Verifica que no se exceda el número máximo de campanas
        this->_pCampanas[_nNumCampanas++] = pCampana; // Añade la campana al array y aumenta el contador
    }
    else {
        #ifdef DEBUGCAMPANARIO
            Serial.println("No se pueden añadir más campanas al campanario.");
        #endif
    }   
    
}

void CAMPANARIO::TocaDifuntos(void) {
    
    #ifdef DEBUGCAMPANARIO
        Serial.println("Tocando campanas para difuntos...");
        Serial.print ("numPasosDifuntos: ");
        Serial.println(numPasosDifuntos);
    #endif
    this->_GeneraraCampanadas(secuenciaDifuntos, numPasosDifuntos);
    this->IniciarSecuenciaCampanadas(); // Inicia la secuencia de campanadas
}
void CAMPANARIO::TocaFiesta(void) {
    
    #ifdef DEBUGCAMPANARIO
        Serial.println("Tocando campanas para fiesta...");
        Serial.print ("numPasosFiesta: ");
        Serial.println(numPasosFiesta);
    #endif
    this->_GeneraraCampanadas(secuenciaFiesta, numPasosFiesta);
    this->IniciarSecuenciaCampanadas(); // Inicia la secuencia de campanadas
}
void CAMPANARIO::_GeneraraCampanadas(const PasoSecuencia* secuencia, int numPasos) {
    int idx = 0;
    this->_LimpiaraCampanadas(); // Limpia las campanadas antes de generar nuevas
    for (int i = 0; i < numPasos; ++i) {
        for (int r = 0; r < secuencia[i].repeticiones; ++r) {
            this->_aCampanadas[idx].indiceCampana = secuencia[i].indiceCampana;
            this->_aCampanadas[idx].intervaloMs = secuencia[i].intervaloMs;
            idx++;
        }
    }
    this->_nCampanadas = idx;
    #ifdef DEBUGCAMPANARIO
        Serial.println("Campanadas generadas:");
        Serial.print ("Numero de campanadas: ");
        Serial.println(this->_nCampanadas);
    #endif

}
void CAMPANARIO::_LimpiaraCampanadas(void) {
    #ifdef DEBUGCAMPANARIO
        Serial.println("Limpiando campanadas...");
    #endif
    this->_nCampanadas = 0; // Resetea el contador de campanadas
    for (int i = 0; i < 200; ++i) {
        this->_aCampanadas[i].indiceCampana = -1; // Resetea el índice de la campana
        this->_aCampanadas[i].intervaloMs = 0; // Resetea el intervalo en milisegundos
    }
    #ifdef DEBUGCAMPANARIO
        Serial.println("Campanadas limpiadas.");
    #endif
}

void CAMPANARIO::IniciarSecuenciaCampanadas() {
    this->_indiceCampanadaActual = 0;
    this->_ultimoToqueMs = 0;
    this->_tocandoSecuencia = (_nCampanadas > 0);
}

void CAMPANARIO::ActualizarSecuenciaCampanadas() {
    if (!this->_tocandoSecuencia || this->_indiceCampanadaActual >= this->_nCampanadas) return;

    unsigned long ahora = millis();
    if (_ultimoToqueMs == 0 || (ahora - this->_ultimoToqueMs) >= this->_aCampanadas[this->_indiceCampanadaActual].intervaloMs) {
        int idxCampana = this->_aCampanadas[this->_indiceCampanadaActual].indiceCampana;
        if (idxCampana >= 0 && idxCampana < this->_nNumCampanas) {
            this->_pCampanas[idxCampana]->Toca();
        }
        this->_ultimoToqueMs = ahora;
        this->_indiceCampanadaActual++;
        if (this->_indiceCampanadaActual >= this->_nCampanadas) {
            this->_tocandoSecuencia = false;
            #ifdef DEBUGCAMPANARIO
                Serial.println("Secuencia de campanadas finalizada.");
            #endif
        }
    }
}