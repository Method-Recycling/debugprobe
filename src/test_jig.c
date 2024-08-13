#include "test_jig.h"
#include "msg_protocol.h"

static struct test_jig_ctx _ctx;
static msg_protocol_ctx_t input_msg_protocol = { };

static void send_ack(void)
{
	uint8_t  data[1] = { TEST_JIG_MSG_ACK };
	uint16_t data_size = 1;
	uint8_t  buf[16];
	uint16_t buf_size = msg_protocol_pack(&input_msg_protocol, data, data_size, buf);

	_ctx.write_to_host(buf, buf_size);
}

static void send_nack(void)
{
	uint8_t  data[1] = { TEST_JIG_MSG_NACK };
	uint16_t data_size = 1;
	uint8_t  buf[4];
	uint16_t buf_size = msg_protocol_pack(&input_msg_protocol, data, data_size, buf);

	_ctx.write_to_host(buf, buf_size);
}


static void send_version(void)
{
	uint8_t  data[5];
	uint16_t data_size = 0;

	data[data_size++] = TEST_JIG_MSG_GET_VERSION;

	data[data_size++] = TEST_JIG_TYPE_ERNIE;

	data[data_size++] = TEST_JIG_VERSION_MAJOR;
	data[data_size++] = TEST_JIG_VERSION_MINOR;
	data[data_size++] = TEST_JIG_VERSION_PATCH;

	uint8_t  buf[16];
	uint16_t buf_size = msg_protocol_pack(&input_msg_protocol, data, data_size, buf);

	_ctx.write_to_host(buf, buf_size);
}

static void process_input_message(uint8_t* data, uint16_t size)
{
	uint16_t idx = 0;
	uint8_t msg_type = data[idx++];

	switch (msg_type) {
	case TEST_JIG_MSG_APPLY_VOLTAGE: {
		uint8_t enable = data[idx++];
		_ctx.apply_voltage(enable);
		send_ack();
		break;
	}

	case TEST_JIG_MSG_APPLY_LOW_VOLTAGE: {
		break;
	}

	case TEST_JIG_MSG_ENABLE_BYPASS: {
		uint8_t enable = data[idx++];
		_ctx.enable_current_measure(enable);
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
		_ctx.write_to_dut(&data[idx], size - idx);
		break;
	}
	case TEST_JIG_MSG_ELMO_GET_WEIGHT:
	case TEST_JIG_MSG_ENABLE_SARA_VUSB:
	case TEST_JIG_MSG_ELMO_GET_VERSION:
	default: {
		send_nack();
	}
	}
}

static void process_input_bad_message(void)
{
 	send_nack();
}



void tj_init(struct test_jig_ctx* ctx)
{
    _ctx.write_to_host = ctx->write_to_host;
    _ctx.write_to_dut = ctx->write_to_dut;
    _ctx.apply_voltage = ctx->apply_voltage;
    _ctx.enable_current_measure = ctx->enable_current_measure;

    input_msg_protocol.rx_callback_func = process_input_message;
	input_msg_protocol.rx_bad_crc_callback_func = process_input_bad_message;
}

void tj_process_input_raw(uint8_t* data, uint16_t size)
{
	for (uint16_t i = 0; i < size; i++)
	{
		msg_protocol_process(&input_msg_protocol, data[i]);
	}
}