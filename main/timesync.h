/*
 * timesync.h
 *
 *  Created on: 03-Oct-2019
 *      Author: shiv
 */

#ifndef MAIN_TIMESYNC_H_
#define MAIN_TIMESYNC_H_

#include "commondef.h"

void taskSyncTime(void *pV);
struct tm getLocalTime(char *tz);

esp_err_t xUpdateTimeInRTC();
esp_err_t xUpdateTimeInRTCManually(time_t rawtime);
esp_err_t xUpdateESPTimeFromRTC();

#endif /* MAIN_TIMESYNC_H_ */
