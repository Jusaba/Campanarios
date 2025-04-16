#include "DNSServicio.h"
#include "Servidor.h"

// Replace with your network credentials
const char* ssid = "T_Wifi_jsb_rma";
const char* password = "9776424223112";

// Configuración de IP estática
IPAddress local_IP(192, 168, 1, 173); 
IPAddress gateway(192, 168, 1, 1);    
IPAddress subnet(255, 255, 255, 0);   
IPAddress primaryDNS(8, 8, 8, 8); // DNS primario (Google DNS)
IPAddress secondaryDNS(8, 8, 4, 4); // DNS secundario (Google DNS)


void setup() {
  Serial.begin(115200);

  pinMode(Rele0, OUTPUT);
  pinMode(Rele1, OUTPUT);
  pinMode(Rele2, OUTPUT);
  digitalWrite(Rele0, LOW);
  digitalWrite(Rele1, LOW);
  digitalWrite(Rele2, LOW);

  
    // Configurar IP estática
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("Error al configurar IP estática");
    }

  // Conexión a la red Wi-Fi
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado.");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());

  
  if (WiFi.status() == WL_CONNECTED) {
    ActualizaDNS();                                   // Llama a la función para actualizar el DNS
  } else {
    Serial.println("Sin conexion a la red Wi-Fi.");
  }
  ServidorOn();                                       // Llama a la función para iniciar el servidor
}

void loop() {
  //TestCliente(); // Llama a la función para manejar el cliente
}