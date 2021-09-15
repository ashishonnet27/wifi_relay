/*
 * cmdAction.c
 *
 *  Created on: 07-Oct-2019
 *      Author: shiv
 */

#include "commondef.h"
#include "cmdAction.h"
#include "mqtt.h"
#include "timesync.h"
#include "ota.h"
#include "peripheralControl.h"
#include "peripheralInit.h"
#include "data-storage.h"
#include "cJSON.h"

const char key_cmd[] = "cmd";
const char key_res[] = "res";
const char key_status[] = "status";
const char key_time[] = "time";
const char key_value[] = "value";

const char key_device_type[] = "device_type";
const char key_timezone[] = "tz";
const char key_fw_version[] = "fw";
const char key_ssid[] = "ssid";
const char key_ontime[] = "on_time";
const char key_port[] = "port";
const char key_remaining_time[] = "r_time";

const char key_scheduler_number[] = "sched";
const char key_scheduler_hh[] = "hh";
const char key_scheduler_mm[] = "mm";
const char key_scheduler_en[] = "enable";
const char key_scheduler_active_days[] = "days";
const char key_ota_file[] = "ota_file";

const char msg_offline[] = "{\"Status\":\"Offline\"}";

const char cmd_get_rtc_voltage[] = "get_rtc_voltage";
const char cmd_get_device_type[] = "get_device_type";
const char cmd_get_device_timezone[] = "get_timezone";
const char cmd_set_device_timezone[] = "set_timezone";
const char cmd_get_device_fw[] = "get_fw_version";
const char cmd_reboot[] = "reboot";
const char cmd_get_wifi_ssid[] = "wifi_ssid";
const char cmd_clear_wifi_cred[] = "wifi_clear";
const char cmd_get_device_time[] = "get_device_time";
const char cmd_set_device_time[] = "set_device_time";
const char cmd_get_port_status[] = "get_port_status";
const char cmd_set_port_status[] = "set_port_status";
const char cmd_get_port_time_to_off[] = "get_port_time_to_off";
const char cmd_off_all_relay[] = "set_all_off";
const char cmd_get_pump_lock[] = "get_clean_mode";
const char cmd_set_pump_lock[] = "set_clean_mode";
const char cmd_set_scheduler[] = "set_scheduler";
const char cmd_get_scheduler[] = "get_scheduler";
const char cmd_get_setting_switch[] = "get_setting_switch";
const char cmd_start_ota[] = "ota";
const char cmd_factory_reset[] = "factory_reset";
const char cmd_reset_scheduler[] = "sched_reset";

xCmdAction action_map[] = {
	{(char *)cmd_get_rtc_voltage, on_get_rtc_voltage, 1},
	{(char *)cmd_get_device_type, on_get_device_type, 1},
	{(char *)cmd_get_device_timezone, on_get_device_time_zone, 1},
	{(char *)cmd_set_device_timezone, on_set_device_time_zone, 1},
	{(char *)cmd_get_device_fw, on_get_device_fw_version, 1},
	{(char *)cmd_reboot, on_device_reboot, 1},
	{(char *)cmd_get_wifi_ssid, on_get_wifi_ssid, 1},
	{(char *)cmd_clear_wifi_cred, on_wifi_clear, 1},
	{(char *)cmd_get_device_time, on_get_device_time, 1},
	{(char *)cmd_set_device_time, on_set_device_time, 1},
	{(char *)cmd_get_port_status, on_get_port_status, 1},
	{(char *)cmd_set_port_status, on_set_relay_status, 1},
	{(char *)cmd_get_port_time_to_off, get_remaining_on_time, 1},
	{(char *)cmd_off_all_relay, on_off_all, 1},
	{(char *)cmd_get_pump_lock, on_get_pump_lock, 1},
	{(char *)cmd_set_pump_lock, on_set_pump_lock, 1},
	{(char *)cmd_set_scheduler, on_set_scheduler, 1},
	{(char *)cmd_get_scheduler, on_get_scheduler, 1},
	{(char *)cmd_get_setting_switch, on_get_switch_status, 1},
	{(char *)cmd_start_ota, on_ota_start, 1},
	{(char *)cmd_factory_reset, on_factory_reset, 1},
	{(char *)cmd_reset_scheduler, on_sched_reset, 1},
	{NULL, NULL, 1}};

