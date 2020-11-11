#include <Arduino.h>
//#include "BLEDevice.h"          /* BLE Library for ESP32*/
//#include "BLEServer.h"          /* BLE Library for ESP32*/
//#include <BLEUtils.h>           /* BLE Library for ESP32*/
//#include <BLE2902.h>            /* BLE Library for ESP32*/
//#include <BLEAddress.h>         /* BLE Library for ESP32*/
#include <NimBLEDevice.h>
#include "Global.h"
#include "ble_comm.h"
#include "MSP.h"
#include "4Way.h"
#include "ESC_Serial.h"


#define UUID128_SVC_COMMUNICATION               "00001000-0000-1000-8000-00805f9b34fb"
#define UUID128_CHR_COMMUNICATION_WRITE         "00001001-0000-1000-8000-00805f9b34fb"
#define UUID128_CHR_COMMUNICATION_READ_NOTIFY   "00001002-0000-1000-8000-00805f9b34fb"


//BLEServer *pServer;              /* Create Server Object*/
static NimBLEServer* pServer;
BLECharacteristic* BlHeli_Write_CHR = NULL;
BLECharacteristic* BlHeli_Read_CHR = NULL;


//BLEDescriptor WriteDescriptor(BLEUUID((uint16_t)0x2901));
//BLEDescriptor ReadDescriptor(BLEUUID((uint16_t)0x2901));

//BLECharacteristic BlHeli_Write_CHR(UUID128_CHR_COMMUNICATION_WRITE, BLECharacteristic::PROPERTY_WRITE);
//BLECharacteristic BlHeli_Read_CHR(UUID128_CHR_COMMUNICATION_READ_NOTIFY, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
//BLECharacteristic Debug_CHR(UUID128_CHR_DEBUG_NOTIFY, BLECharacteristic::PROPERTY_NOTIFY);
//BLECharacteristic Live_CHR(UUID128_CHR_LIVE_NOTIFY, BLECharacteristic::PROPERTY_NOTIFY);


bool ble_command = false;
uint16_t ble_buffer_len = 0;
uint16_t ble_rx_counter = 0;
uint16_t ble_tx_counter = 0;
uint8_t tx_counter = 0;

class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
        Serial.println("Client connected");
        Serial.print("Client address: ");
        Serial.println(NimBLEAddress(desc->peer_ota_addr).toString().c_str());
        /** We can use the connection handle here to ask for different connection parameters.
         *  Args: connection handle, min connection interval, max connection interval
         *  latency, supervision timeout.
         *  Units; Min/Max Intervals: 1.25 millisecond increments.
         *  Latency: number of intervals allowed to skip.
         *  Timeout: 10 millisecond increments, try for 5x interval time for best results.  
         */
        pServer->updateConnParams(desc->conn_handle, 24, 48, 0, 60);
        ble_rx_counter = 0;
        ble_tx_counter = 0;
    };
    void onDisconnect(NimBLEServer* pServer) {
        Serial.println("Client disconnected - start advertising");
        NimBLEDevice::startAdvertising();
        ble_rx_counter = 0;
        ble_tx_counter = 0;
    };
};

class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic){
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onRead(), value: ");
        Serial.println(pCharacteristic->getValue().c_str());
    };
    void onWrite(NimBLECharacteristic* pCharacteristic) {
      //Serial.println(pCharacteristic->getUUID().toString().c_str());
      /*if((pCharacteristic->getUUID().toString())=="0x1003"){
        Serial.println("Debug write");
      }
      if((pCharacteristic->getUUID().toString())=="0x1001"){
        Serial.println("BlHeli write");
      }*/
      std::string data = pCharacteristic->getValue();

      Serial.print("Data is written to  Device Status Service: ");
      for(uint8_t i = 0; i < data.length(); i++){                             // kopiere data Arry in RX_Buffer
        Serial.print(data[i],HEX);                      // mit serieller Anzeige was gesendet wurde
        Serial.print(" ");
        ble_rx[ble_rx_counter] = data[i];
        ble_rx_counter ++;
      }
      Serial.print("ble_rx_counter: ");
      Serial.println(ble_rx_counter);
      ble_command = true;
    };
    void onNotify(NimBLECharacteristic* pCharacteristic) {
        //Serial.println("Sending notification to clients");
    };
    void onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code) {
        /*String str = ("Notification/Indication status code: ");
        str += status;
        str += ", return code: ";
        str += code;
        str += ", "; 
        str += NimBLEUtils::returnCodeToString(code);
        Serial.println(str);*/
    };

    void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) {
        /*String str = "Client ID: ";
        str += desc->conn_handle;
        str += " Address: ";
        str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
        if(subValue == 0) {
            str += " Unsubscribed to ";
        }else if(subValue == 1) {
            str += " Subscribed to notfications for ";
        } else if(subValue == 2) {
            str += " Subscribed to indications for ";
        } else if(subValue == 3) {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID()).c_str();

        Serial.println(str);*/
    };
};

