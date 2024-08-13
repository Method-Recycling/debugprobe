#ifndef MSG_PROTOCOL_H
#define MSG_PROTOCOL_H

#include <stdint.h>

#define PACKET_DELIM_FLAG	  	(0x7E) 	// Packet delimiter byte
#define PACKET_ESC_FLAG		  	(0x7D) 	// Packet escape byte
#define PACKET_ESC_VAL		  	(0x20) 	// Packet escape value (value XOR'd by this)

#define PACKET_OVERHEAD_SIZE 	(6)
#define PACKET_PAYLOAD_MAX 		(1024)	// Max payload size in bytes

#define BROADCAST_ADDRESS		(0xFFFFFFFF)

typedef struct {
	uint8_t 	rx_in_progress;
	uint8_t 	last_byte_was_esc;
	uint16_t 	rxd_size;
	uint8_t 	rxd_data[PACKET_PAYLOAD_MAX];
	void		(*rx_callback_func)(uint8_t*, uint16_t);
	void		(*rx_bad_crc_callback_func)(void);
	// void*		crc_handle;
} msg_protocol_ctx_t;

void 		msg_protocol_process(msg_protocol_ctx_t* ctx, uint8_t value);
uint32_t 	msg_protocol_pack(msg_protocol_ctx_t* ctx, uint8_t* data, uint16_t data_size, uint8_t* buffer);

#endif /* MSG_PROTOCOL_H */
