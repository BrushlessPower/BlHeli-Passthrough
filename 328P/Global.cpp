#include <Arduino.h>

uint8_t ble_rx[300] = {0};          // 
uint8_t serial_rx[300] = {0};

bool Enable4Way = false;
bool EnablePasstrough = false;
//bool ESC_OK = true;
bool ESC_CRC = true;
