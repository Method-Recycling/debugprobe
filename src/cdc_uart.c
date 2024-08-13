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
#include "FreeRTOS.h"
#include "task.h"

#include "tusb.h"

#include "probe_config.h"

#include "msg_protocol.h"

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

#define TEST_JIG_MSG_NONE 				(0x00)
#define TEST_JIG_MSG_ACK 				(0x01)
#define TEST_JIG_MSG_NACK 				(0x02)
#define TEST_JIG_MSG_ERR 				(0x03)
#define TEST_JIG_MSG_APPLY_4V2 			(0x04)
#define TEST_JIG_MSG_APPLY_VOLTAGE 		(0x05)
#define TEST_JIG_MSG_APPLY_LOW_VOLTAGE 	(0x06)
#define TEST_JIG_MSG_APPLY_REVERSE_POL  (0x07)
#define TEST_JIG_MSG_ENABLE_SARA_VUSB 	(0x08)
#define TEST_JIG_MSG_ENABLE_BYPASS 		(0x09)
#define TEST_JIG_MSG_GET_CURRENT 		(0x0A)
#define TEST_JIG_MSG_GET_VOLTAGES 		(0x0B)
#define TEST_JIG_MSG_GET_OC_4V2 		(0x0C)
#define TEST_JIG_MSG_GET_VERSION 		(0x0D)
#define TEST_JIG_MSG_ELMO_GET_WEIGHT 	(0x0E)
#define TEST_JIG_MSG_ELMO_GET_VERSION 	(0x0F)
#define TEST_JIG_MSG_RS485_TICKLE 		(0x10)
#define TEST_JIG_MSG_ENABLE_ADC_PROOF 	(0x11)
#define TEST_JIG_MSG_RESET 				(0xFE)
#define TEST_JIG_MSG_PASS_THRU_TO_DUT 	(0xFF)

#define TEST_JIG_TYPE_OSCAR 			(1)
#define TEST_JIG_TYPE_ERNIE 			(2)
#define TEST_JIG_TYPE_ELMO 				(3)
#define TEST_JIG_TYPE_ELMO_WEIGHT_CAL 	(4)

#define TEST_JIG_VERSION_MAJOR 			(1)
#define TEST_JIG_VERSION_MINOR 			(0)
#define TEST_JIG_VERSION_PATCH 			(0)


static msg_protocol_ctx_t input_msg_protocol = { };

void write_to_cdc(uint8_t* buf, uint32_t buf_len);

void send_ack(void)
{
	uint8_t  data[1] = { TEST_JIG_MSG_ACK };
	uint16_t data_size = 1;
	uint8_t  buf[16];
	uint16_t buf_size = msg_protocol_pack(&input_msg_protocol, data, data_size, buf);

	// HAL_UART_Transmit(&huart1, buf, buf_size, 1000);
	tud_cdc_write(buf, buf_size);
	tud_cdc_write_flush();
	// write_to_cdc(data, data_size);
	// write_to_cdc(buf, buf_size);
}

void send_nack(void)
{
	uint8_t  data[1] = { TEST_JIG_MSG_NACK };
	uint16_t data_size = 1;
	uint8_t  buf[4];
	uint16_t buf_size = msg_protocol_pack(&input_msg_protocol, data, data_size, buf);
	write_to_cdc(buf, buf_size);
}
void send_version(void) {
	uint8_t  data[5];
	uint16_t data_size = 0;

	data[data_size++] = TEST_JIG_MSG_GET_VERSION;

	data[data_size++] = TEST_JIG_TYPE_ERNIE;


	data[data_size++] = TEST_JIG_VERSION_MAJOR;
	data[data_size++] = TEST_JIG_VERSION_MINOR;
	data[data_size++] = TEST_JIG_VERSION_PATCH;

	uint8_t  buf[16];
	uint16_t buf_size = msg_protocol_pack(&input_msg_protocol, data, data_size, buf);

	write_to_cdc(buf, buf_size);
}

