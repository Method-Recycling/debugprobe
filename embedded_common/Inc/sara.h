#ifndef SARA_H_
#define SARA_H_

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include "main.h"

#define METHOD_LTE_APN_NAME_MAX_LEN 20

#define SARA_1V8_ON_VOLTAGE				(1.7f)
#define SARA_1V8_OFF_VOLTAGE			(0.1f)

#define SARA_HARD_POWER_DELAY			(3200)
#define SARA_HARD_POWER_ON_DELAY		(150)
#define SARA_HARD_POWER_OFF_DELAY		(1500)
#define SARA_POWER_OFF_TIMEOUT			(10000)
#define SARA_POWER_ON_TIMEOUT			(10000)
#define SARA_AT_READY_TIMEOUT			(30000)
#define SARA_RESET_DELAY				(30200)
#define SARA_AT_RESPONSE_TIMEOUT 		(30000)
#define SARA_FAST_AT_RESPONSE_TIMEOUT 	(5000)
#define SARA_REGISTER_TIMEOUT 			(480000)
#define SARA_SOCKET_CONNECT_TIMEOUT 	(30000)
#define SARA_SOCKET_CLOSE_TIMEOUT 		(10000)

#define SARA_SOCKET_MAX_PAYLOAD_BYTES 	(1024)
#define SARA_SOCKET_BINARY_SEND_DELAY 	(100)

#define SARA_CMD_BUF_SIZE				(256)
#define SARA_SOCKET_BUF_SIZE 			(64)

#define SARA_SOCKET_NONE 				(-1)
#define SARA_SIGNAL_QUAL_UNKNOWN 		(255)

#define SARA_TRANSMIT_DELAY 			(750) // This annoyingly is required to prevent the SARA from becoming unresponsive between transmissions

#define SARA_PDP_CONTEXT_CID            (1)


enum {
	SARA_RAT_CATM1 = 7,
	SARA_RAT_NBIOT = 8
};

enum {
	SARA_SOCKET_TCP = 6,
	SARA_SOCKET_UDP = 17
};

enum {
	SARA_REGISTERED_NONE = 0,
	SARA_REGISTERED_HOME,
	SARA_REGISTERED_NOT,
	SARA_REGISTERED_DENIED,
	SARA_REGISTERED_UNKNOWN,
	SARA_REGISTERED_ROAMING
};

enum {
	SARA_GPIO_MODE_OUTPUT = 0,
	SARA_GPIO_MODE_INPUT,
	SARA_GPIO_MODE_NETWORK_STATUS_INDICATION,
	SARA_GPIO_MODE_GNSS_SUPPLY_ENABLE,
	SARA_GPIO_MODE_GNSS_DATA_READY,
	SARA_GPIO_MODE_GNSS_RTC_SHARING,
	SARA_GPIO_MODE_SIM_CARD_DETECTION = 7,
	SARA_GPIO_MODE_HEADSET_DETECTION,
	SARA_GPIO_MODE_GSM_TX_BURST_INDICATION,
	SARA_GPIO_MODE_MODULE_OPERATING_STATUS_INDICATION,
	SARA_GPIO_MODE_MODULE_FUNCTIONALITY_STATUS_INDICATION,
	SARA_GPIO_MODE_I2S_DIGITIAL_AUDIO_INTERFACE,
	SARA_GPIO_MODE_SPI_SERIAL_INTERFACE,
	SARA_GPIO_MODE_MASTER_CLOCK_GENERATION,
	SARA_GPIO_MODE_UART_INTERFACE,
	SARA_GPIO_MODE_WIFI_ENABLE,
	SARA_GPIO_MODE_RING_INDICATION = 18,
	SARA_GPIO_MODE_LAST_GASP_ENABLE,
	SARA_GPIO_MODE_LWM2M_PULSE,
	SARA_GPIO_MODE_32768KHZ_OUTPUT = 32,
	SARA_GPIO_MODE_DISABLED = 255
};

enum {
	SARA_STATUS_UNKNOWN = 0,
	SARA_STATUS_AVAILABLE,
	SARA_STATUS_CURRENT,
	SARA_STATUS_FORBIDDEN
};

enum {
	SARA_ACT_GSM = 0,
	SARA_ACT_GPRS = 3,
	SARA_ACT_LTE = 7,
	SARA_ACT_EC_GSM_IOT = 8,
	SARA_ACT_E_UTRAN = 9
};

enum {
	SARA_MODE_DEFAULT = 0,
	SARA_MODE_MANUAL,
	SARA_MODE_DEREGISTER,
	SARA_MODE_SET_ONLY,
	SARA_MODE_AUTOMATIC
};

enum {
	SARA_OP_FORMAT_LONG_ALPHANUMERIC = 0,
	SARA_OP_FORMAT_SHORT_ALPHANUMERIC,
	SARA_OP_FORMAT_NUMERIC
};

enum {
	SARA_PSD_PROTO_IPV4 = 0,
	SARA_PSD_PROTO_IPV6,
	SARA_PSD_PROTO_IPV4V6_PREF_IPV4,
	SARA_PSD_PROTO_IPV4V6_PREF_IPV6
};

