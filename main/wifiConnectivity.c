/*
 * wifiConnectivity.c
 *
 *  Created on: 02-Oct-2019
 *      Author: shiv
 */


#include "wifiConnectivity.h"
#include "commondef.h"
//#include "mqtt.h"
#include "data-storage.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define CPOSTBUFSIZE 256

char device_APName[33];

const char key_w_device_type[] = "device_type";
const char key_w_slave_id[] = "slave_id";
const char key_w_baud_rate[] = "baud_rate";
const char key_w_id[] = "id";
const char key_w_address[] = "address";
const char key_w_enable[] = "enable";
const char key_w_param[] = "param";


uint8_t u8ApListEntry = 0;
wifi_ap_record_t *xApList;

static httpd_handle_t xServer = NULL;
char cPostReqBuf[CPOSTBUFSIZE];
uint8_t CMDProv;

wifi_config_t wifi_config;
wifi_config_t wifi_configSTA;
wifi_scan_config_t scan_config;

enum
{
	prov_start,					//0
	prov_start_ap,				//1
	prov_start_prov_server,			//2
	prov_wait_to_start_scan,		//3
	prov_start_scan,				//4
	prov_scanning,				//5
	prov_done_scanning,			//6
	prov_wait_for_cred,			//7
	prov_conecting_network,	//8
	prov_connected_to_router, 	//9
	prov_wrong_cred,				//10
	
};

uint8_t u8PROVSTATE = 0;
uint8_t currentState, nextState;
esp_netif_t *esp_netif1;
void startServer();

static void wifi_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
	switch(event_id)
	{
	case WIFI_EVENT_STA_START:
		if(xEventGroupGetBits(wifi_event_group) & AP_ON)
		{

		}
		else
		{
			//esp_wifi_connect();
		}
		break;

	case WIFI_EVENT_STA_DISCONNECTED:
		xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED);
		xEventGroupSetBits(wifi_event_group, PROV_FAIL_CONNECT_WIFI);
		gpio_set_level(WIFI_CONNECT_LED_PIN, LED_OFF);
		ESP_LOGI("", "wifi disconnected");
		esp_wifi_connect();
		break;

	case WIFI_EVENT_AP_START:
		xEventGroupSetBits(wifi_event_group, AP_ON);
		ESP_LOGI("", "AP is started, Device is in AP mode");
		startServer();
		break;

	case WIFI_EVENT_AP_STOP:
		xEventGroupClearBits(wifi_event_group, AP_ON);
		ESP_LOGI("", "AP is stopped");
		break;

	case WIFI_EVENT_AP_STACONNECTED:
		ESP_LOGI("", "AP is connected");
		break;

	case WIFI_EVENT_AP_STADISCONNECTED:
		ESP_LOGI("", "AP is disconnected");
		break;

	default:
		break;
	}

}




static void ip_event_handler(void * arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
	ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
	ESP_LOGI("", "got ip:%s", ip4addr_ntoa(&event->ip_info.ip));
	xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED);
	gpio_set_level(WIFI_CONNECT_LED_PIN, LED_ON);
	xEventGroupSetBits(wifi_event_group, PROV_CONNECTED_WIFI);
	xEventGroupClearBits(wifi_event_group, PROV_FAIL_CONNECT_WIFI);



}



esp_err_t init_wifi()
{

	tcpip_adapter_init();
	// ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	

    
	
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	uint8_t deviceETH[6];
	esp_wifi_get_mac(WIFI_IF_STA, deviceETH);
	// sprintf(deviceName, "%s_%02X%02X%02X%02X%02X%02X", DEVICE_PREFIX, 
	// 	deviceETH[0], deviceETH[1], deviceETH[2],
	// 	deviceETH[3], deviceETH[4], deviceETH[5]);
	sprintf(device_APName, "%s","ESP_AP"); //%s_%02X:%02X:%02X:%02X:%02X:%02X", DEVICE_PREFIX,
			// deviceETH[0], deviceETH[1], deviceETH[2],
			// deviceETH[3], deviceETH[4], deviceETH[5]);

	// ESP_LOGI("", "Device name = [%s]\n", getDeviceName());


	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));
	
	return ESP_OK;

}


