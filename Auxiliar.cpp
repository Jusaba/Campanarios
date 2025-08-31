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
bool lConexionInternet = false;
bool lProteccionCampanadas = false;
bool lProteccionCampanadasAnterior = false;
unsigned long ultimoCheckInternet = 0;
const unsigned long intervaloCheckInternet = 5 * 60 * 1000;
int nTemporizacionCalefaccion = 45;
double nSegundosTemporizacion = 0;



/**
 * @brief Verifica si hay una campana que debe sonar y actualiza su estado.
 * 
 * Esta función llama al método ActualizarSecuenciaCampanadas() del objeto global
 * Campanario para verificar si hay una campana que debe sonar en la secuencia actual.
 * Si una campana ha sonado, envía un mensaje a todos los clientes WebSocket conectados
 * notificando qué campana ha sonado. Si la secuencia de campanadas ha finalizado,
 * detiene la secuencia y envía una redirección a la página principal.
 * 
 * En modo depuración (DEBUGAUXILIAR), se imprime por el puerto serie información
 * sobre la campana que ha sonado y si la secuencia ha finalizado.
 */

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
                if (lProteccionCampanadas) {                                                    // 3. Actualizar estado en Campanario
                    Campanario.SetProteccionCampanadas();
                    DBG_AUX("EjecutaSecuencia -> Activando protección");
                } else {
                    Campanario.ClearProteccionCampanadas();
                    DBG_AUX("EjecutaSecuencia -> Desactivando protección");
                }
                ws.textAll(lProteccionCampanadas ? "PROTECCION:ON" : "PROTECCION:OFF");
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
 * @brief Verifica si el minuto actual está dentro de un período protegido para campanadas.
 * 
 * Esta función SOLO verifica el estado, NO modifica variables globales.
 * Devuelve true si estamos en período de protección, false en caso contrario.
 * 
 * @return true si estamos en período protegido, false en caso contrario.
 */
