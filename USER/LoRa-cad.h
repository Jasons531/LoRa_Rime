/*
**************************************************************************************************************
*	@file	LoRa-cad.h
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	应用层头文件：连接MAC层
***************************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LORA_CAD_H
#define __LORA_CAD_H

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include <stdio.h>
#include <stdint.h>

#define MAXLEN						    		 50
#define TIMEONAIR                       (Radio.TimeOnAir( MODEM_LORA, MAXLEN )/2)

typedef struct{
	bool Iq_Invert;
	bool Disturb;
	bool Cad_Done;
	bool Cad_Detect;
	uint32_t symbolTime;
}LoRaCsma_t;

extern LoRaCsma_t LoRaCsma;

extern TimerEvent_t CsmaTimer;
extern volatile bool CsmaTimerEvent;
void OnCsmaTimerEvent( void );
void LoRaMacChannelAddFun( void );

void LoRaCadInit(void);

float SymbolTime(void);

#endif /* __LoRa-cad_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
