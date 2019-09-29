/*
**************************************************************************************************************
*	@file			sensor.c
*	@author 	Jason
*	@version 	V0.1
*	@date     2018/07/13
*	@brief		传感器采集
***************************************************************************************************************
*/
#include "sensor.h"
#include "user-app.h"

#include "stm32l0xx_hal.h"
#include "rs485.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	

CheckRs485_t CheckRs485s[] = {
///Addr  Identifier	RegAddress  RegDatalen  SendDataLen	RevDataLen	SensorToLen  		   TimeOut  SendBuff		revBuff			name
	{0x00, 0x00,				0x0000,     0x0000,         7,          9,    RS485_IDE_LEN+4,    200*1, "EXPEND",  	"",    		"EXPEND"},	///扩展盒
	{0x02, 0x02,				0x0000, 		0x0002,  				6,					9, 		RS485_IDE_LEN+4,		500*1,		"" ,			"" ,		"SWR-100W"},  ///土壤温湿度
	{0x05, 0x05,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		1000*1,	  "" ,			"" ,			 "ST_PH"},  ///土壤、水PH  1000*30
	{0x06, 0x06,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		1000*1,		"" ,			"" ,		   "ST_GH"},	///光合有效
	{0x0C, 0x03,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		500*1,		"" ,			"" ,		   "ST-TW"},  ///土壤温度
	{0x0D, 0x0D,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		1000*1,		"" ,			"" ,		   "ST-EC"},  ///EC
	{0x0E, 0x0E,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		1000*10,	"" ,			"" ,		  "ST_CO2"},	///CO2		
//	{0x0F, 0x24,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		1000*1,		"" ,			"" ,		"ST_AP"		},///气象站：大气压				
	{0x12, 0x12,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		200*1,		"" ,			"" ,		   "andan"},  ///氨氮
	{0x13, 0x13,				0x0000, 		0x0002,  				6,					9, 		RS485_IDE_LEN+4,		200*1,		"" ,			"" ,		"Water-EC"},  ///水EC
	{0x14, 0x14,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		200*1,		"" ,			"" ,		 "Water-T"},  ///水温
	{0x15, 0x15,				0x0000, 		0x0002,  				6,					7, 		RS485_IDE_LEN+2,		200*1,		"" ,			"" ,		"Water-K+"},	///水钾+
	
	{0xF8, 0x29,				0x0000, 		0x0002,  				6,					9, 		RS485_IDE_LEN+4,		1000*1,		"" ,			"" ,		"Soil-EC"},    ///土壤EC 
	{0xF9, 0x2A,				0x0000, 		0x0002,  				6,					9, 		RS485_IDE_LEN+4,		1000*1,		"" ,			"" ,		"Soil-Tech"},  ///土壤温湿度
	
	{0xFD, 0x18,				0x0000, 		0x0004,  				6,				 13, 		RS485_IDE_LEN+8,		1000*1,	  "" ,		  "" ,		 "Air-Ill"},	///空气温湿度、光照

	/****************************   以下传感器不支持广播命令   ***************************/
	{0x07, 0x07,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		1000*1,		"" ,			"" ,		 "ST_Y/MW"},  ///叶面温度
		    
			/**************************   叶面湿度特殊功能码：0x04   *****************/
	{0x08, 0x08,				0x0001, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		1000*1,		"" ,			"" ,		  "ST_YMS"},  ///叶面湿度:功能码：0x04
	{0x17, 0x17,				0x0015, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		1000*4,		"" ,		  "" ,		   "WH_EC"},  ///威海土壤EC	
       
			/**************************   以下传感器：水产   *****************/
	{0x05, 0x05,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		1000*60,	"" ,			"" ,			 "WT_PH"},  ///水PH  1000*30
	{0x11, 0x11,				0x0000, 		0x0002,  				6,					9, 		RS485_IDE_LEN+4,		1000*120,	"" ,			"" ,		     "OXY"},  ///水溶氧 1000*120
	{0x18, 0x11,				0x0000, 		0x0004,  				6,					13, 	RS485_IDE_LEN+4,		1000*30,	"" ,		  "" ,		     "RDO"},  ///荧光DO 180 +8
};

SaveRs485_t  SaveRs485s[3];

SendBuf_t		 SendBufs[10] = {{{0}, {0}, 0}};

Sensor_t     Sensors;
	

volatile 	uint8_t SendBufsCounter = 0; ///记录数据缓存区数组个数

uint8_t OpenExpendBoxbuff[10] = {0x00,0x05,0x00,0x01,0x00,0x00,0x00};
uint8_t ExpendBoxbuff[9] = {0xFE,0x03,0x04,0x00,0x00,0x00,0x00,0x00,0x00};


/*
 *	SensorsInit:		传感器初始化
 *	参数：			  	无
 *	返回值：				无
 */
void SensorsInit(void)
{
	Sensors.Handle		 			= SensorHandle;
	Sensors.CheckHandle			= SensorCheckHandle;
	Sensors.DataProces			=	SensorDataProces;
	Sensors.QueryPinStaus 		= SensorQueryPinStaus;
	Sensors.QueryType 			= SensorQueryType;
	Sensors.GetData 				= SensorGetData;
	Sensors.GetRs485Type 		= GetRs485Type;
	Sensors.ExpendBoxLive 		= SensorExpendBoxLive;
	Sensors.ExpenSigle      	= SensorExpenSigle;
	Sensors.MaBoxData 			= SensorMaBoxData;
	Sensors.ExpBoxAddr 			= SensorExpBoxAddr;
	Sensors.ExpenData 			= SensorExpenData;

	Rs485Init(  );
	Rs485s.PowerOn(  );
}

/*
 *	GetRs485Type:		485接口类型
 *	参数：			  	主板Rs485接口ID
 *	返回值：				Rs485类型：RS485_NONE/RS485_EXPAND_BOX/RS485_SIGNAL
 */
static Rstype_t GetRs485Type(int index)
{	
	return SaveRs485s[index].Type;
}

/*
 *	SensorGetData:		获取Rs185传感器数据
 *	参数：			  		Rs485主板接口
 *	返回值：					获取成功/失败
 */
static HAL_StatusTypeDef SensorGetData(int id)
{	
	HAL_StatusTypeDef status;
	
	if(Sensors.GetRs485Type(id) == RS485_SIGNAL)
	{
		status = Sensors.MaBoxData( id );
	}
	else if(Sensors.GetRs485Type(id) == RS485_EXPAND_BOX)
	{
		status = Sensors.ExpenData( id );
	}
	
	return status;
}

/*
 *	SensorHandle:	  传感器处理
 *	参数：			  	无
 *	返回值：				无
 */
