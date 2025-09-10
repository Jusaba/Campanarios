


//#include "I2CServicio.h"

#include "I2CServicio.h"

    // ============================================================================
    // FUNCIONES INICIALIZACIÓN I2C
    // ============================================================================

    /**
     * @brief Inicializa el bus I2C para comunicación con el campanario
     * 
     * @details Configura e inicializa el bus I2C del M5Dial como maestro para
     *          comunicación con el ESP32 del campanario (esclavo). Establece
     *          velocidad de comunicación, pines SDA/SCL y parámetros de timeout.
     *          
     *          Esta función debe llamarse una sola vez durante la inicialización
     *          del sistema, antes de cualquier operación I2C.
     * 
     * @return true si la inicialización del bus I2C fue exitosa
     * @return false si hubo error en la configuración del bus I2C
     * 
     * @note DEBE llamarse en setup() antes de usar cualquier función I2C
     * @warning Solo llamar una vez durante la inicialización del sistema
     * @warning Verificar conexiones físicas SDA/SCL antes de llamar
     * 
     * @see Wire.begin(), Config::I2C configuración
     * @see setup() en DialCampanario.ino
     * 
     * @since v1.0
     */
        bool I2C_Inicializar() {
            Wire.begin();
            Wire.setClock(Config::I2C::FREQUENCY);  // 100kHz
        
            DBG_I2C("I2C_Inicializar->I2C inicializado");
        
            // Test de comunicación
            Wire.beginTransmission(Config::I2C::SLAVE_ADDR);
            uint8_t result = Wire.endTransmission();

            if (result == 0) {
                DBG_I2C("I2C_Inicializar->Comunicación I2C OK");
                return true;
            } else {
                DBG_I2C_PRINTF("I2C_Inicializar->Sin comunicación I2C, error: %d", result);
                return false;
            }
        }

    // ============================================================================
    // FUNCIONES DE ENVÍO - Solo I2C, sin lógica de negocio
    // ============================================================================

    /**
     * @brief Envía un comando simple al ESP32 campanario via I2C
     * 
     * @details Transmite un comando de un byte al ESP32 esclavo usando el protocolo
     *          I2C estándar. Esta función es para comandos que no requieren parámetros
     *          adicionales, como encender/apagar sistemas, cambiar modos, etc.
     *          
     *          Maneja automáticamente la gestión del bus I2C y proporciona
     *          retroalimentación del éxito de la transmisión.
     * 
     * @param comando Valor entero del comando a enviar (se convierte a uint8_t)
     *                Debe corresponder a valores definidos en Config::States::I2CState
     * 
     * @return true si la transmisión I2C fue exitosa
     * @return false si hubo error en la comunicación I2C
     * 
     * @note No valida el contenido del comando, solo maneja la transmisión
     * @warning El comando debe ser un valor válido para el protocolo del campanario
     * 
     * @see I2C_EnviarComandoConParametro(), Config::States::I2CState
     * @see Config::I2C::CAMPANARIO_ADDRESS
     * 
     * @author: Julian Salas Bartolomé
     * @since v1.0
     */
        bool I2C_EnviarComando(int comando) {
            DBG_I2C_PRINTF("I2C_EnviarComando->Enviando comando: %d", comando);

            Wire.beginTransmission(Config::I2C::SLAVE_ADDR);
            Wire.write(comando);
            uint8_t result = Wire.endTransmission();

            if (result == 0) {
                DBG_I2C_PRINTF("I2C_EnviarComando->Comando %d enviado correctamente", comando);
                return true;
            } else {
                DBG_I2C_PRINTF("I2C_EnviarComando->ERROR al enviar comando %d, código: %d", comando, result);
                return false;
            }
        }

    /**
     * @brief Envía un comando con parámetro al ESP32 campanario via I2C
     * 
     * @details Transmite un comando de dos bytes (comando + parámetro) al ESP32 esclavo.
     *          Usado para comandos que requieren configuración adicional, como
     *          temporizadores, valores de configuración, niveles de intensidad, etc.
     *          
     *          El primer byte es el comando y el segundo byte es el parámetro asociado.
     *          Gestiona automáticamente la secuencia de transmisión I2C.
     * 
     * @param comando Valor entero del comando principal a enviar
     *                Debe corresponder a valores definidos en Config::States::I2CState
     * @param parametro Valor entero del parámetro asociado al comando
     *                  Rango y significado depende del comando específico
     * 
     * @return true si la transmisión I2C fue exitosa
     * @return false si hubo error en la comunicación I2C
     * 
     * @note Principalmente usado para SET_TEMPORIZADOR con minutos como parámetro
     * @warning Ambos valores deben estar en rango 0-255 (limitación uint8_t)
     * 
     * @see I2C_EnviarComando(), Config::States::I2CState::SET_TEMPORIZADOR
     * @see EnviarTemporizacion() en Auxiliar.h para uso específico
     * 
     * @since v1.0
     */
        bool I2C_EnviarComandoConParametro(int comando, int parametro) {
            DBG_I2C_PRINTF("I2C_EnviarComandoConParametro->Enviando comando: %d con parámetro: %d", comando, parametro);

            Wire.beginTransmission(Config::I2C::SLAVE_ADDR);
            Wire.write(comando);
            Wire.write(parametro);
            uint8_t result = Wire.endTransmission();

            if (result == 0) {
                DBG_I2C_PRINTF("I2C_EnviarComandoConParametro->Comando %d con parámetro %d enviado", comando, parametro);
                return true;
            } else {
                DBG_I2C_PRINTF("I2C_EnviarComandoConParametro->ERROR comando %d con parámetro %d, código: %d", comando, parametro, result);
                return false;
            }
        }

