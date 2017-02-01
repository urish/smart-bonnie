#include <BLEPeripheral.h>
#include "ble.h"
#include "ble_gap.h"

#define LED_PIN 29

BLEPeripheral      blePeripheral        = BLEPeripheral();
BLEService         bonnieService        = BLEService("ff09");
BLECharacteristic  soundCharacteristic  = BLECharacteristic("ff0a", BLEWrite | BLEWriteWithoutResponse, 3);
BLECharacteristic  commandCharacteristic = BLECharacteristic("ff0b", BLEWrite | BLEWriteWithoutResponse, 3);

#define RSSI_THRESHOLD -50
#define TARGET_DEVICE_NAME "ng-beacon"

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
  playerCommand(0x12, fileNum);
}

ble_gap_scan_params_t scanParams = { active: 1, selective: 0, p_whitelist: NULL, interval: 50, window: 50 };

char *getDeviceName(const uint8_t *data, byte dlen) {
  static char result[16];
  byte index = 0;

  while (index + 1 < dlen) {
    byte field_len = data[index];
    byte field_type = data[index + 1];
    const void *field_data = &data[index + 2];
    index += field_len + 1;

    if (field_type == BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME) {
      field_len--;
      if (field_len > 15) {
        field_len = 15;
      }
      memcpy(result, field_data, field_len);
      result[field_len] = 0;
      return result;
    }
  }

  return NULL;
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(9600);
  Serial.println("Hello, Bonnie!");

  blePeripheral.setLocalName("BONNIE");
  blePeripheral.setAdvertisingInterval(500);
  blePeripheral.setTxPower(-4);
  blePeripheral.setAdvertisedServiceUuid(bonnieService.uuid());
  blePeripheral.addAttribute(bonnieService);
  blePeripheral.addAttribute(soundCharacteristic);
  blePeripheral.addAttribute(commandCharacteristic);

  blePeripheral.begin();

  scanParams.timeout = 0;
  sd_ble_gap_scan_start(&scanParams);
}

unsigned long lastBeacon = 0;

void loop() {
  uint32_t   evtBuf[(sizeof(ble_evt_t) + (GATT_MTU_SIZE_DEFAULT))];
  uint16_t   evtLen = sizeof(evtBuf);
  ble_evt_t* bleEvt = (ble_evt_t*)evtBuf;

  if (sd_ble_evt_get((uint8_t*)evtBuf, &evtLen) == NRF_SUCCESS) {
    if (bleEvt->header.evt_id == BLE_GAP_EVT_ADV_REPORT) {
      const ble_gap_evt_t * p_gap_evt = &bleEvt->evt.gap_evt;
      const ble_gap_evt_adv_report_t * p_adv_report = &p_gap_evt->params.adv_report;
      char *deviceName = getDeviceName(p_adv_report->data, p_adv_report->dlen);
      if (p_adv_report->rssi > RSSI_THRESHOLD && !strcmp(deviceName, TARGET_DEVICE_NAME)) {
        if (!lastBeacon) {
          playSound(5, 30);
        }
        lastBeacon = millis();
      }
    }
  } else {
    return;
  }

  if (millis() - lastBeacon > 2000 || millis() < lastBeacon) {
    lastBeacon = 0;
  }

  BLECentral central = blePeripheral.central(evtBuf, evtLen);

  if (central) {
    analogWrite(LED_PIN, 200);
    if (soundCharacteristic.written() && soundCharacteristic.valueLength() >= 2) {
      const uint8_t* value = soundCharacteristic.value();
      uint16_t fileNum = value[0] | (value[1] << 8);
      uint8_t volume = soundCharacteristic.valueLength() >= 3 ? value[2] : 0;
      playSound(fileNum, volume);
    }
    if (commandCharacteristic.written() && commandCharacteristic.valueLength() == 3) {
      const uint8_t* value = commandCharacteristic.value();
      playerCommand(value[0], value[1], value[2]);
    }
  } else {
    analogWrite(LED_PIN, millis() % 1000 < 100 ? 200 : 255);
  }
}

