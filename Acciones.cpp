// Acciones.cpp - Añadir estas implementaciones:
#include "Acciones.h"
#include "Auxiliar.h"     // Para EjecutaSecuencia
#include "Campanario.h"   // Para Campanario
#include "Servidor.h"     // Para ws
#include "DNSServicio.h"  // Para ActualizaDNS
#include "ConexionWifi.h" // Para configWiFi
#include "Configuracion.h"// Para los parámetros de configuración


/**
 * @brief Ejecuta una secuencia específica identificada por su ID.
 * 
 * Esta función llama a EjecutaSecuencia pasando el identificador de la secuencia
 * para iniciar la ejecución correspondiente.
 * 
 * @param seqId Identificador de la secuencia a ejecutar.
 */
void accionSecuencia(uint16_t seqId) 
{
    EjecutaSecuencia(seqId);
}

void accionTocaHora(void) {
    // Obtener tiempo actual
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;
    
    // Verificar horario nocturno
    int hora = timeinfo.tm_hour;
    bool esNocturno = (hora >= InicioHorarioNocturno || 
                       hora < FinHorarioNocturno);
    
    if (!esNocturno) {
        Campanario.TocaHoraSinCuartos(hora);
        
        ws.textAll("REDIRECT:/Campanas.html");
        
        // Actualizar DNS si hay internet
        if (hayInternet()) {
            ActualizaDNS(configWiFi.dominio);
        }
    }
}

void accionTocaMedia(void) {
    // Obtener tiempo actual
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;
    
    // Verificar horario nocturno
    int hora = timeinfo.tm_hour;
    bool esNocturno = (hora >= InicioHorarioNocturno || 
                       hora < FinHorarioNocturno);
    
    if (!esNocturno) {
        Campanario.TocaMediaHora();
        
        
        ws.textAll("REDIRECT:/Campanas.html");
        
        // Actualizar DNS si hay internet
        if (hayInternet()) {
            ActualizaDNS(configWiFi.dominio);
        }
    }
}


void SincronizaNTP(void) {
    if (hayInternet()) {
        RTC::begin();  // ← Reutiliza tu lógica existente
        
        #ifdef DebugAcciones
        Serial.println("SincronizaNTP -> Re-sincronización completada");
        #endif
    }
}
