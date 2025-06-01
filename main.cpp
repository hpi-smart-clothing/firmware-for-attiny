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
uint8_t sendIndex = 0;
volatile bool hasPendingCommand = false;
uint8_t pendingCommand = 0;

void setup() {
  TinyWireS.begin(I2C_SLAVE_ADDR);
  TinyWireS.onRequest(onRequest);
  TinyWireS.onReceive(onReceive);
  i2c_init();
}

void loop() {
  TinyWireS_stop_check();
  if (hasPendingCommand) {
    updateMockResponse(pendingCommand);  // Jetzt response aktualisieren
    hasPendingCommand = false;
  }
}

void onReceive(uint8_t howMany) {
  if (howMany > 0) {
    pendingCommand = TinyWireS.receive();
    hasPendingCommand = true;
    sendIndex = 0;
  }
}

void updateMockResponse(uint8_t cmd) {
  // Beispiel-Mockdaten
  switch (cmd) {
    case 0: break;
    case 1: strcpy(response, "QuatW:1.00\n"); break;
    case 2: strcpy(response, "QuatX:0.00\n"); break;
    case 3: strcpy(response, "QuatY:0.00\n"); break;
    case 4: strcpy(response, "QuatZ:0.00\n"); break;
    case 5: strcpy(response, "Gyro:128\n"); break;
    case 6: readChipIDToResponse(); break;
    case 7: initBNO(); break;
    case 8: readBNO_SYS_STATUS(); break;
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

int i2cRead1(uint8_t reg) {
  if (!i2c_start(BNO_ADDR << 1)) return -1;        // Write-Phase, erwarte ACK
  if (!i2c_write(reg)) return -2;                  // Registeradresse senden
  i2c_stop();
  delay(20);

  if (!i2c_start((BNO_ADDR << 1) | 1)) return -3;  // Read-Phase, erwarte ACK
  int val = i2c_read(0);                           // Lese 1 Byte, sende NACK zurück
  i2c_stop();
  delay(10);

  return val; // Rückgabe des gelesenen Werts (0–255)
}
bool i2cWrite1(uint8_t reg, uint8_t val) {
  delay(10);

  if (!i2c_start(BNO_ADDR << 1)) return false;     // Start + Write-Addr, erwarte ACK
  if (!i2c_write(reg)) return false;               // Schreibe Registeradresse, erwarte ACK
  if (!i2c_write(val)) return false;               // Schreibe Wert, erwarte ACK

  i2c_stop();
  delay(10);

  return true; // Alle Schritte erfolgreich
}

bool initBNO() {
  int chip = 0;

  // Set OPR_MODE = CONFIG_MODE
  if (!i2cWrite1(0x3D, 0x00)) {
    strcpy(response, "ERR: WR 0x3D\n");
    return false;
  }
  delay(25); // laut Datenblatt

  // Set PAGE_ID = 0x00
  if (!i2cWrite1(0x07, 0x00)) {
    strcpy(response, "ERR: WR 0x07\n");
    return false;
  }

  // Check CHIP_ID
  chip = i2cRead1(0x00);  // Register 0x00 = CHIP_ID
  if (chip < 0) {
    snprintf(response, sizeof(response), "ERR: RD CHIP %d\n", chip);
    return false;
  }
  if (chip != 0xA0) {
    snprintf(response, sizeof(response), "BAD CHIP: 0x%02X\n", chip);
    return false;
  }

  // Set POWER_MODE = Normal
  if (!i2cWrite1(0x3E, 0x00)) {
    strcpy(response, "ERR: WR 0x3E\n");
    return false;
  }

  // Set OPR_MODE = NDOF (Fusion Mode)
  if (!i2cWrite1(0x3D, 0x0C)) {
    strcpy(response, "ERR: WR 0x3D NDOF\n");
    return false;
  }

  delay(1000); // Sensorfusion Initialisierung

  strcpy(response, "BNO OK\n");
  return true;
}

void readChipIDToResponse() {
  int chip = i2cRead1(0x00);  // 0x00 = CHIP_ID Register

  if (chip < 0) {
    // Fehler beim Lesen
    strcpy(response, "I2C-Err\n");
  } else {
    // Chip ID erfolgreich gelesen
    // z. B. sollte 0xA0 zurückkommen
    snprintf(response, sizeof(response), "CHIP:0x%02X\n", chip);
  }

  sendIndex = 0; // Zurücksetzen für die Ausgabe
}

void readBNO_SYS_STATUS() {
  int status = i2cRead1(0x39);  // SYS_STATUS Register

  if (status < 0) {
    snprintf(response, sizeof(response), "ERR: I2C %d\n", status);
  } else {
    snprintf(response, sizeof(response), "SYS_STATUS:%d\n", status);
  }
}