static void SensorHandle(void)
{			
	Rs485s.PowerOn(  );

	for(uint8_t id = 0; id < NBI_RS485_PIN_COUNT; id++)
	{
		if(Sensors.GetRs485Type( id ) != RS485_NONE)
		{
			id++;
			DEBUG_APP(2,"Start get sensor data main box id = %d",id);
			id--;

			Rs485s.OpenPin(id);
			HAL_Delay(1500);
			
			Sensors.GetData(id);			
		}
	}
	Rs485s.ClosePin(  );	
	
	Rs485s.PowerOff(  );
	
	SendBufsCounter = 0;
	
	Sensors.DataProces(  );
}

/*
 *	SensorCheckHandle:	  第一次上电传感器处理
 *	参数：			  				无
 *	返回值：							无
 */
static void SensorCheckHandle(void)
{
	SendBufsCounter = 0;
	
	Sensors.DataProces(  );
}

/*
 *	SensorDataProces:	传感器数据处理
 *	参数：			  		无
 *	返回值：					无
 */
static void SensorDataProces(void) 
{
	/**********运算长度**************/
	uint8_t Len = 0;
	
	/**********传感器类型BUF下标**************/
	uint8_t Index = 0;
	
	/***********扩展盒ID*************/
	uint8_t ExId = 0;
	
	/***********主板ID*************/
	uint8_t PortId = 0;
	
	/**********实际数据总长度**************/
	uint8_t Length = 0; 
	
	/***********传感器缓存区*************/
	uint8_t Temp[64] = {0};
	
	/***********传感器数据长度*************/
	uint8_t TempIndex = 0;
	
	/***********剩余待获取传感器个数*************/
	uint8_t GetSensorCounter = 0;

#if 1

	DEBUG_APP(2, "-----Start get data Counter : %d----",Sensors.Counter);
	for(PortId = 0; PortId < NBI_RS485_PIN_COUNT; PortId++)
	{
		if(SaveRs485s[PortId].Type == RS485_SIGNAL && GetSensorCounter < Sensors.Counter)
		{
			Len += SaveRs485s[PortId].MainBox.SensorToLen;
			
			if(Len <= MAXLEN) ///作为读取传感器数据是异常标志
			{
					DEBUG_APP(3, "-----SensorToLen : %d----",SaveRs485s[PortId].MainBox.SensorToLen);

				if(SaveRs485s[PortId].MainBox.SensorToLen != 0)
				{
					SendBufs[SendBufsCounter].Port[Index++] = SaveRs485s[PortId].MainBox.Index; //接口ID
					SendBufs[SendBufsCounter].Port[Index++] = SaveRs485s[PortId].MainBox.Identifier; //解码标识
					
					memcpy1(&Temp[TempIndex], &SaveRs485s[PortId].MainBox.SensorBuff[0], SaveRs485s[PortId].MainBox.SensorToLen-RS485_IDE_LEN);
					
					memset(SaveRs485s[PortId].MainBox.SensorBuff, 0, SaveRs485s[PortId].MainBox.SensorToLen-RS485_IDE_LEN);

					TempIndex += SaveRs485s[PortId].MainBox.SensorToLen-RS485_IDE_LEN ;
					Length += SaveRs485s[PortId].MainBox.SensorToLen;
									
					DEBUG_APP(3,"11SendBufsCounter = %d, Len = %d, PortId = %d,GetSensorCounter = %d, Sensors.Counter = %d",\
										SendBufsCounter,Len, PortId, GetSensorCounter, Sensors.Counter);
				}
				GetSensorCounter++;
			}
			else 
			{
				/********************分包第几帧*****************/
				SendBufs[SendBufsCounter].Buf[0] = SendBufsCounter;
				
				/********************指令*****************/
				SendBufs[SendBufsCounter].Buf[1] = 0x00;
				
				/********************电量*****************/
				SendBufs[SendBufsCounter].Buf[2] = ReadBattery(  );
				
				/********************传感器个数*****************/
				SendBufs[SendBufsCounter].Buf[3] = Index/2;

				uint8_t BufLen = 4;
				
				memset(&SendBufs[SendBufsCounter].Buf[BufLen], 0, 64);
				
				memcpy1(&SendBufs[SendBufsCounter].Buf[BufLen], SendBufs[SendBufsCounter].Port, Index);
				
				BufLen += Index;
				memcpy1(&SendBufs[SendBufsCounter].Buf[BufLen], Temp, TempIndex);
				
				Length += 4;
				
				SendBufs[SendBufsCounter].Len = Length;
								
				DEBUG_APP(3,"22SendBufsCounter = %d, Length = %d, PortId = %d,GetSensorCounter = %d, Sensors.Counter = %d",\
									SendBufsCounter,Length, PortId, GetSensorCounter, Sensors.Counter);
				for(uint8_t i = 0; i< Length; i++)
				DEBUG(3,"%02X ",SendBufs[SendBufsCounter].Buf[i]);
				DEBUG(3,"\r\n");
	
				memset(Temp, 0, TempIndex);
				memset(SendBufs[SendBufsCounter].Port, 0, Index);
				
				PortId --; //下标回退	
				SendBufsCounter ++; 
				Len = 0;
				Length = 0;
				TempIndex = 0;	
				Index = 0;
			}				
		}
		else if(SaveRs485s[PortId].Type == RS485_EXPAND_BOX && GetSensorCounter < Sensors.Counter)
		{
			for( ExId = 0 ; ExId < 5; ExId++)
			{
				if(SaveRs485s[PortId].MainBox.ExpendBox[ExId].ExpenCheck) ///接口存在
				{
					Len += SaveRs485s[PortId].MainBox.ExpendBox[ExId].SensorToLen;
			
					DEBUG_APP(3,"PortId = %d, ExId = %d Len = %d SensorToLen = %d",PortId,ExId,Len, \
					SaveRs485s[PortId].MainBox.ExpendBox[ExId].SensorToLen);
					if(Len <= MAXLEN)
					{
						if(SaveRs485s[PortId].MainBox.ExpendBox[ExId].SensorToLen != 0)
						{
							SendBufs[SendBufsCounter].Port[Index++] = SaveRs485s[PortId].MainBox.ExpendBox[ExId].Index; //接口ID
							SendBufs[SendBufsCounter].Port[Index++] = SaveRs485s[PortId].MainBox.ExpendBox[ExId].Identifier; //解码标识
							
							memcpy1(&Temp[TempIndex], &SaveRs485s[PortId].MainBox.ExpendBox[ExId].SensorBuff[0], \
											SaveRs485s[PortId].MainBox.ExpendBox[ExId].SensorToLen-RS485_IDE_LEN);
							
							memset(SaveRs485s[PortId].MainBox.ExpendBox[ExId].SensorBuff, 0, \
							SaveRs485s[PortId].MainBox.ExpendBox[ExId].SensorToLen-RS485_IDE_LEN);

							TempIndex += SaveRs485s[PortId].MainBox.ExpendBox[ExId].SensorToLen-RS485_IDE_LEN;
							Length += SaveRs485s[PortId].MainBox.ExpendBox[ExId].SensorToLen;
							
							DEBUG_APP(3,"33SendBufsCounter = %d, Length = %d, PortId = %d,GetSensorCounter = %d, Sensors.Counter = %d",\
												SendBufsCounter,Length, PortId, GetSensorCounter, Sensors.Counter);
						}
						GetSensorCounter++;

					}
					else
					{					
						
						/********************分包第几帧*****************/
						SendBufs[SendBufsCounter].Buf[0] = SendBufsCounter;
						
						/********************指令*****************/
						SendBufs[SendBufsCounter].Buf[1] = 0x00;
						
						/********************电量*****************/
						SendBufs[SendBufsCounter].Buf[2] = ReadBattery(  );
						
						/********************传感器个数*****************/
						SendBufs[SendBufsCounter].Buf[3] = Index/2;

						
						uint8_t BufLen = 4;
						
						memset(&SendBufs[SendBufsCounter].Buf[BufLen], 0, 64);
				
						memcpy1(&SendBufs[SendBufsCounter].Buf[BufLen], SendBufs[SendBufsCounter].Port, Index);
				
						BufLen += Index;
						memcpy1(&SendBufs[SendBufsCounter].Buf[BufLen], Temp, TempIndex);
						
						Length += 4;

						SendBufs[SendBufsCounter].Len = Length;
						
						DEBUG_APP(2,"44SendBufsCounter = %d, Length = %d, PortId = %d,GetSensorCounter = %d, Sensors.Counter = %d",\
											SendBufsCounter,Length, PortId, GetSensorCounter, Sensors.Counter);
						for(uint8_t i = 0; i<Length; i++)
						DEBUG(2,"%02X ",SendBufs[SendBufsCounter].Buf[i]);
						DEBUG(2,"\r\n");
				
						memset(Temp, 0, TempIndex);
						memset(SendBufs[SendBufsCounter].Port, 0, Index);
											
						ExId --;
						SendBufsCounter ++;
						Len = 0;
						Length = 0;
						TempIndex = 0;	
						Index = 0;
					}
				}
			}
		}					
	}
	
	if(PortId == NBI_RS485_PIN_COUNT && ExId == 5)
	{
		if(1 == GetSensorCounter - Sensors.Counter)
			GetSensorCounter --;
	}
		
	if(GetSensorCounter == Sensors.Counter)  ///<38B
	{			
		/********************分包第几帧*****************/
		SendBufs[SendBufsCounter].Buf[0] = SendBufsCounter;
		
		/********************指令*****************/
		SendBufs[SendBufsCounter].Buf[1] = 0x00;
		
		/********************电量*****************/
		SendBufs[SendBufsCounter].Buf[2] = ReadBattery(  );
		
		/********************传感器个数*****************/
		SendBufs[SendBufsCounter].Buf[3] = Index/2;

		uint8_t BufLen = 4;
		
		memset(&SendBufs[SendBufsCounter].Buf[BufLen], 0, 64);

		memcpy1(&SendBufs[SendBufsCounter].Buf[BufLen], SendBufs[SendBufsCounter].Port, Index); ///传感器标识
		DEBUG_APP(3, "444PortId : %d counter = %d\r\n",PortId, GetSensorCounter);
		
		BufLen += Index;
		
		memcpy1(&SendBufs[SendBufsCounter].Buf[BufLen], Temp, TempIndex); ///传感器数据
		
		Length += 4;

		SendBufs[SendBufsCounter].Len = Length;
		
		DEBUG(2,"55SendBufsCounter = %d, Length = %d,TempIndex = %d, Length = %d\r\n",SendBufsCounter,Length,TempIndex,Length);
		for(uint8_t i = 0; i< Length; i++)
		DEBUG(2,"%02X ",SendBufs[SendBufsCounter].Buf[i]);
		DEBUG(2,"\r\n");

		memset(Temp, 0, TempIndex);
		memset(SendBufs[SendBufsCounter].Port, 0, Index);
		
		SendBufsCounter ++;
		Len = 0;
		Length = 0;
		TempIndex = 0;	
		Index = 0;
	}
	
	Sensors.Counter = 0;
	GetSensorCounter = 0;
	
#endif	
}

