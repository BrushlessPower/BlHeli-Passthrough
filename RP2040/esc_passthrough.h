/*
 * Serial Communication
 * - for BlHeli
 */
#include "hardware/pio.h"
uint8_t processPassthrough(void);
void beginPassthrough(uint8_t *pins, uint8_t pinCount, PIO pio, int8_t sm);
void beginPassthrough(uint8_t *pins, uint8_t pinCount, PIO pio); // why the fuck are default parameters not allowed in the arduino IDE???
void beginPassthrough(uint8_t *pins, uint8_t pinCount); // why the fuck are default parameters not allowed in the arduino IDE???
void beginPassthrough(uint8_t pin, PIO pio, int8_t sm);
void beginPassthrough(uint8_t pin, PIO pio); // why the fuck are default parameters not allowed in the arduino IDE???
void beginPassthrough(uint8_t pin); // why the fuck are default parameters not allowed in the arduino IDE???
void endPassthrough();

extern uint8_t passthroughPins[8];
extern uint8_t escCount;