// ============================================================================
// FUNCIONES DE REQUEST I2C - Solo comunicación, devuelven datos raw
// ============================================================================

    /**
     * @brief Solicita únicamente el estado actual del campanario via I2C
     * 
     * @details Función de consulta más básica que obtiene solo el byte de estado
     *          del ESP32 campanario. Útil para verificaciones rápidas de estado
     *          sin necesidad de información temporal. Mínima transferencia de datos.
     *          
     *          Esta función es la más eficiente en términos de ancho de banda I2C
     *          y tiempo de respuesta.
     * 
     * @return I2CResponse Estructura con:
     *         - success: true si la comunicación fue exitosa
     *         - data[0]: Estado actual del campanario (flags de bits)
     *         - length: 1 (solo un byte de estado)
     * 
     * @note Respuesta mínima: 1 byte (solo estado)
     * @note Función más rápida para consultas de estado simple
     * 
     * @see I2C_SolicitarEstadoHora(), I2C_SolicitarEstadoFechaHora()
     * @see Config::States para interpretación de bits de estado
     * 
     * @since v1.0
     */
        I2CResponse I2C_SolicitarEstado() {
            I2CResponse response = {false, {0}, 0};

            DBG_I2C("I2C_SolicitarEstado->Solicitando estado...");

            Wire.beginTransmission(Config::I2C::SLAVE_ADDR);
            Wire.write(Config::States::I2CState::CAMPANARIO);
            Wire.endTransmission();

            Wire.requestFrom(Config::I2C::SLAVE_ADDR, 1);
            if (Wire.available()) {
                response.data[0] = Wire.read();
                response.length = 1;
                response.success = true;
                DBG_I2C_PRINTF("I2C_SolicitarEstado->Estado recibido: %d", response.data[0]);
            } else {
                DBG_I2C("I2C_SolicitarEstado->Sin respuesta del campanario");
            }

            return response;
        }

    /**
     * @brief Solicita estado y hora actual del campanario via I2C
     * 
     * @details Obtiene información básica temporal junto con el estado del sistema.
     *          Proporciona un balance entre información temporal y eficiencia de
     *          comunicación. Ideal para aplicaciones que necesitan hora actual
     *          sin fecha completa.
     *          
     *          Datos recibidos en orden: estado, hora, minutos, segundos.
     * 
     * @return I2CResponse Estructura con:
     *         - success: true si la comunicación fue exitosa
     *         - data[0]: Estado actual del campanario (flags de bits)
     *         - data[1]: Hora actual (0-23)
     *         - data[2]: Minutos actuales (0-59)
     *         - data[3]: Segundos actuales (0-59)
     *         - length: 4 (estado + hora + minutos + segundos)
     * 
     * @note Respuesta esperada: 4 bytes (estado + tiempo)
     * @note Función intermedia entre estado simple y fecha/hora completa
     * 
     * @see I2C_SolicitarEstado(), I2C_SolicitarEstadoFechaHora()
     * @see SolicitarEstadoHora() en Auxiliar.h para lógica de negocio
     * 
     * @author Julian Salas Bartolomé
     * @since v1.0
     */        
        I2CResponse I2C_SolicitarEstadoHora() {
            I2CResponse response = {false, {0}, 0};

            DBG_I2C("I2C_SolicitarEstadoHora->Solicitando estado y hora...");

            Wire.beginTransmission(Config::I2C::SLAVE_ADDR);
            Wire.write(Config::States::I2CState::HORA);
            Wire.endTransmission();

            Wire.requestFrom(Config::I2C::SLAVE_ADDR, 4);
            if (Wire.available() >= 4) {
                for (int i = 0; i < 4 && Wire.available(); i++) {
                    response.data[i] = Wire.read();
                }
                response.length = 4;
                response.success = true;
                DBG_I2C_PRINTF("I2C_SolicitarEstadoHora->Datos recibidos: [%d, %d, %d, %d]", 
                              response.data[0], response.data[1], response.data[2], response.data[3]);
            } else {
                DBG_I2C("I2C_SolicitarEstadoHora->Sin respuesta del campanario");
            }

            return response;
        }

    /**
     * @brief Solicita estado, fecha y hora completa del campanario via I2C
     * 
     * @details Función de consulta más completa que obtiene toda la información
     *          temporal disponible del ESP32 campanario. Incluye fecha completa
     *          y hora detallada junto con el estado del sistema.
     *          
     *          Puede incluir opcionalmente datos de temporización según el parámetro.
     *          Esta es la consulta más completa pero también la que requiere más
     *          ancho de banda I2C.
     * 
     * @param incluirTemporizacion true para solicitar datos adicionales de temporización,
     *                            false para consulta estándar de fecha/hora
     * 
     * @return I2CResponse Estructura con:
     *         - success: true si la comunicación fue exitosa
     *         - data[0]: Estado actual del campanario (flags de bits)
     *         - data[1]: Día del mes (1-31)
     *         - data[2]: Mes (1-12)
     *         - data[3]: Año (últimos 2 dígitos)
     *         - data[4]: Hora (0-23)
     *         - data[5]: Minutos (0-59)
     *         - data[6]: Segundos (0-59)
     *         - length: 7 (estado + fecha completa + hora completa)
     * 
     * @note Respuesta estándar: 7 bytes (estado + fecha + hora)
     * @note Función más completa pero con mayor uso de ancho de banda I2C
     * @warning Puede devolver datos adicionales si incluirTemporizacion es true
     * 
     * @see I2C_SolicitarEstado(), I2C_SolicitarEstadoHora()
     * @see SolicitarEstadoFechaHora() en Auxiliar.h para lógica de negocio
     * 
     * @author Julian Salas Bartolomé
     * @since v1.0
     */
        I2CResponse I2C_SolicitarEstadoFechaHora(bool incluirTemporizacion) {
            I2CResponse response = {false, {0}, 0};

            // Seleccionar comando según parámetro:
            uint8_t comando = incluirTemporizacion ? 
                             Config::States::I2CState::FECHA_HORA_O_TEMPORIZACION : 
                             Config::States::I2CState::FECHA_HORA;

            const char* tipoSolicitud = incluirTemporizacion ? 
                                       "estado, fecha, hora y temporización" : 
                                       "estado, fecha y hora";

            DBG_I2C_PRINTF("I2C_SolicitarEstadoFechaHora->Solicitando %s...", tipoSolicitud);

            Wire.beginTransmission(Config::I2C::SLAVE_ADDR);
            Wire.write(comando);                                    // ← ÚNICO CAMBIO: comando variable
            Wire.endTransmission();

            Wire.requestFrom(Config::I2C::SLAVE_ADDR, 7);
            if (Wire.available() >= 7) {
                // LÓGICA COMÚN (antes duplicada):
                for (int i = 0; i < 7 && Wire.available(); i++) {
                    response.data[i] = Wire.read();
                }
                response.length = 7;
                response.success = true;
                DBG_I2C_PRINTF("I2C_SolicitarEstadoFechaHora->7 bytes recibidos correctamente (%s)", tipoSolicitud);
            } else {
                DBG_I2C_PRINTF("I2C_SolicitarEstadoFechaHora->Sin respuesta del campanario (%s)", tipoSolicitud);
            }

            return response;
        }



