/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef BOARD_PICO_H_
#define BOARD_PICO_H_

#define PROBE_IO_RAW
#define PROBE_CDC_UART

// PIO config
#define PROBE_SM 0
#define PROBE_PIN_OFFSET 0
#define PROBE_PIN_SWCLK (PROBE_PIN_OFFSET + 0) // 2
#define PROBE_PIN_SWDIO (PROBE_PIN_OFFSET + 1) // 3
// Target reset config
#if false
#define PROBE_PIN_RESET 1
#endif

// UART config
#define PROBE_UART_INTERFACE uart1
#define PROBE_UART_BAUDRATE 115200
#define PROBE_UART_TX 20
#define PROBE_UART_RX 21

// Test jig pins
#define DUT_POWER_ENABLE_PIN 6
#define CURRENT_MEASURE_ENABLE_PIN 7
#define TESTJIG_MODE_PIN 10

#define ADC_CURRENT_READ_PIN 26
#define ADC_3V3_READ_PIN 27
#define ADC_5V_READ_PIN 28

#define PROBE_DAP_CONNECTED_LED 14
#define PROBE_DAP_RUNNING_LED 15
// #define PROBE_UART_RX_LED 8
// #define PROBE_UART_TX_LED 9

#define LOADCELL_UART_INTERFACE uart0
#define LOADCELL_UART_BAUDRATE 115200
#define LOADCELL_UART_TX 12
#define LOADCELL_UART_RX 13
#define LOADCELL_UART_RE 11

#define PROBE_USB_CONNECTED_LED 25

#define PROBE_PRODUCT_STRING "Test Jig V2"

#define TEST_JIG_VERSION_MAJOR    (2)
#define TEST_JIG_VERSION_MINOR    (0)
#define TEST_JIG_VERSION_PATCH    (0)

#endif
