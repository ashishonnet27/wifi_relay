/*
 * userSturct.h
 *
 *  Created on: 06-Oct-2019
 *      Author: shiv
 */

#ifndef MAIN_USERSTURCT_H_
#define MAIN_USERSTURCT_H_

#include <stdlib.h>
// #include <time.h>

typedef struct __xProvData__
{
	char isProvisioned;
	char ssid[32];
	char password[64];
	char isStatic_ip;
	char static_ip_addr[20];
	char static_ip_gw[20];
	char static_ip_netmask[20];

}xProvData;

xProvData xObjProvData;

// typedef struct __xRLScheduler__
// {
// 	uint8_t active;
// 	int hh[7];
// 	int mm[7];
// 	time_t duration[7];
// 	char fileName[32];
// }xRLScheduler;

// xRLScheduler xObjRLScheduler[5];

// typedef struct __xPumpLOCK__
// {
// 	uint8_t lock;
// 	char fileName[32];
// }xPumpLOCK;
// xPumpLOCK xObjPumpLOCK;

// typedef struct __xCmdAction__
// {
// 	char *cmd;
// 	void (*cb) (char *);
// 	uint8_t scope;
// }xCmdAction;

// typedef struct __xMsgPublish__
// {
// 	char msg[256];
// 	char topic[64];
// 	uint8_t retain;
// }xMsgPublish;

// typedef struct __xMsgHanle__
// {
// 	char msg[256];
// }xMsgReceived;

// typedef struct __xDeviceTimeZone__
// {
// 	char tzone[65];
// }device_time_zone_def;

// device_time_zone_def device_timezone;

typedef struct __xRelayData__
{
	//uint8_t status;
	time_t  offTime;
	uint8_t mode;
	uint8_t emergencyOff[32];
	uint8_t gpioNo;
	uint8_t last_sched;
}xRelayData;

typedef struct __xSwitchData__
{
	uint8_t currentState;
	uint8_t presentState;
	uint8_t actionRequired;
	uint8_t display;
	uint8_t gpioNo;
}xSwitchData;


// typedef struct __xMCP79410RTC__
// {
// 	uint8_t sec;
// 	uint8_t min;
// 	uint8_t hrs;
// 	uint8_t wday;
// 	uint8_t mday;
// 	uint8_t mon;
// 	uint8_t yrs;
// }xMCP79410RTC;

// typedef struct __xRGBLED__
// {
// 	uint8_t red;
// 	uint8_t	blue;
// 	uint8_t	green;
// }xRGBLED;

// typedef struct __sched_struct__
// {
// 	uint8_t enable;
// 	uint8_t day;
// 	uint8_t start_time_HH;
// 	uint8_t start_time_MM;
// 	uint32_t duration;
// 	uint8_t port;
// }sched_def;

#endif /* MAIN_USERSTURCT_H_ */
