#ifndef CAMPANARIO_H
	#define CAMPANARIO_H

    #include  "CAMPANA.h"

    #define DEBUGCAMPANARIO

    #define MAX_CAMPANAS 2                                              //!< Número máximo de campanas en el campanario

    #define Campana1 _pCampanas[0]                                       //!< Puntero a la primera campana del campanario
    #define Campana2 _pCampanas[1]                                       //!< Puntero a la segunda campana del campanario



    class CAMPANARIO 
    {
        public:

            CAMPANARIO();                                               //!< Constructor con pin 
            ~CAMPANARIO();
            void AddCampana (CAMPANA* pCampana);                        //!< Añade una campana al campanario
            void TocaDifuntos (void);
            void TocaFiesta (void);                                     //!< Toca la secuencia de campanas para fiesta
        
        private:

            CAMPANA* _pCampanas[MAX_CAMPANAS];                          //!< Array de punteros a las campanas del campanario
            int _nNumCampanas = 0;                                      //!< Número de campanas en el campanario

            void _TocaSecuencia1_2_1000(void);                          //!< Toca la secuencia de campanas 1 y 2 con un intervalo de 1000 ms

    };
#endif


    