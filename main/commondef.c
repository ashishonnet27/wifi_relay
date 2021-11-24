/*
 * commondef.c
 *
 *  Created on: 02-Oct-2019
 *      Author: shiv
 */

#include "commondef.h"

#include "data-storage.h"
// #include "peripheralInit.h"
// #include "peripheralControl.h"

#include "driver/gpio.h"


void commonInit()
{
	// actionONPump1BySwitch = 0;
	memset(&xObjProvData, 0 , sizeof(xObjProvData));

	WIFI_CONNECTED = BIT0;
	AP_ON = BIT1;
	PROV_CONNECTING_WIFI = BIT2;
	PROV_CONNECTED_WIFI = BIT3;
	PROV_FAIL_CONNECT_WIFI = BIT4;
	// PROV_CLOUD_CONNECTING = BIT5;
	// PROV_CLOUD_CONNECTED = BIT6;
	// PROV_CLOUD_FAIL = BIT7;

	wifi_event_group = xEventGroupCreate();

	// MQTT_CONNECTED = BIT0;

	// mqtt_event_group = xEventGroupCreate();
	// xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED);

	// CMN_TIMESET = BIT0;
	// OTA_START = BIT1;
	// common_evnet_group = xEventGroupCreate();
	// xEventGroupClearBits(common_evnet_group, CMN_TIMESET);
	// xEventGroupClearBits(common_evnet_group, OTA_START);

	// EVENT_FAULT_LEC = BIT0;
	// EVENT_FAULT_LEC_SENT = BIT1;
	// EVENT_FAULT_LEO = BIT2;
	// EVENT_FAULT_LEO_SENT = BIT3;
	// EVENT_FAULT_ZCR = BIT4;
	// EVENT_FAULT_ZCR_SENT = BIT5;
	// hardware_evnet_group = xEventGroupCreate();
	// xEventGroupClearBits(hardware_evnet_group,
	// 		EVENT_FAULT_LEC | EVENT_FAULT_LEC_SENT |
	// 		EVENT_FAULT_LEO | EVENT_FAULT_LEO_SENT |
	// 		EVENT_FAULT_ZCR | EVENT_FAULT_ZCR_SENT );

	//Setting queue
	// xQueueAction = xQueueCreate(32, sizeof(xMsgReceived));
	// xQueuePublish = xQueueCreate(32, sizeof(xMsgPublish));

	// //Setting the semaphore
	// xTZBufferMutex = xSemaphoreCreateMutex();
	// xOTABlock = xSemaphoreCreateMutex();

	file_operation_mutex = xSemaphoreCreateMutex();

	// xSemaphoreTake(xOTABlock, portMAX_DELAY);

	//SSID password
	strcpy(deviceAPPass, "password");

	//Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// //I2c Setup
	// ret = i2c_master_init();
	// if(ret != ESP_OK)
	// {
	// 	ESP_LOGE("", "Error in initalisation of I2C = %d", ret);
	// }

	// //Init PCA9555
	// ret = xPCA9555ConfigureP0(0xFF); //P0 as input
	// if(ret != ESP_OK)
	// {
	// 	ESP_LOGE("", "Error in initialization of PCA9555 P0 = %d", ret);
	// }

	// ret = xPCA9555ConfigureP1(0x00); //P1 as output
	// if(ret != ESP_OK)
	// {
	// 	ESP_LOGE("", "Error in initialization of PCA9555 P1 = %d", ret);
	// }

	// xPCA9555WriteP1(0x00);

	//Relay structure clean
	memset(xObjRelay, 0, sizeof(xObjRelay));

	//Relay initialization
	xObjRelay[0].gpioNo = RELAY_1_PIN;
	xObjRelay[1].gpioNo = RELAY_2_PIN;
	xObjRelay[2].gpioNo = RELAY_3_PIN;
	xObjRelay[3].gpioNo = RELAY_4_PIN;
	xObjRelay[4].gpioNo = RELAY_5_PIN;
	xObjRelay[5].gpioNo = RELAY_6_PIN;
	xObjRelay[6].gpioNo = RELAY_7_PIN;
	xObjRelay[7].gpioNo = RELAY_8_PIN;

	for(uint8_t i = 0; i < MAX_RELAY; i++)
	{
		gpio_pad_select_gpio(xObjRelay[i].gpioNo);
		gpio_set_direction(xObjRelay[i].gpioNo, GPIO_MODE_OUTPUT);
		// gpio_set_level(xObjRelay[i].gpioNo, 0x01);
	}
	
	// //Switch initialization
	// memset(xObjSwitch, 0, sizeof(xObjSwitch));
	// xObjSwitch[0].gpioNo = SW1; 	xObjSwitch[0].display = 1;
	// xObjSwitch[1].gpioNo = SW2; 	xObjSwitch[1].display = 2;
	// xObjSwitch[2].gpioNo = SW3; 	xObjSwitch[2].display = 3;
	// xObjSwitch[3].gpioNo = SW4; 	xObjSwitch[3].display = 4;
	// xObjSwitch[4].gpioNo = SW5; 	xObjSwitch[4].display = 5;

	// xObjRSTSwitch.gpioNo = SWRST;	
	// xObjRSTSwitch.display = 19;

	// xObjRGBLED[0].red = RL1;	xObjRGBLED[1].red = RL2;
	// xObjRGBLED[0].green = GL1;	xObjRGBLED[1].green = GL2;
	// xObjRGBLED[0].blue = BL1;	xObjRGBLED[1].blue = BL2;

	// initSwitch();
	// for(uint8_t i = 0; i < MAX_RGBLED; i++)
	// {
		gpio_pad_select_gpio(WIFI_CONNECT_LED_PIN); gpio_set_direction(WIFI_CONNECT_LED_PIN, GPIO_MODE_OUTPUT);
		gpio_pad_select_gpio(CLIENT_CONNECT_LED_PIN); gpio_set_direction(CLIENT_CONNECT_LED_PIN, GPIO_MODE_OUTPUT);
		gpio_pad_select_gpio(COMM_STATUS_LED_PIN); gpio_set_direction(COMM_STATUS_LED_PIN, GPIO_MODE_OUTPUT);
		gpio_pad_select_gpio(RESET_INPUT_PIN); gpio_set_direction(RESET_INPUT_PIN, GPIO_MODE_INPUT);
		
	// }

	//MCP79410 Initialization
	// memset(&xObjMCP79410RTC, 0 ,sizeof(xObjMCP79410RTC));
	// ret = xInitMCP79410();
	// if(ret != ESP_OK)
	// {
	// 	ESP_LOGE("", "Error in initialization of MCP79410");
	// }

	// //RGB LED enable GPIO setting
	// gpio_pad_select_gpio(ENRGB);
	// gpio_set_direction(ENRGB, GPIO_MODE_OUTPUT);

	// for(uint8_t i = 0; i < MAX_RGBLED; i++)
	// {
		gpio_set_level(WIFI_CONNECT_LED_PIN, LED_OFF);
		gpio_set_level(CLIENT_CONNECT_LED_PIN, LED_OFF);
		gpio_set_level(COMM_STATUS_LED_PIN, LED_OFF);
		
	// }

	// gpio_set_level(ENRGB, 1);

	ESP_LOGE("", "Initialization is completed");

}

// char *getDeviceName()
// {
// 	return deviceName;
// }

// char *getDevicePass()
// {
// 	return deviceAPPass;
// }

void actionOnFactoryReset()
{
	memset(&xObjProvData, 0, sizeof(xObjProvData));
	update_prov_data();
}
