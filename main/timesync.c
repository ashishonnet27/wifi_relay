/*
 * timesync.c
 *
 *  Created on: 03-Oct-2019
 *      Author: shiv
 */

#include "timesync.h"
#include "peripheralInit.h"

void time_sync_notification_cb(struct timeval *tv)
{
	xEventGroupSetBits(common_evnet_group, CMN_TIMESET);
	ESP_LOGI("", "Notification of a time synchronization event Time zone [%s]", device_timezone.tzone);
	vTaskDelay(pdMS_TO_TICKS(500));
	//Time Sync
	ESP_LOGI("", "Updating time in RTC");
	xUpdateTimeInRTC();
}


static void initialize_sntp(void)
{
	ESP_LOGI("", "Initializing SNTP");
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, "time.google.com");
	sntp_setservername(1, "pool.ntp.org");

	sntp_set_time_sync_notification_cb(time_sync_notification_cb);
	sntp_init();
}

esp_err_t getInternetTime()
{

	xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED, false, true, portMAX_DELAY);
	xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED, false, true, portMAX_DELAY);

	time_t now = 0;
	struct tm timeinfo = { 0 };
	int retry = 0;
	int retry_count = 10;

	while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count)
	{
		ESP_LOGI("", "Waiting for system time to be set... (%d/%d)", retry, retry_count);
		time(&now);
		localtime_r(&now, &timeinfo);
		vTaskDelay(6000 / portTICK_PERIOD_MS);
	}

	if(retry >= retry_count)
	{
		return ESP_FAIL;
	}



	return ESP_OK;
}

struct tm getLocalTime(char *tz)
{
	time_t now;
	struct tm timeinfo;
	time(&now);

	char strftime_buf[64];

	localtime_r(&now, &timeinfo);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	ESP_LOGI("", "The current date/time in %s: %s", tz, strftime_buf);
	return timeinfo;
}

void taskSyncTime(void *pV)
{
	ESP_LOGI("", "Starting SNTP Time get task");

	xSemaphoreTake(xTZBufferMutex, portMAX_DELAY);
	setenv("TZ", device_timezone.tzone, 1);
	tzset();
	xSemaphoreGive(xTZBufferMutex);
	xUpdateESPTimeFromRTC();


	initialize_sntp();

	while(1)
	{
		if(getInternetTime() == ESP_OK)
		{
			//			getLocalTime("UTC0");
			//			getLocalTime("EST5EDT");
			//			getLocalTime("IST-5:30");
			//			getLocalTime("CST6CDT");
			//			getLocalTime("CET-1CEST,M3.5.0,M10.5.0/3");
			xEventGroupSetBits(common_evnet_group, CMN_TIMESET);
			sntp_stop();
			sntp_enabled();
			sntp_init();
			vTaskDelay(pdMS_TO_TICKS((1000)*(60)*(60)*(1))); //Wait for 1 hour
		}
		else
		{

			vTaskDelay(pdMS_TO_TICKS((1000)*(60)*(1)*(1))); //Wait for 1 minutes
		}

	}
}

esp_err_t xUpdateTimeInRTC()
{
	time_t rawtime;
	time( &rawtime );
	ESP_LOGE("", "%ld", rawtime);
	return xUpdateTimeInRTCManually(rawtime);
}

esp_err_t xUpdateTimeInRTCManually(time_t rawtime)
{
	time_t inputTime = rawtime;
	struct tm info;
	info = *localtime( &inputTime );

	xMCP79410RTC xObj;
	xObj.sec  = info.tm_sec;
	xObj.min  = info.tm_min;
	xObj.hrs  = info.tm_hour;
	xObj.wday = info.tm_wday + 1;
	xObj.mday = info.tm_mday;
	xObj.mon  = info.tm_mon+1;
	xObj.yrs  = info.tm_year%100;

	ESP_LOGE("", "Updating time of RTC from Internet");

	ESP_LOGE("", "Second = %d", xObj.sec);
	ESP_LOGE("", "Minute = %d", xObj.min);
	ESP_LOGE("", "Hours  = %d", xObj.hrs);
	ESP_LOGE("", "WDay   = %d", xObj.wday);
	ESP_LOGE("", "Mday   = %d", xObj.mday);
	ESP_LOGE("", "Month  = %d", xObj.mon);
	ESP_LOGE("", "Years  = %d", xObj.yrs);

	if(xWriteMCP79410(xObj) != ESP_OK)
	{
		ESP_LOGE("", "Error in writing data in RTC");
		return ESP_FAIL;
	}

	return ESP_OK;
}
esp_err_t xUpdateESPTimeFromRTC()
{

	struct tm tm;
	xMCP79410RTC xObj;
	ESP_LOGE("", "Updating time of ESP from RTC");
	if(xReadMCP79410(&xObj) != ESP_OK)
	{
		ESP_LOGI("", "Error in reading RTC");
		return ESP_FAIL;
	}
	else
	{
		tm.tm_year = xObj.yrs+100;
		tm.tm_mon = xObj.mon-1;
		tm.tm_mday = xObj.mday;
		tm.tm_hour = xObj.hrs;
		tm.tm_min = xObj.min;
		tm.tm_sec = xObj.sec;
		ESP_LOGE("", "Year" );
		time_t t = mktime(&tm);
		struct timeval now = { .tv_sec = t };
		settimeofday(&now, NULL);
	}
	xEventGroupSetBits(common_evnet_group, CMN_TIMESET);
	return ESP_OK;
}