wifi_config_t wifi_STA;
esp_err_t start_station()
{
	
	// strcpy((char *)wifi_STA.sta.ssid, xObjProvData.ssid);
	// strcpy((char *)wifi_STA.sta.password, xObjProvData.password);
	strcpy((char *)wifi_STA.sta.ssid, "athlon");
	strcpy((char *)wifi_STA.sta.password, "w1234567");
	wifi_STA.sta.pmf_cfg.capable = true;
	wifi_STA.sta.pmf_cfg.required = false;
    
	if(xObjProvData.isStatic_ip == true)
	{
		esp_netif_t *my_sta;
		esp_netif_ip_info_t ip_info;
		my_sta = esp_netif_create_default_wifi_sta();
		esp_netif_dhcpc_stop(my_sta);
		

		struct sockaddr_in antelope;


		inet_aton(xObjProvData.static_ip_addr, &antelope.sin_addr); // store IP in antelope

		ip_info.ip.addr = antelope.sin_addr.s_addr;
		
		inet_aton(xObjProvData.static_ip_gw, &antelope.sin_addr); // store IP in antelope
		ip_info.gw.addr = antelope.sin_addr.s_addr;

		inet_aton(xObjProvData.static_ip_netmask, &antelope.sin_addr); // store IP in antelope
		ip_info.netmask.addr = antelope.sin_addr.s_addr;

		

		esp_netif_set_ip_info(my_sta, &ip_info);
	}

	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_STA) );
	ESP_ERROR_CHECK(esp_wifi_start());
	ESP_ERROR_CHECK(esp_wifi_connect());

	ESP_LOGI("", "wifi_init_sta finished.");
	startServer();
	//ESP_LOGI("", "connect to ap SSID:%s password:%s", WIFISTASSID, WIFISTAPASS);
	return ESP_OK;
}


esp_err_t start_provisioning()
{
	ESP_LOGI("", "Starting Provisioning");
	xEventGroupSetBits(wifi_event_group, AP_ON);

	// configure and run the scan process in blocking mode
	scan_config.ssid = 0;
	scan_config.bssid = 0;
	scan_config.channel = 0;
	scan_config.show_hidden = true;

	// configure, initialize and start the wifi driver
	//wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	//ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	strcpy((char *)wifi_config.ap.ssid, device_APName);
	strcpy((char *)wifi_config.ap.password, "password");
	wifi_config.ap.ssid_len = strlen(device_APName);
	wifi_config.ap.max_connection = 4;
	wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;


	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));

	currentState = prov_start;

	currentState = prov_start;

	while(1)
	{
		switch (currentState)
		{
		case prov_start:
			ESP_ERROR_CHECK(esp_wifi_start());
			u8PROVSTATE = prov_start;
			switch(CMDProv)
			{
			case prov_start_scan:
				nextState = prov_start_scan;
				break;
			default:
				nextState = prov_start;
				break;
			}
			break;

			case prov_start_scan:
				u8PROVSTATE = prov_scanning;
				CMDProv = 0;
				xEventGroupClearBits(wifi_event_group, PROV_CONNECTING_WIFI | PROV_CONNECTING_WIFI | PROV_FAIL_CONNECT_WIFI);
				ESP_ERROR_CHECK(esp_wifi_start());
				ESP_LOGI("","Start scanning...");
				ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
				ESP_LOGI(""," completed!\n");
				nextState = prov_done_scanning;
				break;

			case prov_done_scanning:
				u8PROVSTATE = prov_done_scanning;
				if(u8ApListEntry != 0)
				{
					u8ApListEntry = 0;
					free(xApList);
				}
				esp_wifi_scan_get_ap_num((uint16_t *)&u8ApListEntry);
				xApList = malloc(u8ApListEntry * sizeof(wifi_ap_record_t));
				ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records((uint16_t *)&u8ApListEntry, xApList));
				nextState = prov_wait_for_cred;
				break;

			case prov_wait_for_cred:
				u8PROVSTATE = prov_wait_for_cred;
				if(CMDProv == prov_start_scan)
				{
					CMDProv = 0;
					nextState = prov_start_scan;
				}
				else
					nextState = prov_wait_for_cred;
				break;

			case prov_conecting_network:
				u8PROVSTATE = prov_conecting_network;
				xEventGroupSetBits(wifi_event_group, PROV_CONNECTING_WIFI);
				if(xEventGroupGetBits(wifi_event_group) & PROV_CONNECTED_WIFI)
				{
					
					xEventGroupClearBits(wifi_event_group, PROV_CONNECTING_WIFI);
					nextState = prov_connected_to_router;
					break;
				}
				if(xEventGroupGetBits(wifi_event_group) & PROV_FAIL_CONNECT_WIFI)
				{
					xEventGroupClearBits(wifi_event_group, PROV_CONNECTING_WIFI);
					nextState = prov_wrong_cred;
					break;
				}
				nextState = prov_conecting_network;
				break;

			case prov_connected_to_router:
				u8PROVSTATE = prov_connected_to_router;
				
				if(xObjProvData.isProvisioned == 0)
				{
					xObjProvData.isProvisioned = 1;
					strcpy(xObjProvData.ssid, (char *)wifi_configSTA.sta.ssid);
					strcpy(xObjProvData.password, (char *)wifi_configSTA.sta.password);
					update_prov_data();
				}
				break;

			case prov_wrong_cred:
				u8PROVSTATE = prov_wrong_cred;

				if(CMDProv == prov_start_scan)
				{
					CMDProv = 0;
					nextState = prov_start_scan;
					ESP_ERROR_CHECK(esp_wifi_disconnect());
				
				}
				else if(xEventGroupGetBits(wifi_event_group) & PROV_CONNECTED_WIFI)
				{
					
					xEventGroupClearBits(wifi_event_group, PROV_CONNECTING_WIFI);
					nextState = prov_connected_to_router;
					break;
				}
				else
					nextState = currentState;
				break;

			default:
				nextState = currentState;
				break;
		}
		// ESP_LOGE("", "Next State = %d", nextState);
		currentState = nextState;
		vTaskDelay(pdMS_TO_TICKS(300));
	}

	return ESP_OK;

}


