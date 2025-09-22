// I2CServicio.cpp - Implementación del servicio I2C
#include "Arduino.h"
#include "I2CServicio.h"
#include "Configuracion.h"
#include "Auxiliar.h"        // Para acceder a Campanario y variables globales
#include "RTC.h"             // Para getLocalTime() y RTC::isNtpSync()
#include "Debug.h"

// Variables I2C (definiciones únicas)
volatile uint8_t secuenciaI2C = 0;
uint8_t requestI2C = 0;
uint8_t ParametroI2C = 0;

    /**
     * @brief Inicializa la comunicación I2C como dispositivo esclavo
     * 
     * @details Configura el ESP32 como esclavo I2C en la dirección especificada
     *          y registra los callbacks necesarios para comunicación bidireccional
     *          con el dispositivo maestro.
     *          
     *          **CONFIGURACIÓN REALIZADA:**
     *          1. Inicializa Wire con dirección esclavo desde Configuracion.h
     *          2. Registra callback recibirSecuencia() para datos entrantes
     *          3. Registra callback enviarRequest() para peticiones de datos
     *          4. Logging de inicialización exitosa con dirección configurada
     * 
     * @note **DIRECCIÓN:** Utiliza Config::I2C::SLAVE_ADDR de Configuracion.h
     * @note **CALLBACKS:** Ambos callbacks son obligatorios para comunicación completa
     * @note **LLAMADA ÚNICA:** Debe llamarse una sola vez durante inicialización
     * @note **DEBUG:** Logging automático si DBG_I2C está habilitado
     * 
     * @warning **ORDEN:** Llamar antes de cualquier operación I2C
     * @warning **CONFLICTOS:** No mezclar con otros usos de Wire en el sistema
     * 
     * @see Config::I2C::SLAVE_ADDR - Dirección I2C utilizada
     * @see recibirSecuencia() - Callback registrado para recepción
     * @see enviarRequest() - Callback registrado para envío
     * @see Wire.begin() - Función ESP32 utilizada internamente
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void initI2C() {
        Wire.begin(Config::I2C::SLAVE_ADDR);  // Dirección desde Configuracion.h
        Wire.onReceive(recibirSecuencia);
        Wire.onRequest(enviarRequest);
        DBG_I2C("I2CServicio -> Inicializado como esclavo en dirección 0x" + String(Config::I2C::SLAVE_ADDR, HEX));
    }

/**
 * @brief Callback para recibir comandos y parámetros del maestro I2C
 * 
 * @details Función callback ejecutada automáticamente cuando el maestro I2C
 *          envía datos al esclavo. Procesa comandos de 1 o 2 bytes y diferencia
 *          entre comandos directos y solicitudes de información.
 *          
 *          **PROCESAMIENTO DE DATOS:**
 *          1. Lee primer byte como comando/secuencia principal
 *          2. Si numBytes == 2, lee segundo byte como parámetro
 *          3. Analiza si es solicitud de información (vs comando directo)
 *          4. Para solicitudes: guarda en requestI2C y resetea secuenciaI2C
 *          5. Para comandos: mantiene en secuenciaI2C para procesamiento en loop
 * 
 * @param numBytes Número de bytes enviados por el maestro (1 o 2 típicamente)
 * 
 * @note **CONTEXTO INTERRUPCIÓN:** Ejecuta en contexto de interrupción I2C
 * @note **PROCESAMIENTO MÍNIMO:** Evita operaciones complejas para no bloquear bus
 * @note **DIFERENCIACIÓN:** Solicitudes se procesan inmediatamente, comandos se quedan pendientes
 * @note **DEBUG CONDICIONAL:** Logging diferente para requests vs secuencias
 * 
 * @warning **INTERRUPCIÓN:** No realizar operaciones largas en esta función
 * @warning **CONCURRENCIA:** Variables modificadas son volatile
 * @warning **WIRE.READ():** Solo leer bytes mientras Wire.available() sea true
 * 
 * @see Wire.onReceive() - Función que registra este callback
 * @see secuenciaI2C - Variable modificada para comandos directos
 * @see requestI2C - Variable modificada para solicitudes de información
 * @see ParametroI2C - Variable modificada cuando numBytes == 2
 * @see Config::States - Constantes utilizadas para identificar tipos
 * 
 * @since v1.0 - Recepción básica de comandos
 * @since v2.0 - Diferenciación entre comandos y solicitudes, parámetros
 * 
 * @author Julian Salas Bartolomé
 */
