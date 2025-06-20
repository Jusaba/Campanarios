//https://f1atb.fr/esp32-relay-integrated-230v-ac-power-supply-sensors/

#include "DNSServicio.h"
#include "Servidor.h"
#include "Campana.h"
#include "Campanario.h"
#include "RTC.h"
#include "ConexionWifi.h"
#include <Wire.h>
#include "Auxiliar.h"
#include "ModoAp.h"


#define DEBUG



void setup() {
  
  Serial.begin(9600);                                         // Iniciar la comunicación serie a 9600 baudios

  pinMode(19, INPUT_PULLUP); // Asegúrate de que el pin 19 está bien configurado

  if (digitalRead(19) == LOW) {
    Serial.println("Pin 19 a 0: Iniciando modo AP...");
    iniciarModoAP();
    while(1)
    {
      delay(100);
    }
  } else {

    cargarConfigWiFi(); // Carga la configuración guardada

    #ifdef DEBUG
      Serial.println("Cargando configuración WiFi...");
      Serial.print("SSID: ");
      Serial.println(configWiFi.ssid);
      Serial.print("IP: ");
      Serial.println(configWiFi.ip);
      Serial.print("Dominio: ");
      Serial.println(configWiFi.dominio);
      Serial.print("Usuario: ");
      Serial.println(configWiFi.usuario);
      Serial.print("Clave: ");
      Serial.println(configWiFi.clave);
    #endif

    #ifdef DEBUG
      Serial.println("Iniciando Campanario...");
    #endif

    Wire.begin(I2C_SLAVE_ADDR);                                 // Iniciar el bus I2C como esclavo con la dirección definida
    Wire.setClock(100000);
    Wire.onReceive(recibirSecuencia);
    Wire.onRequest(enviarEstadoI2C);

    CAMPANA* campana1 = new CAMPANA(PinCampana1);               // Crea una nueva instancia de la clase CAMPANA para la campana 1
    CAMPANA* campana2 = new CAMPANA(PinCampana2);               // Crea una nueva instancia de la clase CAMPANA para la campana 2

    CALEFACCION* calefaccion = new CALEFACCION(CalefaccionPin); // Crea una nueva instancia de la clase CALEFACCION   

    Campanario.AddCampana(campana1);                            // Añade la campana 1 al campanario
    Campanario.AddCampana(campana2);                            // Añade la campana 2 al campanario
    Campanario.AddCalefaccion(calefaccion);                     // Añade la calefacción al campanario  

    ConectarWifi(configWiFi);                                   // Llama a la función para conectar a la red Wi-Fi
    ServidorOn(configWiFi.usuario, configWiFi.clave);           // Llama a la función para iniciar el servidor
  }  
}

void loop() {
 
  ChekearCuartos();                                           // Llama a la función para chequear los cuartos y las horas y tocar las campanas correspondientes

  if (secuenciaI2C > 0) {                                     // Si se ha recibido orden por I2C
    EjecutaSecuencia(secuenciaI2C);                           // Llama a la función para ejecutar la orden recibida 
    secuenciaI2C = 0;                                         // Resetea para esperar la siguiente orden
    nToque = 0;                                               // Resetea el numero de la secuencia a tocar
  }
  
  if (nToque > 0) {                                           // Si la orden se ha recibido por websocket
    EjecutaSecuencia(nToque);                                 // Llama a la función para ejecutar la orden recibida de inernet
    nToque = 0;                                               // Resetea el numero de la secuencia a tocar  
  }

  TestCampanadas();                                           // Llama a la función para probar las campanadas y enviar el número de campana tocada a los clientes conectados

}

