#include "sara.h"
#include "log.h"

#include <stdio.h>
#include <string.h>

//#define SARA_DEBUG
//#define SARA_TX_DEBUG
//#define SARA_RX_DEBUG

const char*   RESP_DELIM 	  = { "\r\n" };
const uint8_t RESP_DELIM_SIZE = 2;

static char tx_buf[1024];
static char rx_buf[1024];

static char  	cmd_buf[1024];
static uint16_t cmd_buf_size;
static uint8_t  cmd_started;

uint8_t process_at_resp(char* resp, const char* cmd, va_list* args);
void hex_str_to_bytes(uint8_t *hex, uint8_t* buf);

uint8_t sara_write(sara_ctx_t* ctx, uint8_t* data, uint16_t size) {
#ifdef SARA_TX_DEBUG
	printf("tx %u\n", size);
	for (uint16_t i = 0; i < size; i++) {
		if (data[i] == '\r') {
			printf("|r");
		} else if (data[i] == '\n') {
			printf("|n");
		} else {
			printf("%c", data[i]);
		}
	}
	printf("\n");
#endif

	return (HAL_UART_Transmit(ctx->handle, data, size, 1000) == HAL_OK);
}

uint8_t find_delim(uint8_t* data, uint16_t size, uint16_t start_idx, uint8_t look_for_end, uint16_t* delim_idx) {
	for (uint16_t i = start_idx; i < (size - 1); i++) {
	    if (memcmp(&data[i], RESP_DELIM, RESP_DELIM_SIZE) == 0) {
	        *delim_idx = i;

	        if (!look_for_end) {
	            *delim_idx += RESP_DELIM_SIZE;
	        }

	        return 1;
	    }
	}

	return 0;
}

void sara_process_raw(sara_ctx_t* ctx, uint8_t* data, uint16_t size) {
	const uint8_t MAX_NO_ITS = 5;

#ifdef SARA_RX_DEBUG
	printf("rx %u\n", size);
	for (uint16_t i = 0; i < size; i++) {
		if (data[i] == '\r') {
			printf("|r");
		} else if (data[i] == '\n') {
			printf("|n");
		} else {
			printf("%c", data[i]);
		}
	}
	printf("\n");
#endif

	if (!ctx->recv_wait || ctx->recv_success) {
		return;
	}

	uint16_t i = 0;
	uint16_t last_i = 0;
	uint8_t  num_no_its = 0;

	while (i < size) {
		// First look for edge case where delim is wrapped over two sets of raw data
		if (cmd_buf_size != 0) {
			if ((cmd_buf[cmd_buf_size - 1] == RESP_DELIM[0]) && (data[0] == RESP_DELIM[1])) {
				if (cmd_started) {
					cmd_started = 0;

					if (cmd_buf_size > 1) {
						cmd_buf_size--; // Remove delim byte
						cmd_buf[cmd_buf_size] = '\0'; // Mark end of command
						if (process_at_resp(cmd_buf, ctx->recv_cmd, &ctx->recv_args)) {
							ctx->recv_success = 1;
							if (ctx->recv_first_only) return;
						}
					}
				} else {
					cmd_started = 1;
				}

				cmd_buf_size = 0;
				i += 1; // Skip delim byte
			}
		}

		uint16_t cmd_start_idx;
		if (find_delim(data, size, i, cmd_started, &cmd_start_idx)) {
			if (cmd_started) {
				// Delim found actually delimits end of previously half received command
				uint16_t cmd_end_idx = cmd_start_idx;

				cmd_started = 0;

				if ((cmd_end_idx == i) && (cmd_buf_size == 0)) {
					// Command can't be zero length, reset and try again
					i = 0;
					continue;
				}

				memcpy(&cmd_buf[cmd_buf_size], &data[i], cmd_end_idx - i);
				cmd_buf_size += cmd_end_idx - i;

				cmd_buf[cmd_buf_size] = '\0'; // Mark end of command
				cmd_buf_size = 0;
				if (process_at_resp(cmd_buf, ctx->recv_cmd, &ctx->recv_args)) {
					ctx->recv_success = 1;
					if (ctx->recv_first_only) return;
				}

				i = cmd_end_idx + RESP_DELIM_SIZE;
			} else {
				// Delim marks the start of a new command
				cmd_started = 1;

				uint16_t cmd_end_idx;
				if (find_delim(data, size, cmd_start_idx, 1, &cmd_end_idx)) {
					// Found the ending delim
					cmd_started = 0;

					if (cmd_start_idx == cmd_end_idx) {
						// Command can't be zero length, reset and try again
						i += RESP_DELIM_SIZE; // Skip over delim
						continue;
					}

					data[cmd_end_idx] = '\0'; // Mark end of command
					if (process_at_resp((char*)&data[cmd_start_idx], ctx->recv_cmd, &ctx->recv_args)) {
						ctx->recv_success = 1;
						if (ctx->recv_first_only) return;
					}

					i = cmd_end_idx + RESP_DELIM_SIZE;
				} else {
					// Look for special case command which has no end delim
					if ((size == 3) && (data[cmd_start_idx] == '@')) {
						data[cmd_start_idx + 1] = '\0'; // Mark end of command
						if (process_at_resp((char*)&data[cmd_start_idx], ctx->recv_cmd, &ctx->recv_args)) {
							ctx->recv_success = 1;
						}
						cmd_started = 0;
						return;
					}

					// Couldn't find an ending delim, persisting this half received command
					uint16_t cmd_size = size - cmd_start_idx;
					memcpy(&cmd_buf[cmd_buf_size], &data[cmd_start_idx], cmd_size);
					cmd_buf_size += cmd_size;
					return;
				}
			}
		} else {
			uint16_t size_left = size - i;

			if (size_left != 0) {
				memcpy(&cmd_buf[cmd_buf_size], &data[i], size_left);
				cmd_buf_size += size_left;
			}

			return;
		}

		if (i == last_i) {
			if (num_no_its++ == MAX_NO_ITS) {
				cmd_buf_size = 0;
				LOG_ERROR("too many loops with no increment of index");
				return;
			}
		}

		last_i = i;
	}
}

