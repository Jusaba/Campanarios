#include "Acciones.h"
//#include "Auxiliar.h"
//#include "RTC.h"
//#include "Servidor.h"


void NTPSync ( void ) {
    SincronizaNTP();
    #ifdef DebugAcciones
        Serial.println("Acciones->NTPSync->[NTP] Sincronización NTP iniciada");
    #endif
}