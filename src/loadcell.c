#include "loadcell.h"


/* Checksum stuff */
static uint16_t modbus_crc16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < length; i++)
	{
        crc ^= data[i];

        for (uint8_t j = 0; j < 8; j++)
		{
            if (crc & 0x0001)
			{
                crc = (crc >> 1) ^ 0xA001;
            }
			else
			{
                crc >>= 1;
            }
        }
    }

    return crc;
}


static void append_checksum(uint8_t* buf, uint16_t buf_len)
{
	uint16_t crc = modbus_crc16(buf, buf_len - 2);
	buf[buf_len - 2] = crc & 0xFF;
	buf[buf_len - 1] = (crc >> 8) & 0xFF;
}


/* Check that the calculated checksum matches the received one */
static bool verify_checksum(const uint8_t* buf, const uint8_t buf_len)
{
	uint16_t crc_calc = modbus_crc16(buf, buf_len - sizeof(uint16_t));
	uint16_t crc_rxd  = (buf[buf_len - 2] | (buf[buf_len - 1] << 8));

	if(crc_calc != crc_rxd)
	{
		return false;
	}

	return true;
}


/* Loadcell stuff */
typedef enum {
    WEIGHT_DATA_ADDR = (uint16_t)0x0001, //ZERO_DATA_ADDR shares the same address
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
	uint8_t id_rate;
	uint8_t weight_data_buf[9];
	uint8_t zero_data_buf[9];
	uint8_t accel_buf[9];
	uint8_t id_rate_buf[9];
	uint8_t filter_param_buf[9];
	uint8_t address_buf[9];
};

#define BROADCAST_ID (0xFF)


static struct loadcell_ctx ctx = {
	.address = 1,
	.current_weight = 0, // grams
	.id_rate = 0
};


TaskHandle_t loadcell_taskhandle;
static uint8_t rx_buf[64];
static TickType_t last_wake, interval = 100;

const uint8_t weight_incr = 1000; // how much the weight will increment after each reading
const uint8_t n_readings  = 5;


// static bool gpb100d_unpack_weight(const uint8_t* data, const uint8_t len, int32_t* result)
// {
// 	if(!verify_basic_resp(data, len))
// 	{
// 		return false;
// 	}

// 	bool    negative = ((data[3] & 0xF0) >> 4) == 0x01;
// 	bool    stable   = (data[3] & 0x0F) == 0x01;
// 	uint8_t n_dp     = ((data[4] & 0xF0) >> 4);

// 	if(!stable)
// 	{
// 		LOG_ERR("The reading wasnt stable");
// 		LOG_HEXDUMP_ERR(data, len, "RX'd packet:");
// 		return false;
// 	}

// 	// Parse the values from the response
// 	uint8_t  BA  = data[LC_WEIGHT_PACKET_RESP_SIZE - sizeof(uint16_t) - 1];
// 	uint8_t  DC  = data[LC_WEIGHT_PACKET_RESP_SIZE - sizeof(uint16_t) - 2];
// 	uint8_t  FE  = data[LC_WEIGHT_PACKET_RESP_SIZE - sizeof(uint16_t) - 3];
// 	uint32_t val = ((BA & 0xFF) | (DC & 0xFF) << 8 | (FE & 0x0F) << 16);

// 	// Insert the decimal point, then convert to float
// 	char str_val[10];
// 	snprintk(str_val, 10, "%4d.%.*d\n", val / 1000, n_dp, val % 1000);
// 	float f = atof(str_val);

// 	// Convert to grams so we don't have to deal with the float
// 	int32_t grams = (f * 1000);
// 	grams = grams * (negative ? -1: 1);

// 	*result = grams;

// 	return true;
// }

#define N_DECIMAL_POINTS 3

void pack_weight_response(bool positive, bool stable, uint32_t val)
{
	uint8_t  BA  = val & 0xFF;
	uint8_t  DC  = (val & 0xFF00) >> 8;
	uint8_t  E   = val & 0x0F0000 >> 16;


	uint8_t buf[] = {
		ctx.address,
		READ_DATA_CODE,
		0x04,
		((uint8_t)positive& 0x0F << 4) | ((uint8_t)stable & 0x0F),
		((N_DECIMAL_POINTS & 0x0F) << 4) | E,
		DC,
		BA,
		0x00,
		0x00
	};

}