void sara_pwr_pulse(sara_ctx_t* ctx, uint8_t on) {
	HAL_GPIO_WritePin(ctx->gpio_pwr_on_port, ctx->gpio_pwr_on_pin, GPIO_PIN_SET);
	HAL_Delay(on ? SARA_HARD_POWER_ON_DELAY : SARA_HARD_POWER_OFF_DELAY);
	HAL_GPIO_WritePin(ctx->gpio_pwr_on_port, ctx->gpio_pwr_on_pin, GPIO_PIN_RESET);
}

uint8_t sara_power_on(sara_ctx_t* ctx, uint8_t* is_configured) {
	if (ctx->on) {
#ifdef SARA_DEBUG
		LOG_DEBUG("already on");
#endif
		return 1;
	}

	HAL_GPIO_WritePin(ctx->gpio_pwr_switch_port, ctx->gpio_pwr_switch_pin, GPIO_PIN_SET);

	// Wait for capacitor to charge and stabilise
	HAL_Delay(200);

	sara_pwr_pulse(ctx, 1);

	uint32_t start_tick = HAL_GetTick();
	float    voltage = 0.f;

	while ((HAL_GetTick() - start_tick) < SARA_POWER_ON_TIMEOUT) {
		voltage = ctx->get_voltage();
		if (voltage >= SARA_1V8_ON_VOLTAGE) {
#ifdef SARA_DEBUG
			LOG_DEBUG("on (voltage: %dmV)", (int)(voltage * 1000.f));
#endif
			ctx->on = 1;
			break;
		}
	}

	if (!ctx->on) {
		HAL_GPIO_WritePin(ctx->gpio_pwr_switch_port, ctx->gpio_pwr_switch_pin, GPIO_PIN_RESET);
#ifdef SARA_DEBUG
		LOG_ERROR("failed to turn on (voltage: %dmV)", (int)(voltage * 1000.f));
#endif
		return 0;
	}

	start_tick = HAL_GetTick();

	while ((HAL_GetTick() - start_tick) < SARA_AT_READY_TIMEOUT) {
		if (HAL_GPIO_ReadPin(ctx->gpio_status_port, ctx->gpio_status_pin) == GPIO_PIN_RESET) {
#ifdef SARA_DEBUG
			LOG_DEBUG("ready");
#endif
			*is_configured = 1;
			break;
		}
	}

	return 1;
}

