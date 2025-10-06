#include "Alarmas.h"



// ============================================================================
// IMPLEMENTACIÓN DE MÉTODOS PÚBLICOS
// ============================================================================

/**
 * @brief Inicializa el sistema de alarmas y opcionalmente carga configuración por defecto
 * 
 * @details Función de inicialización que prepara el sistema de alarmas para su uso.
 *          Limpia cualquier configuración previa y opcionalmente carga un conjunto
 *          de alarmas predeterminadas para el funcionamiento básico del campanario.
 *          
 *          **PROCESO DE INICIALIZACIÓN:**
 *          1. Llama a clear() para limpiar alarmas existentes
 *          2. Si cargarPorDefecto == true: ejecuta initDefaults()
 *          3. Inicializa variables internas del sistema
 *          4. Prepara estructuras para verificación temporal
 * 
 * @param cargarPorDefecto Si true, carga alarmas predeterminadas del sistema
 * @return true siempre (inicialización exitosa garantizada)
 * 
 * @note **ALARMAS POR DEFECTO:** Incluyen misas dominicales, toques de horas y sincronización
 * @note **LLAMADA ÚNICA:** Debe llamarse una sola vez durante el arranque del sistema
 * @note **ORDEN:** Llamar después de inicializar RTC pero antes del loop principal
 * 
 * @see clear() - Función que limpia las alarmas existentes
 * @see initDefaults() - Función que carga configuración predeterminada
 * 
 * @example
 * @code
 * AlarmScheduler alarmas;
 * 
 * void setup() {
 *     // Inicializar con alarmas por defecto
 *     alarmas.begin(true);
 * }
 * @endcode
 * 
 * @since v1.0
 * @author Julian Salas Bartolomé
 */
bool AlarmScheduler::begin(bool cargarPorDefecto) {
    clear();
    
    // ✅ CARGAR ALARMAS PERSONALIZABLES DESDE JSON ANTES DE LAS POR DEFECTO
    DBG_ALM("[ALARM] Cargando alarmas personalizables desde SPIFFS...");
    cargarPersonalizablesDesdeJSON();
    
    // ✅ SOLO CARGAR POR DEFECTO SI NO HAY NINGUNA ALARMA
    if (cargarPorDefecto && _num == 0) {
        DBG_ALM("[ALARM] No hay alarmas, cargando configuración por defecto...");
        initDefaults();
    } else if (cargarPorDefecto) {
        // Si ya hay alarmas personalizables, solo añadir las de sistema
        DBG_ALM("[ALARM] Añadiendo alarmas de sistema a las personalizables existentes...");
        initDefaults();
    }
    
    DBG_ALM_PRINTF("[ALARM] Sistema inicializado con %u alarmas\n", _num);
    this->_siguienteIdWeb = 1;
    return true;
}

/**
 * @brief Añade alarma que ejecuta método miembro de la clase AlarmScheduler
 * 
 * @details Registra una nueva alarma que ejecutará un método específico de la
 *          clase AlarmScheduler cuando se cumplan las condiciones temporales.
 *          Útil para acciones internas que requieren acceso a datos privados.
 *          
 * @param mascaraDias Máscara de días de la semana (DOW_* combinados con OR)
 * @param hora Hora de ejecución (0-23 o ALARMA_WILDCARD para cualquier hora)
 * @param minuto Minuto de ejecución (0-59 o ALARMA_WILDCARD para cualquier minuto)
 * @param intervaloMin Intervalo en minutos (0 = una vez, >0 = repetir cada X minutos)
 * @param accion Puntero a método miembro a ejecutar
 * @param parametro Parámetro uint16_t a pasar al método (valor por defecto: 0)
 * @param habilitada Estado inicial de la alarma (valor por defecto: true)
 * 
 * @return Índice de la alarma creada (0-15) o 255 si no hay espacio disponible
 * 
 * @note **MÉTODO MIEMBRO:** Signatura debe ser void metodo(uint16_t parametro)
 * @note **ACCESO PRIVADO:** El método puede acceder a miembros privados de la clase
 * @note **CONTEXTO:** Se ejecuta en el contexto del objeto AlarmScheduler
 * 
 * @warning **LÍMITE:** Máximo 16 alarmas simultáneas (MAX_ALARMAS)
 * @warning **PUNTERO VÁLIDO:** Verificar que el método existe y es accesible
 * 
 * @see addExternal() - Para funciones externas con parámetro
 * @see addExternal0() - Para funciones externas sin parámetro
 * 
 * @example
 * @code
 * // Añadir alarma que ejecuta método interno cada día a las 12:00
 * uint8_t idx = add(DOW_TODOS, 12, 0, 0, &AlarmScheduler::metodoInterno, 123);
 * @endcode
 * 
 * @since v1.0
 * @author Julian Salas Bartolomé
 */
uint8_t AlarmScheduler::add(uint8_t mascaraDias,
                            uint8_t hora,
                            uint8_t minuto,
                            uint16_t intervaloMin,
                            void (AlarmScheduler::*accion)(uint16_t),
                            uint16_t parametro,
                            bool habilitada)
{
    if (_num >= MAX_ALARMAS) {
        DBG_ALM_PRINTF("[ALARM] Error: Máximo de alarmas alcanzado (%u)\n", MAX_ALARMAS);
        return 255;
    }
    
    Alarm &oAlarma = _alarmas[_num];
    oAlarma.habilitada        = habilitada;
    oAlarma.mascaraDias       = (mascaraDias ? mascaraDias : DOW_TODOS);
    oAlarma.hora              = hora;
    oAlarma.minuto            = minuto;
    oAlarma.intervaloMin      = intervaloMin;
    oAlarma.ultimoDiaAno      = -1;
    oAlarma.ultimoMinuto      = 255;
    oAlarma.ultimaEjecucion   = 0;
    oAlarma.accion            = accion;
    oAlarma.accionExt         = nullptr;
    oAlarma.accionExt0        = nullptr;
    oAlarma.parametro         = parametro;
    
    DBG_ALM_PRINTF("[ALARM] Añadida alarma método idx=%u, dias=0x%02X, %02u:%02u, intervalo=%u min, param=%u\n",
                   _num, mascaraDias, hora, minuto, intervaloMin, parametro);
    
    return _num++;
}

/**
 * @brief Añade alarma que ejecuta función externa con parámetro
 * 
 * @details Registra una nueva alarma que ejecutará una función global externa
 *          que acepta un parámetro uint16_t cuando se cumplan las condiciones
 *          temporales especificadas.
 *          
 * @param mascaraDias Máscara de días de la semana (DOW_* combinados con OR)
 * @param hora Hora de ejecución (0-23 o ALARMA_WILDCARD para cualquier hora)
 * @param minuto Minuto de ejecución (0-59 o ALARMA_WILDCARD para cualquier minuto)
 * @param intervaloMin Intervalo en minutos (0 = una vez, >0 = repetir cada X minutos)
 * @param ext Puntero a función externa a ejecutar
 * @param parametro Parámetro uint16_t a pasar a la función (valor por defecto: 0)
 * @param habilitada Estado inicial de la alarma (valor por defecto: true)
 * 
 * @return Índice de la alarma creada (0-15) o 255 si no hay espacio disponible
 * 
 * @note **FUNCIÓN EXTERNA:** Signatura debe ser void funcion(uint16_t parametro)
 * @note **PARÁMETRO ÚTIL:** Para ID de secuencias, configuraciones, etc.
 * @note **SCOPE GLOBAL:** La función debe ser accesible globalmente
 * 
 * @warning **PUNTERO VÁLIDO:** Verificar que la función existe y está linkeada
 * @warning **CONTEXTO:** Se ejecuta fuera del contexto de la clase
 * 
 * @see add() - Para métodos miembro de la clase
 * @see addExternal0() - Para funciones externas sin parámetro
 * @see Acciones.h - Funciones externas predefinidas disponibles
 * 
 * @example
 * @code
 * // Añadir alarma que ejecuta secuencia de misa los domingos
 * uint8_t idx = addExternal(DOW_DOMINGO, 11, 30, 0, accionSecuencia, Config::States::MISA);
 * @endcode
 * 
 * @since v1.0
 * @author Julian Salas Bartolomé
 */
uint8_t AlarmScheduler::addExternal(uint8_t mascaraDias,
                                    uint8_t hora,
                                    uint8_t minuto,
                                    uint16_t intervaloMin,
                                    void (*ext)(uint16_t),
                                    uint16_t parametro,
                                    bool habilitada)
{
    if (_num >= MAX_ALARMAS) {
        DBG_ALM_PRINTF("[ALARM] Error: Máximo de alarmas alcanzado (%u)\n", MAX_ALARMAS);
        return 255;
    }
    
    Alarm &oAlarma = _alarmas[_num];
    oAlarma.habilitada        = habilitada;
    oAlarma.mascaraDias       = (mascaraDias ? mascaraDias : DOW_TODOS);
    oAlarma.hora              = hora;
    oAlarma.minuto            = minuto;
    oAlarma.intervaloMin      = intervaloMin;
    oAlarma.ultimoDiaAno      = -1;
    oAlarma.ultimoMinuto      = 255;
    oAlarma.ultimaEjecucion   = 0;
    oAlarma.accion            = nullptr;
    oAlarma.accionExt         = ext;
    oAlarma.accionExt0        = nullptr;
    oAlarma.parametro         = parametro;
    
    DBG_ALM_PRINTF("[ALARM] Añadida alarma externa idx=%u, dias=0x%02X, %02u:%02u, intervalo=%u min, param=%u\n",
                   _num, mascaraDias, hora, minuto, intervaloMin, parametro);
    
    return _num++;
}

