#include <M5Dial.h>
#include <Wire.h>

#define I2C_SLAVE_ADDR 0x12

void setup() {

  auto cfg = M5.config();
  M5Dial.begin(cfg, false, false);

  Wire.begin();
  Wire.setClock(50000);
  Serial.begin(115200);

}

void loop() {
  Wire.beginTransmission(I2C_SLAVE_ADDR);
  Wire.write(42); // Prueba con cualquier valor
  Wire.endTransmission();
  Serial.println("Enviado 42");
  delay(1000);
}