uint8_t sara_power_off(sara_ctx_t* ctx) {
	if (!ctx->on) {
#ifdef SARA_DEBUG
		LOG_DEBUG("already off");
#endif
		return 0;
	}

	sara_pwr_pulse(ctx, 0);

	uint32_t start_tick = HAL_GetTick();
	float    voltage = 0.f;

	while ((HAL_GetTick() - start_tick) < SARA_POWER_OFF_TIMEOUT) {
		voltage = ctx->get_voltage();
		if (voltage <= SARA_1V8_OFF_VOLTAGE) {
#ifdef SARA_DEBUG
			LOG_DEBUG("off (voltage: %dmV)", (int)(voltage * 1000.f));
#endif
			ctx->on = 0;
			break;
		}
	}

	HAL_GPIO_WritePin(ctx->gpio_pwr_switch_port, ctx->gpio_pwr_switch_pin, GPIO_PIN_RESET);

	if (ctx->on) {
#ifdef SARA_DEBUG
		LOG_ERROR("failed to turn off (voltage: %dmV)", (int)(voltage * 1000.f));
#endif
		return 0;
	}

	return 1;
}

void sara_reset(sara_ctx_t* ctx) {
	HAL_GPIO_WritePin(ctx->gpio_reset_port, ctx->gpio_reset_pin, GPIO_PIN_SET);
	HAL_Delay(SARA_RESET_DELAY);
	HAL_GPIO_WritePin(ctx->gpio_reset_port, ctx->gpio_reset_pin, GPIO_PIN_RESET);
}

uint8_t at_send(sara_ctx_t* ctx, const char* cmd, ...) {
	const char* AT_PREFIX = "AT";

	memcpy(tx_buf, AT_PREFIX, 2);

	va_list args;
	va_start(args, cmd);
	int len = vsprintf((char*)&tx_buf[2], cmd, args);
	va_end(args);

	tx_buf[len + 2] = '\r';
	tx_buf[len + 3] = 0;
	len += 3;

	return sara_write(ctx, (uint8_t*)tx_buf, len);
}

uint8_t process_at_resp(char* resp, const char* cmd, va_list* args) {
	int resp_len = strlen(resp);
	int cmd_len = strlen(cmd);

	int vars_expected = 0;
	for (int i = 0; i < (cmd_len - 1); i++) {
		if ((cmd[i] == '%') && (cmd[i + 1] != '*')) {
			vars_expected++;
		}
	}

	if (vars_expected == 0) {
		if (resp_len != cmd_len) return 0;

	    return (memcmp(resp, cmd, cmd_len) == 0);
	}

	int vars_found = vsscanf(resp, cmd, *args);

	return (vars_found == vars_expected);
}

uint8_t at_recv(sara_ctx_t* ctx, const char* cmd, ...) {
	va_start(ctx->recv_args, cmd);
	va_end(ctx->recv_args);

	ctx->recv_cmd = cmd;
	ctx->recv_success = 0;
	ctx->recv_wait = 1;

	uint32_t start_tick = HAL_GetTick();
	while ((HAL_GetTick() - start_tick) < SARA_AT_RESPONSE_TIMEOUT) {
		if (ctx->recv_success) break;
	}

	ctx->recv_wait = 0;
	ctx->recv_first_only = 0;

	return ctx->recv_success;
}

uint8_t at_recv_ok(sara_ctx_t* ctx) {
	return at_recv(ctx, "OK");
}

