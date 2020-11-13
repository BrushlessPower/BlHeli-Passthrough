#include <Arduino.h>
#include "Global.h"
#include "serial_comm.h"
#include "MSP.h"
#include "4Way.h"
#include "ESC_Serial.h"

uint16_t serial_rx_counter = 0;
uint16_t serial_tx_counter = 0;
uint16_t serial_buffer_len = 0;

bool serial_command = false;

void process_serial(void){
  if(Serial.available()){
    serial_command = true;
    delay(1);
    //digitalWrite(5,HIGH);
    while(Serial.available()){
      serial_rx[serial_rx_counter] = Serial.read();
      //Serial.print(serial_rx[serial_rx_counter],HEX);                      // mit serieller Anzeige was gesendet wurde
      //Serial.print(" ");
      //Serial.write(serial_rx[serial_rx_counter]);
      if(serial_rx_counter == 4){
        if(serial_rx[4] == 0){
          serial_buffer_len = 256 + 7;
        }
        else{
          serial_buffer_len = serial_rx[4] + 7;
        }
      }
      serial_rx_counter ++;
      if(serial_rx_counter == serial_buffer_len){
        break;
      }
    }
    //digitalWrite(5,LOW);
  }
  
  if(serial_command){
    //digitalWrite(5,HIGH);
    if((serial_rx[0] == 0x2F) && (serial_rx_counter == serial_buffer_len)/*&& Enable4Way*/){
      // 4 Way Command
      // 4 Way proceed
      serial_tx_counter = Check_4Way(serial_rx);
      for(uint16_t b=0; b<serial_tx_counter; b++){
        Serial.write(serial_rx[b]);
      }
      serial_command = false;
      serial_rx_counter = 0;
      serial_tx_counter = 0;
      /*int dummy = Serial.read();
      if( dummy != -1){
        delay(5);
        dummy = Serial.read();
        if(dummy == 0x30){
          while(Serial.read() != -1){
            
          }
          Serial.write(0x2E);
          Serial.write(0x30);
          Serial.write(0x00);
          Serial.write(0x00);
          Serial.write(0x01);
          Serial.write(0x00);
          Serial.write(0x00);
          Serial.write(0x44);
          Serial.write(0xC2);
        }
      }*/
    }
    else if(serial_rx[0] == 0x24 && serial_rx[1] == 0x4D && serial_rx[2] == 0x3C){
      // MSP Command
      // MSP Proceed
      serial_tx_counter = MSP_Check(serial_rx,serial_rx_counter);
      //Serial.print("length ");
      //Serial.println(i);
      for(uint8_t b=0; b<serial_tx_counter; b++){
        Serial.write(serial_rx[b]);
      }
      serial_command = false;
      serial_rx_counter = 0;
      serial_tx_counter = 0;
    }
    serial_command = false;
    //serial_rx_counter = 0;
    //serial_tx_counter = 0;
    //digitalWrite(5,LOW);
  }
}
