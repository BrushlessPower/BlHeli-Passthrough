#include <Arduino.h>
#include "Global.h"
#include "4way_protocol.h"
#include "4Way.h"
#include "ESC_Serial.h"

//#define _DEBUG_


uint16_t Check_4Way(uint8_t buf[]){
  uint8_t cmd = buf[1];
  uint8_t addr_high = buf[2];
  uint8_t addr_low = buf[3];   
  uint8_t I_Param_Len = buf[4];
  uint8_t param = buf[5];               // param = ParamBuf[0]
  uint8_t ParamBuf[256] = {0};          // Parameter Buffer
  uint16_t crc = 0;
  uint16_t buf_size;                    // return Output Buffer Size -> O_Param_Len + Header + CRC
  uint8_t ack_out = ACK_OK;
  uint8_t O_Param_Len = 0;

  for(uint8_t i = 0; i<5 ; i++){                // CRC Check of Header (CMD, Adress, Size
    crc = _crc_xmodem_update (crc, buf[i]);
  }
  uint8_t InBuff = I_Param_Len;                 // I_Param_Len = 0 -> 256Bytes
  uint16_t i = 0;                               // work counter
  do {                                          // CRC Check of Input Parameter Buffer
    crc = _crc_xmodem_update (crc, buf[i+5]);
    ParamBuf[i] = buf[i+5];
    i++;
    InBuff--;
  } while (InBuff != 0);
  uint16_t crc_in = ((buf[i+5] << 8) | buf[i+6]);
  
#ifdef _DEBUG_ 
    Serial.print("4Way CMD: ");
    Serial.print(cmd,HEX);
    Serial.print(" Adress: ");
    Serial.print(addr_high, HEX);
    Serial.print(addr_low, HEX);
    Serial.print(" ParamBuf Size: ");
    Serial.print(I_Param_Len,HEX);
    // buffer
    Serial.print(" ParamBuf[0]: ");
    Serial.print(ParamBuf[0],HEX);
    // buffer
    Serial.print(" CRC_in: ");
    Serial.print(crc_in,HEX);
    Serial.print(" CRC calculated: ");
    Serial.println(crc, HEX);
#endif  

  if(crc_in != crc){
#ifdef _DEBUG_ 
    Serial.print("Wrong CRC ");
#endif
    buf[0] = cmd_Remote_Escape;
    buf[1] = cmd;
    buf[2] = addr_high;
    buf[3] = addr_low;
    buf[4] = 0x01;        // Output Param Lenght
    buf[5] = 0x00;        // Dummy
    buf[6] = ACK_I_INVALID_CRC;    // ACK
    for(uint8_t i = 0; i<7; i++){
      crc = _crc_xmodem_update (crc, buf[i]);
    }
    buf[7] = (crc >> 8) & 0xff;
    buf[8] = crc & 0xff;
#ifdef _DEBUG_ 
    Serial.print("with CRC: 0x");
    Serial.print(buf[7],HEX);
    Serial.print(" ");
    Serial.print(buf[8],HEX);
    Serial.print(" ");
#endif
    buf_size = 9;
  }
  
  crc = 0;

  if(cmd == cmd_DeviceInitFlash){
#ifdef _DEBUG_ 
    Serial.print("DeviceInitFlash ");
#endif
    buf[0] = cmd_Remote_Escape;
    buf[1] = cmd_DeviceInitFlash;
    buf[2] = addr_high;
    buf[3] = addr_low;
    buf[4] = 0x04;        // Output Param Lenght
    if(param == 0x00){    // ESC Count
      uint8_t BootInit[] = {0,0,0,0,0,0,0,0,0x0D,'B','L','H','e','l','i',0xF4,0x7D};
      uint8_t Init_Size = 17;
      uint8_t RX_Size = 0;
      uint8_t RX_Buf[250] = {0};
      // read Answer Format = BootMsg("471c") SIGNATURE_001, SIGNATURE_002, BootVersion, BootPages (,ACK = brSUCCESS = 0x30)
      // if ok -> ACK OK
      // else -> ACK_D_GENERAL_ERROR
      //ESC_OK = false;
      ESC_CRC = false;
      //RX_Size = SendESC(BootInit, RX_Buf, Init_Size);
      RX_Size = SendESC(BootInit, Init_Size);
      delay(5);
      RX_Size = GetESC(RX_Buf, 200);
      ESC_CRC = true;
#ifdef _DEBUG_ 
      Serial.print("ESC Received: ");
      Serial.print(RX_Size);
      Serial.print(" ");
      Serial.print("Bytes: ");
      Serial.print(RX_Buf[0],HEX);
      Serial.print(" ");
      Serial.print(RX_Buf[1],HEX);
      Serial.print(" ");
      Serial.print(RX_Buf[2],HEX);
      Serial.print(" ");
      Serial.print(RX_Buf[3],HEX);
      Serial.print(" ");
      Serial.print(RX_Buf[4],HEX);
      Serial.print(" ");
      Serial.print(RX_Buf[5],HEX);
      Serial.print(" ");
      Serial.print(RX_Buf[6],HEX);
      Serial.print(" ");
      Serial.print(RX_Buf[7],HEX);
      Serial.print(" ");
      Serial.print(RX_Buf[8],HEX);
#endif
      if(RX_Buf[8] == brSUCCESS){
        //ESC_OK = true;
        buf[5] = RX_Buf[5];       // Device Signature2?
        buf[6] = RX_Buf[4];       // Device Signature1?
        buf[7] = RX_Buf[3];       // "c"?
        buf[8] = imARM_BLB;       // 4-Way Mode: ARMBLB = 0x04
        buf[9] = ACK_OK;          // ACK
#ifdef _DEBUG_         
        Serial.print("OK");
#endif
      }
      else{
        buf[5] = 0x06;        // Device Signature2?
        buf[6] = 0x33;        // Device Signature1?
        buf[7] = 0x67;        // "c"?
        buf[8] = imARM_BLB;   // Boot Pages?
        buf[9] = ACK_D_GENERAL_ERROR;    // ACK
#ifdef _DEBUG_         
        Serial.print("General Error");
#endif
      }
    }
    else{
      buf[9] = ACK_I_INVALID_CHANNEL;    // ACK
#ifdef _DEBUG_ 
      Serial.print("Invalid channel");
#endif
    }
    for(uint8_t i = 0; i<10; i++){
      crc = _crc_xmodem_update (crc, buf[i]);
    }
    buf[10] = (crc >> 8) & 0xff;
    buf[11] = crc & 0xff;
#ifdef _DEBUG_ 
    Serial.print("with CRC: 0x");
    Serial.print(buf[10],HEX);
    Serial.print(" ");
    Serial.print(buf[11],HEX);
    Serial.print(" ");
#endif
    buf_size = 12;
  }
  
  else if(cmd == cmd_DeviceReset){
#ifdef _DEBUG_ 
    Serial.print("DeviceReset ");
#endif
    buf[0] = cmd_Remote_Escape;
    buf[1] = cmd_DeviceReset;
    buf[2] = addr_high;
    buf[3] = addr_low;
    buf[4] = 0x01;        // Output Param Lenght
    buf[5] = 0x00;        // Dummy
    if(param == 0x00){    // ESC Count
      buf[6] = ACK_OK;    // ACK
      uint8_t ESC_data[2] = {RestartBootloader,0};
      uint16_t Data_Size = 2;
      uint16_t RX_Size = 0;
      uint8_t RX_Buf[250] = {0};
      ESC_CRC = true;
        //RX_Size = SendESC(ESC_data, RX_Buf, Data_Size);
      RX_Size = SendESC(ESC_data, Data_Size);
      // Keine Antwort vom ESC
      delay(1);
      GetESC(RX_Buf, 50);
    }
    else{
      buf[6] = ACK_I_INVALID_CHANNEL;    // ACK
#ifdef _DEBUG_ 
      Serial.print("Invalid channel");
#endif
    }
    for(uint8_t i = 0; i<7; i++){
      crc = _crc_xmodem_update (crc, buf[i]);
    }
    buf[7] = (crc >> 8) & 0xff;
    buf[8] = crc & 0xff;
#ifdef _DEBUG_ 
    Serial.print("with CRC: 0x");
    Serial.print(buf[7],HEX);
    Serial.print(" ");
    Serial.print(buf[8],HEX);
    Serial.print(" ");
#endif    
    buf_size = 9;
  }
  
  else if(cmd == cmd_InterfaceTestAlive){
#ifdef _DEBUG_ 
    Serial.print("InterfaceTestAlive ");
#endif
    buf[0] = cmd_Remote_Escape;
    buf[1] = cmd_InterfaceTestAlive;
    buf[2] = addr_high;
    buf[3] = addr_low;
    buf[4] = 0x01;        // Output Param Lenght
    buf[5] = 0x00;        // Dummy
    buf[6] = ACK_OK;
    uint8_t ESC_data[2] = {CMD_KEEP_ALIVE,0};
    uint16_t Data_Size = 2;
    uint16_t RX_Size = 0;
    uint8_t RX_Buf[250] = {0};
    //ESC_OK = true;
    //RX_Size = SendESC(ESC_data, RX_Buf, Data_Size);
/*    RX_Size = SendESC(ESC_data, Data_Size);
    delay(5);
    RX_Size = GetESC(RX_Buf, 200);
#ifdef _DEBUG_
    Serial.print("ESC Received: ");
    Serial.print(RX_Size);
    Serial.print(" Bytes: ");
    Serial.print(RX_Buf[0],HEX);
#endif
    if(RX_Buf[0] == brSUCCESS){
      //buf[6] = ACK_OK;    // ACK
    }
    else{
      //buf[6] = ACK_D_GENERAL_ERROR;    // ACK
    }
*/    
    for(uint8_t i = 0; i<7; i++){
      crc = _crc_xmodem_update (crc, buf[i]);
    }
    buf[7] = (crc >> 8) & 0xff;
    buf[8] = crc & 0xff;
#ifdef _DEBUG_ 
    Serial.print("with CRC: 0x");
    Serial.print(buf[7],HEX);
    Serial.print(" ");
    Serial.print(buf[8],HEX);
    Serial.print(" ");
#endif    
    buf_size = 9;
  }
  
  else if(cmd == cmd_DeviceRead){
#ifdef _DEBUG_ 
    Serial.print("DeviceRead @ Adress ");
    Serial.print(addr_high,HEX);
    Serial.println(addr_low,HEX);
#endif
    buf[0] = cmd_Remote_Escape;
    buf[1] = cmd_DeviceRead;
    buf[2] = addr_high;
    buf[3] = addr_low;
    buf[4] = param;        // Output Param Lenght == data Length
    uint8_t ESC_data[4] = {CMD_SET_ADDRESS,0x00,addr_high,addr_low};
    uint16_t Data_Size = 4;
    uint16_t RX_Size = 0;
    uint8_t RX_Buf[300] = {0};
    ESC_CRC = true;
    //RX_Size = SendESC(ESC_data, RX_Buf, Data_Size);
    RX_Size = SendESC(ESC_data, Data_Size);
    delay(5);
    RX_Size = GetESC(RX_Buf, 200);
#ifdef _DEBUG_
    Serial.print("ESC Received: ");
    Serial.print(RX_Size);
    Serial.print(" Bytes: ");
    Serial.print(RX_Buf[0],HEX);
#endif
    if(RX_Buf[0] == brSUCCESS){
      // alles ok
      ESC_data[0] = CMD_READ_FLASH_SIL;
      ESC_data[1] = param;
      Data_Size = 2;
      //RX_Size = SendESC(ESC_data, RX_Buf, Data_Size);
      RX_Size = SendESC(ESC_data, Data_Size);
      if(param == 0){
        delay(256);
      }
      else{
        delay(param);
      }
      RX_Size = GetESC(RX_Buf, 500);
#ifdef _DEBUG_ 
      Serial.print("ESC Received: ");
      Serial.print(RX_Size);
      Serial.print(" Bytes: ");
      Serial.print(RX_Buf[(RX_Size-1)],HEX);
#endif
      if(RX_Buf[(RX_Size-1)] == brSUCCESS){
        //buf[6] = ACK_OK;    // ACK
      }
      else{
        //buf[6] = ACK_D_GENERAL_ERROR;    // ACK
      }
      RX_Size = RX_Size - 3;                              // CRC High, CRC_Low and ACK from ESC ->ToDo: check ACK and CRC in ESC Serial
      
      for(uint16_t i = 5; i<(RX_Size+5); i++){
        buf[i] = RX_Buf[i-5];   // buf[5] = RX_Buf[0]
        Data_Size = i;
      }
      Data_Size++;
      buf[Data_Size] = ACK_OK;    // ACK
      Data_Size++;
    }
    else{
      // nix ok
      buf[4] = 0x01;
      buf[5] = 0x00;
      buf[6] = ACK_D_GENERAL_ERROR;
      Data_Size = 7;
    }
    for(uint16_t i = 0; i < Data_Size; i++){
      crc = _crc_xmodem_update (crc, buf[i]);
    }
    buf[Data_Size] = (crc >> 8) & 0xff;
    Data_Size++;
    buf[Data_Size] = crc & 0xff;
#ifdef _DEBUG_ 
    Serial.print(" with CRC: 0x");
    Serial.print(buf[Data_Size-1],HEX);
    Serial.print(" ");
    Serial.print(buf[Data_Size],HEX);
    Serial.print(" ");
#endif    
    Data_Size++;
    buf_size = Data_Size;
  }
  
  else if(cmd == cmd_InterfaceExit){
#ifdef _DEBUG_ 
    Serial.print("Interface Exit ");
#endif     
    // Only interface itself, no matter what Device
    bool Enable4Way = false;
    // ToDo: close Serial connection to ESC reenable Servo interrupts
    // Serial1.end();
    buf[0] = cmd_Remote_Escape;
    buf[1] = cmd_InterfaceExit;
    buf[2] = addr_high;
    buf[3] = addr_low;
    buf[4] = 0x01;      // Output Param Lenght
    buf[5] = 0x00;      // Dummy
    buf[6] = ACK_OK;    // ACK
    for(uint8_t i = 0; i<7; i++){
      crc = _crc_xmodem_update (crc, buf[i]);
    }
    buf[7] = (crc >> 8) & 0xff;
    buf[8] = crc & 0xff;
#ifdef _DEBUG_ 
    Serial.print("with CRC: 0x");
    Serial.print(buf[7],HEX);
    Serial.print(" ");
    Serial.print(buf[8],HEX);
    Serial.print(" ");
#endif  
    buf_size = 9;
  }
  
  else if(cmd == cmd_ProtocolGetVersion){
#ifdef _DEBUG_     
    Serial.print("ProtocolGetVersion ");
#endif 
    // Only interface itself, no matter what Device
    buf[0] = cmd_Remote_Escape;//escape;
    buf[1] = cmd_ProtocolGetVersion;
    buf[2] = addr_high;
    buf[3] = addr_low;
    buf[4] = 0x01;      // Output Param Lenght
    buf[5] = SERIAL_4WAY_PROTOCOL_VER;//0x6C;      
    buf[6] = ACK_OK;//0x00;      // ACK
    for(uint8_t i = 0; i<7; i++){
      crc = _crc_xmodem_update (crc, buf[i]);
    }
    buf[7] = (crc >> 8) & 0xff;
    buf[8] = crc & 0xff;
#ifdef _DEBUG_     
    Serial.print("with CRC: 0x");
    Serial.print(buf[7],HEX);
    Serial.print(" ");
    Serial.print(buf[8],HEX);
    Serial.print(" ");
#endif 
    buf_size = 9;
  }
  
  else if(cmd == cmd_InterfaceGetName){
#ifdef _DEBUG_ 
    Serial.print("InterfaceGetName ");
#endif
    // Only interface itself, no matter what Device
    // SERIAL_4WAY_INTERFACE_NAME_STR "m4wFCIntf"
    buf[0] = cmd_Remote_Escape;
    buf[1] = cmd_InterfaceGetName;
    buf[2] = addr_high;
    buf[3] = addr_low;
    buf[4] = 0x09;        // Output Param Lenght
    buf[5] = 'm';//0x6D;
    buf[6] = '4';//0x34;
    buf[7] = 'w';//0x77;
    buf[8] = 'F';//0x46;
    buf[9] = 'C';//0x43;
    buf[10] = 'I';//0x49;
    buf[11] = 'n';//0x6E;
    buf[12] = 't';//0x74;
    buf[13] = 'f';//0x66;
    buf[14] = ACK_OK;     // ACK
    for(uint8_t i = 0; i<15; i++){
      crc = _crc_xmodem_update (crc, buf[i]);
    }
    buf[15] = (crc >> 8) & 0xff;
    buf[16] = crc & 0xff;
#ifdef _DEBUG_ 
    Serial.print("with CRC: 0x");
    Serial.print(buf[15],HEX);
    Serial.print(" ");
    Serial.print(buf[16],HEX);
    Serial.print(" ");
#endif
    buf_size = 17;
  }
  
  else if(cmd == cmd_InterfaceGetVersion){
#ifdef _DEBUG_ 
    Serial.print("InterfaceGetVersion ");
#endif
    // Only interface itself, no matter what Device
    buf[0] = cmd_Remote_Escape;
    buf[1] = cmd_InterfaceGetVersion;
    buf[2] = addr_high;
    buf[3] = addr_low;
    buf[4] = 0x02;        // Output Param Lenght
    buf[5] = SERIAL_4WAY_VERSION_HI;//0xC8;
    buf[6] = SERIAL_4WAY_VERSION_LO;//0x04;
    buf[7] = ACK_OK;      // ACK
    for(uint8_t i = 0; i<8; i++){
      crc = _crc_xmodem_update (crc, buf[i]);
    }
    buf[8] = (crc >> 8) & 0xff;
    buf[9] = crc & 0xff;
#ifdef _DEBUG_ 
    Serial.print("with CRC: 0x");
    Serial.print(buf[8],HEX);
    Serial.print(" ");
    Serial.print(buf[9],HEX);
    Serial.print(" ");
#endif
    buf_size = 10;
  }
  else if(cmd == cmd_InterfaceSetMode){
#ifdef _DEBUG_ 
    Serial.print("InterfaceSetMode ");
#endif
    buf[0] = cmd_Remote_Escape;
    buf[1] = cmd_InterfaceSetMode;
    buf[2] = addr_high;
    buf[3] = addr_low;
    buf[4] = 0x01;        // Output Param Lenght
    buf[5] = 0x00;        // Dummy
    if(param == imARM_BLB){
      buf[6] = ACK_OK;      // ACK
    }
    else{
      buf[6] = ACK_I_INVALID_PARAM;      // ACK
    }
    for(uint8_t i = 0; i<7; i++){
      crc = _crc_xmodem_update (crc, buf[i]);
    }
    buf[7] = (crc >> 8) & 0xff;
    buf[8] = crc & 0xff;
#ifdef _DEBUG_ 
    Serial.print("with CRC: 0x");
    Serial.print(buf[7],HEX);
    Serial.print(" ");
    Serial.print(buf[8],HEX);
    Serial.print(" ");
#endif
    buf_size = 9;
  }
  else if(cmd == cmd_DeviceVerify){
#ifdef _DEBUG_ 
    Serial.print("DeviceVerify ");
#endif
    buf[0] = cmd_Remote_Escape;
    buf[1] = cmd_DeviceVerify;
    buf[2] = addr_high;
    buf[3] = addr_low;
    buf[4] = 0x01;        // Output Param Lenght
    buf[5] = 0x00;        // Dummy
    buf[6] = ACK_OK;      // ACK
    
    for(uint8_t i = 0; i<7; i++){
      crc = _crc_xmodem_update (crc, buf[i]);
    }
    buf[7] = (crc >> 8) & 0xff;
    buf[8] = crc & 0xff;
#ifdef _DEBUG_ 
    Serial.print("with CRC: 0x");
    Serial.print(buf[7],HEX);
    Serial.print(" ");
    Serial.print(buf[8],HEX);
    Serial.print(" ");
#endif
    buf_size = 9;
  }
  else if(cmd == cmd_DevicePageErase){
#ifdef _DEBUG_ 
    Serial.print("DevicePageErase ");
#endif
    buf[0] = cmd_Remote_Escape;
    buf[1] = cmd_DevicePageErase;
    buf[2] = addr_high;
    buf[3] = addr_low;
    buf[4] = 0x01;        // Output Param Lenght
    buf[5] = param;        // Dummy
    buf[6] = ACK_OK;      // ACK

    addr_high = (param << 2);
    addr_low = 0;
    
    // Send CMD Adress
    uint8_t ESC_data[4] = {CMD_SET_ADDRESS,0,addr_high,addr_low};
    uint16_t Data_Size = 4;
    uint16_t RX_Size = 0;
    uint8_t RX_Buf[250] = {0};
    ESC_CRC = true;
    //RX_Size = SendESC(ESC_data, RX_Buf, Data_Size);
    RX_Size = SendESC(ESC_data, Data_Size);
    delay(5);
    RX_Size = GetESC(RX_Buf, 200);
#ifdef _DEBUG_
    Serial.print("ESC Received: ");
    Serial.print(RX_Size);
    Serial.print(" Bytes: ");
    Serial.print(RX_Buf[0],HEX);
#endif
    if(RX_Buf[0] == brSUCCESS){
     //buf[6] = ACK_OK;    // ACK
    }
    else{
      buf[6] = ACK_D_GENERAL_ERROR;    // ACK
    }
    // Send Data
    ESC_data[0] = CMD_ERASE_FLASH;
    ESC_data[1] = 0x01;
    Data_Size = 2;
    RX_Size = 0;
    //RX_Size = SendESC(ESC_data, RX_Buf, Data_Size); // ToDo: ACK kommt erst nach 30ms
    RX_Size = SendESC(ESC_data, Data_Size);
    delay(50);
    RX_Size = GetESC(RX_Buf, 100);
#ifdef _DEBUG_
    Serial.print("ESC Received: ");
    Serial.print(RX_Size);
    Serial.print(" Bytes: ");
    Serial.print(RX_Buf[0],HEX);
#endif
    if(RX_Buf[0] == brSUCCESS){
      //buf[6] = ACK_OK;    // ACK
    }
    else{
      buf[6] = ACK_D_GENERAL_ERROR;    // ACK
    }
    
    for(uint8_t i = 0; i<7; i++){
      crc = _crc_xmodem_update (crc, buf[i]);
    }
    buf[7] = (crc >> 8) & 0xff;
    buf[8] = crc & 0xff;
#ifdef _DEBUG_ 
    Serial.print("with CRC: 0x");
    Serial.print(buf[7],HEX);
    Serial.print(" ");
    Serial.print(buf[8],HEX);
    Serial.print(" ");
#endif
    buf_size = 9;
  }
  else if(cmd == cmd_DeviceWrite){
#ifdef _DEBUG_ 
    Serial.print("DeviceWrite ");
#endif
    buf[0] = cmd_Remote_Escape;
    buf[1] = cmd_DeviceWrite;
    buf[2] = addr_high;
    buf[3] = addr_low;
    buf[4] = 0x01;        // Output Param Lenght
    buf[5] = 0x00;        // Dummy
    buf[6] = ACK_OK;      // ACK

    // Send CMD Adress
    uint8_t ESC_data[4] = {CMD_SET_ADDRESS,0,addr_high,addr_low};
    uint16_t Data_Size = 4;
    uint16_t RX_Size = 0;
    uint8_t RX_Buf[250] = {0};
    ESC_CRC = true;
    //RX_Size = SendESC(ESC_data, RX_Buf, Data_Size);
    RX_Size = SendESC(ESC_data, Data_Size);
    delay(50);
    RX_Size = GetESC(RX_Buf, 100);
#ifdef _DEBUG_
    Serial.print("ESC Received: ");
    Serial.print(RX_Size);
    Serial.print(" Bytes: ");
    Serial.print(RX_Buf[0],HEX);
#endif
    if(RX_Buf[0] == brSUCCESS){
      //buf[6] = ACK_OK;    // ACK
    }
    else{
      buf[6] = ACK_D_GENERAL_ERROR;    // ACK
    }
    // sende Buffer init
    ESC_data[0] = CMD_SET_BUFFER;
    ESC_data[1] = 0x00;
    ESC_data[2] = 0x00;
    ESC_data[3] = I_Param_Len;
    Data_Size = 4;
    RX_Size = 0;
    if(I_Param_Len == 0){
      ESC_data[2] = 0x01;
    }
    
    //RX_Size = SendESC(ESC_data, RX_Buf, Data_Size);
    RX_Size = SendESC(ESC_data, Data_Size);
    delay(5);  
    // Keine Anwort vom ESC

    // sende Buffer data
    RX_Size = SendESC(ParamBuf, I_Param_Len);
    //delay(I_Param_Len);
    delay(5);
    RX_Size = GetESC(RX_Buf, 200);
#ifdef _DEBUG_
    Serial.print("ESC Received: ");
    Serial.print(RX_Size);
    Serial.print(" Bytes: ");
    Serial.print(RX_Buf[0],HEX);
#endif
    if(RX_Buf[0] == brSUCCESS){
      //buf[6] = ACK_OK;    // ACK
    }
    else{
      buf[6] = ACK_D_GENERAL_ERROR;    // ACK
    }

    // sende write CMD
    ESC_data[0] = CMD_PROG_FLASH;
    ESC_data[1] = 0x01;
    Data_Size = 2;
    RX_Size = 0;
    //RX_Size = SendESC(ESC_data, RX_Buf, Data_Size);
    RX_Size = SendESC(ESC_data, Data_Size);
    delay(30);
    // brSUCCESS wird nicht sofort gesendet
    RX_Size = GetESC(RX_Buf, 100);
#ifdef _DEBUG_
    Serial.print("ESC Received: ");
    Serial.print(RX_Size);
    Serial.print(" Bytes: ");
    Serial.print(RX_Buf[0],HEX);
#endif
    if(RX_Buf[0] == brSUCCESS){
      //buf[6] = ACK_OK;    // ACK
    }
    else{
      buf[6] = ACK_D_GENERAL_ERROR;    // ACK
    }
    
    for(uint8_t i = 0; i<7; i++){
      crc = _crc_xmodem_update (crc, buf[i]);
    }
    buf[7] = (crc >> 8) & 0xff;
    buf[8] = crc & 0xff;
#ifdef _DEBUG_ 
    Serial.print("with CRC: 0x");
    Serial.print(buf[7],HEX);
    Serial.print(" ");
    Serial.print(buf[8],HEX);
    Serial.print(" ");
#endif
    buf_size = 9;
  }
  
  // ToDo: build Output buffer with crc
  /*
  buf[0] = cmd_Remote_Escape;
  buf[1] = cmd;
  buf[2] = addr_high;
  buf[3] = addr_low;
  buf[4] = O_Param_Len;        // Output Param Lenght

  for(uint8_t i = 0; i<5 ; i++){                // CRC Check of Header (CMD, Adress, Size
    crc = _crc_xmodem_update (crc, buf[i]);
  }
  uint8_t OutBuff = O_Param_Len;
  i = 0;
  do {                                          // CRC Check of Output Parameter Buffer
    crc = _crc_xmodem_update (crc, buf[i+5]);
    buf[i] = ParamBuf[i+5];
    i++;
    OutBuff--;
  } while (OutBuff > 0);
  i++;
  crc = _crc_xmodem_update (crc, ack);
  buf[i] = ack_out;
  i++;
  buf[i] = (crc >> 8) & 0xff;
  i++;
  buf[i] = crc & 0xff;
  i++;

  return i
  */
  
  return buf_size;
}

uint16_t _crc_xmodem_update (uint16_t crc, uint8_t data) {
        int i;

        crc = crc ^ ((uint16_t)data << 8);
        for (i=0; i < 8; i++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
        return crc;
}