uint8_t sara_at_check(sara_ctx_t* ctx) {
	return (at_send(ctx, "") && at_recv_ok(ctx));
}

uint8_t sara_write_config(sara_ctx_t* ctx) {
	return (at_send(ctx, "&W") && at_recv_ok(ctx));
}

uint8_t sara_echo_off(sara_ctx_t* ctx) {
	return (at_send(ctx, "E0") && at_recv_ok(ctx));
}

uint8_t sara_set_gpio(sara_ctx_t* ctx, uint8_t gpio_id, uint8_t gpio_mode) {
	return (at_send(ctx, "+UGPIOC=%u,%u", gpio_id, gpio_mode) && at_recv_ok(ctx));
}

uint8_t sara_set_mno_profile(sara_ctx_t* ctx, uint8_t mno_profile) {
	return (at_send(ctx, "+UMNOPROF=%u", mno_profile) && at_recv_ok(ctx));
}

uint8_t sara_set_power_saving_mode(sara_ctx_t* ctx, uint8_t enable) {
	return (at_send(ctx, "+CPSMS=%u", enable) && at_recv_ok(ctx));
}

uint8_t sara_set_pdp(sara_ctx_t* ctx, char* apn_name) {
	return (at_send(ctx, "+CGDCONT=1,\"IP\",\"%s\"", apn_name) && at_recv_ok(ctx));
}

uint8_t sara_set_rat(sara_ctx_t* ctx, uint8_t rat) {
	return (at_send(ctx, "+URAT=%u", rat) && at_recv_ok(ctx));
}

uint8_t sara_set_bandmask(sara_ctx_t* ctx, uint8_t rat, uint32_t bandmask) {
	return (at_send(ctx, "+UBANDMASK=%u,%010u", rat, bandmask) && at_recv_ok(ctx));
}

uint8_t sara_set_hex_mode(sara_ctx_t* ctx, uint8_t enable) {
	return (at_send(ctx, "+UDCONF=1,%u", enable) && at_recv_ok(ctx));
}

uint8_t sara_set_operator(sara_ctx_t* ctx, uint8_t mode, uint8_t format, char* operator) {
	return (at_send(ctx, "+COPS=%u,%u,\"%s\"", mode, format, operator) && at_recv_ok(ctx));
}

uint8_t sara_get_fw_ver(sara_ctx_t* ctx, char* ver) {
	ctx->recv_first_only = 1;
	return (at_send(ctx, "+GMR") && at_recv(ctx, "%s", ver));
}

uint8_t sara_get_imei(sara_ctx_t* ctx, char* imei) {
	ctx->recv_first_only = 1;
	return (at_send(ctx, "+GSN") && at_recv(ctx, "%s", imei));
}

uint8_t sara_get_ext_signal_quality(sara_ctx_t* ctx, uint8_t* rsrq, uint8_t* rsrp) {
	if (!at_send(ctx, "+CESQ")) return 0;

	int rxlev, ber, rscp, ecn0, _rsrq, _rsrp;
	if (!at_recv(ctx, "+CESQ: %d,%d,%d,%d,%d,%d", &rxlev, &ber, &rscp, &ecn0, &_rsrq, &_rsrp)) return 0;

	*rsrq = (uint8_t)_rsrq;
	*rsrp = (uint8_t)_rsrp;

	return 1;
}

uint8_t sara_get_ccid(sara_ctx_t* ctx, char* ccid) {
	return (at_send(ctx, "+CCID") && at_recv(ctx, "+CCID: %s", ccid));
}

uint8_t sara_get_operator(sara_ctx_t* ctx, char* operator) {
	return (at_send(ctx, "+COPS?") && at_recv(ctx, "+COPS: %*d,%*d,\"%[^\"]\"", operator));
}

uint8_t sara_get_operators(sara_ctx_t* ctx, char* operators) {
	if (!at_send(ctx, "+COPS=?")) return 0;

	uint32_t start_tick = HAL_GetTick();
	while ((HAL_GetTick() - start_tick) < 60000) {
		if (at_recv(ctx, "+COPS: %[^\n]s", operators)) return 1;
	}

	return 0;
}

