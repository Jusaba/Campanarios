/**
 * @class CALEFACCION
 * @brief Clase para el control de la calefacción del campanario.
 *
 * Esta clase permite gestionar el encendido y apagado de la calefacción,
 * así como consultar su estado actual. Está diseñada para controlar un
 * relé o salida digital que activa el sistema de calefacción.
 *
 * Ejemplo de uso:
 * @code
 * CALEFACCION calefaccion(PIN_RELE);
 * calefaccion.Enciende();
 * if (calefaccion.GetEstado()) {
 *     // La calefacción está activa
 * }
 * calefaccion.Apaga();
 * @endcode
 *
 * @author  Julian Salas Baertolome
 * @version 1.0
 * @date    19/06/2025
 */
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