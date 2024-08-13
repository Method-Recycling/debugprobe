#include "uart_dma.h"

#include "log.h"

uint8_t uart_dma_start(uart_dma_ctx_t* ctx) {
	uart_dma_reset(ctx);

	HAL_StatusTypeDef status = HAL_UART_Abort(ctx->uart_handle);

	if (status != HAL_OK) {
		LOG_ERROR("failed to abort: 0x%02X", status);
		return 0;
	}

	status = HAL_UART_Receive_DMA(ctx->uart_handle, ctx->buf_p, ctx->buf_size);

	if (status != HAL_OK) {
		LOG_ERROR("rx dma err: 0x%02X", status);
		return 0;
	}

	ctx->ready = 1;

	return 1;
}

void uart_dma_reset(uart_dma_ctx_t* ctx) {
	ctx->ready = 0;
	ctx->timeout_flag = 0;
	ctx->timeout_timer = 0;
	ctx->prev_CNDTR = ctx->buf_size;
}

void uart_dma_tick(uart_dma_ctx_t* ctx) {
	if (!ctx->ready) {
		return;
	}

	if (ctx->timeout_timer == 1) {
		ctx->timeout_flag = 1;
		ctx->uart_dma_handle->XferCpltCallback(ctx->uart_dma_handle);
	}

	if (ctx->timeout_timer) {
		ctx->timeout_timer--;
	}
}

void uart_dma_idle(uart_dma_ctx_t* ctx) {
	if (__HAL_UART_GET_FLAG(ctx->uart_handle, UART_FLAG_IDLE) != RESET) {
		__HAL_UART_CLEAR_FLAG(ctx->uart_handle, UART_FLAG_IDLE);
		ctx->timeout_timer = UART_DMA_TIMEOUT_MS;
	}
}

void uart_dma_rx_complete_callback(uart_dma_ctx_t* ctx) {
	uint16_t start, length;
	uint16_t curr_CNDTR = __HAL_DMA_GET_COUNTER(ctx->uart_handle->hdmarx);

	// Ignore IDLE Timeout when the received characters exactly filled up the DMA buffer and DMA Rx Complete IT is generated,
	// but there is no new character during timeout
	if (ctx->timeout_flag && (curr_CNDTR == ctx->buf_size)) {
		ctx->timeout_flag = 0;
		return;
	}

	// Determine start position in DMA buffer based on previous CNDTR value
    start = (ctx->prev_CNDTR < ctx->buf_size) ? (ctx->buf_size - ctx->prev_CNDTR) : 0;

    if (ctx->timeout_flag) {
    	// Timeout event

		// Determine new data length based on previous DMA_CNDTR value:
		// * If previous CNDTR is less than DMA buffer size: there is old data in DMA buffer (from previous timeout) that has to be ignored.
		// * If CNDTR == DMA buffer size: entire buffer content is new and has to be processed.
		length = (ctx->prev_CNDTR < ctx->buf_size) ? (ctx->prev_CNDTR - curr_CNDTR) : (ctx->buf_size - curr_CNDTR);
		ctx->prev_CNDTR = curr_CNDTR;
		ctx->timeout_flag = 0;
	} else {
		// DMA rx complete event
		length = ctx->buf_size - start;
		ctx->prev_CNDTR = ctx->buf_size;
	}

	ctx->rx_callback(&ctx->buf_p[start], length);
}
