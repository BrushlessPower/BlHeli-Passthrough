#include <Arduino.h>

#include "ESC_Serial.h" // ESC Serial function for Start/Stop Serial
#include "Global.h"     // Global Variables
#include "MSP.h"        // MSP Header
#include "esc_passthrough.h"

uint8_t MSP_Check(uint8_t MSP_buf[], uint8_t buf_size) {
	// For BlHeli App Communication, there are just MSP Request's
	// http://www.stefanocottafavi.com/msp-the-multiwii-serial-protocol/
	// Checksum is 8 bit XOR of Size,type, and Payload

	uint8_t MSP_OSize = 0;
	uint8_t MSP_type  = MSP_buf[4];
	uint8_t MSP_crc   = MSP_buf[buf_size - 1]; // For Request (buf_size-1) is always 5
	uint8_t crc;

	if (MSP_type == MSP_API_VERSION && MSP_crc == 0x01) { // MSP_API_VERSION
		MSP_buf[2] = 0x3E;                                // Response Header
		MSP_buf[3] = 0x03;                                // Size
		MSP_buf[4] = MSP_API_VERSION;
		MSP_buf[5] = MSP_PROTOCOL_VERSION;
		MSP_buf[6] = API_VERSION_MAJOR;
		MSP_buf[7] = API_VERSION_MINOR;
		MSP_OSize  = 8;
	}

	else if (MSP_type == MSP_FC_VARIANT && MSP_crc == 0x02) { // MSP_FC_VARIANT
		MSP_buf[2] = 0x3E;                                    // Response Header
		MSP_buf[3] = 0x04;                                    // Size
		MSP_buf[4] = MSP_FC_VARIANT;
		MSP_buf[5] = 'B'; // BETAFLIGHT_IDENTIFIER "B"
		MSP_buf[6] = 'P'; // BETAFLIGHT_IDENTIFIER "T"
		MSP_buf[7] = 'M'; // BETAFLIGHT_IDENTIFIER "F"
		MSP_buf[8] = 'C'; // BETAFLIGHT_IDENTIFIER "L"
		MSP_OSize  = 9;
	}

	else if (MSP_type == MSP_FC_VERSION && MSP_crc == 0x03) { // MSP_FC_VERSION
		MSP_buf[2] = 0x3E;                                    // Response Header
		MSP_buf[3] = 0x03;                                    // Size
		MSP_buf[4] = MSP_FC_VERSION;
		MSP_buf[5] = FC_VERSION_MAJOR;
		MSP_buf[6] = FC_VERSION_MINOR;
		MSP_buf[7] = FC_VERSION_PATCH_LEVEL;
		MSP_OSize  = 8;
	}

	else if (MSP_type == MSP_BOARD_INFO && MSP_crc == 0x04) { // MSP_BOARD_INFO
		MSP_buf[2]  = 0x3E;                                   // Response Header
		MSP_buf[3]  = 0x4F;                                   // Size
		MSP_buf[4]  = MSP_BOARD_INFO;
		MSP_buf[5]  = 'B';
		MSP_buf[6]  = 'P';
		MSP_buf[7]  = 'M';
		MSP_buf[8]  = 'C';
		MSP_buf[9]  = 0x00; // HW Revision
		MSP_buf[10] = 0x00; // HW Revision
		MSP_buf[11] = 0x00; // FC w/wo MAX7456
		MSP_buf[12] = 0x00; // targetCapabilities
		MSP_buf[13] = 0x14; // Name String length
		MSP_buf[14] = 'B';  // Name.....
		MSP_buf[15] = 'r';
		MSP_buf[16] = 'u';
		MSP_buf[17] = 's';
		MSP_buf[18] = 'h';
		MSP_buf[19] = 'l';
		MSP_buf[20] = 'e';
		MSP_buf[21] = 's';
		MSP_buf[22] = 's';
		MSP_buf[23] = 'P';
		MSP_buf[24] = 'o';
		MSP_buf[25] = 'w';
		MSP_buf[26] = 'e';
		MSP_buf[27] = 'r';
		MSP_buf[28] = ' ';
		MSP_buf[29] = 'M';
		MSP_buf[30] = 'C';
		MSP_buf[31] = 'T';
		MSP_buf[32] = 'R';
		MSP_buf[33] = 'L';
		MSP_buf[34] = 0x00;
		MSP_buf[35] = 0x00;
		MSP_buf[36] = 0x00;
		MSP_buf[37] = 0x00;
		MSP_buf[38] = 0x00;
		MSP_buf[39] = 0x00;
		MSP_buf[40] = 0x00;
		MSP_buf[41] = 0x00;
		MSP_buf[42] = 0x00;
		MSP_buf[43] = 0x00;
		MSP_buf[44] = 0x00;
		MSP_buf[45] = 0x00;
		MSP_buf[46] = 0x00;
		MSP_buf[47] = 0x00;
		MSP_buf[48] = 0x00;
		MSP_buf[49] = 0x00;
		MSP_buf[50] = 0x00;
		MSP_buf[51] = 0x00;
		MSP_buf[52] = 0x00;
		MSP_buf[53] = 0x00;
		MSP_buf[54] = 0x00;
		MSP_buf[55] = 0x00;
		MSP_buf[56] = 0x00;
		MSP_buf[57] = 0x00;
		MSP_buf[58] = 0x00;
		MSP_buf[59] = 0x00;
		MSP_buf[60] = 0x00;
		MSP_buf[61] = 0x00;
		MSP_buf[62] = 0x00;
		MSP_buf[63] = 0x00;
		MSP_buf[64] = 0x00;
		MSP_buf[65] = 0x00;
		MSP_buf[66] = 0x00;
		MSP_buf[67] = 0x00;
		MSP_buf[68] = 0x00;
		MSP_buf[69] = 0x00;
		MSP_buf[70] = 0x00;
		MSP_buf[71] = 0x00;
		MSP_buf[72] = 0x00;
		MSP_buf[73] = 0x00;
		MSP_buf[74] = 0x00; // getMcuTypeId
		MSP_buf[75] = 0x00; // configurationState
		MSP_buf[76] = 0x00; // Gyro
		MSP_buf[77] = 0x00; // Gyro
		MSP_buf[78] = 0x00; // configurationProblems
		MSP_buf[79] = 0x00; // configurationProblems
		MSP_buf[80] = 0x00; // configurationProblems
		MSP_buf[81] = 0x00; // configurationProblems
		MSP_buf[82] = 0x00; // SPI
		MSP_buf[83] = 0x00; // I2C
		MSP_OSize   = 84;
	}

	else if (MSP_type == MSP_BUILD_INFO && MSP_crc == 0x05) { // MSP_BUILD_INFO
		MSP_buf[2]  = 0x3E;                                   // Response Header
		MSP_buf[3]  = 0x1A;                                   // Size
		MSP_buf[4]  = MSP_BUILD_INFO;
		MSP_buf[5]  = 0x00;
		MSP_buf[6]  = 0x00;
		MSP_buf[7]  = 0x00;
		MSP_buf[8]  = 0x00;
		MSP_buf[9]  = 0x00;
		MSP_buf[10] = 0x00;
		MSP_buf[11] = 0x00;
		MSP_buf[12] = 0x00;
		MSP_buf[13] = 0x00;
		MSP_buf[14] = 0x00;
		MSP_buf[15] = 0x00;
		MSP_buf[16] = 0x00;
		MSP_buf[17] = 0x00;
		MSP_buf[18] = 0x00;
		MSP_buf[19] = 0x00;
		MSP_buf[20] = 0x00;
		MSP_buf[21] = 0x00;
		MSP_buf[22] = 0x00;
		MSP_buf[23] = 0x00;
		MSP_buf[24] = 0x00;
		MSP_buf[25] = 0x00;
		MSP_buf[26] = 0x00;
		MSP_buf[27] = 0x00;
		MSP_buf[28] = 0x00;
		MSP_buf[29] = 0x00;
		MSP_buf[30] = 0x00;
		MSP_OSize   = 31;
	}

	else if (MSP_type == MSP_STATUS && MSP_crc == 0x65) { // MSP_STATUS
		MSP_buf[2]      = 0x3E;                           // Response Header
		MSP_buf[3]      = 0x16;                           // Size
		MSP_buf[4]      = MSP_STATUS;
		MSP_buf[5]      = 0x00; // 0xFA;   //getTaskDeltaTime(TASK_GYROPID)
		MSP_buf[6]      = 0x00; // getTaskDeltaTime(TASK_GYROPID)
		MSP_buf[7]      = 0x00; // i2cGetErrorCounter()
		MSP_buf[8]      = 0x00; // i2cGetErrorCounter()
		MSP_buf[9]      = 0x00; // 0x23;    // sensors(SENSOR_ACC) | sensors(SENSOR_BARO) << 1 | sensors(SENSOR_MAG) << 2 | sensors(SENSOR_GPS) << 3 | sensors(SENSOR_RANGEFINDER) << 4 | sensors(SENSOR_GYRO) << 5
		MSP_buf[10]     = 0x00; // sensors(SENSOR_ACC) | sensors(SENSOR_BARO) << 1 | sensors(SENSOR_MAG) << 2 | sensors(SENSOR_GPS) << 3 | sensors(SENSOR_RANGEFINDER) << 4 | sensors(SENSOR_GYRO) << 5
		MSP_buf[11]     = 0x00; // 0x02;   // flightModeFlags
		MSP_buf[12]     = 0x00; // flightModeFlags
		MSP_buf[13]     = 0x00; // flightModeFlags
		MSP_buf[14]     = 0x00; // flightModeFlags
		MSP_buf[15]     = 0x00; // getCurrentPidProfileIndex()
		MSP_buf[16]     = 0x00; // 0x02;   // constrain(averageSystemLoadPercent, 0, 100)
		MSP_buf[17]     = 0x00; // constrain(averageSystemLoadPercent, 0, 100)
		MSP_buf[18]     = 0x00; // PID_PROFILE_COUNT
		MSP_buf[19]     = 0x00; // getCurrentControlRateProfileIndex()
		MSP_buf[20]     = 0x00; // byteCount -> flightModeFlags
		MSP_buf[21]     = 0x00; // 0x18;   // ARMING_DISABLE_FLAGS_COUNT
		MSP_buf[22]     = 0x00; // 0x04;   // armingDisableFlags
		MSP_buf[23]     = 0x00; // 0x01;   // armingDisableFlags
		MSP_buf[24]     = 0x00, // 0x10;   // armingDisableFlags
			MSP_buf[25] = 0x00; // armingDisableFlags
		MSP_buf[26]     = 0x00; // getRebootRequired()
		MSP_OSize       = 27;
	}

	else if (MSP_type == MSP_MOTOR_3D_CONFIG && MSP_crc == 0x7C) { // MSP_3D
		MSP_buf[2]  = 0x3E;
		MSP_buf[3]  = 0x06;
		MSP_buf[4]  = MSP_MOTOR_3D_CONFIG;
		MSP_buf[5]  = 0x00; // 0x7E;   // flight3DConfig()->deadband3d_low
		MSP_buf[6]  = 0x00; // 0x05;   // flight3DConfig()->deadband3d_low
		MSP_buf[7]  = 0x00; // 0xEA;   // flight3DConfig()->deadband3d_high
		MSP_buf[8]  = 0x00; // 0x05;   // flight3DConfig()->deadband3d_high
		MSP_buf[9]  = 0x00; // 0xB4;   // flight3DConfig()->neutral3d
		MSP_buf[10] = 0x00; // 0x05;    // flight3DConfig()->neutral3d
		MSP_OSize   = 11;
	}

	else if (MSP_type == MSP_MOTOR_CONFIG && MSP_crc == 0x83) { // MSP_MOTOR_CONFIG
		MSP_buf[2]  = 0x3E;
		MSP_buf[3]  = 0x0A;
		MSP_buf[4]  = MSP_MOTOR_CONFIG;
		MSP_buf[5]  = 0x2E;     // motorConfig()->minthrottle -> 1070
		MSP_buf[6]  = 0x04;     // motorConfig()->minthrottle -> 1070
		MSP_buf[7]  = 0xD0;     // motorConfig()->maxthrottle -> 2000
		MSP_buf[8]  = 0x07;     // motorConfig()->maxthrottle -> 2000
		MSP_buf[9]  = 0xE8;     // motorConfig()->mincommand -> 1000
		MSP_buf[10] = 0x03;     // motorConfig()->mincommand -> 1000
		MSP_buf[11] = escCount; // 0x04;   // getMotorCount()
		MSP_buf[12] = 0x00;     // 0x10;   // motorConfig()->motorPoleCount
		MSP_buf[13] = 0x00;     // 0x01;   // motorConfig()->dev.useDshotTelemetry
		MSP_buf[14] = 0x00;     // featureIsEnabled(FEATURE_ESC_SENSOR)
		MSP_OSize   = 15;
	}

	else if (MSP_type == MSP_MOTOR && MSP_crc == 0x68) { // MSP_MOTOR
		MSP_buf[2] = 0x3E;
		MSP_buf[3] = 0x10;
		MSP_buf[4] = MSP_MOTOR;
		// MSP_buf[5]  = 0xE8; // motorConvertToExternal(motor[i]) -> 1000
		// MSP_buf[6]  = 0x03; // motorConvertToExternal(motor[i]) -> 1000
		// MSP_buf[7]  = 0x00; // 0xE8;
		// MSP_buf[8]  = 0x00; // 0x03;
		// MSP_buf[9]  = 0x00; // 0xE8;
		// MSP_buf[10] = 0x00; // 0x03;
		// MSP_buf[11] = 0x00; // 0xE8;
		// MSP_buf[12] = 0x00; // 0x03;
		int m = 0;
		for (; m < escCount; m++) {
			// motor states are 1000 for all running motors, and 0 for all stopped motors
			MSP_buf[5 + (m * 2)] = 0xE8; // motorConvertToExternal(motor[i]) -> 1000
			MSP_buf[6 + (m * 2)] = 0x03; // motorConvertToExternal(motor[i]) -> 1000
		}
		for (; m < 4; m++) {
			MSP_buf[5 + (m * 2)] = 0x00; // 0xE8;
			MSP_buf[6 + (m * 2)] = 0x00; // 0x03;
		}
		MSP_buf[13] = 0x00;
		MSP_buf[14] = 0x00;
		MSP_buf[15] = 0x00;
		MSP_buf[16] = 0x00;
		MSP_buf[17] = 0x00;
		MSP_buf[18] = 0x00;
		MSP_buf[19] = 0x00;
		MSP_buf[20] = 0x00;
		MSP_OSize   = 21;
	}

	else if (MSP_type == MSP_FEATURE_CONFIG && MSP_crc == 0x24) { // MSP_FEATURE_CONFIG
		MSP_buf[2] = 0x3E;
		MSP_buf[3] = 0x04;
		MSP_buf[4] = MSP_FEATURE_CONFIG;
		MSP_buf[5] = 0x00; // 0x08;    // getFeatureMask()
		MSP_buf[6] = 0x00; // 0x04;    // getFeatureMask()
		MSP_buf[7] = 0x00; // 0x00;    // getFeatureMask()
		MSP_buf[8] = 0x00; // 0x30;    // getFeatureMask()
		MSP_OSize  = 9;
	}

	else if (MSP_type == MSP_BOXIDS && MSP_crc == 0x77) { // MSP_BOXIDS
		MSP_buf[2]  = 0x3E;
		MSP_buf[3]  = 0x18;
		MSP_buf[4]  = MSP_BOXIDS;
		MSP_buf[5]  = 0x00;
		MSP_buf[6]  = 0x00;
		MSP_buf[7]  = 0x00;
		MSP_buf[8]  = 0x00;
		MSP_buf[9]  = 0x00;
		MSP_buf[10] = 0x00;
		MSP_buf[11] = 0x00;
		MSP_buf[12] = 0x00;
		MSP_buf[13] = 0x00;
		MSP_buf[14] = 0x00;
		MSP_buf[15] = 0x00;
		MSP_buf[16] = 0x00;
		MSP_buf[17] = 0x00;
		MSP_buf[18] = 0x00;
		MSP_buf[19] = 0x00;
		MSP_buf[20] = 0x00;
		MSP_buf[21] = 0x00;
		MSP_buf[22] = 0x00;
		MSP_buf[23] = 0x00;
		MSP_buf[24] = 0x00;
		MSP_buf[25] = 0x00;
		MSP_buf[26] = 0x00;
		MSP_buf[27] = 0x00;
		MSP_buf[28] = 0x00;
		MSP_OSize   = 29;
	}

	else if (MSP_type == MSP_SET_4WAY_IF && MSP_crc == 0xF5) { // MSP_4wayInit
		MSP_buf[2] = 0x3E;
		MSP_buf[3] = 0x01;
		MSP_buf[4] = MSP_SET_4WAY_IF;
		MSP_buf[5] = escCount; // 0x04;    // get channel number, switch all motor lines HI, reply with the count of ESC found
		InitSerialOutput();
		MSP_OSize = 6;
	}

	else if (MSP_type == MSP_ADVANCED_CONFIG && MSP_crc == 0x5A) { // MSP_ADVACED_CONFIG
		MSP_buf[2]  = 0x3E;
		MSP_buf[3]  = 0x14;
		MSP_buf[4]  = MSP_ADVANCED_CONFIG;
		MSP_buf[5]  = 0x00; // 0x02;    // gyroConfig()->gyro_sync_denom
		MSP_buf[6]  = 0x00; // 0x01;    // pidConfig()->pid_process_denom
		MSP_buf[7]  = 0x00; // motorConfig()->dev.useUnsyncedPwm
		MSP_buf[8]  = 0x06; // motorConfig()->dev.motorPwmProtocol -> 0x06 = Dshot300 / 0x00 = Servo / 0x07 = Dshot600
		MSP_buf[9]  = 0xE0; // motorConfig()->dev.motorPwmRate -> 480 -> DShot alle 2.08ms
		MSP_buf[10] = 0x01; // motorConfig()->dev.motorPwmRate -> 480 -> DShot alle 2.08ms
		MSP_buf[11] = 0x00; // 0x90;    // motorConfig()->digitalIdleOffsetValue
		MSP_buf[12] = 0x00; // 0x01;    // motorConfig()->digitalIdleOffsetValue
		MSP_buf[13] = 0x00; // DEPRECATED: gyro_use_32kHz
		MSP_buf[14] = 0x00; // motorConfig()->dev.motorPwmInversion
		MSP_buf[15] = 0x00; // 0x02;    // gyroConfig()->gyro_to_use
		MSP_buf[16] = 0x00; // 0x00;    // gyroConfig()->gyro_high_fsr
		MSP_buf[17] = 0x00; // 0x30;    // gyroConfig()->gyroMovementCalibrationThreshold
		MSP_buf[18] = 0x00; // 0x7D;    // gyroConfig()->gyroCalibrationDuration
		MSP_buf[19] = 0x00; // 0x00;    // gyroConfig()->gyroCalibrationDuration
		MSP_buf[20] = 0x00; // gyroConfig()->gyro_offset_yaw
		MSP_buf[21] = 0x00; // gyroConfig()->gyro_offset_yaw
		MSP_buf[22] = 0x00; // 0x02;    // gyroConfig()->checkOverflow
		MSP_buf[23] = 0x00; // 0x06;    // systemConfig()->debug_mode
		MSP_buf[24] = 0x00; // 0x3C;    // DEBUG_COUNT
		MSP_OSize   = 25;
	}

	else if (MSP_type == MSP_UID && MSP_crc == 0xA0) { // MSP_UID
		MSP_buf[2]  = 0x3E;
		MSP_buf[3]  = 0x0C;
		MSP_buf[4]  = MSP_UID;
		MSP_buf[5]  = 0x00;
		MSP_buf[6]  = 0x00;
		MSP_buf[7]  = 0x00;
		MSP_buf[8]  = 0x00;
		MSP_buf[9]  = 0x00;
		MSP_buf[10] = 0x00;
		MSP_buf[11] = 0x00;
		MSP_buf[12] = 0x00;
		MSP_buf[13] = 0x00;
		MSP_buf[14] = 0x00;
		MSP_buf[15] = 0x00;
		MSP_buf[16] = 0x00;
		MSP_OSize   = 17;
	}

	crc = MSP_buf[3] ^ MSP_buf[4];
	for (uint8_t i = 5; i < MSP_OSize; i++) {
		crc = crc ^ MSP_buf[i];
	}
	MSP_buf[MSP_OSize] = crc;
	buf_size           = MSP_OSize + 1;

	return buf_size;
}
