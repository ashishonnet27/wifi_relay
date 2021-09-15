/*
 * peripheralInit.h
 *
 *  Created on: 13-Nov-2019
 *      Author: shiv
 */

#ifndef MAIN_PERIPHERALINIT_H_
#define MAIN_PERIPHERALINIT_H_

#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"

#include "userSturct.h"

#define I2C_DUMMY			0

#define I2C_PORT_NO			I2C_NUM_0

#define I2C_SDA_IO			21		//GPIO21
#define I2C_SCL_IO			22		//GPIO22
#define I2C_FREQ			100000	//100KHz

#define I2C_WRITE_BIT		0x00    //I2C master write
#define I2C_READ_BIT		0x01    // I2C master read
#define I2C_ACK_CHECK_EN 	0x01    // I2C master will check ack from slave
#define I2C_ACK_CHECK_DIS 	0x00    // I2C master will not check ack from slave
#define I2C_ACK_VAL 		0x00    // I2C ack value
#define I2C_NACK_VAL 		0x01    // I2C nack value

//PCA Registers
#define	PCA9555_ADDR		(0x40)
#define PCA9555_InP0		(0x00)
#define PCA9555_InP1		(0x01)
#define PCA9555_OutP0		(0x02)
#define PCA9555_OutP1		(0x03)
#define PCA9555_PInvP0		(0x04)
#define PCA9555_PInvP1		(0x05)
#define PCA9555_ConfP0		(0x06)
#define PCA9555_ConfP1		(0x07)

//I2C Related Functions
esp_err_t i2c_master_init(void);
esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t slave_addr, uint8_t *data_rd, size_t size);
esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t slave_addr, uint8_t *data_wr, size_t size);

esp_err_t i2c_master_read_reg(i2c_port_t i2c_num, uint8_t slave_addr, uint8_t reg_addr, uint8_t *data_rd, size_t size);
esp_err_t i2c_master_write_reg(i2c_port_t i2c_num, uint8_t slave_addr,uint8_t reg_addr, uint8_t *data_wr, size_t size);

//PCA Related Functions
esp_err_t xPCA9555ConfigureP0(uint8_t);
esp_err_t xPCA9555ConfigureP1(uint8_t);
esp_err_t xPCA9555WriteP0(uint8_t);
esp_err_t xPCA9555WriteP1(uint8_t);
esp_err_t xPCA9555ReadP0(uint8_t *);
esp_err_t xPCA9555ReadP1(uint8_t *);

//7-Segment control functions
void vShowHexDigit(uint8_t);
void vShowBlank();
void vShowFull();
void vShowDot(uint8_t);
void vShowNoDigit();
void vShowDash();


//------------- MCP79410 ----------------------------------//

#define	MCP79410_ADDR				(0xDE)

#define MCP79410_REG_Sec			(0x00)
#define MCP79410_REG_Min			(0x01)
#define MCP79410_REG_Hour			(0x02)
#define MCP79410_REG_WDay			(0x03)
#define MCP79410_REG_Date			(0x04)
#define MCP79410_REG_Month			(0x05)
#define MCP79410_REG_Year			(0x06)

#define MCP79410_REG_Control		(0x07)
#define MCP79410_REG_OscTrim		(0x08)
#define MCP79410_REG_EEUnlock		(0x09)

#define MCP79410_REG_Alm0_Sec		(0x0A)
#define MCP79410_REG_Alm0_Min		(0x0B)
#define MCP79410_REG_Alm0_Hour		(0x0C)
#define MCP79410_REG_Alm0_WDay		(0x0D)
#define MCP79410_REG_Alm0_Date		(0x0E)
#define MCP79410_REG_Alm0_Month		(0x0F)

#define MCP79410_REG_Alm1_Sec		(0x11)
#define MCP79410_REG_Alm1_Min		(0x12)
#define MCP79410_REG_Alm1_Hour		(0x13)
#define MCP79410_REG_Alm1_WDay		(0x14)
#define MCP79410_REG_Alm1_Date		(0x15)
#define MCP79410_REG_Alm1_Month		(0x16)

#define MCP79410_REG_PWRDN_Min		(0x18)
#define MCP79410_REG_PWRDN_Hour		(0x19)
#define MCP79410_REG_PWRDN_Date		(0x1A)
#define MCP79410_REG_PWRDN_Month 	(0x1B)

#define MCP79410_REG_PWRUP_Min		(0x1C)
#define MCP79410_REG_PWRUP_Hour		(0x1D)
#define MCP79410_REG_PWRUP_Date		(0x1E)
#define MCP79410_REG_PWRUP_Month 	(0x1F)


esp_err_t xInitMCP79410();
esp_err_t xReadMCP79410(xMCP79410RTC *px);
esp_err_t xWriteMCP79410(xMCP79410RTC);

#endif /* MAIN_PERIPHERALINIT_H_ */
