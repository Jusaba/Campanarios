// Auxiliar.cpp
#include "Auxiliar.h"
#include "ConexionWifi.h"
#include "DNSServicio.h"  // Para ActualizaDNS



// Definiciones únicas de variables globales
CAMPANARIO Campanario;                                                  // Campanario
struct tm timeinfo;                                                     // Estructura de tiempo global
int nCampanaTocada = 0;                                                 // Campana que se está tocando
bool lConexionInternet = false;                                         // Estado de conexión a Internet
bool lProteccionCampanadas = false;                                     // Estado de protección de campanadas
bool lProteccionCampanadasAnterior = false;                             // Estado anterior de protección de campanadas
unsigned long ultimoCheckInternet = 0;                                  // Última verificación de conexión a Internet
//const unsigned long intervaloCheckInternet = 5 * 60 * 1000;             // Intervalo de verificación de conexión a Internet
int nTemporizacionCalefaccion = 45;                                     // Temporización de calefacción en minutos
double nSegundosTemporizacion = 0;                                      // Temporización de calefacción en segundos



    /**
     * @brief Ejecuta una secuencia de prueba de todas las campanas del sistema
     * 
     * @details Esta función llama al método ActualizarSecuenciaCampanadas() del objeto global
     *          Campanario para verificar si hay una campana que debe sonar en la secuencia actual.
     *          Si una campana ha sonado, envía un mensaje a todos los clientes WebSocket conectados
     *          notificando qué campana ha sonado. Si la secuencia de campanadas ha finalizado,
     *          detiene la secuencia y envía una redirección a la página principal.
     *          Se llama desde loop() para testear el estado de las campanas
     * 
     *          **PROCESO DE PRUEBA:**
     *          1. Verifica que el campanario esté disponible
     *          2. Actualiza variable global nCampanaTocada
     *          3. Log detallado de cada prueba realizada
     *          
     * 
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void TestCampanadas(void) 
    {
        nCampanaTocada = Campanario.ActualizarSecuenciaCampanadas();
        if (nCampanaTocada > 0) {
            DBG_AUX_PRINTF("Campana tocada: %d\n", nCampanaTocada);
            Campanario.ResetCampanaTocada();
            ws.textAll("CAMPANA:"+String(nCampanaTocada));
            if (!Campanario.GetEstadoSecuencia()) {
                DBG_AUX_PRINTF("Secuencia de campanadas finalizada.\n");
                Campanario.ParaSecuencia();
                ws.textAll("REDIRECT:/index.html");
            }
        }
    }


    /**
     * @brief Ejecuta una secuencia específica.
     * 
     * Esta función ejecuta la secuencia indicada por el parámetro nSecuencia.
     * Llama internamente a la sobrecarga de EjecutaSecuencia con el segundo parámetro en cero.
     * 
     * @param nSecuencia Número identificador de la secuencia a ejecutar.
     */
    void EjecutaSecuencia (int nSecuencia)
    {
        EjecutaSecuencia(nSecuencia, 0);
    }

    /**
     * @brief Ejecuta una secuencia  según el ID especificado y el parametro pasado
     * 
     * @details Version sobrecargada Función principal que mapea IDs numéricos a acciones específicas
     *          de campananario. Actúa como dispatcher central para todas las
     *          acciones disponibles en el sistema.
     *          
     *          **MAPEO DE SECUENCIAS:**
     *          - ID Config::States::DIFUNTOS: Secuencia de Difuntos (tradicional, solemne)
     *          - ID Config::States::MISA: Secuencia de Misa (litúrgica compleja)
     *          - ID Config::States::FIESTA: Secuencia de Fiesta (alegre, rápida)
     *          - ID Config::States::STOP: Detener todas las secuencias activas
     *          - ID Config::States::CALEFACCION_ON: Encender calefacción (sin temporizador)
     *          - ID Config::States::CALEFACCION_OFF: Apagar calefacción
     *          - ID Config::States::PROTECCION_CAMPANADAS: Activar/Desactivar protección de campanadas
     *         - ID Config::States::SET_TEMPORIZADOR: Configurar temporizador de calefacción (requiere parámetro)
     *          
     *          **PROCESO DE EJECUCIÓN:**
     *          1. Verifica validez del ID de secuencia
     *          2. Mapea ID a función específica del campanario
     *          3. Ejecuta secuencia si el campanario está disponible
     *          4. Log de la operación para monitorización
     * 
     * @param nSecuencia Identificador numérico de la secuencia (1-255)
     * @param nParametro Parámetro adicional requerido por algunas secuencias (ej. temporizador)
     * 
     * @note **EXTENSIBLE:** Fácil añadir nuevas secuencias modificando el switch
     * @note **CENTRALIZADO:** Punto único de control para todas las secuencias
     * @note **VALIDACIÓN:** Verifica IDs válidos antes de ejecutar
     * 
     * @warning **ID VÁLIDO:** IDs no definidos se ignoran silenciosamente
     * @warning **CAMPANARIO:** Requiere objeto Campanario inicializado
     * 
     * @see Campanario.TocaDifuntos() - Secuencia ID 1
     * @see Campanario.TocaMisa() - Secuencia ID 2
     * 
     * @example
     * @code
     * // Ejecutar secuencia de Difuntos
     * EjecutaSecuencia(1);
     * 
     * // Ejecutar secuencia de Misa
     * EjecutaSecuencia(2);
     * 
     * // Uso desde alarma programada
     * alarmas.addAlarm(12, 0, ALARM_MATCH_HOUR_MIN, 
     *                 []() { EjecutaSecuencia(2); });  // Misa a las 12:00
     * @endcode
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */    
    void EjecutaSecuencia(int nSecuencia, int nParametro) {
        
        DBG_AUX_PRINTF("EjecutaSecuencia -> Secuencia: %d, Parámetro: %d", nSecuencia, nParametro);

        switch (nSecuencia) {
            case Config::States::DIFUNTOS:
                Campanario.TocaDifuntos();
                ws.textAll("REDIRECT:/Campanas.html");      
                DBG_AUX("EjecutaSecuencia -> Iniciando secuencia de difuntos");
                break;

            case Config::States::MISA:
                Campanario.TocaMisa();
                ws.textAll("REDIRECT:/Campanas.html");
                DBG_AUX("EjecutaSecuencia -> Iniciando secuencia de misa");
                break;

            case Config::States::FIESTA:
                Campanario.TocaFiesta();
                ws.textAll("REDIRECT:/Campanas.html");
                DBG_AUX("EjecutaSecuencia -> Iniciando secuencia de fiesta");
                break;

            case Config::States::STOP:
                Campanario.ParaSecuencia();
                ws.textAll("REDIRECT:/index.html");
                DBG_AUX("EjecutaSecuencia -> Parando todas las secuencias");
                break;

            case Config::States::CALEFACCION_ON:
                  // WebSocket con temporizador global
                    Campanario.EnciendeCalefaccion(nTemporizacionCalefaccion);
                    ws.textAll("CALEFACCION:ON");
                    DBG_AUX_PRINTF("EjecutaSecuencia -> Encendiendo calefacción (%d min desde WebSocket)", nTemporizacionCalefaccion);
                    break;

            case Config::States::CALEFACCION_OFF:
                Campanario.ApagaCalefaccion();
                ws.textAll("CALEFACCION:OFF");
                DBG_AUX("EjecutaSecuencia -> Apagando calefacción");
                break;

            case Config::States::PROTECCION_CAMPANADAS:
               if (lProteccionCampanadas) {
                    Campanario.SetProteccionCampanadas();
                    ws.textAll("PROTECCION:ON");
                    DBG_AUX("EjecutaSecuencia -> Activando protección de campanadas");
                } else {
                    Campanario.ClearProteccionCampanadas();
                    ws.textAll("PROTECCION:OFF");
                    DBG_AUX("EjecutaSecuencia -> Desactivando protección de campanadas");
                }
                break;

            case Config::States::SET_TEMPORIZADOR:
                nTemporizacionCalefaccion = nParametro;
                Campanario.EnciendeCalefaccion(nTemporizacionCalefaccion);
                ws.textAll("CALEFACCION:ON:" + String(nTemporizacionCalefaccion));
                DBG_AUX_PRINTF("EjecutaSecuencia -> Temporizador de calefacción fijado a %d minutos", nTemporizacionCalefaccion);
                break;

            default:
                DBG_AUX_PRINTF("EjecutaSecuencia -> Secuencia no reconocida: %d", nSecuencia);
                break;
        }
    }

    /**
     * @brief Verifica la conectividad a internet y actualiza estado del sistema
     * 
     * @details Función que realiza una verificación completa de la conectividad
     *          a internet y actualiza las variables globales correspondientes.
     *          Incluye optimización temporal para evitar verificaciones excesivas.
     *          
     *          **PROCESO DE VERIFICACIÓN:**
     *          1. Verifica si es momento de hacer check (intervalo configurado)
     *          2. Ejecuta hayInternet() para verificar conectividad real
     *          3. Actualiza variable global lConexionInternet
     *          4. Actualiza estado del campanario (BitSinInternet)
     *          5. Si hay internet: ejecuta ActualizaDNS automáticamente
     *          6. Log del resultado para monitorización
     *          
     *          **OPTIMIZACIÓN TEMPORAL:**
     *          - Solo verifica cada Config::Network::INTERNET_CHECK_INTERVAL_MS milisegundos
     *          - Evita saturar la red con checks excesivos
     *          - Mantiene estado actual entre verificaciones
     * 
     * @note **OPTIMIZADO:** No verifica en cada llamada - usa intervalo configurado
     * @note **AUTOMÁTICO:** Actualiza DNS dinámico si hay conectividad
     * @note **INTEGRADO:** Sincroniza estado con objeto Campanario
     * 
     * @warning **INTERVALO:** Configurar Config::Network::INTERNET_CHECK_INTERVAL_MS apropiadamente
     * @warning **DNS:** Requiere configuración válida de dominio dinámico
     * 
     * @see hayInternet() - Función subyacente de verificación de conectividad
     * @see ActualizaDNS() - Actualización automática de DNS dinámico
     * 
     * @example
     * @code
     * void loop() {
     *     // Verificación periódica en loop principal
     *     TestInternet();
     *     
     *     // Otras tareas...
     *     delay(100);
     * }
     * @endcode
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void TestInternet(void) {
        bool estadoAnteriorInternet = lConexionInternet; // Guarda el estado anterior de la conexión
        if (!hayInternet()) { // hayInternet() debe comprobar acceso real a internet
          lConexionInternet = ConectarWifi(configWiFi); // Intenta reconectar
          if (lConexionInternet) {
              DBG_AUX("TestInternet -> Reconectado a internet correctamente.");
              ServidorOn(configWiFi.usuario, configWiFi.clave); // Reinicia el servidor si es necesario
              if (!estadoAnteriorInternet) { // Si el estado cambió de desconectado a conectado
                  Campanario.SetInternetConectado(); // Notifica al campanario que hay internet
              }
          } else {
              DBG_AUX("TestInternet -> Sin conexión a internet. Funcionando en modo local.");
              if (estadoAnteriorInternet) { // Si el estado cambió de conectado a desconectado
                  Campanario.ClearInternetConectado(); // Notifica al campanario que no hay internet
              }
          }
        }else{
            lConexionInternet = true; // Asegura que la variable esté actualizada
            if (!estadoAnteriorInternet) { // Si el estado cambió de desconectado a conectado
                Campanario.SetInternetConectado(); // Notifica al campanario que hay internet
            }
            DBG_AUX("TestInternet -> Conexión a internet activa.");
        }
    }



/**
 * @brief Verifica si el minuto actual está dentro de un período protegido para campanadas.
 * 
 * Esta función SOLO verifica el estado, NO modifica variables globales.
 * Devuelve true si estamos en período de protección, false en caso contrario.
 * 
 * @return true si estamos en período protegido, false en caso contrario.
 */
    /**
     * @brief Determina si estamos en período permitido para toque de campanas
     * 
     * @details Función que evalúa si el momento actual está dentro del período
     *          permitido para tocar campanas según la configuración de horarios.
     *          Utiliza la configuración centralizada para determinar horarios
     *          nocturnos y períodos de silencio.
     *          
     *          **EVALUACIÓN REALIZADA:**
     *          1. Obtiene hora actual del RTC
     *          2. Compara con horarios de inicio y fin de período nocturno
     *          3. Maneja correctamente transición medianoche (ej: 22:00-07:00)
     *          4. Retorna true si es período permitido
     *          
     *          **CONFIGURACIÓN UTILIZADA:**
     *          - Config::Time::NOCHE_INICIO_HORA: Inicio del período de silencio
     *          - Config::Time::NOCHE_FIN_HORA: Fin del período de silencio
     *          - Manejo automático de horarios que cruzan medianoche
     * 
     * @return true si es período permitido para campanadas, false en horario nocturno
     * 
     * @note **CONFIGURABLE:** Utiliza configuración centralizada de horarios
     * @note **MEDIANOCHE:** Maneja correctamente períodos que cruzan 00:00
     * @note **RTC DEPENDIENTE:** Requiere RTC sincronizado para funcionar
     * 
     * @warning **RTC:** Si no se puede obtener hora, asume período permitido (seguro)
     * @warning **CONFIGURACIÓN:** Verificar que horarios estén bien configurados
     * 
     * @see Config::Time::NOCHE_INICIO_HORA - Configuración de inicio de silencio
     * @see Config::Time::NOCHE_FIN_HORA - Configuración de fin de silencio
     * 
     * @example
     * @code
     * // Verificación antes de tocar campanadas
     * if (EsPeriodoToqueCampanas()) {
     *     Campanario.TocaHoraSinCuartos(12);
     * } else {
     *     Serial.println("Horario nocturno - campanadas silenciadas");
     * }
     * @endcode
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    bool EsPeriodoToqueCampanas(void) {
        if (!RTC::isNtpSync()) {
            DBG_AUX("EsPeriodoToqueCampanas -> RTC no sincronizado con NTP.");
            return false; // Sin sync NTP = sin protección
        }
        
        struct tm localTime;
        if (!getLocalTime(&localTime)) {
            DBG_AUX("EsPeriodoToqueCampanas -> Error obteniendo hora del RTC");
            return false; // Sin hora = sin protección
        }
        
        int minuto = localTime.tm_min;
        bool periodoHoraEnPunto = (minuto >= Config::Time::INICIO_VENTANA_HORA) || 
                                  (minuto <= Config::Time::FIN_VENTANA_HORA);        // 57,58,59,0,1,2,3
        
        bool periodoMediaHora = (minuto >= Config::Time::INICIO_VENTANA_MEDIA) && 
                                (minuto <= Config::Time::FIN_VENTANA_MEDIA);         // 27,28,29,30,31,32,33
        
        bool enPeriodoProteccion = (periodoHoraEnPunto || periodoMediaHora);
        
        
        if (enPeriodoProteccion) {
            DBG_AUX_PRINTF("EsPeriodoToqueCampanas -> Período activo (minuto %d): %s", 
                           minuto, 
                           periodoHoraEnPunto ? "Hora en punto" : "Media hora");
        }
        
        
        return enPeriodoProteccion;
    }
/**
 * @brief Actualiza el estado de protección de campanadas y gestiona cambios.
 *
 * Esta función:
 * 1. Obtiene el nuevo estado llamando a EsPeriodoToqueCampanas()
 * 2. Compara con el estado anterior
 * 3. Si hay cambios, actualiza Campanario y notifica por WebSocket
 * 4. Actualiza las variables globales de estado
 *
 * @note Esta es la función que debe llamarse desde el loop principal.
 */