void recibirSecuencia(int numBytes) {
    if (Wire.available()) { 
        secuenciaI2C = Wire.read();
        if (numBytes == 2) {
            ParametroI2C = Wire.read(); // Lee el segundo byte como parámetro
        }

        // Si es una solicitud de información, guardar la petición
        if (secuenciaI2C == Config::States::CAMPANARIO || secuenciaI2C == Config::States::HORA || 
        secuenciaI2C == Config::States::FECHA_HORA || secuenciaI2C == Config::States::FECHA_HORA_O_TEMPORIZACION) {
            requestI2C = secuenciaI2C;
            secuenciaI2C = 0;                       // Resetea para evitar ejecución como comando
            DBG_I2C_REQ("I2CServicio -> Solicitud request: " + String(requestI2C));
        }else{
            DBG_I2C_SEQ("I2CServicio -> Secuencia recibida por I2C: " + String(secuenciaI2C));
            if (numBytes == 2) {
                DBG_I2C_SEQ("I2CServicio -> Parametro recibido por I2C: " + String(ParametroI2C));
            }
        }
    }
}

/**
 * @brief Callback para enviar datos solicitados al maestro I2C
 * 
 * @details Función callback ejecutada automáticamente cuando el maestro I2C
 *          solicita datos del esclavo. Analiza el tipo de información solicitada
 *          y delega a la función específica correspondiente.
 *          
 *          **TIPOS DE RESPUESTA SOPORTADOS:**
 *          - CAMPANARIO: Solo estado del campanario (1 byte)
 *          - HORA: Estado + hora + minuto + segundo (4 bytes)  
 *          - FECHA_HORA: Estado + día + mes + año + hora + minuto + segundo (7 bytes)
 *          - FECHA_HORA_O_TEMPORIZACION: Fecha/hora normal o temporización calefacción
 * 
 * @note **CONTEXTO INTERRUPCIÓN:** Ejecuta en contexto de interrupción I2C
 * @note **RESPUESTA INMEDIATA:** Debe enviar datos sincronizadamente con maestro
 * @note **RESET:** Resetea requestI2C tras procesar para evitar repeticiones
 * 
 * @warning **INTERRUPCIÓN:** Mantener procesamiento mínimo y rápido
 * @warning **SINCRONIZACIÓN:** Maestro debe estar preparado para recibir bytes exactos
 * @warning **DATOS VÁLIDOS:** Asegurar que datos enviados sean válidos y actuales
 * 
 * @see Wire.onRequest() - Función que registra este callback
 * @see requestI2C - Variable que determina tipo de respuesta a enviar
 * @see enviarEstadoI2C() - Función para enviar solo estado
 * @see enviarHoraI2C() - Función para enviar hora completa
 * @see enviarFechaHoraI2C() - Función para enviar fecha y hora
 * @see enviarFechaoTemporizacionI2C() - Función inteligente fecha vs temporización
 * 
 * @since v1.0 - Envío básico de estado
 * @since v2.0 - Múltiples tipos de respuesta y temporización calefacción
 * 
 * @author Julian Salas Bartolomé
 */
