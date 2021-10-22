#include <Arduino.h>
#include <NimBLEDevice.h>       // BLE Library
#include "Global.h"             // globale variablen
#include "ble_comm.h"           // Header
#include "MSP.h"                // MSP process
#include "4Way.h"               // 4 Way process
#include "ESC_Serial.h"         // Stop ESC Serial bei BLE disconnect

#define UUID128_SVC_COMMUNICATION                 "00001000-0000-1000-8000-00805f9b34fb"
#define UUID128_CHR_COMMUNICATION_WRITE           "00001001-0000-1000-8000-00805f9b34fb"
#define UUID128_CHR_COMMUNICATION_READ_NOTIFY     "00001002-0000-1000-8000-00805f9b34fb"

static NimBLEServer* pServer;

static BLECharacteristic* BlHeli_Read_CHR;
static BLECharacteristic* BlHeli_Write_CHR;

/*
 * Variablen für BLE Kommunikation
 */
bool ble_command = false;
bool ble_connected = false;
bool ble_disconnected = false;
uint16_t ble_buffer_len = 0;
volatile uint16_t ble_rx_counter = 0;
volatile uint16_t true_ble_rx_counter = 0;
uint16_t ble_tx_counter = 0;
uint8_t tx_counter = 0;

class MyServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
      Serial.println("Connected");
      Serial.print("Client address: ");
      Serial.println(NimBLEAddress(desc->peer_ota_addr).toString().c_str());
      pServer->updateConnParams(desc->conn_handle, 6, 12, 0, 60);
      ble_rx_counter = 0;
      ble_tx_counter = 0;
      ble_connected = true;
    };
    void onDisconnect(NimBLEServer* pServer) {
      Serial.println("disconnected");
      ble_rx_counter = 0;
      ble_tx_counter = 0;
      ble_disconnected = true;
      ble_connected = false;
    };
};

class MyCallbacks2: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic) {
      Serial.print(pCharacteristic->getUUID().toString().c_str());
      Serial.print(": onRead(), value: ");
      Serial.println(pCharacteristic->getValue().c_str());
    };
    void onWrite(NimBLECharacteristic* pCharacteristic) {

      std::string data = pCharacteristic->getValue();
      true_ble_rx_counter = data.length();
      Serial.print("Data (");
      Serial.print(data.length());
      Serial.print(") is written to  Device Communication Service: ");
      for (uint16_t i = 0; i < data.length(); i++) {                           // kopiere data Arry in RX_Buffer
        Serial.print(data[i], HEX);                     // mit serieller Anzeige was gesendet wurde
        Serial.print(" ");
        ble_rx[ble_rx_counter] = data[i];
        if (ble_rx_counter == 5000) {
          ble_rx_counter = 0;
        }
        else {
          ble_rx_counter ++;
        }
      }
      Serial.print("ble_rx_counter: ");
      Serial.println(ble_rx_counter);
      if ((pCharacteristic->getUUID().toString()) == "0x1001") {
        //Serial.println("BlHeli write");
        ble_command = true;
      }
    };
};

void init_ble(void) {
  NimBLEDevice::init("SBBUA_ESC");
  NimBLEDevice::setMTU(517);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  NimBLEService *pService2 = pServer->createService("1000");
  BlHeli_Read_CHR = pService2->createCharacteristic("1002", NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ );
  BlHeli_Write_CHR = pService2->createCharacteristic("1001", NIMBLE_PROPERTY::WRITE );
  BlHeli_Write_CHR->setCallbacks(new MyCallbacks2());
  BlHeli_Read_CHR->setCallbacks(new MyCallbacks2());
  pService2->start();
  pServer->start();

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(pService2->getUUID());
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->start();
  //BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void process_ble(void) {
  if (ble_command) {
    ble_command = false;
    delay(5);
    //Serial.print("ble_command: ");
    uint16_t peer_mtu = (pServer->getPeerMTU(0)) - 3;
    if ((peer_mtu < 20) || (peer_mtu > 517)) {
      peer_mtu = 20;
    }
    if (ble_rx[4] == 0) {
      ble_buffer_len = 256 + 7;
    }
    else {
      ble_buffer_len = ble_rx[4] + 7;
    }
    if ((ble_rx[0] == 0x2F) && ((ble_rx_counter == ble_buffer_len) || (true_ble_rx_counter == ble_buffer_len))) {
      // 4 Way Command
      // 4 Way proceed
      ble_command = false;
      Serial.print("4Way Command ");
      ble_rx_counter = 0;
      ble_tx_counter = Check_4Way(ble_rx);
      do {
        if (ble_tx_counter <= peer_mtu) {
          BlHeli_Read_CHR->setValue(&ble_rx[tx_counter], ble_tx_counter);
          BlHeli_Read_CHR->notify(true);
          Serial.print("send with length: ");
          Serial.println(ble_tx_counter);
          ble_tx_counter = 0;
        }
        else {
          BlHeli_Read_CHR->setValue(&ble_rx[tx_counter], peer_mtu);
          BlHeli_Read_CHR->notify(true);
          Serial.print("send with length: ");
          Serial.println(peer_mtu);
          tx_counter = tx_counter + peer_mtu;
          ble_tx_counter = ble_tx_counter - peer_mtu;
        }
        delay(50);
      } while (ble_tx_counter > 0);
      //ble_command = false;
      ble_tx_counter = 0;
      ble_rx_counter = 0;
      tx_counter = 0;
    }
    else if (ble_rx[0] == 0x2F) {
      Serial.print("4Way Command with Buffer length: ");
      Serial.println(ble_buffer_len);
      ble_command = false;
    }

    else if (ble_rx[0] == 0x24 && ble_rx[1] == 0x4D && ble_rx[2] == 0x3C) {
      // MSP Command
      // MSP Proceed
      Serial.print("MSP Command ");
      ble_tx_counter = MSP_Check(ble_rx, ble_rx_counter);
      do {
        if (ble_tx_counter <= peer_mtu) {
          BlHeli_Read_CHR->setValue(&ble_rx[tx_counter], ble_tx_counter);
          BlHeli_Read_CHR->notify(true);
          Serial.println("send");
          ble_tx_counter = 0;
        }
        else {
          BlHeli_Read_CHR->setValue(&ble_rx[tx_counter], ble_tx_counter);
          BlHeli_Read_CHR->notify(true);
          tx_counter = tx_counter + peer_mtu;
          ble_tx_counter = ble_tx_counter - peer_mtu;
        }
      } while (ble_tx_counter > 0);
      ble_tx_counter = 0;
      ble_rx_counter = 0;
      tx_counter = 0;
    }
    else {
      Serial.println("no MSP no 4way");
      Serial.println(Enable4Way);
      Serial.println(ble_rx[0], HEX);
      ble_tx_counter = 0;
      ble_rx_counter = 0;
      tx_counter = 0;
    }
    delay(10);
  }

  if (ble_disconnected) {
    DeinitSerialOutput();
    ble_disconnected = false;
  }
}
