#include "loadcell.h"


typedef enum {
    WEIGHT_DATA_ADDR = (uint16_t)0x0001,
    // ZERO_DATA_ADDR   = (uint16_t)0x0001,
    ACCEL_ADDR       = (uint16_t)0x0009,
    ID_RATE_ADDR     = (uint16_t)0x000B,
    FILTER_ADDR      = (uint16_t)0x000D,
    ADDR_SET_ADDR    = (uint16_t)0x000F
} gpb100d_address_e;


typedef enum {
    READ_DATA_CODE   = (uint8_t)0x03,
    WRITE_DATA_CODE  = (uint8_t)0x10
} gpb100d_fn_code_e;



struct loadcell_ctx
{
	uint8_t address;
	uint16_t current_weight;
	uint8_t weight_data_buf[9];
	uint8_t zero_data_buf[9];
	uint8_t accel_buf[9];
	uint8_t id_rate_buf[9];
	uint8_t filter_param_buf[9];
	uint8_t address_buf[9];
};

static struct loadcell_ctx ctx {};

TaskHandle_t loadcell_taskhandle;


static uint8_t tx_buf[64];
static uint8_t rx_buf[64];

void send_weight_response();
void send_zero_response();
void send_accel_response();
void send_id_rate_response();
void send_filter_param_response();
void send_address_response();


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

	ctx.address = 1;
}


void read_data(uint8_t id, gpb100d_address_e addr)
{
	switch (addr)
	{
	case WEIGHT_DATA_ADDR: break;
	case ACCEL_ADDR:
	case ID_RATE_ADDR:
	case FILTER_ADDR:
	case ADDR_SET_ADDR:
	default:
		break;
	}
}



void write_data(uint8_t id, gpb100d_address_e addr)
{
	switch (addr)
	{
	case WEIGHT_DATA_ADDR: break;
	case ACCEL_ADDR:
	case ID_RATE_ADDR:
	case FILTER_ADDR:
	case ADDR_SET_ADDR:
	default:
		break;
	}
}


void process_message(uint8_t buf, uint8_t buf_len)
{
	if(buf_len >= 9)
	{
		uint8_t           request_id = buf[0];
		gpb100d_fn_code_e fn_code    = buf[1];
		gpb100d_address_e reg_addr 	 = ((buf[2] >> 8) & 0xFF) | (buf[2] & 0xFF);

		switch (fn_code)
		{
		case READ_DATA_CODE:  read_data(request_id, reg_addr);
		case WRITE_DATA_CODE: write_data(request_id, reg_addr);
		default:
			break;
		}

    	// uart_write_blocking(LOADCELL_UART_INTERFACE, rx_buf, rx_len);
	}
}



bool loadcell_task(void)
{
    uint rx_len = 0;
	// Consume uart fifo regardless even if not connected
	while(uart_is_readable(LOADCELL_UART_INTERFACE) && (rx_len < sizeof(rx_buf)))
	{
		rx_buf[rx_len++] = uart_getc(LOADCELL_UART_INTERFACE);
	}

	process_message(rx_buf, rx_len);


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

