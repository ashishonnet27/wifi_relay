/*
 * peripheralInit.c
 *
 *  Created on: 13-Nov-2019
 *      Author: shiv
 */

/*		Digit   7	6	5	4	3	2	1	0
 * 				a	b	c	dp	d	e	g	f
 * 		0		1	1	1	0	1	1	0	1		0xED	0
 * 		1		0	0	0	0	0	1	0	1		0x05	1
 * 		2		1	1	0	0	1	1	1	0		0xCE	2
 * 		3		1	0	0	0	1	1	1	1		0x8F	3
 * 		4		0	0	1	0	0	1	1	1		0x27	4
 * 		5		1	0	1	0	1	0	1	1		0xAB	5
 * 		6		1	1	1	0	1	0	1	1		0xEB	6
 * 		7		0	0	0	0	1	1	0	1		0x0D	7
 * 		8		1	1	1	0	1	1	1	1		0xEF	8
 * 		9		1	0	1	0	1	1	1	1		0xAF	9
 * 		A		0	1	1	0	1	1	1	1		0x6F	A
 * 		B		1	1	1	0	0	0	1	1		0xE3	B
 * 		C		1	1	1	0	1	0	0	0		0xE8	C
 * 		D		1	1	0	0	0	1	1	1		0xC7	D
 * 		E		1	1	1	0	1	0	1	0		0xEA	E
 * 		F		0	1	1	0	1	0	1	0		0x6A	F
 * 		P		0	1	1	0	1	1	1	0		0x6E	P   16 	0x10
 * 		L		1	1	1	0	0	0	0	0		0xE0	L	17	0x11
 * 		o		1	1	0	0	0	0	1	1		0xC3	0	18	0x12
 * 		---		1	0	0	0	1	0	1	0		0x8A	---	19	0x13
 *
 *
 *
 */

#include "peripheralInit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "nvs_flash.h"

#include "driver/i2c.h"

uint8_t	i2cTempBuffer[32];
xSemaphoreHandle xI2CBusy;

char sevenSegment[] = {
		0xED,		//0
		0x05,		//1
		0xCE,		//2
		0x8F,		//3
		0x27,		//4
		0xAB,		//5
		0xEB,		//6
		0x0D,		//7
		0xEF,		//8
		0xAF,		//9
		0x6F,		//A
		0xE3,		//B
		0xE8,		//C
		0xC7,		//D
		0xEA,		//E
		0x6A,		//F
		0x6E,		//P  	0x10  	16
		0xE0,		//L	 	0x11	17
		0xC3,		//o	 	0x12	18
		0x8A,		//---	0x13	19
		0xC2		//c		0x14	20
};

char sevenSegmentDot = (1 << 4);

esp_err_t i2c_master_init(void)
{

#if I2C_DUMMY
	return ESP_OK;
#endif

	xI2CBusy = xSemaphoreCreateMutex();
	if(xI2CBusy == NULL)
	{
		ESP_LOGE("", "Error in Creating Mutex: xI2CBusy");
	}
	int i2c_master_port = I2C_PORT_NO;
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = I2C_SDA_IO;
	conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
	conf.scl_io_num = I2C_SCL_IO;
	conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
	conf.master.clk_speed = I2C_FREQ;
	//conf.clk_flags = 0;
	i2c_param_config(i2c_master_port, &conf);
	return i2c_driver_install(i2c_master_port, conf.mode,0, 0, 0);
}

esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t slave_addr, uint8_t *data_rd, size_t size)
{
#if I2C_DUMMY
	return ESP_OK;
#endif

	if (size == 0) {
		return ESP_OK;
	}

	xSemaphoreTake(xI2CBusy, portMAX_DELAY);
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (slave_addr ) | I2C_READ_BIT, I2C_ACK_CHECK_EN);
	if (size > 1) {
		i2c_master_read(cmd, data_rd, size - 1, I2C_ACK_VAL);
	}
	i2c_master_read_byte(cmd, data_rd + size - 1, I2C_NACK_VAL);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	xSemaphoreGive(xI2CBusy);
	return ret;
}

esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t slave_addr, uint8_t *data_wr, size_t size)
{
#if I2C_DUMMY
	return ESP_OK;
#endif
	xSemaphoreTake(xI2CBusy, portMAX_DELAY);
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (slave_addr ) | I2C_WRITE_BIT, I2C_ACK_CHECK_EN);
	i2c_master_write(cmd, data_wr, size, I2C_ACK_CHECK_EN);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	xSemaphoreGive(xI2CBusy);
	return ret;
}