/**
 * @brief Añade alarma que ejecuta función externa sin parámetros
 * 
 * @details Registra una nueva alarma que ejecutará una función global externa
 *          que no requiere parámetros cuando se cumplan las condiciones
 *          temporales especificadas. Ideal para funciones de mantenimiento
 *          y sincronización del sistema.
 *          
 * @param mascaraDias Máscara de días de la semana (DOW_* combinados con OR)
 * @param hora Hora de ejecución (0-23 o ALARMA_WILDCARD para cualquier hora)
 * @param minuto Minuto de ejecución (0-59 o ALARMA_WILDCARD para cualquier minuto)
 * @param intervaloMin Intervalo en minutos (0 = una vez, >0 = repetir cada X minutos)
 * @param ext0 Puntero a función externa sin parámetros a ejecutar
 * @param habilitada Estado inicial de la alarma (valor por defecto: true)
 * 
 * @return Índice de la alarma creada (0-15) o 255 si no hay espacio disponible
 * 
 * @note **FUNCIÓN EXTERNA:** Signatura debe ser void funcion(void)
 * @note **SIN PARÁMETROS:** Ideal para funciones de sistema (sync, mantenimiento)
 * @note **EJEMPLOS:** SincronizaNTP, ActualizaDNS, accionTocaHora
 * 
 * @warning **PUNTERO VÁLIDO:** Verificar que la función existe y está linkeada
 * @warning **SCOPE GLOBAL:** La función debe estar declarada globalmente
 * 
 * @see addExternal() - Para funciones con parámetro
 * @see add() - Para métodos miembro de la clase
 * @see Acciones.h - Funciones sin parámetros predefinidas
 * 
 * @example
 * @code
 * // Sincronización NTP diaria al mediodía
 * uint8_t idx = addExternal0(DOW_TODOS, 12, 0, 0, SincronizaNTP);
 * 
 * // Tocar horas en punto (wildcard para todas las horas)
 * uint8_t idx2 = addExternal0(DOW_TODOS, ALARMA_WILDCARD, 0, 0, accionTocaHora);
 * @endcode
 * 
 * @since v2.0
 * @author Julian Salas Bartolomé
 */
uint8_t AlarmScheduler::addExternal0(uint8_t mascaraDias,
                                     uint8_t hora,
                                     uint8_t minuto,
                                     uint16_t intervaloMin,
                                     void (*ext0)(),
                                     bool habilitada)
{
    if (_num >= MAX_ALARMAS) {
        DBG_ALM_PRINTF("[ALARM] Error: Máximo de alarmas alcanzado (%u)\n", MAX_ALARMAS);
        return 255;
    }
    
    Alarm &oAlarma = _alarmas[_num];
    oAlarma.habilitada        = habilitada;
    oAlarma.mascaraDias       = (mascaraDias ? mascaraDias : DOW_TODOS);
    oAlarma.hora              = hora;
    oAlarma.minuto            = minuto;
    oAlarma.intervaloMin      = intervaloMin;
    oAlarma.ultimoDiaAno      = -1;
    oAlarma.ultimoMinuto      = 255;
    oAlarma.ultimaEjecucion   = 0;
    oAlarma.accion            = nullptr;
    oAlarma.accionExt         = nullptr;
    oAlarma.accionExt0        = ext0;
    oAlarma.parametro         = 0;  // No se utiliza pero se inicializa
    
    DBG_ALM_PRINTF("[ALARM] Añadida alarma externa0 idx=%u, dias=0x%02X, %02u:%02u, intervalo=%u min\n",
                   _num, mascaraDias, hora, minuto, intervaloMin);
    
    return _num++;
}

/**
 * @brief Motor principal del sistema de alarmas - verifica y ejecuta alarmas programadas
 * 
 * @details Función central que debe ser llamada regularmente (típicamente desde loop())
 *          para verificar si alguna alarma programada debe ejecutarse en el momento actual.
 *          Implementa lógica compleja de verificación temporal y prevención de duplicados.
 *          
 *          **ALGORITMO DE VERIFICACIÓN:**
 *          1. Obtiene tiempo actual del RTC via getLocalTime()
 *          2. Para cada alarma habilitada:
 *             - Verifica máscara de días de la semana
 *             - Aplica lógica de horario (fijo vs wildcard vs intervalo)
 *             - Previene ejecuciones duplicadas con cache temporal
 *             - Verifica que no hay secuencia activa en Campanario
 *          3. Ejecuta acción apropiada (método, función externa con/sin parámetro)
 *          4. Actualiza cache temporal para prevenir re-ejecución
 *          
 *          **TIPOS DE HORARIOS SOPORTADOS:**
 *          - **Fijo:** Hora y minuto específicos (ej: 12:30)
 *          - **Wildcard:** ALARMA_WILDCARD para cualquier hora/minuto
 *          - **Intervalo:** Repetición cada X minutos desde punto de anclaje
 *          
 *          **PREVENCIÓN DE DUPLICADOS:**
 *          - ultimoDiaAno + ultimoMinuto: Evita ejecución múltiple en mismo minuto
 *          - ultimaEjecucion: Timestamp epoch para control de intervalos
 *          - Verificación de secuencia activa: No ejecuta si campanario ocupado
 * 
 * @note **LLAMADA REGULAR:** Debe llamarse cada segundo desde loop() principal
 * @note **RESOLUCIÓN:** Resolución mínima de 1 minuto (verificación por minuto)
 * @note **RTC REQUERIDO:** Falla silenciosamente si getLocalTime() falla
 * @note **DEBUG PERIÓDICO:** Logging cada 5 segundos si DebugAlarma habilitado
 * 
 * @warning **RTC DEPENDENCY:** Requiere sincronización NTP previa para funcionar
 * @warning **BLOCKING PREVENTION:** No ejecuta si Campanario.GetEstadoSecuencia() == true
 * @warning **PERFORMANCE:** Evitar llamadas demasiado frecuentes (>1Hz recomendado)
 * 
 * @see getLocalTime() - Función ESP32 para obtener tiempo actual
 * @see Campanario.GetEstadoSecuencia() - Verificación de estado del sistema
 * @see ultimoDiaAno, ultimoMinuto, ultimaEjecucion - Variables de cache
 * 
 * @example
 * @code
 * AlarmScheduler alarmas;
 * 
 * void loop() {
 *     alarmas.check();  // Verificar alarmas programadas
 *     delay(1000);      // Verificar cada segundo
 * }
 * @endcode
 * 
 * @since v1.0 - Verificación básica de horarios
 * @since v2.0 - Intervalos, wildcards y prevención avanzada de duplicados
 * 
 * @author Julian Salas Bartolomé
 */
void AlarmScheduler::check() {

    if (!getLocalTime(&t)) return;                                                                      // Verificar que se obtiene tiempo válido del RTC
    
    uint8_t horaActual        = t.tm_hour;                                                              // Hora actual (0-23)
    uint8_t minutoActual      = t.tm_min;                                                                // Minuto actual (0-59)
    uint8_t mascaraDiaActual  = mascaraDesdeDiaSemana(t.tm_wday);                                       // Máscara de día actual
    int     diaAnoActual      = t.tm_yday;                                                              // Día del año actual
    time_t  ahora             = time(nullptr);                                                          // Tiempo actual en epoch

    #ifdef DEBUGALARMAS
        static uint32_t lastDbg = 0;
        if (millis() - lastDbg > 5000) {
            DBG_ALM_PRINTF("AlarmScheduler::check -> %02u:%02u DOW=%d YDay=%d, Alarmas=%u\n",
                      horaActual, minutoActual, t.tm_wday, diaAnoActual, this->_num);
            lastDbg = millis();
        }
    #endif

    for (uint8_t i = 0; i < _num; ++i) {                                                                    // Iterar sobre todas las alarmas registradas
        Alarm &oAlarma = _alarmas[i];                                                                       // Referencia a la alarma actual                     
        if (!oAlarma.habilitada) continue;                                                                  // Saltar si la alarma está deshabilitada          
        if (!(oAlarma.mascaraDias & mascaraDiaActual)) continue;                                            // Saltar si el día actual no está en la máscara

        bool disparar = false;                                                                              // Flag para indicar si se debe disparar la alarma             

        // Lógica para alarmas de intervalo 
        if (oAlarma.intervaloMin > 0) {                                                                     // Si es una alarma de intervalo            
            if (oAlarma.ultimaEjecucion == 0) {                                                             // Primera ejecución: verificar ancla de hora/minuto
                bool anclaOk = true;                                                                        // Asumir ancla válida 
                if (oAlarma.hora   != ALARMA_WILDCARD && oAlarma.hora   != horaActual)   anclaOk = false;   // Verificar hora
                if (oAlarma.minuto != ALARMA_WILDCARD && oAlarma.minuto != minutoActual) anclaOk = false;   // Verificar minuto
                if (anclaOk) disparar = true;                                                               // Si ancla válida, disparar alarma    
            } else if ((ahora - oAlarma.ultimaEjecucion) >= (time_t)(oAlarma.intervaloMin * 60)) {          // Ejecuciones posteriores: verificar intervalo transcurrido
                disparar = true;
            }
        } 
        // Lógica para alarmas de horario fijo/wildcard
        else {
            bool coincideHora = (oAlarma.hora == ALARMA_WILDCARD || oAlarma.hora == horaActual);
            bool coincideMinuto = (oAlarma.minuto == ALARMA_WILDCARD || oAlarma.minuto == minutoActual);

           if (coincideHora && coincideMinuto) {               
                bool yaEjecutadaEstaHora = false;
                
                if (oAlarma.hora == ALARMA_WILDCARD) {
                     yaEjecutadaEstaHora = (oAlarma.ultimoDiaAno == diaAnoActual && 
                                          oAlarma.ultimoMinuto == minutoActual &&
                                          oAlarma.ultimaHora == horaActual);  // ← NUEVA VARIABLE NECESARIA
                } else {
                    yaEjecutadaEstaHora = (oAlarma.ultimoDiaAno == diaAnoActual && 
                                          oAlarma.ultimoMinuto == minutoActual);
                }
                
                if (!yaEjecutadaEstaHora) {
                    disparar = true;
                }
            }        
        }

        if (!disparar) continue;
        
 
        // Ejecutar acción apropiada según tipo
        if (oAlarma.accion) {
            (this->*oAlarma.accion)(oAlarma.parametro);
            DBG_ALM_PRINTF("[ALARM] idx=%u ejecutada - método miembro, param=%u\n", i, oAlarma.parametro);
        } else if (oAlarma.accionExt) {
            oAlarma.accionExt(oAlarma.parametro);
            DBG_ALM_PRINTF("[ALARM] idx=%u ejecutada - función externa, param=%u\n", i, oAlarma.parametro);
        } else if (oAlarma.accionExt0) {
            oAlarma.accionExt0();
            DBG_ALM_PRINTF("[ALARM] idx=%u ejecutada - función externa sin parámetros\n", i);
        }

        // Actualizar cache para prevenir re-ejecución
        oAlarma.ultimoDiaAno     = diaAnoActual;
        oAlarma.ultimoMinuto     = minutoActual;
        oAlarma.ultimaHora       = horaActual;
        oAlarma.ultimaEjecucion  = ahora;
    }
}