void msgParser(char *pMsg)
{
	xCmdAction *px;
	px = action_map;
	cJSON *msg = cJSON_Parse(pMsg);
	if (msg == NULL)
	{
		ESP_LOGI("", "Error in mesg JSON");
		return;
	}
	if (cJSON_HasObjectItem(msg, key_cmd))
	{
		while (1)
		{
			if ((px->cmd == NULL) && (px->cb == NULL))
			{
				break;
			}
			if (0 == strcmp(px->cmd, cJSON_GetObjectItem(msg, key_cmd)->valuestring))
			{
				if (px->cb != NULL)
				{
					px->cb(pMsg);
				}
			}

			px++;
		}
	}

	cJSON_Delete(msg);
}

void on_get_rtc_voltage(char *pMsg)
{
	time_t current_time;
	uint8_t status = 0;
	xMsgPublish response_mqtt;
	time(&current_time);

	cJSON *out_msg = cJSON_CreateObject();
	if (out_msg)
	{
		cJSON_AddStringToObject(out_msg, key_res, cmd_get_rtc_voltage);
		cJSON_AddNumberToObject(out_msg, key_status, status);
		cJSON_AddNumberToObject(out_msg, key_time, current_time);
		cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
		cJSON_Delete(out_msg);
	}

	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}

void on_get_device_type(char *pMsg)
{
	time_t current_time;
	uint8_t status = 1;
	xMsgPublish response_mqtt;
	time(&current_time);

	cJSON *out_msg = cJSON_CreateObject();
	if (out_msg)
	{
		cJSON_AddStringToObject(out_msg, key_res, cmd_get_device_type);
		cJSON_AddNumberToObject(out_msg, key_value, DEVICE_TYPE);
		cJSON_AddNumberToObject(out_msg, key_status, status);
		cJSON_AddNumberToObject(out_msg, key_time, current_time);
		cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
		cJSON_Delete(out_msg);
	}

	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}

void on_get_device_time_zone(char *pAvg)
{
	time_t current_time;
	uint8_t status = 1;
	xMsgPublish response_mqtt;
	time(&current_time);

	cJSON *out_msg = cJSON_CreateObject();
	if (out_msg)
	{
		cJSON_AddStringToObject(out_msg, key_res, cmd_get_device_timezone);
		cJSON_AddStringToObject(out_msg, key_timezone, device_timezone.tzone);
		cJSON_AddNumberToObject(out_msg, key_status, status);
		cJSON_AddNumberToObject(out_msg, key_time, current_time);
		cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
		cJSON_Delete(out_msg);
	}
	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}

void on_set_device_time_zone(char *pArg)
{
	time_t current_time;
	uint8_t status = 0;
	xMsgPublish response_mqtt;
	time(&current_time);

	//get timezone first
	cJSON *in_msg = cJSON_Parse(pArg);
	if (in_msg)
	{
		if (cJSON_HasObjectItem(in_msg, key_timezone))
		{
			xSemaphoreTake(xTZBufferMutex, portMAX_DELAY);
			memset(&device_timezone, 0, sizeof(device_time_zone_def));
			strcpy(device_timezone.tzone, cJSON_GetObjectItem(in_msg, key_timezone)->valuestring);
			set_device_time_zone(&device_timezone);
			status = 1;
			xSemaphoreGive(xTZBufferMutex);
		}
		cJSON_Delete(in_msg);
	}

	cJSON *out_msg = cJSON_CreateObject();
	if (out_msg)
	{
		cJSON_AddStringToObject(out_msg, key_res, cmd_set_device_timezone);
		cJSON_AddNumberToObject(out_msg, key_status, status);
		time(&current_time);
		cJSON_AddNumberToObject(out_msg, key_time, current_time);
		if (status)
		{
			cJSON_AddStringToObject(out_msg, key_timezone, device_timezone.tzone);
		}
		cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
		cJSON_Delete(out_msg);
	}
	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}

void on_get_device_fw_version(char *pArg)
{
	time_t current_time;
	uint8_t status = 1;
	xMsgPublish response_mqtt;
	time(&current_time);

	cJSON *out_msg = cJSON_CreateObject();
	if (out_msg)
	{
		cJSON_AddStringToObject(out_msg, key_res, cmd_get_device_fw);
		cJSON_AddNumberToObject(out_msg, key_device_type, DEVICE_TYPE);
		cJSON_AddStringToObject(out_msg, key_fw_version, VERSION_NUM);
		cJSON_AddNumberToObject(out_msg, key_time, current_time);
		cJSON_AddNumberToObject(out_msg, key_status, status);
		cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
		cJSON_Delete(out_msg);
	}

	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}

