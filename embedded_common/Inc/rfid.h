#ifndef RFID_H_
#define RFID_H_

#include <stdint.h>

uint8_t rfid_scan(uint8_t* uid, uint8_t* size);

#endif /* RFID_H_ */