/**
 * @brief Deshabilita una alarma específica sin eliminarla del sistema
 * 
 * @details Marca una alarma como inactiva temporalmente, manteniéndola en
 *          el array pero evitando que se ejecute durante las verificaciones.
 *          Útil para desactivación temporal sin perder la configuración.
 * 
 * @param idx Índice de la alarma a deshabilitar (0-15)
 * 
 * @note La alarma permanece en memoria con su configuración intacta
 * @note Puede ser reactivada posteriormente con enable()
 * @note Índices inválidos son ignorados silenciosamente
 * 
 * @see enable() - Para reactivar la alarma
 * @see get() - Para verificar estado de una alarma
 * 
 * @since v1.0
 * @author Julian Salas Bartolomé
 */
void AlarmScheduler::disable(uint8_t idx) { 
    if (idx < _num) {
        _alarmas[idx].habilitada = false;
        DBG_ALM_PRINTF("[ALARM] Alarma idx=%u deshabilitada\n", idx);
    }
}

/**
 * @brief Habilita una alarma específica previamente deshabilitada
 * 
 * @details Marca una alarma como activa para que sea considerada durante
 *          las verificaciones de check(). Útil para reactivar alarmas
 *          temporalmente deshabilitadas.
 * 
 * @param idx Índice de la alarma a habilitar (0-15)
 * 
 * @note La alarma debe existir en el array para ser habilitada
 * @note Índices inválidos son ignorados silenciosamente
 * @note No resetea el cache temporal (ultimaEjecucion, etc.)
 * 
 * @see disable() - Para desactivar la alarma
 * @see get() - Para verificar estado de una alarma
 * 
 * @since v1.0
 * @author Julian Salas Bartolomé
 */
void AlarmScheduler::enable(uint8_t idx) { 
    if (idx < _num) {
        _alarmas[idx].habilitada = true;
        DBG_ALM_PRINTF("[ALARM] Alarma idx=%u habilitada\n", idx);
    }
}

/**
 * @brief Elimina todas las alarmas del sistema
 * 
 * @details Resetea completamente el array de alarmas, eliminando todas
 *          las configuraciones programadas y liberando espacio para
 *          nuevas alarmas.
 * 
 * @note No libera memoria (array es estático), solo resetea contador
 * @note Todas las alarmas se pierden irreversiblemente
 * @note Llamada automáticamente por begin()
 * 
 * @see begin() - Función que llama automáticamente a clear()
 * @see count() - Para verificar número de alarmas antes de limpiar
 * 
 * @since v1.0
 * @author Julian Salas Bartolomé
 */
void AlarmScheduler::clear() { 
    _num = 0; 
    DBG_ALM_PRINTF("[ALARM] Todas las alarmas eliminadas\n");
}

/**
 * @brief Obtiene el número actual de alarmas configuradas
 * 
 * @details Retorna la cantidad de alarmas actualmente registradas
 *          en el sistema, tanto habilitadas como deshabilitadas.
 * 
 * @return Número de alarmas configuradas (0-16)
 * 
 * @note Incluye alarmas habilitadas y deshabilitadas
 * @note Máximo posible es MAX_ALARMAS (16)
 * @note Útil para verificar espacio disponible antes de añadir nuevas
 * 
 * @see MAX_ALARMAS - Constante con límite máximo
 * @see add(), addExternal(), addExternal0() - Funciones que incrementan este valor
 * 
 * @since v1.0
 * @author Julian Salas Bartolomé
 */
uint8_t AlarmScheduler::count() const { 
    return _num; 
}

/**
 * @brief Obtiene acceso de solo lectura a una alarma específica
 * 
 * @details Proporciona acceso de solo lectura a la configuración de una
 *          alarma específica para inspección o debug. No permite modificación
 *          directa de los datos.
 * 
 * @param idx Índice de la alarma a consultar (0-15)
 * @return Puntero constante a la alarma o nullptr si índice inválido
 * 
 * @note Retorna nullptr para índices fuera de rango
 * @note Solo permite lectura, no modificación de la alarma
 * @note Útil para debug e inspección de configuración
 * 
 * @see count() - Para verificar rango válido de índices
 * @see struct Alarm - Estructura de datos retornada
 * 
 * @example
 * @code
 * const Alarm* alarma = scheduler.get(0);
 * if (alarma) {
 *     Serial.printf("Alarma 0: %02u:%02u, habilitada=%u\n", 
 *                   alarma->hora, alarma->minuto, alarma->habilitada);
 * }
 * @endcode
 * 
 * @since v1.0
 * @author Julian Salas Bartolomé
 */
const Alarm* AlarmScheduler::get(uint8_t idx) const { 
    return (idx < _num) ? &_alarmas[idx] : nullptr; 
}

/**
 * @brief Determina si la hora actual está en horario nocturno
 * 
 * @details Evalúa si la hora actual del sistema está dentro del rango
 *          considerado como horario nocturno según la configuración del
 *          proyecto. Maneja correctamente el cambio de día.
 * 
 * @return true si está en horario nocturno, false en caso contrario
 * 
 * @note Horario nocturno típico: 22:00 - 07:00 (configurable)
 * @note Utiliza t.tm_hour (debe llamarse después de getLocalTime())
 * @note Maneja correctamente el cambio de día (ej: 23:00 - 06:00)
 * 
 * @warning Requiere llamada previa a getLocalTime() para actualizar t
 * 
 * @see Config::Time::NOCHE_INICIO_HORA - Hora de inicio de la noche
 * @see Config::Time::NOCHE_FIN_HORA - Hora de fin de la noche
 * @see check() - Función que actualiza t.tm_hour
 * 
 * @since v2.0
 * @author Julian Salas Bartolomé
 */
bool AlarmScheduler::esHorarioNocturno() const {
    return (t.tm_hour >= Config::Time::NOCHE_INICIO_HORA || 
            t.tm_hour < Config::Time::NOCHE_FIN_HORA);
}

// ============================================================================
// IMPLEMENTACIÓN DE MÉTODOS PRIVADOS
// ============================================================================

/**
 * @brief Convierte día de la semana (0-6) a máscara de bits
 * 
 * @details Función utilitaria estática que convierte el día de la semana
 *          del sistema (donde 0=Domingo, 1=Lunes, ..., 6=Sábado) a una
 *          máscara de bits compatible con el sistema de alarmas.
 * 
 * @param diaSemana Día de la semana (0=Domingo, 1=Lunes, ..., 6=Sábado)
 * @return Máscara de bits correspondiente (bit 0=Domingo, bit 1=Lunes, etc.)
 *         o 0 si el día está fuera del rango válido
 * 
 * @note Utilizada internamente por check() para comparar con mascaraDias
 * @note Convierte formato tm_wday a formato DOW_* del sistema
 * @note Función estática - no requiere instancia de la clase
 * 
 * @see DOW_DOMINGO, DOW_LUNES, etc. - Constantes de días definidas
 * @see struct tm - Estructura que contiene tm_wday
 * @see check() - Función que utiliza esta conversión
 * 
 * @since v2.0
 * @author Julian Salas Bartolomé
 */
uint8_t AlarmScheduler::mascaraDesdeDiaSemana(int diaSemana) {
    return (diaSemana >= 0 && diaSemana <= 6) ? (1 << diaSemana) : 0;
}

