#include "eeprom.h"

#include <string.h> // for: memcpy

#ifdef STM32L072xx
uint8_t eeprom_write(uint32_t addr, uint8_t* bufp, uint16_t len) {
	if ((addr < EEPROM_BANK_1_START_ADDR) || (addr > EEPROM_BANK_2_END_ADDR) || (len > EEPROM_SIZE)) return 0;

	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_SR_PGAERR | FLASH_FLAG_WRPERR);

	if (HAL_FLASHEx_DATAEEPROM_Unlock() != HAL_OK) {
		LOG_ERROR("EEPROM unlock fail");
		return 0;
	}

	for (uint16_t i = 0; i < len; i++) {
		if (HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, addr + i, bufp[i]) != HAL_OK) {
			LOG_ERROR("EEPROM program fail @ 0x%08X", addr + i);
			return 0;
		}
	}

	if (HAL_FLASHEx_DATAEEPROM_Lock() != HAL_OK) {
		LOG_ERROR("EEPROM lock fail");
		return 0;
	}

	return 1;
}

uint8_t eeprom_write_word(uint32_t addr, uint32_t word) {
	if ((addr < EEPROM_BANK_1_START_ADDR) || (addr > EEPROM_BANK_2_END_ADDR)) return 0;

	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_SR_PGAERR | FLASH_FLAG_WRPERR);

	if (HAL_FLASHEx_DATAEEPROM_Unlock() != HAL_OK) {
		return 0;
	}

	if (HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, addr, word) != HAL_OK) {
		return 0;
	}

	if (HAL_FLASHEx_DATAEEPROM_Lock() != HAL_OK) {
		return 0;
	}

	return 1;
}

void eeprom_read(uint32_t addr, uint8_t* bufp, uint16_t len) {
	memcpy(bufp, (uint32_t*)addr, len);
}

uint8_t eeprom_clear(void) {
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_SR_PGAERR | FLASH_FLAG_WRPERR);

	if (HAL_FLASHEx_DATAEEPROM_Unlock() != HAL_OK) {
		return 0;
	}

	for (uint32_t address = EEPROM_VARS_END_ADDR; address < EEPROM_BANK_2_END_ADDR; address += sizeof(uint32_t)) {
		if (HAL_FLASHEx_DATAEEPROM_Erase(address) != HAL_OK) {
			return 0;
		}
	}

	if (HAL_FLASHEx_DATAEEPROM_Lock() != HAL_OK) {
		return 0;
	}

	return 1;
}

uint32_t eeprom_var_get_device_uid(void) {
	return (*((uint32_t*)EEPROM_VAR_ADDR_DEVICE_UID));
}

uint8_t eeprom_var_get_device_type(void) {
	return (*((uint8_t*)EEPROM_VAR_ADDR_DEVICE_TYPE));
}

void eeprom_var_get_device_hw_ver(uint8_t* major, uint8_t* minor, uint8_t* patch) {
	uint32_t val = *((uint32_t*)EEPROM_VAR_ADDR_DEVICE_HW_VER);
	*major = (val >> 16) & 0xFF;
	*minor = (val >> 8) & 0xFF;
	*patch = val & 0xFF;
}

uint8_t eeprom_var_get_device_deployed(void) {
	return (*((uint8_t*)EEPROM_VAR_ADDR_DEVICE_DEPLOYED));
}

void eeprom_var_get_sis_addr(uint32_t* ip, uint16_t* port) {
	*ip   = *((uint32_t*)EEPROM_VAR_ADDR_SIS_ADDR_IP);
	*port = *((uint16_t*)EEPROM_VAR_ADDR_SIS_ADDR_PORT);
}

uint8_t eeprom_var_get_active_app_slot(void) {
	return (*((uint8_t*)EEPROM_VAR_ADDR_ACTIVE_APP_SLOT));
}

uint32_t eeprom_var_get_fw_update_uid(void) {
	return (*((uint32_t*)EEPROM_VAR_ADDR_FW_UPDATE_UID));
}

void eeprom_var_get_fw_update_force_app_slot(uint8_t* force_slot, uint8_t* slot) {
	uint32_t val = *((uint32_t*)EEPROM_VAR_ADDR_FW_UPDATE_FORCE_APP_SLOT);
	*force_slot = val >> 31;
	*slot 		= val & 0xFF;
}

