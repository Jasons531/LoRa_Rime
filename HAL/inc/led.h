/*
**************************************************************************************************************
*	@file	led.h
*	@author Jason_531@163.com
*	@version V0.0.1
*	@date    
*	@brief	ÑÓÊ±º¯Êý
***************************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LED_H
#define __LED_H

#include <stdint.h>
#include "timer.h"

typedef enum ledStates
{
    NoneCare = 0,
    GpsLocation = 1,
    SendSucess = 2,
    SendFail = 3,
    Receive = 4,
}LedStates_t;

extern LedStates_t LedStates;

extern LedStates_t LedSaveState;

void SetLedStates(uint8_t States);

LedStates_t GetLedStates(void);

void LedInit(void);

void LedOn(void);

void LedOff(void);

void LedToggle(void);

void LedSendSucess(int8_t Counter);

void LedSendFail(int8_t Counter);

void LedRev(int8_t Counter);

void LedDisplay(void);


#endif /* __LED_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
