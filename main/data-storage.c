/*
 * data-storage.c
 *
 *  Created on: 06-Oct-2019
 *      Author: shiv
 */


#include "data-storage.h"
#include "commondef.h"

const char file_scheduler[] = "/spiffs/scheduler";

esp_err_t initDataStorage()
{
	esp_vfs_spiffs_conf_t conf = {
			.base_path = "/spiffs",
			.partition_label = NULL,
			.max_files = 15,
			.format_if_mount_failed = false
	};

	// Use settings defined above to initialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is an all-in-one convenience function.
	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK)
	{
		if (ret == ESP_FAIL)
		{
			ESP_LOGE("", "Failed to mount or format filesystem");
		}
		else if (ret == ESP_ERR_NOT_FOUND)
		{
			ESP_LOGE("", "Failed to find SPIFFS partition");
		}
		else
		{
			ESP_LOGE("", "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}
	}

	return ret;
}



void read_stored_prov_data()
{
	FILE *fp;
	fp = fopen("/spiffs/prov", "rb");
	if(fp == NULL)
		return;
	fread(&xObjProvData, sizeof(xObjProvData), 1, fp);
	fclose(fp);

	printf("PROV: [%d]\n", xObjProvData.isProvisioned);
	printf("SSID: [%s]\n", xObjProvData.ssid);
	printf("PASS: [%s]\n", xObjProvData.password);

}

esp_err_t update_prov_data()
{
	FILE *fp;

	fp = fopen("/spiffs/prov", "wb");
	if(fp == NULL)
		return ESP_FAIL;
	fwrite(&xObjProvData, sizeof(xObjProvData), 1, fp);
	fclose(fp);

	return ESP_OK;
}
esp_err_t get_stored_data()
{
	size_t total = 0, used = 0;
	esp_err_t ret = esp_spiffs_info(NULL, &total, &used);
	if (ret != ESP_OK)
	{
		ESP_LOGE("", "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
	}
	else
	{
		ESP_LOGI("", "Partition size: total: %d, used: %d", total, used);
	}

	// Read and display the contents of a small text file (hello.txt)
	//read_hello_txt();

	read_stored_prov_data();


	// getDeviceTimeZone(&device_timezone);
	// printf("\n\n%s\n\n", device_timezone.tzone);

	// memset(&xObjPumpLOCK, 0, sizeof(xObjPumpLOCK));
	// strcpy(xObjPumpLOCK.fileName, "/spiffs/PLOCK");
	// readLockData(&xObjPumpLOCK);

	return ret;
}

/*
void readStoredSchedulerData(xRLScheduler *px)
{
	FILE *fp;
	fp = fopen(px->fileName, "rb");
	if(fp == NULL)
		return;
	fread(px, sizeof(*px), 1, fp);
	fclose(fp);
}

void update_pump_lock_data(xPumpLOCK *px)
{

	FILE *fp;
	fp = fopen(px->fileName, "wb");
	if(fp == NULL)
		return;
	fwrite(px, sizeof(*px), 1, fp);
	fclose(fp);
	ESP_LOGI("", "Lock Data is stored");
}

void readLockData(xPumpLOCK *px)
{
	ESP_LOGE("", "Reading Lock Data ");
	FILE *fp;
	fp = fopen(px->fileName, "rb");
	if(fp == NULL)
		return;
	fread(px, sizeof(*px), 1, fp);
	fclose(fp);
	ESP_LOGE("", "Lock Data is read %d", px->lock);
}

void updateStoredSchedulerData(xRLScheduler *px)
{
	FILE *fp;
	fp = fopen(px->fileName, "wb");
	if(fp == NULL)
		return;
	fwrite(px, sizeof(*px), 1, fp);
	fclose(fp);
}

void getDeviceTimeZone(device_time_zone_def *px)
{
	memset(px, 0, sizeof(device_time_zone_def));
	FILE *fp;
	fp = fopen("/spiffs/tzone", "rb" );
	if(fp == NULL)
		return;
	fread(px, sizeof(device_timezone), 1, fp);
	fclose(fp);
}
void set_device_time_zone(device_time_zone_def *px)
{
	FILE *fp;
	fp = fopen("/spiffs/tzone", "wb" );
	if(fp == NULL)
		return;
	fwrite(px, sizeof(device_time_zone_def), 1, fp);
	fclose(fp);

	getDeviceTimeZone(&device_timezone);
	setenv("TZ", device_timezone.tzone, 1);
	tzset();
}

void init_scheduler()
{
	long int filesize;
	sched_def sched;
	size_t bytes;
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;
	fp = fopen(file_scheduler, "a");
	if(fp == NULL)
	{
		xSemaphoreGive(file_operation_mutex);
		return;
	}
	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	ESP_LOGE("", "File size = %ld %d %d %d", filesize, MAX_SCHED, sizeof(sched_def), MAX_SCHED*sizeof(sched_def));
	if( filesize != (MAX_SCHED*sizeof(sched_def)))
	{
		fclose(fp);
		fp = fopen(file_scheduler, "w");
		if(fp == NULL)
		{
			xSemaphoreGive(file_operation_mutex);
			return;
		}
		ESP_LOGE("", "Scheduler file needs to create");
		fseek(fp, 0, SEEK_SET);
		memset(&sched, 0, sizeof(sched_def));
		for(int i = 0 ; i < MAX_SCHED; i++)
			bytes = fwrite(&sched, 1, sizeof(sched_def), fp);
		ESP_LOGE("","Bytes written = %d\n", bytes);
	}
	else
	{
		ESP_LOGE("","Scheduler file exists");
	}
	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
}

void scheduler_read(uint8_t number, sched_def *sched)
{
	if((number < 1) || (number > MAX_SCHED))
	{
		ESP_LOGE("", "Error in scheduler argument");
		memset((void *)sched, 0, sizeof(sched_def));
		return;
	}
	number--;
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;
	fp = fopen(file_scheduler, "rb");
	if(fp == NULL)
	{
		ESP_LOGE("", "Failed to open scheduler file");
		memset((void *)sched, 0, sizeof(sched_def));
		xSemaphoreGive(file_operation_mutex);
		return;
	}
	fseek(fp, number*sizeof(sched_def), SEEK_SET);
	fread((void *)sched, 1, sizeof(sched_def), fp);
	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
}

void scheduler_update(uint8_t number, sched_def sched)
{
	size_t bytes;
	if((number < 1) || (number > MAX_SCHED))
	{
		ESP_LOGE("", "Error in scheduler argument");
		return;
	}
	number--;
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;
	fp = fopen(file_scheduler, "rb+");
	if(fp == NULL)
	{
		ESP_LOGE("", "Failed to open scheduler file");
		xSemaphoreGive(file_operation_mutex);
		return;
	}
	ESP_LOGI("", "In data : Day-%d Port-%d Dur-%d HH-%d MM-%d EN-%d", sched.day, sched.port,
			sched.duration, sched.start_time_HH, sched.start_time_MM, sched.enable);
	fseek(fp, number*sizeof(sched_def), SEEK_SET);
	ESP_LOGI("", "Position = %ld", ftell(fp));
	bytes = fwrite((void *)&sched, 1, sizeof(sched_def), fp);
	ESP_LOGE("", "Bytes written = %d", bytes);
	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
}

*/