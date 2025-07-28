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
 */
    CAMPANARIO::~CAMPANARIO() {
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
        if (this->_nNumCampanas < MAX_CAMPANAS) {                    // Verifica que no se exceda el número máximo de campanas
            this->_pCampanas[_nNumCampanas++] = pCampana;            // Añade la campana al array y aumenta el contador
        }else {
            #ifdef DEBUGCAMPANARIO
                Serial.println("No se pueden añadir más campanas al campanario.");
            #endif
        }   

    }

/**
 * @brief Inicia la secuencia de campanadas para difuntos
 * 
 * @details Este método ejecuta una secuencia predefinida de campanadas utilizada 
 * tradicionalmente para anunciar un fallecimiento o durante ceremonias fúnebres.
 * Utiliza una secuencia y número de pasos específicos para difuntos, definidos 
 * en las variables secuenciaDifuntos y numPasosDifuntos.
 * 
 * El método realiza las siguientes acciones:
 * 1. Si DEBUGCAMPANARIO está definido, imprime mensajes de depuración
 * 2. Genera la secuencia de campanadas usando el patrón para difuntos
 * 3. Inicia la ejecución de la secuencia
 * 
 * @see _GeneraraCampanadas()
 * @see IniciarSecuenciaCampanadas()
 *
 * @note Esta función es parte de la clase CAMPANARIO y gestiona uno de los
 * toques tradicionales de campanas
 */
    void CAMPANARIO::TocaDifuntos(void) {
        #ifdef DEBUGCAMPANARIO
            Serial.println("Tocando campanas para difuntos...");
            Serial.print ("numPasosDifuntos: ");
            Serial.println(numPasosDifuntos);
        #endif
        this->_GeneraraCampanadas(secuenciaDifuntos, numPasosDifuntos);             // Genera la secuencia plana de campanadas para difuntos
        this->IniciarSecuenciaCampanadas();                                         // Inicia la secuencia de toque de campanadas
        this->_nEstadoCampanario |= BitDifuntos;                                    // Marca el estado del campanario como tocando difuntos
    }

/**
 * @brief Ejecuta la secuencia de campanadas para anunciar la misa
 * 
 * @details Genera y ejecuta una secuencia predefinida de campanadas utilizada para 
 * anunciar la misa. Utiliza un patrón almacenado en secuenciaMisa y 
 * el número de pasos definido en numPasosMisa.
 * 
 * Si está definido DEBUGCAMPANARIO, imprime información de depuración por el puerto serie.
 * 
 * @note Esta función primero genera la secuencia plana de campanadas y luego la inicia
 * 
 * @see _GeneraraCampanadas()
 * @see IniciarSecuenciaCampanadas()
 *
 * @note Esta función es parte de la clase CAMPANARIO y gestiona uno de los 
 * toques tradicionales de campanas
 */
    void CAMPANARIO::TocaMisa(void) {

        #ifdef DEBUGCAMPANARIO
            Serial.println("Tocando campanas para misa...");
            Serial.print ("numPasosMisa: ");
            Serial.println(numPasosMisa);
        #endif
        this->_GeneraraCampanadas(secuenciaMisa, numPasosMisa);
        this->IniciarSecuenciaCampanadas(); // Inicia la secuencia de campanadas
        this->_nEstadoCampanario |= BitMisa;
    }

/**
 * @brief Genera una secuencia plana de campanadas a partir de una secuencia de pasos definida
 * 
 * @details Este método toma una secuencia de pasos y genera un array interno de campanadas
 * expandiendo las repeticiones especificadas en cada paso. Limpia cualquier secuencia
 * anterior antes de generar la nueva.
 * 
 * @param secuencia Puntero a un array de estructura PasoSecuencia que contiene la 
 *                  definición de los pasos a ejecutar
 * @param numPasos Número total de pasos en la secuencia
 * 
 * @note La función actualiza los miembros internos _aCampanadas y _nCampanadas
 * @note Si DEBUGCAMPANARIO está definido, imprime información de depuración por Serial
 */
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

/**
 * @brief Limpia el array de campanadas y reinicia el contador.
 * 
 * Esta función reinicia el estado de las campanadas:
 * - Pone a cero el contador de campanadas (_nCampanadas)
 * - Limpia el array _aCampanadas, estableciendo valores por defecto:
 *   - Índice de campana a -1
 *   - Intervalo a 0 ms
 * para todas las posiciones del array (200 elementos)
 * 
 * @note Función privada de la clase
 */
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

