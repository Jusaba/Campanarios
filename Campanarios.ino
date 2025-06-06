#include <Wire.h>
#define I2C_SLAVE_ADDR 0x12
#define SDA_PIN 21
#define SCL_PIN 22

void recibirSecuencia(int numBytes) {
  while (Wire.available()) {
    uint8_t dato = Wire.read();
    Serial.print("Recibido por I2C: ");
    Serial.println(dato);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SLAVE_ADDR, SDA_PIN, SCL_PIN);
  Wire.setClock(100000); // Baja la velocidad a 100kHz
  Wire.onReceive(recibirSecuencia);
  Serial.println("I2C esclavo listo");
}

void loop() {
  delay(100);
}