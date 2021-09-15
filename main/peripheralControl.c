/*
 * peripheralControl.c
 *
 *  Created on: 08-Oct-2019
 *      Author: shiv
 */

#include "peripheralControl.h"

#include "commondef.h"
#include "mqtt.h"
#include "cmdAction.h"
#include "peripheralInit.h"
#include "data-storage.h"

#include <time.h>
#include <sys/time.h>

const char port_control_string[] = "{\"cmd\":\"set_port_status\", \"port\":%d,"
								   "\"value\":%d, \"on_time\":%ld}";
xMsgPublish xMsg;

uint8_t u8DisplaySeq = 0;
xSemaphoreHandle xDisplaySeqChangeBusy;

//3 second display rotator
TickType_t xStartTime3Sec;

int sentNotificationLEO = 0;
int sentNotificationLEC = 0;
int sentNotificationOTR = 0;

static xSemaphoreHandle xLockSwitch = NULL;

int cntLEO = 0;
int cntLEC = 0;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
	uint32_t gpio_num = (uint32_t)arg;
	if (gpio_num == LEC)
		cntLEO++;
	if (gpio_num == LEO)
		cntLEC++;
}

void initSwitch()
{
	xLockSwitch = xSemaphoreCreateMutex();
	if (xLockSwitch == NULL)
	{
		ESP_LOGE("", "Error in creating Mutex: xLockSwitch");
	}

	gpio_config_t io_conf;
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	io_conf.pin_bit_mask = ((1ULL << xObjSwitch[0].gpioNo) |
							(1ULL << xObjSwitch[1].gpioNo) |
							(1ULL << xObjSwitch[2].gpioNo) |
							(1ULL << xObjSwitch[3].gpioNo) |
							(1ULL << xObjRSTSwitch.gpioNo));
	io_conf.intr_type = GPIO_INTR_DISABLE;
	if (gpio_config(&io_conf) == ESP_OK)
		ESP_LOGI("", "GPIO Switch -- Initialized");
	else
		ESP_LOGE("", "Failed to set GPIO for switch");

	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 1;
	io_conf.pin_bit_mask = ((1ULL << xObjSwitch[4].gpioNo));
	io_conf.intr_type = GPIO_INTR_DISABLE;
	if (gpio_config(&io_conf) == ESP_OK)
		ESP_LOGI("", "GPIO Switch -- Initialized");
	else
		ESP_LOGE("", "Failed to set GPIO for switch");
	//gpio_pullup_en(1ULL << xObjSwitch[4].gpioNo);
}

void initOtherInputs(void)
{
	gpio_config_t io_conf;
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	io_conf.pin_bit_mask = ((1ULL << ZCD) |
							(1ULL << LEO) |
							(1ULL << LEC));
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.intr_type = GPIO_INTR_NEGEDGE;

	if (gpio_config(&io_conf) == ESP_OK)
		ESP_LOGI("", "Other Inputs -- Initialized");
	else
		ESP_LOGE("", "Failed to set GPIO for Other Inputs");

	gpio_install_isr_service(0);
	if (gpio_isr_handler_add(LEO, gpio_isr_handler, (void *)LEO) != ESP_OK)
	{
		ESP_LOGE("", "Error in setting Interrupt for LEO");
	}
	if (gpio_isr_handler_add(LEC, gpio_isr_handler, (void *)LEC) != ESP_OK)
	{
		ESP_LOGE("", "Error in setting Interrupt for LEC");
	}
}

void changeDisplaySeq(uint8_t StartPoint)
{
	xSemaphoreTake(xDisplaySeqChangeBusy, portMAX_DELAY);
	u8DisplaySeq = StartPoint;
	xStartTime3Sec = xTaskGetTickCount();
	xSemaphoreGive(xDisplaySeqChangeBusy);
}

esp_err_t setRelay(uint8_t relayNo, uint8_t val)
{
	TickType_t startTime, currentTime, waitTicks;
	waitTicks = pdMS_TO_TICKS(2000);
	startTime = xTaskGetTickCount();
	currentTime = startTime;

	do
	{
		if (gpio_get_level(ZCD))
		{
			gpio_set_level(relayNo, (val && 0x01));
			ESP_LOGI("", "Relay is operated successfully");
			xEventGroupClearBits(hardware_evnet_group, EVENT_FAULT_ZCR | EVENT_FAULT_ZCR_SENT);
			return ESP_OK;
		}
		currentTime = xTaskGetTickCount();
	} while ((currentTime - startTime) < waitTicks);

	ESP_LOGE("", "\nError in setting GPIO Level\n");
	xEventGroupSetBits(hardware_evnet_group, EVENT_FAULT_ZCR);
	return ESP_FAIL;
}