void on_device_reboot(char *pAvg)
{
	time_t current_time;
	uint8_t status = 1;
	xMsgPublish response_mqtt;
	time(&current_time);

	cJSON *out_msg = cJSON_CreateObject();
	if (out_msg)
	{
		cJSON_AddStringToObject(out_msg, key_res, cmd_reboot);
		cJSON_AddNumberToObject(out_msg, key_status, status);
		cJSON_AddNumberToObject(out_msg, key_time, current_time);
		cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
		cJSON_Delete(out_msg);
	}
	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);

	memset(&response_mqtt, 0, sizeof(xMsgPublish));
	sprintf(response_mqtt.msg, "%s", msg_offline);
	sprintf(response_mqtt.topic, "%s/Status", getDeviceName());
	response_mqtt.retain = 1;
	mqttPublishPushToQueue(&response_mqtt);

	vTaskDelay(pdMS_TO_TICKS(5000));
	esp_restart();
}

void on_get_wifi_ssid(char *pArg)
{
	time_t current_time;
	uint8_t status = 1;
	xMsgPublish response_mqtt;
	time(&current_time);

	cJSON *out_msg = cJSON_CreateObject();
	if (out_msg)
	{
		cJSON_AddStringToObject(out_msg, key_res, cmd_get_wifi_ssid);
		cJSON_AddStringToObject(out_msg, key_ssid, xObjProvData.ssid);
		cJSON_AddNumberToObject(out_msg, key_status, status);
		cJSON_AddNumberToObject(out_msg, key_time, current_time);
		cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
		cJSON_Delete(out_msg);
	}
	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}

void on_wifi_clear(char *pArg)
{
	time_t current_time;
	uint8_t status = 1;
	xMsgPublish response_mqtt;
	time(&current_time);

	xObjProvData.isProvisioned = 0;
	memset(xObjProvData.ssid, 0, sizeof(xObjProvData.ssid));
	memset(xObjProvData.password, 0, sizeof(xObjProvData.password));
	//update_prov_data();

	cJSON *out_msg = cJSON_CreateObject();
	if (out_msg)
	{
		cJSON_AddStringToObject(out_msg, key_res, cmd_clear_wifi_cred);
		cJSON_AddNumberToObject(out_msg, key_status, status);
		cJSON_AddNumberToObject(out_msg, key_time, current_time);
		cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
		cJSON_Delete(out_msg);
	}
	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}

void on_get_device_time(char *pArg)
{
	time_t current_time;
	uint8_t status = 1;
	xMsgPublish response_mqtt;
	time(&current_time);

	ESP_LOGI("", "Get Time is received");

	cJSON *out_msg = cJSON_CreateObject();
	if (out_msg)
	{
		cJSON_AddStringToObject(out_msg, key_res, cmd_get_device_time);
		cJSON_AddNumberToObject(out_msg, key_status, status);
		cJSON_AddNumberToObject(out_msg, key_time, current_time);
		cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
		cJSON_Delete(out_msg);
	}
	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}

void on_set_device_time(char *pArg)
{
	time_t user_time;

	time_t current_time;
	uint8_t status = 0;
	xMsgPublish response_mqtt;

	user_time = 0;

	cJSON *in_msg = cJSON_Parse(pArg);
	if (in_msg)
	{
		if (cJSON_HasObjectItem(in_msg, key_time))
		{
			user_time = (time_t)cJSON_GetObjectItem(in_msg, key_time)->valueint;
			status = 1;
		}

		cJSON_Delete(in_msg);
	}
	else
	{
		return;
	}

	ESP_LOGI("", "Status = %d time = %ld", status, user_time);
	if (status)
	{
		status = 0;
		if (xUpdateTimeInRTCManually((time_t)user_time) == ESP_OK)
		{
			if (xUpdateESPTimeFromRTC() == ESP_OK)
				status = 1;
		}
	}

	cJSON *out_msg = cJSON_CreateObject();
	if (out_msg)
	{
		time(&current_time);
		cJSON_AddStringToObject(out_msg, key_res, cmd_set_device_time);
		cJSON_AddNumberToObject(out_msg, key_status, status);
		cJSON_AddNumberToObject(out_msg, key_time, current_time);
		cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
		cJSON_Delete(out_msg);
	}

	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}

