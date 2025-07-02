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
            #ifdef DEBUG
                Serial.println("Botón A presionado");            
            #endif
            if (lSleep) {
                lSleep = false;  // Desactivar el modo sleep
                nContadorCiclosDisplaySleep = 0;        // Reiniciar el contador de ciclos de sleep del display
               if ( M5.Display.getBrightness() < 100) {
                    BrilloFull();                  // Asegurarse de que el brillo esté al máximo
                } 
                 MostrarOpcion(menuActual[nEstado]);
            }else{
                switch (menuActual[nEstado])
                {
                    #ifdef DEBUGBOTON
                        Serial.printf("Estado Actual: %d\n", menuActual[nEstado]);
                    #endif
                    case EstadoInicio:
                        #ifdef DEBUGBOTON
                            Serial.println("Estado Inicio");
                        #endif
                        break;    
                    case EstadoDifuntos:
                        #ifdef DEBUGBOTON
                            Serial.println("Estado Difuntos");
                        #endif  
                        EnviarEstado(EstadoDifuntos);  // Enviar el estado de difuntos al esclavo I2C
                        nEstado = 0;  // Reiniciar el estado al primer elemento del menú reducido
                        nEstadoAnterior = EstadoDifuntos;
                        break;
                    case EstadoMisa:
                        #ifdef DEBUGBOTON
                            Serial.println("Estado Misa");
                        #endif
                        EnviarEstado(EstadoMisa);  // Enviar el estado de misa al esclavo I2C
                        nEstado = 0;  // Reiniciar el estado al primer elemento del menú reducido
                        nEstadoAnterior = EstadoMisa;
                        break;
                    case EstadoStop:
                        #ifdef DEBUGBOTON
                            Serial.println("Estado Stop");
                        #endif
                        EnviarEstado(EstadoStop);  // Enviar el estado de stop al esclavo I2C
                        nEstado = nEstadoAnterior - 1;  // Volver al estado anterior
                        break;
                    case EstadoCalefaccionOn:
                        #ifdef DEBUGBOTON
                            Serial.println("Estado Calefaccion On");
                        #endif
                        EnviarEstado(EstadoCalefaccionOn);  // Enviar el estado de calefacción encendida al esclavo I2C
                        nEstado = 0;  // Reiniciar el estado al primer elemento del menú reducido
                        nEstadoAnterior = EstadoCalefaccionOn;
                        break;
                    case EstadoCalefaccionOff:
                        #ifdef DEBUGBOTON
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
        }
    if   (!lSleep)
    { 
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
    } 
    if (millis() - nMilisegundoTemporal > nmsGetEstadoCampanario) { // Cada 500 ms
        nMilisegundoTemporal = millis(); 
        SolicitarEstadoHora(); // Solicitar la hora al esclavo I2C
        if (lBrillo && !lSleep) { // Si el brillo está activo y el display no está en modo sleep
            //SolicitarEstadoCampanario();
            nContadorCiclosDisplaySleep++; // Incrementar el contador de ciclos de sleep del display
            if ((nContadorCiclosDisplaySleep >= nCiclosDisplaySleep))
            {
                BajaBrillo(); // Poner el display en modo sleep
                nContadorCiclosDisplaySleep = 0; // Reiniciar el contador de ciclos de sleep del display
                lSleep = true; // Indicar que el display está en modo sleep
            }
        }else{
            if (!lBrillo)
            {
                MensajeHora(campanarioEstado.nHora, campanarioEstado.nMinutos, campanarioEstado.nSegundos); // Mostrar la hora en pantalla
                SubeBrillo(40); // Asegurarse de que el brillo esté al máximo
            }else{
//                BorraHora(); // Borrar la hora de la pantalla
                EscribeHora(campanarioEstado.nHora, campanarioEstado.nMinutos, campanarioEstado.nSegundos); // Escribir la hora en pantalla
            }
            Serial.println("Display Sleep, se preenta la hora ");
        }
    }    
    // Pequeño delay para evitar saturar el procesador
    delay(10);
}