uint8_t sara_register(sara_ctx_t* ctx) {
	if (!at_send(ctx, "+CEREG=%d", 1)) return 0;

	uint32_t start_tick = HAL_GetTick();
	while ((HAL_GetTick() - start_tick) < SARA_REGISTER_TIMEOUT) {
		int stat;
		if (at_recv(ctx, "+CEREG: %d", &stat)) {
			if ((stat == SARA_REGISTERED_HOME) || (stat == SARA_REGISTERED_ROAMING)) {
				return 1;
			}
		}
	}

	return 0;
}

uint8_t sara_deregister(sara_ctx_t* ctx) {
	return (at_send(ctx, "+COPS=2") && at_recv_ok(ctx));
}

uint8_t sara_socket_create(sara_ctx_t* ctx, uint8_t socket_type, uint8_t* socket_id) {
	if (!at_send(ctx, "+USOCR=%u", socket_type)) return 0;

	int id;
	if (!at_recv(ctx, "+USOCR: %d", &id)) return 0;

	*socket_id = (uint8_t)id;

	return 1;
}

uint8_t sara_socket_close(sara_ctx_t* ctx, uint8_t socket_id, uint8_t async_close) {
	if (!at_send(ctx, "+USOCL=%u", socket_id)) return 0;
	uint32_t start_tick = HAL_GetTick();
	while ((HAL_GetTick() - start_tick) < SARA_SOCKET_CLOSE_TIMEOUT) {
		if (at_recv_ok(ctx)) return 1;
	}

	return 0;
}

uint8_t sara_socket_connect(sara_ctx_t* ctx, uint8_t socket_id, const char* address, uint16_t port) {
	if (!at_send(ctx, "+USOCO=%u,\"%s\",%u", socket_id, address, port)) return 0;

	uint32_t start_tick = HAL_GetTick();
	while ((HAL_GetTick() - start_tick) < SARA_SOCKET_CONNECT_TIMEOUT) {
		if (at_recv_ok(ctx)) return 1;
	}

	return 0;
}

uint8_t sara_socket_read(sara_ctx_t* ctx, uint8_t socket_id, uint8_t* data, uint16_t* size) {
	int id, len;
	if (!at_recv(ctx, "+UUSORD: %d,%d", &id, &len)) return 0;

	sara_set_hex_mode(ctx, 1);

	if (!at_send(ctx, "+USORD=%u,%d", socket_id, len)) return 0;
	if (!at_recv(ctx, "+USORD: %d,%d,\"%[^\"]\"", &id, &len, rx_buf)) return 0;

	sara_set_hex_mode(ctx, 0);

	*size = (uint16_t)len;
	hex_str_to_bytes((uint8_t*)rx_buf, data);

	return 1;
}

uint8_t sara_socket_write(sara_ctx_t* ctx, uint8_t socket_id, uint8_t* data, uint16_t size) {
	if (!at_send(ctx, "+USOWR=%u,%u", socket_id, size)) return 0;
	if (!at_recv(ctx, "@")) return 0; // Wait for binary input ready

	HAL_Delay(SARA_SOCKET_BINARY_SEND_DELAY); // Per datasheet
	sara_write(ctx, data, size);

	int id, num_bytes_written;
	if (!at_recv(ctx, "+USOWR: %d,%d", &id, &num_bytes_written)) return 0;

	return ((id == socket_id) && (num_bytes_written == size));
}

uint8_t sara_socket_rxfrom(sara_ctx_t* ctx, uint8_t socket_id, uint8_t* data, uint16_t* size) {
	int id, len, port;
	if (!at_recv(ctx, "+UUSORF: %d,%d", &id, &len)) return 0;

	sara_set_hex_mode(ctx, 1);

	if (!at_send(ctx, "+USORF=%u,%d", socket_id, len)) return 0;
	if (!at_recv(ctx, "+USORF: %d,\"%*[^\"]\",%d,%d,\"%[^\"]\"", &id, &port, &len, rx_buf)) return 0;

	sara_set_hex_mode(ctx, 0);

	*size = (uint16_t)len;
	hex_str_to_bytes((uint8_t*)rx_buf, data);

	return 1;
}