/**
 * @brief Inicia la secuencia de campanadas desde el principio
 * 
 * Reinicia el contador de campanadas actual a 0, limpia el timestamp del último toque
 * y establece el estado de la secuencia como activo si hay campanadas pendientes.
 */
    void CAMPANARIO::IniciarSecuenciaCampanadas() {
        this->_indiceCampanadaActual = 0;
        this->_ultimoToqueMs = 0;
        this->_tocandoSecuencia = (this->_nCampanadas > 0);
    }

/**
 * @brief Actualiza y ejecuta la secuencia de campanadas programada
 * 
 * Este método controla la ejecución de una secuencia de campanadas. Verifica si es tiempo
 * de tocar la siguiente campana en la secuencia basándose en los intervalos definidos.
 * Si corresponde, hace sonar la campana y actualiza los contadores internos.
 * 
 * Esta funcion se llama desde loop para gestionar las campanadas de forma continua y de forma NO BLOQUEANTE.
 * 
 * @return int Retorna:
 *         - 0 si no hay secuencia activa o no hay campanadas pendientes
 *         - Número de la última campana tocada (1-based index) si se tocó una campana
 * 
 * @details El método:
 *          - Verifica si hay una secuencia activa
 *          - Controla los intervalos de tiempo entre campanadas
 *          - Ejecuta el toque de campana cuando corresponde
 *          - Actualiza contadores internos
 *          - Finaliza la secuencia cuando se completan todas las campanadas
 * 
 * @note Utiliza _tocandoSecuencia para controlar si hay una secuencia activa
 * @note Los índices de campana deben estar dentro del rango válido (0 a _nNumCampanas-1)
 */
    int CAMPANARIO::ActualizarSecuenciaCampanadas() {
        if (!this->_tocandoSecuencia || this->_indiceCampanadaActual >= this->_nCampanadas)                                                 // Si no se está tocando una secuencia o no hay campanadas, retorna 0   
        {
            return 0;   
        }    
        unsigned long ahora = millis();                                                                                                     // Obtiene el tiempo actual en milisegundos
        if (this->_ultimoToqueMs == 0 || (ahora - this->_ultimoToqueMs) >= this->_aCampanadas[this->_indiceCampanadaActual].intervaloMs) {  // Si es el primer toque o ha pasado el intervalo definido
            int idxCampana = this->_aCampanadas[this->_indiceCampanadaActual].indiceCampana;                                                // Obtiene el índice de la campana a tocar
            if (idxCampana >= 0 && idxCampana < this->_nNumCampanas) {                                                                      // Verifica que el índice de campana esté dentro del rango válido   
                this->_pCampanas[idxCampana]->Toca();                                                                                       // Llama al método Toca de la campana correspondiente para hacerla sonar
                this->_nCampanaTocada = 1 + idxCampana;                                                                                     // Actualiza el número de campanada tocada ( el 1 es poruq e la campana 1 esta en un indice 0)
                #ifdef DEBUGCAMPANARIO
                    Serial.print("Tocando campana: ");
                    Serial.println(this->_nCampanaTocada);
                #endif
            } else {
                #ifdef DEBUGCAMPANARIO
                    Serial.println("Índice de campana fuera de rango.");
                #endif
            }
            this->_ultimoToqueMs = ahora;                                                                                                   // Actualiza la marca de tiempo del último toque
            this->_indiceCampanadaActual++;                                                                                                 // Incrementa el índice de la campanada actual    
            if (this->_indiceCampanadaActual >= this->_nCampanadas) {                                                                       // Si se han tocado todas las campanadas de la secuencia plana
                this->_tocandoSecuencia = false;                                                                                            // Marca la secuencia como no activa    
                #ifdef DEBUGCAMPANARIO
                    Serial.println("Secuencia de campanadas finalizada.");
                #endif
            }
        }
        return this->_nCampanaTocada;                                                                                                       // Retorna el número de campana tocada en la última secuencia
    }

/**
 * @brief Reinicia la variable que contiene la ultima campana tocada
 * 
 * Se usa para reiniciar el contador de campana tocada a 0 despues de haberla reconocido desde el cliente web 
 * Si está definido DEBUGCAMPANARIO, imprime un mensaje de
 * confirmación por el puerto serie.
 */
    void CAMPANARIO::ResetCampanaTocada() {
        this->_nCampanaTocada = 0; // Resetea el número de campana tocada
        #ifdef DEBUGCAMPANARIO
            Serial.println("Número de campana tocada reseteado.");
        #endif
    }