/**
 * @brief Resetea el cache temporal de todas las alarmas
 * 
 * @details Función utilitaria que limpia el cache de prevención de duplicados
 *          de todas las alarmas registradas. Útil después de cambios de hora
 *          del sistema, reinicializaciones o cuando se detectan problemas
 *          de ejecución de alarmas.
 *          
 *          **VARIABLES RESETEADAS:**
 *          - ultimoDiaAno: Se establece a -1 (valor inicial)
 *          - ultimoMinuto: Se establece a 255 (valor inicial)  
 *          - ultimaEjecucion: Se establece a 0 (timestamp inicial)
 *          
 *          Esto permite que las alarmas se ejecuten nuevamente sin esperar
 *          al cambio natural de día o configuración temporal.
 * 
 * @note **USO TÍPICO:** Después de sincronización NTP o ajustes de sistema
 * @note **EFECTO:** Permite re-ejecución inmediata de alarmas en el minuto actual
 * @note **SEGURIDAD:** No afecta configuración de alarmas, solo cache temporal
 * @note **DEBUG:** Util para depuración de problemas de ejecución de alarmas
 * 
 * @warning **DUPLICADOS:** Puede causar ejecución múltiple si se llama repetidamente
 * @warning **USO MODERADO:** Solo llamar cuando sea realmente necesario
 * 
 * @see check() - Función que utiliza el cache para prevenir duplicados
 * @see ultimoDiaAno, ultimoMinuto, ultimaEjecucion - Variables afectadas
 * @see begin() - Inicialización que resetea automáticamente el cache
 * 
 * @example
 * @code
 * // Después de sincronización manual
 * if (RTC::beginConMultiplesServidores()) {
 *     Alarmas.resetCache();  // Permitir nuevas ejecuciones
 * }
 * 
 * // Para debug de alarmas que no se ejecutan
 * Alarmas.resetCache();
 * Serial.println("Cache de alarmas reseteado para debug");
 * @endcode
 * 
 * @since v2.1 - Función añadida para depuración y mantenimiento
 * @author Julian Salas Bartolomé
 */
void AlarmScheduler::resetCache() {
    for (uint8_t i = 0; i < _num; ++i) {
        _alarmas[i].ultimoDiaAno = -1;
        _alarmas[i].ultimoMinuto = 255;
        _alarmas[i].ultimaHora = 255;
        _alarmas[i].ultimaEjecucion = 0;
    }
    DBG_ALM_PRINTF("[ALARM] Cache de %u alarmas reseteado\n", _num);
}


/**
 * @brief Carga configuración de alarmas predeterminadas del sistema
 * 
 * @details Función privada que configura un conjunto básico de alarmas
 *          necesarias para el funcionamiento estándar del campanario.
 *          Incluye misas dominicales, toques de horas y tareas de mantenimiento.
 *          
 *          **ALARMAS PREDETERMINADAS:**
 *          - Misas dominicales a las 11:05 y 11:25
 *          - Toques de horas en punto (wildcard para todas las horas)
 *          - Toques de medias horas (:30)
 *          - Sincronización NTP diaria (12:02)
 *          - Actualización DNS (12:05)
 * 
 * @note Llamada automáticamente por begin() si cargarPorDefecto == true
 * @note Todas las alarmas se crean habilitadas por defecto
 * @note Utiliza funciones de Acciones.h para las implementaciones
 * 
 * @see begin() - Función que llama a initDefaults()
 * @see Acciones.h - Funciones utilizadas (accionSecuencia, accionTocaHora, etc.)
 * @see Config::States::MISA - Constante para secuencias de misa
 * 
 * @since v1.0 - Configuración básica
 * @since v2.0 - Añadidas funciones de mantenimiento (NTP, DNS)
 * 
 * @author Julian Salas Bartolomé
 */
void AlarmScheduler::initDefaults() {
    DBG_ALM_PRINTF("[ALARM] Cargando configuración por defecto...\n");
    
    // ===== MISAS DOMINICALES =====
    //addExternal(DOW_DOMINGO, 11, 5,  0, accionSecuencia, Config::States::MISA, true);
    //addExternal(DOW_DOMINGO, 11, 25, 0, accionSecuencia, Config::States::MISA, true);

    // ===== CUARTOS Y MEDIAS (FUNCIONALIDAD MIGRADA DE TIMEMANAGER) =====
    // Tocar horas en punto (wildcards = todas las horas)
    addExternal0(DOW_TODOS, ALARMA_WILDCARD, 0, 0, accionTocaHora, true);
    
    // Tocar medias horas
    addExternal0(DOW_TODOS, ALARMA_WILDCARD, 30, 0, accionTocaMedia, true);

    // ===== FUNCIONES DE MANTENIMIENTO DEL SISTEMA =====
    // Sincronización NTP al mediodía
    addExternal0(DOW_TODOS, 12, 2, 0, SincronizaNTP, true);
    
    // Actualización DNS dinámica
    addExternal0(DOW_TODOS, 12, 5, 0, ActualizaDNSSiNecesario, true);

    DBG_ALM_PRINTF("[ALARM] Configuración por defecto cargada: %u alarmas\n", _num);
}

// ============================================================================
// IMPLEMENTACIÓN DE GESTIÓN WEB DE ALARMAS PERSONALIZABLES
// ============================================================================
/**
 * @brief Añade una nueva alarma personalizable editable vía web
 * 
 * @details Crea una alarma completamente configurable desde la interfaz web
 *          con persistencia en JSON y callback proporcionado externamente.
 *          La alarma se marca como personalizable y recibe un ID único para 
 *          gestión web independiente del índice del array.
 * 
 * @param nombre Nombre descriptivo de la alarma (máx 49 caracteres)
 * @param descripcion Descripción opcional de la alarma (máx 99 caracteres)
 * @param mascaraDias Máscara de bits para días de la semana (DOW_DOMINGO, DOW_LUNES, etc.)
 * @param hora Hora de ejecución (0-23)
 * @param minuto Minuto de ejecución (0-59)
 * @param tipoString Tipo de acción como string libre: "MISA", "DIFUNTOS", "FIESTA", etc.
 * @param parametro Parámetro uint16_t a pasar al callback
 * @param callback Puntero a función externa que se ejecutará
 * @param habilitada Estado inicial de la alarma (true por defecto)
 * 
 * @return uint8_t Índice de la alarma en el array (0-MAX_ALARMAS) o MAX_ALARMAS si error
 * 
 * @note La alarma se guarda automáticamente en JSON tras la creación
 * @note El callback debe ser proporcionado externamente (normalmente desde Servidor.cpp)
 * @note Esta función es genérica - no tiene conocimiento de tipos específicos
 * 
 * @warning Máximo MAX_ALARMAS alarmas simultáneas (incluye sistema + personalizables)
 * @warning Los strings se truncan si exceden el tamaño máximo
 * @warning El callback debe ser válido y accesible durante toda la vida de la alarma
 * 
 * @see modificarPersonalizable(), eliminarPersonalizable()
 * 
 * @since v2.1 - Sistema de alarmas personalizables vía web
 */
uint8_t AlarmScheduler::addPersonalizable(const char* nombre, const char* descripcion,
                                         uint8_t mascaraDias, uint8_t hora, uint8_t minuto,
                                         const char* tipoString, uint16_t parametro,
                                         void (*callback)(uint16_t), bool habilitada) {
    DBG_ALM("🔔 Añadiendo alarma personalizable");
    DBG_ALM_PRINTF("  Nombre: %s", nombre);
    DBG_ALM_PRINTF("  Tipo: %s", tipoString);
    DBG_ALM_PRINTF("  Horario: %s %02d:%02d", 
                   mascaraDias == DOW_TODOS ? "Diario" : "Específico", hora, minuto);
    
    if (_num >= MAX_ALARMAS) {
        DBG_ALM("❌ Error: Máximo de alarmas alcanzado");
        return MAX_ALARMAS;
    }
    
    // Crear nueva alarma
    Alarm& alarma = _alarmas[_num];
    alarma.habilitada = habilitada;
    alarma.mascaraDias = mascaraDias;
    alarma.hora = hora;
    alarma.minuto = minuto;
    alarma.intervaloMin = 0;  // Las personalizables no usan intervalo
    alarma.parametro = parametro;
    
    // Asignar callback (ya viene como parámetro)
    alarma.accionExt = callback;
    alarma.accion = nullptr;     // Limpiar otros callbacks
    alarma.accionExt0 = nullptr;
    
    // Campos web
    strncpy(alarma.nombre, nombre, sizeof(alarma.nombre) - 1);
    alarma.nombre[sizeof(alarma.nombre) - 1] = '\0';
    
    strncpy(alarma.descripcion, descripcion, sizeof(alarma.descripcion) - 1);
    alarma.descripcion[sizeof(alarma.descripcion) - 1] = '\0';
    
    strncpy(alarma.tipoString, tipoString, sizeof(alarma.tipoString) - 1);
    alarma.tipoString[sizeof(alarma.tipoString) - 1] = '\0';
    
    alarma.esPersonalizable = true;
    alarma.idWeb = _generarNuevoIdWeb();
    
    uint8_t idx = _num;
    _num++;
    
    DBG_ALM_PRINTF("✅ Alarma personalizable creada - Índice: %d, ID Web: %d", idx, alarma.idWeb);
    
    // Guardar en JSON
    guardarPersonalizablesEnJSON();
    
    return idx;
}
/**
 * @brief Modifica una alarma personalizable existente por su ID web
 * 
 * @details Actualiza todos los campos de una alarma personalizable identificada
 *          por su ID web único. Mantiene el callback existente sin modificarlo.
 *          Actualiza automáticamente el archivo JSON de persistencia.
 *          Resetea el cache temporal para forzar reevaluación en próxima verificación.
 * 
 * @param idWeb ID único web de la alarma a modificar
 * @param nombre Nuevo nombre descriptivo (máx 49 caracteres)
 * @param descripcion Nueva descripción opcional (máx 99 caracteres)
 * @param mascaraDias Nueva máscara de días de la semana
 * @param hora Nueva hora de ejecución (0-23)
 * @param minuto Nuevo minuto de ejecución (0-59)
 * @param tipoString Nuevo tipo como string libre: "MISA", "DIFUNTOS", "FIESTA", etc.
 * @param habilitada Nuevo estado de habilitación
 * 
 * @return bool true si la modificación fue exitosa, false en caso de error
 * 
 * @note Solo se pueden modificar alarmas con esPersonalizable = true
 * @note MANTIENE el callback existente - no lo reasigna
 * @note Los cambios se guardan inmediatamente en JSON
 * @note Cache temporal se resetea para evitar conflictos
 * 
 * @warning El idWeb debe existir y pertenecer a una alarma personalizable
 * @warning Para cambiar callback, eliminar y recrear la alarma
 * 
 * @see addPersonalizable(), eliminarPersonalizable(), habilitarPersonalizable()
 * 
 * @since v2.1 - Sistema de alarmas personalizables vía web
 */
