/*
 * peripheralControl.h
 *
 *  Created on: 08-Oct-2019
 *      Author: shiv
 */

#ifndef MAIN_PERIPHERALCONTROL_H_
#define MAIN_PERIPHERALCONTROL_H_

#include "commondef.h"

uint8_t sendRelayStatus;
void changeDisplaySeq(uint8_t);
void peripheralControlTask(void *pV);
void sevenSegmentControlTask(void *pV);
esp_err_t setRelay(uint8_t relayNo, uint8_t val);
void initSwitch(void);
void initOtherInputs(void);

void waitingForInActivitiy();
void SwitchMonitorTask(void *pV);

void zcrTask(void *pV);





#endif /* MAIN_PERIPHERALCONTROL_H_ */
