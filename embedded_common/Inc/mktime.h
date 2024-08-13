#ifndef MKTIME_H_
#define MKTIME_H_

#include <stdint.h>

#define SEC_PER_MIN         (60)
#define SEC_PER_HOUR        (3600)
#define SEC_PER_DAY         (86400)
#define MONTHS_PER_YEAR     (12)
#define EPOCH_YEAR          (1970)
#define IS_LEAP_YEAR(year)  (((((year) % 4) == 0) && (((year) % 100) != 0)) || (((year) % 400) == 0))

static const uint8_t DAYS_PER_MONTH[2][MONTHS_PER_YEAR] = {
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static const uint16_t DAYS_PER_YEAR[2] = { 365, 366 };

uint32_t unix_time_in_seconds(uint8_t sec, uint8_t min, uint8_t hrs, uint8_t day, uint8_t mon, uint16_t year);

#endif /* MKTIME_H_ */
