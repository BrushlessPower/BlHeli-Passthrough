#include "esc_passthrough.h"

#define PIN_COUNT 4

void setup(){
  pinMode(LED_BUILTIN, OUTPUT);
  uint8_t pins[PIN_COUNT] = {10, 11, 12, 13}; // place your motor pins in this array, up to 8 motors
  // there are no limitations as to which pins you can use
  beginPassthrough(pins, PIN_COUNT);
  // alternative 1: beginPassthrough(10); will only start the passthrough on one pin (10 in this case)
  // alternative 2: beginPassthrough(pins, 4, pio0, 2); you can define the pio block and state machine that it will use, default are pio0 and an unused sm
  while(true){
    bool breakout = processPassthrough(); // the function returns true when you hit the "disconnect" button in BLHeliSuite32. You can ignore that if you want to be able to reconnect
    if (breakout) break;
  }
  endPassthrough(); // this restores the previous state of the pins, i.e. which peripheral they were assigned to. It will also free the sm and delete the pio program from this passthrough.
}

void loop(){
  // in this example, we blink the led after disconnect
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