/*
 *	SensorQueryPinStaus:		广播查询485、读取传感器数据
 *	参数：			  					无
 *	返回值：								查询状态：成功/失败/超时
 */
static HAL_StatusTypeDef SensorQueryPinStaus(void)
{	
	HAL_StatusTypeDef Status = HAL_TIMEOUT;
	
	DEBUG_APP(2,"Start get Rs485 Sensor data, It need some time, waiting......\r\n");
	
	RS485CmdPackage(NBI_RS485_SEARCH_CODE);///获取预存485命令缓存
		
	for(int id = 0; id < NBI_RS485_PIN_COUNT ; id++)
	{		
		//遍历plus的6个口，分别是什么
		//打开io口	
		Rs485s.OpenPin(id);
		HAL_Delay(1500);
		//判断是什么口，并且获取其中传感器的地址
		if(Sensors.QueryType(id) != RS485_NONE) 
		{			
			DEBUG_APP(3,"pin %02x: find device\r\n",id);
		}		
		else 
		{
			id++;
			DEBUG_WARNING(2,"pin %d:not find device\r\n",id);
			id--;
		}
	}
	Rs485s.ClosePin(  );				
	return Status;
}

/*
 *	SensorQueryType:		广播查询主板485类型
 *	参数：			  			Rs485接口
 *	返回值：						Rs485类型：RS485_NONE/RS485_EXPAND_BOX/RS485_SIGNAL
 */
