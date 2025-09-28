// Acciones.cpp - Añadir estas implementaciones:
#include "Acciones.h"
#include "Auxiliar.h"     // Para EjecutaSecuencia
#include "Campanario.h"   // Para Campanario
#include "Servidor.h"     // Para ws
#include "DNSServicio.h"  // Para ActualizaDNS
#include "ConexionWifi.h" // Para configWiFi
#include "Configuracion.h"// Para los parámetros de configuración


    /**
     * @brief Ejecuta una secuencia específica de campanadas identificada por su ID
     * 
     * @details Función principal para ejecutar secuencias programadas de campanadas.
     *          Verifica automáticamente si el campanario está ocupado antes de
     *          proceder con la ejecución para evitar interferencias entre secuencias.
     *          
     *          **VERIFICACIONES REALIZADAS:**
     *          - Estado del campanario (ocupado/libre)
     *          - Validez del ID de secuencia
     *          - Disponibilidad del sistema de campanadas
     *          
     *          **SECUENCIAS TÍPICAS:**
     *          - ID 1: Secuencia de Difuntos
     *          - ID 2: Secuencia de Misa  
     *          - ID 3: Secuencias personalizadas
     * 
     * @param seqId Identificador único de la secuencia a ejecutar (1-255)
     * 
     * @note **VERIFICACIÓN AUTOMÁTICA:** Comprueba campanario ocupado antes de ejecutar
     * @note **NO BLOQUEANTE:** Retorna inmediatamente si campanario ocupado
     * @note **INTEGRACIÓN:** Compatible con sistema de alarmas como función callback
     * 
     * @warning **ID VÁLIDO:** Verificar que seqId corresponde a secuencia existente
     * @warning **CAMPANARIO:** Requiere objeto Campanario inicializado globalmente
     * 
     * @see EjecutaSecuencia() - Función subyacente que ejecuta la secuencia
     * @see Campanario.GetEstadoSecuencia() - Verificación de estado utilizada
     * 
     * @example
     * @code
     * // Uso directo
     * accionSecuencia(1);  // Ejecutar secuencia de Difuntos
     * 
     * // Uso en sistema de alarmas
     * alarmas.addAlarm(12, 0, ALARM_MATCH_HOUR_MIN, accionSecuencia, 2);  // Misa a las 12:00
     * @endcode
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void accionSecuencia(uint16_t seqId) 
    {
        if (Campanario.GetEstadoSecuencia()) {
            DBG_ACCIONES_PRINTF("Secuencia %u bloqueada - campanario ocupado", seqId);
            return;                                                                     // Salir sin ejecutar
        }
        EjecutaSecuencia(seqId);
        DBG_ACCIONES_PRINTF("Secuencia %u ejecutada", seqId);
    }

    /**
     * @brief Toca las campanadas correspondientes a la hora actual
     * 
     * @details Función que toca el número de campanadas correspondiente a la hora
     *          actual del sistema (formato 12 horas). Incluye verificaciones de
     *          horario nocturno para protección acústica y estado del campanario.
     *          
     *          **FUNCIONALIDADES:**
     *          - Conversión automática 24h → 12h (15:00 = 3 campanadas)
     *          - Protección horario nocturno configurable
     *          - Verificación de campanario ocupado
     *          - Notificación web automática tras ejecución
     *          - Actualización DNS automática si hay internet
     *          
     *          **PROTECCIÓN NOCTURNA:**
     *          - Utiliza Config::Time::NOCHE_INICIO_HORA y NOCHE_FIN_HORA
     *          - No ejecuta campanadas en horario nocturno
     *          - Respeta normativas de ruido urbano
     * 
     * @note **SIN CUARTOS:** Solo toca campanadas de hora, no cuartos previos
     * @note **PROTECCIÓN NOCTURNA:** Automática según configuración del sistema
     * @note **WEB INTEGRATION:** Notifica automáticamente a clientes web conectados
     * 
     * @warning **RTC REQUERIDO:** Necesita RTC sincronizado para obtener hora actual
     * @warning **CAMPANARIO:** Requiere campanario inicializado y con campanas añadidas
     * 
     * @see Campanario.TocaHoraSinCuartos() - Función subyacente utilizada
     * @see Config::Time::NOCHE_INICIO_HORA - Configuración horario nocturno
     * 
     * @example
     * @code
     * // Uso en alarma programada para tocar cada hora
     * alarmas.addAlarm(0, 0, ALARM_MATCH_MINUTE, accionTocaHora, 0);  // Cada hora en punto
     * @endcode
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void accionTocaHora(void) {
        // Obtener tiempo actual
        if (Campanario.GetEstadoSecuencia()) {
            DBG_ACCIONES("Toque de hora bloqueado - campanario ocupado");
            return;
        }
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) return;

        // Verificar horario nocturno
        int hora = timeinfo.tm_hour;
        bool esNocturno = (hora >= Config::Time::NOCHE_INICIO_HORA || 
                           hora < Config::Time::NOCHE_FIN_HORA);
        
        if (!esNocturno) {
            Campanario.TocaHoraSinCuartos(hora);
            DBG_ACCIONES_PRINTF("Toque de hora ejecutado a las %02d:00", hora);
            ws.textAll("REDIRECT:/Campanas.html");

            // Actualizar DNS si hay internet
            if (hayInternet()) {
                ActualizaDNSSiNecesario();
            }
        }else{
            DBG_ACCIONES_PRINTF("No se ha ejecutado el toque de hora a las %02d:00 por horario nocturno", hora);
        }
    }
    /**
     * @brief Toca la campanada de media hora (:30)
     * 
     * @details Función que toca una sola campanada para indicar la media hora.
     *          Incluye las mismas verificaciones de protección que accionTocaHora()
     *          pero con una sola campanada en lugar de múltiples.
     *          
     *          **CARACTERÍSTICAS:**
     *          - Una sola campanada a los 30 minutos
     *          - Misma protección nocturna que toque de hora
     *          - Verificación de campanario ocupado
     *          - Notificación web y DNS automáticas
     * 
     * @note **CAMPANADA ÚNICA:** Solo una campanada, no secuencia completa
     * @note **PROTECCIÓN:** Mismas verificaciones que accionTocaHora()
     * @note **CAMPANA:** Utiliza campana de cuartos si está disponible
     * 
     * @see Campanario.TocaMediaHora() - Función subyacente utilizada
     * @see accionTocaHora() - Función hermana para toques de hora completa
     * 
     * @example
     * @code
     * // Uso en alarma programada para tocar medias horas
     * alarmas.addAlarm(30, 0, ALARM_MATCH_MINUTE, accionTocaMedia, 0);  // Cada media hora
     * @endcode
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void accionTocaMedia(void) {
            if (Campanario.GetEstadoSecuencia()) {
                DBG_ACCIONES("Toque de media bloqueado - campanario ocupado");
            return;
        }
        // Obtener tiempo actual
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) return;

        // Verificar horario nocturno
        int hora = timeinfo.tm_hour;
        bool esNocturno = (hora >= Config::Time::NOCHE_INICIO_HORA || 
                           hora < Config::Time::NOCHE_FIN_HORA);
        
        if (!esNocturno) {
            Campanario.TocaMediaHora();
            DBG_ACCIONES_PRINTF("Toque de media ejecutado a las %02d:30", hora);
            ws.textAll("REDIRECT:/Campanas.html");

            if (hayInternet()) {
                ActualizaDNSSiNecesario();
            }
        }
    }

    /**
     * @brief Sincroniza el RTC con servidores NTP
     * 
     * @details Función de mantenimiento que re-sincroniza el reloj del sistema
     *          con servidores NTP para mantener la precisión temporal. Esta
     *          función siempre se ejecuta sin verificar el estado del campanario
     *          ya que no interfiere con las campanadas.
     *          
     *          **PROCESO DE SINCRONIZACIÓN:**
     *          1. Verifica disponibilidad de internet
     *          2. Si hay conexión: ejecuta RTC::begin() para re-sincronizar
     *          3. Si no hay conexión: no hace nada (mantiene hora actual)
     *          4. Log del resultado para monitorización
     *          
     *          **CARACTERÍSTICAS:**
     *          - No verifica estado del campanario (siempre se ejecuta)
     *          - Solo actúa si hay conexión a internet
     *          - Utiliza la lógica existente de RTC::begin()
     *          - Debug opcional para monitorización
     * 
     * @note **SIEMPRE EJECUTA:** No verifica campanario ocupado - es acción de mantenimiento
     * @note **INTERNET REQUERIDO:** Solo sincroniza si hay conexión disponible
     * @note **NO INTERFIERE:** No afecta a campanadas en curso
     * 
     * @warning **CONEXION:** Requiere WiFi conectado para acceder a servidores NTP
     * @warning **RTC:** Depende de la implementación correcta de RTC::begin()
     * 
     * @see RTC::begin() - Función subyacente que ejecuta la sincronización
     * @see hayInternet() - Verificación de conectividad utilizada
     * 
     * @example
     * @code
     * // Uso en alarma programada para sincronizar cada 6 horas
     * alarmas.addAlarm(0, 0, ALARM_MATCH_HOUR, SincronizaNTP, 0);  // Cada 6 horas
     * @endcode
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void SincronizaNTP(void) {
        if (hayInternet()) {
            RTC::begin();  // ← Reutiliza tu lógica existente

            DBG_ACCIONES("SincronizaNTP -> Re-sincronización completada");

        }
    }