void on_get_port_status(char *pArg)
{
	time_t current_time;
	uint8_t status = 1;
	xMsgPublish response_mqtt;
	int status_array[MAX_RELAY];

	ESP_LOGI("", "Asking for relay status");

	for (int i = 0; i < MAX_RELAY; i++)
	{
		status_array[i] = xObjRelay[i].mode;
	}

	cJSON *out_msg = cJSON_CreateObject();
	if (out_msg)
	{
		time(&current_time);
		cJSON_AddStringToObject(out_msg, key_res, cmd_get_port_status);
		cJSON_AddNumberToObject(out_msg, key_status, status);
		cJSON_AddNumberToObject(out_msg, key_time, current_time);
		cJSON_AddItemToObject(out_msg, key_value, cJSON_CreateIntArray(status_array, MAX_RELAY));
		cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
		cJSON_Delete(out_msg);
	}

	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}

void on_set_relay_status(char *pArg)
{
	time_t current_time;
	uint8_t status = 1;
	xMsgPublish response_mqtt;

	int port = 0;
	int on_time_sec = 0;
	int action_to_taken = 0;

	cJSON *in_msg = cJSON_Parse(pArg);
	if (in_msg)
	{
		if (cJSON_HasObjectItem(in_msg, key_port) && cJSON_HasObjectItem(in_msg, key_value) && cJSON_HasObjectItem(in_msg, key_ontime))
		{
			port = cJSON_GetObjectItem(in_msg, key_port)->valueint;
			on_time_sec = cJSON_GetObjectItem(in_msg, key_ontime)->valueint;
			action_to_taken = cJSON_GetObjectItem(in_msg, key_value)->valueint;
			status = 1;
		}
		cJSON_Delete(in_msg);
	}
	else
	{
		return;
	}

	if (status == 1)
	{
		if ((port > MAX_RELAY) || (port < 1))
			status = 0;
	}

	cJSON *out_msg = cJSON_CreateObject();
	if (out_msg)
	{

		time(&current_time);
		cJSON_AddStringToObject(out_msg, key_res, cmd_set_port_status);
		cJSON_AddNumberToObject(out_msg, key_status, status);
		cJSON_AddNumberToObject(out_msg, key_time, current_time);
		cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
		cJSON_Delete(out_msg);
	}

	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);

	time(&current_time);
	if (status == 1)
	{
		port = port - 1;

		if ((uint8_t)gpio_get_level(xObjRelay[port].gpioNo) != (uint8_t)action_to_taken)
		{
			if ((action_to_taken == 1))
			{
				if ((xObjPumpLOCK.lock == 0))
				{
					if ((gpio_get_level(xObjRelay[port].gpioNo) == 0))
					{
						setRelay(xObjRelay[port].gpioNo, action_to_taken & 0x01);
						sendRelayStatus = 1;
						changeDisplaySeq(port);

						for (int x = 0; x < MAX_SCHED; x++)
							xObjRelay[port].emergencyOff[x] = 0;

						if (on_time_sec != 0)
						{

							xObjRelay[port].offTime = current_time + (time_t)(on_time_sec);
							xObjRelay[port].mode = 2;
						}
						else
						{
							xObjRelay[port].offTime = 0xFFFFFFFF;
							xObjRelay[port].mode = 1;
						}
					}
				}
				else
				{
					on_get_pump_lock(NULL);
					sendRelayStatus = 1;
				}
			}
			else
			{
				if (gpio_get_level(xObjRelay[port].gpioNo) == 1)
				{

					xObjRelay[port].offTime = current_time;
					setRelay(xObjRelay[port].gpioNo, 0);
					xObjRelay[port].mode = 0;
					if (xObjRelay[port].last_sched > 0 && xObjRelay[port].last_sched <= MAX_SCHED)
						xObjRelay[port].emergencyOff[xObjRelay[port].last_sched - 1] = 1;
					else
						for (int x = 0; x < MAX_SCHED; x++)
							xObjRelay[port].emergencyOff[x] = 1;
					sendRelayStatus = 1;
				}
			}
		}

		//on_get_port_status(NULL);
	}
}

