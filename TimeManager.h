#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <time.h>
#include "Configuracion.h"

class TimeManager {
public:
    static bool getCurrentTime(struct tm& timeinfo);
    static bool EsHorarioNocturno();
    static bool EsPeriodoProteccionToqueCampanas();
    static void ActualizaEstadoProteccionCampanadas();
    static void checkCuartos();
    static void autoSyncNTP();
    static void reset();

private:
    static int _lUltimoMinuto;
    static bool _lProteccionCampanadasAnterior;
    static bool _EstaEnVentanaProtegida(int minuto, int minutoObjetivo, int margen) ;
};

#endif