/**
 * @brief Detiene la ejecución de una secuencia de campanadas en curso
 * 
 * Esta función interrumpe la secuencia de campanadas actual, limpiando el estado
 * interno del campanario y deteniendo cualquier campanada que estuviera en progreso.
 * Si está habilitado el modo debug, imprime un mensaje de confirmación.
 * 
 * @note Utiliza el flag interno _tocandoSecuencia para controlar el estado
 */
    void CAMPANARIO::ParaSecuencia() {
        this->_tocandoSecuencia = false;                                                                // Detiene la secuencia de campanadas
        this->_LimpiaraCampanadas();                                                                    // Limpia las campanadas
        #ifdef DEBUGCAMPANARIO
            Serial.println("Secuencia de campanadas detenida.");
        #endif
        this->_nEstadoCampanario &= ~((BitCalefaccion - 1));                                            // Limpia los bits de estado del campanario relacionados con las campanadas                     
    }

/**
 * @brief Toca la campana para marcar los cuartos de hora
 * 
 * Esta función programa una secuencia de campanadas para indicar los cuartos de hora.
 * Utiliza la primera campana (índice 0) y un intervalo fijo de 1000ms entre campanadas.
 * 
 * @param nCuarto Número de cuartos a tocar (1-4)
 * 
 * @note La función limpia cualquier secuencia previa de campanadas antes de programar la nueva
 * @note Utiliza la campana 1 (índice 0) para todas las campanadas de cuartos
 * 
 * @see _LimpiaraCampanadas()
 * @see IniciarSecuenciaCampanadas()
 */
    void CAMPANARIO::TocaCuarto(int nCuarto) {
        this->_LimpiaraCampanadas();                                                                    // Limpia las campanadas antes de tocar la hora
        for (int i = 0; i < nCuarto; ++i) {                                                             // Itera sobre el número de cuartos a tocar                           
            this->_aCampanadas[i].indiceCampana = 1;                                                    // Toca la campana 2
            this->_aCampanadas[i].intervaloMs = 1000;                                                   // espaciado 1000 ms
        }
        this->_nCampanadas = nCuarto;                                                                   // Actualiza el número de campanadas a tocar
        this->_nEstadoCampanario |= BitCuartos;                                                         // Actualiza el estado del campanario para indicar que se están tocando cuartos
        this->IniciarSecuenciaCampanadas();                                                             // Inicia la secuencia de campanadas
        #ifdef DEBUGCAMPANARIO
            Serial.print("Tocando cuarto: ");
            Serial.println(nCuarto);    
        #endif
    }
