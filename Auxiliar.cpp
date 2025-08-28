// Auxiliar.cpp
#include "Auxiliar.h"
#include "ConexionWifi.h"
#include "DNSServicio.h"  // Para ActualizaDNS



// Definiciones únicas de variables globales
CAMPANARIO Campanario;
struct tm timeinfo;
int ultimoMinuto = -1;
int nCampanaTocada = 0;
int ultimaHora = -1;
volatile uint8_t secuenciaI2C = 0;
uint8_t requestI2C = 0;
uint8_t ParametroI2C = 0;
bool lConexionInternet = false;
bool lProteccionCampanadas = false;
bool lProteccionCampanadasAnterior = false;
unsigned long ultimoCheckInternet = 0;
const unsigned long intervaloCheckInternet = 5 * 60 * 1000;
int nTemporizacionCalefaccion = 45;
double nSegundosTemporizacion = 0;



void ChekearCuartos(void) 
{
    if (!Campanario.GetEstadoSecuencia()) {
        if (getLocalTime(&timeinfo)) {
            int hora = timeinfo.tm_hour;
            int minuto = timeinfo.tm_min;
            if (minuto != ultimoMinuto) {
                ultimoMinuto = minuto;
                if (minuto % 30 == 0) {
                    ws.textAll("REDIRECT:/Campanas.html");
                    if (minuto == 0) {
                        if(!EsHorarioNocturno()) {
                            Campanario.TocaHoraSinCuartos(hora);
                        }
                        if (hora == 12) {
                            if(hayInternet()){
                                RTC::begin();
                            }
                        }   
                    } else {
                        if (!EsHorarioNocturno()) {
                            Campanario.TocaMediaHora();
                        }
                    }
                    if (hayInternet()){
                        ActualizaDNS(configWiFi.dominio);
                    } else {
                        #ifdef DEBUGAUXILIAR
                            Serial.println("Sin conexión a internet. No se actualiza DNS.");
                        #endif
                    }
                }
            }    
        } else {
            #ifdef DEBUGAUXILIAR
                Serial.println("No se pudo obtener la hora del RTC");
            #endif
        }
    }    
}

void TestCampanadas(void) 
{
    nCampanaTocada = Campanario.ActualizarSecuenciaCampanadas();
    if (nCampanaTocada > 0) {
        #ifdef DEBUGAUXILIAR
            Serial.print("Campana tocada: ");
            Serial.println(nCampanaTocada);
        #endif
        Campanario.ResetCampanaTocada();
        ws.textAll("CAMPANA:"+String(nCampanaTocada));
        if (!Campanario.GetEstadoSecuencia()) {
            #ifdef DEBUGAUXILIAR
                Serial.println("Secuencia de campanadas finalizada.");
            #endif
            Campanario.ParaSecuencia();
            ws.textAll("REDIRECT:/index.html");
        }
    }
}
    /**
     * @brief Recibe una secuencia de datos por I2C y procesa los bytes recibidos.
     * 
     * Esta función lee los datos disponibles en el bus I2C utilizando la librería Wire.
     * El primer byte recibido se almacena en 'secuenciaI2C'. Si se reciben dos bytes,
     * el segundo byte se interpreta como un parámetro adicional y se almacena en 'ParametroI2C'.
     * 
     * Si la secuencia recibida corresponde a una solicitud de estado, hora o fecha/hora,
     * se guarda la solicitud en 'requestI2C' y se resetea 'secuenciaI2C'.
     * 
     * En modo depuración (DEBUGAUXILIAR), se imprime por el puerto serie la información recibida.
     * 
     * @param numBytes Número de bytes esperados en la secuencia recibida (1 o 2).
     */
    void recibirSecuencia(int numBytes) 
    {
        if (Wire.available()) { 

            secuenciaI2C = Wire.read();
            if (numBytes == 2) {
                ParametroI2C = Wire.read(); // Lee el primer byte que es el comando de solicitud
            }
            #ifdef DEBUGAUXILIAR
                Serial.println("Secuencia recibida por I2C: " + String(secuenciaI2C));
            #endif
            if (numBytes == 2 )
            {
                #ifdef DEBUGAUXILIAR
                    Serial.print("Parametro recibido por I2C: ");
                    Serial.println(ParametroI2C);
                #endif

            }
            if (secuenciaI2C == EstadoCampanario || secuenciaI2C == EstadoHora || secuenciaI2C == EstadoFechaHora || secuenciaI2C == EstadoFechaHoraoTemporizacion) {
                requestI2C = secuenciaI2C;              // Guarda la solicitud I2C para enviar la hora o el estado
                secuenciaI2C = 0;                        // Resetea la secuencia si es una solicitud de estado o hora
            }
        }
    }

