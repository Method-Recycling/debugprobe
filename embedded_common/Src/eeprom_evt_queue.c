#include "eeprom_evt_queue.h"
#include "eeprom.h"

#include <stddef.h>
#include <string.h>

/*
 * Queue in EEPROM:
 *
 * x = eeprom_evt_queue_t.addr
 * n = eeprom_evt_queue_t.evt_size
 *
 * x + 0: eeprom_cbuf_header_t.head, size: 2
 * x + 2: eeprom_cbuf_header_t.tail, size: 2
 * x + 4: eeprom_cbuf_header_t.full, size: 4
 * x + 8: eeprom_evt_queue_t.last_sent_evt, size: n
 * x + 8 + n + n * eeprom_cbuf_header_t.head: events, size: n
 */

#ifdef STM32L072xx
void eeprom_evt_queue_load(eeprom_evt_queue_t* evt_queue) {
	eeprom_cbuf_header_t cbuf_header;
	eeprom_read(evt_queue->addr, (uint8_t*)&cbuf_header, sizeof(eeprom_cbuf_header_t));

	evt_queue->cbuf.head 		= cbuf_header.head;
	evt_queue->cbuf.tail 		= cbuf_header.tail;
	evt_queue->cbuf.full 		= cbuf_header.full;
	evt_queue->cbuf.buffer 		= (uint8_t*)evt_queue->addr + sizeof(eeprom_cbuf_header_t) + evt_queue->evt_size;
	evt_queue->last_sent_evt 	= (uint8_t*)evt_queue->addr + sizeof(eeprom_cbuf_header_t);
}

uint8_t eeprom_evt_queue_save(eeprom_evt_queue_t* evt_queue) {
	eeprom_cbuf_header_t cbuf_header = { evt_queue->cbuf.head, evt_queue->cbuf.tail, evt_queue->cbuf.full };
	return eeprom_write(evt_queue->addr, (uint8_t*)&cbuf_header, sizeof(eeprom_cbuf_header_t));
}

uint8_t eeprom_evt_queue_clear(eeprom_evt_queue_t* evt_queue) {
	circular_buf_reset(&evt_queue->cbuf);
	return eeprom_evt_queue_save(evt_queue);
}

uint16_t eeprom_evt_queue_length(eeprom_evt_queue_t* evt_queue) {
	return circular_buf_size(&evt_queue->cbuf);
}

uint8_t eeprom_evt_queue_full(eeprom_evt_queue_t* evt_queue) {
	return circular_buf_full(&evt_queue->cbuf);
}

uint8_t eeprom_evt_queue_add(eeprom_evt_queue_t* evt_queue, uint8_t* evt) {
	uint32_t evt_addr = evt_queue->addr + sizeof(eeprom_cbuf_header_t) + evt_queue->evt_size + (evt_queue->cbuf.head * evt_queue->evt_size);

	if (eeprom_write(evt_addr, evt, evt_queue->evt_size)) {
		circular_buf_advance_pointer(&evt_queue->cbuf);
		return eeprom_evt_queue_save(evt_queue);
	}

	return 0;
}

uint8_t eeprom_evt_queue_get(eeprom_evt_queue_t* evt_queue, uint8_t* evt) {
	return circular_buf_get(&evt_queue->cbuf, evt, evt_queue->evt_size);
}

uint8_t eeprom_evt_queue_peek_n(eeprom_evt_queue_t* evt_queue, uint16_t index, uint8_t* evt) {
	return circular_buf_peek_n(&evt_queue->cbuf, index, evt, evt_queue->evt_size);
}

uint8_t eeprom_evt_queue_peek_last(eeprom_evt_queue_t* evt_queue, uint8_t* evt) {
	return circular_buf_peek_last(&evt_queue->cbuf, evt, evt_queue->evt_size);
}

uint8_t eeprom_evt_queue_save_last_evt(eeprom_evt_queue_t* evt_queue, uint8_t* evt) {
	return eeprom_write(evt_queue->addr + sizeof(eeprom_cbuf_header_t), evt, evt_queue->evt_size);
}

uint8_t eeprom_evt_queue_get_last_evt(eeprom_evt_queue_t* evt_queue, uint8_t* evt) {
	if (evt_queue->last_sent_evt != NULL) {
		memcpy(evt, evt_queue->last_sent_evt, evt_queue->evt_size);
		return 1;
	}

	return 0;
}
#endif