void enviarRequest() {
    DBG_I2C_REQ("I2CServicio -> enviarRequest -> Respuesta a " + String(requestI2C) + " por I2C");
    switch (requestI2C) {
        case Config::States::CAMPANARIO:
            enviarEstadoI2C();
            break;
        case Config::States::HORA:
            enviarHoraI2C();
            break;
        case Config::States::FECHA_HORA:
            enviarFechaHoraI2C();
            break;
        case Config::States::FECHA_HORA_O_TEMPORIZACION:
            enviarFechaoTemporizacionI2C();
            break;
    }
    requestI2C = 0; // Resetea la solicitud I2C
}

/**
 * @brief Envía únicamente el estado actual del campanario a través de I2C
 * 
 * @details Función especializada que responde a solicitudes I2C del tipo CAMPANARIO
 *          enviando solo el estado actual del sistema como un único byte.
 *          Es la respuesta más básica y rápida del protocolo I2C.
 *          
 *          **DATOS ENVIADOS:**
 *          - Byte 0: Estado del campanario (Config::States::*)
 *          
 *          **ESTADOS TÍPICOS:**
 *          - STOP: Sistema detenido
 *          - MISA: Secuencia de misa activa
 *          - DIFUNTOS: Secuencia de difuntos activa
 *          - Bits adicionales para calefacción, alarmas, etc.
 * 
 * @note **TAMAÑO:** 1 byte total enviado
 * @note **VELOCIDAD:** Respuesta más rápida del protocolo I2C
 * @note **USO:** Para verificación rápida de estado sin datos temporales
 * 
 * @warning **CONTEXTO:** Ejecuta en callback de interrupción I2C
 * @warning **SINCRONIZACIÓN:** Maestro debe leer exactamente 1 byte
 * 
 * @see Campanario.GetEstadoCampanario() - Función que obtiene el estado
 * @see Config::States - Constantes de estados utilizadas
 * @see Wire.write() - Función utilizada para envío
 * 
 * @since v1.0
 * @author Julian Salas Bartolomé
 */
void enviarEstadoI2C() {
    int nEstadoCampanario = Campanario.GetEstadoCampanario();
    Wire.write(nEstadoCampanario);
    DBG_I2C_REQ("I2CServicio -> Estado enviado por I2C: " + String(nEstadoCampanario));
}

/**
 * @brief Envía hora actual y estado del campanario a través de I2C
 * 
 * @details Función que responde a solicitudes I2C del tipo HORA enviando
 *          el estado del campanario junto con la hora actual obtenida del RTC.
 *          Utiliza sincronización NTP para garantizar precisión temporal.
 *          
 *          **DATOS ENVIADOS (4 bytes):**
 *          - Byte 0: Estado del campanario (Config::States::*)
 *          - Byte 1: Hora actual (0-23)
 *          - Byte 2: Minutos actuales (0-59)  
 *          - Byte 3: Segundos actuales (0-59)
 *          
 *          **MANEJO DE ERRORES:**
 *          - Si RTC no sincronizado: envía 0xFF en campos temporales
 *          - Si getLocalTime() falla: envía 0xFF en campos temporales
 *          - Estado siempre se envía correctamente
 * 
 * @note **TAMAÑO:** 4 bytes total enviados
 * @note **PRECISIÓN:** Depende de sincronización NTP previa exitosa
 * @note **ERROR:** 0xFF indica dato temporal no válido/disponible
  * 
 * @warning **CONTEXTO:** Ejecuta en callback de interrupción I2C
 * @warning **SINCRONIZACIÓN:** Maestro debe leer exactamente 4 bytes
 * @warning **NTP:** Requiere sincronización NTP previa para datos válidos
 * 
 * @see RTC::isNtpSync() - Verificación de sincronización NTP
 * @see getLocalTime() - Función ESP32 para obtener tiempo local
 * @see Campanario.GetEstadoCampanario() - Obtención del estado
 * @see Wire.write() - Función utilizada para envío de cada byte
 * 
 * @since v1.0 - Funcionalidad básica
 * @since v2.0 - Manejo robusto de errores NTP
 * 
 * @author Julian Salas Bartolomé
 */
