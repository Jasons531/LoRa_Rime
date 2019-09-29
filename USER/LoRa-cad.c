/*
**************************************************************************************************************
*	@file	LoRa-cad.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	应用层头文件：连接MAC层
***************************************************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include "LoRa-cad.h"

/******************************12路选择：TX1:0 --- 11**************************************/
#define LC4                { 470000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC5                { 472000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC6                { 474000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC7                { 476000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC8                { 478000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }

#define LC9                { 480000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC10               { 482000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC11               { 484000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC12               { 486000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }

/******************************12路选择：RX1:12 --- 23: 对应网关下行数据：4MHZ隔离**************************************/
#define LC13               { 488000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC14               { 490000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC15               { 492000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC16               { 494000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }

#define LC17               { 496000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC18               { 498000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC19               { 500000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC20               { 502000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC21               { 504000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC22               { 506000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC23               { 508000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC24               { 510000000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }



/************************设置CAD模式参数**************************/
LoRaCsma_t LoRaCsma = {false, false, false, false, 0};

TimerEvent_t CsmaTimer;
volatile bool CsmaTimerEvent = false;
void OnCsmaTimerEvent( void )
{  
	TimerStop( &CsmaTimer );

	LoRaCsma.Disturb = false;
	
	CsmaTimerEvent = true;
}

/*
*LoRaMacChannelAddFun：增加设备频段
*参数：                无
*返回值：              无
*/
void LoRaMacChannelAddFun( void )
{
	LoRaMacChannelAdd( 3, ( ChannelParams_t )LC4  );
	LoRaMacChannelAdd( 4, ( ChannelParams_t )LC5  );
	LoRaMacChannelAdd( 5, ( ChannelParams_t )LC6  );
	LoRaMacChannelAdd( 6, ( ChannelParams_t )LC7  );
	LoRaMacChannelAdd( 7, ( ChannelParams_t )LC8  );
	LoRaMacChannelAdd( 8, ( ChannelParams_t )LC9  );
	LoRaMacChannelAdd( 9, ( ChannelParams_t )LC10 );
	LoRaMacChannelAdd( 10,( ChannelParams_t )LC11 );
	LoRaMacChannelAdd( 11,( ChannelParams_t )LC12 );
	LoRaMacChannelAdd( 12,( ChannelParams_t )LC13 );
	LoRaMacChannelAdd( 13,( ChannelParams_t )LC14 );
	LoRaMacChannelAdd( 14,( ChannelParams_t )LC15 );
	LoRaMacChannelAdd( 15,( ChannelParams_t )LC16 );
	LoRaMacChannelAdd( 16,( ChannelParams_t )LC17 );
	LoRaMacChannelAdd( 17,( ChannelParams_t )LC18 );
	LoRaMacChannelAdd( 18,( ChannelParams_t )LC19 );
	LoRaMacChannelAdd( 19,( ChannelParams_t )LC20 );
	LoRaMacChannelAdd( 20,( ChannelParams_t )LC21 );
	LoRaMacChannelAdd( 21,( ChannelParams_t )LC22 );
	LoRaMacChannelAdd( 22,( ChannelParams_t )LC23 );
	LoRaMacChannelAdd( 23,( ChannelParams_t )LC24 );
}

/*
 * LoRaCadInit:	 CAD初始化
 * 参数:	     无
 * 返回值:		 无
*/
void LoRaCadInit(void)
{
	Radio.Standby();
	Radio.StartCad( );  // Set the device into CAD mode
}

float SymbolTime(void)
{
	LoRaCsma.symbolTime = 0;
	uint8_t LORA_SPREADING_FACTOR = 0;
	
	if(Sx127xHandle.DefaultDatarate == 0)
		LORA_SPREADING_FACTOR = 12;
	else if(Sx127xHandle.DefaultDatarate == 1)
		LORA_SPREADING_FACTOR = 11;
	else if(Sx127xHandle.DefaultDatarate == 2)
		LORA_SPREADING_FACTOR = 10;
	else if(Sx127xHandle.DefaultDatarate == 3)
		LORA_SPREADING_FACTOR = 9;
	else if(Sx127xHandle.DefaultDatarate == 4)
		LORA_SPREADING_FACTOR = 8;
	else 
		LORA_SPREADING_FACTOR = 7;
	
	 LoRaCsma.symbolTime = (( pow( (float)2, (float)LORA_SPREADING_FACTOR ) ) + 32 ) / 125000;  // SF7 and BW = 125 KHz
	 LoRaCsma.symbolTime = LoRaCsma.symbolTime * 1000000;  // symbol Time is in us
	 DEBUG_APP(3,"LORA_SPREADING_FACTOR = %d symbolTime = %d",LORA_SPREADING_FACTOR,LoRaCsma.symbolTime);
	 return LoRaCsma.symbolTime;
}

