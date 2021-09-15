/*
 * mqtt.c
 *
 *  Created on: 02-Oct-2019
 *      Author: shiv
 */


#include "mqtt.h"
#include "cmdAction.h"


char mqttusername[]= "sftpmq";
char mqttpassword[]= "sftpmq*352&";

const char mqttLastWillMsg[] = "{\"Status\":\"Offline\"}";
const char mqttOnlineMsg[] = "{\"Status\":\"Online\"}";
const size_t mqttLastWillMsgSize = 20;

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event);
esp_mqtt_client_handle_t mqttClient;

char mqttTopicBuf[MAX_MQTT_TPC_SIZE];
char mqttData[MAX_MQTT_MSG_SIZE];
char mqttLWTopic[MAX_MQTT_TPC_SIZE];

xMsgPublish xPubMsg;
xMsgReceived xActMsg;

char pubTopic_Status[] = "Status";
char pubTopic_Ack[] = "Ack";
char pubTopic_Info[] = "Info";

char subTopic_Action[] = "Action";

void onConnectPublish()
{
	sprintf(xPubMsg.topic, "%s/%s", getDeviceName(), pubTopic_Status);
	sprintf(xPubMsg.msg, "%s", mqttOnlineMsg);
	xPubMsg.retain = 1;
	mqttPublishPushToQueue(&xPubMsg);

	memset(&xPubMsg, 0, sizeof(xPubMsg));
	sprintf(xPubMsg.msg, "{\"info\":\"%s\"}", VERSION);
	sprintf(xPubMsg.topic, "%s/%s", getDeviceName(), pubTopic_Info);
	xPubMsg.retain = 1;
	mqttPublishPushToQueue(&xPubMsg);
}

void onConnectSubscribe()
{

}

void mqttStart()
{
	ESP_LOGI("", "Starting MQTT STuffs");
	sprintf(mqttLWTopic, "%s/%s", getDeviceName(), pubTopic_Status);
	sprintf(mqttMsgPubChannel, "%s/%s", getDeviceName(), pubTopic_Ack);
	esp_mqtt_client_config_t mqtt_cfg = {
			.uri = "mqtts://baduconnect.com:8883",
			.event_handle = mqtt_event_handler,
			.cert_pem = NULL, //(const char *)server_cert_pem_start,
			.username = mqttusername,
			.password = mqttpassword,
			.lwt_qos = 0,
			.lwt_retain = 1,
			.lwt_msg = mqttLastWillMsg,
			.lwt_msg_len = mqttLastWillMsgSize,
			.lwt_topic = mqttLWTopic,
			.keepalive = 30,
	};

	mqttClient = esp_mqtt_client_init(&mqtt_cfg);

	xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED, false, true, portMAX_DELAY);
	if(xEventGroupGetBits(wifi_event_group) & AP_ON)
	{
		ESP_LOGI("", "No need to get time");
	}
	else
	{
		xEventGroupWaitBits(common_evnet_group, CMN_TIMESET, false, true, portMAX_DELAY);
	}
	esp_mqtt_client_start(mqttClient);
	ESP_LOGI("", "Free memory: %d bytes", esp_get_free_heap_size());

	xMsgPublish queMsg;

	if(xEventGroupGetBits(wifi_event_group) & AP_ON)
	{
	}
	else
	{
		while(1)
		{
			memset(&queMsg, 0, sizeof(queMsg));
			if(xQueueReceive(xQueuePublish, &queMsg, 0) == pdTRUE)
			{
				//mqttPublish(queMsg.topic, queMsg.msg, queMsg.retain);
				//xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED, false, true, portMAX_DELAY);
				//xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED, false, true, portMAX_DELAY);
				//esp_mqtt_client_publish(mqttClient, queMsg.topic, queMsg.msg, strlen(queMsg.msg), 0, 0);
				if( (xEventGroupGetBits(wifi_event_group) & WIFI_CONNECTED) &&
						(xEventGroupGetBits(mqtt_event_group) & MQTT_CONNECTED)
						)
				{
					mqttPublish(queMsg.topic, queMsg.msg, queMsg.retain);
				}
			}
			vTaskDelay(pdMS_TO_TICKS(100));
		}
	}
}

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
	mqttClient = event->client;
	switch(event->event_id)
	{

	case MQTT_EVENT_CONNECTED:
		ESP_LOGI("", "MQTT is connected \n");
		sprintf(mqttTopicBuf, "%s", getDeviceName());
		xEventGroupSetBits(wifi_event_group, PROV_CLOUD_CONNECTED);
		xEventGroupClearBits(wifi_event_group, PROV_CLOUD_CONNECTING);
		xEventGroupClearBits(wifi_event_group, PROV_CLOUD_FAIL);
		xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED);
		if(xEventGroupGetBits(wifi_event_group) & AP_ON)
		{
			ESP_LOGI("", "No need to publish anything");
		}
		else
		{
			memset(mqttTopicBuf, 0, sizeof(mqttTopicBuf));
			sprintf(mqttTopicBuf, "%s/%s", getDeviceName(), subTopic_Action);
			ESP_LOGI("","Subscribe to : [%s]", mqttTopicBuf);

			esp_mqtt_client_subscribe(mqttClient, mqttTopicBuf, 0);
			onConnectPublish();
		}

		break;

	case MQTT_EVENT_DISCONNECTED:
		ESP_LOGI("", "MQTT is disconnected \n");
		xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED);
		xEventGroupSetBits(wifi_event_group, PROV_CLOUD_FAIL);
		xEventGroupClearBits(wifi_event_group, PROV_CLOUD_CONNECTING);
		xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED, false, true, portMAX_DELAY);
		break;

	case MQTT_EVENT_DATA:
		ESP_LOGI("", "MQTT_EVENT_DATA");
		ESP_LOGI("","TOPIC=%.*s\r\n", event->topic_len, event->topic);
		ESP_LOGI("","DATA=%.*s\r\n", event->data_len, event->data);

		if(event->data_len < MAX_MQTT_MSG_SIZE)
		{
			sprintf(xActMsg.msg, "%.*s", event->data_len, event->data);
			ESP_LOGI("", "Message = %s", xActMsg.msg);
			pushToActionQueue(&xActMsg);
		}
		break;

	default:
		break;
	}
	return ESP_OK;
}

void mqttPublish(char *topic, char *Msg, uint8_t retain)
{
	esp_mqtt_client_publish(mqttClient, topic, Msg, strlen(Msg), 0, retain);
}

void mqttPublishTask(void *pV)
{
	xQueueHandle *queue;
	queue = (xQueueHandle *) pV;

	xMsgPublish queMsg;

	while(1)
	{
		memset(&queMsg, 0, sizeof(queMsg));
		if(xQueueReceive(*queue, &queMsg, 0) == pdTRUE)
		{
			mqttPublish(queMsg.topic, queMsg.msg, queMsg.retain);
		}
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

esp_err_t mqttPublishPushToQueue(xMsgPublish *pubMsg)
{
	if(xQueueSend(xQueuePublish, pubMsg, 10) == pdTRUE)
		return ESP_OK;
	else
		return ESP_FAIL;
}
