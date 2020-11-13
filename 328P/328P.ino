/*
 * Firmware for BlHeli-Passthrough
 * Prototype Version V0.0
 */
 
#include <Arduino.h>
#include "Global.h"             /* Global Variables*/
#include "serial_comm.h"
//#include "SoftwareSerial.h"

//SoftwareSerial swSer2(11,10);

void setup() {
  pinMode(5,OUTPUT),
  digitalWrite(5,HIGH);
  Serial.begin(115200);
  //swSer2.begin(19200);
}

void loop() {
  process_serial();
  digitalWrite(5,LOW);
  /*Serial.println("Hallo");
  while(Serial.available()){
    Serial.println(Serial.read());
  }
  delay(5000);*/
  /*swSer2.write(0x33);
  swSer2.write(0x44);
  swSer2.write(0x55);
  swSer2.write(0x66);


  while(swSer2.available()){
    Serial.println(swSer2.read());
  }
  delay(5000);*/
}