esp_err_t i2c_master_read_reg(i2c_port_t i2c_num, uint8_t slave_addr, uint8_t reg_addr, uint8_t *data_rd, size_t size)
{
#if I2C_DUMMY
	return ESP_OK;
#endif
	if (size == 0) {
		return ESP_OK;
	}

	esp_err_t ret;

	xSemaphoreTake(xI2CBusy, portMAX_DELAY);
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (slave_addr ) | I2C_WRITE_BIT, I2C_ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg_addr, I2C_ACK_CHECK_EN);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	if(ret != ESP_OK)
	{
		xSemaphoreGive(xI2CBusy);
		return ret;
	}
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (slave_addr ) | I2C_READ_BIT, I2C_ACK_CHECK_EN);
	if (size > 1) {
		i2c_master_read(cmd, data_rd, size - 1, I2C_ACK_VAL);
	}
	i2c_master_read_byte(cmd, data_rd + size - 1, I2C_NACK_VAL);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);


	xSemaphoreGive(xI2CBusy);
	return ret;
}

esp_err_t i2c_master_write_reg(i2c_port_t i2c_num, uint8_t slave_addr, uint8_t reg_addr, uint8_t *data_wr, size_t size)
{
#if I2C_DUMMY
	return ESP_OK;
#endif
	xSemaphoreTake(xI2CBusy, portMAX_DELAY);
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (slave_addr ) | I2C_WRITE_BIT, I2C_ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg_addr, I2C_ACK_CHECK_EN);
	i2c_master_write(cmd, data_wr, size, I2C_ACK_CHECK_EN);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	xSemaphoreGive(xI2CBusy);
	return ret;
}

esp_err_t xPCA9555ConfigureP0(uint8_t val)
{
	return i2c_master_write_reg(I2C_PORT_NO, PCA9555_ADDR, PCA9555_ConfP0, &val, 1);
}

esp_err_t xPCA9555ConfigureP1(uint8_t val)
{
	return i2c_master_write_reg(I2C_PORT_NO, PCA9555_ADDR, PCA9555_ConfP1, &val, 1);
}

esp_err_t xPCA9555WriteP0(uint8_t val)
{
	return i2c_master_write_reg(I2C_PORT_NO, PCA9555_ADDR, PCA9555_OutP0, &val, 1);
}

esp_err_t xPCA9555WriteP1(uint8_t val)
{

	return i2c_master_write_reg(I2C_PORT_NO, PCA9555_ADDR, PCA9555_OutP1, &val, 1);
}

esp_err_t xPCA9555ReadP0(uint8_t *pVal)
{
	return i2c_master_read_reg(I2C_PORT_NO, PCA9555_ADDR, PCA9555_InP0, pVal, 1);
}

esp_err_t xPCA9555ReadP1(uint8_t *pVal)
{
	return i2c_master_read_reg(I2C_PORT_NO, PCA9555_ADDR, PCA9555_InP1, pVal, 1);
}

void vShowHexDigit(uint8_t u8)
{
#if I2C_DUMMY
	return ;
#endif
	char dot;
	char readvalue;
	esp_err_t rc;
	rc = xPCA9555ReadP1((uint8_t *)&readvalue);
	if(rc != ESP_OK)
	{
		ESP_LOGE("", "Error : Reading PCA9555 P1 : RC = %d", rc);
		return;
	}
	dot = readvalue & sevenSegmentDot;
	rc = xPCA9555WriteP1(sevenSegment[u8] | dot);
	if(rc != ESP_OK)
	{
		ESP_LOGE("", "Error : Writing PCA9555 P1 : RC = %d", rc);
	}
}
void vShowBlank()
{
	esp_err_t rc;
	rc = xPCA9555WriteP1(0x00);
	if(rc != ESP_OK)
	{
		ESP_LOGE("", "Error : Writing PCA9555 P1 Blank : RC = %d", rc);
	}
}
void vShowFull()
{
	esp_err_t rc;
	rc = xPCA9555WriteP1(0xFF);
	if(rc != ESP_OK)
	{
		ESP_LOGE("", "Error : Writing PCA9555 P1 Full : RC = %d", rc);
	}
}
void vShowDot(uint8_t u8Dot)
{
#if I2C_DUMMY
	return;
#endif
	char readvalue;
	char digit;
	esp_err_t rc;
	rc = xPCA9555ReadP1((uint8_t *)&readvalue);
	if(rc != ESP_OK)
	{
		ESP_LOGE("", "Error : Reading PCA9555 P1 : RC = %d", rc);
		return;
	}

	digit = readvalue & (~sevenSegmentDot);
	if(u8Dot)
		digit = digit | sevenSegmentDot;

	rc = xPCA9555WriteP1(digit);
	if(rc != ESP_OK)
	{
		ESP_LOGE("", "Error : Writing PCA9555 P1 ShowDot : RC = %d", rc);
	}
}

