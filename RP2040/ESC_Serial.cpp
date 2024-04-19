#include <Arduino.h>

#include "ESC_Serial.h" // ESC Serial Header
#include "Global.h"     // Global variables
#include "elapsedMillis.h"
#include <deque>

uint32_t offsetPioReceive, offsetPioTransmit;
pio_sm_config configPioReceive, configPioTransmit;
PIO escPassthroughPio;
uint8_t escPassthroughSm;
uint8_t currentPin = 255;
bool isTxEnabled   = false;

std::deque<uint8_t> escRxBuf;

uint16_t esc_crc = 0;

void pioSetProgram(uint offset, pio_sm_config c) {
	pio_sm_set_config(escPassthroughPio, escPassthroughSm, &c);
	pio_sm_exec(escPassthroughPio, escPassthroughSm, pio_encode_jmp(offset));
}

void pioResetESC() {
	pioSetProgram(offsetPioTransmit, configPioTransmit);
	delay(1);
	pio_sm_exec_wait_blocking(escPassthroughPio, escPassthroughSm, pio_encode_set(pio_pins, 0));
	delay(300);
	pio_sm_exec_wait_blocking(escPassthroughPio, escPassthroughSm, pio_encode_set(pio_pins, 1));
	pioSetProgram(offsetPioReceive, configPioReceive);
}

void changePin(uint8_t newPin) {
	sm_config_set_in_pins(&configPioReceive, newPin);
	sm_config_set_set_pins(&configPioReceive, newPin, 1);
	sm_config_set_jmp_pin(&configPioReceive, newPin);
	sm_config_set_out_pins(&configPioTransmit, newPin, 1);
	sm_config_set_set_pins(&configPioTransmit, newPin, 1);
	currentPin = newPin;
	if (isTxEnabled) {
		pioSetProgram(offsetPioTransmit, configPioTransmit);
	} else {
		pioSetProgram(offsetPioReceive, configPioReceive);
	}
}

void pioEnableTx() {
	pioSetProgram(offsetPioTransmit, configPioTransmit);
	isTxEnabled = true;
}
void pioDisableTx() {
	while (!pio_sm_is_tx_fifo_empty(escPassthroughPio, escPassthroughSm)) {
	}
	while (pio_sm_get_pc(escPassthroughPio, escPassthroughSm) != offsetPioTransmit + 2) {
	}
	pioSetProgram(offsetPioReceive, configPioReceive);
	isTxEnabled = false;
}

void InitSerialOutput() {
	Enable4Way = true;
}

void DeinitSerialOutput() {
	Enable4Way = false;
}

void pioWrite(uint8_t data) {
	pio_sm_put_blocking(escPassthroughPio, escPassthroughSm, data);
}

uint32_t pioAvailable() {
	return escRxBuf.size();
}
uint8_t pioRead() {
	uint8_t data = escRxBuf.front();
	escRxBuf.pop_front();
	return data;
}

void delayWhileRead(uint16_t ms) {
	elapsedMillis x = 0;
	do {
		if (pio_sm_get_rx_fifo_level(escPassthroughPio, escPassthroughSm)) {
			escRxBuf.push_back(pio_sm_get(escPassthroughPio, escPassthroughSm) >> 24);
		}
	} while (x < ms);
}
void delayMicrosWhileRead(uint16_t us) {
	elapsedMicros x = 0;
	do {
		if (pio_sm_get_rx_fifo_level(escPassthroughPio, escPassthroughSm)) {
			escRxBuf.push_back(pio_sm_get(escPassthroughPio, escPassthroughSm) >> 24);
		}
	} while (x < us);
}

void SendESC(uint8_t tx_buf[], uint16_t buf_size, bool CRC) {
	uint16_t i = 0;
	esc_crc    = 0;
	if (buf_size == 0) {
		buf_size = 256;
	}
	pioEnableTx();
	for (i = 0; i < buf_size; i++) {
		pioWrite(tx_buf[i]);
		esc_crc = ByteCrc(tx_buf[i], esc_crc);
	}
	if (CRC) {
		pioWrite(esc_crc & 0xff);
		pioWrite((esc_crc >> 8) & 0xff);
		buf_size = buf_size + 2;
	}
	pioDisableTx();
}

uint16_t GetESC(uint8_t rx_buf[], uint16_t wait_ms) {
	uint16_t i   = 0;
	esc_crc      = 0;
	bool timeout = false;
	while ((!pioAvailable()) && (!timeout)) {
		delayWhileRead(1);
		i++;
		if (i >= wait_ms) {
			timeout = true;
			return 0;
		}
	}
	i = 0;
	while (pioAvailable()) {
		rx_buf[i] = pioRead();
		i++;
		delayMicrosWhileRead(500);
	}
	return i;
}

uint16_t ByteCrc(uint8_t data, uint16_t crc) {
	uint8_t xb = data;
	for (uint8_t i = 0; i < 8; i++) {
		if (((xb & 0x01) ^ (crc & 0x0001)) != 0) {
			crc = crc >> 1;
			crc = crc ^ 0xA001;
		} else {
			crc = crc >> 1;
		}
		xb = xb >> 1;
	}
	return crc;
}
