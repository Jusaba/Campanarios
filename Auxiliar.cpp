// Auxiliar.cpp
#include "Auxiliar.h"
#include "ConexionWifi.h"
#include "DNSServicio.h"  // Para ActualizaDNS



// Definiciones únicas de variables globales
CAMPANARIO Campanario;
struct tm timeinfo;
//int ultimoMinuto = -1;
int nCampanaTocada = 0;
//int ultimaHora = -1;
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
        EjecutaSecuencia(nSecuencia, 0);
    }
    void EjecutaSecuencia(int nSecuencia, int nParametro) {
        
        DBG_AUX_PRINTF("EjecutaSecuencia -> Secuencia: %d, Parámetro: %d", nSecuencia, parametro);

        switch (nSecuencia) {
            case Config::States::DIFUNTOS:
                Campanario.TocaDifuntos();
                ws.textAll("REDIRECT:/Campanas.html");      // ✅ COMO ESTABA ORIGINALMENTE
                DBG_AUX("EjecutaSecuencia -> Iniciando secuencia de difuntos");
                break;

            case Config::States::MISA:
                Campanario.TocaMisa();
                ws.textAll("REDIRECT:/Campanas.html");      // ✅ COMO ESTABA ORIGINALMENTE
                DBG_AUX("EjecutaSecuencia -> Iniciando secuencia de misa");
                break;

            case Config::States::STOP:
                Campanario.ParaSecuencia();
                ws.textAll("REDIRECT:/index.html");         // ✅ COMO ESTABA ORIGINALMENTE
                DBG_AUX("EjecutaSecuencia -> Parando todas las secuencias");
                break;

            case Config::States::CALEFACCION_ON:
                  // WebSocket con temporizador global
                    Campanario.EnciendeCalefaccion(nTemporizacionCalefaccion);
                    ws.textAll("CALEFACCION:ON");            // ✅ COMO ESTABA ORIGINALMENTE
                    DBG_AUX_PRINTF("EjecutaSecuencia -> Encendiendo calefacción (%d min desde WebSocket)", nTemporizacionCalefaccion);
                    break;

            case Config::States::CALEFACCION_OFF:
                Campanario.ApagaCalefaccion();
                ws.textAll("CALEFACCION:OFF");               // ✅ COMO ESTABA ORIGINALMENTE
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
                // ✅ MANTENER lógica I2C temporizador (esta SÍ estaba):
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
/*
    void EjecutaSecuencia (int nSecuencia, int nParametro) 
    {
        DBG_AUX_PRINTF("EjecutaSecuencia->Secuencia: %d\n", nSecuencia);
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
                nTemporizacionCalefaccion = nParametro; // Asigna el valor del parámetro recibido al temporizador de calefacción
                Campanario.EnciendeCalefaccion(nTemporizacionCalefaccion); // Llama a la función para fijar el temporizador de calefacción
                ws.textAll("CALEFACCION:ON:" + String(nTemporizacionCalefaccion));
                DBG_AUX_PRINTF("EjecutaSecuencia -> Temporizador de calefacción fijado a %d minutos.\n", nTemporizacionCalefaccion);
                break;
            default:
                DBG_AUX("EjecutaSecuencia -> Secuencia no reconocida.");
                break;
        }
    }
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
        
        EjecutaSecuencia(Config::States::PROTECCION_CAMPANADAS);                                   // 4. Notificar cambio por WebSocket (UNA SOLA VEZ)
        
        DBG_PROT_PRINTF("ActualizaEstadoProteccionCampanadas -> Cambio: %s -> %s",      // 6. Log del cambio
                       lProteccionCampanadasAnterior ? "ACTIVA" : "INACTIVA",
                       lProteccionCampanadas ? "ACTIVA" : "INACTIVA");
    }
    

}
