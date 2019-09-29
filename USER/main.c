/*
**************************************************************************************************************
*	@file	main.c
*	@author Jason_531@163.com
*	@version V1.1
*	@date    2017/12/13
*	@brief	NBI_LoRaWAN功能代码: add OTAA
***************************************************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <math.h>
#include "stm32l0xx_hal.h"
#include "usart.h"
#include "rtc-board.h"
#include "timerserver.h"
#include "delay.h"
#include "board.h"
#include "user-app.h"
#include "etimer.h"
#include "autostart.h"


#ifndef SUCCESS
#define SUCCESS                         1
#endif

#ifndef FAIL
#define FAIL                            0
#endif


/*!***********************************空中激活************************************/

#if  OVER_THE_AIR_ACTIVATION

extern uint8_t DevEui[8];
static uint8_t AppEui[] = LORAWAN_APPLICATION_EUI;
static uint8_t AppKey[] = LORAWAN_APPLICATION_KEY;

extern TimerEvent_t JoinReqTimer;
extern volatile bool IsNetworkJoined;
extern bool JoinReq_flag;

#endif

LoRaMacRxInfo *loramac_rx_info;
mac_evt_t loramac_evt;

void app_mac_cb (mac_evt_t evt, void *msg)
{
    switch(evt){
    case MAC_STA_TXDONE:                
    case MAC_STA_RXDONE:
    case MAC_STA_RXTIMEOUT:
    case MAC_STA_ACK_RECEIVED:
    case MAC_STA_ACK_UNRECEIVED:
    case MAC_STA_CMD_JOINACCEPT:         
    case MAC_STA_CMD_RECEIVED:
         loramac_rx_info = msg;   ///mac层接收数据信息：rssi 端口等
         loramac_evt = evt;
         
         break;
    }
}


/*!***********************************分割线************************************/

extern UART_HandleTypeDef 			   UartHandle;
extern RTC_HandleTypeDef 				RtcHandle;
extern SPI_HandleTypeDef            SPI1_Handler;  


PROCESS(SX1278Send_process,"SX1278Send_process");
PROCESS(GPS_process,"GPS_process");
AUTOSTART_PROCESSES(&etimer_process,&SX1278Send_process);

void RFTXDONE(void)
{
	process_poll(&SX1278Send_process);
}

extern uint32_t UpLinkCounter;

PROCESS_THREAD(GPS_process,ev,data)
{
	static struct 	etimer et;
	
	PROCESS_BEGIN();
	
	while(1)
	{
	
	}
	
	PROCESS_END();
}

PROCESS_THREAD(SX1278Send_process,ev,data)
{		
	static struct 	etimer et;
	static uint32_t WorkTime = 0;
	static int32_t  Csmatime = 0;
	static uint8_t  SedId = 0;
	static uint32_t symbolTime = 0;
	
	PROCESS_BEGIN();
	    
	etimer_set(&et,CLOCK_SECOND*1);	
	
	while(1)
	{		
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
       			
		WorkTime = HAL_GetTick( );
		
		User.SleepTime = 60;
		
		Csmatime = randr(-9, 9)*TIMEONAIR;
		
#if 1
		UserSendSensor(  );
			
		for(SedId = 0; SedId <= SendBufsCounter - 1; SedId++)
		{				
			Sx127xHandle.Buf[0] = (VERSIOS << 4); ///|充电状态
			
			DEBUG_APP(2,"SendBufsCounter: %d",SendBufsCounter);	

			memcpy1(&Sx127xHandle.Buf[1], SendBufs[SedId].Buf, SendBufs[SedId].Len); ///payload
			
			Sx127xHandle.Buf[0] |= User.BatState;
					
			Sx127xHandle.Buf[1] += 1; 
			
			Sx127xHandle.Buf[1] |= (SendBufsCounter<<4);  ///总包数|当前第几包
			
			Sx127xHandle.Len = 1 + SendBufs[SedId].Len; /// +sensor_len	
			
			DEBUG(2,"Sx127xSendBuf: ");
			for(uint8_t i = 0; i < Sx127xHandle.Len; i++)
			DEBUG(2,"%02X ",Sx127xHandle.Buf[i]);
			DEBUG(2,"\r\n");		
			
			CsmaTimerEvent = false;
			
			LoRaCsma.Cad_Done = false;
		
			//开启接收模式
			Radio.Standby( );
			LoRaMacSetDeviceClass( CLASS_C );
			
			LoRaCsma.Disturb = true;
			
			TimerStop( &CsmaTimer );
			TimerSetValue( &CsmaTimer, 10*TIMEONAIR + Csmatime); 
			TimerStart( &CsmaTimer );

			DEBUG_APP(2,"TimeOnAir = %d", 10*TIMEONAIR + Csmatime);					
					
			symbolTime = SymbolTime( );

			///sleep
//			HAL_PWR_EnterSLEEPMode( PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFE );
			while(!CsmaTimerEvent)
			{
				LoRaCadInit();
				
				delay_us( 240 ); 

				delay_us( symbolTime + 240 ); 
				
				while( !LoRaCsma.Cad_Done );
				
				if(LoRaCsma.Cad_Detect)
				{
					TimerStop( &CsmaTimer );									
					DEBUG(2,"LoRaCsma.Disturb is true\r\n");
					CsmaTimerEvent = false;
				 
					int32_t Csmatime = randr(-9, 9)* TIMEONAIR;
				 
					TimerSetValue( &CsmaTimer, 10 * TIMEONAIR + Csmatime); //+ randr(-1.5*TimeOnAir, 0)
					TimerStart( &CsmaTimer );
					
					LoRaCsma.Cad_Detect = false;
				}
				
				LoRaCsma.Cad_Done = false;
			}		

#endif		
		
			Sx127xHandle.PackageCounter = 0;
			Radio.Standby( );
			LoRaMacSetDeviceClass( CLASS_A );
	
			do
			{
				if(UserAppSend(UNCONFIRMED, Sx127xHandle.Buf, Sx127xHandle.Len, 2) == 0) //MAC+PHY=56  MAC = 13  SAMPLE_SIZE
				{
					DEBUG_APP(2,"Wait ACK app_send UpLinkCounter ***** %d *****", LoRaMacGetUpLinkCounter( ));
											
					User.WakeUp = false;
					PROCESS_YIELD_UNTIL(User.LoramacEvtFlag == 1);
					
					LoRaCsma.Disturb = false; ///解除侦听状态
					CsmaTimerEvent = false;
					
					/********************缓存清除*******************/
					memset(&Sx127xHandle.Buf[0], 0, Sx127xHandle.Len);
					SendBufs[SedId].Len = 0; ///payload清零					
					
					///休眠前校准RTC时钟
//					RtcvRtcCalibrate(  );
					
					User.WorkTime = HAL_GetTick( ) - WorkTime;
					
					User.WorkTime /= 1000; 
					
					DEBUG_APP(2,"sleep_times = %d Work_Time = %d", User.SleepTime, User.WorkTime);
					
					User.WorkTime = 0;
					User.LoramacEvtFlag = 0;
		
					User.WakeUp = true;
							
					DEBUG_APP(2,);
					
					break;
				}
				else
				{
					DEBUG_APP(2,"app_send again");
					Radio.Sleep( );
					etimer_set(&et,CLOCK_SECOND*4 + randr(-CLOCK_SECOND*4,CLOCK_SECOND*4));
					PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
				}
		}while(!User.LoramacEvtFlag);
			
			User.LoramacEvtFlag = 0;				
		}
		
		DEBUG_APP(2,);
				
		SetRtcAlarm(User.SleepTime > User.WorkTime?\
							 (User.SleepTime - User.WorkTime):30);  ///设置闹钟时间 ReadFlash.sleep_times
		
		IntoLowPower(  );
					
		WakeupInitMcu( );
		
		etimer_set(&et,CLOCK_SECOND*1);	
		
	}
	PROCESS_END();
}