/**
 * @brief Envía la hora actual y el estado del campanario a través del bus I2C.
 *
 * Esta función obtiene la hora local desde el RTC si está sincronizado con NTP.
 * Si la obtención es exitosa, envía el estado del campanario, la hora, los minutos y los segundos por I2C.
 * En caso de error al obtener la hora, se envían valores por defecto (0xFF) para hora, minuto y segundos.
 * 
 * @note Si está definido DEBUGAUXILIAR, se imprime información de depuración por el puerto serie.
 */
    void enviarHoraI2C() 
    {
        uint8_t hora = 0xFF;
        uint8_t minuto = 0xFF;
        uint8_t segundos = 0xFF; // Inicializa las variables de hora, minuto y segundos
        struct tm localTime;
        int nEstadoCampanario = Campanario.GetEstadoCampanario(); // Obtiene el estado del campanario
    
        if (RTC::isNtpSync() && getLocalTime(&localTime)) {
            hora = (uint8_t)localTime.tm_hour;
            minuto = (uint8_t)localTime.tm_min;
            segundos = (uint8_t)localTime.tm_sec; // Obtiene la hora, minuto y segundos del RTC
            #ifdef DEBUGAUXILIAR
                printf("enviarHoraI2C -> Hora enviada por I2C: %02d:%02d\n", hora, minuto);
            #endif
        } else {
            #ifdef DEBUGAUXILIAR
                Serial.println("Error obteniendo hora para enviar por I2C");
            #endif    
        }
        Wire.write(nEstadoCampanario); // Envía el estado del campanario
        Wire.write(hora);
        Wire.write(minuto);
        Wire.write(segundos); // Envía la hora, minuto y segundos por I2C
    }

