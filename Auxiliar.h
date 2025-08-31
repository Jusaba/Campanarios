// Auxiliar.h - SOLO interfaz
#ifndef AUXILIAR_H
#define AUXILIAR_H

#include "RTC.h"
#include <Wire.h>
#include "Servidor.h"
#include "ConexionWifi.h"  // Para ConfigWiFi
#include "Configuracion.h"
#include "Debug.h"

#define DEBUGAUXILIAR
#define DEBUGPROTECCION

// Variables globales (SOLO extern - declaraciones)
extern CAMPANARIO Campanario;
extern struct tm timeinfo;
extern int ultimoMinuto;
extern int nCampanaTocada;
extern int ultimaHora;
//extern volatile uint8_t secuenciaI2C;
//extern uint8_t requestI2C;
//extern uint8_t ParametroI2C;
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
void TestInternet(void);                                // Prototipo de la función para probar la conexión a Internet

//bool EsHorarioNocturno(void);                           // Prototipo de la función para comprobar si es horario nocturno
bool EsPeriodoToqueCampanas(void);                      // Prototipo de la función para comprobar si estamos en el período de toque de campanas
void ActualizaEstadoProteccionCampanadas(void);         // Prototipo de la función para actualizar el estado de protección de campanadas
// Funciones auxiliares de protección
bool EstaEnVentanaProtegida(int minuto, int minutoObjetivo, int margen);  // ← Era private en TimeManager

#endif