#include "loadcell.h"

TaskHandle_t loadcell_taskhandle;


static uint8_t tx_buf[64];
static uint8_t rx_buf[64];


static void receive_enable(bool enable)
{
    gpio_put(LOADCELL_UART_RE, !enable);
}

void loadcell_init(void)
{
    gpio_set_function(LOADCELL_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(LOADCELL_UART_RX, GPIO_FUNC_UART);
    gpio_set_pulls(LOADCELL_UART_TX, 1, 0);
    gpio_set_pulls(LOADCELL_UART_RX, 1, 0);
    uart_init(LOADCELL_UART_INTERFACE, LOADCELL_UART_BAUDRATE);

    gpio_init(LOADCELL_UART_RE);
    gpio_set_dir(LOADCELL_UART_RE, GPIO_OUT);
    receive_enable(true);
}


bool loadcell_task(void)
{
    uint rx_len = 0;
	// Consume uart fifo regardless even if not connected
	while(uart_is_readable(LOADCELL_UART_INTERFACE) && (rx_len < sizeof(rx_buf)))
	{
		rx_buf[rx_len++] = uart_getc(LOADCELL_UART_INTERFACE);
	}

    uart_write_blocking(LOADCELL_UART_INTERFACE,rx_buf, rx_len);

    return false;
}

static TickType_t last_wake, interval = 100;

void loadcell_thread(void* ptr)
{
	BaseType_t delayed;
	last_wake = xTaskGetTickCount();
	bool keep_alive;

	/* Threaded with a polling interval that scales according to linerate */
	while (1)
	{
		keep_alive = loadcell_task();
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