bool AlarmScheduler::modificarPersonalizable(int idWeb, const char* nombre, const char* descripcion,
                                           uint8_t mascaraDias, uint8_t hora, uint8_t minuto,
                                           const char* tipoString, bool habilitada) {
    DBG_ALM_PRINTF("✏️ Modificando alarma personalizable ID Web: %d", idWeb);
    
    uint8_t idx = _buscarIndicePorIdWeb(idWeb);
    if (idx >= MAX_ALARMAS) {
        DBG_ALM("❌ Error: Alarma no encontrada");
        return false;
    }
    
    Alarm& alarma = _alarmas[idx];
    
    // Verificar que es personalizable
    if (!alarma.esPersonalizable) {
        DBG_ALM("❌ Error: Alarma no es personalizable");
        return false;
    }
    
    // Actualizar campos (MANTENER callback existente)
    alarma.habilitada = habilitada;
    alarma.mascaraDias = mascaraDias;
    alarma.hora = hora;
    alarma.minuto = minuto;
    
    strncpy(alarma.nombre, nombre, sizeof(alarma.nombre) - 1);
    alarma.nombre[sizeof(alarma.nombre) - 1] = '\0';
    
    strncpy(alarma.descripcion, descripcion, sizeof(alarma.descripcion) - 1);
    alarma.descripcion[sizeof(alarma.descripcion) - 1] = '\0';
    
    strncpy(alarma.tipoString, tipoString, sizeof(alarma.tipoString) - 1);
    alarma.tipoString[sizeof(alarma.tipoString) - 1] = '\0';
    
    // Reset cache para forzar reevaluación
    alarma.ultimoDiaAno = -1;
    alarma.ultimoMinuto = 255;
    alarma.ultimaHora = 255;
    alarma.ultimaEjecucion = 0;
    
    DBG_ALM("✅ Alarma personalizable modificada");
    
    // Guardar en JSON
    guardarPersonalizablesEnJSON();
    
    return true;
}
/**
 * @brief Elimina permanentemente una alarma personalizable por su ID web
 * 
 * @details Elimina completamente una alarma personalizable del sistema,
 *          reorganizando el array para evitar huecos y actualizando el
 *          contador de alarmas. La operación es irreversible y actualiza
 *          automáticamente la persistencia JSON.
 * 
 * @param idWeb ID único web de la alarma a eliminar
 * 
 * @return bool true si la eliminación fue exitosa, false si no se encontró
 * 
 * @note Solo se pueden eliminar alarmas con esPersonalizable = true
 * @note Reorganiza automáticamente el array moviendo alarmas posteriores
 * @note Los cambios se guardan inmediatamente en JSON
 * @note Operación irreversible - no hay papelera de reciclaje
 * 
 * @warning Las alarmas de sistema (esPersonalizable = false) no se pueden eliminar
 * @warning Los índices de array pueden cambiar tras eliminación
 * 
 * @see addPersonalizable(), modificarPersonalizable()
 * 
 * @since v2.1 - Sistema de alarmas personalizables vía web
 */
bool AlarmScheduler::eliminarPersonalizable(int idWeb) {
    DBG_ALM_PRINTF("🗑️ Eliminando alarma personalizable ID Web: %d", idWeb);
    
    uint8_t idx = _buscarIndicePorIdWeb(idWeb);
    if (idx >= MAX_ALARMAS) {
        DBG_ALM("❌ Error: Alarma no encontrada");
        return false;
    }
    
    if (!_alarmas[idx].esPersonalizable) {
        DBG_ALM("❌ Error: Alarma no es personalizable");
        return false;
    }
    
    // Mover todas las alarmas posteriores una posición hacia atrás
    for (uint8_t i = idx; i < _num - 1; i++) {
        _alarmas[i] = _alarmas[i + 1];
    }
    
    // Limpiar la última posición
    _alarmas[_num - 1] = Alarm();
    _num--;
    
    DBG_ALM("✅ Alarma personalizable eliminada");
    
    // Guardar en JSON
    guardarPersonalizablesEnJSON();
    
    return true;
}
/**
 * @brief Habilita o deshabilita una alarma personalizable sin eliminarla
 * 
 * @details Cambia el estado de habilitación de una alarma personalizable,
 *          permitiendo pausar/reanudar su ejecución sin perder la configuración.
 *          Al habilitar, resetea el cache temporal para asegurar evaluación inmediata.
 * 
 * @param idWeb ID único web de la alarma a modificar
 * @param estado true para habilitar, false para deshabilitar
 * 
 * @return bool true si el cambio fue exitoso, false si no se encontró la alarma
 * 
 * @note Solo aplica a alarmas con esPersonalizable = true
 * @note Al habilitar se resetea cache para evaluación inmediata
 * @note Los cambios se guardan automáticamente en JSON
 * @note Alarma deshabilitada no se ejecuta pero conserva configuración
 * 
 * @warning El idWeb debe existir y pertenecer a una alarma personalizable
 * 
 * @see modificarPersonalizable(), eliminarPersonalizable()
 * 
 * @since v2.1 - Sistema de alarmas personalizables vía web
 */
bool AlarmScheduler::habilitarPersonalizable(int idWeb, bool estado) {
    DBG_ALM_PRINTF("🔄 %s alarma personalizable ID Web: %d", 
                   estado ? "Habilitando" : "Deshabilitando", idWeb);
    
    uint8_t idx = _buscarIndicePorIdWeb(idWeb);
    if (idx >= MAX_ALARMAS) {
        DBG_ALM("❌ Error: Alarma no encontrada");
        return false;
    }
    
    if (!_alarmas[idx].esPersonalizable) {
        DBG_ALM("❌ Error: Alarma no es personalizable");
        return false;
    }
    
    _alarmas[idx].habilitada = estado;
    
    // Reset cache si se habilita
    if (estado) {
        _alarmas[idx].ultimoDiaAno = -1;
        _alarmas[idx].ultimoMinuto = 255;
        _alarmas[idx].ultimaHora = 255;
        _alarmas[idx].ultimaEjecucion = 0;
    }
    
    DBG_ALM_PRINTF("✅ Alarma personalizable %s", estado ? "habilitada" : "deshabilitada");
    
    // Guardar en JSON
    guardarPersonalizablesEnJSON();
    
    return true;
}



// ============================================================================
// MÉTODOS AUXILIARES PRIVADOS
// ============================================================================

/**
 * @brief Busca el índice de array de una alarma por su ID web único
 * 
 * @details Itera sobre todas las alarmas personalizables buscando coincidencia
 *          de ID web. Utilizado internamente para operaciones de modificación,
 *          eliminación y cambio de estado de alarmas vía interfaz web.
 * 
 * @param idWeb ID único web de la alarma a buscar
 * 
 * @return uint8_t Índice de la alarma en el array o MAX_ALARMAS si no encontrada
 * 
 * @note Solo busca en alarmas con esPersonalizable = true
 * @note Retorna MAX_ALARMAS como valor de error (fuera de rango válido)
 * 
 * @see _generarNuevoIdWeb()
 * 
 * @since v2.1 - Sistema de alarmas personalizables vía web
 */
uint8_t AlarmScheduler::_buscarIndicePorIdWeb(int idWeb) {
    for (uint8_t i = 0; i < _num; i++) {
        if (_alarmas[i].esPersonalizable && _alarmas[i].idWeb == idWeb) {
            return i;
        }
    }
    return MAX_ALARMAS; // No encontrado
}
/**
 * @brief Genera un nuevo ID web único para alarma personalizable
 * 
 * @details Busca el ID web más alto existente entre alarmas personalizables
 *          y retorna el siguiente número disponible. Garantiza unicidad de
 *          identificadores para la interfaz web.
 * 
 * @return int Nuevo ID web único (entero positivo)
 * 
 * @note IDs web son independientes de índices de array
 * @note Busca solo en alarmas personalizables existentes
 * @note ID mínimo es 1 (nunca retorna 0)
 * 
 * @see _buscarIndicePorIdWeb()
 * 
 * @since v2.1 - Sistema de alarmas personalizables vía web
 */
int AlarmScheduler::_generarNuevoIdWeb() {
    // Buscar el ID más alto existente
    int maxId = 0;
    for (uint8_t i = 0; i < _num; i++) {
        if (_alarmas[i].esPersonalizable && _alarmas[i].idWeb > maxId) {
            maxId = _alarmas[i].idWeb;
        }
    }
    return maxId + 1;
}

