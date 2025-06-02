#include "DNSServicio.h"
#include "Servidor.h"
#include "Campana.h"
#include "Campanario.h"
#include "RTC.h"
#include "ConexionWifi.h"
#include "Auxiliar.h"

#define DEBUG

#define PinCampana1 26                          // Definición del pin para la campana 1
#define PinCampana2 27                          // Definición del pin para la campana 2



void setup() {
  Serial.begin(9600);


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
  
  TestCampanadas();                            // Llama a la función para probar las campanadas y enviar el número de campana tocada a los clientes conectados

}