#include "rfid.h"

#include "main.h"
#include "helpers.h"

#include <string.h>

#ifdef ENABLE_RFID
#include "rfal_rf.h"
#include "rfal_analogConfig.h"
#include "rfal_nfca.h"
#include "st25R3911_interrupt.h"

uint8_t rfid_scan(uint8_t* uid, uint8_t* size) {
	ReturnCode err;
	rfalNfcaSensRes sens_res;
	uint8_t success = 0;

	rfalNfcaPollerInitialize();
	rfalFieldOnAndStartGT();

	err = rfalNfcaPollerTechnologyDetection(RFAL_COMPLIANCE_MODE_NFC, &sens_res);
	if (err == ERR_NONE) {
	    rfalNfcaListenDevice nfca_dev_list[1];
	    uint8_t dev_cnt;

	    err = rfalNfcaPollerFullCollisionResolution(RFAL_COMPLIANCE_MODE_NFC, 1, nfca_dev_list, &dev_cnt);
	    if ((err == ERR_NONE) && (dev_cnt > 0)) {
			if (nfca_dev_list[0].type != RFAL_NFCA_T1T) {
				LOG_DEBUG("NFC-A tag found: %s (%d)", hex_to_str(nfca_dev_list[0].nfcId1, nfca_dev_list[0].nfcId1Len), nfca_dev_list[0].nfcId1Len);
				if ((uid != NULL) && (size != NULL)) {
					memcpy(uid, nfca_dev_list[0].nfcId1, nfca_dev_list[0].nfcId1Len);
					*size = nfca_dev_list[0].nfcId1Len;
				}
			}

			success = 1;
	    }
	}

	rfalFieldOff();

	return success;
}
#endif