void get_remaining_on_time(char *pArg)
{
	ESP_LOGI("", "Request for getting remaining on time of relay");

	int port = 0;
	time_t current_time;
	time_t remaining_time = 0;
	xMsgPublish response_mqtt;
	uint8_t status = 0;

	cJSON *in_msg = cJSON_Parse(pArg);
	if (in_msg)
	{
		if (cJSON_HasObjectItem(in_msg, key_port))
		{
			port = cJSON_GetObjectItem(in_msg, key_port)->valueint;
			if ((port > MAX_RELAY) || (port < 1))
			{
				status = 0;
			}
			else
				status = 1;
		}
		cJSON_Delete(in_msg);
	}

	cJSON *out_msg = cJSON_CreateObject();
	if (out_msg)
	{
		time(&current_time);
		cJSON_AddStringToObject(out_msg, key_res, cmd_get_port_time_to_off);
		cJSON_AddNumberToObject(out_msg, key_status, status);
		cJSON_AddNumberToObject(out_msg, key_time, current_time);
		cJSON_AddNumberToObject(out_msg, key_port, port);
		if (status == 1)
		{
			port = port - 1;
			if ((gpio_get_level(xObjRelay[port].gpioNo) == 1) && (xObjRelay[port].mode != 0))
			{
				if (current_time < xObjRelay[port].offTime)
				{
					remaining_time = xObjRelay[port].offTime - current_time;
				}
				if (xObjRelay[port].offTime == 0xFFFFFFFF)
				{
					remaining_time = 0;
				}
				cJSON_AddNumberToObject(out_msg, key_remaining_time, remaining_time);
			}
			else
			{
				cJSON_AddNumberToObject(out_msg, key_remaining_time, -1);
			}
		}
		cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
		cJSON_Delete(out_msg);
	}

	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}

void on_off_all(char *x)
{
	ESP_LOGI("", "Request for turning all relays");

	time_t current_time;
	xMsgPublish response_mqtt;
	uint8_t status = 1;

	time(&current_time);
	for (int i = 0; i < MAX_RELAY; i++)
	{
		xObjRelay[i].offTime = current_time;
		setRelay(xObjRelay[i].gpioNo, 0);
		xObjRelay[i].mode = 0;
		if (xObjRelay[i].last_sched > 0 && xObjRelay[i].last_sched <= MAX_SCHED)
			xObjRelay[i].emergencyOff[xObjRelay[i].last_sched - 1] = 1;
		else
			for (int x = 0; x < MAX_SCHED; x++)
				xObjRelay[i].emergencyOff[x] = 1;
	}
	sendRelayStatus = 1;

	cJSON *out_msg = cJSON_CreateObject();
	if (out_msg)
	{
		cJSON_AddStringToObject(out_msg, key_res, cmd_off_all_relay);
		cJSON_AddNumberToObject(out_msg, key_status, status);
		cJSON_AddNumberToObject(out_msg, key_time, current_time);
		cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
		cJSON_Delete(out_msg);
	}
	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}

void on_set_pump_lock(char *pArg)
{
	time_t current_time;
	xMsgPublish response_mqtt;
	uint8_t status = 1;
	int lock_value = 0;

	char temp[128];

	//get lock value from command
	cJSON *in_msg = cJSON_Parse(pArg);
	if (in_msg)
	{
		if (cJSON_HasObjectItem(in_msg, key_value))
		{
			lock_value = cJSON_GetObjectItem(in_msg, key_value)->valueint;
			status = 1;
		}
		cJSON_Delete(in_msg);
	}

	cJSON *out_msg = cJSON_CreateObject();
	if (out_msg)
	{
		time(&current_time);
		cJSON_AddStringToObject(out_msg, key_res, cmd_set_pump_lock);
		cJSON_AddNumberToObject(out_msg, key_status, status);
		if (status == 1)
		{
			if (lock_value == 0)
			{
				if (xObjPumpLOCK.lock == 1)
				{
					xObjPumpLOCK.lock = 0;
					for(int i = 1; i <= MAX_SCHED; i++)
					{
						memset(temp, 0, sizeof(temp));
						sprintf(temp, "{\"cmd\":\"sched_reset\",\"sched\":%d}", i);
						on_sched_reset(temp);
						
					}
				}
			}
			else
			{
				if (xObjPumpLOCK.lock == 0)
				{
					xObjPumpLOCK.lock = 1;
					on_off_all(NULL);
				}
			}

			update_pump_lock_data(&xObjPumpLOCK);
			cJSON_AddNumberToObject(out_msg, key_value, xObjPumpLOCK.lock);
		}
		cJSON_AddNumberToObject(out_msg, key_time, current_time);
		cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
		cJSON_Delete(out_msg);
	}

	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}

