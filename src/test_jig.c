#include "test_jig.h"
#include "msg_protocol.h"
#include "helpers.h"


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

static const float conversion_factor = 3.3f / (1 << 12);

static void send_current(void)
{
	float avg_current = 0.f;

	// enable_ucurrent_short(0);

	for (uint8_t i = 0; i < TEST_JIG_N_ADC_MEAS; ++i)
    {
		avg_current += _ctx.read_adc(TEST_JIG_ADC_CURRENT_CHAN);
		// HAL_Delay(100);
	}

	// enable_ucurrent_short(1);

	avg_current /= ((float)TEST_JIG_N_ADC_MEAS * 1.f);
    avg_current *= conversion_factor;

	uint8_t  data[3];
	uint16_t data_size = 0;

	data[data_size++] = TEST_JIG_MSG_GET_CURRENT;
	pack_f16(data, avg_current, 1e3, &data_size);

	uint8_t  buf[8];
	uint16_t buf_size = msg_protocol_pack(&input_msg_protocol, data, data_size, buf);

    _ctx.write_to_host(buf, buf_size);
}


static void send_voltage(void)
{
	uint8_t  data[5];
	uint16_t data_size = 0;

	data[data_size++] = TEST_JIG_MSG_GET_VOLTAGES;

    uint16_t v1 = _ctx.read_adc(TEST_JIG_ADC_3V3_CHAN) * conversion_factor;
    uint16_t v2 = _ctx.read_adc(TEST_JIG_ADC_5V_CHAN) * conversion_factor;

	pack_f16(data, v1, 1e3, &data_size);
	pack_f16(data, v2, 1e3, &data_size);

	uint8_t  buf[12];
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
		break;
	}

	case TEST_JIG_MSG_GET_CURRENT: {
		send_current();
		break;
	}

	case TEST_JIG_MSG_GET_VOLTAGES: {
		send_voltage();
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
	default: { send_nack();}
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
    _ctx.read_adc = ctx->read_adc;

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