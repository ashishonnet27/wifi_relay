/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "driver/gpio.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "commondef.h"
#include "wifiConnectivity.h"
#include "data-storage.h"
#include "tcp_server.h"



static const char *TAG = "tcp_server";



const char *Relay1_On_Cmd = "Relay1_ON";
const char *Relay2_On_Cmd = "Relay2_ON";
const char *Relay3_On_Cmd = "Relay3_ON";
const char *Relay4_On_Cmd = "Relay4_ON";
const char *Relay5_On_Cmd = "Relay5_ON";
const char *Relay6_On_Cmd = "Relay6_ON";
const char *Relay7_On_Cmd = "Relay7_ON";
const char *Relay8_On_Cmd = "Relay8_ON";
const char *All_Relay_On_Cmd = "All_Relay_ON";

const char *Relay1_Off_Cmd = "Relay1_OFF";
const char *Relay2_Off_Cmd = "Relay2_OFF";
const char *Relay3_Off_Cmd = "Relay3_OFF";
const char *Relay4_Off_Cmd = "Relay4_OFF";
const char *Relay5_Off_Cmd = "Relay5_OFF";
const char *Relay6_Off_Cmd = "Relay6_OFF";
const char *Relay7_Off_Cmd = "Relay7_OFF";
const char *Relay8_Off_Cmd = "Relay8_OFF";
const char *All_Relay_Off_Cmd = "All_Relay_OFF";