void on_get_pump_lock(char *pArg)
{

	time_t current_time;
	xMsgPublish response_mqtt;
	uint8_t status = 1;

	cJSON *out_msg = cJSON_CreateObject();
	if (out_msg)
	{
		time(&current_time);
		cJSON_AddStringToObject(out_msg, key_res, cmd_get_pump_lock);
		cJSON_AddNumberToObject(out_msg, key_value, xObjPumpLOCK.lock);
		cJSON_AddNumberToObject(out_msg, key_status, status);
		cJSON_AddNumberToObject(out_msg, key_time, current_time);
		cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
		cJSON_Delete(out_msg);
	}
	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}

void on_set_scheduler(char *msg)
{
	xMsgPublish response_mqtt;

	time_t current_unix_time;
	time(&current_unix_time);

	uint8_t scheduler_number = 0;
	uint8_t scheduler_hh = 0;
	uint8_t scheduler_mm = 0;
	uint16_t scheduler_ontime = 0;
	uint8_t scheduler_day = 0;
	uint16_t scheduler_en = 0;
	uint8_t scheduler_port = 0;
	uint8_t success = 0;

	sched_def scheduler;

	cJSON *rcvd = cJSON_Parse(msg);

	if (cJSON_HasObjectItem(rcvd, key_scheduler_number) &&
		cJSON_HasObjectItem(rcvd, key_scheduler_en) &&
		cJSON_HasObjectItem(rcvd, key_scheduler_active_days) &&
		cJSON_HasObjectItem(rcvd, key_scheduler_hh) &&
		cJSON_HasObjectItem(rcvd, key_scheduler_mm) &&
		cJSON_HasObjectItem(rcvd, key_ontime))
	{
		scheduler_number = cJSON_GetObjectItem(rcvd, key_scheduler_number)->valueint;
		scheduler_en = cJSON_GetObjectItem(rcvd, key_scheduler_en)->valueint;
		scheduler_day = cJSON_GetObjectItem(rcvd, key_scheduler_active_days)->valueint;
		scheduler_hh = cJSON_GetObjectItem(rcvd, key_scheduler_hh)->valueint;
		scheduler_mm = cJSON_GetObjectItem(rcvd, key_scheduler_mm)->valueint;
		scheduler_ontime = cJSON_GetObjectItem(rcvd, key_ontime)->valueint;
		scheduler_port = cJSON_GetObjectItem(rcvd, key_port)->valueint;
		success = 1;

		if ((scheduler_number < 1) || (scheduler_number > MAX_SCHED))
		{
			success = 0;
		}

		if ((scheduler_port < 1) || (scheduler_port > (MAX_RELAY)))
		{
			success = 0;
		}
	}
	cJSON_Delete(rcvd);

	if (success == 1)
	{
		scheduler.enable = scheduler_en;
		scheduler.day = scheduler_day;
		scheduler.start_time_HH = scheduler_hh;
		scheduler.start_time_MM = scheduler_mm;
		scheduler.duration = scheduler_ontime;
		scheduler.port = scheduler_port;
		scheduler_update(scheduler_number, scheduler);
	}

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_res, cmd_set_scheduler);
	cJSON_AddNumberToObject(response, key_time, current_unix_time);
	cJSON_AddNumberToObject(response, key_status, success);
	if (success == 1)
	{
		cJSON_AddNumberToObject(response, key_scheduler_number, scheduler_number);
	}
	cJSON_PrintPreallocated(response, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
	ESP_LOGI("", "%s", response_mqtt.msg);
	cJSON_Delete(response);

	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}

