void InitSerialOutput(void);
uint16_t SendESC(uint8_t tx_buf[], uint16_t buf_size);
uint16_t ByteCrc(uint8_t data, uint16_t crc);
uint16_t GetESC(uint8_t rx_buf[], uint16_t wait_ms );
