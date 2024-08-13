/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
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

#include <pico/stdlib.h>
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "FreeRTOS.h"
#include "task.h"

#include "tusb.h"

#include "probe_config.h"

#include "msg_protocol.h"
#include "test_jig.h"

TaskHandle_t uart_taskhandle;
TickType_t last_wake, interval = 100;
volatile TickType_t break_expiry;
volatile bool timed_break;

/* Max 1 FIFO worth of data */
static uint8_t tx_buf[64];
static uint8_t rx_buf[64];
// Actually s^-1 so 25ms
#define DEBOUNCE_MS 40
static uint debounce_ticks = 5;

#ifdef PROBE_UART_TX_LED
static volatile uint tx_led_debounce;
#endif

#ifdef PROBE_UART_RX_LED
static uint rx_led_debounce;
#endif


void cdc_uart_init(void)
{
    gpio_set_function(PROBE_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(PROBE_UART_RX, GPIO_FUNC_UART);
    gpio_set_pulls(PROBE_UART_TX, 1, 0);
    gpio_set_pulls(PROBE_UART_RX, 1, 0);
    uart_init(PROBE_UART_INTERFACE, PROBE_UART_BAUDRATE);
}


bool cdc_task(void)
{
	static int was_connected = 0;
	static uint cdc_tx_oe = 0;
	uint rx_len = 0;
	bool keep_alive = false;

	// Consume uart fifo regardless even if not connected
	while(uart_is_readable(PROBE_UART_INTERFACE) && (rx_len < sizeof(rx_buf)))
	{
		rx_buf[rx_len++] = uart_getc(PROBE_UART_INTERFACE);
	}

	if (tud_cdc_connected())
	{
		was_connected = 1;
		int written = 0;
		/* Implicit overflow if we don't write all the bytes to the host.
		* Also throw away bytes if we can't write... */
		if (rx_len)
		{
#ifdef PROBE_UART_RX_LED
			gpio_put(PROBE_UART_RX_LED, 1);
			rx_led_debounce = debounce_ticks;
#endif

			written = MIN(tud_cdc_write_available(), rx_len);
			if (rx_len > written)
			{
				cdc_tx_oe++;
			}

			if (written > 0)
			{
				tud_cdc_write(rx_buf, written);
				tud_cdc_write_flush();
			}
		}
		else
		{
#ifdef PROBE_UART_RX_LED
			if (rx_led_debounce)
				rx_led_debounce--;
			else
				gpio_put(PROBE_UART_RX_LED, 0);
#endif
		}

		/* Reading from a firehose and writing to a FIFO. */
		size_t watermark = MIN(tud_cdc_available(), sizeof(tx_buf));

		if (watermark > 0)
		{
			size_t tx_len;

#ifdef PROBE_UART_TX_LED
			gpio_put(PROBE_UART_TX_LED, 1);
			tx_led_debounce = debounce_ticks;
#endif
			/* Batch up to half a FIFO of data - don't clog up on RX */
			watermark = MIN(watermark, 16);
			tx_len = tud_cdc_read(tx_buf, watermark);

			tj_process_input_raw(tx_buf, tx_len);
		}
		else
		{
#ifdef PROBE_UART_TX_LED
			if (tx_led_debounce)
			{
				tx_led_debounce--;
			}
			else
			{
				gpio_put(PROBE_UART_TX_LED, 0);
			}
#endif
		}
		/* Pending break handling */
		if (timed_break)
		{
			if (((int)break_expiry - (int)xTaskGetTickCount()) < 0)
			{
				timed_break = false;
				uart_set_break(PROBE_UART_INTERFACE, false);
#ifdef PROBE_UART_TX_LED
				tx_led_debounce = 0;
#endif
			}
			else
			{
				keep_alive = true;
			}
		}
	}
	else if (was_connected)
	{
		tud_cdc_write_clear();
		uart_set_break(PROBE_UART_INTERFACE, false);
		timed_break = false;
		was_connected = 0;
#ifdef PROBE_UART_TX_LED
		tx_led_debounce = 0;
#endif
		cdc_tx_oe = 0;
	}

	return keep_alive;
}


void init_gpios(void)
{
    gpio_init(DUT_POWER_ENABLE_PIN);
    gpio_set_dir(DUT_POWER_ENABLE_PIN, GPIO_OUT);

    gpio_init(CURRENT_MEASURE_ENABLE_PIN);
    gpio_set_dir(CURRENT_MEASURE_ENABLE_PIN, GPIO_OUT);
}

#include "board_pico_config.h"
void init_adcs(void)
{
    adc_init();
    adc_gpio_init(ADC_CURRENT_READ_PIN);
    adc_gpio_init(ADC_3V3_READ_PIN);
    adc_gpio_init(ADC_5V_READ_PIN);
}


void write_to_cdc(uint8_t* buf, uint32_t buf_len)
{
	tud_cdc_write(buf, buf_len);
	tud_cdc_write_flush();
}


void write_to_uart(uint8_t* buf, uint32_t buf_len)
{
	tud_cdc_write(buf, buf_len);
	tud_cdc_write_flush();
}


void apply_voltage(bool enable)
{
	gpio_put(DUT_POWER_ENABLE_PIN, enable);
}


void enable_current_measure(bool enable)
{
	gpio_put(CURRENT_MEASURE_ENABLE_PIN, enable);
}


struct test_jig_ctx test_jig = {
	.write_to_host          = write_to_cdc,
	.write_to_dut           = write_to_uart,
	.apply_voltage          = apply_voltage,
	.enable_current_measure = enable_current_measure
};


void cdc_thread(void *ptr)
{
	init_gpios();
	init_adcs();
	tj_init(&test_jig);
	BaseType_t delayed;
	last_wake = xTaskGetTickCount();
	bool keep_alive;

	/* Threaded with a polling interval that scales according to linerate */
	while (1)
	{
		keep_alive = cdc_task();
		if (!keep_alive)
		{
			delayed = xTaskDelayUntil(&last_wake, interval);
			if (delayed == pdFALSE)
			{
				last_wake = xTaskGetTickCount();
			}
		}
	}
}


void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* line_coding)
{
  uart_parity_t parity;
  uint data_bits, stop_bits;
  /* Set the tick thread interval to the amount of time it takes to
   * fill up half a FIFO. Millis is too coarse for integer divide.
   */
  uint32_t micros = (1000 * 1000 * 16 * 10) / MAX(line_coding->bit_rate, 1);
  /* Modifying state, so park the thread before changing it. */
  vTaskSuspend(uart_taskhandle);
  interval = MAX(1, micros / ((1000 * 1000) / configTICK_RATE_HZ));
  debounce_ticks = MAX(1, configTICK_RATE_HZ / (interval * DEBOUNCE_MS));
  probe_info("New baud rate %ld micros %ld interval %lu\n",
                  line_coding->bit_rate, micros, interval);
  uart_deinit(PROBE_UART_INTERFACE);
  tud_cdc_write_clear();
  tud_cdc_read_flush();
  uart_init(PROBE_UART_INTERFACE, line_coding->bit_rate);

  switch (line_coding->parity) {
  case CDC_LINE_CODING_PARITY_ODD:
    parity = UART_PARITY_ODD;
    break;
  case CDC_LINE_CODING_PARITY_EVEN:
    parity = UART_PARITY_EVEN;
    break;
  default:
    probe_info("invalid parity setting %u\n", line_coding->parity);
    /* fallthrough */
  case CDC_LINE_CODING_PARITY_NONE:
    parity = UART_PARITY_NONE;
    break;
  }

  switch (line_coding->data_bits) {
  case 5:
  case 6:
  case 7:
  case 8:
    data_bits = line_coding->data_bits;
    break;
  default:
    probe_info("invalid data bits setting: %u\n", line_coding->data_bits);
    data_bits = 8;
    break;
  }

  /* The PL011 only supports 1 or 2 stop bits. 1.5 stop bits is translated to 2,
   * which is safer than the alternative. */
  switch (line_coding->stop_bits) {
  case CDC_LINE_CONDING_STOP_BITS_1_5:
  case CDC_LINE_CONDING_STOP_BITS_2:
    stop_bits = 2;
  break;
  default:
    probe_info("invalid stop bits setting: %u\n", line_coding->stop_bits);
    /* fallthrough */
  case CDC_LINE_CONDING_STOP_BITS_1:
    stop_bits = 1;
  break;
  }

  uart_set_format(PROBE_UART_INTERFACE, data_bits, stop_bits, parity);
  vTaskResume(uart_taskhandle);
}