/**
 * @brief Actualiza el estado de protección de campanadas según horarios
 * 
 * @details Función que evalúa automáticamente si debe activarse la protección
 *          de campanadas basándose en el horario actual y actualiza tanto
 *          las variables globales como el estado del campanario.
 *          
 *          **PROCESO DE ACTUALIZACIÓN:**
 *          1. Obtiene el nuevo estado llamando a EsPeriodoToqueCampanas()
 *          2. Compara con el estado anterior
 *          3. Si hay cambios, actualiza Campanario y notifica por WebSocket
 *          4. Actualiza las variables globales de estado
 *
 *          **INTEGRACIÓN SISTEMA:**
 *          - Sincroniza variables globales con estado de Campanario
 *          - Detecta y reporta cambios de estado automáticamente
 *          - Mantiene coherencia entre subsistemas
 * 
 * @note **AUTOMÁTICO:** Debe llamarse periódicamente desde loop () para mantener sincronización
 * @note **INTEGRADO:** Sincroniza múltiples variables de estado
 * @note **INFORMATIVO:** Log solo cuando cambia el estado (reduce ruido)
 * 
 * @warning **LLAMADA PERIÓDICA:** Debe ejecutarse regularmente desde loop principal
 * @warning **COHERENCIA:** Mantiene múltiples variables sincronizadas
 * 
 * @see EsPeriodoToqueCampanas() - Función utilizada para evaluar período
 * @see Campanario.SetProteccionCampanadas() - Actualización de estado
 * 
 * @since v1.0
 * @author Julian Salas Bartolomé
 */
void ActualizaEstadoProteccionCampanadas(void) {
    
    bool nuevoEstadoProteccion = EsPeriodoToqueCampanas();                              // Obtener nuevo estado (SIN efectos secundarios)
    if (lProteccionCampanadas != nuevoEstadoProteccion) {                               // Solo procesar si hay cambio
        lProteccionCampanadasAnterior = lProteccionCampanadas;
        lProteccionCampanadas = nuevoEstadoProteccion;
        
        EjecutaSecuencia(Config::States::PROTECCION_CAMPANADAS);                        // Notificar cambio por WebSocket (UNA SOLA VEZ)

        DBG_AUX_PRINTF("ActualizaEstadoProteccionCampanadas -> Cambio: %s -> %s",       // Log del cambio
                       lProteccionCampanadasAnterior ? "ACTIVA" : "INACTIVA",
                       lProteccionCampanadas ? "ACTIVA" : "INACTIVA");
    }
    

}
