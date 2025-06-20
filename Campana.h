/**
 * @class CAMPANA
 * @brief Representa una campana controlada electrónicamente.
 *
 * Esta clase permite controlar una campana individual, gestionando su activación y
 * desactivación. Proporciona metodos de contructor, destructor y un método para tocar la campana.
 *
 * Ejemplo de uso:
 * @code
 * CAMPANA campana1(PIN_CAMPANA1);
 * campana1.Tocar(); // Toca la campana 
 * @endcode
 *
 * @author  Julian Salas Baertolome
 * @version 1.0
 * @date    19/06/2025
 * @note    Esta clase es parte del proyecto de control de campanas para ESP32.
 */
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