void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
  /* CDC drivers use linestate as a bodge to activate/deactivate the interface.
   * Resume our UART polling on activate, stop on deactivate */
  if (!dtr && !rts) {
    vTaskSuspend(uart_taskhandle);
#ifdef PROBE_UART_RX_LED
    gpio_put(PROBE_UART_RX_LED, 0);
    rx_led_debounce = 0;
#endif
#ifdef PROBE_UART_TX_LED
    gpio_put(PROBE_UART_TX_LED, 0);
    tx_led_debounce = 0;
#endif
  } else
    vTaskResume(uart_taskhandle);
}


void tud_cdc_send_break_cb(uint8_t itf, uint16_t wValue) {
  switch(wValue) {
    case 0:
    uart_set_break(PROBE_UART_INTERFACE, false);
    timed_break = false;
#ifdef PROBE_UART_TX_LED
    tx_led_debounce = 0;
#endif
    break;
    case 0xffff:
    uart_set_break(PROBE_UART_INTERFACE, true);
    timed_break = false;
#ifdef PROBE_UART_TX_LED
    gpio_put(PROBE_UART_TX_LED, 1);
    tx_led_debounce = 1 << 30;
#endif
    break;
    default:
    uart_set_break(PROBE_UART_INTERFACE, true);
    timed_break = true;
#ifdef PROBE_UART_TX_LED
    gpio_put(PROBE_UART_TX_LED, 1);
    tx_led_debounce = 1 << 30;
#endif
    break_expiry = xTaskGetTickCount() + (wValue * (configTICK_RATE_HZ / 1000));
    break;
  }
}
