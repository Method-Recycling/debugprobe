#ifndef TEST_JIG_H_
#define TEST_JIG_H_

#include <stdint.h>
#include <stdbool.h>


enum test_jig_msg
{
	TEST_JIG_MSG_NONE 				= (0x00),
	TEST_JIG_MSG_ACK 				= (0x01),
	TEST_JIG_MSG_NACK 				= (0x02),
	TEST_JIG_MSG_ERR 				= (0x03),
	TEST_JIG_MSG_APPLY_4V2 			= (0x04),
	TEST_JIG_MSG_APPLY_VOLTAGE 		= (0x05),
	TEST_JIG_MSG_APPLY_LOW_VOLTAGE 	= (0x06),
	TEST_JIG_MSG_APPLY_REVERSE_POL  = (0x07),
	TEST_JIG_MSG_ENABLE_SARA_VUSB 	= (0x08),
	TEST_JIG_MSG_ENABLE_BYPASS 		= (0x09),
	TEST_JIG_MSG_GET_CURRENT 		= (0x0A),
	TEST_JIG_MSG_GET_VOLTAGES 		= (0x0B),
	TEST_JIG_MSG_GET_OC_4V2 		= (0x0C),
	TEST_JIG_MSG_GET_VERSION 		= (0x0D),
	TEST_JIG_MSG_ELMO_GET_WEIGHT 	= (0x0E),
	TEST_JIG_MSG_ELMO_GET_VERSION 	= (0x0F),
	TEST_JIG_MSG_RS485_TICKLE 		= (0x10),
	TEST_JIG_MSG_ENABLE_ADC_PROOF 	= (0x11),
	TEST_JIG_MSG_RESET 				= (0xFE),
	TEST_JIG_MSG_PASS_THRU_TO_DUT 	= (0xFF)
};


enum test_jig_type
{
	TEST_JIG_TYPE_OSCAR 			= (1),
	TEST_JIG_TYPE_ERNIE 			= (2),
	TEST_JIG_TYPE_ELMO 				= (3),
	TEST_JIG_TYPE_ELMO_WEIGHT_CAL 	= (4)
};


enum test_jig_adc_chan
{
    TEST_JIG_ADC_3V3_CHAN     = (0),
    TEST_JIG_ADC_5V_CHAN      = (1),
    TEST_JIG_ADC_CURRENT_CHAN = (2),
    TEST_JIG_ADC_N_CHAN       = (3)
};


#define TEST_JIG_VERSION_MAJOR    (2)
#define TEST_JIG_VERSION_MINOR    (0)
#define TEST_JIG_VERSION_PATCH    (0)

// Number of measurements to take for each adc reading
#define TEST_JIG_N_ADC_MEAS       (10)

struct test_jig_ctx
{
    uint16_t voltages[TEST_JIG_ADC_N_CHAN];
	void     (*write_to_host)         (uint8_t*, uint32_t);
	void     (*write_to_dut)          (uint8_t*, uint32_t);
	void     (*apply_voltage)         (bool);
	void     (*enable_current_measure)(bool);
	uint16_t (*read_adc)              (uint8_t);
};

void tj_init(struct test_jig_ctx* ctx);
void tj_process_input_raw(uint8_t* data, uint16_t size);


#endif /* TEST_JIG_H_ */