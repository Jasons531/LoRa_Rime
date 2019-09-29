/*
**************************************************************************************************************
*	@file	delay.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief 延时函数
***************************************************************************************************************
*/

#include <stdint.h>
#include "delay.h"
#include "board.h"
								    

//延时nus
//nus为要延时的us数.	
//注意:nus的值不要大于1000us
void delay_us(uint16_t nus)
{		
	clock_delay_usec(nus);
}

void delay_ms(uint16_t tick_1ms)
{
	uint16_t i;
	for(i=0;i<tick_1ms;i++) 
	delay_us(1000);
}

void Delay( float s )
{
  DelayMs( s * 1000.0f );
}

extern bool LowPowerModeEnable;

void DelayMs( uint16_t ms )
{   
  delay_ms( ms );
}

