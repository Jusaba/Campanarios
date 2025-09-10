/**
 * @file I2CServicio.h
 * @brief Módulo de comunicación I2C pura para interfaz con ESP32 del campanario
 * 
 * @details Este módulo proporciona una capa de abstracción limpia para la comunicación
 *          I2C entre el M5Dial (maestro) y el ESP32 del campanario (esclavo). Se enfoca
 *          exclusivamente en la transmisión y recepción de datos, sin implementar
 *          lógica de negocio.
 *          
 *          **Características principales:**
 *          - Comunicación I2C bidireccional confiable
 *          - Gestión automática de errores de transmisión
 *          - Estructura de respuesta unificada para todas las operaciones
 *          - Separación clara entre datos brutos y lógica de aplicación
 *          - Funciones especializadas para diferentes tipos de consulta
 * 
 * @note Este módulo NO contiene lógica de negocio, solo comunicación I2C
 * @warning Requiere que el bus I2C esté inicializado antes de usar
 * 
 * @author Julian Salas Bartolomé
 * @date 2025-06-15
 * @version 1.0
 * 
 * @copyright Copyright (c) 2025
 * 
 * @see Configuracion.h, Debug.h, Auxiliar.h
 * @see Config::I2C::CAMPANARIO_ADDRESS
 */

#ifndef I2CSERVICIO_H
    #define I2CSERVICIO_H

    #include <Wire.h>
    #include "Configuracion.h"
    #include "Debug.h"

    // ============================================================================
    // ESTRUCTURA DE DATOS - Solo para comunicación I2C
    // ============================================================================
    /**
     * @brief Estructura para encapsular respuestas de comunicación I2C
     * 
     * @details Proporciona una interfaz unificada para todas las operaciones I2C,
     *          encapsulando tanto el éxito/fallo de la comunicación como los datos
     *          recibidos. Permite un manejo consistente de errores y datos en
     *          todas las funciones del módulo.
     * 
     * @note Máximo 8 bytes de datos por respuesta I2C
     * @see I2C_SolicitarEstado(), I2C_SolicitarEstadoHora(), I2C_SolicitarEstadoFechaHora()
     * 
     * @since v1.0
     */
        struct I2CResponse {
            bool success;           // Si la comunicación fue exitosa
            uint8_t data[8];        // Datos recibidos (máximo 8 bytes)
            uint8_t length;         // Cantidad de bytes recibidos
        };

        // ============================================================================
        // FUNCIONES INICIALIZACIÓN I2C
        // ============================================================================

            bool I2C_Inicializar();

        // ============================================================================
        // FUNCIONES DE ENVÍO I2C - Solo comunicación, sin lógica de negocio
        // ============================================================================

            bool I2C_EnviarComando(int comando);
            bool I2C_EnviarComandoConParametro(int comando, int parametro);

        // ============================================================================
        // FUNCIONES DE REQUEST I2C - Solo comunicación, devuelven datos raw
        // ============================================================================

            I2CResponse I2C_SolicitarEstado();
            I2CResponse I2C_SolicitarEstadoHora();
            I2CResponse I2C_SolicitarEstadoFechaHora(bool incluirTemporizacion = false);

#endif
