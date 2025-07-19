/**
 * @class CAMPANARIO
 * @brief Clase principal para la gestión y control del campanario.
 *
 * Esta clase permite gestionar múltiples campanas y la calefacción del campanario,
 * coordinando las secuencias de toques, el estado general y la interacción con otros sistemas.
 * Proporciona métodos para añadir campanas, activar secuencias predefinidas (difuntos, misa, fiesta, horas, cuartos),
 * controlar la calefacción y consultar el estado global del campanario.
 *
 * Ejemplo de uso:
 * @code
 * CAMPANARIO campanario;
 * campanario.AddCampana(&campana1);
 * campanario.AddCalefaccion(&calefaccion);
 * campanario.TocaDifuntos();
 * int estado = campanario.GetEstadoCampanario();
 * @endcode
 *
 * @author  Julian Salas Baertolome
 * @version 1.0
 * @date    19/06/2025
 */
#ifndef CAMPANARIO_H
	#define CAMPANARIO_H

    #include  "CAMPANA.h"
    #include "Calefaccion.h"

    #define DEBUGCAMPANARIO

    // Definición de los bits que representan el estado del campanario
    #define BitDifuntos     0x01  // 0000 0001                  // Bit para indicar si la secuencia de difuntos está activa
    #define BitMisa         0x02  // 0000 0010                  // Bit para indicar si la secuencia de misa está activa
    #define BitHora         0x04  // 0000 0100                  // Bit para indicar si la secuencia de horas está activa
    #define BitCuartos      0x08  // 0000 1000                  // Bit para indicar si la secuencia de cuartos está activa
    #define BitCalefaccion  0x10  // 0001 0000                  // Bit para indicar si la calefacción está activa

    #define MAX_CAMPANAS 2                                      //!< Número máximo de campanas en el campanario

    struct PasoSecuencia {                                      // Estructura que representa un paso en una secuencia de campanadas
        int indiceCampana;                                      //!< Índice de la campana a tocar (0 para la primera, 1 para la segunda, etc.)    
        int repeticiones;                                       //!< Número de veces que se repite el toque de la campana en este paso
        int intervaloMs;                                        //!< Intervalo en milisegundos entre toques
    };

    struct ToquePlano {                                         // Estructura que representa un toque de campana en una secuencia
        int indiceCampana;                                      //!< Índice de la campana a tocar
        int intervaloMs;                                        //!< Intervalo en milisegundos entre toques
    };

    const PasoSecuencia secuenciaDifuntos[] PROGMEM = {
        {1, 3, 2200},
        {1, 1, 1600},
        {0, 1, 2600},
        {1, 2, 2200},
        {1, 1, 1800},
        {0, 1, 2700},
        {1, 2, 2600},
        {0, 1, 2900},
        {1, 2, 2500},
        {1, 1, 2000},
        {0, 1, 2000},
        {1, 3, 2200},
        {1, 1, 1600},
        {0, 1, 2600},
        {1, 2, 2200},
        {1, 1, 1800},
        {0, 1, 2700},
        {1, 2, 2600},
        {0, 1, 2900},
        {1, 2, 2500},
        {1, 1, 2000},
        {0, 1, 2000}
    };
    const int numPasosDifuntos = sizeof(secuenciaDifuntos) / sizeof(PasoSecuencia);
    const PasoSecuencia secuenciaMisa[] PROGMEM = {
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000}, 
        {0, 2, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000}, 
        {0, 7, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000} 
    };    
    const int numPasosMisa = sizeof(secuenciaMisa) / sizeof(PasoSecuencia);

    class CAMPANARIO 
    {
        public:

            CAMPANARIO();                                               //!< Constructor con pin 
            ~CAMPANARIO();
            void AddCampana (CAMPANA* pCampana);                        //!< Añade una campana al campanario
            void TocaDifuntos (void);                                   //!< Toca la secuencia de campanadas para difuntos   
            void TocaMisa (void);                                       //!< Toca la secuencia de campanadas para misa   
            void IniciarSecuenciaCampanadas(void);                      //!< Inicia la secuencia de campanadas
            int ActualizarSecuenciaCampanadas(void);                    //!< Actualiza la secuencia de campanadas, tocando las campanas según el intervalo definido
            void ResetCampanaTocada(void);                              //!< Resetea el número de campana tocada
            void ParaSecuencia(void);                                   //!< Detiene la secuencia de campanadas
            void TocaHora(int nHora);                                   //!< Toca la campana de la hora
            void TocaCuarto(int nCuarto);                               //!< Toca la campana del cuarto correspondiente
            bool GetEstadoSecuencia (void);                             //!< Devuelve el estado de la secuencia de campanadas (true si está activa, false si no lo está)
            void AddCalefaccion(CALEFACCION* pCalefaccion);             //!< Añade la calefacción al campanario
            void EnciendeCalefaccion(void);                             //!< Enciende la calefacción del campanario
            void ApagaCalefaccion(void);                                //!< Apaga la calefacción del campanario
            bool GetEstadoCalefaccion(void);                            //!< Devuelve el estado de la calefacción del campanario (true si está encendida, false si está apagada)
            int GetEstadoCampanario(void);                              //!< Devuelve el estado del campanario
            private:

            CAMPANA* _pCampanas[MAX_CAMPANAS];                          //!< Array de punteros a las campanas del campanario
            int _nNumCampanas = 0;                                      //!< Número de campanas en el campanario
            
            ToquePlano _aCampanadas[200];                               //!< Array de campanadas a tocar en formato plano, un elemento por campanada, con un máximo de 200 elementos
            int _nCampanadas = 0;                                       //!< Número de campanadas a tocar en la secuencia actual   
            void _GeneraraCampanadas(const PasoSecuencia* secuencia, int numPasos); // Genera una secuencia de campanadas planas a partir de una secuencia de pasos definida
            void _LimpiaraCampanadas(void) ;                            //!< Limpia el array de campanadas y reinicia el contador.    
    
            int _indiceCampanadaActual = 0;                             //!< Índice de la campanada actual en la secuencia
            unsigned long _ultimoToqueMs = 0;                           //!< Marca de tiempo del último toque de campana en milisegundos
            bool _tocandoSecuencia = false;                             //!< Indica si se está tocando una secuencia de campanadas

            int _nCampanaTocada = 0;                                    //!< Número de campana tocada en la última secuencia
            bool _lCalefaccion = false;                                 //!< Estado de la calefacción del campanario
            
            CALEFACCION* _pCalefaccion = nullptr;                       //!< Puntero a la calefacción del campanario

            int _nEstadoCampanario = 0;                                 //!< Estado del campanario, representado por un entero donde cada bit indica un estado específico

            bool _tocando = false;                                      //!< Flag que indica si las campanas estan en reposo o tocando (1/0)
            bool _Calefaccion = false;                                  //!< Flag que indica si la calefacción está encendida o apagada (1/0)

        };
#endif


    