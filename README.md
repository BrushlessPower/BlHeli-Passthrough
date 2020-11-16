# BlHeli-Passthrough
Arduino Library and Examples for BlHeli Passtrough with ESP32


Sketch for BlHeli Passthrough without Flight Controller


## ESP32
Please edit the #define ESC_RX in ESC_Serial.cpp to choose your Pin where the ESC is connected

- Espressif ESP32: https://github.com/espressif/arduino-esp32
- NimBLE-Arduino: https://github.com/h2zero/NimBLE-Arduino
- ESPSoftwareserial: https://github.com/plerup/espsoftwareserial/

## Atmega328P
Please edit the #define ESC_RX and ESC_TX in ESC_Serial.cpp to choose your Pins where the ESC is connected
Connect a 1k Resistor between RX and TX
You have to Change SoftwareSerial RX Buffer Size in C:\Users\User\AppData\Local\Arduino15\packages\arduino\hardware\avr\1.6.23\libraries\SoftwareSerial\src
Change #define _SS_MAX_RX_BUFF 64 to 300 in SoftwareSerial.h

BlHeli Configurator Firmware Flashing doesn't work (Keep Alive Bug in Configurator)

## ESC
### BlHeli_32 https://github.com/bitdump/BLHeli/tree/master/BLHeli_32%20ARM
- BlHeliSuite32: https://drive.google.com/drive/folders/1Y1bUMnRRolmMD_lezL0FYd3aMBrNzCig
- BlHeli32 Android App: https://play.google.com/store/apps/details?id=org.blheli.BLHeli_32

### AlkaMotors_32 https://github.com/AlkaMotors/AM32-MultiRotor-ESC-firmware
- ESCConfigTool: https://github.com/AlkaMotors/AM32-MultiRotor-ESC-firmware/tree/master/Release/CONFIG%20TOOL
- BlHeliConfigurator 1.3.0: https://drive.google.com/file/d/16N_l4Ukb4IBh8jnZvUq8hfmHY9Nbazs3/view?usp=sharing