//============================================================================================
/**
 * @brief Genera la secuencia de campanadas para marcar una hora específica.
 * 
 * Esta función prepara y ejecuta el patrón de campanadas tradicional para marcar una hora:
 * primero los cuatro cuartos (usando la campana 1) y luego las campanadas 
 * correspondientes a la hora (usando la campana 2).
 * 
 * @param nHora Hora a tocar (0-23). Se convertirá automáticamente al formato 12h.
 * 
 * @details
 * - Primero limpia cualquier secuencia de campanadas anterior
 * - Programa los 4 cuartos usando la campana 1 con intervalos de 1000ms
 * - Convierte la hora al formato 12h (1-12)
 * - Programa las campanadas de la hora usando la campana 2 con intervalos de 1000ms
 * - Inicia la secuencia completa de campanadas
 * 
 * @note Si la hora es 0 o 12, sonará 12 campanadas
 */
    void CAMPANARIO::TocaHorayCuartos(int nHora) {
        this->_LimpiaraCampanadas(); // Limpia las campanadas antes de tocar la hora
        int i = 0;
        for ( i = 0; i < 4; ++i) {
            this->_aCampanadas[i].indiceCampana = 1;        // Toca la campana 2 los cuatro cuartos
            this->_aCampanadas[i].intervaloMs = 1000;       // espaciado 1000 ms
        }

        int nHoraReal = nHora % 12; // Asegura que la hora esté en el rango de 0 a 11
        int nHoraTocada = (nHoraReal == 0) ? 12 : nHoraReal; // Si es 0, se toca la campana 12
        for ( int i = 0; i < nHoraTocada; ++i) {
            this->_aCampanadas[i+4].indiceCampana = 0;        // Toca la campana 2 para la hora
            this->_aCampanadas[i+4].intervaloMs = ( i== 0) ? 3000 : 1000;       // espaciados 1000 ms o 3000 en el primer toque de hora
        }    
        this->_nCampanadas = nHoraTocada + 4; // Actualiza el número de campanadas a tocar (4 cuartos + hora)
        this->_nEstadoCampanario |= BitHora;                                   // Actualiza el estado del campanario para indicar que se está tocando la hora
        this->IniciarSecuenciaCampanadas(); // Inicia la secuencia de campanadas
        #ifdef DEBUGCAMPANARIO
            Serial.print("Tocando hora con cuartos: ");
            Serial.println(nHora);    
        #endif
    }

    void CAMPANARIO::TocaHoraSinCuartos(int nHora) {
        this->_LimpiaraCampanadas();                            // Limpia las campanadas antes de tocar la hora
        int nHoraReal = nHora % 12;                             // Asegura que la hora esté en el rango de 0 a 11
        int nHoraTocada = (nHoraReal == 0) ? 12 : nHoraReal;    // Si es 0, se toca la campana 12
        for (int i = 0; i < nHoraTocada; ++i) {
            this->_aCampanadas[i].indiceCampana = 0;            // Toca la campana 2 para la hora
            this->_aCampanadas[i].intervaloMs = (i == 0) ? 3000 : 1000; // espaciados 1000 ms o 3000 en el primer toque de hora
        }
        this->_nCampanadas = nHoraTocada;                       // Actualiza el número de campanadas a tocar (solo la hora)
        this->_nEstadoCampanario |= BitHora;                    // Actualiza el estado del campanario para indicar que se está tocando la hora
        this->IniciarSecuenciaCampanadas();                     // Inicia la secuencia de campanadas
        #ifdef DEBUGCAMPANARIO
            Serial.print("Tocando hora sin cuartos: ");
            Serial.println(nHora);
        #endif
    }

    /**
     * @brief Toca la campanada correspondiente a la media hora.
     *
     * Este método limpia cualquier campanada previa, configura una sola campanada
     * (utilizando la campana 2) con un intervalo de 1000 ms, actualiza el estado
     * interno para indicar que se está tocando la media hora e inicia la secuencia
     * de campanadas. Si está habilitado DEBUGCAMPANARIO, imprime un mensaje por
     * el puerto serie.
     */
    void CAMPANARIO::TocaMediaHora(void) {
        this->_LimpiaraCampanadas(); // Limpia las campanadas antes de tocar la media hora
        this->_aCampanadas[0].indiceCampana = 1; // Toca la campana 2 para la media hora
        this->_aCampanadas[0].intervaloMs = 1000; // espaciado 1000 ms
        this->_nCampanadas = 1; // Actualiza el número de campanadas a tocar (1 para media hora)
        this->_nEstadoCampanario |= BitHora; // Actualiza el estado del campanario para indicar que se está tocando la media hora
        this->IniciarSecuenciaCampanadas(); // Inicia la secuencia de campanadas
        #ifdef DEBUGCAMPANARIO
            Serial.println("Tocando media hora.");
        #endif
    }
/**
 * @brief Obtiene el estado actual de la secuencia de campanadas
 * 
 * @return true Si hay una secuencia de campanadas en ejecución
 * @return false Si no hay ninguna secuencia de campanadas ejecutándose
 */
    bool CAMPANARIO::GetEstadoSecuencia() {
        return this->_tocandoSecuencia; // Retorna el estado de la secuencia de campanadas
    }

/**
 * @brief Añade una calefacción al campanario
 * 
 * 
 * @param pCalefaccion Puntero a la instancia de CALEFACCION que se desea añadir al campanario
 */ 
    void CAMPANARIO::AddCalefaccion (CALEFACCION* pCalefaccion) {
        if (this->_pCalefaccion == nullptr) { // Verifica que no haya calefacción añadida
            this->_pCalefaccion = pCalefaccion; // Asigna la calefacción al campanario
            #ifdef DEBUGCAMPANARIO
                Serial.println("Calefacción añadida al campanario.");
            #endif
        } else {
            #ifdef DEBUGCAMPANARIO
                Serial.println("Ya hay una calefacción añadida al campanario.");
            #endif
        }

    }
