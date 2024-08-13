#include "../Inc/helpers.h"
#include "test_helpers.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>


/*
    Helpers
*/
/* Unpackers */
bool test_unpack_u16() {
    uint8_t  data[2] = {0x01, 0x02};
    uint16_t idx     = 0;
    uint16_t u16     = unpack_u16(data, &idx);

    return (u16 == 0x0201) && (idx == 2);
}


bool test_unpack_i16() {
    uint8_t  data[2] = {0x01, 0x02};
    uint16_t idx     = 0;
    int16_t i16      = unpack_i16(data, &idx);

    return (i16 == 0x0201) && (idx == 2);
}


bool test_unpack_u24() {
    uint8_t  data[3] = {0x01, 0x02, 0x03};
    uint16_t idx     = 0;
    uint32_t u24     = unpack_u24(data, &idx);

    return (u24 == 0x030201) && (idx == 3);
    }


bool test_unpack_u32() {
    uint8_t  data[4] = {0x01, 0x02, 0x03, 0x04};
    uint16_t idx     = 0;
    uint32_t u32     = unpack_u32(data, &idx);

    return (u32 == 0x04030201) && (idx == 4);
}


bool test_unpack_i32() {
    uint8_t  data[4] = {0x01, 0x02, 0x03, 0x04};
    uint16_t idx     = 0;
    int32_t  i32     = unpack_i32(data, &idx);

    return (i32 == 0x04030201) && (idx == 4);
}


bool test_unpack_f32() {
    uint8_t  data[4] = {0xCD, 0xCC, 0x44, 0x41};
    uint16_t idx     = 0;
    float    f32     = unpack_f32(data, &idx);

    return (f32 == 12.3f) && (idx == 4);
}


bool test_unpack_f16() {
    uint8_t  data[2] = {0x01, 0x02};
    float    scale   = 100;
    uint16_t idx     = 0;
    float    f16     = unpack_f16(data, scale, &idx);

    return (f16 == 5.130000f) && (idx == 2);
}


bool test_unpack_ip() {
    uint32_t packed_ip       = 0xC3FB2514;
    char     unpacked_ip[16] = {0};
    char*    true_result     = "20.37.251.195";

    unpack_ip(packed_ip, unpacked_ip);

    return strcmp(unpacked_ip, true_result) == 0;
}


/* Packers */
bool test_pack_u16() {
    uint8_t  buf[2] = {0};
    uint16_t u16    = 0x0201;
    uint16_t idx    = 0;
    pack_u16(buf, u16, &idx);

    uint8_t  data[2] = {0x01, 0x02};

    bool success = true;
    for(uint8_t i = 0; i < 2; ++i) {
        if(data[i] != buf[i]) {
            success = false;
        }
    }

    if(idx != 2) {
        success = false;
    }

    return success;
}


bool test_pack_i16() {
    bool     success       = true;
    uint8_t  n_bytes       = 2;
    uint8_t  data[]        = {0x01, 0x02};
    uint8_t  buf[n_bytes];
    int16_t  u16           = 0x0201;
    uint16_t idx           = 0;

    pack_i16(buf, u16, &idx);

    for(uint8_t i = 0; i < n_bytes; ++i) {
        if(data[i] != buf[i]) {
            success = false;
        }
    }

    if(idx != 2) {
        success = false;
    }

    return success;
}


bool test_pack_u24() {
    bool     success       = true;
    uint8_t  n_bytes       = 3;
    uint8_t  data[]        = {0x01, 0x02, 0x03};
    uint8_t  buf[n_bytes];
    uint32_t u24           = 0x030201;
    uint16_t idx           = 0;

    pack_u24(buf, u24, &idx);

    for(uint8_t i = 0; i < n_bytes; ++i) {
        if(data[i] != buf[i]) {
            success = false;
        }
    }

    if(idx != n_bytes) {
        success = false;
    }

    return success;
}