void on_get_scheduler(char *msg)
{

	xMsgPublish response_mqtt;
	time_t current_unix_time;
	time(&current_unix_time);

	uint8_t scheduler_number = 0;
	uint8_t success = 0;

	sched_def scheduler;

	cJSON *rcvd = cJSON_Parse(msg);
	if (cJSON_HasObjectItem(rcvd, key_scheduler_number))
	{
		scheduler_number = cJSON_GetObjectItem(rcvd, key_scheduler_number)->valueint;
		if ((scheduler_number >= 1) && (scheduler_number <= MAX_SCHED))
			success = 1;
	}
	cJSON_Delete(rcvd);

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_res, cmd_get_scheduler);
	cJSON_AddNumberToObject(response, key_time, current_unix_time);
	cJSON_AddNumberToObject(response, key_status, success);
	if (success == 1)
	{
		scheduler_read(scheduler_number, &scheduler);
		cJSON_AddNumberToObject(response, key_scheduler_number, scheduler_number);
		cJSON_AddNumberToObject(response, key_scheduler_en, scheduler.enable);
		cJSON_AddNumberToObject(response, key_scheduler_active_days, scheduler.day);
		cJSON_AddNumberToObject(response, key_scheduler_hh, scheduler.start_time_HH);
		cJSON_AddNumberToObject(response, key_scheduler_mm, scheduler.start_time_MM);
		cJSON_AddNumberToObject(response, key_ontime, scheduler.duration);
		cJSON_AddNumberToObject(response, key_port, scheduler.port);
	}
	cJSON_PrintPreallocated(response, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
	ESP_LOGI("", "%s", response_mqtt.msg);
	cJSON_Delete(response);
	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}

void on_get_switch_status(char *pArg)
{
	uint8_t switchStat;
	uint8_t status = 0;
	xMsgPublish response_mqtt;
	time_t current_unix_time;
	time(&current_unix_time);
	int value_array[8];

	cJSON *response = cJSON_CreateObject();
	time(&current_unix_time);
	cJSON_AddStringToObject(response, key_res, cmd_get_setting_switch);
	cJSON_AddNumberToObject(response, key_time, current_unix_time);

	if (xPCA9555ReadP0(&switchStat) != ESP_OK)
	{
		status = 0;
	}
	else
	{
		status = 1;
		switchStat = ~switchStat;

		value_array[0] = ((switchStat & (1 << 0)) == 0) ? 0 : 1;
		value_array[1] = ((switchStat & (1 << 1)) == 0) ? 0 : 1;
		value_array[2] = ((switchStat & (1 << 2)) == 0) ? 0 : 1;
		value_array[3] = ((switchStat & (1 << 3)) == 0) ? 0 : 1;
		value_array[4] = ((switchStat & (1 << 4)) == 0) ? 0 : 1;
		value_array[5] = ((switchStat & (1 << 5)) == 0) ? 0 : 1;
		value_array[6] = ((switchStat & (1 << 6)) == 0) ? 0 : 1;
		value_array[7] = ((switchStat & (1 << 7)) == 0) ? 0 : 1;

		cJSON_AddItemToObject(response, key_value, cJSON_CreateIntArray(value_array, 8));
	}
	cJSON_AddNumberToObject(response, key_status, status);
	cJSON_PrintPreallocated(response, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
	cJSON_Delete(response);
	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}

void on_ota_start(char *pArg)
{

	uint8_t status = 0;
	xMsgPublish response_mqtt;
	time_t current_unix_time;
	time(&current_unix_time);

	cJSON *in_msg = cJSON_Parse(pArg);
	if (in_msg)
	{
		if (cJSON_HasObjectItem(in_msg, key_ota_file))
		{
			memset(otaVersion, 0, sizeof(otaVersion));
			strcpy(otaVersion, cJSON_GetObjectItem(in_msg, key_ota_file)->valuestring);
			xEventGroupSetBits(common_evnet_group, OTA_START);
			status = 1;
		}
		cJSON_Delete(in_msg);
	}
	else
	{
		return;
	}

	cJSON *out_msg = cJSON_CreateObject();
	cJSON_AddStringToObject(out_msg, key_res, cmd_start_ota);
	cJSON_AddNumberToObject(out_msg, key_time, current_unix_time);
	cJSON_AddNumberToObject(out_msg, key_status, status);
	cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
	cJSON_Delete(out_msg);
	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}

void on_factory_reset(char *pArg)
{
	uint8_t status = 1;
	xMsgPublish response_mqtt;
	time_t current_unix_time;
	time(&current_unix_time);

	ESP_LOGI("", "Factory Reset is called\n");

	xObjProvData.isProvisioned = 0;
	memset(xObjProvData.ssid, 0, sizeof(xObjProvData.ssid));
	memset(xObjProvData.password, 0, sizeof(xObjProvData.password));
	update_prov_data();

	sched_def scheduler;
	memset(&scheduler, 0, sizeof(sched_def));
	for (int i = 1; i <= MAX_SCHED; i++)
	{
		scheduler_update(i, scheduler);
	}

	memset(&device_timezone, 0, sizeof(device_time_zone_def));
	set_device_time_zone(&device_timezone);

	cJSON *out_msg = cJSON_CreateObject();
	cJSON_AddStringToObject(out_msg, key_res, cmd_factory_reset);
	cJSON_AddNumberToObject(out_msg, key_time, current_unix_time);
	cJSON_AddNumberToObject(out_msg, key_status, status);
	cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
	cJSON_Delete(out_msg);
	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);

	memset(&response_mqtt, 0, sizeof(response_mqtt));
	sprintf(response_mqtt.msg, "%s", msg_offline);
	sprintf(response_mqtt.topic, "%s/Status", getDeviceName());
	mqttPublishPushToQueue(&response_mqtt);

	vTaskDelay(pdMS_TO_TICKS(5000));
	esp_restart();
}

