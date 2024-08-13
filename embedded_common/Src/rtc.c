#include "rtc.h"
#include "mktime.h"
#include "log.h"

#ifdef ENABLE_RTC
uint8_t set_rtc_datetime(RTC_HandleTypeDef* hrtc, uint8_t year, uint8_t mon, uint8_t day, uint8_t hrs, uint8_t min, uint8_t sec) {
	RTC_TimeTypeDef time = { 0 };
	RTC_DateTypeDef date = { 0 };

	if (HAL_RTC_WaitForSynchro(hrtc) != HAL_OK) {
		LOG_ERROR("rtc sync fail");
		return 0;
	}

	time.Hours = hrs;
	time.Minutes = min;
	time.Seconds = sec;
	time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	time.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(hrtc, &time, RTC_FORMAT_BIN) != HAL_OK) {
		LOG_ERROR("set rtc time fail");
		return 0;
	}

	date.WeekDay = RTC_WEEKDAY_MONDAY;
	date.Month = mon;
	date.Date = day;
	date.Year = year;
	if (HAL_RTC_SetDate(hrtc, &date, RTC_FORMAT_BIN) != HAL_OK) {
		LOG_ERROR("set rtc date fail");
		return 0;
	}

	return 1;
}

uint32_t get_rtc_unix_time(RTC_HandleTypeDef* hrtc) {
	RTC_TimeTypeDef time = { 0 };
	RTC_DateTypeDef date = { 0 };

	HAL_RTC_GetTime(hrtc, &time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(hrtc, &date, RTC_FORMAT_BIN);

	return unix_time_in_seconds(time.Seconds, time.Minutes, time.Hours, date.Date, date.Month, 2000 + date.Year);
}

uint8_t set_rtc_alarm(RTC_HandleTypeDef* hrtc, uint32_t alarm_num, uint32_t seconds_from_now) {
	RTC_TimeTypeDef  time  = { 0 };
	RTC_DateTypeDef  date  = { 0 };
	RTC_AlarmTypeDef alarm = { 0 };

	if (HAL_RTC_WaitForSynchro(hrtc) != HAL_OK) {
		LOG_ERROR("rtc sync fail");
		return 0;
	}

	HAL_RTC_GetTime(hrtc, &time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(hrtc, &date, RTC_FORMAT_BIN);

    uint32_t new_hours   = time.Hours;
    uint32_t new_minutes = time.Minutes;
    uint32_t new_seconds = time.Seconds + seconds_from_now;

    if (new_seconds >= 60) {
        new_minutes += new_seconds / 60;
        new_seconds %= 60;

        if (new_minutes >= 60) {
            new_hours += new_minutes / 60;
            new_minutes %= 60;
            new_hours %= 24;
        }
    }

    LOG_DEBUG("0x%X: %02d:%02d:%02d", (int)alarm_num, (int)new_hours, (int)new_minutes, (int)new_seconds);

	alarm.AlarmTime.Hours 			= (uint8_t)new_hours;
	alarm.AlarmTime.Minutes 		= (uint8_t)new_minutes;
	alarm.AlarmTime.Seconds 		= (uint8_t)new_seconds;
	alarm.AlarmTime.SubSeconds 		= time.SubSeconds;
	alarm.AlarmTime.DayLightSaving 	= RTC_DAYLIGHTSAVING_NONE;
	alarm.AlarmTime.StoreOperation 	= RTC_STOREOPERATION_RESET;
	alarm.AlarmMask 				= RTC_ALARMMASK_DATEWEEKDAY;
	alarm.AlarmSubSecondMask 		= RTC_ALARMSUBSECONDMASK_ALL;
	alarm.AlarmDateWeekDaySel 		= RTC_ALARMDATEWEEKDAYSEL_DATE;
	alarm.AlarmDateWeekDay 			= 1;
	alarm.Alarm 					= alarm_num;

	if (HAL_RTC_DeactivateAlarm(hrtc, alarm_num) != HAL_OK) {
		LOG_ERROR("deact alarm fail");
		return 0;
	}

	if (HAL_RTC_SetAlarm_IT(hrtc, &alarm, RTC_FORMAT_BIN) != HAL_OK) {
		LOG_ERROR("set alarm fail");
		return 0;
	}

	return 1;
}
#endif
