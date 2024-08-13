#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>
#include "main.h"

#define I2C_MAX_RX_TX_SIZE (32)

uint8_t i2c_write(void* handle, uint8_t addr, uint8_t reg, uint8_t *bufp, uint16_t len);
uint8_t i2c_read(void* handle, uint8_t addr, uint8_t reg, uint8_t *bufp, uint16_t len, uint8_t delay);

uint8_t i2c_write_half_word_reg(void* handle, uint8_t addr, uint16_t reg, uint8_t *bufp, uint16_t len);
uint8_t i2c_read_half_word_reg(void* handle, uint8_t addr, uint16_t reg, uint8_t *bufp, uint16_t len, uint8_t delay);

#endif /* I2C_H_ */
