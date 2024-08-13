#include "helpers.h"

#include <stdio.h>
#include <math.h>

uint16_t unpack_u16(uint8_t* data, uint16_t* idx) {
	uint16_t val = ((uint16_t)data[(*idx) + 0] << 0) | \
				   ((uint16_t)data[(*idx) + 1] << 8);
	*idx += 2;
	return val;
}

int16_t unpack_i16(uint8_t* data, uint16_t* idx) {
	int16_t val = ((int16_t)data[(*idx) + 0] << 0) | \
				  ((int16_t)data[(*idx) + 1] << 8);
	*idx += 2;
	return val;
}

uint32_t unpack_u24(uint8_t* data, uint16_t* idx) {
	uint32_t val = ((uint32_t)data[(*idx) + 0] << 0) | \
				   ((uint32_t)data[(*idx) + 1] << 8) | \
				   ((uint32_t)data[(*idx) + 2] << 16);
	*idx += 3;
	return val;
}

uint32_t unpack_u32(uint8_t* data, uint16_t* idx) {
	uint32_t val = ((uint32_t)data[(*idx) + 0] << 0) | \
				   ((uint32_t)data[(*idx) + 1] << 8) | \
				   ((uint32_t)data[(*idx) + 2] << 16)  | \
				   ((uint32_t)data[(*idx) + 3] << 24);
	*idx += 4;
	return val;
}

int32_t unpack_i32(uint8_t* data, uint16_t* idx) {
	int32_t val = ((int32_t)data[(*idx) + 0] << 0) | \
				  ((int32_t)data[(*idx) + 1] << 8) | \
				  ((int32_t)data[(*idx) + 2] << 16)  | \
				  ((int32_t)data[(*idx) + 3] << 24);
	*idx += 4;
	return val;
}

float unpack_f32(uint8_t* data, uint16_t* idx) {
	union Float f;
	f.m_word = unpack_u32(data, idx);

	return f.m_float;
}

float unpack_f16(uint8_t* data, float scale, uint16_t* idx) {
	return ((float)unpack_i16(data, idx) / scale);
}

void pack_u16(uint8_t* buf, uint16_t value, uint16_t* idx) {
	buf[(*idx) + 0] = (uint8_t)((value >> 0) & 0xFF);
	buf[(*idx) + 1] = (uint8_t)((value >> 8) & 0xFF);
	*idx += 2;
}

void pack_i16(uint8_t* buf, int16_t value, uint16_t* idx) {
	buf[(*idx) + 0] = (uint8_t)((value >> 0) & 0xFF);
	buf[(*idx) + 1] = (uint8_t)((value >> 8) & 0xFF);
	*idx += 2;
}

void pack_u24(uint8_t* buf, uint32_t value, uint16_t* idx) {
	buf[(*idx) + 0] = (uint8_t)((value >> 0) & 0xFF);
	buf[(*idx) + 1] = (uint8_t)((value >> 8) & 0xFF);
	buf[(*idx) + 2] = (uint8_t)((value >> 16) & 0xFF);
	*idx += 3;
}

void pack_u32(uint8_t* buf, uint32_t value, uint16_t* idx) {
	buf[(*idx) + 0] = (uint8_t)((value >> 0) & 0xFF);
	buf[(*idx) + 1] = (uint8_t)((value >> 8) & 0xFF);
	buf[(*idx) + 2] = (uint8_t)((value >> 16) & 0xFF);
	buf[(*idx) + 3] = (uint8_t)((value >> 24) & 0xFF);
	*idx += 4;
}

void pack_i32(uint8_t* buf, int32_t value, uint16_t* idx) {
	buf[(*idx) + 0] = (uint8_t)((value >> 0) & 0xFF);
	buf[(*idx) + 1] = (uint8_t)((value >> 8) & 0xFF);
	buf[(*idx) + 2] = (uint8_t)((value >> 16) & 0xFF);
	buf[(*idx) + 3] = (uint8_t)((value >> 24) & 0xFF);
	*idx += 4;
}

void pack_f32(uint8_t* buf, float value, uint16_t* idx) {
	union Float f;
	f.m_float = value;
	pack_u32(buf, f.m_word, idx);
}