static Rstype_t SensorQueryType(int PortId)
{
	uint8_t repbuff[20] = {0};	
	uint8_t expend_sensor = 0;
	
	uint8_t id = PortId;
	id++;

	SaveRs485s[PortId].Type = RS485_NONE;
	
	int len = Rs485s.Cmd(ExpendBoxbuff,7, 2, 1000);   // 地址广播：get expend return data 	
	
	memcpy1(repbuff,Rs485s.Revbuff,len);
	Rs485s.Print(repbuff, len, 2);
			
  ///判断广播回复地址：扩展盒地址应答
	
	if(len == CheckRs485s[EXPENDBOX].RevDataLen && repbuff[3] == 0) //扩展盒的地址 为0
	{
		//返回的数据长度大于0，认为是外部盒子
		expend_sensor = repbuff[4];  	//扩展盒哪些口接入传感器
				
		Sensors.ExpendBoxLive(expend_sensor,PortId); ///扩展盒相应接口状态标志
        
		SaveRs485s[PortId].Type = RS485_EXPAND_BOX;
				
		Sensors.ExpBoxAddr( PortId );		
	}
	else if(len >5) ///非扩展盒地址应答
	{				
		//没有找到扩展盒但是单个口有了回复
		DEBUG_APP(3,"siganal ok and has rpy addr = %x",repbuff[3]);		
		int i = 0;

		////判断接口对应接入485传感器地址，记录
		for(i = 0; i < ARRAY(CheckRs485s); i ++)
		{
			if(CheckRs485s[i].Addr == repbuff[3]) 
			{					
				SaveRs485s[PortId].Type = RS485_SIGNAL;
				
				SaveRs485s[PortId].MainBox.Index = id;  
				SaveRs485s[PortId].MainBox.CheckIndex = i; ///记录查询下标
				SaveRs485s[PortId].MainBox.Identifier = CheckRs485s[i].Identifier; ///解码标识
						
				DEBUG_APP(2,"CheckRs485s is ok index = %d",i);		
	
#if CHECKGETDATA		
				
				Sensors.MaBoxData( PortId );

#endif				
				break;
			}
		}
		if(i == ARRAY(CheckRs485s))
		{
				//没有找到
				SaveRs485s[PortId].Type = RS485_NONE;				
				DEBUG_ERROR(2,"address = %d not in array",repbuff[3]);		
		}
		else
		{
				SaveRs485s[PortId].Type = RS485_SIGNAL;
		}
	}
	else
	{
		//没有找到扩展盒，单个口也没有回复，开始遍历所有的外部设备的数据
		DEBUG_WARNING(3,"siganal ok and foreach");		
		uint32_t startTime = HAL_GetTick(  );
		
		uint32_t Rs485TimeOut = 0;
		uint8_t  CheckRs485Index = 0;
		uint8_t	 i = 0;
		
		/*******************工作场景模式选择：农场则不获取水产传感器数据*******************/
		if(0x01 == Sensors.SceneSelection)
		{
			i = 14;
			CheckRs485Index = ARRAY(CheckRs485s) - WATERSENSORNUMBER;
		}
		else
		{
			i = ARRAY(CheckRs485s) - WATERSENSORNUMBER;
			CheckRs485Index = ARRAY(CheckRs485s);
		}

		for(; i <  CheckRs485Index; i ++) ///只查询叶面传感器 
		{			
			if(i>=14)
			{
				Rs485TimeOut = 400; ///快速遍历水产传感器地址，当前数据无效不做处理
			}
			else
			{
				Rs485TimeOut = CheckRs485s[i].TimeOut;
			}
			len = Rs485s.Cmd(CheckRs485s[i].SendBuff, CheckRs485s[i].SendDataLen, NODEBUG, Rs485TimeOut);
			while(HAL_GetTick() - startTime < Rs485TimeOut && len != CheckRs485s[i].RevDataLen)
			{
				len = Rs485s.Cmd(CheckRs485s[i].SendBuff,CheckRs485s[i].SendDataLen, NODEBUG, Rs485TimeOut);				
				HAL_Delay(500);
			}
			if(len == CheckRs485s[i].RevDataLen)
			{
				//找到了
				DEBUG_APP(2,"main_box = %d, device had find:0x%02x",++PortId, CheckRs485s[i].Addr);
				--PortId;
				
				SaveRs485s[PortId].MainBox.Index = id;
				SaveRs485s[PortId].MainBox.CheckIndex = i; ///记录查询下标
				SaveRs485s[PortId].MainBox.Identifier = CheckRs485s[i].Identifier; ///解码标识

				SaveRs485s[PortId].Type = RS485_SIGNAL;				
				
				///查询数据保存处理
				SaveRs485s[PortId].MainBox.SensorToLen = CheckRs485s[i].SensorToLen; ///传感器总长度		
				
#if CHECKGETDATA				
				Sensors.Counter ++;
#endif
				
				DEBUG_APP(2,"CheckRs485s[i].Identifier = 0x%02x Sensors.Counter = %d",CheckRs485s[i].Identifier,Sensors.Counter);
				
				DEBUG_APP(2,"sensor Rs485 revData :");
				
				memset(SaveRs485s[PortId].MainBox.DataBuff,0,len);
				memcpy1(SaveRs485s[PortId].MainBox.DataBuff,Rs485s.Revbuff,len);
				
				Rs485s.Print(SaveRs485s[PortId].MainBox.DataBuff, len, 2);
				
				/*********************************当前接入水产传感器，自动更新休眠时间标识生效**************************************/
				if(SaveRs485s[PortId].MainBox.Identifier == 0x05 || SaveRs485s[PortId].MainBox.Identifier == 0x11)
				{
					Sensors.WaterSensor = true;
				}
#if CHECKGETDATA				
				//get data
				if(SaveRs485s[PortId].MainBox.Identifier != 0x11)
				{
					for(int bufid = 0, j = 3 ; j < len-2 ; ++j, ++bufid)
					{
						SaveRs485s[PortId].MainBox.SensorBuff[bufid] = SaveRs485s[PortId].MainBox.DataBuff[j];						
						DEBUG_APP(3,"get data[%d] = 0x%02x data0x%02x\r\n",j,SaveRs485s[PortId].MainBox.SensorBuff[bufid],SaveRs485s[PortId].MainBox.DataBuff[j]);						
					}
				}
				else
				{
					uint16_t databuf[2] = {0};	
										
					if(SaveRs485s[PortId].MainBox.SensorToLen == RS485_IDE_LEN+4)  //DO数据处理
					{
						for( uint8_t k = 0, j = 3; j < len-2; )
						{
							databuf[k] |= (SaveRs485s[PortId].MainBox.DataBuff[j++]<<8);
							databuf[k++] |= (SaveRs485s[PortId].MainBox.DataBuff[j++]<<0);
						}	
					}
					else  ///RDO数据处理
					{
						for( uint8_t k = 0, j = 3; j < len-4; )
						{
							databuf[k] |= (SaveRs485s[PortId].MainBox.DataBuff[j++]<<8);
							databuf[k++] |= (SaveRs485s[PortId].MainBox.DataBuff[j++]<<0);
							j += 2;
						}	
						SaveRs485s[PortId].MainBox.SensorToLen -= 4; ///过滤掉4个字节数据类型表示
					}		
					
					databuf[1] *= 10;
					
					DEBUG_APP(2,"databuf[0] = 0x%04x databuf[1] = 0x%04x\r\n",databuf[0],databuf[1]);						
					for( uint8_t  bufid = 0, dataid = 0; bufid < 4; )
					{
						SaveRs485s[PortId].MainBox.SensorBuff[bufid++] = (databuf[dataid]>>8)&0xff;
						SaveRs485s[PortId].MainBox.SensorBuff[bufid++] = (databuf[dataid++])&0xff;
					}						
				}
#endif
				
				break;
			}
			else
			{
        DEBUG_WARNING(3,"device had not find g_rs485index:%02x",id);
				SaveRs485s[PortId].Type = RS485_NONE;
			}
		}		
	}
	return SaveRs485s[PortId].Type;
}