bool test_pack_u32() {
    bool     success       = true;
    uint8_t  n_bytes       = 4;
    uint8_t  data[]        = {0x01, 0x02, 0x03, 0x04};
    uint8_t  buf[n_bytes];
    uint32_t u32           = 0x04030201;
    uint16_t idx           = 0;

    pack_u32(buf, u32, &idx);

    for(uint8_t i = 0; i < n_bytes; ++i) {
        if(data[i] != buf[i]) {
            success = false;
        }
    }

    if(idx != n_bytes) {
        success = false;
    }

    return success;
}


bool test_pack_i32() {
    uint16_t idx           = 0;
    uint8_t  n_bytes       = 4;
    uint8_t  result[]      = {0x01, 0x02, 0x03, 0x04};

    int32_t  i32           = 0x04030201;
    uint8_t  buf[n_bytes];

    pack_i32(buf, i32, &idx);

    return arrays_equal(buf, result, idx) && (n_bytes == idx);
}


bool test_pack_f32() {
    uint16_t idx           = 0;
    uint8_t  n_bytes       = 4;
    uint8_t  result[]      = {0xCD, 0xCC, 0x44, 0x41};

    float    f32           = 12.3;
    uint8_t  buf[n_bytes];

    pack_f32(buf, f32, &idx);
    return arrays_equal(buf, result, idx) && (n_bytes == idx);
}


bool test_pack_f16() {
    uint16_t idx           = 0;
    uint8_t  n_bytes       = 2;
    uint8_t  result[]      = {0x01, 0x00};

    uint8_t  buf[n_bytes];
    float    f16           = 1000.0f;
    float    scale         = 1e-3;

    pack_f16(buf, f16, scale, &idx);

    return arrays_equal(buf, result, idx) && (n_bytes == idx);
}


bool test_pack_sis_addr() {
    uint16_t idx           = 0;
    uint8_t  n_bytes       = 6;
    uint8_t  result[]      = {0x14, 0x25, 0xFB, 0xC3, 0x34, 0x12};

    uint8_t  buf[n_bytes];
    uint32_t ip            = 0xC3FB2514;
    uint16_t port          = 0x1234;

    pack_sis_addr(buf, ip, port, &idx);

    return arrays_equal(buf, result, idx) && (n_bytes == idx);
}


/* The others */
bool test_ip_str_to_ip_int() {
    char     ip_str[16] = "020.036.043.124";
    uint32_t ip_int     = 0x7C2B2414;

    return ip_int == ip_str_to_ip_int(ip_str);
}


bool test_hex_to_str() {
    uint8_t idx     = 0;
    uint8_t data[]  = {0x12, 0x34};
    char*   ans     = "1234";
    char*   hex_str = hex_to_str(data, 2);

    return strcmp(ans, hex_str) == 0;
}


bool test_compare_i32() {
    int32_t x = 1, y = 1;
    if(compare_i32(&x, &y) != 0) return false;

    x = 1; y = 2;
    if(compare_i32(&x, &y) != -1) return false;

    x = 2; y = 1;
    if(compare_i32(&x, &y) != 1) return false;

    return true;
}


bool test_mins_to_secs() {
    return mins_to_secs(1) == 60;
}


bool test_median_u16() {
    uint16_t vals_odd[]  = {1, 2, 3, 4, 5};
    uint16_t vals_even[] = {1, 2, 3, 4};

    if(median_u16(vals_odd, 5) != 3) return false;
    if(median_u16(vals_even, 4) != 3) return false;

    return true;
}


bool test_mean_u16() {
    uint16_t vals_int[]   = {1, 1, 1, 1};
    if(mean_u16(vals_int, 4) != 1)   return false;

    //mean_u16 throws way anything after the decimal
    uint16_t vals_float[] = {1, 2, 3, 4};
    if(mean_u16(vals_float, 4) != 2) return false;

    return true;
}


bool test_get_array_diff_u16() {
    uint16_t vals[] = {5, 0};
    return get_array_diff_u16(vals, 2) == 5;
}