/**
 * @brief Obtiene el estado de la calefacción del campanario
 * 
 * @return true Si la calefacción está activada
 * @return false Si la calefacción está desactivada
 */
    bool CAMPANARIO::GetEstadoCalefaccion() {
        if (this->_pCalefaccion != nullptr) {
            return (this->_nEstadoCampanario & BitCalefaccion) != 0; // Retorna true si el bit de calefacción está activado
        } else {
            #ifdef DEBUGCAMPANARIO
                Serial.println("No hay calefacción añadida al campanario.");
            #endif
            return false; // Si no hay calefacción, retorna false
        }
    }
/**
 * @brief Enciende la calefacción del campanario
 * 
 * Si hay una calefacción añadida, la enciende. Si no, imprime un mensaje de error.
 * Actualiza el estado del campanario para indicar que la calefacción está encendida.
 */
    void CAMPANARIO::EnciendeCalefaccion(int nMinutos) {
        if (this->_pCalefaccion != nullptr) {
            this->_pCalefaccion->Enciende(nMinutos); // Enciende la calefacción
            this->_nEstadoCampanario |= BitCalefaccion; // Actualiza el estado del campanario para indicar que la calefacción está encendida
            #ifdef DEBUGCAMPANARIO
                Serial.println("Calefacción encendida.");
            #endif
        } else {
            #ifdef DEBUGCAMPANARIO
                Serial.println("No hay calefacción añadida al campanario.");
            #endif
        }
    }
/**
 * @brief Apaga la calefacción del campanario
 * 
 * Si hay una calefacción añadida, la apaga. Si no, imprime un mensaje de error.
 * Actualiza el estado del campanario para indicar que la calefacción está apagada.
 */
    void CAMPANARIO::ApagaCalefaccion() {
        if (this->_pCalefaccion != nullptr) {
            this->_pCalefaccion->Apaga(); // Apaga la calefacción
            this->_nEstadoCampanario &= ~BitCalefaccion; // Actualiza el estado del campanario para indicar que la calefacción está apagada
            #ifdef DEBUGCAMPANARIO
                Serial.println("Calefacción apagada.");
            #endif
        } else {
            #ifdef DEBUGCAMPANARIO
                Serial.println("No hay calefacción añadida al campanario.");
            #endif
        }
    }

    double CAMPANARIO::TestTemporizacionCalefaccion(void) {
        if (this->_pCalefaccion != nullptr) {
            return this->_pCalefaccion->VerificarTemporizador(); // Retorna el estado de la temporización de la calefacción y los segundos que faltan para apagar
        } else {
            #ifdef DEBUGCAMPANARIO
                Serial.println("No hay calefacción añadida al campanario.");
            #endif
            return -1; // Si no hay calefacción, retorna false
        }
    }

    void CAMPANARIO::SetInternetConectado (void) {
        this->_nEstadoCampanario &= ~BitEstadoSinInternet; // Actualiza el estado del campanario para indicar que hay conexión a Internet
        #ifdef DEBUGCAMPANARIO
            Serial.println("Internet conectado.");
        #endif
    }

    void CAMPANARIO::ClearInternetConectado (void) {
        this->_nEstadoCampanario |= BitEstadoSinInternet; // Actualiza el estado del campanario para indicar que no hay conexión a Internet
        #ifdef DEBUGCAMPANARIO
            Serial.println("Internet desconectado.");
        #endif
    }

    void CAMPANARIO::SetProteccionCampanadas(void) {
        this->_nEstadoCampanario |= BitEstadoProteccionCampanadas; // Establece el estado de protección de campanadas
        #ifdef DEBUGCAMPANARIO
            Serial.print("Protección de campanadas activada");
        #endif
    }

    void CAMPANARIO::ClearProteccionCampanadas(void) {
        this->_nEstadoCampanario &= ~BitEstadoProteccionCampanadas; // Limpia el estado de protección de campanadas
        #ifdef DEBUGCAMPANARIO
            Serial.print("Protección de campanadas desactivada");
        #endif
    }

/**
 * @brief Obtiene el estado del campanario
 * 
 * @return int Devuelve el estado del campanario como un entero que representa los bits de estado
 * 
 * El estado se compone de varios bits que indican diferentes estados:
 * - BitDifuntos: Si la secuencia de difuntos está activa
 * - BitMisa: Si la secuencia de misa está activa
 * - BitHora: Si la secuencia de hora está activa
 * - BitCuartos: Si la secuencia de cuartos está activa
 * - BitCalefaccion: Si la calefacción está encendida
 */
    int CAMPANARIO::GetEstadoCampanario(void) {
        return this->_nEstadoCampanario;
    }
