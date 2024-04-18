#include <Arduino.h>

#include "4Way.h"       // 4Way defines
#include "ESC_Serial.h" // ESC Serial Code
#include "Global.h"     // Global variables
#include "esc_passthrough.h"

uint8_t passthroughBreakout = false;

uint16_t Check_4Way(uint8_t buf[]) {
	uint8_t cmd           = buf[1];
	uint8_t addr_high     = buf[2];
	uint8_t addr_low      = buf[3];
	uint8_t I_Param_Len   = buf[4];
	uint8_t param         = buf[5]; // param = ParamBuf[0]
	uint8_t ParamBuf[256] = {0};    // Parameter Buffer
	uint16_t crc          = 0;
	uint16_t buf_size; // return Output Buffer Size -> O_Param_Len + Header + CRC
	uint8_t ack_out      = ACK_OK;
	uint16_t O_Param_Len = 0;

	for (uint8_t i = 0; i < 5; i++) { // CRC Check of Header (CMD, Adress, Size)
		crc = _crc_xmodem_update(crc, buf[i]);
	}
	uint8_t InBuff = I_Param_Len; // I_Param_Len = 0 -> 256Bytes
	uint16_t i     = 0;           // work counter
	do {                          // CRC Check of Input Parameter Buffer
		crc         = _crc_xmodem_update(crc, buf[i + 5]);
		ParamBuf[i] = buf[i + 5];
		i++;
		InBuff--;
	} while (InBuff != 0);
	uint16_t crc_in = ((buf[i + 5] << 8) | buf[i + 6]);

	if (crc_in != crc) {
		buf[0]      = cmd_Remote_Escape;
		buf[1]      = cmd;
		buf[2]      = addr_high;
		buf[3]      = addr_low;
		O_Param_Len = 0x01;
		buf[4]      = 0x01; // Output Param Lenght
		buf[5]      = 0x00; // Dummy
		ack_out     = ACK_I_INVALID_CRC;
		buf[6]      = ACK_I_INVALID_CRC; // ACK
		for (uint8_t i = 0; i < 7; i++) {
			crc = _crc_xmodem_update(crc, buf[i]);
		}
		buf[7]   = (crc >> 8) & 0xff;
		buf[8]   = crc & 0xff;
		buf_size = 9;
		if (cmd < 0x50) {
			return buf_size;
		}
	}

	crc     = 0;
	ack_out = ACK_OK;
	buf[5]  = 0;

	if (cmd == cmd_DeviceInitFlash) {
		O_Param_Len = 0x04;
		if (param < escCount) { // ESC Count
			changePin(passthroughPins[param]);
			uint8_t BootInit[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0x0D, 'B', 'L', 'H', 'e', 'l', 'i', 0xF4, 0x7D};
			uint8_t Init_Size   = 17;
			uint8_t RX_Size     = 0;
			uint8_t RX_Buf[250] = {0};
			// DeviceInitFlash hat die CRC bereits im Array enthalten, daher darf keine CRC gesendet werden
			SendESC(BootInit, Init_Size, false); // send without CRC
			delayWhileRead(50);
			// read Answer Format = BootMsg("471c") SIGNATURE_001, SIGNATURE_002, BootVersion, BootPages (,ACK = brSUCCESS = 0x30)
			RX_Size = GetESC(RX_Buf, 200);
			if (RX_Buf[RX_Size - 1] == brSUCCESS) {
				buf[5] = RX_Buf[5]; // Device Signature2?
				buf[6] = RX_Buf[4]; // Device Signature1?
				buf[7] = RX_Buf[3]; // "c"?
				buf[8] = imARM_BLB; // 4-Way Mode: ARMBLB = 0x04
				buf[9] = ACK_OK;    // ACK
			} else {
				buf[5]  = 0x06;      // Device Signature2?
				buf[6]  = 0x33;      // Device Signature1?
				buf[7]  = 0x67;      // "c"?
				buf[8]  = imARM_BLB; // Boot Pages?
				ack_out = ACK_D_GENERAL_ERROR;
				buf[9]  = ACK_D_GENERAL_ERROR; // ACK
			}
		} else {
			ack_out = ACK_I_INVALID_CHANNEL;
			buf[9]  = ACK_I_INVALID_CHANNEL; // ACK
		}
	}

	else if (cmd == cmd_DeviceReset) {
		O_Param_Len = 0x01;
		if (param < escCount) { // ESC Count
			changePin(passthroughPins[param]);
			if (Enable4Way) {
				buf[6]              = ACK_OK; // ACK
				uint8_t ESC_data[2] = {RestartBootloader, 0};
				uint16_t Data_Size  = 2;
				uint16_t RX_Size    = 0;
				SendESC(ESC_data, Data_Size);
				// Betaflight setzt ausgang Low -> wartet 300ms -> setzt Ausgang High
				pioResetESC();
				// Keine Antwort vom ESC -> trotzdem serial leeren
				uint8_t RX_Buf[5] = {0};
				RX_Size           = GetESC(RX_Buf, 50);
			} else {
				ack_out = ACK_D_GENERAL_ERROR;
				buf[6]  = ACK_D_GENERAL_ERROR;
			}
		} else {
			buf[5]  = 0x00;
			ack_out = ACK_I_INVALID_CHANNEL;
			buf[6]  = ACK_I_INVALID_CHANNEL; // ACK
		}
	}

	else if (cmd == cmd_InterfaceTestAlive) {
		O_Param_Len         = 0x01;
		uint8_t ESC_data[2] = {CMD_KEEP_ALIVE, 0};
		uint16_t Data_Size  = 2;
		uint16_t RX_Size    = 0;
		uint8_t RX_Buf[250] = {0};
		SendESC(ESC_data, Data_Size);
		delayWhileRead(5);
		RX_Size = GetESC(RX_Buf, 200);
	}

	else if (cmd == cmd_DeviceRead) {
		uint8_t ESC_data[4] = {CMD_SET_ADDRESS, 0x00, addr_high, addr_low};
		uint16_t Data_Size  = 4;
		uint16_t RX_Size    = 0;
		uint8_t RX_Buf[300] = {0};
		uint16_t esc_rx_crc = 0;
		SendESC(ESC_data, Data_Size);
		delayWhileRead(5);
		RX_Size = GetESC(RX_Buf, 200);
		if (RX_Buf[0] == brSUCCESS) {
			// alles ok
			ESC_data[0] = CMD_READ_FLASH_SIL;
			ESC_data[1] = param;
			Data_Size   = 2;
			SendESC(ESC_data, Data_Size);
			if (param == 0) {
				O_Param_Len = 256;
				delayWhileRead(256);
			} else {
				delayWhileRead(param);
				O_Param_Len = param;
			}
			RX_Size = GetESC(RX_Buf, 500);
			if (RX_Size != 0) {
				if (RX_Buf[(RX_Size - 1)] == brSUCCESS) {
				} else {
					ack_out = ACK_D_GENERAL_ERROR;
				}
				RX_Size     = RX_Size - 3; // CRC High, CRC_Low and ACK from ESC
				O_Param_Len = RX_Size;
				for (uint16_t i = 5; i < (RX_Size + 5); i++) {
					buf[i]     = RX_Buf[i - 5]; // buf[5] = RX_Buf[0]
					esc_rx_crc = ByteCrc(buf[i], esc_rx_crc);
					// Data_Size = i;
				}
				esc_rx_crc = ByteCrc(RX_Buf[(RX_Size)], esc_rx_crc);
				esc_rx_crc = ByteCrc(RX_Buf[(RX_Size + 1)], esc_rx_crc);
				if (esc_rx_crc == 0) {
				} else {
					ack_out     = ACK_D_GENERAL_ERROR;
					O_Param_Len = 0x01;
				}
			} else {
				ack_out     = ACK_D_GENERAL_ERROR;
				O_Param_Len = 0x01;
			}
		} else {
			// nix ok
			O_Param_Len = 0x01;
			ack_out     = ACK_D_GENERAL_ERROR;
		}
	}

	else if (cmd == cmd_InterfaceExit) {
		DeinitSerialOutput(); // initialisiert PPM IN/OUT
		O_Param_Len         = 0x01;
		passthroughBreakout = true;
	}

	else if (cmd == cmd_ProtocolGetVersion) {
		O_Param_Len = 0x01;
		buf[5]      = SERIAL_4WAY_PROTOCOL_VER; // 0x6C;
	}

	else if (cmd == cmd_InterfaceGetName) {
		// SERIAL_4WAY_INTERFACE_NAME_STR "m4wFCIntf"
		O_Param_Len = 0x09;
		buf[5]      = 'm'; // 0x6D;
		buf[6]      = '4'; // 0x34;
		buf[7]      = 'w'; // 0x77;
		buf[8]      = 'F'; // 0x46;
		buf[9]      = 'C'; // 0x43;
		buf[10]     = 'I'; // 0x49;
		buf[11]     = 'n'; // 0x6E;
		buf[12]     = 't'; // 0x74;
		buf[13]     = 'f'; // 0x66;
	}

	else if (cmd == cmd_InterfaceGetVersion) {
		O_Param_Len = 0x02;
		buf[5]      = SERIAL_4WAY_VERSION_HI; // 0xC8;
		buf[6]      = SERIAL_4WAY_VERSION_LO; // 0x04;
	}

	else if (cmd == cmd_InterfaceSetMode) {
		O_Param_Len = 0x01;
		if (param == imARM_BLB) {
		} else {
			buf[6]  = ACK_I_INVALID_PARAM; // ACK
			ack_out = ACK_I_INVALID_PARAM;
		}
	}

	else if (cmd == cmd_DeviceVerify) {
		O_Param_Len = 0x01;
	}

	else if (cmd == cmd_DevicePageErase) {
		O_Param_Len = 0x01;
		addr_high   = (param << 2);
		addr_low    = 0;
		// Send CMD Adress
		uint8_t ESC_data[4] = {CMD_SET_ADDRESS, 0, addr_high, addr_low};
		uint16_t Data_Size  = 4;
		uint16_t RX_Size    = 0;
		uint8_t RX_Buf[250] = {0};
		SendESC(ESC_data, Data_Size);
		delayWhileRead(5);
		RX_Size = GetESC(RX_Buf, 200);
		if (RX_Buf[0] == brSUCCESS) {
			// buf[6] = ACK_OK;    // ACK
		} else {
			ack_out = ACK_D_GENERAL_ERROR;
			buf[6]  = ACK_D_GENERAL_ERROR; // ACK
		}
		// Send Data
		ESC_data[0] = CMD_ERASE_FLASH;
		ESC_data[1] = 0x01;
		Data_Size   = 2;
		RX_Size     = 0;
		SendESC(ESC_data, Data_Size);
		delayWhileRead(50);
		RX_Size = GetESC(RX_Buf, 100);
		if (RX_Buf[0] == brSUCCESS) {
			// buf[6] = ACK_OK;    // ACK
		} else {
			ack_out = ACK_D_GENERAL_ERROR;
			buf[6]  = ACK_D_GENERAL_ERROR; // ACK
		}
	}

	else if (cmd == cmd_DeviceWrite) {
		O_Param_Len = 0x01;
		// Send CMD Adress
		uint8_t ESC_data[4] = {CMD_SET_ADDRESS, 0, addr_high, addr_low};
		uint16_t Data_Size  = 4;
		uint16_t RX_Size    = 0;
		uint8_t RX_Buf[250] = {0};
		SendESC(ESC_data, Data_Size);
		delayWhileRead(50);
		RX_Size = GetESC(RX_Buf, 100);
		if (RX_Buf[0] == brSUCCESS) {

		} else {
			ack_out = ACK_D_GENERAL_ERROR;
		}
		// sende Buffer init
		ESC_data[0] = CMD_SET_BUFFER;
		ESC_data[1] = 0x00;
		ESC_data[2] = 0x00;
		ESC_data[3] = I_Param_Len;
		Data_Size   = 4;
		RX_Size     = 0;
		if (I_Param_Len == 0) {
			ESC_data[2] = 0x01;
		}
		SendESC(ESC_data, Data_Size);
		delayWhileRead(5);
		// Keine Anwort vom ESC

		// sende Buffer data
		SendESC(ParamBuf, I_Param_Len);
		delayWhileRead(5);
		RX_Size = GetESC(RX_Buf, 200);
		if (RX_Buf[0] == brSUCCESS) {

		} else {
			ack_out = ACK_D_GENERAL_ERROR;
		}

		// sende write CMD
		ESC_data[0] = CMD_PROG_FLASH;
		ESC_data[1] = 0x01;
		Data_Size   = 2;
		RX_Size     = 0;
		SendESC(ESC_data, Data_Size);
		delayWhileRead(30);
		// brSUCCESS wird nicht sofort gesendet
		RX_Size = GetESC(RX_Buf, 100);
		if (RX_Buf[0] == brSUCCESS) {
		} else {
			ack_out = ACK_D_GENERAL_ERROR;
		}
	}

	else {
		buf_size = 0;
	}

	crc                  = 0;
	buf[0]               = cmd_Remote_Escape;
	buf[1]               = cmd;
	buf[2]               = addr_high;
	buf[3]               = addr_low;
	buf[4]               = O_Param_Len & 0xff; // Output Param Lenght
	buf[O_Param_Len + 5] = ack_out;
	// CRC
	for (uint16_t i = 0; i < (O_Param_Len + 6); i++) {
		crc = _crc_xmodem_update(crc, buf[i]);
	}
	buf[O_Param_Len + 6] = (crc >> 8) & 0xff;
	buf[O_Param_Len + 7] = crc & 0xff;
	buf_size             = (O_Param_Len + 8);

	return buf_size;
}

uint16_t _crc_xmodem_update(uint16_t crc, uint8_t data) {
	crc = crc ^ ((uint16_t)data << 8);
	for (int i = 0; i < 8; i++) {
		if (crc & 0x8000)
			crc = (crc << 1) ^ 0x1021;
		else
			crc <<= 1;
	}
	return crc;
}
