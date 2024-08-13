#include "lte.h"
#include "rtc.h"
#include "ntp.h"
#include "eeprom.h"
#include "helpers.h"
#include "log.h"

#include <time.h>
#include <string.h>

//#define LTE_DEBUG

#define ntohl(n) (((((unsigned long)(n) & 0xFF)) << 24) | \
                  ((((unsigned long)(n) & 0xFF00)) << 8) | \
                  ((((unsigned long)(n) & 0xFF0000)) >> 8) | \
                  ((((unsigned long)(n) & 0xFF000000)) >> 24))

void lte_radio_init(sara_ctx_t* ctx) {
  	ctx->mno_profile = eeprom_var_get_lte_mno_profile();
  	ctx->rat         = eeprom_var_get_lte_rat();
  	ctx->bandmask    = eeprom_var_get_lte_bandmask();

	eeprom_var_get_lte_apn_name(ctx->apn_name, METHOD_LTE_APN_NAME_MAX_LEN);
}

uint8_t lte_radio_on(sara_ctx_t* ctx, uint8_t configure) {
	uint8_t is_configured = 0;
	if (!sara_power_on(ctx, &is_configured)) {
		LOG_ERROR("failed to turn radio on");
		return LTE_RESULT_FAILED_TO_POWER_ON;
	}

	LOG_INFO("radio powered on");

	if (configure) {
		if (sara_echo_off(ctx) && \
			sara_set_gpio(ctx, 16, SARA_GPIO_MODE_MODULE_OPERATING_STATUS_INDICATION) && \
			sara_set_mno_profile(ctx, ctx->mno_profile) && \
			sara_set_power_saving_mode(ctx, 0) && \
			sara_set_pdp(ctx, (char*)ctx->apn_name) && \
			sara_set_rat(ctx, ctx->rat) && \
			sara_set_bandmask(ctx, 0, ctx->bandmask) && \
			sara_set_bandmask(ctx, 1, ctx->bandmask) && \
			sara_set_hex_mode(ctx, 0) && \
			sara_set_psd_profile(ctx, SARA_PSD_PROTO_IPV4, (char*)ctx->apn_name) && \
			sara_write_config(ctx))
		{
			LOG_INFO("radio config written");
		}
		else
		{
			LOG_ERROR("radio config write fail");
			return LTE_RESULT_FAILED_TO_CONFIGURE;
		}
	}

	return LTE_RESULT_OK;
}

uint8_t lte_radio_off(sara_ctx_t* ctx) {
	if (!sara_power_off(ctx)) {
		LOG_ERROR("failed to turn radio off");
		return LTE_RESULT_FAILED_TO_POWER_OFF;
	}

	LOG_INFO("radio off");

	return LTE_RESULT_OK;
}

uint8_t lte_register(sara_ctx_t* ctx) {
#ifdef LTE_DEBUG
	uint32_t start_time = HAL_GetTick();
#endif

	if (!sara_register(ctx)) {
		LOG_WARN("failed to register radio");
		return LTE_RESULT_FAILED_TO_REGISTER;
	}

#ifdef LTE_DEBUG
	LOG_DEBUG("register took %ums", HAL_GetTick() - start_time);
#endif

	return LTE_RESULT_OK;
}

uint8_t lte_deregister(sara_ctx_t* ctx) {
	if (!sara_deregister(ctx)) {
		LOG_ERROR("failed to deregister radio");
		return LTE_RESULT_FAILED_TO_DEREGISTER;
	}

	return LTE_RESULT_OK;
}

uint8_t lte_get_info(sara_ctx_t* ctx, char* ccid, char* operator, uint8_t* rsrq, uint8_t* rsrp) {
	uint8_t result = 1;
	if (ccid != NULL) 						result &= sara_get_ccid(ctx, ccid);
	HAL_Delay(100);
	if (operator != NULL) 					result &= sara_get_operator(ctx, operator);
	HAL_Delay(100);
	if ((rsrq != NULL) && (rsrp != NULL)) 	result &= sara_get_ext_signal_quality(ctx, rsrq, rsrp);
	HAL_Delay(100);
	return (result ? LTE_RESULT_OK : LTE_RESULT_FAILED_TO_GET_INFO);
}

uint8_t lte_socket_create(sara_ctx_t* ctx, uint8_t socket_type, uint8_t* socket_id) {
	if (!sara_socket_create(ctx, socket_type, socket_id)) {
#ifdef LTE_DEBUG
		LOG_DEBUG("create socket fail");
#endif
		return LTE_RESULT_FAILED_TO_SOCKET_CREATE;
	}

#ifdef LTE_DEBUG
	LOG_DEBUG("created %s socket with id: %u", (socket_type == SARA_SOCKET_TCP) ? "TCP" : "UDP", *socket_id);
#endif

	return LTE_RESULT_OK;
}

