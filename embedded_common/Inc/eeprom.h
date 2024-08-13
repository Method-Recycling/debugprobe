#ifndef EEPROM_H_
#define EEPROM_H_

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

#ifdef STM32L072xx
#define EEPROM_BANK_1_START_ADDR	(DATA_EEPROM_BASE)
#define EEPROM_BANK_1_END_ADDR		(DATA_EEPROM_BANK1_END)
#define EEPROM_BANK_2_START_ADDR	(DATA_EEPROM_BANK2_BASE)
#define EEPROM_BANK_2_END_ADDR		(DATA_EEPROM_BANK2_END)

#define EEPROM_SIZE					(EEPROM_BANK_2_END_ADDR - EEPROM_BANK_1_START_ADDR)

#define EEPROM_VARS_SIZE 			(0x40)
#define EEPROM_VARS_START_ADDR 		(DATA_EEPROM_BASE)
#define EEPROM_VARS_END_ADDR 		(DATA_EEPROM_BASE + EEPROM_VARS_SIZE)

#define EEPROM_VAR_ADDR_DEVICE_UID 						(EEPROM_VARS_START_ADDR + (sizeof(uint32_t) * 0))
#define EEPROM_VAR_ADDR_DEVICE_TYPE 					(EEPROM_VARS_START_ADDR + (sizeof(uint32_t) * 1))
#define EEPROM_VAR_ADDR_DEVICE_HW_VER 					(EEPROM_VARS_START_ADDR + (sizeof(uint32_t) * 2))
#define EEPROM_VAR_ADDR_DEVICE_DEPLOYED 				(EEPROM_VARS_START_ADDR + (sizeof(uint32_t) * 3))
#define EEPROM_VAR_ADDR_SIS_ADDR_IP						(EEPROM_VARS_START_ADDR + (sizeof(uint32_t) * 4))
#define EEPROM_VAR_ADDR_SIS_ADDR_PORT 					(EEPROM_VARS_START_ADDR + (sizeof(uint32_t) * 5))
#define EEPROM_VAR_ADDR_ACTIVE_APP_SLOT 				(EEPROM_VARS_START_ADDR + (sizeof(uint32_t) * 6))
#define EEPROM_VAR_ADDR_FW_UPDATE_UID 					(EEPROM_VARS_START_ADDR + (sizeof(uint32_t) * 7))
#define EEPROM_VAR_ADDR_FW_UPDATE_FORCE_APP_SLOT 		(EEPROM_VARS_START_ADDR + (sizeof(uint32_t) * 8))
#define EEPROM_VAR_ADDR_FW_NUM_CONSECUTIVE_IWDG_RESETS 	(EEPROM_VARS_START_ADDR + (sizeof(uint32_t) * 9))

#define EEPROM_VAR_ADDR_LTE_MNO_PROFILE                 (EEPROM_VARS_START_ADDR + (sizeof(uint32_t) * 10))
#define EEPROM_VAR_ADDR_LTE_RAT                         (EEPROM_VARS_START_ADDR + (sizeof(uint32_t) * 11))
#define EEPROM_VAR_ADDR_LTE_BANDMASK                    (EEPROM_VARS_START_ADDR + (sizeof(uint32_t) * 12))
#define EEPROM_VAR_ADDR_LTE_APN_NAME                    (EEPROM_VARS_START_ADDR + (sizeof(uint32_t) * 13))


uint8_t 	eeprom_write(uint32_t addr, uint8_t* bufp, uint16_t len);
uint8_t 	eeprom_write_word(uint32_t addr, uint32_t word);
void 		eeprom_read(uint32_t addr, uint8_t* bufp, uint16_t len);
uint8_t 	eeprom_clear(void);
uint32_t 	eeprom_var_get_device_uid(void);
uint8_t 	eeprom_var_get_device_type(void);
void 		eeprom_var_get_device_hw_ver(uint8_t* major, uint8_t* minor, uint8_t* patch);
uint8_t 	eeprom_var_get_device_deployed(void);
void 		eeprom_var_get_sis_addr(uint32_t* ip, uint16_t* port);
uint8_t 	eeprom_var_get_active_app_slot(void);
uint32_t 	eeprom_var_get_fw_update_uid(void);
void 		eeprom_var_get_fw_update_force_app_slot(uint8_t* force_slot, uint8_t* slot);
uint32_t 	eeprom_var_get_fw_num_consecutive_iwdg_resets(void);
uint8_t 	eeprom_var_set_device_uid(uint32_t uid);
uint8_t 	eeprom_var_set_device_type(uint8_t type);
uint8_t 	eeprom_var_set_device_hw_ver(uint8_t major, uint8_t minor, uint8_t patch);
uint8_t 	eeprom_var_set_device_deployed(uint8_t deployed);
uint8_t 	eeprom_var_set_sis_addr(uint32_t ip, uint16_t port);
uint8_t 	eeprom_var_set_active_app_slot(uint8_t slot);
uint8_t 	eeprom_var_set_fw_update_uid(uint32_t uid);
uint8_t 	eeprom_var_set_fw_update_force_app_slot(uint8_t force_slot, uint8_t slot);
uint8_t 	eeprom_var_set_fw_num_consecutive_iwdg_resets(uint32_t num_resets);

uint32_t 	eeprom_var_get_lte_mno_profile(void);
uint32_t 	eeprom_var_get_lte_rat(void);
uint32_t 	eeprom_var_get_lte_bandmask(void);
uint32_t 	eeprom_var_get_lte_apn_name(unsigned char* apn_str_buf, uint8_t len);

bool 	eeprom_var_set_lte_mno_profile(uint32_t mno_profile);
bool 	eeprom_var_set_lte_rat(uint32_t rat);
bool 	eeprom_var_set_lte_bandmask(uint32_t bandmask);
bool 	eeprom_var_set_lte_apn_name(const unsigned char* apn_str_buf, uint8_t len);


#endif

#endif /* EEPROM_H_ */
