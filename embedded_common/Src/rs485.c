#include "rs485.h"
#include "helpers.h"

void rs485_tx_mode(rs485_ctx_t* ctx) {
	HAL_GPIO_WritePin(ctx->re_port, ctx->re_pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ctx->de_port, ctx->de_pin, GPIO_PIN_SET);
}

void rs485_rx_mode(rs485_ctx_t* ctx) {
	HAL_GPIO_WritePin(ctx->re_port, ctx->re_pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ctx->de_port, ctx->de_pin, GPIO_PIN_RESET);
}

void rs485_sleep(rs485_ctx_t* ctx) {
	HAL_GPIO_WritePin(ctx->re_port, ctx->re_pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ctx->de_port, ctx->de_pin, GPIO_PIN_RESET);
}

HAL_StatusTypeDef rs485_transmit(rs485_ctx_t* ctx, uint8_t* data, uint16_t size) {
	HAL_StatusTypeDef result;

	rs485_tx_mode(ctx);
	result = HAL_UART_Transmit(ctx->handle, data, size, 1000);
	rs485_rx_mode(ctx);

	return result;
}

HAL_StatusTypeDef rs485_receive(rs485_ctx_t* ctx, uint8_t* data) {
	HAL_StatusTypeDef result;

	rs485_rx_mode(ctx);
	result = HAL_UART_Receive(ctx->handle, data, 1, 1000);

	return result;
}

void rs485_pack_header(rs485_msg_header_t* header, uint8_t* data, uint16_t* idx) {
	data[(*idx)++] = header->msg_type;
	data[(*idx)++] = header->msg_ver;
	data[(*idx)++] = header->device_type;
	pack_u32(data, header->src, idx);
	pack_u32(data, header->dst, idx);
}

rs485_msg_header_t rs485_unpack_header(uint8_t* data, uint16_t* idx) {
	rs485_msg_header_t h;
	h.msg_type 		= data[(*idx)++];
	h.msg_ver 		= data[(*idx)++];
	h.device_type 	= data[(*idx)++];
	h.src 			= unpack_u32(data, idx);
	h.dst 			= unpack_u32(data, idx);
	return h;
}