// ============================================================================
// IMPLEMENTACIÓN DE FUNCIONES JSON Y PERSISTENCIA
// ============================================================================
/**
 * @brief Genera JSON con todas las alarmas personalizables para interfaz web
 * 
 * @details Construye una estructura JSON completa conteniendo únicamente las
 *          alarmas marcadas como personalizables (esPersonalizable = true),
 *          incluyendo metadatos, estadísticas y formato optimizado para consumo web.
 *          
 *          **ESTRUCTURA JSON GENERADA:**
 *          - version: Versión del formato JSON (2.1)
 *          - timestamp: Timestamp de generación en millis()
 *          - total: Número de alarmas personalizables
 *          - alarmas[]: Array con datos de cada alarma personalizable
 *            - id: ID web único para identificación
 *            - nombre: Nombre descriptivo de la alarma
 *            - descripcion: Descripción opcional
 *            - dia: Día de la semana (0=todos, 1-7=dom-sab)
 *            - diaNombre: Nombre legible del día
 *            - hora/minuto: Horario de ejecución
 *            - accion: Tipo de acción (valor de tipoString del struct)
 *            - habilitada: Estado de activación
 *            - horaTexto: Hora formateada (ej: "11:05")
 *            - indiceArray: Índice en array (para debug)
 * 
 * @return String JSON estructurado con alarmas personalizables y metadatos
 * 
 * @note **FILTRADO:** Solo incluye alarmas con esPersonalizable = true
 * @note **CONVERSIÓN:** Convierte máscaras de días a números (0-7)
 * @note **FORMATO WEB:** Incluye campos calculados para mostrar en UI
 * @note **GENÉRICO:** No interpreta tipos - solo los copia como strings
 * 
 * @warning **TAMAÑO:** JSON puede ser grande con muchas alarmas - considerar paginación
 * @warning **MEMORIA:** Usa JsonDocument que consume RAM durante generación
 * 
 * @see obtenerEstadisticasJSON(), guardarPersonalizablesEnJSON()
 * @see _diaToString() - Conversión de números a nombres de días
 * 
 * @since v2.1 - Sistema de alarmas personalizables vía web
 */
String AlarmScheduler::obtenerPersonalizablesJSON() {
    DBG_ALM("📋 Generando JSON con alarmas personalizables...");
    
    JsonDocument doc;
    doc["version"] = "2.1";
    doc["timestamp"] = millis();
    
    // Contar alarmas personalizables
    uint8_t personalizables = 0;
    for (uint8_t i = 0; i < _num; i++) {
        if (_alarmas[i].esPersonalizable) {
            personalizables++;
        }
    }
    
    doc["total"] = personalizables;
    
    JsonArray alarmasArray = doc.createNestedArray("alarmas");
    
    // Añadir cada alarma personalizable al JSON
    for (uint8_t i = 0; i < _num; i++) {
        const Alarm& alarma = _alarmas[i];
        
        if (!alarma.esPersonalizable) continue; // Solo personalizables
        
        JsonObject alarmaObj = alarmasArray.createNestedObject();
        
        alarmaObj["id"] = alarma.idWeb;
        alarmaObj["nombre"] = alarma.nombre;
        alarmaObj["descripcion"] = alarma.descripcion;
        
        // Convertir máscara de días a número de día (0-7)
        int dia = 0;
        if (alarma.mascaraDias == DOW_TODOS) {
            dia = 0; // Todos los días
        } else {
            // Buscar primer bit activo
            for (int d = 0; d < 7; d++) {
                if (alarma.mascaraDias & (1 << d)) {
                    dia = d + 1; // 1=Domingo, 2=Lunes, etc.
                    break;
                }
            }
        }
        
        alarmaObj["dia"] = dia;
        alarmaObj["diaNombre"] = _diaToString(dia);
        alarmaObj["hora"] = alarma.hora;
        alarmaObj["minuto"] = alarma.minuto;
        alarmaObj["accion"] = alarma.tipoString;
        alarmaObj["habilitada"] = alarma.habilitada;
        
        // Formatear hora para mostrar (ej: "11:05")
        char horaFormateada[8];
        sprintf(horaFormateada, "%02d:%02d", alarma.hora, alarma.minuto);
        alarmaObj["horaTexto"] = horaFormateada;
        
        // Información de estado
        alarmaObj["indiceArray"] = i; // Para debug
    }
    
    String resultado;
    serializeJson(doc, resultado);
    
    DBG_ALM_PRINTF("📋 JSON generado con %d alarmas (%d chars)", personalizables, resultado.length());
    return resultado;
}
/**
 * @brief Genera JSON con estadísticas completas del sistema de alarmas
 * 
 * @details Construye estructura JSON con información estadística detallada
 *          del sistema de alarmas, incluyendo conteos por categoría, estado de
 *          callbacks, información de memoria y configuración actual del módulo.
 *          Ideal para monitoreo, debug y dashboards administrativos.
 *          
 *          **ESTADÍSTICAS INCLUIDAS:**
 *          - Información general: módulo, versión, timestamp
 *          - Contadores: total, sistema, personalizables, habilitadas, deshabilitadas
 *          - Capacidad: espacioLibre, maxAlarmas, siguienteIdWeb
 *          - Estados: (sin callbacks específicos - módulo genérico)
 *          - Persistencia: archivoJSON, archivoExiste
 *          - Tiempo actual: hora, minuto, diaSemana, diaAno (si RTC válido)
 * 
 * @return String JSON con estadísticas completas del sistema
 * 
 * @note **CONTADORES:** Diferencia alarmas de sistema vs personalizables
 * @note **CALLBACKS:** Indica si están configurados (no null)
 * @note **TIEMPO:** Incluye estado actual del RTC si está sincronizado
 * @note **ARCHIVOS:** Verifica existencia del archivo de persistencia
 * 
 * @warning **RTC DEPENDENCY:** Campos de tiempo solo válidos si getLocalTime() funciona
 * @warning **SPIFFS:** Verificación de archivo requiere SPIFFS montado
 * 
 * @see obtenerPersonalizablesJSON(), cargarPersonalizablesDesdeJSON()
 * @see getLocalTime() - Función para obtener tiempo actual
 * 
 * @example
 * @code
 * String stats = Alarmas.obtenerEstadisticasJSON(); 
 * Serial.println("Stats: " + stats);
 * ws.textAll("STATS_ALARMAS_WEB:" + stats);
 * @endcode
 * 
 * @since v2.1 - Sistema de alarmas personalizables vía web
 * @author Julian Salas Bartolomé
 */
String AlarmScheduler::obtenerEstadisticasJSON() {
    DBG_ALM("📊 Generando estadísticas del sistema...");
    
    JsonDocument doc;
    
    // Información general
    doc["modulo"] = "AlarmScheduler";
    doc["version"] = "2.1";
    doc["timestamp"] = millis();
    
    // Contadores de alarmas
    uint8_t sistema = 0, personalizables = 0, habilitadas = 0, deshabilitadas = 0;
    
    for (uint8_t i = 0; i < _num; i++) {
        if (_alarmas[i].esPersonalizable) {
            personalizables++;
        } else {
            sistema++;
        }
        
        if (_alarmas[i].habilitada) {
            habilitadas++;
        } else {
            deshabilitadas++;
        }
    }
    
    doc["totalAlarmas"] = _num;
    doc["sistema"] = sistema;
    doc["personalizables"] = personalizables;
    doc["habilitadas"] = habilitadas;
    doc["deshabilitadas"] = deshabilitadas;
    doc["espacioLibre"] = MAX_ALARMAS - _num;
    doc["maxAlarmas"] = MAX_ALARMAS;
        
    // Información del sistema
    doc["siguienteIdWeb"] = _siguienteIdWeb;
    doc["archivoJSON"] = "/alarmas_personalizadas.json";
    doc["archivoExiste"] = SPIFFS.exists("/alarmas_personalizadas.json");
    
    // Estado actual del tiempo
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        doc["tiempoActual"]["valido"] = true;
        doc["tiempoActual"]["hora"] = timeinfo.tm_hour;
        doc["tiempoActual"]["minuto"] = timeinfo.tm_min;
        doc["tiempoActual"]["diaSemana"] = timeinfo.tm_wday;
        doc["tiempoActual"]["diaAno"] = timeinfo.tm_yday;
    } else {
        doc["tiempoActual"]["valido"] = false;
    }
    
    String resultado;
    serializeJson(doc, resultado);
    
    DBG_ALM_PRINTF("📊 Estadísticas generadas (%d chars)", resultado.length());
    return resultado;
}
/**
 * @brief Carga alarmas personalizables desde archivo JSON en SPIFFS
 * 
 * @details Lee el archivo JSON de persistencia y recrea todas las alarmas
 *          personalizables en el sistema, validando integridad de datos y
 *          asignando callbacks según configuración. Si el archivo no existe,
 *          crea alarmas por defecto automáticamente.
 *          
 *          **PROCESO DE CARGA:**
 *          1. Verifica existencia del archivo JSON en SPIFFS
 *          2. Si no existe: crea alarmas por defecto y guarda JSON
 *          3. Si existe: lee contenido y parsea JSON
 *          4. Elimina alarmas personalizables existentes (mantiene sistema)
 *          5. Para cada alarma en JSON:
 *             - Valida datos básicos (nombre, hora, minuto, idWeb)
 *             - Convierte día (0-7) a máscara de bits
 *             - Crea alarma en array con todos los campos
 *             - Actualiza siguienteIdWeb si es necesario
 *          6. Actualiza contador total de alarmas
 * 
 * @return bool true si la carga fue exitosa, false si error o archivo corrupto
 * 
 * @note **PRESERVACIÓN:** Mantiene alarmas de sistema intactas
 * @note **VALIDACIÓN:** Descarta alarmas con datos inválidos
 * @note **CALLBACKS:** Requiere callbacks configurados previamente con setCallback*()
 * @note **AUTO-CREACIÓN:** Crea alarmas por defecto si archivo no existe
 * @note **ID MANAGEMENT:** Actualiza siguienteIdWeb para evitar duplicados
 * 
 * @warning **ARCHIVO CORRUPTO:** JSON malformado puede causar pérdida de alarmas
 * @warning **CALLBACKS NULL:** Alarmas sin callback válido no se ejecutarán
 * @warning **SPIFFS:** Requiere sistema de archivos montado
 * 
 * @see guardarPersonalizablesEnJSON(), addPersonalizable()
 * @see _crearAlarmasPersonalizablesPorDefecto() - Alarmas por defecto
 * 
 * @example
 * @code
 * void setup() {
 *     // Configurar callbacks antes de cargar
 *     Alarmas.setCallbackMisa(&accionSecuencia);
 *     Alarmas.setCallbackDifuntos(&accionSecuencia);  
 *     Alarmas.setCallbackFiesta(&accionSecuencia);
 *     
 *     // Cargar alarmas persistidas
 *     if (Alarmas.cargarPersonalizablesDesdeJSON()) {
 *         Serial.println("Alarmas cargadas correctamente");
 *     }
 * }
 * @endcode
 * 
 * @since v2.1 - Sistema de alarmas personalizables vía web
 * @author Julian Salas Bartolomé
 */
