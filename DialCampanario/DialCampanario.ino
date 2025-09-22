#include "Auxiliar.h"



    void setup() {
        delay (3000);
        Serial.begin(9600);
        
        auto cfg = M5.config();
        M5Dial.begin(cfg, true, false);
        
        InicioDisplay();                                                                        // Inicializa el display y crea los sprites de los menús
        ClearPantalla();                                                                        // Limpia la pantalla
        InicializarVariablesGlobales();                                                         // Inicializa las variables globales del sistema
        I2C_Inicializar();                                                                      // Inicializa la comunicación I2C

        menu.hayQueMostrarCambio = true;                                                        // Fuerza la asignación del menú inicial
        MensajeInicio();                                                                        // Muestra el mensaje de inicio
        delay(Config::Timing::DELAY_INICIO_MS);                                                 // Espera para que se inicie el modulo del campanario
        //MenuState.indiceMenuActivo = menu0;                                                   // Establece el menú inicial 
        DBG("Setup completado exitosamente");
    }

    void loop() {
        
        yield();                                                                                // Alimenta el watchdog
        M5Dial.update();                                                                        // Actualizar el estado del M5Dial

        if (menu.hayQueMostrarCambio) {                                                         // Cambia el menú según el modo
            AsignaMenu();                                                                       // Asigna el menú correspondiente
            menu.hayQueMostrarCambio = false;
            display.contadorCiclosSleep = 0;                                                    // Reiniciar el contador de ciclos de sleep del display
        }

        if (M5Dial.BtnA.wasPressed()) { 
            ManejarBotonA();                                                                    // Manejar el botón A
        }

        if   (!display.estaEnModoSleep)
        {
            ManejarEncoder();                                                                   // Manejar el encoder
        }

        if (campanarioEstado.estadoActual != Config::States::I2CState::SET_TEMPORIZADOR) {
            if (millis() - nMilisegundoTemporal > Config::Timing::I2C_REQUEST_INTERVAL_MS) { // Cada 500 ms
                nMilisegundoTemporal = millis();
                SolicitarEstadoFechaHora(true);                                                 // Solicitar la hora al esclavo I2C
                ActualizarDisplaySleep();                                                       // Actualizar el estado del display
            }
        }

        if (campanarioEstado.estadoActual != Config::States::I2CState::SET_TEMPORIZADOR) {      // Si no estamos en modo configuración de temporizador
            if (!lComandoRecienEnviado) {                                                       //Si no se ha enviado un comando recientemente
                // Comportamiento normal: peticiones cada 500ms
                if (millis() - nMilisegundoTemporal > Config::Timing::I2C_REQUEST_INTERVAL_MS) {    // Cada 500 ms
                    nMilisegundoTemporal = millis();
                    SolicitarEstadoFechaHora(true);                                             // Solicitar la hora al esclavo I2C
                    ActualizarDisplaySleep();                                                   // Actualizar el estado del display
                }
            } else {                                                                            //Si se ha enviado un comando recientemente
                // Comando recién enviado: esperar tiempo de procesamiento
                if (millis() - nMilisegundoTemporal > Config::Timing::I2C_COMMAND_PROCESSING_DELAY_MS) {    // Esperar 200 ms
                    DBG("loop->Tiempo de procesamiento completado, reanudando peticiones normales");
                    lComandoRecienEnviado = false;                                              // Permitir peticiones normales
                    nMilisegundoTemporal = millis();                                            // Resetear timer para próxima petición
                }
            }
        }        
        delay(10);                                                                              // Pequeña pausa para evitar sobrecarga
}



