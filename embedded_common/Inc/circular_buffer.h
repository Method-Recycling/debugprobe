#ifndef CIRCULAR_BUFFER_H_
#define CIRCULAR_BUFFER_H_

#include <stdint.h>

typedef struct {
    uint8_t* buffer;
    uint16_t head;
    uint16_t tail;
    uint16_t max;
    uint8_t  full;
} circular_buf_t;

void 		circular_buf_reset(circular_buf_t* c_buf);
uint8_t 	circular_buf_full(circular_buf_t* c_buf);
uint8_t 	circular_buf_empty(circular_buf_t* c_buf);
uint16_t 	circular_buf_size(circular_buf_t* c_buf);
void 		circular_buf_advance_pointer(circular_buf_t* c_buf);
void 		circular_buf_retreat_pointer(circular_buf_t* c_buf);
void 		circular_buf_put(circular_buf_t* c_buf, void* data, uint16_t size);
uint8_t 	circular_buf_get(circular_buf_t* c_buf, void* data, uint16_t size);
uint8_t 	circular_buf_peek_first(circular_buf_t* c_buf, void* data, uint16_t size);
uint8_t 	circular_buf_peek_last(circular_buf_t* c_buf, void* data, uint16_t size);
uint8_t		circular_buf_peek_n(circular_buf_t* c_buf, uint16_t index, void* data, uint16_t size);

#endif /* CIRCULAR_BUFFER_H_ */
