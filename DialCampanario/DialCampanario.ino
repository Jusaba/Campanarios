
#include <Wire.h>

#define I2C_SLAVE_ADDR 0x12

void setup() {
  
  Wire.begin(13, 15); // SDA = 13, SCL = 15
  Wire.setClock(100000);
  Serial.begin(9600);
}

void loop() {
  Wire.beginTransmission(I2C_SLAVE_ADDR);
  Wire.write(42); // Prueba con cualquier valor
  Wire.endTransmission();
  Serial.println("Enviado 42");
  delay(1000);
}