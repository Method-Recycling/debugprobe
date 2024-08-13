#include "circular_buffer.h"
#include <string.h>

void circular_buf_reset(circular_buf_t* c_buf) {
    c_buf->head = 0;
    c_buf->tail = 0;
    c_buf->full = 0;
}

uint8_t circular_buf_full(circular_buf_t* c_buf) {
    return c_buf->full;
}

uint8_t circular_buf_empty(circular_buf_t* c_buf) {
    return (!c_buf->full && (c_buf->head == c_buf->tail));
}

uint16_t circular_buf_size(circular_buf_t* c_buf) {
    uint16_t size = c_buf->max;

    if (!c_buf->full) {
        if (c_buf->head >= c_buf->tail) {
            size = (c_buf->head - c_buf->tail);
        } else {
            size = (c_buf->max + c_buf->head - c_buf->tail);
        }
    }

    return size;
}

void circular_buf_advance_pointer(circular_buf_t* c_buf) {
    if (c_buf->full) {
        c_buf->tail = (c_buf->tail + 1) % c_buf->max;
    }

    c_buf->head = (c_buf->head + 1) % c_buf->max;
    c_buf->full = (c_buf->head == c_buf->tail);
}

void circular_buf_retreat_pointer(circular_buf_t* c_buf) {
    c_buf->full = 0;
    c_buf->tail = (c_buf->tail + 1) % c_buf->max;
}

void circular_buf_put(circular_buf_t* c_buf, void* data, uint16_t size) {
    memcpy(&c_buf->buffer[c_buf->head * size], data, size);
    circular_buf_advance_pointer(c_buf);
}

uint8_t _circular_buf_get(circular_buf_t* c_buf, void* data, uint16_t size, uint16_t index, uint8_t remove) {
    if (!circular_buf_empty(c_buf)) {
        memcpy(data, &c_buf->buffer[index * size], size);
        if (remove) {
        	circular_buf_retreat_pointer(c_buf);
        }
        return 1;
    }

    return 0;
}

uint8_t circular_buf_get(circular_buf_t* c_buf, void* data, uint16_t size) {
    return _circular_buf_get(c_buf, data, size, c_buf->tail, 1);
}

uint8_t circular_buf_peek_first(circular_buf_t* c_buf, void* data, uint16_t size) {
	return _circular_buf_get(c_buf, data, size, c_buf->tail, 0);
}

uint8_t circular_buf_peek_last(circular_buf_t* c_buf, void* data, uint16_t size) {
	return _circular_buf_get(c_buf, data, size, (c_buf->head == 0) ? (c_buf->max - 1) : (c_buf->head - 1), 0);
}

uint8_t	circular_buf_peek_n(circular_buf_t* c_buf, uint16_t index, void* data, uint16_t size) {
	return _circular_buf_get(c_buf, data, size, (c_buf->tail + index) % c_buf->max, 0);
}