void pack_f16(uint8_t* buf, float value, float scale, uint16_t* idx) {
	pack_i16(buf, (int16_t)(value * scale), idx);
}

void pack_sis_addr(uint8_t* buf, uint32_t ip, uint16_t port, uint16_t* idx) {
	pack_u32(buf, ip, idx);
	pack_u16(buf, port, idx);
}

void unpack_ip(uint32_t packed_ip, char* ip) {
    uint8_t data[4];
    uint16_t size = 0;
    pack_u32(data, packed_ip, &size);

    sprintf(ip, "%d.%d.%d.%d", data[0], data[1], data[2], data[3]);
}

/*
 * Changes an IP address in the form "A.B.C.D" to 0xDCBA
 * i.e. 	20.36.43.124 = 0x7C2B2414
 */
uint32_t ip_str_to_ip_int(char* str)
{
    uint8_t ip[IP_ADDR_N_BYTES]  = {0};
    char *ch;
    ch = strtok(str, ".\""); // IP address returned from SARA DNS contains "", so we need to handle that as well as .

    uint8_t i = 0;


    // First get the ip address bytes
    while (ch != NULL)
    {
        ip[i] = atoi(ch);
        ch = strtok(NULL, ".\"");
		i++;

        if(i == IP_ADDR_N_BYTES)
        {
        	break;
        }
    }

    // Pack the ip addr bytes
    uint32_t ip_int = 0;
	for(i = 0; i < IP_ADDR_N_BYTES; i++)
	{
		ip_int |= ip[i] << (i * 8);
	}
    return ip_int;
}

char* hex_to_str(uint8_t* data, uint8_t size) {
    const char* HEX_STR = "0123456789ABCDEF";
    static char buf[16] = { 0 };
    uint8_t* pin = data;
    char* pout = buf;

    for (uint8_t i = 0; i < size; i++) {
        *pout++ = HEX_STR[(*pin >> 4) & 0x0F];
        *pout++ = HEX_STR[(*pin++) & 0x0F];
    }

    return buf;
}

int compare_i32(const void* a, const void* b) {
    int32_t x = *(const int32_t*)a;
    int32_t y = *(const int32_t*)b;

	if (x < y) {
		return -1;
	} else if (x > y) {
		return 1;
	}

	return 0;
}

uint32_t mins_to_secs(uint32_t mins) {
	return mins * 60;
}

#ifndef TESTING
uint32_t millis_since(uint32_t t) {
	uint32_t t_now = HAL_GetTick();

	if (t >= t_now) {
		return 0;
	}

	return (t_now - t);
}
#endif

uint8_t is_rfid_diff(uint8_t* new_uid, uint8_t new_size, uint8_t* old_uid, uint8_t old_size) {
	if (new_size != old_size) {
		return 1;
	}

	if ((new_size == 0) && (old_size == 0)) {
		return 0;
	}

	if (memcmp(new_uid, old_uid, new_size) != 0) {
		return 1;
	}

	return 0;
}

void swap(uint16_t *p, uint16_t *q) {
   uint32_t t = *p;
   *p = *q;
   *q = t;
}

void sort(uint16_t* a, uint16_t n) {
   for (uint8_t i = 0; i < (n - 1); i++) {
      for (uint8_t j = 0; j < (n - i - 1); j++) {
         if (a[j] > a[j + 1]) {
            swap(&a[j], &a[j + 1]);
         }
      }
   }
}

uint16_t median_u16(uint16_t* vals, uint8_t n) {
    sort(vals, n);

    uint8_t mid = n / 2;

    if ((n % 2) == 0) {
        return (uint16_t)roundf(((float)vals[mid] + vals[mid - 1]) / 2.f);
    }

    return vals[mid];
}

uint16_t mean_u16(uint16_t* vals, uint8_t n) {
	uint32_t mean = 0;

	for (uint8_t i = 0; i < n; i++) {
		mean += vals[i];
	}

	return (uint16_t)(mean / n);
}

uint16_t get_array_diff_u16(uint16_t* vals, uint8_t n) {
	uint16_t min = vals[0];
	uint16_t max = vals[0];

	for (uint8_t i = 1; i < n; i++) {
		if (vals[i] < min) {
			min = vals[i];
		} else if (vals[i] > max) {
			max = vals[i];
		}
	}

	return (max - min);
}
