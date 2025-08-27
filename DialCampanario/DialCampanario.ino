#include "Auxiliar.h"



    void setup() {
        delay (3000);
        Serial.begin(9600);
        
        auto cfg = M5.config();
        M5Dial.begin(cfg, true, false);
        
        InicioDisplay();
        ClearPantalla();
        
        Wire.begin();
        Wire.setClock(100000);
        //Wire.setTimeout(1000);

        lCambioEstado = true;
        MensajeInicio();
        delay(5000);
        menuActual = menu0;
        
        #ifdef DEBUG
            Serial.println("Setup completado exitosamente");
        #endif
    }

    void loop() {
        // Alimentar el watchdog para evitar reinicios
        yield();
        
        M5Dial.update();                                // Actualizar el estado del M5Dial
        
        if (lCambioEstado) {                            // Cambia el menú según el modo
            AsignaMenu();                               // Asigna el menú correspondiente
            lCambioEstado = false;
            nContadorCiclosDisplaySleep = 0;            // Reiniciar el contador de ciclos de sleep del display
        }
        
        // Leer el boton 
        if (M5Dial.BtnA.wasPressed()) { 
            ManejarBotonA(); // Manejar el botón A
        }

        if   (!lSleep)
        { 
            // Leer el encoder
            ManejarEncoder(); // Manejar el encoder
/*
Serial.print("Estado actualizado a");
Serial.print(nEstado);
Serial.print("  ");
Serial.print(nEstadoAnterior);
Serial.print("  "); 
Serial.print(nEstadoActual);
Serial.println("  ");
*/
        }
        if (nEstado != EstadoSetTemporizador) {
            if (millis() - nMilisegundoTemporal > nmsGetEstadoCampanario) { // Cada 500 ms
                nMilisegundoTemporal = millis();
                //SolicitarEstadoFechaHora(); // Solicitar la hora al esclavo I2C
                SolicitarEstadoFechaHoraoTemporizacion(); // Solicitar el estado del campanario al esclavo I2C
                ActualizarDisplaySleep(); // Actualizar el estado del display
            }
        }
    
        // Pequeño delay para evitar saturar el procesador
        delay(10);
}



