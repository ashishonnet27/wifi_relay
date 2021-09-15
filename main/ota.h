/*
 * ota.h
 *
 *  Created on: 08-Oct-2019
 *      Author: shiv
 */

#ifndef MAIN_OTA_H_
#define MAIN_OTA_H_

#include "commondef.h"
#include "mqtt.h"

//#define OTA_BASE_URL "https://firmwarevms.s3.amazonaws.com/"
#define OTA_BASE_URL "https://speckpumps.s3.amazonaws.com/firmware/"

void simple_ota_example_task(void *);

#endif /* MAIN_OTA_H_ */
