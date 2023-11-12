# BlHeli-Passthrough
Arduino Library and Examples for BlHeli Passtrough with ESP32 or Atmega328P (Arduino UNO, Pro Mini, ...)


Sketch for BlHeli Passthrough without Flight Controller Just for BlHeli32 or AM32 and all other imARM_BLB ESC's


## ESP32
- Default Servo Signal Pin: GPIO16
- Please edit the #define ESC_RX in ESC_Serial.cpp to choose another Pin


### needed Libraries for ESP32 use
- Espressif ESP32: https://github.com/espressif/arduino-esp32
- NimBLE-Arduino: https://github.com/h2zero/NimBLE-Arduino
- ESPSoftwareserial: https://github.com/plerup/espsoftwareserial/

all Libraries are available in Arduino Library Manager

## Atmega328P
- Default Pins: Servo RX = GPIO11; Servo TX = GPIO 10
- connect Servo Signal to Servo RX (11)
- connect 1k Resistor between Servo RX (11) and Servo TX (10)

- Please edit the #define ESC_RX and ESC_TX in ESC_Serial.cpp to choose other Pins

You have to Change SoftwareSerial RX Buffer Size in:
- Windows: C:\Users\User\AppData\Local\Arduino15\packages\arduino\hardware\avr\1.6.23\libraries\SoftwareSerial\src
- MaxOS: /Users/{username}/Library/Arduino15/packages/arduino/hardware/avr/1.8.6/libraries/SoftwareSerial/src 

Change #define _SS_MAX_RX_BUFF 64 to 300 in SoftwareSerial.h

BlHeli Configurator Firmware Flashing doesn't work (Keep Alive Bug in Configurator) -> should be fixed already

## ESC
### BlHeli_32 https://github.com/bitdump/BLHeli/tree/master/BLHeli_32%20ARM
- BlHeliSuite32: https://drive.google.com/drive/folders/1Y1bUMnRRolmMD_lezL0FYd3aMBrNzCig
- BlHeli32 Android App: https://play.google.com/store/apps/details?id=org.blheli.BLHeli_32

### AlkaMotors_32 https://github.com/AlkaMotors/AM32-MultiRotor-ESC-firmware
- ESCConfigTool: https://github.com/AlkaMotors/AM32-MultiRotor-ESC-firmware/tree/master/Release/CONFIG%20TOOL
- BlHeliConfigurator 1.3.0: https://drive.google.com/file/d/16N_l4Ukb4IBh8jnZvUq8hfmHY9Nbazs3/view?usp=sharing

