//#include <bluefruit.h>
#include <Arduino.h>
#include "Global.h"
#include "ESC_Serial.h"
#include "SoftwareSerial.h"

//#define _DEBUG_
#define ESC_RX 11 
#define ESC_TX 10 


uint16_t esc_crc = 0;

SoftwareSerial swSer1(ESC_RX,ESC_TX);

void InitSerialOutput(){
  //swSer1.begin(19200, SWSERIAL_8N1, ESC_RX, ESC_RX, false, 512);
  swSer1.begin(19200);
  //swSer1.enableIntTx(false);
#ifdef _DEBUG_   
  Serial.println("Init ESC Serial");
#endif
}

uint16_t SendESC(uint8_t tx_buf[], uint16_t buf_size){
  uint16_t i = 0;
  uint8_t dummy;
  esc_crc = 0;
  if(buf_size == 0){
    buf_size = 256;
  }
#ifdef _DEBUG_  
  Serial.print("write ESC: ");
#endif
  //swSer1.enableTx(true);
  for(i = 0; i < buf_size; i++){
    //Serial1.write(tx_buf[i]);
    swSer1.write(tx_buf[i]);
#ifdef _DEBUG_  
    Serial.print(tx_buf[i],HEX);
    Serial.print(" ");
#endif
    esc_crc = ByteCrc(tx_buf[i],esc_crc);
  }
  if(ESC_CRC){
    //Serial1.write(esc_crc & 0xff);
    //Serial1.write((esc_crc >> 8) & 0xff);
    swSer1.write(esc_crc & 0xff);
    swSer1.write((esc_crc >> 8) & 0xff);
#ifdef _DEBUG_  
    Serial.print(esc_crc & 0xff,HEX);
    Serial.print(" ");
    Serial.print((esc_crc >> 8) & 0xff,HEX);
    Serial.print(" ");
#endif
    buf_size = buf_size + 2;
  }
  //swSer1.enableTx(false);
#ifdef _DEBUG_  
  Serial.println("done");
#endif
  i = 0;
  /*delay(50);
  while((Serial1.available()) && (i < buf_size)){
    dummy = Serial1.read();
    delayMicroseconds(100);
    i++;
  }*/
  return i;
}

uint16_t GetESC(uint8_t rx_buf[], uint16_t wait_ms ){
  uint16_t i = 0;
  bool timeout = false;
#ifdef _DEBUG_  
  Serial.print("ESC Read: ");
#endif
  while((!swSer1.available()) && (!timeout) ){
    delay(1);
    i++;
    if(i >= wait_ms){
      timeout = true;
#ifdef _DEBUG_  
      Serial.println(" Timeout");
#endif
      return 0;
    }
  }
  i = 0;
  while(swSer1.available()){
    rx_buf[i] = swSer1.read();
    //swSer1.write(rx_buf[i]);
#ifdef _DEBUG_  
    Serial.print(rx_buf[i],HEX);
    Serial.print(" ");
#endif
    i++;
    delayMicroseconds(100);
  }
#ifdef _DEBUG_  
  Serial.println("done");
#endif
  return i;
}

uint16_t ByteCrc(uint8_t data,uint16_t crc)
{
    uint8_t xb = data;
    for (uint8_t i = 0; i < 8; i++)
    {
        if (((xb & 0x01) ^ (crc & 0x0001)) !=0 ) {
            crc = crc >> 1;
            crc = crc ^ 0xA001;
        } else {
            crc = crc >> 1;
        }
        xb = xb >> 1;
    }
    return crc;
}
