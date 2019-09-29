/*
**************************************************************************************************************
*	@file	power.c
*	@author Jason_531@163.com
*	@version V0.0.1
*	@date    
*	@brief 延时函数
***************************************************************************************************************
*/

#include <stdint.h>
#include "power.h"
#include "debug.h"
#include "adc.h"

#define VREFINT_CAL_ADDR                   							((uint16_t*) ((uint32_t)0x1FF80078U)) /* Internal voltage reference, address of parameter VREFINT_CAL: VrefInt ADC raw data acquired at temperature 30 DegC (tolerance: +-5 DegC), Vref+ = 3.0 V (tolerance: +-10 mV). */
#define VREFINT_CAL_VREF                   							((uint32_t) 3U)                    /* Analog voltage reference (Vref+) value with which temperature sensor has been calibrated in production (tolerance: +-10 mV) (unit: mV). */
#define VDD_APPLI                      		 						((uint32_t) 1220U)    /* Value of analog voltage supply Vdda (unit: mV) */
#define VFULL																	((uint32_t) 4095U)

void BatteryInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = OUT_CH_CE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(OUT_CH_CE_GPIO_Port, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = IN_CH_STAT1_Pin|IN_CH_STAT2_Pin|IN_CH_PG_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(OUT_CH_CE_GPIO_Port, &GPIO_InitStruct);
	
	BatEnableCharge(  );
}

/*
 * 开始充电
*/
void BatEnableCharge(void)
{
   HAL_GPIO_WritePin(OUT_CH_CE_GPIO_Port,OUT_CH_CE_Pin,GPIO_PIN_RESET);
}

/*
 * 关闭充电
*/
void BatDisableCharge(void)
{
   HAL_GPIO_WritePin(OUT_CH_CE_GPIO_Port,OUT_CH_CE_Pin,GPIO_PIN_SET);;
}

/*
*CheckBattery：读取电池电量
*返回值：		电量百分比
*/
int8_t CheckBattery(void)
{
	uint16_t adc[3] = {0};
	int8_t  Battery = 0;
	
	adc[0] = AdcReadParameter(ADC_CHANNEL_1, 10);
	float VBAT = VREFINT_CAL_VREF*(*VREFINT_CAL_ADDR)*adc[0]; 
	
	adc[1] = AdcReadParameter(ADC_CHANNEL_VREFINT, 10);
	
	float temp = adc[1] * VFULL;
	
	DEBUG_APP(2, "BAT = %d adc17 = %d , adc0 = %d, VBAT = %.2fmV", *VREFINT_CAL_ADDR,  adc[1], adc[0], (VBAT/temp)*2000);  ///100:510
		
	Battery = (((VBAT/temp)*2000 - 3600)/6);
	
	if(Battery >= 100)
		Battery = 100;
	else if(Battery <= 5)
		Battery = 3;
	return Battery;
}

/*
*CheckRecharge：判断是否接入适配器/太阳能板
*返回值：			  充电状态
*/
uint16_t CheckRecharge(void)
{
	uint16_t adc[3] = {0};
	uint16_t Battery = 1000;
	
	adc[0] = AdcReadParameter(ADC_CHANNEL_0, 10);
	float Rechargeing = VREFINT_CAL_VREF*(*VREFINT_CAL_ADDR)*adc[0]; 
	
	adc[1] = AdcReadParameter(ADC_CHANNEL_VREFINT, 10);
	
	float temp = adc[1] * VFULL;
	
	DEBUG_APP(2, "BAT = %d adc17 = %d , adc0 = %d %.2f\r\n", *VREFINT_CAL_ADDR,  adc[1], adc[0],temp);  ///100:510
		
	Battery *= (float)((Rechargeing/temp)*222/22);
	
	DEBUG_APP(2,"VBAT = %.2fmV",(float)Battery);
		
	return Battery;
}

int8_t ReadBattery(void)
{
	uint8_t s1, s2, PG;
//	uint16_t RechargeData = 0;
	
	int8_t  Battery = CheckBattery(  );
	
	s1 = (uint8_t)HAL_GPIO_ReadPin(OUT_CH_CE_GPIO_Port,IN_CH_STAT1_Pin);
	s2 = (uint8_t)HAL_GPIO_ReadPin(OUT_CH_CE_GPIO_Port,IN_CH_STAT2_Pin);
	PG = (uint8_t)HAL_GPIO_ReadPin(OUT_CH_CE_GPIO_Port,IN_CH_PG_Pin);	

	switch( (s2<<1) | s1 )
	{
		case 0x01:
				DEBUG_APP(2,"11充电完成\r\n");
		break;
		case 0x02:
				DEBUG_APP(2,"11正在充电\r\n");
		break;
		default:
				DEBUG_APP(2,"11未充电\r\n");
					
		break;
	}
	
//	RechargeData = CheckRecharge(  );
//	if(RechargeData >= 4000)
//	{
//		User.BatState = 0x04;
//		DEBUG_APP(2,"有效充电电压\r\n");
//	}
	
	/*************电量<80%手动开启充电**************/
	if(Battery <= 85)
	{
		BatDisableCharge(  );
		HAL_Delay(1000);
		BatEnableCharge(  );
	}

	return Battery;
}

