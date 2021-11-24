/*
 * data-storage.h
 *
 *  Created on: 06-Oct-2019
 *      Author: shiv
 */

#ifndef MAIN_DATA_STORAGE_H_
#define MAIN_DATA_STORAGE_H_

#include "commondef.h"
#include "userSturct.h"

esp_err_t initDataStorage();

esp_err_t get_stored_data();

void read_stored_prov_data();
esp_err_t update_prov_data();

void read_stored_relay_data();
esp_err_t update_prov_relay_data();
// void readStoredSchedulerData(xRLScheduler *);
// void updateStoredSchedulerData(xRLScheduler *);

// void getDeviceTimeZone(device_time_zone_def *);
// void set_device_time_zone(device_time_zone_def *);
// void update_pump_lock_data(xPumpLOCK *);
// void readLockData(xPumpLOCK *);

// void init_scheduler();
// void scheduler_read(uint8_t number, sched_def *sched);
// void scheduler_update(uint8_t number, sched_def sched);


#endif /* MAIN_DATA_STORAGE_H_ */