uint8_t lte_socket_close(sara_ctx_t* ctx, uint8_t socket_id) {
	if (!sara_socket_close(ctx, socket_id, 1)) {
		LOG_WARN("close socket fail");
		return LTE_RESULT_FAILED_TO_SOCKET_CLOSE;
	}

	return LTE_RESULT_OK;
}

uint8_t lte_socket_tcp_connect(sara_ctx_t* ctx, uint8_t socket_id, const char* address, uint16_t port) {
	if (!sara_socket_connect(ctx, socket_id, address, port)) {
		LOG_ERROR("connect socket fail (%s:%d)", address, port);
		return LTE_RESULT_FAILED_TO_SOCKET_TCP_CONNECT;
	}

	return LTE_RESULT_OK;
}

uint8_t lte_socket_tcp_write(sara_ctx_t* ctx, uint8_t socket_id, uint8_t* data, uint16_t size) {
	if (!sara_socket_write(ctx, socket_id, data, size)) {
		LOG_ERROR("write socket fail");
		return LTE_RESULT_FAILED_TO_SOCKET_TCP_WRITE;
	}

	return LTE_RESULT_OK;
}

uint8_t lte_socket_tcp_read(sara_ctx_t* ctx, uint8_t socket_id, uint8_t* data, uint16_t* size) {
	if (!sara_socket_read(ctx, socket_id, data, size)) {
		LOG_ERROR("read socket fail");
		return LTE_RESULT_FAILED_TO_SOCKET_TCP_READ;
	}

	return LTE_RESULT_OK;
}

#ifdef ENABLE_RTC
uint8_t lte_update_rtc_from_ntp(sara_ctx_t* ctx, RTC_HandleTypeDef* hrtc) {
	uint8_t socket_id;
	if (lte_socket_create(ctx, SARA_SOCKET_UDP, &socket_id) != LTE_RESULT_OK) {
		return LTE_RESULT_FAILED_TO_SOCKET_CREATE;
	}

	ntp_packet_t packet = { 0 };
	packet.li_vn_mode = 0x1B; // li = 0, vn = 3, and mode = 3

	if (!sara_socket_sendto(ctx, socket_id, NTP_SERVER_ADDR, NTP_SERVER_PORT, (uint8_t*)&packet, sizeof(ntp_packet_t))) {
		return LTE_RESULT_FAILED_TO_SOCKET_UDP_SENDTO;
	}

	uint8_t data[sizeof(ntp_packet_t)];
	uint16_t size;
	if (!sara_socket_rxfrom(ctx, socket_id, data, &size)) {
		return LTE_RESULT_FAILED_TO_SOCKET_UDP_RXFROM;
	}

	sara_socket_close(ctx, socket_id, 0);

	if (size != sizeof(ntp_packet_t)) {
		LOG_ERROR("resp wrong size: %u", size);
		return LTE_RESULT_FAILED_TO_GET_VALID_NTP;
	}

	memcpy((uint8_t*)&packet, data, size);

	time_t t = (time_t)(ntohl(packet.txTm_s) - NTP_TIMESTAMP_DELTA);
	struct tm* ptm;
	ptm = gmtime(&t);

	set_rtc_datetime(hrtc, ptm->tm_year - 100, ptm->tm_mon + 1, \
			ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

	LOG_INFO("%u", get_rtc_unix_time(hrtc));

	return LTE_RESULT_OK;
}
#endif

void lte_get_sis_addr(char* ip, uint16_t* port, uint8_t get_default) {
	uint32_t _ip   = 0;
	uint16_t _port = 0;

#ifdef STM32L072xx
	eeprom_var_get_sis_addr(&_ip, &_port);
#endif

	unpack_ip(_ip, ip);
	*port = _port;
}

/* Assumes that the SARA is already on */
bool lte_update_sis_addr(sara_ctx_t* ctx)
{
	bool success = false;

	if(sara_pdp_context_activate(ctx, true))
	{
		LOG_DEBUG("ctx activated");
		char sis_ip[16] = { 0 };

		if(sara_resolve_dns(ctx, METHOD_DEFAULT_SIS_URL, sis_ip)) {
			uint32_t ip_int = ip_str_to_ip_int(sis_ip);
			if(eeprom_var_set_sis_addr(ip_int, METHOD_DEFAULT_SIS_PORT)) {
				success = true;
			}
		}
	}
	return success;
}


