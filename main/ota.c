/*
 * ota.c
 *
 *  Created on: 08-Oct-2019
 *      Author: shiv
 */

#include "ota.h"
#include "commondef.h"
#include "mqtt.h"
#include "peripheralControl.h"

#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

extern const uint8_t server_cert_pem_start[] asm("_binary_s3all_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_s3all_pem_end");

xMsgPublish xMsgOTA;

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
	switch (evt->event_id) {
	case HTTP_EVENT_ERROR:
		ESP_LOGD("", "HTTP_EVENT_ERROR");
		break;
	case HTTP_EVENT_ON_CONNECTED:
		ESP_LOGD("", "HTTP_EVENT_ON_CONNECTED");
		break;
	case HTTP_EVENT_HEADER_SENT:
		ESP_LOGD("", "HTTP_EVENT_HEADER_SENT");
		break;
	case HTTP_EVENT_ON_HEADER:
		ESP_LOGD("", "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
		break;
	case HTTP_EVENT_ON_DATA:
		ESP_LOGD("", "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
		break;
	case HTTP_EVENT_ON_FINISH:
		ESP_LOGD("", "HTTP_EVENT_ON_FINISH");
		break;
	case HTTP_EVENT_DISCONNECTED:
		ESP_LOGD("", "HTTP_EVENT_DISCONNECTED");
		break;
	default:
		break;
	}
	return ESP_OK;
}

void push_ota_msg(char *msg)
{
	time_t current_time;
	time(&current_time);
	memset(&xMsgOTA, 0 , sizeof(xMsgOTA));

	sprintf(xMsgOTA.msg, "{\"res\":\"ota\", \"msg\":\"%s\", \"time\":%ld}", msg, current_time);
	strcpy(xMsgOTA.topic, mqttMsgPubChannel);
	xMsgOTA.retain = 0;

	mqttPublishPushToQueue(&xMsgOTA);
}

void simple_ota_example_task(void *pvParameter)
{
	ESP_LOGI("", "Starting OTA example");

	xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED, false, true, portMAX_DELAY);
	xEventGroupWaitBits(common_evnet_group, OTA_START, false, true, portMAX_DELAY);



	char otaURL[256];

	push_ota_msg("OTA Started");

	sprintf(otaURL, "%s%s", OTA_BASE_URL, (char *)otaVersion);
	esp_http_client_config_t config = {
			.url = otaURL,
			.cert_pem = (char *)server_cert_pem_start,
			.event_handler = _http_event_handler,
	};

	push_ota_msg("OTA Downloading updates");


	esp_err_t ret = esp_https_ota(&config);

	if (ret == ESP_OK)
	{
		push_ota_msg("OTA Success Now rebooting in 10 Seconds");
		vTaskDelay(pdMS_TO_TICKS(1000*10));
		waitingForInActivitiy();
		esp_restart();
	}
	else
	{
		push_ota_msg("OTA Failed, Rebooting");
		ESP_LOGE("", "Firmware upgrade failed");
		vTaskDelay(pdMS_TO_TICKS(1000*10));
		waitingForInActivitiy();
		esp_restart();
	}

	while (1)
	{
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}


