#ifndef UART_DMA_H_
#define UART_DMA_H_

#include <stdio.h>

#ifdef STM32G031xx
#include "stm32g0xx_hal.h"
#else
#include "stm32l0xx_hal.h"
#endif

#define UART_DMA_TIMEOUT_MS	(10)

typedef struct {
	UART_HandleTypeDef* uart_handle;
	USART_TypeDef* 		uart_periph_handle;
	DMA_HandleTypeDef* 	uart_dma_handle;
	uint8_t 			ready;
    volatile uint8_t 	timeout_flag;
    uint16_t 			timeout_timer;
    uint16_t 			prev_CNDTR;
	uint8_t* 			buf_p;
	uint16_t 			buf_size;
	void 				(*rx_callback)(uint8_t*, uint16_t);
} uart_dma_ctx_t;

uint8_t uart_dma_start(uart_dma_ctx_t* ctx);
void 	uart_dma_reset(uart_dma_ctx_t* ctx);
void 	uart_dma_tick(uart_dma_ctx_t* ctx);
void 	uart_dma_idle(uart_dma_ctx_t* ctx);
void 	uart_dma_rx_complete_callback(uart_dma_ctx_t* ctx);

#endif /* UART_DMA_H_ */