uint32_t eeprom_var_get_fw_num_consecutive_iwdg_resets(void) {
	return (*((uint32_t*)EEPROM_VAR_ADDR_FW_NUM_CONSECUTIVE_IWDG_RESETS));
}

uint8_t eeprom_var_set_device_uid(uint32_t uid) {
	return eeprom_write_word(EEPROM_VAR_ADDR_DEVICE_UID, uid);
}

uint8_t eeprom_var_set_device_type(uint8_t type) {
	return eeprom_write_word(EEPROM_VAR_ADDR_DEVICE_TYPE, (uint32_t)type);
}

uint8_t eeprom_var_set_device_hw_ver(uint8_t major, uint8_t minor, uint8_t patch) {
	uint32_t val = (major << 16) | (minor << 8) | patch;
	return eeprom_write_word(EEPROM_VAR_ADDR_DEVICE_HW_VER, val);
}

uint8_t eeprom_var_set_device_deployed(uint8_t deployed) {
	return eeprom_write_word(EEPROM_VAR_ADDR_DEVICE_DEPLOYED, (uint32_t)deployed);
}

uint8_t eeprom_var_set_sis_addr(uint32_t ip, uint16_t port) {
	return eeprom_write_word(EEPROM_VAR_ADDR_SIS_ADDR_IP, ip) && \
			eeprom_write_word(EEPROM_VAR_ADDR_SIS_ADDR_PORT, (uint32_t)port);
}

uint8_t eeprom_var_set_active_app_slot(uint8_t slot) {
	return eeprom_write_word(EEPROM_VAR_ADDR_ACTIVE_APP_SLOT, (uint32_t)slot);
}

uint8_t eeprom_var_set_fw_update_uid(uint32_t uid) {
	return eeprom_write_word(EEPROM_VAR_ADDR_FW_UPDATE_UID, uid);
}

uint8_t eeprom_var_set_fw_update_force_app_slot(uint8_t force_slot, uint8_t slot) {
	uint32_t val = (force_slot << 31) | slot;
	return eeprom_write_word(EEPROM_VAR_ADDR_FW_UPDATE_FORCE_APP_SLOT, val);
}

uint8_t eeprom_var_set_fw_num_consecutive_iwdg_resets(uint32_t num_resets) {
	return eeprom_write_word(EEPROM_VAR_ADDR_FW_NUM_CONSECUTIVE_IWDG_RESETS, num_resets);
}

uint32_t eeprom_var_get_lte_mno_profile(void)
{
	return (*((uint32_t*)EEPROM_VAR_ADDR_LTE_MNO_PROFILE));
}

uint32_t eeprom_var_get_lte_rat(void)
{
	return (*((uint32_t*)EEPROM_VAR_ADDR_LTE_RAT));
}

uint32_t eeprom_var_get_lte_bandmask(void)
{
	return (*((uint32_t*)EEPROM_VAR_ADDR_LTE_BANDMASK));
}

uint32_t eeprom_var_get_lte_apn_name(unsigned char* apn_str_buf, uint8_t len)
{
	eeprom_read(EEPROM_VAR_ADDR_LTE_APN_NAME, apn_str_buf, len);
	return 0;
}

bool eeprom_var_set_lte_mno_profile(uint32_t mno_profile) {
	return eeprom_write_word(EEPROM_VAR_ADDR_LTE_MNO_PROFILE, mno_profile);
}

bool eeprom_var_set_lte_rat(uint32_t rat) {
	return eeprom_write_word(EEPROM_VAR_ADDR_LTE_RAT, rat);
}

bool eeprom_var_set_lte_bandmask(uint32_t bandmask) {
	return eeprom_write_word(EEPROM_VAR_ADDR_LTE_BANDMASK, bandmask);
}

bool eeprom_var_set_lte_apn_name(const unsigned char* apn_str_buf, uint8_t len) {
	return eeprom_write(EEPROM_VAR_ADDR_LTE_APN_NAME, (uint8_t*)apn_str_buf, len);
}


#endif
