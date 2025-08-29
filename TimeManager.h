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
    
    //static void autoSyncNTP();
    static void reset();

private:
    static bool _lProteccionCampanadasAnterior;
    static bool _EstaEnVentanaProtegida(int minuto, int minutoObjetivo, int margen) ;
};

#endif