/* Response handlers */
static void send_weight_response()
{
	bool positive = true;
	bool stable = true;
	uint8_t  BA  = ctx.current_weight  & 0xFF;
	uint8_t  DC  = (ctx.current_weight & 0xFF00) >> 8;
	uint8_t  E   = (ctx.current_weight & 0x0F0000) >> 16;


	uint8_t buf[] = {
		ctx.address,
		READ_DATA_CODE,
		0x04,
		((uint8_t)positive & 0x0F << 4) | ((uint8_t)stable & 0x0F),
		((N_DECIMAL_POINTS & 0x0F) << 4) | E,
		DC,
		BA,
		0x00,
		0x00
	};

	append_checksum(buf, sizeof(buf));
	uart_write_blocking(LOADCELL_UART_INTERFACE, buf, sizeof(buf));

	ctx.current_weight = ctx.current_weight + weight_incr;
}


static void send_zero_response()
{
	uint8_t buf[] = {ctx.address, WRITE_DATA_CODE, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00};
	append_checksum(buf, sizeof(buf));
	uart_write_blocking(LOADCELL_UART_INTERFACE, buf, sizeof(buf));
}


static void send_accel_response()
{
	uint8_t buf[] = {ctx.address, WRITE_DATA_CODE, 0x00, 0x09, 0x00, 0x02, 0x00, 0x00};
	append_checksum(buf, sizeof(buf));
	uart_write_blocking(LOADCELL_UART_INTERFACE, buf, sizeof(buf));
}


static void send_id_rate_response()
{
	uint8_t buf[] = {ctx.address, WRITE_DATA_CODE, 0x04, 0x0F, 0x00, 0x00, ctx.id_rate, 0x00, 0x00};
	append_checksum(buf, sizeof(buf));
	uart_write_blocking(LOADCELL_UART_INTERFACE, buf, sizeof(buf));
}


static void send_filter_param_response()
{
	uint8_t buf[] = {ctx.address, WRITE_DATA_CODE, 0x00, 0x0F, 0x00, 0x02, 0x00, 0x00};
	append_checksum(buf, sizeof(buf));
	uart_write_blocking(LOADCELL_UART_INTERFACE, buf, sizeof(buf));
}


static void send_address_response()
{
	uint8_t buf[] = {ctx.address, WRITE_DATA_CODE, 0x00, 0x0F, 0x00, 0x02, 0x00, 0x00};
	append_checksum(buf, sizeof(buf));
	uart_write_blocking(LOADCELL_UART_INTERFACE, buf, sizeof(buf));
}


static void receive_enable(bool enable)
{
    gpio_put(LOADCELL_UART_RE, !enable);
}


static void read_data(uint8_t id, gpb100d_address_e addr)
{
	switch (addr)
	{
	case WEIGHT_DATA_ADDR: send_weight_response();
	case ACCEL_ADDR:
	case ID_RATE_ADDR:
	case FILTER_ADDR:
	case ADDR_SET_ADDR:
	default:
		break;
	}
}


static void write_data(uint8_t id, gpb100d_address_e addr)
{
	switch (addr)
	{
	case WEIGHT_DATA_ADDR: send_zero_response();         break;
	case ACCEL_ADDR:       send_accel_response();        break;
	case ID_RATE_ADDR:     send_id_rate_response();      break;
	case FILTER_ADDR:      send_filter_param_response(); break;
	case ADDR_SET_ADDR:    send_address_response();      break;
	default:
		break;
	}
}


static void process_message(uint8_t* buf, uint8_t buf_len)
{

	if(buf_len < 4)
	{
		return;
	}
	receive_enable(false);

	uint8_t  request_id = buf[0];
	uint16_t fn_code    = buf[1];
	uint16_t reg_addr 	= ((buf[2] << 8) & 0xFF) | (buf[3] & 0xFF);


	if(((request_id != ctx.address) && (request_id != BROADCAST_ID)) || !verify_checksum(buf, buf_len))
	{
		return;
	}


	switch ((gpb100d_fn_code_e)fn_code)
	{
		case READ_DATA_CODE:  read_data(request_id, reg_addr); break;
		case WRITE_DATA_CODE: write_data(request_id, reg_addr); break;
		default:              break;
	}

	receive_enable(true);
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


bool loadcell_task(void)
{
    uint rx_len = 0;
	// Consume uart fifo regardless even if not connected
	while(uart_is_readable(LOADCELL_UART_INTERFACE) && (rx_len < sizeof(rx_buf)))
	{
		rx_buf[rx_len++] = uart_getc(LOADCELL_UART_INTERFACE);
	}

	if(rx_len)
	{
		process_message(rx_buf, rx_len);
	}

    return false;
}


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

