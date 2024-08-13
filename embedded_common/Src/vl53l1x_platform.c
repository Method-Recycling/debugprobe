
/* 
* This file is part of VL53L1 Platform 
* 
* Copyright (c) 2016, STMicroelectronics - All Rights Reserved 
* 
* License terms: BSD 3-clause "New" or "Revised" License. 
* 
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met: 
* 
* 1. Redistributions of source code must retain the above copyright notice, this 
* list of conditions and the following disclaimer. 
* 
* 2. Redistributions in binary form must reproduce the above copyright notice, 
* this list of conditions and the following disclaimer in the documentation 
* and/or other materials provided with the distribution. 
* 
* 3. Neither the name of the copyright holder nor the names of its contributors 
* may be used to endorse or promote products derived from this software 
* without specific prior written permission. 
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
* 
*/

#include "vl53l1x_platform.h"

#include "main.h"
#include "i2c.h"
#include "helpers.h"

#include <string.h>

extern I2C_HandleTypeDef hi2c1;

#define I2C_TIME_OUT_BASE   10
#define I2C_TIME_OUT_BYTE   1

uint8_t _I2CBuffer[256];

int _I2CWrite(uint16_t Dev, uint8_t *pdata, uint32_t count) {
    int status;
    int i2c_time_out = I2C_TIME_OUT_BASE + (count * I2C_TIME_OUT_BYTE);

    status = HAL_I2C_Master_Transmit(&hi2c1, Dev, pdata, count, i2c_time_out);
    if (status != HAL_OK) {
        //VL6180x_ErrLog("I2C error 0x%x %d len", dev->I2cAddr, len);
        //XNUCLEO6180XA1_I2C1_Init(&hi2c1);
    	//printf("i2c tx err: %d, %d\n", status, hi2c1.ErrorCode);
    }
    return status;
}

int _I2CRead(uint16_t Dev, uint8_t *pdata, uint32_t count) {
    int status;
    int i2c_time_out = I2C_TIME_OUT_BASE + (count * I2C_TIME_OUT_BYTE);

    status = HAL_I2C_Master_Receive(&hi2c1, Dev|1, pdata, count, i2c_time_out);
    if (status != HAL_OK) {
        //VL6180x_ErrLog("I2C error 0x%x %d len", dev->I2cAddr, len);
        //XNUCLEO6180XA1_I2C1_Init(&hi2c1);
    	//printf("i2c rx err: %d, %d\n", status, hi2c1.ErrorCode);
    }
    return status;
}

int8_t VL53L1_WriteMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count) {
    int status_int;

    if (count > sizeof(_I2CBuffer) - 1) {
        return -1;
    }

    _I2CBuffer[0] = index>>8;
    _I2CBuffer[1] = index&0xFF;
    memcpy(&_I2CBuffer[2], pdata, count);

    status_int = _I2CWrite(dev, _I2CBuffer, count + 2);
    if (status_int != 0) {
    	//printf("VL53L1_WriteMulti: i2c write fail\n");
    }

	return 0;
}

int8_t VL53L1_ReadMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count) {
    int32_t status_int;

    _I2CBuffer[0] = index>>8;
    _I2CBuffer[1] = index&0xFF;

    status_int = _I2CWrite(dev, _I2CBuffer, 2);
    if (status_int != 0) {
        //printf("VL53L1_ReadMulti: i2c write fail\n");
        goto done;
    }

    status_int = _I2CRead(dev, pdata, count);
    if (status_int != 0) {
    	//printf("VL53L1_ReadMulti: i2c read fail\n");
    }

done:
	return 0;
}

int8_t VL53L1_WrByte(uint16_t dev, uint16_t index, uint8_t data) {
    int32_t status_int;

    _I2CBuffer[0] = index>>8;
    _I2CBuffer[1] = index&0xFF;
    _I2CBuffer[2] = data;

    status_int = _I2CWrite(dev, _I2CBuffer, 3);
    if (status_int != 0) {
        //printf("VL53L1_WrByte: i2c write fail\n");
    }

	return 0;
}

int8_t VL53L1_WrWord(uint16_t dev, uint16_t index, uint16_t data) {
    int32_t status_int;

    _I2CBuffer[0] = index>>8;
    _I2CBuffer[1] = index&0xFF;
    _I2CBuffer[2] = data >> 8;
    _I2CBuffer[3] = data & 0x00FF;

    status_int = _I2CWrite(dev, _I2CBuffer, 4);
    if (status_int != 0) {
    	//printf("VL53L1_WrWord: i2c write fail\n");
    }

    return 0;
}

int8_t VL53L1_WrDWord(uint16_t dev, uint16_t index, uint32_t data) {
    int32_t status_int;

    _I2CBuffer[0] = index>>8;
    _I2CBuffer[1] = index&0xFF;
    _I2CBuffer[2] = (data >> 24) & 0xFF;
    _I2CBuffer[3] = (data >> 16) & 0xFF;
    _I2CBuffer[4] = (data >> 8)  & 0xFF;
    _I2CBuffer[5] = (data >> 0 ) & 0xFF;

    status_int = _I2CWrite(dev, _I2CBuffer, 6);
    if (status_int != 0) {
    	//printf("VL53L1_WrDWord: i2c write fail\n");
    }

    return 0;
}

int8_t VL53L1_RdByte(uint16_t dev, uint16_t index, uint8_t *data) {
    int32_t status_int;

	_I2CBuffer[0] = index>>8;
	_I2CBuffer[1] = index&0xFF;

    status_int = _I2CWrite(dev, _I2CBuffer, 2);
    if( status_int ){
    	//printf("VL53L1_RdByte: i2c write fail\n");
        goto done;
    }

    status_int = _I2CRead(dev, data, 1);
    if (status_int != 0) {
    	//printf("VL53L1_RdByte: i2c read fail\n");
    }

done:
    return 0;
}

int8_t VL53L1_RdWord(uint16_t dev, uint16_t index, uint16_t *data) {
    int32_t status_int;

	_I2CBuffer[0] = index>>8;
	_I2CBuffer[1] = index&0xFF;

    status_int = _I2CWrite(dev, _I2CBuffer, 2);
    if( status_int ){
    	//printf("VL53L1_RdWord: i2c write fail\n");
        goto done;
    }

    status_int = _I2CRead(dev, _I2CBuffer, 2);
    if (status_int != 0) {
    	//printf("VL53L1_RdWord: i2c read fail\n");
    }

    *data = ((uint16_t)_I2CBuffer[0]<<8) | (uint16_t)_I2CBuffer[1];

done:
    return 0;
}

int8_t VL53L1_RdDWord(uint16_t dev, uint16_t index, uint32_t *data) {
    int32_t status_int;

	_I2CBuffer[0] = index>>8;
	_I2CBuffer[1] = index&0xFF;

    status_int = _I2CWrite(dev, _I2CBuffer, 2);
    if( status_int ){
    	//printf("VL53L1_RdDWord: i2c write fail\n");
        goto done;
    }

    status_int = _I2CRead(dev, _I2CBuffer, 4);
    if (status_int != 0) {
    	//printf("VL53L1_RdDWord: i2c read fail\n");
    }

    *data = ((uint32_t)_I2CBuffer[0]<<24) | ((uint32_t)_I2CBuffer[1]<<16) | ((uint32_t)_I2CBuffer[2]<<8) | (uint32_t)_I2CBuffer[3];

done:
    return 0;
}

int8_t VL53L1_WaitMs(uint16_t dev, int32_t wait_ms){
	HAL_Delay(wait_ms);
	return 1;
}
