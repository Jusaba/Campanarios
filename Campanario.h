#ifndef CAMPANARIO_H
	#define CAMPANARIO_H

    #include  "CAMPANA.h"

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
        {1, 4, 1000},
        {0, 3, 1000}
    };    
    const int numPasosFiesta = sizeof(secuenciaFiesta) / sizeof(PasoSecuencia);

    class CAMPANARIO 
    {
        public:

            CAMPANARIO();                                               //!< Constructor con pin 
            ~CAMPANARIO();
            void AddCampana (CAMPANA* pCampana);                        //!< Añade una campana al campanario
            void TocaDifuntos (void);
            void TocaFiesta (void);
            void IniciarSecuenciaCampanadas(void);
            void ActualizarSecuenciaCampanadas(void);
        private:

            CAMPANA* _pCampanas[MAX_CAMPANAS];                          //!< Array de punteros a las campanas del campanario
            int _nNumCampanas = 0;                                      //!< Número de campanas en el campanario
            
            ToquePlano _aCampanadas[200];
            int _nCampanadas = 0;
            void _GeneraraCampanadas(const PasoSecuencia* secuencia, int numPasos);
            void _LimpiaraCampanadas(void) ;
    
            int _indiceCampanadaActual = 0;
            unsigned long _ultimoToqueMs = 0;
            bool _tocandoSecuencia = false;

        };
#endif


    