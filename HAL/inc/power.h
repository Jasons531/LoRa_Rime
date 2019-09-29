/*
**************************************************************************************************************
*	@file	power.h
*	@author Jason_531@163.com
*	@version V0.0.1
*	@date    
*	@brief	ÑÓÊ±º¯Êý
***************************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __POWER_H
#define __POWER_H

#include <stdint.h>

void BatteryInit(void);

int8_t CheckBattery(void);

uint16_t CheckRecharge(void);

int8_t ReadBattery(void);

void BatEnableCharge(void);

void BatDisableCharge(void);

#endif /* __POWER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