bool AlarmScheduler::cargarPersonalizablesDesdeJSON() {
    DBG_ALM("📂 Cargando alarmas personalizables desde JSON...");
    
    const char* archivo = "/alarmas_personalizadas.json";
    
    if (!SPIFFS.exists(archivo)) {
        DBG_ALM("📄 Archivo de alarmas no existe, creando alarmas por defecto");
        _crearAlarmasPersonalizablesPorDefecto();
        return guardarPersonalizablesEnJSON();
    }
    
    File file = SPIFFS.open(archivo, "r");
    if (!file) {
        DBG_ALM("❌ Error al abrir archivo de alarmas");
        return false;
    }
    
    String contenido = file.readString();
    file.close();
    
    DBG_ALM_PRINTF("📄 Contenido leído (%d bytes)", contenido.length());
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, contenido);
    
    if (error) {
        DBG_ALM_PRINTF("❌ Error parseando JSON: %s", error.c_str());
        return false;
    }
    
    // Eliminar alarmas personalizables existentes (manteniendo las de sistema)
    for (int i = _num - 1; i >= 0; i--) {
        if (_alarmas[i].esPersonalizable) {
            // Mover alarmas posteriores hacia atrás
            for (uint8_t j = i; j < _num - 1; j++) {
                _alarmas[j] = _alarmas[j + 1];
            }
            _num--;
        }
    }
    
    // Cargar alarmas del JSON
    JsonArray alarmasArray = doc["alarmas"];
    int cargadas = 0;
    
    for (JsonObject alarmaObj : alarmasArray) {
        if (_num >= MAX_ALARMAS) {
            DBG_ALM("⚠️ Máximo de alarmas alcanzado, ignorando restantes");
            break;
        }
        
        // Leer datos del JSON
        const char* nombre = alarmaObj["nombre"] | "";
        const char* descripcion = alarmaObj["descripcion"] | "";
        int dia = alarmaObj["dia"] | 0;
        uint8_t hora = alarmaObj["hora"] | 0;
        uint8_t minuto = alarmaObj["minuto"] | 0;
        const char* tipoString = alarmaObj["accion"] | "SISTEMA";
        bool habilitada = alarmaObj["habilitada"] | true;
        int idWeb = alarmaObj["id"] | -1;
        
        // Validar datos básicos
        if (strlen(nombre) == 0 || hora > 23 || minuto > 59 || idWeb <= 0) {
            DBG_ALM_PRINTF("⚠️ Alarma inválida ignorada: %s", nombre);
            continue;
        }
        
        // Convertir día a máscara
        uint8_t mascaraDias;
        if (dia == 0) {
            mascaraDias = DOW_TODOS;
        } else {
            mascaraDias = 1 << (dia - 1); // dia 1-7 -> bit 0-6
        }
        
        // Crear alarma
        Alarm& alarma = _alarmas[_num];
        alarma.habilitada = habilitada;
        alarma.mascaraDias = mascaraDias;
        alarma.hora = hora;
        alarma.minuto = minuto;
        alarma.intervaloMin = 0;
        alarma.parametro = 0;
        
        // Campos web
        strncpy(alarma.nombre, nombre, sizeof(alarma.nombre) - 1);
        alarma.nombre[sizeof(alarma.nombre) - 1] = '\0';
        
        strncpy(alarma.descripcion, descripcion, sizeof(alarma.descripcion) - 1);
        alarma.descripcion[sizeof(alarma.descripcion) - 1] = '\0';
        
        strncpy(alarma.tipoString, tipoString, sizeof(alarma.tipoString) - 1);
        alarma.tipoString[sizeof(alarma.tipoString) - 1] = '\0';
            
        alarma.esPersonalizable = true;
        alarma.idWeb = idWeb;
        
         
        // Actualizar siguiente ID si es necesario
        if (idWeb >= _siguienteIdWeb) {
            _siguienteIdWeb = idWeb + 1;
        }
        
        _num++;
        cargadas++;
        
        DBG_ALM_PRINTF("✅ Alarma cargada: %s (%s %02d:%02d)", 
                      nombre, _diaToString(dia).c_str(), hora, minuto);
    }
    
    DBG_ALM_PRINTF("✅ Alarmas personalizables cargadas: %d", cargadas);
    return true;
}
/**
 * @brief Guarda alarmas personalizables en archivo JSON en SPIFFS
 * 
 * @details Serializa únicamente las alarmas personalizables a formato JSON
 *          estructurado y las persiste en SPIFFS para supervivencia a reinicios.
 *          Incluye validación de escritura y manejo de errores de filesystem.
 *          
 *          **PROCESO DE GUARDADO:**
 *          1. Crea documento JSON con metadatos (versión, timestamp)
 *          2. Cuenta alarmas personalizables en el sistema
 *          3. Crea array JSON con datos de cada alarma personalizable:
 *             - Convierte máscaras de días a números (0-7)
 *             - Incluye todos los campos web necesarios
 *             - Excluye campos internos (cache, callbacks)
 *          4. Abre archivo en modo escritura (sobreescribe)
 *          5. Serializa JSON directamente al archivo
 *          6. Valida bytes escritos antes de confirmar éxito
 * 
 * @return bool true si el guardado fue exitoso, false si error de escritura
 * 
 * @note **FILTRADO:** Solo guarda alarmas con esPersonalizable = true
 * @note **FORMATO:** JSON compatible con cargarPersonalizablesDesdeJSON()
 * @note **SOBREESCRITURA:** Reemplaza archivo existente completamente
 * @note **VALIDACIÓN:** Verifica operación de escritura antes de confirmar
 * @note **METADATOS:** Incluye versión y timestamp para control de versiones
 * 
 * @warning **ESPACIO DISCO:** Requiere SPIFFS montado y espacio suficiente
 * @warning **OPERACIÓN SÍNCRONA:** Puede bloquear temporalmente (ms) durante escritura
 * @warning **PÉRDIDA DATOS:** Error de escritura puede corromper archivo existente
 * 
 * @see cargarPersonalizablesDesdeJSON(), obtenerPersonalizablesJSON()
 * @see SPIFFS.open(), serializeJson() - Funciones utilizadas internamente
 * 
 * @example
 * @code
 * // Guardar después de modificar alarma
 * if (Alarmas.modificarPersonalizable(id, "Nueva Misa", "", DOW_DOMINGO, 12, 0, "MISA", true)) {
 *     // El guardado se hace automáticamente, pero se puede verificar:
 *     Serial.println("Alarma modificada y guardada");
 * }
 * 
 * // Guardado manual explícito
 * if (Alarmas.guardarPersonalizablesEnJSON()) {
 *     Serial.println("Alarmas guardadas en SPIFFS");
 * } else {
 *     Serial.println("Error guardando alarmas");
 * }
 * @endcode
 * 
 * @since v2.1 - Sistema de alarmas personalizables vía web
 * @author Julian Salas Bartolomé
 */
bool AlarmScheduler::guardarPersonalizablesEnJSON() {
    DBG_ALM("💾 Guardando alarmas personalizables en JSON...");
    
    const char* archivo = "/alarmas_personalizadas.json";
    
    JsonDocument doc;
    doc["version"] = "2.1";
    doc["timestamp"] = millis();
    
    // Contar personalizables
    uint8_t personalizables = 0;
    for (uint8_t i = 0; i < _num; i++) {
        if (_alarmas[i].esPersonalizable) {
            personalizables++;
        }
    }
    
    doc["total"] = personalizables;
    
    JsonArray alarmasArray = doc.createNestedArray("alarmas");
    
    // Guardar solo alarmas personalizables
    for (uint8_t i = 0; i < _num; i++) {
        const Alarm& alarma = _alarmas[i];
        
        if (!alarma.esPersonalizable) continue;
        
        JsonObject alarmaObj = alarmasArray.createNestedObject();
        
        alarmaObj["id"] = alarma.idWeb;
        alarmaObj["nombre"] = alarma.nombre;
        alarmaObj["descripcion"] = alarma.descripcion;
        
        // Convertir máscara a día
        int dia = 0;
        if (alarma.mascaraDias == DOW_TODOS) {
            dia = 0;
        } else {
            for (int d = 0; d < 7; d++) {
                if (alarma.mascaraDias & (1 << d)) {
                    dia = d + 1;
                    break;
                }
            }
        }
        
        alarmaObj["dia"] = dia;
        alarmaObj["hora"] = alarma.hora;
        alarmaObj["minuto"] = alarma.minuto;
        alarmaObj["accion"] = alarma.tipoString;
        alarmaObj["habilitada"] = alarma.habilitada;
    }
    
    // Escribir archivo
    File file = SPIFFS.open(archivo, "w");
    if (!file) {
        DBG_ALM("❌ Error al crear archivo JSON");
        return false;
    }
    
    // ✅ PARTE FALTANTE - ESCRIBIR Y CERRAR ARCHIVO
    size_t bytesEscritos = serializeJson(doc, file);
    file.close();
    
    if (bytesEscritos == 0) {
        DBG_ALM("❌ Error escribiendo JSON - 0 bytes escritos");
        return false;
    }
    
    DBG_ALM_PRINTF("✅ JSON guardado exitosamente: %d alarmas, %d bytes", personalizables, bytesEscritos);
    
    // ✅ VERIFICACIÓN OPCIONAL - Comprobar que el archivo se escribió
    if (SPIFFS.exists(archivo)) {
        File verificacion = SPIFFS.open(archivo, "r");
        if (verificacion) {
            size_t tamanoArchivo = verificacion.size();
            verificacion.close();
            DBG_ALM_PRINTF("✅ Archivo verificado: %d bytes en disco", tamanoArchivo);
        }
    } else {
        DBG_ALM("⚠️ Advertencia: Archivo no encontrado tras guardado");
    }
    
    return true;
}

