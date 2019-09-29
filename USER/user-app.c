/*
**************************************************************************************************************
*	@file	user-app.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	协议采用分层方式：mac app进行区分
***************************************************************************************************************
*/
#include <math.h>
#include "user-app.h"
#include "board.h"
#include "LoRaMac.h"
#include "LoRa-cad.h"
#include "LoRaMac-api-v3.h"


#define APP_DATA_SIZE                                   (43)
#define APP_TX_DUTYCYCLE                                (100000)     // 5min
#define APP_TX_DUTYCYCLE_RND                            (100)   // ms
#define APP_START_DUTYCYCLE                             (10000)     // 3S


/*!
 * Join requests trials duty cycle.
 */
#define OVER_THE_AIR_ACTIVATION_DUTYCYCLE           		10000 // 10 [s] value in ms

//mac_callback_t mac_callback_g;

volatile bool IsNetworkJoined = false;

bool JoinReq_flag = true;

/*!
 * Defines the join request timer
 */
TimerEvent_t JoinReqTimer;

uint8_t DevEui[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t DEV[4] = {
   0 
};

static uint8_t NwkSKey[] = {
    0x2B, 0x7E, 0x99, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
    0xAB, 0xF7, 0x99, 0x88, 0x09, 0xCF, 0xF4, 0x1A
};
static uint8_t AppSKey[] = {
    0x2B, 0x7E, 0x99, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
    0xAB, 0xF7, 0x99, 0x88, 0x09, 0xCF, 0xF4, 0x1A
};

static uint32_t DevAddr;
static LoRaMacCallbacks_t LoRaMacCallbacks;
LoRaMacRxInfo 						RxInfo;

/************************设置RF数据包参数************************/
Sx127x_Handle_t Sx127xHandle = {0, 0, 0, 5, 0, 0, 0, false, false}; 

User_t 					User = {false, false, false, false, false, 0, 11, 0, 0, 0, 300, 0};


/****************************OTAA参数******************************
*********【1】read flash or get buy AT commed.
*********/

uint8_t AppEui[16] = {0};  
uint8_t AppKey[32] = {0};

/*!
 * \brief Function executed on JoinReq Timeout event
 */
void OnJoinReqTimerEvent( void )
{
	TimerStop( &JoinReqTimer );
	JoinReq_flag = true;
	DEBUG(2,"OnJoinReqTimerEvent \r\n");
}

LoRaFrameType_t LoRaFrameType;

/*!
 * \brief Function to be executed on MAC layer event
 */
void OnMacEvent( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info )///MAC层发送、接收状态判断、数据处理
{
	switch( info->Status )
	{
		case LORAMAC_EVENT_INFO_STATUS_OK:
				break;
		case LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT:;
				break;
		case LORAMAC_EVENT_INFO_STATUS_ERROR:
				break;
		default:
				break;
	}

	if( flags->Bits.JoinAccept == 1 )
	{       
		DEBUG(2,"join done\r\n");
		TimerStop( &JoinReqTimer );
		IsNetworkJoined = true;
		RxLed( );
		mac_callback_g(MAC_STA_CMD_JOINACCEPT, NULL);
	}  
  
	if( info->TxAckReceived == true )
	{ 
		if(mac_callback_g!=NULL)
		{
			mac_callback_g(MAC_STA_ACK_RECEIVED, &RxInfo); ///相当于调用app_lm_cb函数
			User.Ack = true;    
			DEBUG(2,"ACK Received\r\n");
		}
	}
	else if((flags->Bits.Rx != 1) && (flags->Bits.JoinAccept != 1) && (LoRaFrameType == CONFIRMED))
	{
		DEBUG(2,"=====NO ACK REPLY=====\r\n");     
	}
	
	if( flags->Bits.Rx == 1 )
  {  
	  ///接收到数据，数据信息保存RxData: 网关应答数据默认不打印接收信息
		RxInfo.size = info->RxBufferSize;
		memcpy(RxInfo.buf, info->RxBuffer, RxInfo.size);
		RxInfo.rssi = info->RxRssi;
		RxInfo.snr = info->RxSnr;
		RxInfo.win = flags->Bits.RxSlot+1;
		RxInfo.port = info->RxPort;
		RxLed( );
		DEBUG(2,"win = %d snr = %d rssi = %d size = %d \r\n",RxInfo.win, RxInfo.snr, RxInfo.rssi, RxInfo.size);
		if(flags->Bits.RxData == 1)
		{
			if(mac_callback_g!=NULL)
			{
				mac_callback_g(MAC_STA_RXDONE, &RxInfo);
				if( RxInfo.size > 0 )
				{
					DEBUG(2,"RxInfo.buf = ");
					for( uint8_t i = 0; i < RxInfo.size; i++ )
					DEBUG(2,"%02x ",RxInfo.buf[i]);
					DEBUG(2,"\r\n");									
				}
		 
				if(RxInfo.buf[0] == 0xA0) ///下行更改休眠时间
				{				
					User.SleepTime  = RxInfo.buf[1] << 8;
					User.SleepTime |= RxInfo.buf[2] << 0;
					
					FlashWrite32(SLEEP_ADDR, &User.SleepTime, 1);	
					DEBUG(2,"sleep_times :%d\r\n", User.SleepTime);
				}
				else if(RxInfo.buf[0] == 0xA1) ///重新获取GPS定位信息
				{
					DEBUG(2,"RxInfo.buf[0] = 'G'");
				}	
			}            
			memset(RxInfo.buf, 0, strlen((char *)RxInfo.buf));
		}     
	}
	 
	if( flags->Bits.Tx == 1 )
	{
		if(mac_callback_g!=NULL)
		{
			mac_callback_g(MAC_STA_TXDONE, NULL);
			User.LoramacEvtFlag = 1;

			DEBUG(2,"Done\r\n");
			SendDoneLed( );
		}
	}
}

void UserAppInit(mac_callback_t mac)
{
	LoRaMacCallbacks.MacEvent = OnMacEvent; ///MAC层数据接口
	LoRaMacCallbacks.GetBatteryLevel = NULL;
	LoRaMacInit( &LoRaMacCallbacks );

	IsNetworkJoined = false;
	
#if( OVER_THE_AIR_ACTIVATION == 0 )  
    
	DevAddr  = DEV[3];
	DevAddr |= (DEV[2] << 8);
	DevAddr |= (DEV[1] << 16);
	DevAddr |= (DEV[0] << 24);
	DEBUG(2,"DevAddr : %02x-%02x-%02x-%02x\r\n",DEV[0],DEV[1],DEV[2],DEV[3]);
	
	LoRaMacInitNwkIds( 0x000000, DevAddr, NwkSKey, AppSKey );
	IsNetworkJoined = true;

#else
     // Initialize LoRaMac device unique ID : 空中激活时作为激活参数
	BoardGetUniqueId( DevEui );
	for(uint8_t i = 0; i < 8; i++)
	DEBUG(2,"%02x ", DevEui[i]);
	DEBUG(2,"\r\n");

	
	 // Sends a JoinReq Command every OVER_THE_AIR_ACTIVATION_DUTYCYCLE
	// seconds until the network is joined
	TimerInit( &JoinReqTimer, OnJoinReqTimerEvent );
	TimerSetValue( &JoinReqTimer, OVER_THE_AIR_ACTIVATION_DUTYCYCLE );
	
#endif
  
	LoRaMacSetAdrOn( User.AdrOpen );
	LoRaMacTestSetDutyCycleOn(false);
	
	mac_callback_g = mac;

}

char String_Buffer[33]; ///读取flash写入字符串

int PowerXY(int x, int y)
{
	if(y == 0)
	return 1 ;
	else
	return x * PowerXY(x, y -1 ) ;
}
/*!
*Convert16To10：16进制转化为10进制
*返回值: 		     		  10进制数值
*/
int Convert16To10(int number)
{
	int r = 0 ;
	int i = 0 ;
	int result = 0 ;
	while(number)
	{
		r = number % 16 ;
		result += r * PowerXY(16, i++) ;
		number /= 16 ;
	}
	return result ;
}

/*!
*Read_DecNumber：字符串中的数字转化为10进制
*返回值: 		  10进制数值
*/
uint32_t ReadDecNumber(char *str)
{
	uint32_t value;

	if (! str)
	{
		return 0;
	}
	value = 0;
	while ((*str >= '0') && (*str <= '9'))
	{
		value = value*10 + (*str - '0');
		str++;
	}
	return value;
}

/*!
*String_Conversion：字符串转换为16进制
*返回值: 		     无
*/
void StringConversion(char *str, uint8_t *src, uint8_t len)
{
	volatile int i,v;
			
	for(i=0; i<len/2; i++)
	{
		sscanf(str+i*2,"%2X",&v);
		src[i]=(uint8_t)v;
		DEBUG(2,"%02X ",src[i]);
	}
}

/*!
*ReadFlashData：读取ABP入网参数
*返回值: 		      无
*/
void ReadFlashData(void)
{	
	Sx127xHandle.UpdateAdr = Sx127xHandle.DefaultDatarate = 1;
	
	uint8_t Temp[2] = {0};

	if( OVER_THE_AIR_ACTIVATION == 0 ) 
	{ 
		uint8_t Devaddr[16] = {0};

		FlashRead16More(DEV_ADDR, (uint16_t*)String_Buffer, DEV_ADDR_SIZE/2);         ////DEV

		StringConversion(String_Buffer, Devaddr, DEV_ADDR_SIZE); 
        
		memset(String_Buffer, 33, 0);	
		
		DEV[0] = Devaddr[1];
		DEV[1] = Devaddr[3];
		memcpy(&DEV[2],&Devaddr[6],2);

		DEBUG(2,"Devaddr : ");
		for(uint8_t i = 0; i < 8; i++)
		DEBUG(2,"%02X",Devaddr[i]);
		DEBUG(2,"\r\n");
	}   
	
	///read sleep time
	FlashRead16More(SLEEP_ADDR, (uint16_t*)String_Buffer, SLEEP_ADDR_SIZE/2); 
	
	StringConversion(String_Buffer,Temp, SLEEP_ADDR_SIZE);

	User.SleepTime  = Temp[0] << 8;
	User.SleepTime |= Temp[1] << 0;
	
	DEBUG_APP(2,"User.SleepTime : %d", User.SleepTime);
	
	memset(String_Buffer, 33, 0);	
}


/*UserCheckSensors：	用户查询传感器信息
*参数：								无
*返回值：   					无
*/
void UserCheckSensors(void)
{			
	LedOn(  );
			
	Sensors.WaterSensor = false;
	
	Sensors.SceneSelection = 0x01;
	
	/****************获取传感器数据时间可能较长，先开启GPS定位节省时间*****************/
	Sensors.QueryPinStaus(  );
	
	if(Sensors.WaterSensor)
	{
		DEBUG_APP(2, "Sensors.WaterSensor Sleep time: 30min");
		User.SleepTime = 30 * 60;
	}
		
  LedOff(  );
}

/*UserSendSensor：用户调用Zeta发送传感器数据
*						
*参数：				无
*返回值：   		无
*/
void UserSendSensor(void)
{	
#if CHECKGETDATA
	
	if(!User.Sleep) ///上电第一次查询操作
	Sensors.CheckHandle(  );
	else   ///休眠后操作
	{
		LedOn(  );
		Sensors.Handle(  );
		LedOff(  );
	}

#else	
	
	LedOn(  );
	Sensors.Handle(  );
	LedOff(  );
	
#endif	
			
//	LedSendSucess(8);   ///每包数据间隔4S
}

int UserAppSend( LoRaFrameType_t frametype, uint8_t *buf, int size, int retry)
{
	int sendFrameStatus;

	if(size == 0 || buf == 0){
		return -3;
	}
    DEBUG(2,"%s\r\n",__func__);
	LoRaFrameType = frametype;
	if(frametype == UNCONFIRMED){
		sendFrameStatus = LoRaMacSendFrame( Sx127xHandle.Fport, buf, size );
	}else{
		if(retry <= 0){
			retry = 3;
		}
		sendFrameStatus = LoRaMacSendConfirmedFrame( Sx127xHandle.Fport, buf, size, retry );
	}

	switch( sendFrameStatus )
	{
		case 1: // LoRaMac is Busy
            DEBUG(2,"return : %d\r\n",sendFrameStatus);
		return -1;
		case 2:
		case 3: // LENGTH_PORT_ERROR
		case 4: // MAC_CMD_ERROR
		case 5: // NO_FREE_CHANNEL
		case 6:
            DEBUG(2,"return : %d\r\n",sendFrameStatus);
			return -2;
		default:
			break;
	}
	return 0;
}

uint32_t app_get_devaddr(void)
{
	return DevAddr;
}

/*
 *	AppLoramacJoinreq:	OTAA入网申请
 *	返回值: 		    		无
 */
void AppLoramacJoinreq(void)
{
	while( ( !IsNetworkJoined ) )
	{
		if( JoinReq_flag )
		{
			JoinReq_flag = false;

			DEBUG(2,"LoRaMacJoinReq \r\n");
			int  sendFrameStatus = LoRaMacJoinReq( DevEui, AppEui, AppKey );
			SendDoneLed( ); 
			DEBUG(2,"sendFrameStatus = %d\r\n",sendFrameStatus);
			switch( sendFrameStatus )
			{
				case 1: // BUSY
						break;
				case 0: // OK
				case 2: // NO_NETWORK_JOINED
				case 3: // LENGTH_PORT_ERROR
				case 4: // MAC_CMD_ERROR
				case 6: // DEVICE_OFF
				default:
				// Relaunch timer for next trial                
						TimerStart( &JoinReqTimer );                  
						break;
			}
		}
	}   
}

void SendDoneLed(void)
{
	for(uint8_t i = 0; i < 5; i++)
	{
		HAL_GPIO_WritePin(LORA_LED,LORA_LED_PIN,GPIO_PIN_SET);
		delay_ms(200);
		HAL_GPIO_WritePin(LORA_LED,LORA_LED_PIN,GPIO_PIN_RESET);;
		delay_ms(200);
	}
}

void RxLed(void)
{
	for(uint8_t i = 0; i < 5; i++)
	{
		GpioWrite(LORA_LED,LORA_LED_PIN,GPIO_PIN_SET);
		delay_ms(50);
		GpioWrite(LORA_LED,LORA_LED_PIN,GPIO_PIN_RESET);
		delay_ms(50);
	}
}

void ErrorLed(void)
{
	for(uint8_t i = 0; i < 3; i++)
	{
		GpioWrite(LORA_LED,LORA_LED_PIN,GPIO_PIN_SET);
		delay_ms(1000);
		GpioWrite(LORA_LED,LORA_LED_PIN,GPIO_PIN_RESET);
		delay_ms(200);
	}
}

void PowerEnableLed(void)
{
	GpioWrite(LORA_LED,LORA_LED_PIN,GPIO_PIN_SET);
}

void PowerDisbleLed(void)
{
	GpioWrite(LORA_LED,LORA_LED_PIN,GPIO_PIN_RESET);
}

/*******************************定义发送数据************************************/

uint8_t AppData[APP_DATA_SIZE];

/*
 *	IntoLowPower:	进入低功耗模式：停机
 *	返回值: 		无
 */
void IntoLowPower(void)
{	    
	///复位硬件看门狗，防止异常每4个小时复位一次
	LoRaPowerOff();
	HAL_Delay(1);
	LoRaPowerOn();
	
	BoardDeInitMcu(); ///关闭时钟线
	
	// Disable the Power Voltage Detector
	HAL_PWR_DisablePVD( );
	
	/* Set MCU in ULP (Ultra Low Power) */
	HAL_PWREx_EnableUltraLowPower( );
	
	/* Enable the fast wake up from Ultra low power mode */
	HAL_PWREx_EnableFastWakeUp();
	
	/*****************进入停机模式*********************/
	/* Enter Stop Mode */
	HAL_PWR_EnterSTOPMode( PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI );  
}


/*--------------------------------------------------------------------------
                                                     0ooo
                                          ooo0      (   )
                                          (   )      ) /
                                           \ (      (_/
                                            \_)
----------------------------------------------------------------------------*/

