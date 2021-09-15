/*
 * mqtt.h
 *
 *  Created on: 02-Oct-2019
 *      Author: shiv
 */

#ifndef MAIN_MQTT_H_
#define MAIN_MQTT_H_

#include "commondef.h"
#include "userSturct.h"

char mqttMsgPubChannel[64];

void mqttStart();
void mqttPublish(char *, char *, uint8_t retain);
void mqttPublishTask(void *);
esp_err_t mqttPublishPushToQueue(xMsgPublish *);

#endif /* MAIN_MQTT_H_ */