/*******************************************************************************
  * @函数名称		main
  * @函数说明   主函数 
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无

	版本说明：
	【1】：V2.1.1：MCU---stm32L0，数据采集设备 PRO II适应小网关;

	优化功能：
	【1】： 实现LORAWAN与小网关通信。
	【2】： RTC停机唤醒机制。
	【3】： 双频段FreqTX = FreqRX2  FreqRX1 = 高频段
	【4】： FreqTX预留一频段作为FreqRX1偏移使用
	【5】： 增加GPS一个数据状态：0x40：定位中(GPS定位过程上报传感器数据，定位完成后再上报一次GPS数据，然后休眠)
	【6】： ACK重发机制，采用LoRaWan自带补充
	【7】： 上行数据帧采用16位，32位需要服务器数据库处理才能使用
  *****************************************************************************/
/* variable functions ---------------------------------------------------------*/	

int main(void)
{	
	BoardInitMcu(  );
	
	/******************空速初始化*****************/
	ReadFlashData(  );

	PowerEnableLed(  ); 	
	DEBUG_APP(2,"TIME : %s  DATE : %s\r\n",__TIME__, __DATE__);

	UserAppInit(app_mac_cb);
	LoRaMacTestRxWindowsOn( true ); ///开启接收窗口
    
	LoRaMacChannelAddFun( );
		
	User.Sx127xTxChannel = User.Sx127xRx2Channel = 0; ///获取信道TX ID 
	
	User.Sx127xRx1Channel = 12; ///获取信道RX1 ID 
	
	LoRaCsma.Iq_Invert = true; 

	User.LoramacEvtFlag = 0;

	Sx127xHandle.Fport = randr( 1, 0xDF );
	
	UserCheckSensors(  );
	
	TimerInit( &CsmaTimer, OnCsmaTimerEvent );
	
	uint8_t TxData = 0X42;
   uint8_t pRxData = SX1276Read( TxData );

	DEBUG_APP(2, "SX1276 ID = 0x%x",pRxData);  ///读取到0x12则正确，否则错误	
	
	Radio.Standby( );
	LoRaMacSetDeviceClass( CLASS_A );
	
	Radio.Sleep();
	
	User.WakeUp = false;	

	process_init(  );
	
   autostart_start(autostart_processes);///自动包含下面的线程
	
	while (1)
	{		
		do
		{
		}while(process_run() > 0); 
	}
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{ 
	DEBUG_APP(2,"error\r\n");
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/*--------------------------------------------------------------------------------------------------------
                   									      0ooo											
                   							 ooo0    (   )
                								(   )     ) /
                								 \ (     (_/
                								  \_)
----------------------------------------------------------------------------------------------------------*/

