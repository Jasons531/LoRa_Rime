/*
**************************************************************************************************************
*	@file	user-app.h
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	应用层头文件：连接MAC层
***************************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USER_APP_H
#define __USER_APP_H

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include <stdio.h>
#include <stdint.h>

#define VERSIOS								0x01


#define DEV_ADDR								0x0801ffe0			//设备ID存储地址
#define DEV_ADDR_SIZE						0x12				//设备ID占用存储空间

#define SLEEP_ADDR							0x0801ffc0			//休眠时间存储地址 
#define SLEEP_ADDR_SIZE						0x04				//休眠时间占用存储空间

/*!
 * When set to 1 the application uses the Over-the-Air activation procedure
 * When set to 0 the application uses the Personalization activation procedure
 */
#define OVER_THE_AIR_ACTIVATION         0

/*!
 * Application IEEE EUI (big endian)
 */
#define LORAWAN_APPLICATION_EUI         { 0x12,0x34,0x56,0x78,0x90,0xAB,0xCD,0xEF }


/*!
 * AES encryption/decryption cipher application key
 */
#define LORAWAN_APPLICATION_KEY         { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C }

typedef enum{
	IDLE,
	BUSY,
	DONE,
}LoRaStatus_t;

typedef enum{
	CONFIRMED,
	UNCONFIRMED
}LoRaFrameType_t;

typedef enum{
	INVALID,
	RECEIVED,
	UNRECEIVED,
}LoRaTxAckReceived_t;

typedef enum{
	LORAMAC_NWKSKEY=0,
	LORAMAC_APPSKEY=1,
	LORAMAC_APPKEY=2,
}LoRaMacKey_t;

typedef struct{
	int16_t 		rssi;
	uint8_t 		snr;
	uint8_t 		win;
	uint8_t 		port;
	uint16_t 	size;
	uint8_t 		buf[256];
}LoRaMacRxInfo;

typedef enum{
	MAC_STA_TXDONE,
	MAC_STA_RXDONE,
	MAC_STA_RXTIMEOUT,
	MAC_STA_ACK_RECEIVED,
	MAC_STA_ACK_UNRECEIVED,
	MAC_STA_CMD_JOINACCEPT,
	MAC_STA_CMD_RECEIVED,
}mac_evt_t;

typedef struct f_User
{
	bool 			Ack; 
	bool 			Sleep;
	bool 			TestMode;
	bool 			WakeUp;
	bool 			AdrOpen;
	
	uint8_t		Sx127xTxChannel;
	uint8_t		Sx127xRx1Channel;
	uint8_t		Sx127xRx2Channel;
	uint8_t 		BatState;
	uint8_t     LoramacEvtFlag;
	uint32_t 	SleepTime;
	uint32_t 	WorkTime;
}User_t;

/*
*RF发送包配置
*/
typedef struct{
	uint16_t 	Len;	///发送数据长度
	uint8_t  	Fport; ///FPort
	uint8_t 		Buf[64];
	uint8_t		UpdateAdr;   ///获取到ADR时SF更改
	uint8_t     DefaultDatarate;  ///默认datarate
	uint8_t     Battery;  ///电量
  uint8_t      PackageCounter;
	bool			SendAgain;	///RF ACK Fail Send Again 
  bool         OnRxWindow1;
}Sx127x_Handle_t;

extern 		User_t User;

extern 		Sx127x_Handle_t Sx127xHandle;

//typedef 		void (*mac_callback_t) (mac_evt_t mac_evt, void *msg);

int 			PowerXY(int x, int y);

int 			Convert16To10(int number);

uint32_t 	ReadDecNumber(char *str);

void 			StringConversion(char *str, uint8_t *src, uint8_t len);

void 			StringTurnBase(char *str, uint8_t *src, uint8_t len);

void		 	ReadFlashData(void);

//void 			UserAppInit(mac_callback_t mac);

void 			UserCheckSensors(void);

void 			UserSendSensor(void);

int 			UserAppSend( LoRaFrameType_t frametype, uint8_t *buf, int size, int retry);

uint32_t 	app_get_devaddr(void);

void 			OnReportTimerEvent( void );

void 			SystemReset(void);

void 			GpsSendAgainTime(void);

void 			IntoLowPower(void);

void 			SendDoneLed(void);

void 			RxLed(void);

void 			ErrorLed(void);

void 			PowerEnableLed(void);

void 			PowerDisbleLed(void);

void 			RFTXDONE(void);


#endif /* __USER_APP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