static CharacteristicCallbacks chrCallbacks;

void init_ble(void){
  NimBLEDevice::init("SBBUA_ESC"); 
  NimBLEDevice::setMTU(517);
  //uint16_t a = BLEDevice::getMTU();
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);

  // Create the BLE Server
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());
  
  // Create the BLE Service
  NimBLEService *pService = pServer->createService("1000");

  /*pService->addCharacteristic(&BlHeli_Read_CHR);
  pService->addCharacteristic(&BlHeli_Write_CHR);
  pService->addCharacteristic(&Debug_CHR);
  pService->addCharacteristic(&Live_CHR);
  BlHeli_Write_CHR.setCallbacks(new MyCallbacks());*/


  BlHeli_Read_CHR = pService->createCharacteristic(
                                               "1002",
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::NOTIFY 
                                              );
  BlHeli_Read_CHR->setCallbacks(&chrCallbacks);

  BlHeli_Write_CHR = pService->createCharacteristic(
                                               "1001",
                                               NIMBLE_PROPERTY::WRITE 
                                              );
  BlHeli_Write_CHR->setCallbacks(&chrCallbacks);

  //BlHeli_Read_CHR.addDescriptor(&ReadDescriptor);
  //BlHeli_Read_CHR.addDescriptor(new BLE2902());

  //BlHeli_Write_CHR.addDescriptor(&WriteDescriptor);
  //BlHeli_Write_CHR.addDescriptor(new BLE2902());


    // Start the service
  pService->start();

  // Start advertising
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    /** Add the services to the advertisment data **/
    pAdvertising->addServiceUUID(pService->getUUID());
    /** If your device is battery powered you may consider setting scan response
     *  to false as it will extend battery life at the expense of less data sent.
     */
    pAdvertising->setScanResponse(true);
    pAdvertising->start();
}


void process_ble(void){
  if(ble_command){
    uint16_t peer_mtu = /*20;//*/(pServer->getPeerMTU(0))-3;
    if((peer_mtu < 20) || (peer_mtu > 517)){
      peer_mtu = 20;
    }
    if(ble_rx[4] == 0){
      ble_buffer_len = 256 + 7;
    }
    else{
      ble_buffer_len = ble_rx[4] + 7;
    }
    if((ble_rx[0] == 0x2F) && (ble_rx_counter == ble_buffer_len)/*&& Enable4Way*/){
      // 4 Way Command
      // 4 Way proceed
      ble_rx_counter = 0;
      ble_tx_counter = Check_4Way(ble_rx);
      do{
        if(ble_tx_counter <= peer_mtu){
          BlHeli_Read_CHR->setValue(&ble_rx[tx_counter], ble_tx_counter);
          BlHeli_Read_CHR->notify(true);
          Serial.print("send with length: ");
          Serial.println(ble_tx_counter);
          ble_tx_counter = 0;
        }
        else{
          BlHeli_Read_CHR->setValue(&ble_rx[tx_counter], peer_mtu);
          BlHeli_Read_CHR->notify(true);
          Serial.print("send with length: ");
          Serial.println(peer_mtu);
          tx_counter = tx_counter + peer_mtu;
          ble_tx_counter = ble_tx_counter - peer_mtu;
        }
        delay(50);
      }while(ble_tx_counter>0);
      ble_command = false;
      ble_tx_counter = 0;
      ble_rx_counter = 0;
      tx_counter = 0;
    }
    else if(ble_rx[0] == 0x2F){
      Serial.print("4Way Command with Buffer length: ");
      Serial.println(ble_buffer_len);
      ble_command = false;
    }
  
    else if(ble_rx[0] == 0x24 && ble_rx[1] == 0x4D && ble_rx[2] == 0x3C){
      // MSP Command
      // MSP Proceed
      Serial.print("MSP Command ");
      ble_tx_counter = MSP_Check(ble_rx,ble_rx_counter);
      do{
        if(ble_tx_counter <= peer_mtu){
          BlHeli_Read_CHR->setValue(&ble_rx[tx_counter], ble_tx_counter);
          //delay(500);
          BlHeli_Read_CHR->notify(true);
          Serial.println("send");
          ble_tx_counter = 0;
        }
        else{
          BlHeli_Read_CHR->setValue(&ble_rx[tx_counter], ble_tx_counter);
          BlHeli_Read_CHR->notify(true);
          tx_counter = tx_counter + peer_mtu;
          ble_tx_counter = ble_tx_counter - peer_mtu;
        }
        //delay(500);
      }while(ble_tx_counter>0);
      ble_command = false;
      ble_tx_counter = 0;
      ble_rx_counter = 0;
      tx_counter = 0;
    }
    else{
      Serial.println("no MSP no 4way");
      Serial.println(Enable4Way);
      Serial.println(ble_rx[0]);
      ble_command = false;
      ble_tx_counter = 0;
      ble_rx_counter = 0;
      tx_counter = 0;
    }
  }
}
