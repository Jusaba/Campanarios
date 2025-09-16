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
    if (cargarPorDefecto) initDefaults();
    DBG_ALM_PRINTF("[ALARM] Sistema inicializado con %u alarmas\n", _num);
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
            if ((oAlarma.hora == ALARMA_WILDCARD || oAlarma.hora == horaActual) &&
                (oAlarma.minuto == ALARMA_WILDCARD || oAlarma.minuto == minutoActual) &&
                !(oAlarma.ultimoDiaAno == diaAnoActual && oAlarma.ultimoMinuto == minutoActual))
            {
                disparar = true;
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
    addExternal(DOW_DOMINGO, 11, 5,  0, accionSecuencia, Config::States::MISA, true);
    addExternal(DOW_DOMINGO, 11, 25, 0, accionSecuencia, Config::States::MISA, true);

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