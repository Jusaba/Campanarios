#ifndef CAMPANARIO_H
	#define CAMPANARIO_H

    #include  "CAMPANA.h"
    #include "Calefaccion.h"

    #define DEBUGCAMPANARIO

    #define MAX_CAMPANAS 2                                              //!< Número máximo de campanas en el campanario

    struct PasoSecuencia {
        int indiceCampana;
        int repeticiones;
        int intervaloMs;
    };

    struct ToquePlano {
        int indiceCampana;
        int intervaloMs;
    };

    const PasoSecuencia secuenciaDifuntos[] PROGMEM = {
        {1, 4, 1000},
        {0, 1, 1000},
        {1, 3, 1000},
        {0, 1, 1000},
        {1, 2, 1000},
        {0, 1, 1000},
        {1, 3, 1000},
        {0, 1, 1000},
        {1, 4, 1000},
        {0, 1, 1000},
        {1, 3, 1000},
        {0, 1, 1000},
        {1, 2, 1000},
        {0, 1, 1000},
        {1, 3, 1000},
        {0, 1, 1000},
        {1, 4, 1000},
        {0, 1, 1000},
        {1, 3, 1000},
        {0, 1, 1000},
        {1, 2, 1000},
        {0, 1, 1000},
        {1, 3, 1000},
        {0, 1, 1000},
        {1, 4, 1000},
        {0, 1, 1000},
        {1, 3, 1000},
        {0, 1, 1000},
        {1, 2, 1000},
        {0, 1, 1000},
        {1, 3, 1000},
        {0, 1, 1000}
    };
    const int numPasosDifuntos = sizeof(secuenciaDifuntos) / sizeof(PasoSecuencia);
    const PasoSecuencia secuenciaFiesta[] PROGMEM = {
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
    const int numPasosFiesta = sizeof(secuenciaFiesta) / sizeof(PasoSecuencia);

    class CAMPANARIO 
    {
        public:

            CAMPANARIO();                                               //!< Constructor con pin 
            ~CAMPANARIO();
            void AddCampana (CAMPANA* pCampana);                        //!< Añade una campana al campanario
            void TocaDifuntos (void);                                   //!< Toca la secuencia de campanadas para difuntos   
            void TocaFiesta (void);                                     //!< Toca la secuencia de campanadas para fiesta   
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

            private:

            CAMPANA* _pCampanas[MAX_CAMPANAS];                          //!< Array de punteros a las campanas del campanario
            int _nNumCampanas = 0;                                      //!< Número de campanas en el campanario
            
            ToquePlano _aCampanadas[200];                               //!< Arreglo de campanadas a tocar, con un máximo de 200 elementos
            int _nCampanadas = 0;                                       //!< Número de campanadas a tocar en la secuencia actual   
            void _GeneraraCampanadas(const PasoSecuencia* secuencia, int numPasos); // Genera una secuencia de campanadas a partir de una secuencia de pasos definida
            void _LimpiaraCampanadas(void) ;                            //!< Limpia el arreglo de campanadas y reinicia el contador.    
    
            int _indiceCampanadaActual = 0;                             //!< Índice de la campanada actual en la secuencia
            unsigned long _ultimoToqueMs = 0;                           //!< Marca de tiempo del último toque de campana en milisegundos
            bool _tocandoSecuencia = false;                             //!< Indica si se está tocando una secuencia de campanadas

            int _nCampanaTocada = 0;                                    //!< Número de campana tocada en la última secuencia
            bool _lCalefaccion = false;                                 //!< Estado de la calefacción del campanario
            
            CALEFACCION* _pCalefaccion = nullptr;                       //!< Puntero a la calefacción del campanario

        };
#endif


    