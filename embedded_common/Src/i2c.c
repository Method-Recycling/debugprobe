#include "i2c.h"
#include <string.h>

uint8_t i2c_write(void* handle, uint8_t addr, uint8_t reg, uint8_t *bufp, uint16_t len) {
	uint8_t tx_buf[I2C_MAX_RX_TX_SIZE];

	tx_buf[0] = reg;

	if ((bufp != NULL) && (len != 0)) {
		memcpy(&tx_buf[1], bufp, len);
	}

	return HAL_I2C_Master_Transmit(handle, (uint16_t)addr << 1, tx_buf, 1 + len, 1000);
}

uint8_t i2c_read(void* handle, uint8_t addr, uint8_t reg, uint8_t *bufp, uint16_t len, uint8_t delay) {
	uint8_t tx_buf[1] = { reg };
	uint8_t result;

	result = HAL_I2C_Master_Transmit(handle, (uint16_t)addr << 1, tx_buf, 1, 1000);
	HAL_Delay(delay);
	result &= HAL_I2C_Master_Receive(handle, (uint16_t)addr << 1, bufp, len, 1000);

	return result;
}

uint8_t i2c_write_half_word_reg(void* handle, uint8_t addr, uint16_t reg, uint8_t *bufp, uint16_t len) {
	uint8_t tx_buf[I2C_MAX_RX_TX_SIZE];

	tx_buf[0] = (uint8_t)((reg >> 8) & 0xFF);
	tx_buf[1] = (uint8_t)(reg & 0xFF);

	if ((bufp != NULL) && (len != 0)) {
		memcpy(&tx_buf[2], bufp, len);
	}

	return HAL_I2C_Master_Transmit(handle, (uint16_t)addr << 1, tx_buf, 2 + len, 1000);
}

uint8_t i2c_read_half_word_reg(void* handle, uint8_t addr, uint16_t reg, uint8_t *bufp, uint16_t len, uint8_t delay) {
	uint8_t tx_buf[2] = { (uint8_t)((reg >> 8) & 0xFF), (uint8_t)(reg & 0xFF) };
	uint8_t result;

	result = HAL_I2C_Master_Transmit(handle, (uint16_t)addr << 1, tx_buf, 2, 1000);
	HAL_Delay(delay);
	result &= HAL_I2C_Master_Receive(handle, (uint16_t)addr << 1, bufp, len, 1000);

	return result;
}
