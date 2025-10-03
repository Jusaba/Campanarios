/**
 * @file Campana.cpp
 * @brief Implementación del control individual de campanas electrónicas
 * 
 * @details Este archivo implementa el control de campanas individuales para
 *          el sistema de campanario automatizado. Cada campana se controla
 *          mediante un pin GPIO que activa un relé o solenoide durante un
 *          tiempo específico para simular el golpe del badajo.
 *          
 *          **FUNCIONALIDADES IMPLEMENTADAS:**
 *          - Control temporal preciso del badajo (200ms por defecto)
 *          - Gestión segura del pin GPIO (configuración automática)
 *          - Activación/desactivación controlada por software
 *          - Constructor con inicialización automática del pin
 *          - Destructor con apagado seguro
 *          
 *          **CARACTERÍSTICAS TÉCNICAS:**
 *          - Tiempo de activación configurable via TiempoBadajoOn
 *          - Pin configurado automáticamente como OUTPUT
 *          - Estado inicial siempre LOW (campana inactiva)
 *          - Bloqueo durante la activación para evitar solapamientos
 *          
 *          **INTEGRACIÓN SISTEMA:**
 *          - Compatible con sistema de secuencias (Campanario.h)
 *          - Utilizable desde sistema de alarmas programadas
 *          - Control individual independiente de otras campanas
 *          - Debug integrado para monitorización
 * 
 * @note **HARDWARE:** Diseñado para relés, solenoides o drivers de campana
 * @note **TIMING:** Utiliza delay() para simplicidad - no bloqueante en uso normal
 * @note **SEGURIDAD:** Pin se apaga automáticamente en destructor
 * 
 * @warning **BLOQUEO TEMPORAL:** Durante Toca() el hilo se bloquea 200ms
 * @warning **PIN ÚNICO:** Cada instancia debe usar un pin GPIO diferente
 * @warning **VOLTAJE:** Verificar que el pin soporta el voltaje del relé/solenoide
 * 
 * @author Julian Salas Bartolomé
 * @date 2025-09-16
 * @version 1.0
 * 
 * @see Campana.h - Definiciones de la clase y constantes
 * @see Campanario.h - Sistema que utiliza múltiples instancias de esta clase
 * @see Acciones.h - Funciones que pueden utilizar campanas individuales
 * 
 * @todo Implementar versión no-bloqueante con timer/interrupt
 * @todo Añadir soporte para diferentes tiempos de badajo por campana
 * @todo Integrar sensor de vibración para feedback del golpe real
 */
#ifndef CAMPANA_H
	#define CAMPANA_H

        #include <Arduino.h>
        #include "Debug.h"
        #include "Configuracion.h"

        #define TiempoBadajoOn 200                                          //!< Tiempo que se mantiene el pin de la campana activo (1 segundo)


        class CAMPANA 
        {
            public:

                CAMPANA(int nPin );                                         //!< Constructor con pin 
                ~CAMPANA();                                                 //!< Destructor por defecto
                void Toca (void);                                           //!< Activa la campana durante un tiempo predeterminado
           private:
                int _nPin;                                                  //!< Pin de la campana    
        };

#endif