//======================================================================

char pcState[32];
char pcPostResOk[] = "{\"Ok\":1}";
char pcPostResFail[] = "{\"Ok\":0}";

void vGetPostData(httpd_req_t *req)
{
	int iLength ;
	iLength = (req->content_len < CPOSTBUFSIZE) ? req->content_len : CPOSTBUFSIZE-1;
	memset(cPostReqBuf, 0 ,sizeof(cPostReqBuf));
	httpd_req_recv(req, cPostReqBuf,iLength);
}

esp_err_t xGetProvState(httpd_req_t *req)
{
	sprintf(pcState, "{\"state\":%d}", u8PROVSTATE);
	httpd_resp_send(req, (char *)pcState, strlen(pcState));
	return ESP_OK;
}

httpd_uri_t xGetProvStateUri = {
		.uri       = "/getProvState",
		.method    = HTTP_GET,
		.handler   = xGetProvState,
		.user_ctx  = NULL,
};

esp_err_t xPostProvCMD(httpd_req_t *req)
{
	vGetPostData(req);
	ESP_LOGI("", "[%s]", cPostReqBuf);
	cJSON *root = cJSON_Parse(cPostReqBuf);
	CMDProv = cJSON_GetObjectItem(root, "cmd")->valueint;
	cJSON_Delete(root);
	httpd_resp_send(req, (char *)pcPostResOk, strlen(pcPostResOk));
	return ESP_OK;
}

httpd_uri_t xPostProvCMDUri = {
		.uri       = "/provCMD",
		.method    = HTTP_POST,
		.handler   = xPostProvCMD,
		.user_ctx  = NULL,
};



esp_err_t xPostWifiSSIDList(httpd_req_t *req)
{
	vGetPostData(req);

	cJSON *root = cJSON_CreateObject();
	cJSON *array= cJSON_CreateArray();
	for(int i = 0; i < u8ApListEntry; i++)
	{
		cJSON *item = cJSON_CreateObject();
		cJSON_AddStringToObject(item, "ssid", (char *)xApList[i].ssid);
		cJSON_AddNumberToObject(item, "security", xApList[i].authmode);
		cJSON_AddItemToArray(array, item);
	}
	cJSON_AddItemToObject(root, "ssidList", array);
	char *res;
	res = cJSON_PrintUnformatted(root);
	//	if((u8Item < u8ApListEntry) && (u8Item > 0))
	//		sprintf(cPostReqBuf, "{\"ssid\":\"%s\"}", xApList[u8Item-1].ssid);
	//	else
	//		sprintf(cPostReqBuf, "{\"ssid\":\"\"}");

	httpd_resp_send(req, (char *)res, strlen(res));
	return ESP_OK;
}

httpd_uri_t xGetWifiSSIDListUri = {
		.uri       = "/scanlist",
		.method    = HTTP_GET,
		.handler   = xPostWifiSSIDList,
		.user_ctx  = NULL,
};