bool EsPeriodoToqueCampanas(void) {
    if (!RTC::isNtpSync()) {
        DBG_PROT("EsPeriodoToqueCampanas -> RTC no sincronizado con NTP.");
        return false; // Sin sync NTP = sin protección
    }
    
    struct tm localTime;
    if (!getLocalTime(&localTime)) {
        DBG_PROT("EsPeriodoToqueCampanas -> Error obteniendo hora del RTC");
        return false; // Sin hora = sin protección
    }
    
    int minuto = localTime.tm_min;
    bool periodoHoraEnPunto = (minuto >= Config::Time::INICIO_VENTANA_HORA) || 
                              (minuto <= Config::Time::FIN_VENTANA_HORA);        // 57,58,59,0,1,2,3
    
    bool periodoMediaHora = (minuto >= Config::Time::INICIO_VENTANA_MEDIA) && 
                            (minuto <= Config::Time::FIN_VENTANA_MEDIA);         // 27,28,29,30,31,32,33
    
    bool enPeriodoProteccion = (periodoHoraEnPunto || periodoMediaHora);
    
    #ifdef DEBUGPROTECCION
        if (enPeriodoProteccion) {
            DBG_PROT_PRINTF("EsPeriodoToqueCampanas -> Período activo (minuto %d): %s", 
                           minuto, 
                           periodoHoraEnPunto ? "Hora en punto" : "Media hora");
        }
    #endif
    
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
void ActualizaEstadoProteccionCampanadas(void) {
    
    bool nuevoEstadoProteccion = EsPeriodoToqueCampanas();                              // 1. Obtener nuevo estado (SIN efectos secundarios)
    if (lProteccionCampanadas != nuevoEstadoProteccion) {                               // 2. Solo procesar si hay cambio
        lProteccionCampanadasAnterior = lProteccionCampanadas;
        lProteccionCampanadas = nuevoEstadoProteccion;
        
        EjecutaSecuencia(EstadoProteccionCampanadas);                                   // 4. Notificar cambio por WebSocket (UNA SOLA VEZ)
        
        DBG_PROT_PRINTF("ActualizaEstadoProteccionCampanadas -> Cambio: %s -> %s",      // 6. Log del cambio
                       lProteccionCampanadasAnterior ? "ACTIVA" : "INACTIVA",
                       lProteccionCampanadas ? "ACTIVA" : "INACTIVA");
    }
    

}
    /**
     * @brief Comprueba si el minuto actual está dentro de un período protegido para el toque de secuencia de campanadas.
     * 
     * Esta función verifica si el minuto actual está dentro de los períodos protegidos definidos
     * por las constantes INICIO_VENTANA_HORA, FIN_VENTANA_HORA, INICIO_VENTANA_MEDIA y FIN_VENTANA_MEDIA.
     * Si el RTC no está sincronizado con NTP, se desactiva la protección de campanadas.
     * La función también gestiona el estado de la protección de campanadas y notifica a los clientes web
     * sobre cualquier cambio en este estado.
     * 
     * @return true si la hora actual está dentro de un período protegido, false en caso contrario.
     */
    /*
    bool EsPeriodoToqueCampanas (void) 
    {
        
        if (!RTC::isNtpSync())                                                              //Si no hay sincronizacion NTP, quita las protecciones
        {    
            DBG_PROT  ("EsPeriodoToqueCampanas -> RTC no sincronizado con NTP.");
            // Detecta cambio de estado y notifica si es necesario
            if (lProteccionCampanadas != false) {                                           //Si esta la proteccion de campanadas la deshabilitamos
                lProteccionCampanadasAnterior = lProteccionCampanadas;
                lProteccionCampanadas = false;
                EjecutaSecuencia(EstadoProteccionCampanadas);                               // Notifica el cambio de estado a cliente web
                Campanario.ClearProteccionCampanadas();                                     // Desactiva la protección de campanadas
                DBG_PROT("EsPeriodoToqueCampanas -> Protección desactivada (RTC no sincronizado)");
                
            }
            return false; // Si el RTC no está sincronizado, no se puede determinar el período
        }
        
        struct tm localTime;                                                                //Si hay sincronizacion
        if (getLocalTime(&localTime)) {                                                     // Obtiene la hora local                            
            int minuto = localTime.tm_min;                                                  //Obtenemos el minuto actual
            bool periodoHoraEnPunto = (minuto >= Config::Time::INICIO_VENTANA_HORA) || (minuto < Config::Time::FIN_VENTANA_HORA);                       // Comprueba si estamos ±5 minutos de la hora en punto (minuto 0)
            bool periodoMediaHora = (minuto >= Config::Time::INICIO_VENTANA_MEDIA) && (minuto < Config::Time::FIN_VENTANA_MEDIA);                         // Comprueba si estamos ±5 minutos de la media hora (minuto 30)
            bool nuevoEstadoProteccion = (periodoHoraEnPunto || periodoMediaHora);          //Si esta en algun periodo de proteccion
            if (lProteccionCampanadas != nuevoEstadoProteccion) {                           // Si el estado de protección ha cambiado     
                lProteccionCampanadasAnterior = lProteccionCampanadas;                      // Guarda el estado anterior de la protección de campanadas 
                lProteccionCampanadas = nuevoEstadoProteccion;                              // Actualiza el estado de la protección de campanadas
                if (lProteccionCampanadas) {                                                // Si estamos en un período de toque de campanas
                    Campanario.SetProteccionCampanadas();                                   // Activa la protección de campanadas
                    DBG_PROT_PRINTF("EsPeriodoToqueCampanas -> Activando protección de campanadas (minuto %d)\n", minuto);
                } else {                                                                    // Si no estamos en un período de toque de campanas
                    Campanario.ClearProteccionCampanadas();                                 // Desactiva la protección de campanadas
                    DBG_PROT_PRINTF("EsPeriodoToqueCampanas -> Desactivando protección de campanadas (minuto %d)\n", minuto);
                }
                EjecutaSecuencia(EstadoProteccionCampanadas);                               // Notifica el cambio de estado Al cliete web
                DBG_PROT_PRINTF("EsPeriodoToqueCampanas -> Cambio de protección: %s -> %s (minuto %d)\n", lProteccionCampanadasAnterior ? "ACTIVA" : "INACTIVA",lProteccionCampanadas ? "ACTIVA" : "INACTIVA",minuto);
            }
            #ifdef DEBUGPROTECCION
                if (nuevoEstadoProteccion) {
                    DBG_PROT_PRINTF("EsPeriodoToqueCampanas -> Período activo (minuto %d): %s\n", minuto, periodoHoraEnPunto ? "Hora en punto" : "Media hora");
                }
            #endif
            
            return nuevoEstadoProteccion;
        }
        DBG_PROT("EsPeriodoToqueCampanas -> Error obteniendo hora del RTC");
  
        if (lProteccionCampanadas != false) {                                                   // Si no se pudo obtener la hora, desactiva la protección de campanadas si estaba activa
            lProteccionCampanadasAnterior = lProteccionCampanadas;
            lProteccionCampanadas = false;
            EjecutaSecuencia(EstadoProteccionCampanadas); // Notifica el cambio de estado
            DBG_PROT("EsPeriodoToqueCampanas -> Protección desactivada (error obteniendo hora)");
        }
        return false; // Si no se pudo obtener la hora, devuelve false
    }    
*/
/**
 * @brief Actualiza el estado de protección de las campanadas y notifica cambios.
 *
 * Esta función compara el estado actual de protección de las campanadas con el anterior.
 * Si hay un cambio de estado, lo notifica a través de WebSocket y muestra mensajes de depuración.
 *
 * - Actualiza las variables de estado de protección.
 * - Muestra el estado actual mediante DBG_AUX.
 * - Si el estado ha cambiado, notifica el cambio y envía el estado actualizado por WebSocket.
 *
 * @note Utiliza la función EsPeriodoToqueCampanas() para determinar el estado de protección.
 */
/*
 void ActualizaEstadoProteccionCampanadas(void) 
 {
    lProteccionCampanadasAnterior = lProteccionCampanadas;
    lProteccionCampanadas = EsPeriodoToqueCampanas();
    
    DBG_AUX("Estado protección: " + String(lProteccionCampanadas ? "ACTIVA" : "INACTIVA"));
    
    // Si hay cambio de estado, notificar
    if (lProteccionCampanadas != lProteccionCampanadasAnterior) {
        DBG_AUX("Cambio estado protección: " + String(lProteccionCampanadasAnterior) + " -> " + String(lProteccionCampanadas));

        // Notificar a través de WebSocket si está disponible
        if (lProteccionCampanadas) {
            ws.textAll("PROTECTION:ON");
        } else {
            ws.textAll("PROTECTION:OFF");
        }
    }
}
*/