void process_input_message(uint8_t* data, uint16_t size)
{
	uint16_t idx = 0;
	uint8_t msg_type = data[idx++];
// #if defined(ERNIE_TEST_JIG) || defined(ELMO_TEST_JIG) || defined(ELMO_WEIGHT_CAL_JIG)
// 	HAL_GPIO_TogglePin(LED_UART_ACTIVE_GPIO_Port, LED_UART_ACTIVE_Pin);
// #endif

	switch (msg_type) {
	case TEST_JIG_MSG_APPLY_VOLTAGE: {
		uint8_t enable = data[idx++];
		// apply_voltage(enable);
		send_ack();
		break;
	}

	case TEST_JIG_MSG_APPLY_LOW_VOLTAGE: {
		break;
	}

	case TEST_JIG_MSG_ENABLE_SARA_VUSB: {
		uint8_t enable = data[idx++];
		// enable_sara_usb_v(enable);
		send_ack();
		break;
	}

	case TEST_JIG_MSG_ENABLE_BYPASS: {
		uint8_t enable = data[idx++];
		// enable_ucurrent_short(enable);
		send_ack();
		break;
	}

	case TEST_JIG_MSG_RS485_TICKLE: {
		// rxd_tickle = 0;
		// do_tickle = 1;
		break;
	}

	case TEST_JIG_MSG_GET_CURRENT: {
		// req_current = 1;
		break;
	}

	case TEST_JIG_MSG_GET_VOLTAGES: {
		// req_voltage = 1;
		break;
	}

	case TEST_JIG_MSG_GET_VERSION: {
		send_version();
		break;
	}

	case TEST_JIG_MSG_ELMO_GET_WEIGHT: {
		// elmo_adc_num_samples 	= data[idx++];
		// elmo_adc_gain 			= data[idx++];
		// elmo_adc_rej_freq 		= data[idx++];
		// elmo_adc_speed 			= data[idx++];
		// elmo_adc_read_delay 	= unpack_u32(data, &idx);
		// elmo_adc_read_timeout 	= unpack_u32(data, &idx);
		// elmo_rxd_weight = 0;
		// elmo_req_weight = 1;
		break;
	}

	case TEST_JIG_MSG_ELMO_GET_VERSION: {
		// elmo_rxd_version = 0;
		// elmo_req_version = 1;
		break;
	}

	case TEST_JIG_MSG_ENABLE_ADC_PROOF: {
		// uint8_t enable = data[idx++];
		// enable_adc_proof(enable);
		send_ack();
		break;
	}

	case TEST_JIG_MSG_RESET: {
		// NVIC_SystemReset();
		break;
	}

	case TEST_JIG_MSG_PASS_THRU_TO_DUT: {
		// dut_uart_restart();
		// pass_thru_to_dut(&data[idx], size - idx);
		break;
	}

	default: {
		send_nack();
	}
	}
}

void process_input_bad_message(void)
{
 	send_nack();
}





static void init_msg_protocol()
{
	input_msg_protocol.rx_callback_func = process_input_message;
	input_msg_protocol.rx_bad_crc_callback_func = process_input_bad_message;
}


void process_input_raw(uint8_t* data, uint16_t size)
{
	for (uint16_t i = 0; i < size; i++)
	{
		msg_protocol_process(&input_msg_protocol, data[i]);
	}
}


void write_to_cdc(uint8_t* buf, uint32_t buf_len)
{
		tud_cdc_write(buf, buf_len);
		tud_cdc_write_flush();
// 		return;
// 		int written = 0;
// 		/* Implicit overflow if we don't write all the bytes to the host.
// 		* Also throw away bytes if we can't write... */
// 		if (buf_len)
// 		{

// #ifdef PROBE_UART_RX_LED
// 			gpio_put(PROBE_UART_RX_LED, 1);
// 			rx_led_debounce = debounce_ticks;
// #endif

// 			written = MIN(tud_cdc_write_available(), buf_len);
// 			if (buf_len > written)
// 			{
// 				// cdc_tx_oe++;
// 			}

// 			if (written > 0)
// 			{
// 				tud_cdc_write(buf, written);
// 				tud_cdc_write_flush();
// 			}
// 		}
// 		else
// 		{
// #ifdef PROBE_UART_RX_LED
// 			if (rx_led_debounce)
// 				rx_led_debounce--;
// 			else
// 				gpio_put(PROBE_UART_RX_LED, 0);
// #endif
// 		}
}


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

			// uint8_t buf[] = {1,2,3,4};
			// tud_cdc_write(buf, 4);
			// tud_cdc_write_flush();
			send_ack();
			// uart_write_blocking(PROBE_UART_INTERFACE, tx_buf, tx_len);
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

void cdc_thread(void *ptr)
{
	init_msg_protocol();
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
