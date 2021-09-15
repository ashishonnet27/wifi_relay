/*
 * wifiConnectivity.h
 *
 *  Created on: 02-Oct-2019
 *      Author: shiv
 */

#ifndef MAIN_WIFICONNECTIVITY_H_
#define MAIN_WIFICONNECTIVITY_H_

#include "esp_err.h"



char ip_address_str[32];

esp_err_t init_wifi();

esp_err_t start_station();

esp_err_t start_provisioning();



#endif /* MAIN_WIFICONNECTIVITY_H_ */
