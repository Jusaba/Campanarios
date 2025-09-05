// I2CServicio.cpp - Implementación del servicio I2C
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
 * @brief Inicializa la comunicación I2C como esclavo
 */
void initI2C() {
    Wire.begin(Config::I2C::SLAVE_ADDR);  // Dirección desde Configuracion.h
    Wire.onReceive(recibirSecuencia);
    Wire.onRequest(enviarRequest);
    DBG_I2C("I2CServicio -> Inicializado como esclavo en dirección 0x" + String(Config::I2C::SLAVE_ADDR, HEX));
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
 * En modo depuración (DEBUGI2CREQUEST, DEBUGI2CSECUENCIA), se imprime por el puerto serie la información recibida.
 * 
 * @param numBytes Número de bytes esperados en la secuencia recibida (1 o 2).
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
 * @brief Procesa una solicitud recibida por I2C y responde según el tipo de petición.
 *
 * Esta función verifica el valor de la variable 'requestI2C' para determinar el tipo de solicitud recibida.
 * Dependiendo del caso, envía el estado del campanario o la hora actual a través de I2C.
 * Después de procesar la solicitud, reinicia 'requestI2C' para esperar una nueva petición.
 *
 * @note Si está definido DEBUGAUXILIAR, imprime por el puerto serie información sobre la solicitud recibida.
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
 * @brief Envía el estado actual del campanario a través de comunicación I2C
 */
void enviarEstadoI2C() {
    int nEstadoCampanario = Campanario.GetEstadoCampanario();
    Wire.write(nEstadoCampanario);
    DBG_I2C_REQ("I2CServicio -> Estado enviado por I2C: " + String(nEstadoCampanario));
}

/**
 * @brief Envía la hora actual y el estado del campanario a través del bus I2C.
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
 * @brief Envía la fecha y hora completa junto con el estado del campanario a través del bus I2C.
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
 * @brief Envía el estado de temporización del campanario a través del bus I2C.
 * * Convierte la temporización en segundos a horas, minutos y segundos antes de enviarla.
 *  Esta opcion se utiliza cuando la calefaccion está en marcha para informar en el estado 
 *  de reposo del Maestro I2C del tiempo que queda de calefaccion encendida
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
 * @brief Decide si enviar fecha/hora normal o temporización según el estado de calefacción
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