void vShowNoDigit()
{
#if I2C_DUMMY
	return;
#endif
	char readvalue;
	char digit;
	esp_err_t rc;
	rc = xPCA9555ReadP1((uint8_t *)&readvalue);
	if(rc != ESP_OK)
	{
		ESP_LOGE("", "Error : Reading PCA9555 P1 ShowNoDigit: RC = %d", rc);
		return;
	}
	digit = readvalue & (sevenSegmentDot);
	rc = xPCA9555WriteP1(digit);
	if(rc != ESP_OK)
	{
		ESP_LOGE("", "Error : Writing PCA9555 P1 ShowNoDigit: RC = %d", rc);
	}
}

void vShowDash()
{
	esp_err_t rc;
	rc = xPCA9555WriteP1(0x02);
	if(rc != ESP_OK)
	{
		ESP_LOGE("", "Error : Writing PCA9555 P1 Dash : RC = %d", rc);
	}
}

//-----------------------MCP79410-----------------------//

uint8_t decToBCD(uint8_t x)
{
	return (((x/10) << 4) | ((x%10) & 0x0F));
}

uint8_t bcdToDEC(uint8_t x)
{
	return ((x >> 4)*10 + (x & 0x0F));
}

esp_err_t xInitMCP79410()
{

	uint8_t u8Temp;

	//Set the control register
	u8Temp = 0xC3;
	if(ESP_OK != i2c_master_write_reg(I2C_PORT_NO, MCP79410_ADDR, MCP79410_REG_Control, &u8Temp, 1))
	{
		ESP_LOGE("", "Error in initialization of MCP79410 : 1");
		return ESP_FAIL;
	}

	//Battery enable
	u8Temp = 0x00;
	if(ESP_OK != i2c_master_read_reg(I2C_PORT_NO, MCP79410_ADDR, MCP79410_REG_WDay, &u8Temp, 1))
	{
		ESP_LOGE("", "Error in initialization of MCP79410 : 2");
		return ESP_FAIL;
	}
	u8Temp |= (1 << 3);
	if(ESP_OK != i2c_master_write_reg(I2C_PORT_NO, MCP79410_ADDR, MCP79410_REG_WDay, &u8Temp, 1))
	{
		ESP_LOGE("", "Error in initialization of MCP79410 : 3");
		return ESP_FAIL;
	}

	//Oscillator enable
	u8Temp = 0x00;
	if(ESP_OK != i2c_master_read_reg(I2C_PORT_NO, MCP79410_ADDR, MCP79410_REG_Sec, &u8Temp, 1))
	{
		ESP_LOGE("", "Error in initialization of MCP79410 : 4");
		return ESP_FAIL;
	}

	ESP_LOGE("", "MCP79410  Read %x", u8Temp);
	if(u8Temp & 0x80)
	{
		return ESP_OK;
	}
	else
	{
		u8Temp |= 0x80;
		ESP_LOGE("", "MCP79410  Read %x", u8Temp);
		if(ESP_OK != i2c_master_write_reg(I2C_PORT_NO, MCP79410_ADDR, MCP79410_REG_Sec, &u8Temp, 1))
		{
			ESP_LOGE("", "Error in initialization of MCP79410 : 5");
			return ESP_FAIL;
		}
	}
	return ESP_OK;
}

esp_err_t xReadMCP79410(xMCP79410RTC *px)
{
	memset(i2cTempBuffer, 0, sizeof(i2cTempBuffer));
	if(ESP_OK != i2c_master_read_reg(I2C_PORT_NO, MCP79410_ADDR, MCP79410_REG_Sec, i2cTempBuffer, 7))
	{
		ESP_LOGE("", "Error in reading MCP79410");
	}

	px->sec = bcdToDEC(i2cTempBuffer[0] & 0x7F);
	px->min = bcdToDEC(i2cTempBuffer[1]);
	px->hrs = bcdToDEC(i2cTempBuffer[2]);
	px->wday = bcdToDEC(i2cTempBuffer[3] & 0x07);
	px->mday = bcdToDEC(i2cTempBuffer[4]);
	px->mon = bcdToDEC(i2cTempBuffer[5] & 0x1F);
	px->yrs = bcdToDEC(i2cTempBuffer[6]);

	return ESP_OK;
}

esp_err_t xWriteMCP79410(xMCP79410RTC x)
{
	memset(i2cTempBuffer, 0, sizeof(i2cTempBuffer));

	i2cTempBuffer[0] = 0x80 | decToBCD(x.sec);
	i2cTempBuffer[1] = decToBCD(x.min);
	i2cTempBuffer[2] = decToBCD(x.hrs);
	i2cTempBuffer[3] = 0x08 | decToBCD(x.wday);
	i2cTempBuffer[4] = decToBCD(x.mday);
	i2cTempBuffer[5] = decToBCD(x.mon);
	i2cTempBuffer[6] = decToBCD(x.yrs);

	if(ESP_OK != i2c_master_write_reg(I2C_PORT_NO, MCP79410_ADDR, MCP79410_REG_Sec, i2cTempBuffer, 7))
	{
		ESP_LOGE("", "Error in writing data in MCP79410");
	}

	return ESP_OK;
}