void enviarHoraI2C() {
    uint8_t hora = 0xFF;
    uint8_t minuto = 0xFF;
    uint8_t segundos = 0xFF;
    struct tm localTime;
    int nEstadoCampanario = Campanario.GetEstadoCampanario();

    if (RTC::isNtpSync() && getLocalTime(&localTime)) {
        hora = (uint8_t)localTime.tm_hour;
        minuto = (uint8_t)localTime.tm_min;
        segundos = (uint8_t)localTime.tm_sec;

        DBG_I2C_REQ_PRINTF("I2CServicio -> Hora enviada por I2C: %02d:%02d:%02d\n", hora, minuto, segundos);
    } else {
        DBG_I2C_REQ("I2CServicio -> Error obteniendo hora para enviar por I2C");
    }
    
    Wire.write(nEstadoCampanario);
    Wire.write(hora);
    Wire.write(minuto);
    Wire.write(segundos);
}

/**
 * @brief Envía fecha y hora completa junto con estado del campanario a través de I2C
 * 
 * @details Función que responde a solicitudes I2C del tipo FECHA_HORA enviando
 *          información temporal completa del sistema. Proporciona datos completos
 *          para sincronización temporal del dispositivo maestro.
 *          
 *          **DATOS ENVIADOS (7 bytes):**
 *          - Byte 0: Estado del campanario (Config::States::*)
 *          - Byte 1: Día del mes (1-31)
 *          - Byte 2: Mes (1-12, no 0-11 como tm_mon)
 *          - Byte 3: Año (00-99, últimos 2 dígitos del año 20XX)
 *          - Byte 4: Hora (0-23)
 *          - Byte 5: Minutos (0-59)
 *          - Byte 6: Segundos (0-59)
 *          
 *          **CONVERSIONES REALIZADAS:**
 *          - tm_mon + 1: Convierte 0-11 a 1-12 para meses
 *          - tm_year % 100: Convierte año completo a 2 dígitos
 *          - Casting a uint8_t para todos los campos temporales
 *          
 *          **MANEJO DE ERRORES:**
 *          - Si RTC no sincronizado: envía 0xFF en todos los campos temporales
 *          - Si getLocalTime() falla: envía 0xFF en todos los campos temporales
 *          - Estado siempre se envía correctamente
 * 
 * @note **TAMAÑO:** 7 bytes total enviados
 * @note **FORMATO:** DD/MM/YY HH:MM:SS compatible con formatos estándar
 * @note **PRECISIÓN:** Depende de sincronización NTP previa exitosa
 * @note **ERROR:** 0xFF indica datos temporales no válidos/disponibles
 * 
 * @warning **CONTEXTO:** Ejecuta en callback de interrupción I2C
 * @warning **SINCRONIZACIÓN:** Maestro debe leer exactamente 7 bytes
 * @warning **NTP:** Requiere sincronización NTP previa para datos válidos
 * @warning **AÑO 2100:** Problema del año 2100 por usar solo 2 dígitos
 * 
 * @see RTC::isNtpSync() - Verificación de sincronización NTP
 * @see getLocalTime() - Función ESP32 para obtener tiempo local
 * @see Campanario.GetEstadoCampanario() - Obtención del estado
 * @see struct tm - Estructura de tiempo utilizada
 * @see Wire.write() - Función utilizada para envío de cada byte
 * 
 * @since v1.0 - Funcionalidad básica
 * @since v2.0 - Conversiones mejoradas y manejo robusto de errores
 * 
 * @author Julian Salas Bartolomé
 */
