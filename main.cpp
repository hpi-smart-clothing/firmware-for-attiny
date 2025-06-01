#include <TinyWireS.h>
// SoftI2C: PB3 = SDA, PB4 = SCL
#define SDA_PORT PORTB
#define SDA_PIN 3
#define SCL_PORT PORTB
#define SCL_PIN 4
#include <SoftI2CMaster.h>

#define BNO_ADDR 0x28

#define I2C_SLAVE_ADDR 0x08

char response[16] = "INIT\n";
volatile uint8_t lastCommand = 0;
uint8_t sendIndex = 0;

void setup() {
  TinyWireS.begin(I2C_SLAVE_ADDR);
  TinyWireS.onRequest(onRequest);
  TinyWireS.onReceive(onReceive);
  i2c_init();
}

void loop() {
  TinyWireS_stop_check();
}

void onReceive(uint8_t howMany) {
  if (howMany > 0) {
    lastCommand = TinyWireS.receive();
    updateMockResponse(lastCommand);
    sendIndex = 0;  // <<<<< wichtig: Reset bei neuem Kommando!
  }
}

void updateMockResponse(uint8_t cmd) {
  // Beispiel-Mockdaten
  switch (cmd) {
    case 1: strcpy(response, "QuatW:1.00\n"); break;
    case 2: strcpy(response, "QuatX:0.00\n"); break;
    case 3: strcpy(response, "QuatY:0.00\n"); break;
    case 4: strcpy(response, "QuatZ:0.00\n"); break;
    case 5: strcpy(response, "Gyro:128\n"); break;
    default: strcpy(response, "Unbekannt\n"); break;
  }
}

void onRequest() {
  TinyWireS.send(response[sendIndex]);
  sendIndex++;
  if (response[sendIndex] == '\0') {
    sendIndex = 0;
  }
}

uint8_t readRegister(uint8_t reg) {
  i2c_start(BNO_ADDR << 1);
  i2c_write(reg);
  i2c_stop();
  delay(10);

  i2c_start((BNO_ADDR << 1) | 1);
  uint8_t val = i2c_read(0);
  i2c_stop();
  delay(10);
  return val;
}

bool i2cWrite1(uint8_t reg, uint8_t val) {
  delay(10);
  i2c_start(BNO_ADDR << 1); // Write
  i2c_write(reg);
  i2c_write(val);
  i2c_stop();
  delay(10);
  return true; // SoftI2CMaster gibt kein ACK zurück, daher immer true
}

bool initBNO() {
  //Set OPR_MODE = CONFIG_MODE
  i2cWrite1(0x3D, 0x00); // OPR_MODE = 0x00
  delay(25); // Datenblatt sagt min. 19ms

  //Set PAGE_ID = 0x00
  i2cWrite1(0x07, 0x00); // PAGE_ID

  //Check CHIP_ID
  uint8_t chip = i2cRead1(0x00); // CHIP_ID
  if (chip != 0xA0) {
    return false;
  }

  //Set POWER_MODE = 0x00 (normal)
  i2cWrite1(0x3E, 0x00); // POWER_MODE

  delay(10);

  //Set OPR_MODE = NDOF
  i2cWrite1(0x3D, 0x0C);
  delay(1000); // Sensor braucht Zeit für Sensorfusion-Start

  return true;
}