/**
 * @brief Envía la fecha y hora completa junto con el estado del campanario a través del bus I2C.
 *
 * Esta función obtiene la fecha y hora local desde el RTC si está sincronizado con NTP.
 * Si la obtención es exitosa, envía el estado del campanario, día, mes, año, hora, minutos y segundos por I2C.
 * En caso de error al obtener la fecha/hora, se envían valores por defecto (0xFF) para todos los campos.
 * 
 * Orden de envío por I2C:
 * 1. Estado del campanario (1 byte)
 * 2. Día (1 byte)
 * 3. Mes (1 byte) 
 * 4. Año (1 byte - solo los dos últimos dígitos)
 * 5. Hora (1 byte)
 * 6. Minutos (1 byte)
 * 7. Segundos (1 byte)
 * 
 * @note Si está definido DEBUGAUXILIAR, se imprime información de depuración por el puerto serie.
 */
    void enviarFechaHoraI2C() 
    {
        uint8_t dia = 0xFF;
        uint8_t mes = 0xFF;
        uint8_t ano = 0xFF;
        uint8_t hora = 0xFF;
        uint8_t minuto = 0xFF;
        uint8_t segundos = 0xFF; // Inicializa las variables de fecha y hora
        struct tm localTime;
        int nEstadoCampanario = Campanario.GetEstadoCampanario(); // Obtiene el estado del campanario
        if (RTC::isNtpSync() && getLocalTime(&localTime)) {
            dia = (uint8_t)localTime.tm_mday;        // Día del mes (1-31)
            mes = (uint8_t)(localTime.tm_mon + 1);   // Mes (1-12) - tm_mon va de 0-11
            ano = (uint8_t)(localTime.tm_year % 100); // Año (solo los 2 últimos dígitos)
            hora = (uint8_t)localTime.tm_hour;       // Hora (0-23)
            minuto = (uint8_t)localTime.tm_min;      // Minutos (0-59)
            segundos = (uint8_t)localTime.tm_sec;    // Segundos (0-59)
            #ifdef DEBUGAUXILIAR
                Serial.printf("enviarFechaHoraI2C -> Fecha y hora enviada por I2C: %02d/%02d/%02d %02d:%02d:%02d\n", 
                             dia, mes, ano, hora, minuto, segundos);
            #endif
        } else {
            #ifdef DEBUGAUXILIAR
                Serial.println("Error obteniendo fecha y hora para enviar por I2C");
            #endif
        }
        Wire.write(nEstadoCampanario); // Envía el estado del campanario
        Wire.write(dia);               // Envía el día
        Wire.write(mes);               // Envía el mes
        Wire.write(ano);               // Envía el año (2 dígitos)
        Wire.write(hora);              // Envía la hora
        Wire.write(minuto);            // Envía los minutos
        Wire.write(segundos);          // Envía los segundos
    }

    void enviarEstadoTemporizacionI2C() 
    {
        uint8_t dia = 0xFF;
        uint8_t mes = 0xFF;
        uint8_t ano = 0xFF;
        uint8_t hora = 0xFF;
        uint8_t minuto = 0xFF;
        uint8_t segundos = 0xFF; // Inicializa las variables de fecha y hora
        struct tm localTime;
        int nEstadoCampanario = Campanario.GetEstadoCampanario(); // Obtiene el estado del campanario
        //convierte los nSegundosTemporizacion a Horas minutos y segundos
        unsigned long nSgTemporizacion = nSegundosTemporizacion; // Obtiene los segundos de temporización del campanario
        hora = (uint8_t)((unsigned long)nSgTemporizacion / 3600);
        minuto = (uint8_t)((unsigned long)(nSgTemporizacion % 3600) / 60);
        segundos = (uint8_t)((unsigned long)(nSgTemporizacion % 60));
/*
        if (RTC::isNtpSync() && getLocalTime(&localTime)) {
            dia = (uint8_t)localTime.tm_mday;        // Día del mes (1-31)
            mes = (uint8_t)(localTime.tm_mon + 1);   // Mes (1-12) - tm_mon va de 0-11
            ano = (uint8_t)(localTime.tm_year % 100); // Año (solo los 2 últimos dígitos)
            hora = (uint8_t)localTime.tm_hour;       // Hora (0-23)
            minuto = (uint8_t)localTime.tm_min;      // Minutos (0-59)
            segundos = (uint8_t)localTime.tm_sec;    // Segundos (0-59)
            #ifdef DEBUGAUXILIAR
                Serial.printf("enviarEstadoTemporizacionI2C ->Temporizacion enviada por I2C: %02d/%02d/%02d %02d:%02d:%02d\n", 
                             dia, mes, ano, hora, minuto, segundos);
            #endif
        } else {
            #ifdef DEBUGAUXILIAR
                Serial.println("Error obteniendo fecha y hora para enviar por I2C");
            #endif
        }
*/            
        Wire.write(nEstadoCampanario); // Envía el estado del campanario
        Wire.write(dia);               // Envía el día
        Wire.write(mes);               // Envía el mes
        Wire.write(ano);               // Envía el año (2 dígitos)
        Wire.write(hora);              // Envía la hora
        Wire.write(minuto);            // Envía los minutos
        Wire.write(segundos);          // Envía los segundos
    }
    void enviarFechaoTemporizacionI2C (void)
    {
        int nEstadoCampanario = Campanario.GetEstadoCampanario(); // Obtiene el estado del campanario
        #ifdef DEBUGAUXILIAR
            Serial.printf("enviarFechaoTemporizacionI2C -> Estado del campanario: %d\n", nEstadoCampanario);
        #endif
        //Si el bit 4 de nEstadoCampanario está activo, es decir, si la calefaccion esta encendida
        if (nEstadoCampanario & (1 << 4)) {
            enviarEstadoTemporizacionI2C();
        }else{
            enviarFechaHoraI2C();
        }
    }
/**
 * @brief Procesa una solicitud recibida por I2C y responde según el tipo de petición.
 *
 * Esta función verifica el valor de la variable 'requestI2C' para determinar el tipo de solicitud recibida.
 * Dependiendo del caso, envía el estado del campanario o la hora actual a través de I2C.
 * Después de procesar la solicitud, reinicia 'requestI2C' para esperar una nueva petición.
 *
 * @note Si está definido DEBUGAUXILIAR, imprime por el puerto serie información sobre la solicitud recibida.
 */
    void enviarRequest() 
    {
        #ifdef DEBUGAUXILIAR
            Serial.print("enviarRequest -> Solicitud recibida por I2C: ");
            Serial.println(requestI2C);
        #endif
        switch (requestI2C) {
            case EstadoCampanario: // Si se solicita el estado del campanario
                enviarEstadoI2C(); // Envía el estado del campanario
                break;
            case EstadoHora: // Si se solicita la hora
                enviarHoraI2C(); // Envía la hora actual
                break;
            case EstadoFechaHora:
                enviarFechaHoraI2C(); // Envía la fecha actual
                break;
            case EstadoFechaHoraoTemporizacion: 
                enviarFechaoTemporizacionI2C();
                break;
        }
        requestI2C = 0; // Resetea la solicitud I2C para esperar la siguiente
    }