void enviarFechaHoraI2C() {
    uint8_t dia = 0xFF;
    uint8_t mes = 0xFF;
    uint8_t ano = 0xFF;
    uint8_t hora = 0xFF;
    uint8_t minuto = 0xFF;
    uint8_t segundos = 0xFF;
    struct tm localTime;
    int nEstadoCampanario = Campanario.GetEstadoCampanario();
    
    if (RTC::isNtpSync() && getLocalTime(&localTime)) {
        dia = (uint8_t)localTime.tm_mday;
        mes = (uint8_t)(localTime.tm_mon + 1);
        ano = (uint8_t)(localTime.tm_year % 100);
        hora = (uint8_t)localTime.tm_hour;
        minuto = (uint8_t)localTime.tm_min;
        segundos = (uint8_t)localTime.tm_sec;

        DBG_I2C_REQ_PRINTF("I2CServicio -> Fecha y hora enviada por I2C: %02d/%02d/%02d %02d:%02d:%02d\n", 
                     dia, mes, ano, hora, minuto, segundos);
    } else {
        DBG_I2C_REQ("I2CServicio -> Error obteniendo fecha y hora para enviar por I2C");
    }
    
    Wire.write(nEstadoCampanario);
    Wire.write(dia);
    Wire.write(mes);
    Wire.write(ano);
    Wire.write(hora);
    Wire.write(minuto);
    Wire.write(segundos);
}

/**
 * @brief Envía temporización de calefacción convertida a formato hora/minuto/segundo
 * 
 * @details Función especializada que convierte el tiempo restante de calefacción
 *          (almacenado en segundos) a formato HH:MM:SS y lo envía como si fuera
 *          una fecha/hora. Se utiliza para informar al maestro I2C del tiempo
 *          restante de calefacción cuando está activa.
 *          
 *          **DATOS ENVIADOS (7 bytes):**
 *          - Byte 0: Estado del campanario (Config::States::*)
 *          - Byte 1: 0xFF (día - no aplicable para temporización)
 *          - Byte 2: 0xFF (mes - no aplicable para temporización)  
 *          - Byte 3: 0xFF (año - no aplicable para temporización)
 *          - Byte 4: Horas restantes (0-255, calculadas de nSegundosTemporizacion)
 *          - Byte 5: Minutos restantes (0-59, calculados de nSegundosTemporizacion)
 *          - Byte 6: Segundos restantes (0-59, calculados de nSegundosTemporizacion)
 *          
 *          **CONVERSIÓN TEMPORAL:**
 *          1. nSegundosTemporizacion / 3600 = horas restantes
 *          2. (nSegundosTemporizacion % 3600) / 60 = minutos restantes  
 *          3. nSegundosTemporizacion % 60 = segundos restantes
 * 
 * @note **FORMATO:** Reutiliza estructura de 7 bytes de enviarFechaHoraI2C()
 * @note **DIFERENCIACIÓN:** Campos de fecha en 0xFF, solo tiempo es válido
 * @note **VARIABLE GLOBAL:** Utiliza nSegundosTemporizacion de Auxiliar.h
 * @note **USO:** Solo llamada desde enviarFechaoTemporizacionI2C()
 * 
 * @warning **LÍMITE HORAS:** Máximo 255 horas (uint8_t), temporizaciones mayores se truncan
 * @warning **VARIABLE GLOBAL:** Depende de nSegundosTemporizacion actualizada
 * @warning **CONTEXTO:** Ejecuta en callback de interrupción I2C
 * 
 * @see nSegundosTemporizacion - Variable global utilizada (Auxiliar.h)
 * @see enviarFechaoTemporizacionI2C() - Función que decide cuándo llamar esta
 * @see Config::States::BIT_CALEFACCION - Bit que indica calefacción activa
 * @see Wire.write() - Función utilizada para envío de cada byte
 * 
 * @since v2.0
 * @author Julian Salas Bartolomé
 */
