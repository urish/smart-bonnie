#include <BLEPeripheral.h>

BLEPeripheral      blePeripheral        = BLEPeripheral();
BLEService         bonnieService        = BLEService("ff09");
BLECharacteristic  soundCharacteristic  = BLECharacteristic("ff0a", BLEWrite | BLEWriteWithoutResponse, 3);

void playerCommand(uint8_t cmd, uint8_t arg1, uint8_t arg2) {
  uint8_t data[10] = {0x7e, 0xff, 0x6, cmd, 0, arg1, arg2, 0, 0, 0xef};
  int16_t checksum = 0 - data[1] - data[2] - data[3] - data[4] - data[5] - data[6];
  data[7] = (checksum >> 8) & 0xff;
  data[8] = checksum & 0xff;
  Serial.write(data, sizeof(data));
}

void playerCommand(uint8_t cmd, uint16_t arg) {
  playerCommand(cmd, arg >> 8, arg & 0xff);
}

void playSound(uint16_t fileNum, uint8_t volume) {
  if (volume > 0) {
    playerCommand(0x6, 0, volume);
    delay(10);
  }
  playerCommand(0x3, fileNum);
}

void setup() {
  Serial.begin(9600);
  Serial.println("Hello, Bonnie!");

  blePeripheral.setLocalName("BONNIE");
  blePeripheral.setAdvertisingInterval(500);
  blePeripheral.setTxPower(-4);
  blePeripheral.setAdvertisedServiceUuid(bonnieService.uuid());
  blePeripheral.addAttribute(bonnieService);
  blePeripheral.addAttribute(soundCharacteristic);

  blePeripheral.begin();
}

void loop() {
  BLECentral central = blePeripheral.central();

  if (central) {
    Serial.println("Game on!");

    while (central.connected()) {
      if (soundCharacteristic.written() && soundCharacteristic.valueLength() >= 2) {
        const uint8_t* value = soundCharacteristic.value();
        uint16_t fileNum = value[0] | (value[1] << 8);
        uint8_t volume = soundCharacteristic.valueLength() >= 3 ? value[2] : 0;
        playSound(fileNum, volume);
      }
    }

    Serial.println("Bye bye :-)");
  }
}