esp_err_t xPostWifiCred(httpd_req_t *req)
{
	vGetPostData(req);
	cJSON *root = cJSON_Parse(cPostReqBuf);
	ESP_LOGI("", "SSID = [%s]", cJSON_GetObjectItem(root, "ssid")->valuestring);
	ESP_LOGI("", "PASS = [%s]", cJSON_GetObjectItem(root, "pass")->valuestring);
	strcpy((char *)wifi_configSTA.sta.ssid, cJSON_GetObjectItem(root, "ssid")->valuestring);
	strcpy((char *)wifi_configSTA.sta.password, cJSON_GetObjectItem(root, "pass")->valuestring);

	cJSON_Delete(root);
	httpd_resp_send(req, (char *)pcPostResOk, strlen(pcPostResOk));

	ESP_LOGI("", "connecting");
	if(xObjProvData.isProvisioned != 1)
	{
		if(u8PROVSTATE == prov_wrong_cred)
		ESP_ERROR_CHECK(esp_wifi_disconnect());

		ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configSTA));
		ESP_ERROR_CHECK(esp_wifi_connect());
		
		nextState = prov_conecting_network;
		currentState = prov_conecting_network;
		xEventGroupSetBits(wifi_event_group, PROV_CONNECTING_WIFI);
		xEventGroupClearBits(wifi_event_group, PROV_CONNECTED_WIFI | PROV_FAIL_CONNECT_WIFI);
	}
	else
	{
		ESP_ERROR_CHECK(esp_wifi_disconnect());

		ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configSTA));
		ESP_ERROR_CHECK(esp_wifi_connect());
		startServer();		
	}
	
	return ESP_OK;
}

httpd_uri_t xPostWifiCredUri = {
		.uri       = "/wificred",
		.method    = HTTP_POST,
		.handler   = xPostWifiCred,
		.user_ctx  = NULL,
};


esp_err_t xPostSetStaticIp(httpd_req_t *req)
{
	vGetPostData(req);
	cJSON *root = cJSON_Parse(cPostReqBuf);
	if(cJSON_GetObjectItem(root, "setStatic")->valueint == true)
	{
		ESP_LOGI("", "Static Ip is Assigned !!!");	
		xObjProvData.isStatic_ip = 1;
		ESP_LOGI("", "IP = [%s]", cJSON_GetObjectItem(root, "ip")->valuestring);
		ESP_LOGI("", "Gateway = [%s]", cJSON_GetObjectItem(root, "gw")->valuestring);
		ESP_LOGI("", "Netmask = [%s]", cJSON_GetObjectItem(root, "netmask")->valuestring);
		strcpy((char *)xObjProvData.static_ip_addr, cJSON_GetObjectItem(root, "ip")->valuestring);
		strcpy((char *)xObjProvData.static_ip_gw, cJSON_GetObjectItem(root, "gw")->valuestring);
		strcpy((char *)xObjProvData.static_ip_netmask, cJSON_GetObjectItem(root, "netmask")->valuestring);
	}
	else{
		ESP_LOGI("", "DHCP is Enabled !!!");	
		xObjProvData.isStatic_ip = 0;
	}

	cJSON_Delete(root);
	httpd_resp_send(req, (char *)pcPostResOk, strlen(pcPostResOk));
	update_prov_data();
	
	return ESP_OK;
}

httpd_uri_t xPostSetStaticIpUri = {
		.uri       = "/SetStaticIp",
		.method    = HTTP_POST,
		.handler   = xPostSetStaticIp,
		.user_ctx  = NULL,
};


esp_err_t xIndex_page(httpd_req_t *req)
{
	
	/* Get handle to embedded file upload script */
    extern const unsigned char index_start[] asm("_binary_home_html_start");
    extern const unsigned char index_end[]   asm("_binary_home_html_end");
    const size_t index_size = (index_end - index_start);
	httpd_resp_send_chunk(req, (char *)index_start, index_size);
	httpd_resp_sendstr_chunk(req, NULL);
	
	return ESP_OK;
}

httpd_uri_t xGetIndexPageUri = {
		.uri       = "/",
		.method    = HTTP_GET,
		.handler   = xIndex_page,
		.user_ctx  = NULL,
};

esp_err_t xSettings_page(httpd_req_t *req)
{
	
	/* Get handle to embedded file upload script */
	
    // httpd_resp_send(req, NULL, 0);  // Response body can be empty
    extern const unsigned char set_start[] asm("_binary_settings_html_start");
    extern const unsigned char set_end[]   asm("_binary_settings_html_end");
    const size_t set_size = (set_end - set_start);
	httpd_resp_send_chunk(req, (char *)set_start, set_size);
	httpd_resp_sendstr_chunk(req, NULL);
	// httpd_resp_set_status(req, "303 See Other");
    // httpd_resp_set_hdr(req, "Location", "/");
	// httpd_resp_sendstr(req, "File uploaded successfully");
	ESP_LOGI("", "setting page uri called");
	return ESP_OK;
}

