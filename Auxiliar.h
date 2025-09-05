// Auxiliar.h - SOLO interfaz
#ifndef AUXILIAR_H
#define AUXILIAR_H

#include "RTC.h"
//#include <Wire.h>
#include "Servidor.h"
#include "ConexionWifi.h"  // Para ConfigWiFi
#include "Configuracion.h"
#include "Debug.h"
#include "Campanario.h"   // Para CAMPANARIO
#include <time.h>

// Variables globales 

extern CAMPANARIO Campanario;                           //Campanario

extern struct tm timeinfo;                  
//extern int ultimoMinuto;
//extern int ultimaHora;
extern int nCampanaTocada;
extern bool lConexionInternet;
extern bool lProteccionCampanadas;
extern bool lProteccionCampanadasAnterior;
extern unsigned long ultimoCheckInternet;
extern const unsigned long intervaloCheckInternet;
extern int nTemporizacionCalefaccion;
extern double nSegundosTemporizacion;
extern ConfigWiFi configWiFi;

// SOLO prototipos (sin implementaciones)

void TestCampanadas(void);                              // Prototipo de la función para probar las campanadas
void EjecutaSecuencia(int nSecuencia);                  // Prototipo de la función para ejecutar una secuencia
void EjecutaSecuencia(int nSecuencia, int nParametro);  // Prototipo de la función para ejecutar una secuencia con parámetro
void TestInternet(void);                                // Prototipo de la función para probar la conexión a Internet

bool EsPeriodoToqueCampanas(void);                      // Prototipo de la función para comprobar si estamos en el período de toque de campanas
void ActualizaEstadoProteccionCampanadas(void);         // Prototipo de la función para actualizar el estado de protección de campanadas


#endif