void waitingForInActivitiy()
{
	ESP_LOGI("", "Waiting for inactivity");
}

void peripheralControlTask(void *pV)
{
	ESP_LOGI("", "Peripheral control task");
	ESP_LOGI("", "Waiting for the time to sync");
	xEventGroupWaitBits(common_evnet_group, CMN_TIMESET, false, true, portMAX_DELAY);
	ESP_LOGI("", "Time is synchronized");

	sendRelayStatus = 0;

	time_t unixTimeNow;
	//time_t unixTimeScheduler;
	uint8_t relayIndex = 0;
	//uint8_t day;
	//struct tm localTimeStruct;
	//time_t turnofftime;
	uint8_t u8RGBIndex = 0;

	TickType_t xRGBStartTimeBlink;
	TickType_t xRGBStartTimeBlinkFast;
	TickType_t xRGBGreenBlinkSW34;

	TickType_t xRGBCurrentTimeBlink;

	uint8_t u8RGBOnOff;
	uint8_t u8RGBOnOffFast;

	TickType_t xRGBTicksTimeout = pdMS_TO_TICKS(RGB_TIMEOUT * 1000);
	TickType_t xRGBTicksTimeoutFast = pdMS_TO_TICKS(RGB_TIMEOUT * 500);
	TickType_t xRGBGreenBlinkSW34Timout = pdMS_TO_TICKS(500);

	u8RGBOnOff = 0;
	u8RGBOnOffFast = 0;

	uint8_t sw3_4_green_blink_on = 0;

	uint8_t port3_previous_state;
	uint8_t port4_previous_state;

	xRGBStartTimeBlink = xTaskGetTickCount();
	xRGBCurrentTimeBlink = xRGBStartTimeBlink;
	xRGBStartTimeBlinkFast = xRGBStartTimeBlink;
	xRGBGreenBlinkSW34 = xRGBStartTimeBlink;

	port3_previous_state = gpio_get_level(xObjRelay[2].gpioNo);
	port4_previous_state = gpio_get_level(xObjRelay[3].gpioNo);

	struct tm tm_current_unix_time;
	time_t current_unix_time;
	struct tm tm_scheduler_unix_start_time;
	time_t scheduler_unix_start_time;
	sched_def scheduler_current;

	while (1)
	{
		time(&unixTimeNow);
		for (relayIndex = 0; relayIndex < MAX_RELAY; relayIndex++)
		{
			if (gpio_get_level(xObjRelay[relayIndex].gpioNo) == 1)
			{
				//ESP_LOGI("", "Relay %d Offtime %ld current time %ld", relayIndex+1, xObjRelay[relayIndex].offTime, unixTimeNow);
				if ((uint32_t)xObjRelay[relayIndex].offTime < (uint32_t)unixTimeNow)
				{
					setRelay(xObjRelay[relayIndex].gpioNo, 0);
					xObjRelay[relayIndex].mode = 0;
					for (int x = 0; x < MAX_SCHED; x++)
						xObjRelay[relayIndex].emergencyOff[x] = 0;

					ESP_LOGI("", "Turning off Relay %d", relayIndex + 1);
					sendRelayStatus = 1;
				}
			}
		}

		//Scheduler polling
		if (xEventGroupGetBits(common_evnet_group) && CMN_TIMESET)
		{
			time(&current_unix_time);
			localtime_r(&current_unix_time, &tm_current_unix_time);
			for (int scheduler_number = 1; scheduler_number <= MAX_SCHED; scheduler_number++)
			{
				scheduler_read(scheduler_number, &scheduler_current);

				if ((scheduler_current.port > 0) && (scheduler_current.port <= MAX_RELAY) && (scheduler_current.enable == 1))
				{
					//					ESP_LOGI("", "Scheduler %d Enabled %d hh %d mm %d dur %d port %d",
					//							scheduler_number, scheduler_current.enable,
					//							scheduler_current.start_time_HH, scheduler_current.start_time_MM,
					//							scheduler_current.duration, scheduler_current.port);
					relayIndex = scheduler_current.port - 1;
					if ((gpio_get_level(xObjRelay[relayIndex].gpioNo) == 0) && (scheduler_current.day & (1 << tm_current_unix_time.tm_wday)))
					{
						memcpy((void *)&tm_scheduler_unix_start_time, (void *)&tm_current_unix_time, sizeof(struct tm));

						tm_scheduler_unix_start_time.tm_hour = scheduler_current.start_time_HH;
						tm_scheduler_unix_start_time.tm_min = scheduler_current.start_time_MM;
						tm_scheduler_unix_start_time.tm_sec = 0;
						scheduler_unix_start_time = mktime(&tm_scheduler_unix_start_time);

						//						ESP_LOGI("", "Starttime = %ld, EndTime = %ld, current time = %ld",
						//								scheduler_unix_start_time, scheduler_unix_start_time + (scheduler_current.duration*60),
						//								current_unix_time);
						if ((scheduler_unix_start_time <= current_unix_time) &&
							((scheduler_unix_start_time + (scheduler_current.duration * 60)) >= current_unix_time))
						{
							if (xObjPumpLOCK.lock == 0)
							{

								if ((xObjRelay[relayIndex].emergencyOff[scheduler_number - 1] == 0))
								{
									xObjRelay[relayIndex].offTime = scheduler_unix_start_time + (scheduler_current.duration * 60);
									setRelay(xObjRelay[relayIndex].gpioNo, 1);
									xObjRelay[relayIndex].mode = 3;
									xObjRelay[relayIndex].emergencyOff[scheduler_number - 1] = 0;
									sendRelayStatus = 1;
								}
							}
						}
						else
						{
							xObjRelay[relayIndex].emergencyOff[scheduler_number - 1] = 0;
						}
					}
				}
			}
		}

		/*
		for(relayIndex = 0; relayIndex < MAX_RELAY; relayIndex++)
		{
			//ESP_LOGI("---", "Relay = %d Active = %d", relayIndex, xObjRLScheduler[relayIndex].active);
			if( (xObjRLScheduler[relayIndex].active == 1) && (gpio_get_level(xObjRelay[relayIndex].gpioNo) == 0) )
			{
				//Get localtime day
				//setenv("TZ", xObjRLScheduler[relayIndex].timezone, 1);
				//tzset();
				xSemaphoreTake(xTZBufferMutex, portMAX_DELAY);
				localtime_r(&unixTimeNow, &localTimeStruct);
				xSemaphoreGive(xTZBufferMutex);

				day = (uint8_t)localTimeStruct.tm_wday;
				if(xObjRLScheduler[relayIndex].duration[day] > 0)
				{
					localTimeStruct.tm_hour = xObjRLScheduler[relayIndex].hh[day];
					localTimeStruct.tm_min = xObjRLScheduler[relayIndex].mm[day];
					localTimeStruct.tm_sec = 0;

					unixTimeScheduler = mktime(&localTimeStruct);
					//ESP_LOGI("", "%ld  %ld", unixTimeNow, unixTimeScheduler);
					turnofftime = unixTimeScheduler + xObjRLScheduler[relayIndex].duration[day];
					//ESP_LOGI("", "Relay=%d %ld  %ld %ld %ld", relayIndex, unixTimeNow, unixTimeScheduler, turnofftime, xObjRLScheduler[relayIndex].duration[day]);
					if( (unixTimeNow > unixTimeScheduler) && (unixTimeNow < turnofftime ))
					{
						if(xObjRelay[relayIndex].emergencyOff == 0 )
						{
							xObjRelay[relayIndex].offTime = unixTimeScheduler + xObjRLScheduler[relayIndex].duration[day];
							setRelay(xObjRelay[relayIndex].gpioNo, 1);
							xObjRelay[relayIndex].mode = 3;
							xObjRelay[relayIndex].emergencyOff = 0;
							sendRelayStatus = 1;
						}
					}
					else
					{
						xObjRelay[relayIndex].emergencyOff = 0;
					}
				}
			}
		}
		 */

		if (sendRelayStatus == 1)
		{
			sendRelayStatus = 0;
			on_get_port_status(NULL);
		}

		xRGBCurrentTimeBlink = xTaskGetTickCount();
		if ((xRGBCurrentTimeBlink - xRGBStartTimeBlink) > xRGBTicksTimeout)
		{
			xRGBStartTimeBlink = xRGBCurrentTimeBlink;
			u8RGBOnOff = ~u8RGBOnOff;
		}

		if ((xRGBCurrentTimeBlink - xRGBStartTimeBlinkFast) > xRGBTicksTimeoutFast)
		{
			xRGBStartTimeBlinkFast = xRGBCurrentTimeBlink;
			u8RGBOnOffFast = ~u8RGBOnOffFast;
		}

		if (port3_previous_state != gpio_get_level(xObjRelay[2].gpioNo))
		{
			if (sw3_4_green_blink_on == 0)
			{
				xRGBGreenBlinkSW34 = xRGBCurrentTimeBlink;
				sw3_4_green_blink_on = 1;
			}
		}

		if (port4_previous_state != gpio_get_level(xObjRelay[3].gpioNo))
		{
			if (sw3_4_green_blink_on == 0)
			{
				xRGBGreenBlinkSW34 = xRGBCurrentTimeBlink;
				sw3_4_green_blink_on = 1;
			}
		}

		port3_previous_state = gpio_get_level(xObjRelay[2].gpioNo);
		port4_previous_state = gpio_get_level(xObjRelay[3].gpioNo);

		if ((xRGBCurrentTimeBlink - xRGBGreenBlinkSW34) > xRGBGreenBlinkSW34Timout)
		{
			xRGBGreenBlinkSW34 = xRGBCurrentTimeBlink;
			if (sw3_4_green_blink_on != 0)
				sw3_4_green_blink_on++;
			if (sw3_4_green_blink_on > 4)
				sw3_4_green_blink_on = 0;
		}

		gpio_set_level(xObjRGBLED[0].blue, RGBOFF);
		gpio_set_level(xObjRGBLED[1].blue, RGBOFF);

		u8RGBIndex = 0;
		switch (xObjRelay[u8RGBIndex].mode)
		{
		case 1:
			gpio_set_level(xObjRGBLED[u8RGBIndex].red, RGBOFF);
			gpio_set_level(xObjRGBLED[u8RGBIndex].blue, RGBOFF);

			if (sw3_4_green_blink_on)
			{
				if (sw3_4_green_blink_on & 0x01)
				{
					gpio_set_level(xObjRGBLED[u8RGBIndex].green, RGBON);
				}
				else
				{
					gpio_set_level(xObjRGBLED[u8RGBIndex].green, RGBOFF);
				}
			}
			else
				gpio_set_level(xObjRGBLED[u8RGBIndex].green, RGBON);
			break;

		case 2:
		case 3:
			gpio_set_level(xObjRGBLED[u8RGBIndex].red, RGBOFF);
			gpio_set_level(xObjRGBLED[u8RGBIndex].blue, RGBOFF);
			if (sw3_4_green_blink_on)
			{
				if (sw3_4_green_blink_on & 0x01)
				{
					gpio_set_level(xObjRGBLED[u8RGBIndex].green, RGBON);
				}
				else
				{
					gpio_set_level(xObjRGBLED[u8RGBIndex].green, RGBOFF);
				}
			}
			else
			{
				if ((xObjRelay[u8RGBIndex].offTime - unixTimeNow) < 120)
				{
					if (u8RGBOnOffFast)
					{
						gpio_set_level(xObjRGBLED[u8RGBIndex].green, RGBON);
					}
					else
					{
						gpio_set_level(xObjRGBLED[u8RGBIndex].green, RGBOFF);
					}
				}
				else
				{
					if (u8RGBOnOff)
					{
						gpio_set_level(xObjRGBLED[u8RGBIndex].green, RGBON);
					}
					else
					{
						gpio_set_level(xObjRGBLED[u8RGBIndex].green, RGBOFF);
					}
				}
			}
			break;
		default:
			gpio_set_level(xObjRGBLED[u8RGBIndex].red, RGBOFF);
			gpio_set_level(xObjRGBLED[u8RGBIndex].green, RGBOFF);
			gpio_set_level(xObjRGBLED[u8RGBIndex].blue, RGBON);
			break;
		}

		u8RGBIndex = 1;
		if ((xEventGroupGetBits(hardware_evnet_group) & EVENT_FAULT_LEC) |
			(xEventGroupGetBits(hardware_evnet_group) & EVENT_FAULT_LEO))
		{
			gpio_set_level(xObjRGBLED[u8RGBIndex].green, RGBOFF);
			gpio_set_level(xObjRGBLED[u8RGBIndex].blue, RGBOFF);
			if (u8RGBOnOffFast)
			{
				gpio_set_level(xObjRGBLED[u8RGBIndex].red, RGBON);
			}
			else
			{
				gpio_set_level(xObjRGBLED[u8RGBIndex].red, RGBOFF);
			}
		}
		else
		{
			switch (xObjRelay[u8RGBIndex].mode)
			{
			case 1:
				gpio_set_level(xObjRGBLED[u8RGBIndex].red, RGBOFF);
				gpio_set_level(xObjRGBLED[u8RGBIndex].blue, RGBOFF);
				gpio_set_level(xObjRGBLED[u8RGBIndex].green, RGBON);

				break;
			case 2:
			case 3:
				gpio_set_level(xObjRGBLED[u8RGBIndex].red, RGBOFF);
				gpio_set_level(xObjRGBLED[u8RGBIndex].blue, RGBOFF);
				if ((xObjRelay[u8RGBIndex].offTime - unixTimeNow) < 120)
				{
					if (u8RGBOnOffFast)
					{
						gpio_set_level(xObjRGBLED[u8RGBIndex].green, RGBON);
					}
					else
					{
						gpio_set_level(xObjRGBLED[u8RGBIndex].green, RGBOFF);
					}
				}
				else
				{
					if (u8RGBOnOff)
					{
						gpio_set_level(xObjRGBLED[u8RGBIndex].green, RGBON);
					}
					else
					{
						gpio_set_level(xObjRGBLED[u8RGBIndex].green, RGBOFF);
					}
				}
				break;
			default:
				gpio_set_level(xObjRGBLED[u8RGBIndex].red, RGBOFF);
				gpio_set_level(xObjRGBLED[u8RGBIndex].green, RGBOFF);
				gpio_set_level(xObjRGBLED[u8RGBIndex].blue, RGBON);
				break;
			}
		}

		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

uint8_t pumpSwitch = 0x00;

void sevenSegmentControlTask(void *pV)
{
	//Initialize semaphore
	xDisplaySeqChangeBusy = xSemaphoreCreateMutex();
	if (xDisplaySeqChangeBusy == NULL)
	{
		ESP_LOGE("", "Error in the Creating Mutex : xDisplaySeqChangeBusy\n");
	}

	// 200msTicks
	TickType_t x200msTime = pdMS_TO_TICKS(200);
	//3 Seconds Ticks
	TickType_t x3SecTime = pdMS_TO_TICKS(3000);
	//10 seconds Ticks
	TickType_t x10SecTime = pdMS_TO_TICKS(10000);

	//Current Tick
	TickType_t xCurrentTime = xTaskGetTickCount();

	//Start Tick 200ms Timer
	TickType_t xStartTime200ms = xTaskGetTickCount();

	//Start Tick 3Second Timer
	xStartTime3Sec = xTaskGetTickCount();

	//start Tick 10seconds
	TickType_t xStartTime10Sec = xTaskGetTickCount();

	//dot blinkTurn
	uint8_t DotBinkTurn = 0x00;

	//Digit display Turn
	uint8_t DigitBlinkTurn = 0x00;

	//Display Switch
	uint8_t switchDisplay = 0x00;

	//Manual Relay
	uint8_t switchManualAction = 0x00;

	//Switch cmd buffer
	char switchManualActionCMD[256];

	//Pump1 Switches

	long int pumpOnTime;

	//Error
	uint8_t u8Err = 0xFF;

	//Error counts
	int cntLEO_PR = cntLEO;
	int cntLEC_PR = cntLEC;

	int u8Err_HF = 0x00;

	//Power ON Condition
	vShowFull();
	vTaskDelay(pdMS_TO_TICKS(3000));

	//Index
	uint8_t Index = 0;

	uint8_t temp;

	while (1)
	{
		//ESP_LOGI("","sevenSegmentControlTask");
		vTaskDelay(pdMS_TO_TICKS(100)); //Task delay 100 ms

		//Dot blink value -- 100ms change every time
		DotBinkTurn = ~DotBinkTurn;
		DotBinkTurn &= 0x01;

		//Current time
		xCurrentTime = xTaskGetTickCount();

		//Display blink value -- 200ms
		if ((xCurrentTime - xStartTime200ms) >= x200msTime)
		{
			xStartTime200ms = xCurrentTime;
			DigitBlinkTurn = ~DigitBlinkTurn;
			DigitBlinkTurn &= 0x01;
		}

		//Switch input handling -- Start
		switchDisplay = 0x00;
		switchManualAction = 0x00;
		xSemaphoreTake(xLockSwitch, portMAX_DELAY);
		//for detection of switch pressed and displaying same on the 7-segment
		for (Index = 0; Index < MAX_SWITCH; Index++)
		{
			if (xObjSwitch[Index].currentState == 0)
			{
				switchDisplay = xObjSwitch[Index].display;
				break;
			}
		}
		for (Index = 0; Index < MAX_SWITCH; Index++)
		{
			if (xObjSwitch[Index].actionRequired == 1)
			{
				ESP_LOGI("", "Action required Detected on : %d", Index);
				switchManualAction = xObjSwitch[Index].display;
				xObjSwitch[Index].actionRequired = 0;
				break;
			}
		}
		xSemaphoreGive(xLockSwitch); //Switch input handling -- End

		//Switch input Action -- Start
		if (switchManualAction)
		{
			pumpOnTime = 0;
			temp = 0x00;
			u8Err = 0xFF;
			memset(switchManualActionCMD, 0, sizeof(switchManualActionCMD));
			if (switchManualAction <= MAX_RELAY)
			{
				if ((switchManualAction == 1) || (switchManualAction == 2))
				{
					if (xPCA9555ReadP0(&temp) != ESP_OK)
					{
						ESP_LOGE("", "Error in reading switch data");
					}
					else
					{
						pumpSwitch = ~temp;
						ESP_LOGE("--->", "DIPSwitch = 0x%X", pumpSwitch);
						//If pump switch is pressed
						if (switchManualAction == 1)
						{
							actionONPump1BySwitch = 1;
							pumpSwitch = (pumpSwitch) >> 5;
							ESP_LOGE("--->", "PumpSwitch = 0x%X", pumpSwitch);

							/*
							 * SW8		SW7		SW6		Minutes
							 * OFF		OFF		OFF		60
							 * OFF		OFF		ON		15
							 * OFF		ON		OFF		30
							 * OFF		ON		ON		20
							 * ON		OFF		OFF		45
							 * ON		OFF		ON		90
							 * ON		ON		OFF		120
							 * ON		ON		ON		Infi
							 */

							switch (pumpSwitch)
							{

							case 0:
								pumpOnTime = 60 * 60;
								ESP_LOGE("***", "Pump ON = 60 Min");
								break;

							case 1:
								pumpOnTime = 15 * 60;
								ESP_LOGE("***", "Pump ON = 15 Min");
								break;

							case 2:
								pumpOnTime = 30 * 60;
								ESP_LOGE("***", "Pump ON = 30 Min");
								break;

							case 3:
								pumpOnTime = 20 * 60;
								ESP_LOGE("***", "Pump ON = 20 Min");
								break;

							case 4:
								pumpOnTime = 45 * 60;
								ESP_LOGE("***", "Pump ON = 45 Min");
								break;

							case 5:
								pumpOnTime = 90 * 60;
								ESP_LOGE("***", "Pump ON = 90 Min");
								break;

							case 6:
								pumpOnTime = 120 * 60;
								ESP_LOGE("***", "Pump ON = 120 Min");
								break;

							default:
								pumpOnTime = 0;
								ESP_LOGE("***", "Pump ON = Infi--");
								break;
							}
						}

						/*
						 * SW3		SW2		SW1		Minutes
						 * OFF		OFF		OFF		60
						 * OFF		OFF		ON		180
						 * OFF		ON		OFF		120
						 * all other				Infi
						 */
						//If pool-light is pressed
						if (switchManualAction == 2)
						{

							pumpSwitch = (pumpSwitch)&0x03;
							ESP_LOGE("--->", "PumpSwitch = 0x%X", pumpSwitch);
							switch (pumpSwitch)
							{
							case 0:
								pumpOnTime = 60 * 60;
								ESP_LOGE("***", "Pool light ON = 60 Min");
								break;
							case 1:
								pumpOnTime = 180 * 60;
								ESP_LOGE("***", "Pool light ON = 180 Min");
								break;
							case 2:
								pumpOnTime = 120 * 60;
								ESP_LOGE("***", "Pool light ON = 120 Min");
								break;
							default:
								pumpOnTime = 0;
								ESP_LOGE("***", "Pool light ON = Infi time");
								break;
							}

						} // Pool light pressed -- End
					}
				}

				switch (switchManualAction)
				{
				case 1:
				case 2:
					if (gpio_get_level(xObjRelay[switchManualAction - 1].gpioNo) == 1)
					{
						sprintf(switchManualActionCMD, port_control_string, switchManualAction, 0, (long int)0);
						//sprintf(switchManualActionCMD, "%1d 0 0", switchManualAction);
					}
					else
					{
						sprintf(switchManualActionCMD, port_control_string, switchManualAction, 1, (long int)pumpOnTime);
						//sprintf(switchManualActionCMD, "%1d 1 %ld", switchManualAction, (long int)pumpOnTime);
					}
					break;
				case 3:
				case 4:
				case 5:
					if (gpio_get_level(xObjRelay[switchManualAction - 1].gpioNo) == 1)
					{
						sprintf(switchManualActionCMD, port_control_string, switchManualAction, 0, (long int)0);
						//sprintf(switchManualActionCMD, "%1d 0 0", switchManualAction);
					}
					else
					{
						sprintf(switchManualActionCMD, port_control_string, switchManualAction, 1, (long int)0);
						//sprintf(switchManualActionCMD, "%1d 1 0", switchManualAction);
					}
					break;

				default:
					break;
				}
			}
			on_set_relay_status(switchManualActionCMD);
		}

		if (switchDisplay) //If switch is pressed
		{

			if (DigitBlinkTurn)
				vShowHexDigit(switchDisplay);
			else
				vShowNoDigit();

			vShowDot(0);
			continue;
		}

		//Check for the fault condition if the timeout has occurred
		if ((xCurrentTime - xStartTime10Sec) >= x10SecTime)
		{
			xStartTime10Sec = xCurrentTime;

			if (cntLEC_PR == cntLEC)
				xEventGroupSetBits(hardware_evnet_group, EVENT_FAULT_LEC);
			else
				xEventGroupClearBits(hardware_evnet_group, EVENT_FAULT_LEC | EVENT_FAULT_LEC_SENT);
			cntLEC_PR = cntLEC;

			if (cntLEO_PR == cntLEO)
				xEventGroupSetBits(hardware_evnet_group, EVENT_FAULT_LEO);
			else
				xEventGroupClearBits(hardware_evnet_group, EVENT_FAULT_LEO | EVENT_FAULT_LEO_SENT);
			cntLEO_PR = cntLEO;

			u8Err_HF = 0x00;
			if ((xEventGroupGetBits(hardware_evnet_group) & EVENT_FAULT_LEC) ||
				(xEventGroupGetBits(hardware_evnet_group) & EVENT_FAULT_LEO))
			{
				u8Err_HF = 0x11;
			}

			if (xEventGroupGetBits(hardware_evnet_group) & EVENT_FAULT_ZCR)
			{
				u8Err_HF = 0x0E;
			}

			if ((xEventGroupGetBits(wifi_event_group) & WIFI_CONNECTED) &&
				(xEventGroupGetBits(mqtt_event_group) & MQTT_CONNECTED))
			{
				//ZCR fault
				if (xEventGroupGetBits(hardware_evnet_group) & EVENT_FAULT_ZCR)
				{
					if (xEventGroupGetBits(hardware_evnet_group) & EVENT_FAULT_ZCR_SENT)
					{
					}
					else
					{
						send_notification(3);
						xEventGroupSetBits(hardware_evnet_group, EVENT_FAULT_LEC_SENT);
					}
				}

				//LEC Fault
				if (xEventGroupGetBits(hardware_evnet_group) & EVENT_FAULT_LEC)
				{
					if (xEventGroupGetBits(hardware_evnet_group) & EVENT_FAULT_LEC_SENT)
					{
					}
					else
					{
						send_notification(2);
						xEventGroupSetBits(hardware_evnet_group, EVENT_FAULT_LEC_SENT);
					}
				}

				//LE0 Fault
				if (xEventGroupGetBits(hardware_evnet_group) & EVENT_FAULT_LEO)
				{
					if (xEventGroupGetBits(hardware_evnet_group) & EVENT_FAULT_LEO_SENT)
					{
					}
					else
					{
						send_notification(1);
						xEventGroupSetBits(hardware_evnet_group, EVENT_FAULT_LEO_SENT);
					}
				}
			}

		} //Check for the fault condition - END

		//Normal Condition
		//Display rotator timer
		xSemaphoreTake(xDisplaySeqChangeBusy, portMAX_DELAY);
		if ((xCurrentTime - xStartTime3Sec) >= x3SecTime)
		{
			xStartTime3Sec = xCurrentTime;
			for (Index = 0; Index <= (MAX_RELAY + 1); Index++)
			{
				u8DisplaySeq++;
				//				if(u8DisplaySeq == 3) u8DisplaySeq++;
				//				if(u8DisplaySeq == 4) u8DisplaySeq++;
				if (u8DisplaySeq > (MAX_RELAY + 1))
				{
					u8DisplaySeq = 0;
				}
				if (u8DisplaySeq == MAX_RELAY)
				{
					if (xEventGroupGetBits(common_evnet_group) & OTA_START)
					{
						u8Err = 0x13; //Display ---
						break;
					}
					if (xEventGroupGetBits(wifi_event_group) & AP_ON)
					{
						u8Err = 0x12; //Display o
						break;
					}
					if (xEventGroupGetBits(wifi_event_group) & PROV_FAIL_CONNECT_WIFI)
					{
						u8Err = 0x10; //Display P
						break;
					}
					else if (xEventGroupGetBits(wifi_event_group) & PROV_CLOUD_FAIL)
					{
						u8Err = 0x00; // DISPLAY O
						break;
					}
					else if (xObjPumpLOCK.lock == 1)
					{
						u8Err = 0x14; // DISPLAY c
						break;
					}
					else if (0)
					{
						//Hardware error
					}
					else
					{
						u8Err = 0xFF;
					}

					//Check for the error
				}
				else if (u8DisplaySeq == (MAX_RELAY + 1))
				{
					if (u8Err_HF)
						break;
				}
				else
				{
					if (xObjRelay[u8DisplaySeq].mode != 0)
					{
						break;
					}
				}
			}
			if (Index > (MAX_RELAY + 1))
				u8DisplaySeq = 0xFF;
		}

		if (u8DisplaySeq == MAX_RELAY)
		{

			if (u8Err != 0xFF)
			{
				vShowDot(0);
				if (DigitBlinkTurn)
					vShowHexDigit(u8Err);
				else
					vShowNoDigit();
			}
		}
		else if (u8DisplaySeq == (MAX_RELAY + 1))
		{
			//TODO: hardware fault
			if (u8Err_HF)
			{
				vShowDot(0);
				if (DigitBlinkTurn)
					vShowHexDigit(u8Err_HF);
				else
					vShowNoDigit();
			}
		}
		else if (u8DisplaySeq < MAX_RELAY)
		{
			switch (xObjRelay[u8DisplaySeq].mode)
			{
			case 1:
				vShowDot(0);
				vShowHexDigit(u8DisplaySeq + 1);
				break;
			case 2:
				vShowDot(0);
				if (DigitBlinkTurn)
					vShowHexDigit(u8DisplaySeq + 1);
				else
					vShowNoDigit();
				break;
			case 3:
				if (DigitBlinkTurn)
					vShowHexDigit(u8DisplaySeq + 1);
				else
					vShowNoDigit();
				if (DotBinkTurn)
					vShowDot(1);
				else
					vShowDot(0);
				break;
			default:
				break;
			}
		}
		else
		{
			if (DigitBlinkTurn)
				vShowDash();
			else
				vShowBlank();
		}
		xSemaphoreGive(xDisplaySeqChangeBusy);
	}
}

void SwitchMonitorTask(void *pV)
{
	TickType_t delay = pdMS_TO_TICKS(50);
	TickType_t x10Sec = pdMS_TO_TICKS(10000);

	//start Tick 10Seconds -- RST switch
	TickType_t xStartTimeRST10Sec;
	//Current time
	TickType_t xCurrentTime = xTaskGetTickCount();

	xStartTimeRST10Sec = xCurrentTime;
	while (1)
	{
		xSemaphoreTake(xLockSwitch, portMAX_DELAY);
		for (int i = 0; i < MAX_SWITCH; i++)
		{
			xObjSwitch[i].presentState = xObjSwitch[i].currentState;
			xObjSwitch[i].currentState = gpio_get_level(xObjSwitch[i].gpioNo);
			if ((xObjSwitch[i].currentState != xObjSwitch[i].presentState) && (xObjSwitch[i].currentState == 0))
			{
				xObjSwitch[i].actionRequired = 1;
				ESP_LOGI("", "Switch is Pressed : %d", xObjSwitch[i].gpioNo);
			}
		}
		xSemaphoreGive(xLockSwitch);
		xObjRSTSwitch.currentState = gpio_get_level(xObjRSTSwitch.gpioNo);
		//ESP_LOGI("", "%d", xObjRSTSwitch.currentState);
		if ((xObjRSTSwitch.currentState == 0))
		{
			xCurrentTime = xTaskGetTickCount();
			
			if ((xCurrentTime - xStartTimeRST10Sec) > x10Sec)
			{
				xStartTimeRST10Sec = xCurrentTime;
				if ((pumpSwitch & (1 << 4)))
				{
					ESP_LOGE("", "\n\nTime to factory reset the device\n\n");
					on_factory_reset(NULL);
				}
				else
				{
					ESP_LOGE("", "\n\nTime to change clean mode\n\n");
					if(xObjPumpLOCK.lock == 1)
					{
						xObjPumpLOCK.lock = 0;
					}
					else
					{
						xObjPumpLOCK.lock = 1;
					}
					update_pump_lock_data(&xObjPumpLOCK);
					on_get_pump_lock(NULL);
				}
			}
		}
		else
		{
			xStartTimeRST10Sec = xTaskGetTickCount();
		}

		vTaskDelay(delay);
	}
}
void zcrTask(void *pV)
{
	int delay;
	delay = pdMS_TO_TICKS(10);
	int i = 0;
	vTaskDelay(pdMS_TO_TICKS(2 * 60 * 1000));
	while (1)
	{
		for (i = 0; i < MAX_RELAY; i++)
		{
		}
		cntLEC++;
		cntLEO++;

		vTaskDelay(delay);
	}
}
