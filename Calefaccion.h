/**
 * @file Calefaccion.h
 * @brief Implementación del sistema de control de calefacción para campanario
 * 
 * @details Este módulo implementa un sistema simple pero robusto de control
 *          de calefacción específico para el entorno del campanario, que incluye:
 *          
 *          **FUNCIONALIDADES PRINCIPALES:**
 *          - Control digital ON/OFF mediante relé
 *          - Temporizador automático con auto-apagado
 *          - Verificación de tiempo restante en tiempo real
 *          - Integración completa con RTC para temporización precisa
 *          - Control de estado con feedback inmediato
 *          - Gestión segura del pin de control
 *          
 *          **CARACTERÍSTICAS DE SEGURIDAD:**
 *          - Auto-apagado por temporizador 
 *          - Verificación continua del tiempo transcurrido
 *          - Estado persistente durante la ejecución
 *          
 *          **INTEGRACIÓN SISTEMA:**
 *          - Compatible con sistema de alarmas para programación automática
 *          - Utiliza RTC sincronizado para temporización precisa
 *          - Interfaz simple para control manual y automático
 *          - Debug integrado para monitorización
 * 
 * @note **TEMPORIZACIÓN:** Utiliza struct tm y time.h para cálculos precisos
 * @note **HARDWARE:** Diseñado para relés de estado sólido o mecánicos
 * @note **SEGURIDAD:** Auto-apagado obligatorio - nunca permanece encendido indefinidamente
 * 
 * @warning **RTC REQUERIDO:** Requiere RTC sincronizado para funcionamiento correcto
 * @warning **PIN VÁLIDO:** Verificar que el pin GPIO es válido y no está en uso
 * @warning **CARGA ELÉCTRICA:** Verificar que el relé soporta la carga de la calefacción
 * 
 * @author Julian Salas Bartolomé
 * @date 2025-09-16
 * @version 1.0
 * 
 * @see Calefaccion.h - Definiciones de la clase
 * @see RTC.h - Sistema de tiempo utilizado para temporización
 * @see Acciones.h - Funciones que utilizan esta clase (accionCalefaccion)
 * 
 * @todo Implementar log de uso para estadísticas de consumo
 * @todo Añadir protección contra encendidos muy frecuentes
 * @todo Integrar sensor de corriente para verificar funcionamiento real
 */
#ifndef CALEFACCION_H
	#define CALEFACCION_H

        #include <Arduino.h>
        #include <time.h>
        #include "Debug.h"
        #include "Configuracion.h"



        class CALEFACCION
        {
            public:

                CALEFACCION(int nPin );                                                 //!< Constructor con pin 
                ~CALEFACCION();
                void Enciende(int nMinutos);                                            //!< Enciende la calefacción
                void Apaga(void);                                                       //!< Apaga la calefacción
                bool GetEstado(void);                                                   //!< Devuelve el estado de la calefacción (true si está encendida, false si está apagada)
                double VerificarTemporizador(void);                                     //!< Verifica si debe apagarse automáticamente por temporizador y devuelve los segundos que faltan para apagarse
           private:

                int _nPin;                                                              //!< Pin de la campana    
                bool _lCalefaccion = false;                                             //!< Estado de la calefacción del campanario
                int _nMinutosOn = 0;                                                    //!< Número de minutos que se ha solicitado encender la calefacción
                struct tm _tiempoEncendido;                                             //!< Tiempo (RTC) cuando se encendió la calefacción
                 
        };
#endif