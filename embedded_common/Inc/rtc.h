#ifndef RTC_H_
#define RTC_H_

#include <stdint.h>
#include "main.h"

#ifdef ENABLE_RTC
uint8_t  set_rtc_datetime(RTC_HandleTypeDef* hrtc, uint8_t year, uint8_t mon, uint8_t day, uint8_t hrs, uint8_t min, uint8_t sec);
uint32_t get_rtc_unix_time(RTC_HandleTypeDef* hrtc);
uint8_t  set_rtc_alarm(RTC_HandleTypeDef* hrtc, uint32_t alarm_num, uint32_t seconds_from_now);
#endif

#endif /* RTC_H_ */
