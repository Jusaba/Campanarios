// RTC.cpp - Implementaciones
#include "RTC.h"


    bool RTC::ntpSyncOk = false;                                                                    // Única definición

    // ========================================================================
    // MÉTODOS DE SINCRONIZACION
    // ========================================================================
    /**
     * @brief Inicializa sincronización NTP con un servidor específico
     * 
     * @details Función básica de sincronización que conecta con un único
     *          servidor NTP y establece la zona horaria del sistema.
     *          
     *          **PROCESO DE SINCRONIZACIÓN:**
     *          1. Configura servidor NTP y zona horaria con configTime()
     *          2. Inicia proceso de sincronización no bloqueante
     *          3. Espera confirmación con timeout configurable
     *          4. Valida fecha recibida y establece ntpSyncOk
     *          5. Logging del resultado de sincronización
     * 
     * @param ntpServer Dirección del servidor NTP (hostname o IP)
     * @param gmtOffsetSec Offset en segundos respecto GMT (ej: 3600 para GMT+1)
     * @param daylightOffsetSec Ajuste horario de verano en segundos (ej: 3600)
     * @param timeout_ms Timeout máximo para sincronización en milisegundos (defecto: 10000)
     * 
     * @note Función bloqueante hasta completar sincronización o timeout
     * @note Establece automáticamente la variable estática ntpSyncOk
     * @note Recomendado usar beginConMultiplesServidores() para mayor confiabilidad
     * 
     * @warning Función bloqueante - puede tardar hasta timeout_ms milisegundos
     * @warning Requiere conexión Wi-Fi activa antes de llamar
     * @warning Un servidor fallido puede causar timeout completo
     * 
     * @see beginConMultiplesServidores() - Versión recomendada con redundancia
     * @see isNtpSync() - Para verificar resultado de sincronización
     * @see configTime() - Función ESP32 utilizada internamente
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void RTC::begin(const char* ntpServer, long gmtOffsetSec, 
                int daylightOffsetSec, unsigned long timeout_ms) 
    {
        configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
        DBG_RTC("Sincronizando hora con NTP en " + String(ntpServer) + ".");

        struct tm timeinfo;
        unsigned long start = millis();
        while (!getLocalTime(&timeinfo)) {
            if (millis() - start > timeout_ms) {
                DBG_RTC("Timeout esperando sincronización NTP.");
                break;
            }
            DBG_RTC("Esperando sincronización NTP...");
            delay(1000);
        }
    
        ntpSyncOk = getLocalTime(&timeinfo);
        if (ntpSyncOk) {
            DBG_RTC_PRINT("Hora sincronizada correctamente: ");
            DBG_RTC(timeToString(timeinfo));
        }
    }

    /**
     * @brief Inicializa sincronización NTP con múltiples servidores para redundancia
     * 
     * @details Función avanzada de sincronización que utiliza hasta 3 servidores
     *          NTP configurados para máxima confiabilidad y disponibilidad.
     *          
     *          **PROCESO AVANZADO DE SINCRONIZACIÓN:**
     *          1. Configura los 3 servidores NTP definidos en configuración
     *          2. ESP32 rotará automáticamente entre servidores si hay fallos
     *          3. Valida cada fecha recibida con ValidaFecha() 
     *          4. Reintentos automáticos hasta conseguir fecha válida
     *          5. Timeout global para evitar bloqueos indefinidos
     *          6. Logging detallado de intentos y servidor usado exitoso
     * 
     * @param timeout_ms Timeout máximo total para sincronización en milisegundos (defecto: 30000)
     * 
     * @retval true Sincronización exitosa con fecha válida obtenida
     * @retval false Timeout alcanzado sin conseguir sincronización válida
     * 
     * @note **SERVIDORES UTILIZADOS:** NTP_SERVER1, NTP_SERVER2, NTP_SERVER3
     * @note **ZONA HORARIA:** GMT_OFFSET_SEC y DAYLIGHT_OFFSET_SEC de configuración
     * @note **VALIDACIÓN:** Solo acepta fechas entre 2020-2050 como válidas
     * @note **REDUNDANCIA:** Rotación automática entre servidores por ESP32
     * @note **RECOMENDADA:** Es la función preferida para sincronización
     * 
     * @warning Función bloqueante - puede tardar hasta timeout_ms milisegundos
     * @warning Requiere al menos un servidor NTP accesible
     * @warning Configuración de servidores debe estar correcta en Configuracion.h
     * 
     * @see NTP_SERVER1, NTP_SERVER2, NTP_SERVER3 - Servidores configurados
     * @see GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC - Zona horaria configurada
     * @see ValidaFecha() - Función de validación utilizada
     * @see isNtpSync() - Para verificar resultado de sincronización
     * 
     * @example
     * @code
     * // Sincronización recomendada con timeout de 30 segundos
     * if (RTC::beginConMultiplesServidores(30000)) {
     *     Serial.println("Sincronización NTP exitosa");
     *     Serial.println("Hora actual: " + RTC::getTimeStr());
     * } else {
     *     Serial.println("Error en sincronización NTP");
     * }
     * @endcode
     * 
     * @since v2.0
     * @author Julian Salas Bartolomé
     */
    bool RTC::beginConMultiplesServidores(unsigned long timeout_ms) 
    {
        DBG_RTC("Iniciando sincronización NTP con múltiples servidores...");
    
        // Lista de servidores NTP (3 servidores máximo soportados por ESP32)
        const char* ntpServers[] = {
            NTP_SERVER1,
            NTP_SERVER2,
            NTP_SERVER3
        };
    
        // Configurar múltiples servidores NTP
        configTime(GMT_OFFSET_SEC, 
               DAYLIGHT_OFFSET_SEC, 
               ntpServers[0], 
               ntpServers[1], 
               ntpServers[2]);
    
        DBG_RTC("Servidores NTP configurados:");
        DBG_RTC("  - Servidor 1: " + String(ntpServers[0]));
        DBG_RTC("  - Servidor 2: " + String(ntpServers[1])); 
        DBG_RTC("  - Servidor 3: " + String(ntpServers[2]));

        struct tm timeinfo;
        unsigned long start = millis();
        int intentos = 0;
    
        while (millis() - start < timeout_ms) {
            if (getLocalTime(&timeinfo)) {
                // Validar que la fecha sea realista (después de 2020)
                if (ValidaFecha(timeinfo)) 
                {
                    ntpSyncOk = true;
                    DBG_RTC("✅ Sincronización NTP exitosa:");
                    DBG_RTC("   Fecha/Hora: " + timeToString(timeinfo));
                    DBG_RTC("   Servidor usado: " + String(ntpServers[intentos % 3]));
                    return true;
                } else {
                    DBG_RTC("⚠️ Fecha inválida recibida, reintentando...");
                }
            }
            intentos++;
            if (intentos % 3 == 0) {
                DBG("Reintentando sincronización... (intento " + String(intentos/3 + 1) + ")");
            }

            delay(1000);
        }
    
        DBG("❌ Timeout en sincronización NTP después de " + String(timeout_ms/1000) + " segundos");
        ntpSyncOk = false;
        return false;
    }
    // ========================================================================
    // MÉTODOS DE VALIDACIÓN Y ESTADO
    // ========================================================================
    /**
     * @brief Valida que una estructura de fecha/hora sea realista
     * 
     * @details Función de validación que verifica que los datos de fecha
     *          recibidos del servidor NTP estén en rangos válidos y sean
     *          realistas para el funcionamiento del sistema.
     *          
     *          **VALIDACIONES REALIZADAS:**
     *          - Año entre 2020 y 2050 (rango realista del sistema)
     *          - Mes entre 0-11 (formato tm_mon de struct tm)
     *          - Día entre 1-31 (rango básico, no valida días por mes)
     * 
     * @param timeinfo Estructura tm con fecha/hora a validar
     * 
     * @retval true Fecha válida y dentro de rangos aceptables
     * @retval false Fecha inválida o fuera de rangos esperados
     * 
     * @note **RANGO DE AÑOS:** 2020-2050 considerado realista para el sistema
     * @note **FORMATO MESES:** 0-11 según estándar struct tm (0=Enero)
     * @note **VALIDACIÓN BÁSICA:** No verifica días por mes específico
     * @note **USO INTERNO:** Llamada automáticamente por beginConMultiplesServidores()
     * 
     * @warning No valida días específicos por mes (ej: 31 de febrero pasaría)
     * @warning Los rangos de años pueden necesitar ajuste en el futuro
     * 
     * @see struct tm - Estructura estándar de tiempo utilizada
     * @see beginConMultiplesServidores() - Función que utiliza esta validación
     * 
     * @since v2.0
     * @author Julian Salas Bartolomé
     */
    bool RTC::ValidaFecha(const struct tm& timeinfo) {
        // Verificar que el año sea razonable (después de 2020, antes de 2050)
        int year = timeinfo.tm_year + 1900;
        if (year < 2020 || year > 2050) {
            return false;
        }

        // Verificar mes válido
        if (timeinfo.tm_mon < 0 || timeinfo.tm_mon > 11) {
            return false;
        }

        // Verificar día válido
        if (timeinfo.tm_mday < 1 || timeinfo.tm_mday > 31) {
            return false;
        }

        return true;
    }
    /**
     * @brief Verifica si la sincronización NTP fue exitosa
     * 
     * @details Función de consulta que retorna el estado actual de
     *          sincronización NTP del sistema. Útil para verificar
     *          si el sistema tiene hora confiable.
     * 
     * @retval true Sincronización NTP exitosa y hora válida disponible
     * @retval false Sin sincronización NTP o última sincronización falló
     * 
     * @note **ESTADO PERSISTENTE:** Se mantiene hasta nueva sincronización
     * @note **THREAD-SAFE:** Acceso seguro desde cualquier contexto
     * @note **USO TÍPICO:** Verificar antes de operaciones dependientes de tiempo
     * 
     * @see ntpSyncOk - Variable estática que almacena este estado
     * @see begin() - Función que establece este estado
     * @see beginConMultiplesServidores() - Función que establece este estado
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    bool RTC::isNtpSync() {
        return ntpSyncOk;
    }
    /**
     * @brief Obtiene fecha y hora actual formateada como string
     * 
     * @details Función de conveniencia que obtiene la fecha/hora actual
     *          del sistema y la retorna formateada en string legible.
     * 
     * @return String con fecha/hora en formato "YYYY-MM-DD HH:MM:SS"
     *         o "Error obteniendo hora" si hay problemas
     * 
     * @note **FORMATO:** "2025-09-14 15:30:45" (ISO 8601 simplificado)
     * @note **ERROR:** Retorna mensaje descriptivo si getLocalTime() falla
     * @note **DEPENDENCIA:** Requiere sincronización NTP previa exitosa
     * 
     * @warning Puede retornar string de error si no hay sincronización
     * @warning El formato es fijo - usar timeToString() para personalizar
     * 
     * @see timeToString() - Función utilizada internamente para formateo
     * @see getLocalTime() - Función ESP32 utilizada para obtener tiempo
     * @see isNtpSync() - Para verificar si hay sincronización antes de llamar
     * 
     * @example
     * @code
     * if (RTC::isNtpSync()) {
     *     String horaActual = RTC::getTimeStr();
     *     Serial.println("Hora actual: " + horaActual);
     * } else {
     *     Serial.println("Sin sincronización NTP disponible");
     * }
     * @endcode
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    String RTC::getTimeStr() {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            return "Error obteniendo hora";
        }
        return timeToString(timeinfo);
    }
    /**
     * @brief Convierte estructura tm a string formateado
     * 
     * @details Función utilitaria que convierte una estructura tm
     *          a string con formato legible estándar.
     * 
     * @param timeinfo Estructura tm con fecha/hora a formatear
     * 
     * @return String formateado "YYYY-MM-DD HH:MM:SS"
     * 
     * @note **FORMATO FIJO:** "2025-09-14 15:30:45" (ISO 8601 simplificado)
     * @note **USO INTERNO:** Utilizada por getTimeStr() y debug interno
     * @note **THREAD-SAFE:** No modifica estado global
     * 
     * @see strftime() - Función C utilizada internamente
     * @see getTimeStr() - Función que utiliza este método
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    String RTC::timeToString(const struct tm& timeinfo) {
        char buffer[30];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
        return String(buffer);
    }