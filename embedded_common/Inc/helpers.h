#ifndef HELPERS_H_
#define HELPERS_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define TESTING
#ifndef TESTING
#include "main.h"
#endif
#define IP_ADDR_N_BYTES (uint8_t)(4)

union Float {
    float    m_float;
    uint8_t  m_bytes[sizeof(float)];
	uint32_t m_word;
};

/* Unpackers */
uint16_t 	unpack_u16(uint8_t* data, uint16_t* idx);
int16_t 	unpack_i16(uint8_t* data, uint16_t* idx);
uint32_t 	unpack_u24(uint8_t* data, uint16_t* idx);
uint32_t 	unpack_u32(uint8_t* data, uint16_t* idx);
int32_t 	unpack_i32(uint8_t* data, uint16_t* idx);
float 		unpack_f32(uint8_t* data, uint16_t* idx);
float 		unpack_f16(uint8_t* data, float scale, uint16_t* idx);
void 		unpack_ip(uint32_t packed_ip, char* ip);

/* Packers */
void 		pack_u16(uint8_t* buf, uint16_t value, uint16_t* idx);
void 		pack_i16(uint8_t* buf, int16_t value, uint16_t* idx);
void 		pack_u24(uint8_t* buf, uint32_t value, uint16_t* idx);
void 		pack_u32(uint8_t* buf, uint32_t value, uint16_t* idx);
void 		pack_i32(uint8_t* buf, int32_t value, uint16_t* idx);
void 		pack_f32(uint8_t* buf, float value, uint16_t* idx);
void 		pack_f16(uint8_t* buf, float value, float scale, uint16_t* idx);
void 		pack_sis_addr(uint8_t* buf, uint32_t ip, uint16_t port, uint16_t* idx);

uint32_t    ip_str_to_ip_int(char* str); // convert the IP address string returned from SARA dns to an int
char* 		hex_to_str(uint8_t* data, uint8_t size);
int 		compare_i32(const void* a, const void* b);
uint32_t 	mins_to_secs(uint32_t mins);
uint8_t 	is_rfid_diff(uint8_t* new_uid, uint8_t new_size, uint8_t* old_uid, uint8_t old_size);
uint16_t 	median_u16(uint16_t* vals, uint8_t n);
uint16_t 	mean_u16(uint16_t* vals, uint8_t n);
uint16_t 	get_array_diff_u16(uint16_t* vals, uint8_t n);
#ifndef TESTING
uint32_t 	millis_since(uint32_t t);
#endif
#endif /* HELPERS_H_ */
