#ifndef LTE_H_
#define LTE_H_

#include "sara.h"

#define METHOD_LTE_APN_NAME			"telstra.iot"
#define METHOD_LTE_MNO_PROFILE		(100)
#define METHOD_LTE_BANDMASK 		(0x8000084)   //restrict to B3, B8, B28 for Australia, New Zealand networks
//#define METHOD_LTE_BANDMASK 		(0x8080084) // Channels 3, 8, 20 and 28 .. Binary to hex of 1000000010000000000010000100
//#define METHOD_LTE_BANDMASK 		(0x8000004) // Channels 3 and 28 .. Binary to hex of 1000000000000000000000000100
//#define METHOD_LTE_BANDMASK 		(0x8000000) // Channel 28 .. Binary to hex of 1000000000000000000000000000
#define METHOD_DEFAULT_SIS_ADDR 	(0xC3FB2514) // 20.37.251.195


#ifdef STAGING_ENV
#define METHOD_DEFAULT_SIS_URL      (char*)"devices.staging.methodrecycling.com"
#define METHOD_DEFAULT_SIS_PORT		(6283)
#else
#define METHOD_DEFAULT_SIS_URL      (char*)"devices.methodrecycling.com"
#define METHOD_DEFAULT_SIS_PORT		(7283)
#endif
#define NTP_SERVER_ADDR 			"pool.ntp.org"
#define NTP_SERVER_PORT 			(123)

// All timeouts below in milliseconds
#define LTE_REGISTER_TIMEOUT			(180000)
#define LTE_SOCKET_CREATE_TIMEOUT 		(10000)
#define LTE_SOCKET_CLOSE_TIMEOUT 		(5000)
#define LTE_SOCKET_CONNECT_TIMEOUT 		(10000)
#define LTE_SOCKET_WRITE_TIMEOUT 		(10000)
#define LTE_SOCKET_READ_TIMEOUT 		(10000)
#define LTE_SOCKET_RXFROM_TIMEOUT 		(10000)
#define LTE_SIGNAL_QUAL_TIMEOUT 		(10000)

enum {
	LTE_RESULT_OK = 0,
	LTE_RESULT_FAILED_TO_POWER_ON,
	LTE_RESULT_FAILED_TO_CONFIGURE,
	LTE_RESULT_FAILED_TO_POWER_OFF,
	LTE_RESULT_FAILED_TO_REGISTER,
	LTE_RESULT_FAILED_TO_DEREGISTER,
	LTE_RESULT_FAILED_TO_GET_INFO,
	LTE_RESULT_FAILED_TO_GET_TIME,
	LTE_RESULT_FAILED_TO_SOCKET_CREATE,
	LTE_RESULT_FAILED_TO_SOCKET_CLOSE,
	LTE_RESULT_FAILED_TO_SOCKET_TCP_CONNECT,
	LTE_RESULT_FAILED_TO_SOCKET_TCP_WRITE,
	LTE_RESULT_FAILED_TO_SOCKET_TCP_READ,
	LTE_RESULT_FAILED_TO_SOCKET_UDP_SENDTO,
	LTE_RESULT_FAILED_TO_SOCKET_UDP_RXFROM,
	LTE_RESULT_FAILED_TO_SOCKET_READ,
	LTE_RESULT_FAILED_TO_GET_VALID_NTP
};

void    lte_radio_init(sara_ctx_t* ctx);
uint8_t lte_radio_on(sara_ctx_t* ctx, uint8_t configure);
uint8_t lte_radio_off(sara_ctx_t* ctx);
uint8_t lte_register(sara_ctx_t* ctx);
uint8_t lte_deregister(sara_ctx_t* ctx);
uint8_t lte_get_info(sara_ctx_t* ctx, char* ccid, char* operator, uint8_t* rsrq, uint8_t* rsrp);
uint8_t lte_socket_create(sara_ctx_t* ctx, uint8_t socket_type, uint8_t* socket_id);
uint8_t lte_socket_close(sara_ctx_t* ctx, uint8_t socket_id);
uint8_t lte_socket_tcp_connect(sara_ctx_t* ctx, uint8_t socket_id, const char* address, uint16_t port);
uint8_t lte_socket_tcp_write(sara_ctx_t* ctx, uint8_t socket_id, uint8_t* data, uint16_t size);
uint8_t lte_socket_tcp_read(sara_ctx_t* ctx, uint8_t socket_id, uint8_t* data, uint16_t* size);
bool    lte_update_sis_addr(sara_ctx_t* ctx);
#ifdef ENABLE_RTC
uint8_t lte_update_rtc_from_ntp(sara_ctx_t* ctx, RTC_HandleTypeDef* hrtc);
#endif
void 	lte_get_sis_addr(char* ip, uint16_t* port, uint8_t get_default);

#endif /* LTE_H_ */