/*
 *	SensorExpendBoxLive:		判断扩展盒哪些口接入传感器
 *	参数：			  					扩展盒接入标识，主板485ID
 *	返回值：								无
 */
static void SensorExpendBoxLive(int expend_sensor,int index)
{
	//0x4f
	for(int i = 0 ; i < 5 ; i ++)
	{
		if(i == 4)
		{
			if(expend_sensor & (0x01 << 6))
			{
				SaveRs485s[index].MainBox.ExpendBox[i].ExpendBoxLive = true;
			}
			break;
		}
		if(expend_sensor & (0x01 << i))
		{
			SaveRs485s[index].MainBox.ExpendBox[i].ExpendBoxLive = true;
		}  
	}	
}

/*
 *	SensorExpBoxAddr:		查询扩展接入传感器地址
 *	参数：			  			主板485ID
 *	返回值：						查询状态：成功/失败
 */
static HAL_StatusTypeDef SensorExpBoxAddr(int index)
{
	uint8_t len = 0;
	uint8_t repbuff[20] = {0};	
	
	uint8_t ExpId = 0;
	
	uint8_t MainIndex = index;
	
	uint8_t ExpIndex = ExpId;
	
	MainIndex++;
	
	for(ExpId = 0 ; ExpId < 5 ; ExpId ++)
	{
		ExpIndex++;

		SaveRs485s[index].MainBox.ExpendBox[ExpId].ExpenCheck = false;
		
		if( SaveRs485s[index].MainBox.ExpendBox[ExpId].ExpendBoxLive )
		{			
			DEBUG_APP(3,"main port=%02x ExpId = %d",MainIndex, ExpIndex);

			if(CheckRs485s[EXPENDBOX].RevDataLen != OpenExpenBox(ExpId))
			{
				HAL_Delay(100);
				DEBUG_ERROR(2,"main00 port = %02x, exbox port=%d sensor open faile",MainIndex,ExpIndex);				
				
				if(CheckRs485s[EXPENDBOX].RevDataLen != OpenExpenBox(ExpId))
				{
					DEBUG_ERROR(2,"main port = %02x, exbox port=%d sensor open faile",MainIndex,ExpIndex);				
					continue;
				}
			}
			else
			{
				DEBUG_APP(3,"exbox port=%d sensor open success",ExpIndex);
			}
			
		  len = Rs485s.Cmd(ExpendBoxbuff,7, NODEBUG,1000);   // 地址广播：get expend return data 	
	
			memcpy1(repbuff,Rs485s.Revbuff,len);		
			
			if(len >5) ///广播应答
			{											
				DEBUG_APP(3,"siganal ok and has rpy addr = 0x%02x",repbuff[3]);		
				int i = 0;

				////判断接口对应接入485传感器地址，记录
				for(i = 0 ; i < ARRAY(CheckRs485s); i ++)
				{
					if(CheckRs485s[i].Addr == repbuff[3]) 
					{											
						SaveRs485s[index].MainBox.ExpendBox[ExpId].ExpenCheck = true;
						SaveRs485s[index].MainBox.ExpendBox[ExpId].Index = (MainIndex<<4)|ExpId;   
						SaveRs485s[index].MainBox.ExpendBox[ExpId].CheckIndex = i; ///记录查询下标
						SaveRs485s[index].MainBox.ExpendBox[ExpId].Identifier = CheckRs485s[i].Identifier; ///解码标识

#if CHECKGETDATA
						Sensors.ExpenSigle( index, ExpId );
#endif						
									
						DEBUG_APP(3,"add device ok id = %d",i);		
						break;
					}
				}
				if(i == ARRAY(CheckRs485s))
				{
						//没有找到
						SaveRs485s[index].MainBox.ExpendBox[ExpId].ExpendBoxLive = false;				
						DEBUG_WARNING(2,"ExpendBox address = %d not in array",repbuff[3]);		
				}
			}
			else
			{
				//广播信息没有返回，认为这个传感器不接受FE查询
        //遍历
        DEBUG_WARNING(2,"port=%d sensor don`t return FE statr foreach CheckRs485s = %d",ExpIndex,ARRAY(CheckRs485s));			
				
				uint32_t startTime = HAL_GetTick();
				
				uint32_t Rs485TimeOut = 0;
				uint8_t	 i = 0;
							
				uint8_t  CheckRs485Index = 0;		
		
				/*******************工作场景模式选择：农场则不获取水产传感器数据*******************/
				if(0x01 == Sensors.SceneSelection)
				{
					i = 14;
					CheckRs485Index = ARRAY(CheckRs485s) - WATERSENSORNUMBER;
				}
				else
				{
					i = ARRAY(CheckRs485s) - WATERSENSORNUMBER;
					CheckRs485Index = ARRAY(CheckRs485s);
				}
				  
				for(; i < CheckRs485Index; i ++) ///只查询叶面传感器  4   7
				{
					if(i>=14)
					{
						Rs485TimeOut = 400; ///快速遍历水产传感器地址
					}
					else
					{
						Rs485TimeOut = CheckRs485s[i].TimeOut;
					}
						
					len = Rs485s.Cmd(CheckRs485s[i].SendBuff,CheckRs485s[i].SendDataLen, NODEBUG, Rs485TimeOut);
				
					if(len == CheckRs485s[i].RevDataLen)
					{						
						SaveRs485s[index].MainBox.ExpendBox[ExpId].ExpenCheck = true;
						SaveRs485s[index].MainBox.ExpendBox[ExpId].Index = (MainIndex<<4)|ExpId;   
						SaveRs485s[index].MainBox.ExpendBox[ExpId].CheckIndex = i; ///记录查询下标
						SaveRs485s[index].MainBox.ExpendBox[ExpId].Identifier = CheckRs485s[i].Identifier; ///解码标识

						///查询数据保存处理
						SaveRs485s[index].MainBox.ExpendBox[ExpId].SensorToLen = CheckRs485s[i].SensorToLen; ///传感器总长度		
						
#if CHECKGETDATA	
						
						Sensors.Counter ++;
						
#endif						
						memset(SaveRs485s[index].MainBox.ExpendBox[ExpId].DataBuff,0,len);
						
						memcpy1(SaveRs485s[index].MainBox.ExpendBox[ExpId].DataBuff,Rs485s.Revbuff,len);
						
						DEBUG_APP(2,"main_port = %02x, exbox_port=%d sensor Rs485 revData : ",MainIndex,ExpIndex);		
						DEBUG_APP(2,"rs485 ExpendBox get data = ");		
						Rs485s.Print(SaveRs485s[index].MainBox.ExpendBox[ExpId].DataBuff,len, APP); 
						
						/*********************************当前接入水产传感器，自动更新休眠时间标识生效**************************************/
						if(SaveRs485s[index].MainBox.ExpendBox[ExpId].Identifier == 0x05 || SaveRs485s[index].MainBox.ExpendBox[ExpId].Identifier == 0x11)
						{
							Sensors.WaterSensor = true;
						}
#if CHECKGETDATA
						//get data
						if(SaveRs485s[index].MainBox.ExpendBox[ExpId].Identifier != 0x11)
						{
							for(int k = 0, j = 3 ; j < len-2 ; j ++)
							{
								SaveRs485s[index].MainBox.ExpendBox[ExpId].SensorBuff[k] = SaveRs485s[index].MainBox.ExpendBox[ExpId].DataBuff[j];		
								DEBUG_APP(3,"get data[%d] = 0x%02x data0x%02x\r\n",j,SaveRs485s[index].MainBox.ExpendBox[ExpId].SensorBuff[k],SaveRs485s[index].MainBox.ExpendBox[ExpId].DataBuff[j]);
								k++;
							}
						}
						else 
						{
							uint16_t databuf[2] = {0};
							
							if(SaveRs485s[index].MainBox.ExpendBox[ExpId].SensorToLen == RS485_IDE_LEN+4)  //DO数据处理
							{
								for( uint8_t k = 0, j = 3; j < len-2; )
								{
									databuf[k] |= (SaveRs485s[index].MainBox.ExpendBox[ExpId].DataBuff[j++] << 8);
									databuf[k++] |= (SaveRs485s[index].MainBox.ExpendBox[ExpId].DataBuff[j++] << 0);
								}	
							}
							else  ///RDO数据处理
							{
								for( uint8_t k = 0, j = 3; j < len-4; )
								{
									databuf[k] |= (SaveRs485s[index].MainBox.ExpendBox[ExpId].DataBuff[j++] << 8);
									databuf[k++] |= (SaveRs485s[index].MainBox.ExpendBox[ExpId].DataBuff[j++] << 0);
									j += 2;
								}	
								SaveRs485s[index].MainBox.ExpendBox[ExpId].SensorToLen -= 4; ///过滤掉4字节，数据类型表示
							}
							
							databuf[1] *= 10;
							DEBUG_APP(3,"databuf[0] = 0x%04x databuf[1] = 0x%04x\r\n",databuf[0],databuf[1]);
							for( uint8_t  bufid = 0, dataid = 0; bufid < 4; )
							{
								SaveRs485s[index].MainBox.ExpendBox[ExpId].SensorBuff[bufid++] = (databuf[dataid]>>8)&0xff;
								SaveRs485s[index].MainBox.ExpendBox[ExpId].SensorBuff[bufid++] = (databuf[dataid++])&0xff;
							}									
						}
#endif						
						break;	
					}								
				}
				if(i == ARRAY(CheckRs485s))
				{
					//没有找到
					SaveRs485s[index].MainBox.ExpendBox[ExpId].ExpendBoxLive = false;				
					DEBUG_ERROR(3,"ExpendBox address = %d not in array",repbuff[3]);	
				}
			}			
		}
	}	
		
	//关闭 扩展盒
	OpenExpendBoxbuff[5] = 0x00;
	len = Rs485s.Cmd(OpenExpendBoxbuff,7, NODEBUG, 200);		
	if(len == 0)
	{
		DEBUG_ERROR(2,"close expend box falie");		
		return HAL_ERROR;		
	}
	else
	{
		DEBUG_APP(3,"close expend box");
		return HAL_OK;
	}
}