void on_sched_reset(char *pArg)
{
	uint8_t status = 0;
	xMsgPublish response_mqtt;
	time_t current_unix_time;
	time(&current_unix_time);
	uint8_t scheduler_number = 0;

	cJSON *in_msg = cJSON_Parse(pArg);
	if (in_msg)
	{
		if (cJSON_HasObjectItem(in_msg, key_scheduler_number))
		{
			scheduler_number = cJSON_GetObjectItem(in_msg, key_scheduler_number)->valueint;
			if ((scheduler_number < 1) || (scheduler_number > MAX_SCHED))
				status = 0;
			else
				status = 1;
		}
		cJSON_Delete(in_msg);
	}
	else
	{
		return;
	}

	time(&current_unix_time);
	cJSON *out_msg = cJSON_CreateObject();
	cJSON_AddStringToObject(out_msg, key_res, cmd_reset_scheduler);
	cJSON_AddNumberToObject(out_msg, key_time, current_unix_time);
	cJSON_AddNumberToObject(out_msg, key_scheduler_number, scheduler_number);
	cJSON_AddNumberToObject(out_msg, key_status, status);
	if (status == 1)
	{
		xObjRelay[0].emergencyOff[scheduler_number - 1] = 0;
		xObjRelay[1].emergencyOff[scheduler_number - 1] = 0;
		xObjRelay[2].emergencyOff[scheduler_number - 1] = 0;
		xObjRelay[3].emergencyOff[scheduler_number - 1] = 0;
		xObjRelay[4].emergencyOff[scheduler_number - 1] = 0;
	}

	cJSON_PrintPreallocated(out_msg, response_mqtt.msg, sizeof(response_mqtt.msg), 0);
	cJSON_Delete(out_msg);
	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	response_mqtt.retain = 0;
	mqttPublishPushToQueue(&response_mqtt);
}
//=============================================================

void cmdActionTask(void *pV)
{
	xQueueHandle *queue;
	queue = (xQueueHandle *)pV;

	xMsgPublish xMsgTask;

	while (1)
	{
		memset(&xMsgTask, 0, sizeof(xMsgTask));
		if (xQueueReceive(*queue, &xMsgTask, 0) == pdTRUE)
		{
			ESP_LOGI("", "Msg is received on queue");
			msgParser(xMsgTask.msg);
		}
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

//=============================================================

esp_err_t pushToActionQueue(xMsgReceived *px)
{
	ESP_LOGI("", "Pushing Action in queue");
	if (xQueueSend(xQueueAction, px, 10) == pdTRUE)
		return ESP_OK;
	else
		return ESP_FAIL;
}

//SendNotification for hardware fault
void send_notification(int faultType)
{
	char tempBuf[32];
	xMsgPublish response_mqtt;
	if (faultType == 1)
	{
		strcpy(tempBuf, "LEO");
	}
	else if (faultType == 2)
	{
		strcpy(tempBuf, "LEC");
	}
	else if (faultType == 3)
	{
		strcpy(tempBuf, "Hardware Fault");
	}
	sprintf(response_mqtt.msg, "{\"fault\":\"%s\"}", tempBuf);
	sprintf(response_mqtt.topic, mqttMsgPubChannel);
	mqttPublishPushToQueue(&response_mqtt);
}
