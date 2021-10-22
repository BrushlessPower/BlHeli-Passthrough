/*
 * Firmware for BlHeli-Passthrough
 * Prototype Version V1.0
 */
 
#include <Arduino.h>
#include "Global.h"             /* Global Variables*/
#include "serial_comm.h"        /* Serial Code*/
#include "ble_comm.h"           /* BLE Code*/


void setup() {
  Serial.begin(115200);
  init_ble();                   // Init BLE Stack
}

void loop() {
  process_serial();             // Process Serial MSP/4-Way Data
  process_ble();                // Process BLE MSP/4-Way Data
}
