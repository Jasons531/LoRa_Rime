/*
**************************************************************************************************************
*	@file	LED.c
*	@author Jason_531@163.com
*	@version V0.0.1
*	@date    
*	@brief Led״̬����
***************************************************************************************************************
*/

#include <stdint.h>
#include "debug.h"
#include "led.h"

LedStates_t LedStates;

/*
*����LED״̬
*/
void SetLedStates(LedStates_t States)
{	
	LedStates = States;
	
	DEBUG_APP(3,"LedStates = %d",LedStates);
}

/*
*��ԭLED״̬
*/
void RestLedStates(LedStates_t States)
{
	LedStates = States;
	
	DEBUG_APP(2,"RestLedStates = %d",LedStates);
}

/*
*��ȡLED״̬
*/
LedStates_t GetLedStates(void)
{
	LedStates_t States;
	
	States = LedStates;
	
	return States;
}

/*
*LED��ʼ��
*/
void LedInit(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_GPIOC_CLK_ENABLE(  );          

	GPIO_Initure.Pin=LORA_LED_PIN;  
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  
	GPIO_Initure.Pull=GPIO_PULLUP;          
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     
	HAL_GPIO_Init(LORA_LED,&GPIO_Initure);
	
	SetLedStates(NoneCare);
		
	LedOff(  );
}

/*
*LED��
*/
void LedOn(void)
{
	HAL_GPIO_WritePin(LORA_LED,LORA_LED_PIN,GPIO_PIN_SET);
}

/*
*LED��
*/
void LedOff(void)
{
	HAL_GPIO_WritePin(LORA_LED,LORA_LED_PIN,GPIO_PIN_RESET);
}

/*
*LED��ת
*/
void LedToggle(void)
{
	HAL_GPIO_TogglePin(LORA_LED,LORA_LED_PIN);
}

/*
*�������ݳɹ�LED״̬
*/
void LedSendSucess(int8_t Counter)
{
	if(GetLedStates(  ) == GpsLocation)  ///��ԭ��λ��ʱ��
	HAL_TIM_Base_Stop_IT(&htim2);   ///��λ���̹رն�ʱ������ֹLED״̬����
		
	for( int8_t i = Counter; i > 0; i -- )
	{
		LedToggle(  );
		HAL_Delay(500);
	}
	LedOff(  );
	
	if(GetLedStates(  ) == GpsLocation)  ///��ԭ��λ��ʱ��
	{
		HAL_TIM_Base_Start_IT(&htim2);
	}

}

/*
*��������ʧ��LED״̬
*/
void LedSendFail(int8_t Counter)
{
	for( int8_t i = Counter; i > 0; i -- )
	{
		LedOn(  );
		HAL_Delay(1000);
		LedOff(  );
		HAL_Delay(200);
	}
}

/*
*��������LED״̬
*/
void LedRev(int8_t Counter)
{
	if(GetLedStates(  ) == GpsLocation)  ///��ԭ��λ��ʱ��
	HAL_TIM_Base_Stop_IT(&htim2);   ///��λ���̹رն�ʱ������ֹLED״̬����
	
	for( int8_t i = Counter; i > 0; i -- )
	{
		LedToggle(  );
		HAL_Delay(200);
	}
	LedOff(  );
	
	if(GetLedStates(  ) == GpsLocation)  ///��ԭ��λ��ʱ��
	{
		HAL_TIM_Base_Start_IT(&htim2);
		DEBUG_APP(3,"----Start_IT----");
	}
}

/*
*LED����
*/
void LedDisplay(void)
{
	
	switch(GetLedStates(  ))
	{
		case GpsLocation:
			
					DEBUG_APP(3,"LedStates = %d",GpsLocation);
					LedToggle(  );	
			
		break;
		
		case SendSucess:
			
		break;
		
		case SendFail:   ////����1S������200ms
			
		break;
		
		case Receive:
			
		break;
		
		default:
			
		break;
	}
}
