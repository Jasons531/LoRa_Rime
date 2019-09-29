/*
**************************************************************************************************************
*	@file	delay.h
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	ÑÓÊ±º¯Êý
***************************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DELAY_H
#define __DELAY_H

#include <stdint.h>

#define RTC_CNT Tick

void DelayInit(uint8_t SYSCLK);
void delay_us(uint16_t nus);

void delay_ms(uint16_t i);
void Delay( float s );
void DelayMs( uint16_t ms );

#endif /* __DELAY_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
