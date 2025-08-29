#include "TimeManager.h"
#include "RTC.h"
#include "Auxiliar.h"
#include "Servidor.h"
#include "DNSServicio.h"
#include "Configuracion.h"


bool TimeManager::_lProteccionCampanadasAnterior = false;

/**
 * @brief Obtiene la hora local actual y la almacena en la estructura tm proporcionada.
 * 
 * @param timeinfo Referencia a una estructura tm donde se almacenará la información de la hora actual.
 * @return true si se pudo obtener la hora local correctamente, false en caso contrario.
 */
bool TimeManager::getCurrentTime(struct tm& timeinfo) 
{
    return getLocalTime(&timeinfo);
}

/**
 * @brief Determina si la hora actual corresponde al horario nocturno.
 *
 * Esta función obtiene la hora actual y verifica si está dentro del rango definido
 * como horario nocturno, según los valores configurados en Config::Time::NOCHE_INICIO_HORA
 * y Config::Time::NOCHE_FIN_HORA.
 *
 * @return true si la hora actual está en el horario nocturno, false en caso contrario
 *         o si no se pudo obtener la hora actual.
 */
bool TimeManager::EsHorarioNocturno() 
{
    struct tm timeinfo;
    if (!getCurrentTime(timeinfo)) return false;
    int hora = timeinfo.tm_hour;
    return (hora >= Config::Time::NOCHE_INICIO_HORA || hora < Config::Time::NOCHE_FIN_HORA);
}

/**
 * @brief Determina si un minuto dado se encuentra dentro de una ventana protegida alrededor de un minuto objetivo.
 *
 * Esta función verifica si el parámetro 'minuto' está dentro de un rango definido por 'minutoObjetivo' y un 'margen'.
 * El rango puede envolver el ciclo de 60 minutos (por ejemplo, de 58 a 2).
 *
 * @param minuto Minuto actual a evaluar (0-59).
 * @param minutoObjetivo Minuto central de la ventana protegida (0-59).
 * @param margen Margen de minutos alrededor del minuto objetivo.
 * @return true si 'minuto' está dentro de la ventana protegida, false en caso contrario.
 */
bool TimeManager::_EstaEnVentanaProtegida(int minuto, int minutoObjetivo, int margen) 
{
    int inferior = (minutoObjetivo - margen + 60) % 60;
    int superior = (minutoObjetivo + margen) % 60;
    if (inferior <= superior)
        return (minuto >= inferior && minuto <= superior);
    else
        return (minuto >= inferior || minuto <= superior);
}

/**
 * @brief Determina si el momento actual está dentro de un periodo de protección para el toque de campanas.
 *
 * Esta función verifica si el minuto actual se encuentra dentro de las ventanas protegidas
 * definidas para el inicio y la media campanada, utilizando un margen configurable.
 * Si no se puede obtener la hora actual, retorna el valor anterior de protección.
 *
 * @return true si el momento actual está dentro de alguna ventana de protección, false en caso contrario.
 */
bool TimeManager::EsPeriodoProteccionToqueCampanas() 
{
    struct tm timeinfo;
    if (!getCurrentTime(timeinfo)) return _lProteccionCampanadasAnterior;
    int minuto = timeinfo.tm_min;
    return _EstaEnVentanaProtegida(minuto, Config::Time::MINUTO_HORA_CAMPANADA,  Config::Time::MARGEN_PROTECCION_VENTANA) ||
           _EstaEnVentanaProtegida(minuto, Config::Time::MINUTO_MEDIA_CAMPANADA,  Config::Time::MARGEN_PROTECCION_VENTANA);
}



/**
 * @brief Actualiza el estado de protección de las campanadas.
 *
 * Esta función verifica si ha cambiado el periodo de protección de toque de campanas.
 * Si el estado ha cambiado, actualiza la variable interna de protección y ejecuta la secuencia
 * correspondiente. Además, activa o desactiva la protección en el objeto Campanario según corresponda.
 * Si la depuración está habilitada, imprime mensajes en el puerto serie indicando el cambio de estado.
 */
void TimeManager::ActualizaEstadoProteccionCampanadas() 
{
    bool lProteccionCampanadasActual = EsPeriodoProteccionToqueCampanas();
    if (lProteccionCampanadasActual != _lProteccionCampanadasAnterior) {
        _lProteccionCampanadasAnterior = lProteccionCampanadasActual;
        EjecutaSecuencia(Config::States::PROTECCION_CAMPANADAS);
        
        if (lProteccionCampanadasActual) {
            Campanario.SetProteccionCampanadas();
            if constexpr (Config::Debug::PROTECTION_DEBUG) {
                Serial.println("TimeManager: Protección activada");
            }
        } else {
            Campanario.ClearProteccionCampanadas();
            if constexpr (Config::Debug::PROTECTION_DEBUG) {
                Serial.println("TimeManager: Protección desactivada");
            }
        }
    }
}
/**
 * @brief Verifica y gestiona los eventos de campanas cada media hora.
 *
 * Esta función comprueba si ha cambiado el minuto actual y, si es múltiplo de 30,
 * realiza las siguientes acciones:
 * - Redirige la interfaz a la página de campanas.
 * - Si es la hora en punto (minuto == 0), y no es horario nocturno, toca la campana de la hora.
 * - Si la hora coincide con la configurada para sincronización NTP y hay conexión a Internet, realiza la sincronización automática.
 * - Si es media hora (minuto == 30), y no es horario nocturno, toca la campana de la media hora.
 * - Si hay conexión a Internet, actualiza el DNS dinámico.
 *
 * La función no realiza ninguna acción si hay una secuencia de campanas activa o si no se puede obtener la hora actual.
 */


/**
 * @brief Sincroniza automáticamente la hora utilizando NTP.
 *
 * Inicializa el módulo RTC para comenzar la sincronización con un servidor NTP.
 * Esta función debe ser llamada para asegurar que el reloj del sistema esté actualizado.
 */
/*
void TimeManager::autoSyncNTP() 
{
    RTC::begin();
}
*/
/**
 * @brief Restablece el estado interno del gestor de tiempo.
 *
 * Establece el valor de lastMinute_ a -1 y lastProtectionState_ a false,
 * reiniciando así el seguimiento del minuto y el estado de protección.
 */
void TimeManager::reset() 
{
    _lProteccionCampanadasAnterior = false;
}