uint8_t sara_socket_sendto(sara_ctx_t* ctx, uint8_t socket_id, char* address, uint16_t port, uint8_t* data, uint16_t size) {
	if (!at_send(ctx, "+USOST=%u,\"%s\",%u,%u", socket_id, address, port, size)) return 0;
	if (!at_recv(ctx, "@")) return 0; // Wait for binary input ready

	HAL_Delay(SARA_SOCKET_BINARY_SEND_DELAY); // Per datasheet
	sara_write(ctx, data, size);

	int id, num_bytes_written;
	if (!at_recv(ctx, "+USOST: %d,%d", &id, &num_bytes_written)) return 0;

	return ((id == socket_id) && (num_bytes_written == size));
}

uint8_t sara_socket_get_error(sara_ctx_t* ctx, uint8_t socket_id, uint8_t* error) {
	if (!at_send(ctx, "+USOER")) return 0;

	int err;
	if (!at_recv(ctx, "+USOER: %d", &err)) return 0;

	*error = (uint8_t)err;

	return 1;
}

uint8_t sara_socket_enter_direct_link_mode(sara_ctx_t* ctx, uint8_t socket_id) {
	if (!at_send(ctx, "+UDCONF=5,%u,100", socket_id)) return 0;
	if (!at_recv_ok(ctx)) return 0;

	if (!at_send(ctx, "+USODL=%u", socket_id)) return 0;
	if (at_recv(ctx, "CONNECT")) {
		ctx->socket_direct_link_mode = 1;
		return 1;
	}

	return 0;
}

uint8_t sara_socket_exit_direct_link_mode(sara_ctx_t* ctx) {
	if (!sara_write(ctx, (uint8_t*)"+++", 3)) return 0;
	if (!at_recv(ctx, "DISCONNECT")) return 0;

	ctx->socket_direct_link_mode = 0;

	return 1;
}

uint8_t sara_socket_set_direct_link_timer_trigger(sara_ctx_t* ctx, uint8_t socket_id, uint32_t delay) {
	return (at_send(ctx, "+UDCONF=5,%u,%u", socket_id, delay) && at_recv_ok(ctx));
}


bool sara_pdp_context_activate(sara_ctx_t* ctx, bool activate)
{
	return at_send(ctx, "+CGACT=%d,%d", (uint8_t)activate, SARA_PDP_CONTEXT_CID) && at_recv_ok(ctx);
}

bool sara_set_psd_profile(sara_ctx_t* ctx, uint8_t psd_protocol, char* apn_name) {
	uint8_t profile_id = 0;
	return at_send(ctx, "+UPSD=%d,%d,%s", profile_id, psd_protocol, apn_name) && at_recv_ok(ctx);
}

bool sara_resolve_dns(sara_ctx_t* ctx, char* url, char* resolved_ip) {
	return at_send(ctx, "+UDNSRN=%d,\"%s\"", 0, url) && at_recv(ctx, "+UDNSRN:%s", resolved_ip);
}

void hex_str_to_bytes(uint8_t *hex, uint8_t* buf) {
    while (*hex) {
        uint8_t val = 0;

        for (uint8_t i = 0; i < 2; i++) {
            uint8_t byte = *hex++;

			// transform hex character to the 4bit equivalent number, using the ascii table indexes
			if ((byte >= '0') && (byte <= '9')) {
				byte = byte - '0';
			} else if ((byte >= 'a') && (byte <='f')) {
				byte = byte - 'a' + 10;
			} else if ((byte >= 'A') && (byte <='F')) {
				byte = byte - 'A' + 10;
			}

            val = (val << 4) | (byte & 0x0F);
        }

        *(buf++) = val;
    }
}
