#ifndef TEST_HELPERS_H_
#define TEST_HELPERS_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* These are helpers for running tests, not tests for the helpers */

bool arrays_equal(uint8_t* arr_a, uint8_t* arr_b, uint16_t len) {
    bool success = true;

    for(uint8_t i = 0; i < len; ++i) {
        if(arr_a[i] != arr_b[i]) {
            success = false;
        }
    }

    return success;
}



#endif /* TEST_HELPERS_H_ */
