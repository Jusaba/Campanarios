#ifndef I2CSERVICIO_H
	#define I2CSERVICIO_H

    #include <Arduino.h>
    #include <Wire.h>
    #include <stdint.h>

    #define DEBUGI2C
    #define DEBUGAUXILIAR
    #define DEBUGI2CREQUEST
    #define DEBUGI2CSECUENCIA


    // Variables I2C globales (extern para usar desde otros módulos)
    extern volatile uint8_t secuenciaI2C;
    extern uint8_t requestI2C;
    extern uint8_t ParametroI2C;

    // Funciones de inicialización I2C
    void initI2C();

    // Funciones de comunicación I2C
    void recibirSecuencia(int numBytes);
    void enviarRequest();

    // Funciones de envío específicas I2C
    void enviarEstadoI2C();
    void enviarHoraI2C();
    void enviarFechaHoraI2C();
    void enviarEstadoTemporizacionI2C();
    void enviarFechaoTemporizacionI2C();

#endif