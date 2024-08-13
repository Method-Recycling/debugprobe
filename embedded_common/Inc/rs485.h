#ifndef RS485_H_
#define RS485_H_

#include <stdint.h>

#ifdef STM32G031xx
#include "stm32g0xx_hal.h"
#else
#include "stm32l0xx_hal.h"
#endif

typedef struct {
	void* 			handle;
	GPIO_TypeDef* 	de_port;
	GPIO_TypeDef* 	re_port;
	uint16_t		de_pin;
	uint16_t 		re_pin;
} rs485_ctx_t;

typedef struct {
	uint32_t src;
	uint32_t dst;
	uint8_t  msg_type;
	uint8_t	 msg_ver;
	uint8_t  device_type;
} rs485_msg_header_t;

#define RS485_BROADCAST_ADDR (0xFFFFFFFF)

void 				rs485_tx_mode(rs485_ctx_t* ctx);
void 				rs485_rx_mode(rs485_ctx_t* ctx);
void 				rs485_sleep(rs485_ctx_t* ctx);
HAL_StatusTypeDef 	rs485_transmit(rs485_ctx_t* ctx, uint8_t* data, uint16_t size);
HAL_StatusTypeDef 	rs485_receive(rs485_ctx_t* ctx, uint8_t* data);
void 				rs485_pack_header(rs485_msg_header_t* header, uint8_t* data, uint16_t* idx);
rs485_msg_header_t 	rs485_unpack_header(uint8_t* data, uint16_t* idx);

#endif /* RS485_H_ */
