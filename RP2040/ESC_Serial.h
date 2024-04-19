
#ifdef USE_TINYUSB
#include <Adafruit_TinyUSB.h>
#endif

void InitSerialOutput(void);
void DeinitSerialOutput(void);
void SendESC(uint8_t tx_buf[], uint16_t buf_size, bool CRC = true);
uint16_t ByteCrc(uint8_t data, uint16_t crc);
uint16_t GetESC(uint8_t rx_buf[], uint16_t wait_ms);
void delayWhileRead(uint16_t ms);
void pioResetESC();
void changePin(uint8_t newPin);

extern uint32_t offsetPioReceive, offsetPioTransmit;
extern pio_sm_config configPioReceive, configPioTransmit;
extern PIO escPassthroughPio;
extern uint8_t escPassthroughSm;
extern uint8_t currentPin;