/*
 *	SensorMaBoxData:		读取主板Rs485传感器数据
 *	参数：			  			主板485ID
 *	返回值：						查询状态：成功/失败
 */
static HAL_StatusTypeDef SensorMaBoxData(uint8_t id)
{	
	uint32_t  startTime = 0;
	uint32_t  getDataTimeOut = 4000;
	uint16_t	databuf[2] = {0};
	uint8_t 	len = 0;	
	uint8_t   bufid = 0;
	
	len = Rs485s.Cmd(CheckRs485s[SaveRs485s[id].MainBox.CheckIndex].SendBuff,CheckRs485s[SaveRs485s[id].MainBox.CheckIndex].SendDataLen,\
								   NODEBUG, CheckRs485s[SaveRs485s[id].MainBox.CheckIndex].TimeOut);
	//等待发送完成,				
	startTime = HAL_GetTick();
	while(HAL_GetTick() - startTime < getDataTimeOut && len != CheckRs485s[SaveRs485s[id].MainBox.CheckIndex].RevDataLen)
	{
		len = Rs485s.Cmd(CheckRs485s[SaveRs485s[id].MainBox.CheckIndex].SendBuff,CheckRs485s[SaveRs485s[id].MainBox.CheckIndex].SendDataLen,\
								     NODEBUG, CheckRs485s[SaveRs485s[id].MainBox.CheckIndex].TimeOut);
		HAL_Delay(500);
	}
	if(len != CheckRs485s[SaveRs485s[id].MainBox.CheckIndex].RevDataLen)
	{
		DEBUG_ERROR(2,"Get MainBox : %02x data error",SaveRs485s[id].MainBox.Index);
		
		///获取数据时某个接口传感器异常：传感器总长度清零，作为识别标识
		SaveRs485s[id].MainBox.SensorToLen = 0; ///传感器总长度		
		Sensors.Counter ++;
		return HAL_ERROR;
	}
	
	SaveRs485s[id].MainBox.SensorToLen = CheckRs485s[SaveRs485s[id].MainBox.CheckIndex].SensorToLen; ///传感器总长度		
	Sensors.Counter ++;

	memset(SaveRs485s[id].MainBox.DataBuff,0,len);
	memcpy1(SaveRs485s[id].MainBox.DataBuff,Rs485s.Revbuff,len);
	id ++;
	DEBUG_APP(2,"main_box = %d, sensor Rs485 revData : ",id);
	id--;
	Rs485s.Print(SaveRs485s[id].MainBox.DataBuff,len, 2);	
	
	DEBUG_APP(2,"len = %d, len-4 = %d",len,len-4);
	
	//get data
	if(CheckRs485s[SaveRs485s[id].MainBox.CheckIndex].Identifier != 0x11)
	{
		for(int j = 3 ; j < len-2 ; j ++)
		{
			SaveRs485s[id].MainBox.SensorBuff[bufid] = SaveRs485s[id].MainBox.DataBuff[j];	
			
			DEBUG_APP(3,"get data[%d] = 0x%02x\r\n",j,SaveRs485s[id].MainBox.SensorBuff[bufid]);	
			bufid++;
		}
	}
	else
	{	
//		if(CheckRs485s[SaveRs485s[id].MainBox.CheckIndex].SensorToLen == RS485_IDE_LEN+4)  //DO数据处理
		if(len == 9)  //DO数据处理
		{
			for( uint8_t k = 0, j = 3; j < len-2; )
			{
				databuf[k] 		= (SaveRs485s[id].MainBox.DataBuff[j++] << 8);
				databuf[k++] |= (SaveRs485s[id].MainBox.DataBuff[j++] << 0);
			}	
		}
		else if(len == 13)  ///RDO数据处理
		{
			for( uint8_t k = 0, j = 3; j < len-4; )
			{
				databuf[k] 		= (SaveRs485s[id].MainBox.DataBuff[j++] << 8);
				databuf[k++] |= (SaveRs485s[id].MainBox.DataBuff[j++] << 0);
				j += 2;
			}	
			
//			CheckRs485s[SaveRs485s[id].MainBox.CheckIndex].SensorToLen -= 4; ///过滤掉4字节，数据类型表示
		}		
		
		DEBUG_APP(2,"databuf[0] = %04x, databuf[1] = %04x",databuf[0],databuf[1]);
		
		databuf[1] *= 10;
		
		DEBUG_APP(2,"databuf[0] = %04x, databuf[1] = %04x",databuf[0],databuf[1]);
		
		for( uint8_t  bufid = 0, dataid = 0; bufid < 4; )
		{
			SaveRs485s[id].MainBox.SensorBuff[bufid++] = (databuf[dataid]>>8)&0xff;			
			SaveRs485s[id].MainBox.SensorBuff[bufid++] = (databuf[dataid++])&0xff;			
		}			
	}
		
	return HAL_OK;
}