typedef float (*get_voltage_fptr)(void);

typedef struct {
	GPIO_TypeDef* 		gpio_pwr_on_port;
	GPIO_TypeDef* 		gpio_reset_port;
	GPIO_TypeDef* 		gpio_pwr_switch_port;
	GPIO_TypeDef* 		gpio_status_port;
	uint16_t 			gpio_pwr_on_pin;
	uint16_t			gpio_reset_pin;
	uint16_t			gpio_pwr_switch_pin;
	uint16_t 			gpio_status_pin;
	void* 				handle;
	volatile uint8_t 	recv_wait;
	const char* 		recv_cmd;
	va_list 			recv_args;
	uint8_t 			recv_success;
	uint8_t 			recv_first_only;
	get_voltage_fptr 	get_voltage;
	uint8_t 			on;
	uint8_t 			socket_direct_link_mode;
	uint8_t 			rat;
	uint8_t             mno_profile;
	uint32_t             bandmask;
	unsigned char       apn_name[METHOD_LTE_APN_NAME_MAX_LEN];
} sara_ctx_t;

typedef struct {
	char 	name_long[24];
	char 	name_short[10];
	char 	name_numeric[6];
	uint8_t stat;
	uint8_t act;
} sara_operator_t;

uint8_t sara_write(sara_ctx_t* ctx, uint8_t* data, uint16_t size);
void	sara_process_raw(sara_ctx_t* ctx, uint8_t* data, uint16_t size);
uint8_t sara_power_on(sara_ctx_t* ctx, uint8_t* is_configured);
uint8_t sara_power_off(sara_ctx_t* ctx);
void 	sara_reset(sara_ctx_t* ctx);
uint8_t sara_at_check(sara_ctx_t* ctx);
uint8_t sara_write_config(sara_ctx_t* ctx);
uint8_t sara_echo_off(sara_ctx_t* ctx);
uint8_t sara_set_gpio(sara_ctx_t* ctx, uint8_t gpio_id, uint8_t gpio_mode);
uint8_t sara_set_mno_profile(sara_ctx_t* ctx, uint8_t mno_profile);
uint8_t sara_set_power_saving_mode(sara_ctx_t* ctx, uint8_t enable);
uint8_t sara_set_pdp(sara_ctx_t* ctx, char* apn_name);
uint8_t sara_set_rat(sara_ctx_t* ctx, uint8_t rat);
uint8_t sara_set_bandmask(sara_ctx_t* ctx, uint8_t rat, uint32_t bandmask);
uint8_t sara_set_hex_mode(sara_ctx_t* ctx, uint8_t enable);
uint8_t sara_set_operator(sara_ctx_t* ctx, uint8_t mode, uint8_t format, char* operator);
uint8_t sara_get_fw_ver(sara_ctx_t* ctx, char* ver);
uint8_t sara_get_imei(sara_ctx_t* ctx, char* imei);
uint8_t sara_get_ext_signal_quality(sara_ctx_t* ctx, uint8_t* rsrq, uint8_t* rsrp);
uint8_t sara_get_ccid(sara_ctx_t* ctx, char* ccid);
uint8_t sara_get_operator(sara_ctx_t* ctx, char* operator);
uint8_t sara_get_operators(sara_ctx_t* ctx, char* operators);
uint8_t sara_register(sara_ctx_t* ctx);
uint8_t sara_deregister(sara_ctx_t* ctx);
uint8_t sara_socket_create(sara_ctx_t* ctx, uint8_t socket_type, uint8_t* socket_id);
uint8_t sara_socket_close(sara_ctx_t* ctx, uint8_t socket_id, uint8_t async_close);
uint8_t sara_socket_connect(sara_ctx_t* ctx, uint8_t socket_id, const char* address, uint16_t port);
uint8_t sara_socket_read(sara_ctx_t* ctx, uint8_t socket_id, uint8_t* data, uint16_t* size);
uint8_t sara_socket_write(sara_ctx_t* ctx, uint8_t socket_id, uint8_t* data, uint16_t size);
uint8_t sara_socket_sendto(sara_ctx_t* ctx, uint8_t socket_id, char* address, uint16_t port, uint8_t* data, uint16_t size);
uint8_t sara_socket_get_error(sara_ctx_t* ctx, uint8_t socket_id, uint8_t* error);
uint8_t sara_socket_rxfrom(sara_ctx_t* ctx, uint8_t socket_id, uint8_t* data, uint16_t* size);
uint8_t sara_socket_enter_direct_link_mode(sara_ctx_t* ctx, uint8_t socket_id);
uint8_t sara_socket_exit_direct_link_mode(sara_ctx_t* ctx);
uint8_t sara_socket_set_direct_link_timer_trigger(sara_ctx_t* ctx, uint8_t socket_id, uint32_t delay);
bool sara_pdp_context_activate(sara_ctx_t* ctx, bool activate);
bool sara_set_psd_profile(sara_ctx_t* ctx, uint8_t psd_protocol, char* apn_name);
bool sara_resolve_dns(sara_ctx_t* ctx, char* url, char* resolved_ip);

#endif /* SARA_H_ */
