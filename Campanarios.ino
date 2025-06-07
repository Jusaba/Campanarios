#include "DNSServicio.h"
#include "Servidor.h"
#include "Campana.h"
#include "Campanario.h"
#include "RTC.h"
#include "ConexionWifi.h"
#include <Wire.h>
#include "Auxiliar.h"



#define DEBUG

#define PinCampana1 26                          // Definición del pin para la campana 1
#define PinCampana2 27                          // Definición del pin para la campana 2



void setup() {
  Serial.begin(9600);

  Wire.begin(I2C_SLAVE_ADDR, SDA_PIN, SCL_PIN);
  Wire.setClock(100000);
  Wire.onReceive(recibirSecuencia);
  Wire.onRequest(enviarEstadoI2C);

  CAMPANA* campana1 = new CAMPANA(PinCampana1); // Crea una nueva instancia de la clase CAMPANA para la campana 1
  CAMPANA* campana2 = new CAMPANA(PinCampana2); // Crea una nueva instancia de la clase CAMPANA para la campana 2 

  Campanario.AddCampana(campana1);              // Añade la campana 1 al campanario
  Campanario.AddCampana(campana2);              // Añade la campana 2 al campanario

  #ifdef DEBUG
    Serial.println("Iniciando Campanario...");
  #endif

  ConectarWifi();                                 // Llama a la función para conectar a la red Wi-Fi
  ServidorOn();                                   // Llama a la función para iniciar el servidor

}

void loop() {


  ChekearCuartos();                               // Llama a la función para chequear los cuartos y las horas y tocar las campanas correspondientes

if (secuenciaI2C > 0) {
    //nToque = secuenciaI2C; // Asigna la secuencia recibida
    Serial.print("OnEvent->WS_EVT_DATA: ");
    Serial.println(secuenciaI2C);                // Imprime la secuencia recibida por I2C
    secuenciaI2C = 0;      // Resetea para esperar la siguiente orden
/*
    if (secuenciaI2C == 1) {
      nToque = 1;                                 // Asigna la secuencia de difuntos
    } else if (secuenciaI2C == 2) {
      nToque = 2;                                 // Asigna la secuencia de fiesta
    } else if (secuenciaI2C == 3) {
      nToque = 3;                                 // Asigna la secuencia de hora
    } else {
      nToque = 0;                                 // Resetea la secuencia a 0 si no es válida
    }
*/
}

  switch (nToque)
  {
    case 1:
      Campanario.TocaDifuntos();                  // Llama a la función para tocar la secuencia de difuntos
      nToque = 0;                                 // Resetea la secuencia a 0
      break;
    case 2:
      Campanario.TocaFiesta();                    // Llama a la función para tocar la secuencia de fiesta
      nToque=0;                                   // Resetea la secuencia a 0
      break;
    case 3:
      Campanario.TocaHora(6); // Toca la campana del cuarto correspondiente
      //Campanario.TocaMisa();
      nToque = 0;                                 // Resetea la secuencia a 0
    default:
      break;
  }
  
  if (secuenciaI2C > 0) {
    //nToque = secuenciaI2C;                        // Asigna la secuencia recibida
    //secuenciaI2C = 0;                             // Resetea para esperar la siguiente orden
  }

  TestCampanadas();                            // Llama a la función para probar las campanadas y enviar el número de campana tocada a los clientes conectados

}