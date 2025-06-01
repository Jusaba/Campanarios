#ifndef CAMPANA_H
	#define CAMPANA_H

        #include <Arduino.h>

        #define TiempoBadajoOn 200                                          //!< Tiempo que se mantiene el pin de la campana activo (1 segundo)


        class CAMPANA 
        {
            public:

                CAMPANA(int nPin );                                         //!< Constructor con pin 
                ~CAMPANA();
                void Toca (void);
           private:

                int _nPin;                                                  //!< Pin de la campana    
        };

#endif