/*
 *	SensorExpenSigle:		读取扩展盒Rs485
 *	参数：			  			主板485ID, 指定扩展盒接口
 *	返回值：						查询状态：成功/失败
 */
static HAL_StatusTypeDef SensorExpenSigle(uint8_t index, uint8_t Exid)
{
	uint8_t len = 0;
	
	uint32_t startTime = 0;
	uint32_t getDataTimeOut = 4000;
	
	uint8_t main_box = index;
	uint8_t ex_box   = Exid;
	
	main_box ++;	

	/***********************判断对应485接口是否接入传感器***********************************/
	if( SaveRs485s[index].MainBox.ExpendBox[Exid].ExpenCheck ) 
	{					
		ex_box ++;
		/***********************打开扩展盒接口状态***********************************/
		if(CheckRs485s[EXPENDBOX].RevDataLen != OpenExpenBox(Exid))
		{
			
			DEBUG_ERROR(2,"111main port = %d, exbox port=%d RevDataLen = %d sensor open faile",main_box,ex_box,CheckRs485s[EXPENDBOX].RevDataLen);	
			HAL_Delay(200);		

			if(CheckRs485s[EXPENDBOX].RevDataLen != OpenExpenBox(Exid))
			{
				
				DEBUG_ERROR(2,"222main port = %d, exbox port=%d sensor open faile",main_box,ex_box);	
				
				Sensors.Counter ++;
				SaveRs485s[index].MainBox.ExpendBox[Exid].SensorToLen = 0;					
				return HAL_ERROR;
			}
		}

		len = Rs485s.Cmd(CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].SendBuff,CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].SendDataLen,\
										 NODEBUG,CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].TimeOut);      
		//等待发送完成,				
		startTime = HAL_GetTick();
		while(HAL_GetTick() - startTime < getDataTimeOut && len != CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].RevDataLen)
		{
			len = Rs485s.Cmd(CheckRs485s[SaveRs485s[index].MainBox.CheckIndex].SendBuff,CheckRs485s[SaveRs485s[index].MainBox.CheckIndex].SendDataLen,\
											 NODEBUG,CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].TimeOut);                                       
			HAL_Delay(500);
		}
		if(len != CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].RevDataLen)
		{
			DEBUG_ERROR(2,"Get Exid : %02x data error",SaveRs485s[index].MainBox.ExpendBox[Exid].Index);	
			
#if CHECKGETDATA			
			
			Sensors.Counter ++;
			
#endif			
						
			SaveRs485s[index].MainBox.ExpendBox[Exid].SensorToLen = 0;
			
			return HAL_ERROR;
		}
		else
		{
#if CHECKGETDATA	
			
			Sensors.Counter ++;
			
#endif			
			
			SaveRs485s[index].MainBox.ExpendBox[Exid].SensorToLen = CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].SensorToLen;
		
			memset(SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff,0,len);
			memcpy1(SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff,Rs485s.Revbuff,len);
			
			DEBUG_APP(2,"main_box = %d, exbox port=%d Index = 0x%02x sensor Rs485 revData : ",main_box,ex_box,SaveRs485s[index].MainBox.ExpendBox[Exid].Index);
			Rs485s.Print(SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff,len, 2);
				//get data
			if(CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].Identifier != 0x11)
			{
				for(int k = 0, j = 3 ; j < len-2 ; j ++)
				{
					SaveRs485s[index].MainBox.ExpendBox[Exid].SensorBuff[k++] = SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff[j];						
				}
			}
			else
			{
				uint16_t databuf[2] = {0};
				
//				if(CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].SensorToLen == RS485_IDE_LEN+4)  //DO数据处理
				
				if(len == 9)//DO数据处理 
				{
					for( uint8_t k = 0, j = 3; j < len-2; )
					{
						databuf[k] 		= (SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff[j++] << 8);
						databuf[k++] |= (SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff[j++] << 0);
					}	
				}
				else if(len == 13) ///RDO数据处理
				{
					for( uint8_t k = 0, j = 3; j < len-4; )
					{
						databuf[k] 		= (SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff[j++] << 8);
						databuf[k++] |= (SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff[j++] << 0);
						j += 2;
					}	
										
//					CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].SensorToLen -= 4; ///过滤掉4字节，数据类型表示
				}
				
				databuf[1] *= 10;
				
				for( uint8_t  bufid = 0, dataid = 0; bufid < 4; )
				{
					SaveRs485s[index].MainBox.ExpendBox[Exid].SensorBuff[bufid++] = (databuf[dataid]>>8)&0xff;
					SaveRs485s[index].MainBox.ExpendBox[Exid].SensorBuff[bufid++] = (databuf[dataid++])&0xff;
				}									
			}
		}			
	}
	
	//关闭 扩展盒
	OpenExpendBoxbuff[5] = 0x00;
	len = Rs485s.Cmd(OpenExpendBoxbuff,7, NODEBUG, 100);		
	if(len == 0)
	{
		DEBUG_ERROR(2,"close expend box falie");		
		return HAL_ERROR;		
	}
	else
	{
		DEBUG_APP(3,"close expend box ok");
		return HAL_OK;
	}
}

/*
 *	SensorExpenData:		读取扩展盒Rs485传感器数据
 *	参数：			  			主板485ID
 *	返回值：						查询状态：成功/失败
 */
