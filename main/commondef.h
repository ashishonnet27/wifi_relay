/*
 * commondef.h
 *
 *  Created on: 02-Oct-2019
 *      Author: shiv
 */

#ifndef MAIN_COMMONDEF_H_
#define MAIN_COMMONDEF_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <time.h>
// #include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "driver/gpio.h"

#include "userSturct.h"

// #include "mqtt_client.h"
// #include "esp_sntp.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include "esp_spiffs.h"

#include "version.h"


#define DEVICE_PREFIX	"SPECK"


// #define MAX_MQTT_MSG_SIZE	256
// #define MAX_MQTT_TPC_SIZE	64

#define MAX_RELAY			8
#define	MAX_SWITCH			5
// #define MAX_RGBLED			2
// #define	RGB_TIMEOUT			1	//Seconds

// #define MAX_SCHED			32

//Relay ports

#define RELAY_1_PIN				21  //IO-21
#define RELAY_2_PIN			    19	//IO-19
#define RELAY_3_PIN				18	//IO-18
#define RELAY_4_PIN				17	//IO-17
#define RELAY_5_PIN				16	//IO-16
#define RELAY_6_PIN				18	//IO-18
#define RELAY_7_PIN				4	//IO-4
#define RELAY_8_PIN				2	//IO-2

//Switch Ports
// #define SW1					36	//IO-36
// #define SW2					39	//IO-39
// #define SW3					34	//IO-34
// #define	SW4					35	//IO-35
// #define SW5					5	//IO-5

#define SWRST				14	//IO-14

//ZCD Port
// #define ZCD					27 	//IO-27
// #define LEC					33	//IO-33
// #define LEO					14  //IO-14

// //Switch LED pins
// #define	RL1					2	//IO-02
// #define	GL1					15	//IO-15
// #define	BL1					32	//IO-32    <<----------------- Need changes
// #define RL2					0	//IO-00
// #define GL2					16	//IO-16
// #define	BL2					4	//IO-04
// #define ENRGB				12	//IO-12

#define RELAY_OFF				0
#define RELAY_ON				1

char deviceName[33];
char deviceAPPass[65];

char otaVersion[32];

EventGroupHandle_t wifi_event_group;
// EventGroupHandle_t mqtt_event_group;
EventGroupHandle_t common_evnet_group;
EventGroupHandle_t hardware_evnet_group;

// xSemaphoreHandle xTZBufferMutex;
// xSemaphoreHandle xOTABlock;
xSemaphoreHandle file_operation_mutex;

// xQueueHandle xQueueAction;

// xQueueHandle xQueuePublish;

xRelayData xObjRelay[MAX_RELAY];
// xSwitchData xObjSwitch[MAX_SWITCH];
xSwitchData xObjRSTSwitch;
// xRGBLED	xObjRGBLED[MAX_RGBLED];

// xMCP79410RTC xObjMCP79410RTC;

int WIFI_CONNECTED;
int AP_ON;
int PROV_CONNECTING_WIFI;
int PROV_CONNECTED_WIFI;
int PROV_FAIL_CONNECT_WIFI;
int PROV_CLOUD_CONNECTING;
int PROV_CLOUD_CONNECTED;
int PROV_CLOUD_FAIL;

// int MQTT_CONNECTED;
// int CMN_TIMESET;
// int OTA_START;

// int EVENT_FAULT_LEO;
// int EVENT_FAULT_LEO_SENT;
// int EVENT_FAULT_LEC;
// int EVENT_FAULT_LEC_SENT;
// int EVENT_FAULT_ZCR;
// int EVENT_FAULT_ZCR_SENT;

// uint8_t actionONPump1BySwitch;

void commonInit();
// char *getDeviceName();
// char *getDevicePass();
void actionOnFactoryReset();



#endif /* MAIN_COMMONDEF_H_ */
