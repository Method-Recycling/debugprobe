#ifndef EEPROM_EVT_QUEUE_H_
#define EEPROM_EVT_QUEUE_H_

#include <stdint.h>

#include "circular_buffer.h"

#define EEPROM_EVT_QUEUE_HEADER_SIZE (8)

typedef struct {
	uint32_t 		addr;
	uint32_t 		evt_size;
	uint8_t* 		last_sent_evt;
	circular_buf_t 	cbuf;
} eeprom_evt_queue_t;

typedef struct {
	uint16_t head;
	uint16_t tail;
	uint32_t full;
} eeprom_cbuf_header_t;

void 		eeprom_evt_queue_load(eeprom_evt_queue_t* evt_queue);
uint8_t 	eeprom_evt_queue_save(eeprom_evt_queue_t* evt_queue);
uint8_t 	eeprom_evt_queue_clear(eeprom_evt_queue_t* evt_queue);
uint16_t 	eeprom_evt_queue_length(eeprom_evt_queue_t* evt_queue);
uint8_t 	eeprom_evt_queue_full(eeprom_evt_queue_t* evt_queue);
uint8_t 	eeprom_evt_queue_add(eeprom_evt_queue_t* evt_queue, uint8_t* evt);
uint8_t 	eeprom_evt_queue_get(eeprom_evt_queue_t* evt_queue, uint8_t* evt);
uint8_t 	eeprom_evt_queue_peek_n(eeprom_evt_queue_t* evt_queue, uint16_t index, uint8_t* evt);
uint8_t 	eeprom_evt_queue_peek_last(eeprom_evt_queue_t* evt_queue, uint8_t* evt);
uint8_t 	eeprom_evt_queue_save_last_evt(eeprom_evt_queue_t* evt_queue, uint8_t* evt);
uint8_t 	eeprom_evt_queue_get_last_evt(eeprom_evt_queue_t* evt_queue, uint8_t* evt);

#endif /* EEPROM_EVT_QUEUE_H_ */
