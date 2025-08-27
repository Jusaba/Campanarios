#ifndef AUXILIAR_H
#define AUXILIAR_H

#include <Arduino.h>
#include <Wire.h>
#include "RTC.h"
#include "Servidor.h"
#include "ConexionWifi.h"   // define ConfigWiFi
#include "Campanario.h"
#include "DNSServicio.h"

// Defines (igual que antes)
#define PinCampana1         26
#define PinCampana2         25
#define CalefaccionPin      33
#define PinRele4            32
#define PinConfiguracion    19

#define I2C_SLAVE_ADDR      0x12
#define SDA_PIN             21
#define SCL_PIN             22

#define EstadoDifuntos              1
#define EstadoMisa                  2
#define EstadoStop                  3
#define EstadoCalefaccionOn         4
#define EstadoCalefaccionOff        5
#define EstadoCampanario            6
#define EstadoHora                  7
#define EstadoSinInternet           8
#define EstadoProteccionCampanadas  9
#define EstadoFechaHora             10
#define EstadoSetTemporizador       11
#define EstadoFin                   12
#define EstadoFechaHoraoTemporizacion 13
#define EstadoTemporizacion         14

#define BitEstadoSinInternet        0x20
#define BitEstadoProteccionCampanadas 0x40

#define InicioHorarioNocturno 24
#define FinHorarioNocturno     7

// Variables globales (SOLO declaraciones extern)
extern ConfigWiFi configWiFi;
extern CAMPANARIO Campanario;
extern struct tm timeinfo;

extern int ultimoMinuto;
extern int nCampanaTocada;
extern int ultimaHora;

extern volatile uint8_t secuenciaI2C;
extern uint8_t requestI2C;
extern uint8_t ParametroI2C;
extern bool lConexionInternet;
extern bool lProteccionCampanadas;
extern bool lProteccionCampanadasAnterior;

extern unsigned long ultimoCheckInternet;
extern const unsigned long intervaloCheckInternet;
extern int nTemporizacionCalefaccion;
extern double nSegundosTemporizacion;

// Prototipos (solo declaraciones)
void ChekearCuartos(void);
void TestCampanadas(void);

void recibirSecuencia(int numBytes);
void enviarRequest(void);
void enviarHoraI2C(void);
void enviarFechaHoraI2C(void);
void enviarEstadoI2C(void);
void enviarEstadoTemporizacionI2C(void);
void enviarFechaoTemporizacionI2C(void);
void EjecutaSecuencia(int nSecuencia);

void TestInternet(void);
void SincronizaNTP(void);

bool EsHorarioNocturno(void);
bool EsPeriodoToqueCampanas(void);
void TocaHora(void);
void TocaMediaHora(void);

#endif