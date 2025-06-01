#include "DNSServicio.h"
#include "Servidor.h"
#include "Campana.h"
#include "Campanario.h"
#include "RTC.h"
#include "ConexionWifi.h"

#define DEBUG

#define PinCampana1 26 // Definición del pin para la campana 1
#define PinCampana2 27 // Definición del pin para la campana 2


CAMPANARIO Campanario;

//CAMPANA Campana1(PinCampana1); // Instancia de la clase CAMPANA para la campana 0
//CAMPANA Campana2(PinCampana2); // Instancia de la clase CAMPANA para la campana 1

void setup() {
  Serial.begin(9600);


  CAMPANA* campana1 = new CAMPANA(PinCampana1);
  CAMPANA* campana2 = new CAMPANA(PinCampana2);

  Campanario.AddCampana(campana1);                     // Añade la campana 1 al campanario
  Campanario.AddCampana(campana2);                     // Añade la campana 2 al campanario

  #ifdef DEBUG
    Serial.println("Iniciando Campanario...");
  #endif

  ConectarWifi();                                     // Llama a la función para conectar a la red Wi-Fi

  ServidorOn();                                       // Llama a la función para iniciar el servidor

  //Campanario.TocaDifuntos();                          // Toca la secuencia de difuntos al iniciar
  //Campanario.TocaFiesta();                            // Toca la secuencia de fiesta al iniciar

}

void loop() {
  
  switch (nToque)
  {
    case 1:
      Campanario.TocaDifuntos();                        // Llama a la función para tocar la secuencia de difuntos
      nToque = 0;                                   // Resetea la secuencia a 0
      break;
    case 2:
      Campanario.TocaFiesta();                          // Llama a la función para tocar la secuencia de fiesta
      nToque=0;                                   // Resetea la secuencia a 0
      break;
    case 3:
      //Campanario.TocaMisa();
      nToque = 0;                                   // Resetea la secuencia a 0
    default:
      break;
  }
  Campanario.ActualizarSecuenciaCampanadas();
}