#ifndef ACCIONES_H
    #define ACCIONES_H
    #include <Arduino.h>  
    #include <stdint.h>
    // Funciones específicas del campanario para usar con Alarmas
    void accionSecuencia(uint16_t seqId);
    void accionTocaHora(void);
    void accionTocaMedia(void);

    void SincronizaNTP( void );
    void NTPSync ( void ); 


#endif
