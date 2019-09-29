/*
**************************************************************************************************************
*	@file	sx1276-board.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	
***************************************************************************************************************
*/
#include "board.h"

/*!
 * Flag used to set the RF switch control pins in low power mode when the radio is not active.
 */
static bool RadioIsActive = false;

/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio =
{
	SX1276Init,
	SX1276ResetInit,
	SX1276GetStatus,
	SX1276SetModem,
	SX1276SetChannel,
	SX1276IsChannelFree,
	SX1276Random,
	SX1276SetRxConfig,
	SX1276SetTxConfig,
	SX1276CheckRfFrequency,
	SX1276GetTimeOnAir,
	SX1276Send,
	SX1276SetSleep,
	SX1276SetStby, 
	SX1276SetRx,
	SX1276StartCad,
	SX1276ReadRssi,
	SX1276Write,
	SX1276Read,
	SX1276WriteBuffer,
	SX1276ReadBuffer,
	SX1276SetMaxPayloadLength
};


/**
  * @brief SX1276 I/O初始化
  * @param 中断初始化；电源、复位引脚初始化
  * @retval None
  */
void SX1276IoInit( void )
{
  SX1276EXTI_Init( );			///SX1276: DIO设置外部中断
	SX1276GPIO_Init( );		///SX1276: RESET复位引脚  
}

uint8_t SX1276GetPaSelect( uint32_t channel )
{
	if( channel < RF_MID_BAND_THRESH )
	{
		return RF_PACONFIG_PASELECT_PABOOST;
	}
	else
	{
		return RF_PACONFIG_PASELECT_RFO;
	}
}

/**
  * @brief  射频开关模式选择
  * @param  注意该模块不具有此功能，硬件上硬件直接拉高使用
  * @retval None
  */
void SX1276SetAntSwLowPower( bool status )
{
	if( RadioIsActive != status )
	{
		RadioIsActive = status;

		if( status == false )
		{
			SX1276AntSwInit( );
		}
		else
		{
			SX1276AntSwDeInit( );
		}
	}
}

void SX1276AntSwInit( void )
{
    /* RF switch save power mode */
	SX1276.RxTx = 2;
}

void SX1276AntSwDeInit( void )
{
    /* RF switch save power mode */
	SX1276.RxTx = 2;
}

void SX1276SetAntSw( uint8_t rxTx )
{
	if( SX1276.RxTx == rxTx )
	{
		return;
	}

	SX1276.RxTx = rxTx;
}

bool SX1276CheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supportted
	return true;
}
