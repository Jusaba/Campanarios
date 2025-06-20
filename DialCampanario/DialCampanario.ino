#include "Auxiliar.h"


#define DEBUG

    void setup() {
        delay (3000);
        Serial.begin(9600);
        
        #ifdef DEBUG
            Serial.println("Inicio");
        #endif

        InicioDisplay();                            // Inicializar la pantalla del M5Dial
        ClearPantalla();                            // Limpiar la pantalla
        
        Wire.begin();                               // Iniciar el bus I2C como maestro
        Wire.setClock(100000);

        lCambioEstado = true;                       // Indicar que se debe mostrar el menu por un cambio de estado
        MensajeInicio();                            // Mostrar el mensaje de inicio 
        menuActual = menu0;                        // Establecer el menú actual
    }

    void loop() {
 
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
                        Serial.println("Estado Inicio");
                        break;    
                    case EstadoDifuntos:
                        Serial.println("Estado Difuntos");
                        EnviarEstado(EstadoDifuntos);  // Enviar el estado de difuntos al esclavo I2C
                        nEstado = 0;  // Reiniciar el estado al primer elemento del menú reducido
                        nEstadoAnterior = EstadoDifuntos;
                        break;
                    case EstadoMisa:
                        Serial.println("Estado Misa");
                        EnviarEstado(EstadoMisa);  // Enviar el estado de misa al esclavo I2C
                        nEstado = 0;  // Reiniciar el estado al primer elemento del menú reducido
                        nEstadoAnterior = EstadoMisa;
                        break;
                    case EstadoStop:
                        Serial.println("Estado Stop");
                        EnviarEstado(EstadoStop);  // Enviar el estado de stop al esclavo I2C
                        nEstado = nEstadoAnterior - 1;  // Volver al estado anterior
                        break;
                    case EstadoCalefaccionOn:
                        Serial.println("Estado Calefaccion On");
                        EnviarEstado(EstadoCalefaccionOn);  // Enviar el estado de calefacción encendida al esclavo I2C
                        nEstado = 0;  // Reiniciar el estado al primer elemento del menú reducido
                        nEstadoAnterior = EstadoCalefaccionOn;
                        break;
                    case EstadoCalefaccionOff:
                        Serial.println("Estado Calefaccion Off");
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

}