httpd_uri_t xGetSettingsPageUri = {
		.uri       = "/settings",
		.method    = HTTP_GET,
		.handler   = xSettings_page,
		.user_ctx  = NULL,
};



esp_err_t xGetRootESP(httpd_req_t *req)
{
	httpd_resp_send(req, (char *)pcPostResOk, strlen(pcPostResOk));


	vTaskDelay(pdMS_TO_TICKS(3000));
	esp_restart();
	return ESP_OK;
}

httpd_uri_t xGetRootESPUri = {
		.uri       = "/reboot",
		.method    = HTTP_GET,
		.handler   = xGetRootESP,
		.user_ctx  = NULL,
};



esp_err_t xPostDeviceType(httpd_req_t *req)
{

	httpd_resp_send(req, (char *)pcPostResFail, strlen(pcPostResFail));

	return ESP_OK;

}

httpd_uri_t xPostDeviceTypeUri = {
		.uri       = "/device",
		.method    = HTTP_POST,
		.handler   = xPostDeviceType,
		.user_ctx  = NULL,
};

esp_err_t xGetDeviceInfo(httpd_req_t *req)
{

	ESP_LOGI("", "Device Info");
	cJSON *root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "device_type", DEVICE_TYPE);
	char *res;
	res = cJSON_PrintUnformatted(root);
	httpd_resp_send(req, (char *)res, strlen(res));
	cJSON_Delete(root);
	return ESP_OK;
}

httpd_uri_t xGetInfoUri = {
		.uri       = "/info",
		.method    = HTTP_GET,
		.handler   = xGetDeviceInfo,
		.user_ctx  = NULL,
};

esp_err_t xPostrelaycmd(httpd_req_t *req)
{
	char buf[256];
	gpio_set_level(COMM_STATUS_LED_PIN, LED_ON);
	gpio_set_level(CLIENT_CONNECT_LED_PIN, LED_ON);
	httpd_req_get_url_query_str(req, buf, sizeof(buf));
	printf("Requested uri = %s\n",buf);
	char relay_no[3]={0};
	char relay_val[2]={0};
	httpd_query_key_value(buf, "relay", relay_no, sizeof(relay_no));
	httpd_query_key_value(buf, "state", relay_val, sizeof(relay_val));

	printf("Relay No = %s is ",relay_no);
	atoi(relay_val)? printf("ON\n"):printf("OFF\n");
	
	if(atoi(relay_no) == 0)
	{
		uint8_t i = 0;
		
		for(i=0;i<MAX_RELAY;i++)
		{
			gpio_set_level(xObjRelay[i].gpioNo, atoi(relay_val) && 0x01);
		}
	}
	else
	{
		gpio_set_level(xObjRelay[atoi(relay_no)-1].gpioNo, atoi(relay_val) && 0x01);
	}
	
	httpd_resp_send(req, (char *)pcPostResOk, strlen(pcPostResOk));
	gpio_set_level(COMM_STATUS_LED_PIN, LED_OFF);
	gpio_set_level(CLIENT_CONNECT_LED_PIN, LED_OFF);
	return ESP_OK;
}

httpd_uri_t xPostRelayCMDUri = {
		.uri       = "/relaycmd",
		.method    = HTTP_GET,
		.handler   = xPostrelaycmd,
		.user_ctx  = NULL,
};


void startServer()
{
	if(xServer != NULL)
		return;
	ESP_LOGI("", "Stared Server for provisioning");

	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.max_uri_handlers = 15;
	config.uri_match_fn = httpd_uri_match_wildcard;
	if(httpd_start(&xServer, &config) == ESP_OK)
	{

		httpd_register_uri_handler(xServer, &xGetProvStateUri);
		httpd_register_uri_handler(xServer, &xPostProvCMDUri);
		httpd_register_uri_handler(xServer, &xGetWifiSSIDListUri);
		httpd_register_uri_handler(xServer, &xGetRootESPUri);
		httpd_register_uri_handler(xServer, &xPostWifiCredUri);
		httpd_register_uri_handler(xServer, &xGetInfoUri);
		httpd_register_uri_handler(xServer, &xPostDeviceTypeUri);
		httpd_register_uri_handler(xServer, &xPostRelayCMDUri);
		httpd_register_uri_handler(xServer, &xPostSetStaticIpUri);
		httpd_register_uri_handler(xServer, &xGetIndexPageUri);
		httpd_register_uri_handler(xServer, &xGetSettingsPageUri);
		
			
	}			

}