/**
 * @brief Envía el estado actual del campanario a través de comunicación I2C
 * 
 * Esta función obtiene el estado actual del campanario y lo transmite
 * a través del bus I2C. Si está definido DEBUGAUXILIAR, también imprime
 * el estado enviado por el puerto serie.
 * 
 * @note Requiere que la comunicación I2C (Wire) esté inicializada
 * @note Utiliza la clase Campanario para obtener el estado
 */
    void enviarEstadoI2C() {
        int nEstadoCampanario = Campanario.GetEstadoCampanario(); // Obtiene el estado del campanario
        Wire.write(nEstadoCampanario); // estadoCampanario es la variable que quieres enviar
        #ifdef DEBUGAUXILIAR
            Serial.println("enviarEstadoI2C -> Estado enviado por I2C: " + String(nEstadoCampanario));
        #endif
    }

/**
 * @brief Ejecuta diferentes secuencias de control del campanario según el parámetro recibido
 * 
 * @param nSecuencia Número entero que indica la secuencia a ejecutar:
 *                   - EstadoDifuntos: Activa secuencia de toque de difuntos
 *                   - EstadoMisa: Activa secuencia de toque de misa
 *                   - EstadoStop: Detiene cualquier secuencia en ejecución
 *                   - EstadoCalefaccionOn: Enciende el sistema de calefacción
 *                   - EstadoCalefaccionOff: Apaga el sistema de calefacción
 *                   - EstadoProteccionCampanadas: Notifica el estado de la protección de campanadas
 * 
 * @details La función maneja diferentes estados del campanario y notifica a los
 *          clientes web conectados mediante websockets sobre los cambios realizados.
 *          Para los toques de campana, redirige a los clientes a la página de campanas.
 *          Para el control de calefacción, notifica el estado actual.
 *          Si la secuencia no es reconocida, genera un mensaje de debug si DEBUGSERVIDOR está definido.
 */
    void EjecutaSecuencia (int nSecuencia) 
    {
        #ifdef DEBUGSERVIDOR
            Serial.println("Ejecutando secuencia: ");
            Serial.println(nSecuencia);
        #endif
        switch (nSecuencia) {
            case EstadoDifuntos:
                Campanario.TocaDifuntos();                  // Toca la secuencia de difuntos
                ws.textAll("REDIRECT:/Campanas.html");      // Indica a los clientes que deben redirigir a la pantalla de presentacion de las campanas
                break;
           case EstadoMisa:
                Campanario.TocaMisa(); // Toca la secuencia de hora
                ws.textAll("REDIRECT:/Campanas.html");      // Indica a los clientes que deben redirigir a la pantalla de presentacion de las campanas
                break;
            case EstadoStop:
                Campanario.ParaSecuencia();                 // Llama a la función para detener la secuencia de campanadas
                ws.textAll("REDIRECT:/index.html");         // Indica a los clientes que deben redirigir a la página principal
                break;
            case EstadoCalefaccionOn:
                Campanario.EnciendeCalefaccion(nTemporizacionCalefaccion);         // Llama a la función para encender la calefacción con temporizacion
                ws.textAll("CALEFACCION:ON");               // Envía el estado de la calefacción a todos los clientes conectados
                break;
            case EstadoCalefaccionOff:
                Campanario.ApagaCalefaccion();              // Llama a la función para apagar la calefacción
                ws.textAll("CALEFACCION:OFF");              // Envía el estado de la calefacción a todos los clientes conectados
                break;
            case EstadoProteccionCampanadas:
                // Notifica el estado de la protección de campanadas a todos los clientes conectados
                ws.textAll(lProteccionCampanadas ? "PROTECCION:ON" : "PROTECCION:OFF");
                #ifdef DEBUGAUXILIAR
                    Serial.printf("EjecutaSecuencia -> Protección campanadas notificada: %s\n", lProteccionCampanadas ? "ACTIVA" : "INACTIVA");
                #endif
                break;
            case EstadoSetTemporizador:                     //Fija el temporizador de la calefacción
                nTemporizacionCalefaccion = (int)ParametroI2C; // Asigna el valor del parámetro recibido al temporizador de calefacción
                Campanario.EnciendeCalefaccion(nTemporizacionCalefaccion); // Llama a la función para fijar el temporizador de calefacción
                ws.textAll("CALEFACCION:ON:" + String(nTemporizacionCalefaccion));
                #ifdef DEBUGAUXILIAR
                    Serial.printf("EjecutaSecuencia -> Temporizador de calefacción fijado a %d minutos.\n", nTemporizacionCalefaccion);
                #endif
                break;
            default:
                #ifdef DEBUGSERVIDOR
                    Serial.println("Secuencia no reconocida.");
                #endif
                break;
        }
    }
    void TestInternet(void) {
        bool estadoAnteriorInternet = lConexionInternet; // Guarda el estado anterior de la conexión
        if (!hayInternet()) { // hayInternet() debe comprobar acceso real a internet
          lConexionInternet = ConectarWifi(configWiFi); // Intenta reconectar
          if (lConexionInternet) {
              #ifdef DEBUG
                Serial.println("TestInternet -> Reconectado a internet correctamente.");
              #endif
              ServidorOn(configWiFi.usuario, configWiFi.clave); // Reinicia el servidor si es necesario
              if (!estadoAnteriorInternet) { // Si el estado cambió de desconectado a conectado
                  Campanario.SetInternetConectado(); // Notifica al campanario que hay internet
              }
          } else {
              #ifdef DEBUG
                Serial.println("TestInternet -> Sin conexión a internet. Funcionando en modo local.");
              #endif
              if (estadoAnteriorInternet) { // Si el estado cambió de conectado a desconectado
                  Campanario.ClearInternetConectado(); // Notifica al campanario que no hay internet
              }
          }
        }else{
            lConexionInternet = true; // Asegura que la variable esté actualizada
            if (!estadoAnteriorInternet) { // Si el estado cambió de desconectado a conectado
                Campanario.SetInternetConectado(); // Notifica al campanario que hay internet
            }
            #ifdef DEBUG
                Serial.println("TestInternet -> Conexión a internet activa.");
            #endif
        }
    }

    /**
     * @brief Comprueba si la hora actual se encuentra dentro del horario nocturno.
     *
     * Esta función obtiene la hora local actual y verifica si está dentro del rango definido
     * por las variables globales InicioHorarioNocturno y FinHorarioNocturno. El horario nocturno
     * se considera desde InicioHorarioNocturno (inclusive) hasta FinHorarioNocturno (exclusive).
     *
     * @return true si la hora actual está en el horario nocturno, false en caso contrario o si no se pudo obtener la hora.
     */
    bool EsHorarioNocturno (void) {
        if (!RTC::isNtpSync()) {
            #ifdef DEBUG
                Serial.println("EsHorarioNocturno -> RTC no sincronizado con NTP.");
            #endif
            return false; // Si el RTC no está sincronizado, no se puede determinar el horario nocturno
        }
        struct tm localTime;
        if (getLocalTime(&localTime)) { // Obtiene la hora local
            int hora = localTime.tm_hour; // Obtiene la hora actual
            return (hora >= InicioHorarioNocturno || hora < FinHorarioNocturno); // Comprueba si está en horario nocturno
        }
        return false; // Si no se pudo obtener la hora, devuelve false
    }    

    /**
     * @brief Comprueba si estamos en el período de ±5 minutos alrededor de un toque de campanas.
     *
     * Esta función determina si el momento actual está dentro de la ventana de 5 minutos antes
     * o 5 minutos después de los momentos en que suenan las campanas:
     * - Toques de hora: minuto 0 (en punto)
     * - Toques de media hora: minuto 30
     * 
     * Períodos de actividad:
     * - Para hora en punto: entre 55-59 y 0-5 minutos
     * - Para media hora: entre 25-29 y 30-35 minutos
     *
     * @return true si estamos en período de toque de campanas (±5 min), false en caso contrario o si no se pudo obtener la hora.
     * 
     * @note Requiere que el RTC esté sincronizado con NTP para funcionar correctamente.
     */
    bool EsPeriodoToqueCampanas (void) {
        
        if (!RTC::isNtpSync()) {    
            #ifdef DEBUGPROTECCION
                Serial.println  ("EsPeriodoToqueCampanas -> RTC no sincronizado con NTP.");
            #endif
            // Detecta cambio de estado y notifica si es necesario
            if (lProteccionCampanadas != false) {                                           //Si esta la proteccion de campanadas la deshabilitamos
                lProteccionCampanadasAnterior = lProteccionCampanadas;
                lProteccionCampanadas = false;
                EjecutaSecuencia(EstadoProteccionCampanadas);                               // Notifica el cambio de estado a cliente web
                Campanario.ClearProteccionCampanadas();                                     // Desactiva la protección de campanadas
                #ifdef DEBUGPROTECCION
                    Serial.println("EsPeriodoToqueCampanas -> Protección desactivada (RTC no sincronizado)");
                #endif
            }
            return false; // Si el RTC no está sincronizado, no se puede determinar el período
        }
        
        struct tm localTime;                                                                //Si hay sincronizacion
        if (getLocalTime(&localTime)) {                                                     // Obtiene la hora local                            
            int minuto = localTime.tm_min;                                                  //Obtenemos el minuto actual
            bool periodoHoraEnPunto = (minuto >= 58) || (minuto < 3);                       // Comprueba si estamos ±5 minutos de la hora en punto (minuto 0)
            bool periodoMediaHora = (minuto >= 28 && minuto <= 32);                         // Comprueba si estamos ±5 minutos de la media hora (minuto 30)
            bool nuevoEstadoProteccion = (periodoHoraEnPunto || periodoMediaHora);          //Si esta en algun periodo de proteccion
            if (lProteccionCampanadas != nuevoEstadoProteccion) {                           // Si el estado de protección ha cambiado     
                lProteccionCampanadasAnterior = lProteccionCampanadas;                      // Guarda el estado anterior de la protección de campanadas 
                lProteccionCampanadas = nuevoEstadoProteccion;                              // Actualiza el estado de la protección de campanadas
                if (lProteccionCampanadas) {                                                // Si estamos en un período de toque de campanas
                    Campanario.SetProteccionCampanadas();                                   // Activa la protección de campanadas
                    #ifdef DEBUGPROTECCION
                        Serial.printf("EsPeriodoToqueCampanas -> Activando protección de campanadas (minuto %d)\n", minuto);
                    #endif
                } else {                                                                    // Si no estamos en un período de toque de campanas
                    Campanario.ClearProteccionCampanadas();                                 // Desactiva la protección de campanadas
                    #ifdef DEBUGPROTECCION
                        Serial.printf("EsPeriodoToqueCampanas -> Desactivando protección de campanadas (minuto %d)\n", minuto);
                    #endif
                }
                EjecutaSecuencia(EstadoProteccionCampanadas);                               // Notifica el cambio de estado Al cliete web
                #ifdef DEBUGPROTECCION
                    Serial.printf("EsPeriodoToqueCampanas -> Cambio de protección: %s -> %s (minuto %d)\n", lProteccionCampanadasAnterior ? "ACTIVA" : "INACTIVA",lProteccionCampanadas ? "ACTIVA" : "INACTIVA",minuto);
                #endif
            }
            #ifdef DEBUGPROTECCION
                if (nuevoEstadoProteccion) {
                    Serial.printf("EsPeriodoToqueCampanas -> Período activo (minuto %d): %s\n", minuto, periodoHoraEnPunto ? "Hora en punto" : "Media hora");
                }
            #endif
            
            return nuevoEstadoProteccion;
        }
        #ifdef DEBUGPROTECCION
            Serial.println("EsPeriodoToqueCampanas -> Error obteniendo hora del RTC");
        #endif
        if (lProteccionCampanadas != false) {                                                   // Si no se pudo obtener la hora, desactiva la protección de campanadas si estaba activa
            lProteccionCampanadasAnterior = lProteccionCampanadas;
            lProteccionCampanadas = false;
            EjecutaSecuencia(EstadoProteccionCampanadas); // Notifica el cambio de estado
            #ifdef DEBUGPROTECCION
                Serial.println("EsPeriodoToqueCampanas -> Protección desactivada (error obteniendo hora)");
            #endif
        }
        return false; // Si no se pudo obtener la hora, devuelve false
    }
    void SincronizaNTP( void ) {
        // Lógica para sincronizar NTP
        if(hayInternet()){
            RTC::begin();                       // Sincroniza el RTC con NTP si es mediodía
        }else{
            #ifdef DEBUGPROTECCION
                Serial.println("Auxiliar->SincronizaNTP->[NTP] Sin conexión a Internet");
            #endif
        }
    }
