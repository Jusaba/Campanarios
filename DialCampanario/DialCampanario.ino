#include "Auxiliar.h"


#define DEBUG

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
        
        M5Dial.update();                            // Actualizar el estado del M5Dial

  

        // Cambia el menú según el modo
    if (lCambioEstado) {
        switch(nEstadoMenu) {
            case 0: menuActual = menu0; nMenuItems = ItemsMenu0; break;
            case 1: menuActual = menu1; nMenuItems = ItemsMenu1; break;
            case 2: menuActual = menu2; nMenuItems = ItemsMenu2; break;
            case 3: menuActual = menu3; nMenuItems = ItemsMenu3; break;
        }
        MostrarMenu(menuActual[nEstado]);
        lCambioEstado = false;
    }
        
        // Leer el encoder
        if (M5Dial.BtnA.wasPressed()) { 
                #ifdef DEBUG
                    Serial.println("Botón A presionado");            
                #endif
                switch (menuActual[nEstado])
                {
                    #ifdef DEBUG
                        Serial.printf("Estado Actual: %d\n", menuActual[nEstado]);
                    #endif
                    case EstadoInicio:
                        #ifdef DEBUG
                            Serial.println("Estado Inicio");
                        #endif
                        break;    
                    case EstadoDifuntos:
                        #ifdef DEBUG
                            Serial.println("Estado Difuntos");
                        #endif  
                        EnviarEstado(EstadoDifuntos);  // Enviar el estado de difuntos al esclavo I2C
                        nEstado = 0;  // Reiniciar el estado al primer elemento del menú reducido
                        nEstadoAnterior = EstadoDifuntos;
                        break;
                    case EstadoMisa:
                        #ifdef DEBUG
                            Serial.println("Estado Misa");
                        #endif
                        EnviarEstado(EstadoMisa);  // Enviar el estado de misa al esclavo I2C
                        nEstado = 0;  // Reiniciar el estado al primer elemento del menú reducido
                        nEstadoAnterior = EstadoMisa;
                        break;
                    case EstadoStop:
                        #ifdef DEBUG
                            Serial.println("Estado Stop");
                        #endif
                        EnviarEstado(EstadoStop);  // Enviar el estado de stop al esclavo I2C
                        nEstado = nEstadoAnterior - 1;  // Volver al estado anterior
                        break;
                    case EstadoCalefaccionOn:
                        #ifdef DEBUG
                            Serial.println("Estado Calefaccion On");
                        #endif
                        EnviarEstado(EstadoCalefaccionOn);  // Enviar el estado de calefacción encendida al esclavo I2C
                        nEstado = 0;  // Reiniciar el estado al primer elemento del menú reducido
                        nEstadoAnterior = EstadoCalefaccionOn;
                        break;
                    case EstadoCalefaccionOff:
                        #ifdef DEBUG
                            Serial.println("Estado Calefaccion Off");
                        #endif
                        EnviarEstado(EstadoCalefaccionOff);  // Enviar el estado de calefacción apagada al esclavo I2C
                        nEstado = 0;  // Reiniciar el estado al primer elemento del menú reducido
                        nEstadoAnterior = EstadoCalefaccionOff;
                        break;
                    default:
                        break;
               }    
               lCambioEstado = true;  // Indicar que ha habido un cambio de estado
        }
    
    nPosicionActual = M5Dial.Encoder.read();
    if (nPosicionActual != nPosicionAneterior) {
        M5Dial.Speaker.tone(8000, 20);  // Emitir un pitido
        if (nPosicionActual > nPosicionAneterior) {
             #ifdef DEBUG
                Serial.printf("Posicion Actual: %d, Estado Actual: %d\n", nPosicionAneterior, nEstado);
            #endif
            if (nEstado < nMenuItems - 1) {
                nEstado++;
            } else {
                nEstado = nMenuItems - 1; // Limitar al último estado
            }
        } else {
            if (nEstado > 0) {
                nEstado--;
            } else {
                 nEstado = 0;
            }
         }
        #ifdef DEBUG
            Serial.printf("Nueva Posicion Actual: %d, Nuevo0 Estado Actual: %d\n", nPosicionActual, nEstado);
        #endif
        lCambioEstado = true;  // Indicar que ha habido un cambio de estado
        nPosicionAneterior = nPosicionActual;        
    }
    if (millis() - nMilisegundoTemporal > nmsGetEstadoCampanario) { // Cada 500 ms
        nMilisegundoTemporal = millis();    
        SolicitarEstadoCampanario();
    }
    
    // Pequeño delay para evitar saturar el procesador
    delay(10);
}



