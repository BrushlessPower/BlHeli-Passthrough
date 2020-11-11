/*
 * Firmware for BlHeli-Passthrough
 * Prototype Version V0.0
 */
 
#include <Arduino.h>
#include "Global.h"             /* Global Variables*/
#include "serial_comm.h"
#include "ble_comm.h"


void setup() {
  Serial.begin(115200);
  init_ble();
}

void loop() {
  process_serial();
  process_ble();
}