static void do_retransmit(const int sock)
{
    int len;
    char rx_buffer[128];

    do {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        } else if (len == 0) {
            ESP_LOGW(TAG, "Connection closed");
        } else {
            rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
            ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
			gpio_set_level(COMM_STATUS_LED_PIN, LED_ON);
            // send() can return less bytes than supplied length.
            // Walk-around for robust implementation. 
            // int to_write = len;
            // while (to_write > 0) {
            //     int written = send(sock, rx_buffer + (len - to_write), to_write, 0);
            //     if (written < 0) {
            //         ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            //     }
            //     to_write -= written;
            // }
			if(strcmp(rx_buffer,Relay1_On_Cmd) ==0)
			{
				gpio_set_level(xObjRelay[0].gpioNo, 1);
                xObjProvRelayData.relay_state = xObjProvRelayData.relay_state | 1;
				update_prov_relay_data();
			}
			else if(strcmp(rx_buffer,Relay1_Off_Cmd) ==0)
			{
				gpio_set_level(xObjRelay[0].gpioNo, 0);
				xObjProvRelayData.relay_state = xObjProvRelayData.relay_state & ~(1);
				update_prov_relay_data();
			}
            if(strcmp(rx_buffer,Relay2_On_Cmd) ==0)
			{
				gpio_set_level(xObjRelay[1].gpioNo, 1);
                xObjProvRelayData.relay_state = xObjProvRelayData.relay_state | 1 << 1;
				update_prov_relay_data();
			}
			else if(strcmp(rx_buffer,Relay2_Off_Cmd) ==0)
			{
				gpio_set_level(xObjRelay[1].gpioNo, 0);
				xObjProvRelayData.relay_state = xObjProvRelayData.relay_state & ~(1 << 1);
				update_prov_relay_data();
			}
            if(strcmp(rx_buffer,Relay3_On_Cmd) ==0)
			{
				gpio_set_level(xObjRelay[2].gpioNo, 1);
                xObjProvRelayData.relay_state = xObjProvRelayData.relay_state | 1 << 2;
				update_prov_relay_data();
			}
			else if(strcmp(rx_buffer,Relay3_Off_Cmd) ==0)
			{
				gpio_set_level(xObjRelay[2].gpioNo, 0);
				xObjProvRelayData.relay_state = xObjProvRelayData.relay_state & ~(1 << 2);
				update_prov_relay_data();
			}
            if(strcmp(rx_buffer,Relay4_On_Cmd) ==0)
			{
				gpio_set_level(xObjRelay[3].gpioNo, 1);
                xObjProvRelayData.relay_state = xObjProvRelayData.relay_state | 1 << 3;
				update_prov_relay_data();
			}
			else if(strcmp(rx_buffer,Relay4_Off_Cmd) ==0)
			{
				gpio_set_level(xObjRelay[3].gpioNo, 0);
				xObjProvRelayData.relay_state = xObjProvRelayData.relay_state & ~(1 << 3);
				update_prov_relay_data();
			}
            if(strcmp(rx_buffer,Relay5_On_Cmd) ==0)
			{
				gpio_set_level(xObjRelay[4].gpioNo, 1);
                xObjProvRelayData.relay_state = xObjProvRelayData.relay_state | 1 << 4;
				update_prov_relay_data();
			}
			else if(strcmp(rx_buffer,Relay5_Off_Cmd) ==0)
			{
				gpio_set_level(xObjRelay[4].gpioNo, 0);
				xObjProvRelayData.relay_state = xObjProvRelayData.relay_state & ~(1 << 4);
				update_prov_relay_data();
			}
            if(strcmp(rx_buffer,Relay6_On_Cmd) ==0)
			{
				gpio_set_level(xObjRelay[5].gpioNo, 1);
                xObjProvRelayData.relay_state = xObjProvRelayData.relay_state | 1 << 5;
				update_prov_relay_data();
			}
			else if(strcmp(rx_buffer,Relay6_Off_Cmd) ==0)
			{
				gpio_set_level(xObjRelay[5].gpioNo, 0);
				xObjProvRelayData.relay_state = xObjProvRelayData.relay_state & ~(1 << 5);
				update_prov_relay_data();
			}
            if(strcmp(rx_buffer,Relay7_On_Cmd) ==0)
			{
				gpio_set_level(xObjRelay[6].gpioNo, 1);
                xObjProvRelayData.relay_state = xObjProvRelayData.relay_state | 1 << 6;
				update_prov_relay_data();
			}
			else if(strcmp(rx_buffer,Relay7_Off_Cmd) ==0)
			{
				gpio_set_level(xObjRelay[6].gpioNo, 0);
				xObjProvRelayData.relay_state = xObjProvRelayData.relay_state & ~(1 << 6);
				update_prov_relay_data();
			}
            if(strcmp(rx_buffer,Relay8_On_Cmd) ==0)
			{
				gpio_set_level(xObjRelay[7].gpioNo, 1);
                xObjProvRelayData.relay_state = xObjProvRelayData.relay_state | 1 << 7;
				update_prov_relay_data();
			}
			else if(strcmp(rx_buffer,Relay8_Off_Cmd) ==0)
			{
				gpio_set_level(xObjRelay[7].gpioNo, 0);
				xObjProvRelayData.relay_state = xObjProvRelayData.relay_state & ~(1 << 7);
				update_prov_relay_data();
			}
			else if(strcmp(rx_buffer,All_Relay_On_Cmd) ==0)
			{
                int j;
                for(j=0;j<8;j++)
                {
                    gpio_set_level(xObjRelay[j].gpioNo, 1);
                }
                xObjProvRelayData.relay_state = 0xFF;
                update_prov_relay_data();
			}
            else if(strcmp(rx_buffer,All_Relay_Off_Cmd) ==0)
			{
                int j;
                for(j=0;j<8;j++)
                {
                    gpio_set_level(xObjRelay[j].gpioNo, 0);
                }
                xObjProvRelayData.relay_state = 0;
                update_prov_relay_data();
			}
            else{

            }
			gpio_set_level(COMM_STATUS_LED_PIN, LED_OFF);
            
        }
    } while (len > 0);
}

void tcp_server_task(void *pvParameters)
{
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    } else if (addr_family == AF_INET6) {
        bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
        dest_addr.sin6_family = AF_INET6;
        dest_addr.sin6_port = htons(PORT);
        ip_protocol = IPPROTO_IPV6;
    }

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
// #if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
//     // Note that by default IPV6 binds to both protocols, it is must be disabled
//     // if both protocols used at the same time (used in CI)
//     int opt = 1;
//     setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
//     setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
// #endif

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1) {
		gpio_set_level(CLIENT_CONNECT_LED_PIN, LED_OFF);
        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
        uint addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Convert ip address to string
        if (source_addr.sin6_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
        } else if (source_addr.sin6_family == PF_INET6) {
            inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);
		gpio_set_level(CLIENT_CONNECT_LED_PIN, LED_ON);
        do_retransmit(sock);

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}