static HAL_StatusTypeDef SensorExpenData(uint8_t index)
{
	uint8_t len = 0;
	
	uint32_t startTime = 0;
	uint32_t getDataTimeOut = 4000;
	
	uint8_t Exid = 0;
		
	uint8_t main_box = index;
	uint8_t ex_box   = Exid;
	
	main_box ++;

	for(Exid = 0; Exid < 5; Exid++)
	{
		ex_box ++;
		/***********************判断对应485接口是否接入传感器***********************************/
		if( SaveRs485s[index].MainBox.ExpendBox[Exid].ExpenCheck ) 
		{								
			/***********************打开扩展盒接口状态***********************************/
			if(CheckRs485s[EXPENDBOX].RevDataLen != OpenExpenBox(Exid))
			{
				
				DEBUG_ERROR(2,"111main port = %d, exbox port=%d RevDataLen = %d sensor open faile",main_box,ex_box,CheckRs485s[EXPENDBOX].RevDataLen);	
				HAL_Delay(200);		

				if(CheckRs485s[EXPENDBOX].RevDataLen != OpenExpenBox(Exid))
				{
					
					DEBUG_ERROR(2,"222main port = %d, exbox port=%d sensor open faile",main_box,ex_box);	
					
					Sensors.Counter ++;
					SaveRs485s[index].MainBox.ExpendBox[Exid].SensorToLen = 0;					
					continue;
				}
			}

			len = Rs485s.Cmd(CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].SendBuff,CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].SendDataLen,\
											 NODEBUG,CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].TimeOut);      
			//等待发送完成,				
			startTime = HAL_GetTick();
			while(HAL_GetTick() - startTime < getDataTimeOut && len != CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].RevDataLen)
			{
				len = Rs485s.Cmd(CheckRs485s[SaveRs485s[index].MainBox.CheckIndex].SendBuff,CheckRs485s[SaveRs485s[index].MainBox.CheckIndex].SendDataLen,\
												 NODEBUG,CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].TimeOut);                                       
				HAL_Delay(500);
			}
			if(len != CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].RevDataLen)
			{
				DEBUG_ERROR(2,"Get Exid : %02x data error",SaveRs485s[index].MainBox.ExpendBox[Exid].Index);	
				
				Sensors.Counter ++;
								
				SaveRs485s[index].MainBox.ExpendBox[Exid].SensorToLen = 0;
				
				continue;
			}
			else
			{
				Sensors.Counter ++;
				
				SaveRs485s[index].MainBox.ExpendBox[Exid].SensorToLen = CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].SensorToLen;
			
				memset(SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff,0,len);
				memcpy1(SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff,Rs485s.Revbuff,len);
				
				DEBUG_APP(2,"main_box = %d, exbox port=%d Index = 0x%02x sensor Rs485 revData : ",main_box,ex_box,SaveRs485s[index].MainBox.ExpendBox[Exid].Index);
				Rs485s.Print(SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff,len, 2);
					//get data
				if(CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].Identifier != 0x11)
				{
					for(int k = 0, j = 3 ; j < len-2 ; j ++)
					{
						SaveRs485s[index].MainBox.ExpendBox[Exid].SensorBuff[k++] = SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff[j];						
					}
				}
				else
				{
					uint16_t databuf[2] = {0};
					
//					if(CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].SensorToLen == RS485_IDE_LEN+4)  //DO数据处理
					if(len == 9) //DO数据处理
					{
						for( uint8_t k = 0, j = 3; j < len-2; )
						{
							databuf[k] 		= (SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff[j++] << 8);
							databuf[k++] |= (SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff[j++] << 0);
						}	
					}
					else if(len == 13) ///RDO数据处理
					{
						for( uint8_t k = 0, j = 3; j < len-4; )
						{
							databuf[k] 		= (SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff[j++] << 8);
							databuf[k++] |= (SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff[j++] << 0);
							j += 2;
						}	
//						CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].SensorToLen -= 4; ///过滤掉4字节，数据类型表示
					}		
					
					databuf[1] *= 10;
					
					for( uint8_t  bufid = 0, dataid = 0; bufid < 4; )
					{
						SaveRs485s[index].MainBox.ExpendBox[Exid].SensorBuff[bufid++] = (databuf[dataid]>>8)&0xff;
						SaveRs485s[index].MainBox.ExpendBox[Exid].SensorBuff[bufid++] = (databuf[dataid++])&0xff;
					}									
				}

			}			
		}
	}
	
	//关闭 扩展盒
	OpenExpendBoxbuff[5] = 0x00;
	len = Rs485s.Cmd(OpenExpendBoxbuff,7, NODEBUG, 100);		
	if(len == 0)
	{
		DEBUG_ERROR(2,"close expend box falie");		
		return HAL_ERROR;		
	}
	else
	{
		DEBUG_APP(3,"close expend box ok");
		return HAL_OK;
	}
}

/*
*OpenExpenBox：打开扩展盒接口
*ExpId：			 扩展口ID
*返回：				 扩展盒返回数据长度
*/
static uint8_t OpenExpenBox(uint8_t ExpId)
{
	int temp = 0;
	uint8_t len = 0;
	
	if(ExpId == 4)
	{
		temp = 6;
	}
	else
	{
		temp = ExpId;				
	}
	OpenExpendBoxbuff[5] = 0x01 << temp;
	len = Rs485s.Cmd(OpenExpendBoxbuff,7, NODEBUG, 100); 

	return len;
}

/*
*RS485CmdPackage：Rs485查询数据命令包
*mode：			 			Rs485功能码
*返回：				    无
*/
static void RS485CmdPackage(char mode)
{
	for(int index = 0 ; index < ARRAY(CheckRs485s); index ++)
	{
		DEBUG_APP(3,"index = %d,addr = %02X\r\n",index,CheckRs485s[index].Addr);
		memset(CheckRs485s[index].SendBuff,0,sizeof(CheckRs485s[index].SendBuff));
		CheckRs485s[index].SendBuff[0] = CheckRs485s[index].Addr;
		
		if(15 == index)
		{
			CheckRs485s[index].SendBuff[1] = NBI_RS485_MOISTURE_LEAF;
		}
		else
		{
			CheckRs485s[index].SendBuff[1] = mode;
		}
		CheckRs485s[index].SendBuff[2] = (CheckRs485s[index].RegAddress & 0xFF00)  >> 8;		
		CheckRs485s[index].SendBuff[3] = (CheckRs485s[index].RegAddress & 0x00FF);
		CheckRs485s[index].SendBuff[4] = (CheckRs485s[index].RegDatalen & 0xFF00)  >> 8;				
		CheckRs485s[index].SendBuff[5] = (CheckRs485s[index].RegDatalen & 0x00FF);
		
//		for(uint8_t i = 0; i < 6; i ++)
//	  DEBUG(3,"%02X ",CheckRs485s[index].SendBuff[i]);
//		DEBUG(3,"\r\n");
	}
}


