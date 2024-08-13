#include "helper_tests.h"

#include <assert.h>

#define ASSERT_TRUE(func, msg)  if(!func()) { printf(msg); return false;}
#define ASSERT_FALSE(func, msg) if(func())  { printf(msg); return false;}

bool test_helpers() {
    // Unpackers
    ASSERT_TRUE(test_unpack_u16,         "unpack_u16 fail\n")
    ASSERT_TRUE(test_unpack_i16,         "unpack_i16 fail\n")
    ASSERT_TRUE(test_unpack_u24,         "unpack_u24 fail\n")
    ASSERT_TRUE(test_unpack_u32,         "unpack_u32 fail\n")
    ASSERT_TRUE(test_unpack_i32,         "unpack_i32 fail\n")
    ASSERT_TRUE(test_unpack_f32,         "unpack_f32 fail\n")
    ASSERT_TRUE(test_unpack_f16,         "unpack_f16 fail\n")
    ASSERT_TRUE(test_unpack_ip,          "unpack_ip fail\n")

    // Packers
    ASSERT_TRUE(test_pack_u16,           "pack_u16 fail\n")
    ASSERT_TRUE(test_pack_i16,           "pack_i16 fail\n")
    ASSERT_TRUE(test_pack_u24,           "pack_u24 fail\n")
    ASSERT_TRUE(test_pack_u32,           "pack_u32 fail\n")
    ASSERT_TRUE(test_pack_i32,           "pack_i32 fail\n")
    ASSERT_TRUE(test_pack_f32,           "pack_f32 fail\n")
    ASSERT_TRUE(test_pack_f16,           "pack_f16 fail\n")
    ASSERT_TRUE(test_pack_sis_addr,      "pack_sis_addr fail\n")

    // The others
    ASSERT_TRUE(test_ip_str_to_ip_int,   "ip_str_to_ip_int fail\n")
    ASSERT_TRUE(test_hex_to_str,         "hex_to_str fail\n")
    ASSERT_TRUE(test_compare_i32,        "compare_i32 fail\n")
    ASSERT_TRUE(test_mins_to_secs,       "mins_to_secs fail\n")
    ASSERT_TRUE(test_median_u16,         "median_u16 fail\n")
    ASSERT_TRUE(test_mean_u16,           "mean_u16 fail\n")
    ASSERT_TRUE(test_get_array_diff_u16, "get_array_diff_u16 fail\n")

    return true;
}


int main()
{
    if(!test_helpers()) return 1;
    return 0;
}
