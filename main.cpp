#include <TinyWireS.h>

#define I2C_SLAVE_ADDR 0x08

char response[16] = "INIT\n";
volatile uint8_t lastCommand = 0;
uint8_t sendIndex = 0;

void setup() {
  TinyWireS.begin(I2C_SLAVE_ADDR);
  TinyWireS.onRequest(onRequest);
  TinyWireS.onReceive(onReceive);
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
