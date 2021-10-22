
#include "sdkconfig.h"






#include "commondef.h"
#include "wifiConnectivity.h"
// #include "mqtt.h"
// #include "timesync.h"
#include "data-storage.h"
// #include "cmdAction.h"
// #include "peripheralControl.h"
// #include "peripheralInit.h"
// #include "ota.h"

void task_heap_memory_print(void *pv)
{
	while(1)
	{
		ESP_LOGI("", "Free memory: %d bytes Version: %s", esp_get_free_heap_size(), VERSION);
		vTaskDelay(pdMS_TO_TICKS(3*1000));
	}
}

void app_main(void)
{

	ESP_LOGI("", " \n\n Hello world! Version = [%s]\n\n", VERSION);

	commonInit();

	ESP_ERROR_CHECK(initDataStorage());

	get_stored_data();
	// init_scheduler();

	ESP_ERROR_CHECK(init_wifi());
	// xTaskCreate(&sevenSegmentControlTask, "SevenSegment_Task", 4096, NULL, 5, NULL);

	// xTaskCreate(&taskSyncTime, "ntp_task", 4096, NULL, 5, NULL);
	// xTaskCreate(&peripheralControlTask, "Peripheral_control", 4096, NULL, 5, NULL);
	// xTaskCreate(&zcrTask, "DummyTask", 4096, NULL, 5, NULL);
	// xTaskCreate(&cmdActionTask, "Action_cmd", 4096, (void *)&xQueueAction, 5, NULL);

	// xTaskCreate(&SwitchMonitorTask, "Switch_input", 4096, NULL, 4, NULL);

	// xTaskCreate(&task_heap_memory_print, "heap_print", 4096, NULL, 5, NULL);
	// start_station();
	// if(xObjProvData.isProvisioned != 1)
	// {
	// 	printf("In provisioning mode\n");
		// start_provisioning();
	// }
	// else
	// {
		xObjProvData.isProvisioned = 1;
		start_station();
		// xTaskCreate(&simple_ota_example_task, "OTA_task", 16*1024, NULL, 5, NULL);
		// mqttStart();

	// }

}

