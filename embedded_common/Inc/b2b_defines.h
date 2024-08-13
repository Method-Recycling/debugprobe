#ifndef B2B_DEFINES_H_
#define B2B_DEFINES_H_

// Board 2 Board message types
enum b2b_msg_types_e {
	B2B_MSG_TYPE_INVALID = 0x00,
	B2B_MSG_TYPE_ANNOUNCE,
	B2B_MSG_TYPE_TICKLE,
	B2B_MSG_TYPE_SLEEP,
	B2B_MSG_TYPE_GET_VERSION,
	B2B_MSG_TYPE_GET_STATS,
	B2B_MSG_TYPE_GET_RFID,
	B2B_MSG_TYPE_GET_WEIGHT,
	B2B_MSG_TYPE_ZERO_WEIGHT,
	B2B_MSG_TYPE_CAL_WEIGHT,
	B2B_MSG_TYPE_FW_UPDATE_BEGIN = 0xF0,
	B2B_MSG_TYPE_FW_UPDATE_DATA,
	B2B_MSG_TYPE_FW_UPDATE_END
};

enum b2b_errors_e {
	B2B_ERR_NONE = 0x00,
	B2B_ERR_ADC_INIT_FAIL,
	B2B_ERR_ADC_READ_FAIL
};

#endif /* B2B_DEFINES_H_ */