void enviarEstadoTemporizacionI2C() {
    uint8_t dia = 0xFF;
    uint8_t mes = 0xFF;
    uint8_t ano = 0xFF;
    uint8_t hora = 0xFF;
    uint8_t minuto = 0xFF;
    uint8_t segundos = 0xFF;
    int nEstadoCampanario = Campanario.GetEstadoCampanario();
    
    // Convertir nSegundosTemporizacion a horas, minutos y segundos
    unsigned long nSgTemporizacion = nSegundosTemporizacion;
    hora = (uint8_t)(nSgTemporizacion / 3600);
    minuto = (uint8_t)((nSgTemporizacion % 3600) / 60);
    segundos = (uint8_t)(nSgTemporizacion % 60);
    DBG_I2C_REQ_PRINTF("I2CServicio -> Temporización enviada por I2C: %02d:%02d:%02d\n", hora, minuto, segundos);
    Wire.write(nEstadoCampanario);
    Wire.write(dia);
    Wire.write(mes);
    Wire.write(ano);
    Wire.write(hora);
    Wire.write(minuto);
    Wire.write(segundos);
}

/**
 * @brief Función inteligente que decide enviar fecha/hora normal o temporización de calefacción
 * 
 * @details Función de decisión que analiza el estado del campanario para determinar
 *          si debe enviar la fecha/hora actual del sistema o el tiempo restante de
 *          temporización de calefacción. Implementa lógica de negocio para respuesta
 *          contextual según el estado de la calefacción.
 *          
 *          **LÓGICA DE DECISIÓN:**
 *          1. Obtiene estado actual del campanario
 *          2. Verifica si bit de calefacción (BIT_CALEFACCION) está activo
 *          3. Si calefacción activa: llama a enviarEstadoTemporizacionI2C()
 *          4. Si calefacción inactiva: llama a enviarFechaHoraI2C()
 *          
 *          **CASOS DE USO:**
 *          - **Calefacción ON:** Maestro recibe tiempo restante (HH:MM:SS temporización)
 *          - **Calefacción OFF:** Maestro recibe fecha/hora actual del sistema
 *          - **Reposo:** Maestro puede mostrar countdown vs reloj según contexto
 * 
 * @note **BIT CALEFACCIÓN:** Utiliza Config::States::BIT_CALEFACCION para decisión
 * @note **DELEGACIÓN:** No envía datos directamente, delega a funciones especializadas
 * @note **CONTEXTO MAESTRO:** Permite al maestro mostrar información contextual
 *
 * @warning **ESTADO CONSISTENTE:** Depende de que bit calefacción esté actualizado
 * @warning **SINCRONIZACIÓN:** Maestro debe interpretar correctamente el contexto
 * @warning **CONTEXTO:** Ejecuta en callback de interrupción I2C
 * 
 * @see Config::States::BIT_CALEFACCION - Bit utilizado para decisión
 * @see Campanario.GetEstadoCampanario() - Función que obtiene estado con bits
 * @see enviarEstadoTemporizacionI2C() - Función llamada si calefacción activa
 * @see enviarFechaHoraI2C() - Función llamada si calefacción inactiva
 * @see nSegundosTemporizacion - Variable global utilizada para temporización
 * 
 * @example
 * @code
 * // Maestro I2C solicita FECHA_HORA_O_TEMPORIZACION
 * // Si calefacción activa (bit 4 = 1):
 * //   Recibe: [Estado][0xFF][0xFF][0xFF][02][30][45] = 2h 30m 45s restantes
 * // Si calefacción inactiva (bit 4 = 0):
 * //   Recibe: [Estado][15][09][25][14][30][45] = 15/09/25 14:30:45
 * @endcode
 * 
 * @since v2.0
 * @author Julian Salas Bartolomé
 */
void enviarFechaoTemporizacionI2C(void) {
    int nEstadoCampanario = Campanario.GetEstadoCampanario();

     
    // Si el bit 4 está activo (calefacción encendida)
    if (nEstadoCampanario & Config::States::BIT_CALEFACCION) {
        enviarEstadoTemporizacionI2C();
        DBG_I2C_REQ("I2CServicio -> enviarFechaoTemporizacionI2C ->Enviando estado de temporización por I2C");
    } else {
        enviarFechaHoraI2C();
        DBG_I2C_REQ("I2CServicio -> enviarFechaoTemporizacionI2C ->Enviando fecha y hora por I2C");
    }
}