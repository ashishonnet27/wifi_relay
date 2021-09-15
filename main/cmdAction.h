/*
 * cmdAction.h
 *
 *  Created on: 07-Oct-2019
 *      Author: shiv
 */

#ifndef MAIN_CMDACTION_H_
#define MAIN_CMDACTION_H_

#include "commondef.h"
#include "userSturct.h"

void msgParser(char *);

void on_factory_reset(char *);
void on_ota_start(char *);





void on_off_all(char *);
void on_set_relay_status(char *);
void get_remaining_on_time(char *);
void on_get_rtc_voltage(char *);
void on_get_device_type(char *);
void on_get_device_time_zone(char *);
void on_set_device_time_zone(char *);
void on_get_device_fw_version(char *);
void on_device_reboot(char *);
void on_get_wifi_ssid(char *);
void on_wifi_clear(char *);
void on_get_device_time(char *);
void on_set_device_time(char *);
void on_get_port_status(char *);
void on_set_scheduler(char *msg);
void on_get_scheduler(char *msg);
void on_set_pump_lock(char *);
void on_get_pump_lock(char *);
void on_get_switch_status(char *);
void on_sched_reset(char *);

esp_err_t pushToActionQueue(xMsgReceived *);
void cmdActionTask(void *);

void send_notification(int faultType);




#endif /* MAIN_CMDACTION_H_ */
