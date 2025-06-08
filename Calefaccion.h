#ifndef CALEFACCION_H
	#define CALEFACCION_H

        #include <Arduino.h>


        class CALEFACCION
        {
            public:

                CALEFACCION(int nPin );                                         //!< Constructor con pin 
                ~CALEFACCION();
                void Enciende(void);                                          //!< Enciende la calefacción
                void Apaga(void);                                             //!< Apaga la calefacción
                bool GetEstado(void);                                         //!< Devuelve el estado de la calefacción (true si está encendida, false si está apagada)
           private:

                int _nPin;                                                  //!< Pin de la campana    
                bool _lCalefaccion = false;                                 //!< Estado de la calefacción del campanario
        };
#endif