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
            switch(nEstadoMenu) {
                case 0: menuActual = menu0; nMenuItems = ItemsMenu0; break;
                case 1: menuActual = menu1; nMenuItems = ItemsMenu1; break;
                case 2: menuActual = menu2; nMenuItems = ItemsMenu2; break;
                case 3: menuActual = menu3; nMenuItems = ItemsMenu3; break;
            }
            MostrarOpcion(menuActual[nEstado]);
            lCambioEstado = false;
            nContadorCiclosDisplaySleep = 0;            // Reiniciar el contador de ciclos de sleep del display
            if (lSleep) {
               lSleep = false;                           // Asegurarse de que el brillo esté al máximo
               if ( M5.Display.getBrightness() < 100) {
                    BrilloFull();                  // Asegurarse de que el brillo esté al máximo
                } 
              
            }
        }
        
        // Leer el boton 
        if (M5Dial.BtnA.wasPressed()) { 
            ManejarBotonA(); // Manejar el botón A
        }

    if   (!lSleep)
    { 
        // Leer el encoder
        ManejarEncoder(); // Manejar el encoder
 
    } 
    if (millis() - nMilisegundoTemporal > nmsGetEstadoCampanario) { // Cada 500 ms
        nMilisegundoTemporal = millis(); 
        SolicitarEstadoHora(); // Solicitar la hora al esclavo I2C
        ActualizarDisplaySleep(); // Actualizar el estado del display

    }    
    // Pequeño delay para evitar saturar el procesador
    delay(10);
}