// ============================================================================
// MÉTODOS AUXILIARES ADICIONALES
// ============================================================================
/**
 * @brief Convierte número de día (0-7) a texto descriptivo en español
 * 
 * @details Función utilitaria privada que convierte la representación numérica
 *          de días utilizada en JSON y interfaz web a texto legible en español.
 *          Utilizada para generar campos "diaNombre" en JSON y debug.
 * 
 * @param dia Número de día (0=Todos los días, 1=Domingo, 2=Lunes, ..., 7=Sábado)
 * @return String con nombre del día en español o "Día inválido" si fuera de rango
 * 
 * @note **FORMATO:** 0=Todos los días, 1-7=Domingo a Sábado
 * @note **IDIOMA:** Nombres en español para interfaz local  
 * @note **VALIDACIÓN:** Retorna "Día inválido" para valores fuera de rango
 * @note **USO INTERNO:** Función privada para uso interno de la clase
 * 
 * @see obtenerPersonalizablesJSON() - Principal usuario de esta función
 * @see cargarPersonalizablesDesdeJSON() - También la utiliza para debug
 * 
 * @since v2.1 - Sistema de alarmas personalizables vía web
 * @author Julian Salas Bartolomé
 */
String AlarmScheduler::_diaToString(int dia) {
    switch (dia) {
        case 0: return "Todos los días";
        case 1: return "Domingo";
        case 2: return "Lunes";
        case 3: return "Martes";
        case 4: return "Miércoles";
        case 5: return "Jueves";
        case 6: return "Viernes";
        case 7: return "Sábado";
        default: return "Día inválido";
    }
}
/**
 * @brief Crea alarmas personalizables predeterminadas del sistema
 * 
 * @details Función privada que configura un conjunto básico de alarmas
 *          personalizables cuando no existe archivo de persistencia. Crea las
 *          misas dominicales tradicionales como punto de partida para el usuario.
 *          
 *          **ALARMAS CREADAS POR DEFECTO:**
 *          - "Misa Domingo 11:05": Primera llamada misa dominical  
 *          - "Misa Domingo 11:25": Segunda llamada misa dominical
 *          
 *          Ambas configuradas como tipo "MISA" y habilitadas por defecto.
 * 
 * @note **LLAMADA AUTOMÁTICA:** Solo se ejecuta si no existe archivo JSON
 * @note **PERSISTENCIA:** Las alarmas creadas se guardan automáticamente
 * @note **HORARIOS:** Basados en tradición católica española estándar
 * @note **MODIFICABLES:** Usuario puede editarlas posteriormente vía web
 * 
 * @warning **CALLBACKS:** Requiere que los callbacks estén configurados previamente
 * 
 * @see cargarPersonalizablesDesdeJSON() - Función que la llama
 * @see addPersonalizable() - Función utilizada para crear las alarmas
 * 
 * @since v2.1 - Sistema de alarmas personalizables vía web
 * @author Julian Salas Bartolomé
 */
void AlarmScheduler::_crearAlarmasPersonalizablesPorDefecto() {
    DBG_ALM("🔄 No creando alarmas por defecto - se crearán desde web");
    // Las alarmas personalizables se crean desde la interfaz web
    // que tiene acceso a los callbacks apropiados
}

/**
 * @brief Imprime información detallada de todas las alarmas registradas
 * 
 * @details Función de debug que muestra por Serial Monitor todos los campos
 *          de todas las alarmas configuradas en el sistema, tanto de sistema
 *          como personalizables, con formato legible y estructurado.
 * 
 * @note Útil para depuración y verificación del estado del sistema
 * @note Incluye campos internos y de cache temporal
 * 
 * @since v2.1 - Función de debug añadida
 * @author Julian Salas Bartolomé
 */
void AlarmScheduler::imprimirTodasLasAlarmas() {
    Serial.println("\n🔔 ========== LISTADO COMPLETO DE ALARMAS ==========");
    Serial.printf("📊 Total de alarmas registradas: %u/%u\n", _num, MAX_ALARMAS);
    Serial.printf("🆔 Siguiente ID Web: %d\n", _siguienteIdWeb);
    Serial.println();
    
    if (_num == 0) {
        Serial.println("❌ No hay alarmas registradas");
        return;
    }
    
    for (uint8_t i = 0; i < _num; i++) {
        const Alarm& alarma = _alarmas[i];
        
        Serial.printf("📋 ========== ALARMA ÍNDICE: %u ==========\n", i);
        
        // === IDENTIFICACIÓN ===
        Serial.printf("🆔 ID Web: %d\n", alarma.idWeb);
        Serial.printf("📛 Nombre: '%s'\n", alarma.nombre);
        Serial.printf("📝 Descripción: '%s'\n", alarma.descripcion);
        Serial.printf("🎯 Tipo String: '%s'\n", alarma.tipoString);
        Serial.printf("⚙️ Es Personalizable: %s\n", alarma.esPersonalizable ? "SÍ" : "NO");
        
        // === HORARIO ===
        Serial.printf("🕐 Hora: %u\n", alarma.hora);
        Serial.printf("🕐 Minuto: %u\n", alarma.minuto);
        Serial.printf("⏰ Intervalo (min): %u\n", alarma.intervaloMin);
        
        // === DÍAS DE LA SEMANA ===
        Serial.printf("📅 Máscara Días: 0x%02X (", alarma.mascaraDias);
        if (alarma.mascaraDias == DOW_TODOS) {
            Serial.print("TODOS LOS DÍAS");
        } else {
            bool primero = true;
            if (alarma.mascaraDias & DOW_DOMINGO) { 
                if (!primero) Serial.print(", "); 
                Serial.print("DOM"); 
                primero = false; 
            }
            if (alarma.mascaraDias & DOW_LUNES) { 
                if (!primero) Serial.print(", "); 
                Serial.print("LUN"); 
                primero = false; 
            }
            if (alarma.mascaraDias & DOW_MARTES) { 
                if (!primero) Serial.print(", "); 
                Serial.print("MAR"); 
                primero = false; 
            }
            if (alarma.mascaraDias & DOW_MIERCOLES) { 
                if (!primero) Serial.print(", "); 
                Serial.print("MIÉ"); 
                primero = false; 
            }
            if (alarma.mascaraDias & DOW_JUEVES) { 
                if (!primero) Serial.print(", "); 
                Serial.print("JUE"); 
                primero = false; 
            }
            if (alarma.mascaraDias & DOW_VIERNES) { 
                if (!primero) Serial.print(", "); 
                Serial.print("VIE"); 
                primero = false; 
            }
            if (alarma.mascaraDias & DOW_SABADO) { 
                if (!primero) Serial.print(", "); 
                Serial.print("SÁB"); 
                primero = false; 
            }
        }
        Serial.println(")");
        
        // === ESTADO ===
        Serial.printf("✅ Habilitada: %s\n", alarma.habilitada ? "SÍ" : "NO");
        Serial.printf("🔢 Parámetro: %u\n", alarma.parametro);
        
        // === CALLBACKS ===
        Serial.printf("🔗 Acción (método): %s\n", alarma.accion ? "CONFIGURADO" : "NULL");
        Serial.printf("🔗 Acción Externa (param): %s\n", alarma.accionExt ? "CONFIGURADO" : "NULL");
        Serial.printf("🔗 Acción Externa 0: %s\n", alarma.accionExt0 ? "CONFIGURADO" : "NULL");
        
        // === CACHE TEMPORAL ===
        Serial.printf("📅 Último Día Año: %d\n", alarma.ultimoDiaAno);
        Serial.printf("🕐 Último Minuto: %u\n", alarma.ultimoMinuto);
        Serial.printf("🕐 Última Hora: %u\n", alarma.ultimaHora);
        Serial.printf("⏰ Última Ejecución: %lu\n", (unsigned long)alarma.ultimaEjecucion);
        
        // === HORARIO FORMATEADO ===
        if (alarma.hora == ALARMA_WILDCARD && alarma.minuto == ALARMA_WILDCARD) {
            Serial.println("🕒 Horario: WILDCARD:WILDCARD (cada minuto)");
        } else if (alarma.hora == ALARMA_WILDCARD) {
            Serial.printf("🕒 Horario: WILDCARD:%02u (cada hora a los %u min)\n", alarma.minuto, alarma.minuto);
        } else if (alarma.minuto == ALARMA_WILDCARD) {
            Serial.printf("🕒 Horario: %02u:WILDCARD (cada minuto de las %u:XX)\n", alarma.hora, alarma.hora);
        } else {
            Serial.printf("🕒 Horario: %02u:%02u (fijo)\n", alarma.hora, alarma.minuto);
        }
        
        Serial.println();
    }
    
    Serial.println("🔔 ========== FIN LISTADO DE ALARMAS ==========\n");
}
