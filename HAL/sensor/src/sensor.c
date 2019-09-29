/*
**************************************************************************************************************
*	@file			sensor.c
*	@author 	Jason
*	@version 	V0.1
*	@date     2018/07/13
*	@brief		�������ɼ�
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
	{0x00, 0x00,				0x0000,     0x0000,         7,          9,    RS485_IDE_LEN+4,    200*1, "EXPEND",  	"",    		"EXPEND"},	///��չ��
	{0x02, 0x02,				0x0000, 		0x0002,  				6,					9, 		RS485_IDE_LEN+4,		500*1,		"" ,			"" ,		"SWR-100W"},  ///������ʪ��
	{0x05, 0x05,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		1000*1,	  "" ,			"" ,			 "ST_PH"},  ///������ˮPH  1000*30
	{0x06, 0x06,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		1000*1,		"" ,			"" ,		   "ST_GH"},	///�����Ч
	{0x0C, 0x03,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		500*1,		"" ,			"" ,		   "ST-TW"},  ///�����¶�
	{0x0D, 0x0D,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		1000*1,		"" ,			"" ,		   "ST-EC"},  ///EC
	{0x0E, 0x0E,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		1000*10,	"" ,			"" ,		  "ST_CO2"},	///CO2		
//	{0x0F, 0x24,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		1000*1,		"" ,			"" ,		"ST_AP"		},///����վ������ѹ				
	{0x12, 0x12,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		200*1,		"" ,			"" ,		   "andan"},  ///����
	{0x13, 0x13,				0x0000, 		0x0002,  				6,					9, 		RS485_IDE_LEN+4,		200*1,		"" ,			"" ,		"Water-EC"},  ///ˮEC
	{0x14, 0x14,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		200*1,		"" ,			"" ,		 "Water-T"},  ///ˮ��
	{0x15, 0x15,				0x0000, 		0x0002,  				6,					7, 		RS485_IDE_LEN+2,		200*1,		"" ,			"" ,		"Water-K+"},	///ˮ��+
	
	{0xF8, 0x29,				0x0000, 		0x0002,  				6,					9, 		RS485_IDE_LEN+4,		1000*1,		"" ,			"" ,		"Soil-EC"},    ///����EC 
	{0xF9, 0x2A,				0x0000, 		0x0002,  				6,					9, 		RS485_IDE_LEN+4,		1000*1,		"" ,			"" ,		"Soil-Tech"},  ///������ʪ��
	
	{0xFD, 0x18,				0x0000, 		0x0004,  				6,				 13, 		RS485_IDE_LEN+8,		1000*1,	  "" ,		  "" ,		 "Air-Ill"},	///������ʪ�ȡ�����

	/****************************   ���´�������֧�ֹ㲥����   ***************************/
	{0x07, 0x07,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		1000*1,		"" ,			"" ,		 "ST_Y/MW"},  ///Ҷ���¶�
		    
			/**************************   Ҷ��ʪ�����⹦���룺0x04   *****************/
	{0x08, 0x08,				0x0001, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		1000*1,		"" ,			"" ,		  "ST_YMS"},  ///Ҷ��ʪ��:�����룺0x04
	{0x17, 0x17,				0x0015, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		1000*4,		"" ,		  "" ,		   "WH_EC"},  ///��������EC	
       
			/**************************   ���´�������ˮ��   *****************/
	{0x05, 0x05,				0x0000, 		0x0001,  				6,					7, 		RS485_IDE_LEN+2,		1000*60,	"" ,			"" ,			 "WT_PH"},  ///ˮPH  1000*30
	{0x11, 0x11,				0x0000, 		0x0002,  				6,					9, 		RS485_IDE_LEN+4,		1000*120,	"" ,			"" ,		     "OXY"},  ///ˮ���� 1000*120
	{0x18, 0x11,				0x0000, 		0x0004,  				6,					13, 	RS485_IDE_LEN+4,		1000*30,	"" ,		  "" ,		     "RDO"},  ///ӫ��DO 180 +8
};

SaveRs485_t  SaveRs485s[3];

SendBuf_t		 SendBufs[10] = {{{0}, {0}, 0}};

Sensor_t     Sensors;
	

volatile 	uint8_t SendBufsCounter = 0; ///��¼���ݻ������������

uint8_t OpenExpendBoxbuff[10] = {0x00,0x05,0x00,0x01,0x00,0x00,0x00};
uint8_t ExpendBoxbuff[9] = {0xFE,0x03,0x04,0x00,0x00,0x00,0x00,0x00,0x00};


/*
 *	SensorsInit:		��������ʼ��
 *	������			  	��
 *	����ֵ��				��
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
 *	GetRs485Type:		485�ӿ�����
 *	������			  	����Rs485�ӿ�ID
 *	����ֵ��				Rs485���ͣ�RS485_NONE/RS485_EXPAND_BOX/RS485_SIGNAL
 */
static Rstype_t GetRs485Type(int index)
{	
	return SaveRs485s[index].Type;
}

/*
 *	SensorGetData:		��ȡRs185����������
 *	������			  		Rs485����ӿ�
 *	����ֵ��					��ȡ�ɹ�/ʧ��
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
 *	SensorHandle:	  ����������
 *	������			  	��
 *	����ֵ��				��
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
 *	SensorCheckHandle:	  ��һ���ϵ紫��������
 *	������			  				��
 *	����ֵ��							��
 */
static void SensorCheckHandle(void)
{
	SendBufsCounter = 0;
	
	Sensors.DataProces(  );
}

/*
 *	SensorDataProces:	���������ݴ���
 *	������			  		��
 *	����ֵ��					��
 */
static void SensorDataProces(void) 
{
	/**********���㳤��**************/
	uint8_t Len = 0;
	
	/**********����������BUF�±�**************/
	uint8_t Index = 0;
	
	/***********��չ��ID*************/
	uint8_t ExId = 0;
	
	/***********����ID*************/
	uint8_t PortId = 0;
	
	/**********ʵ�������ܳ���**************/
	uint8_t Length = 0; 
	
	/***********������������*************/
	uint8_t Temp[64] = {0};
	
	/***********���������ݳ���*************/
	uint8_t TempIndex = 0;
	
	/***********ʣ�����ȡ����������*************/
	uint8_t GetSensorCounter = 0;

#if 1

	DEBUG_APP(2, "-----Start get data Counter : %d----",Sensors.Counter);
	for(PortId = 0; PortId < NBI_RS485_PIN_COUNT; PortId++)
	{
		if(SaveRs485s[PortId].Type == RS485_SIGNAL && GetSensorCounter < Sensors.Counter)
		{
			Len += SaveRs485s[PortId].MainBox.SensorToLen;
			
			if(Len <= MAXLEN) ///��Ϊ��ȡ�������������쳣��־
			{
					DEBUG_APP(3, "-----SensorToLen : %d----",SaveRs485s[PortId].MainBox.SensorToLen);

				if(SaveRs485s[PortId].MainBox.SensorToLen != 0)
				{
					SendBufs[SendBufsCounter].Port[Index++] = SaveRs485s[PortId].MainBox.Index; //�ӿ�ID
					SendBufs[SendBufsCounter].Port[Index++] = SaveRs485s[PortId].MainBox.Identifier; //�����ʶ
					
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
				/********************�ְ��ڼ�֡*****************/
				SendBufs[SendBufsCounter].Buf[0] = SendBufsCounter;
				
				/********************ָ��*****************/
				SendBufs[SendBufsCounter].Buf[1] = 0x00;
				
				/********************����*****************/
				SendBufs[SendBufsCounter].Buf[2] = ReadBattery(  );
				
				/********************����������*****************/
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
				
				PortId --; //�±����	
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
				if(SaveRs485s[PortId].MainBox.ExpendBox[ExId].ExpenCheck) ///�ӿڴ���
				{
					Len += SaveRs485s[PortId].MainBox.ExpendBox[ExId].SensorToLen;
			
					DEBUG_APP(3,"PortId = %d, ExId = %d Len = %d SensorToLen = %d",PortId,ExId,Len, \
					SaveRs485s[PortId].MainBox.ExpendBox[ExId].SensorToLen);
					if(Len <= MAXLEN)
					{
						if(SaveRs485s[PortId].MainBox.ExpendBox[ExId].SensorToLen != 0)
						{
							SendBufs[SendBufsCounter].Port[Index++] = SaveRs485s[PortId].MainBox.ExpendBox[ExId].Index; //�ӿ�ID
							SendBufs[SendBufsCounter].Port[Index++] = SaveRs485s[PortId].MainBox.ExpendBox[ExId].Identifier; //�����ʶ
							
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
						
						/********************�ְ��ڼ�֡*****************/
						SendBufs[SendBufsCounter].Buf[0] = SendBufsCounter;
						
						/********************ָ��*****************/
						SendBufs[SendBufsCounter].Buf[1] = 0x00;
						
						/********************����*****************/
						SendBufs[SendBufsCounter].Buf[2] = ReadBattery(  );
						
						/********************����������*****************/
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
		/********************�ְ��ڼ�֡*****************/
		SendBufs[SendBufsCounter].Buf[0] = SendBufsCounter;
		
		/********************ָ��*****************/
		SendBufs[SendBufsCounter].Buf[1] = 0x00;
		
		/********************����*****************/
		SendBufs[SendBufsCounter].Buf[2] = ReadBattery(  );
		
		/********************����������*****************/
		SendBufs[SendBufsCounter].Buf[3] = Index/2;

		uint8_t BufLen = 4;
		
		memset(&SendBufs[SendBufsCounter].Buf[BufLen], 0, 64);

		memcpy1(&SendBufs[SendBufsCounter].Buf[BufLen], SendBufs[SendBufsCounter].Port, Index); ///��������ʶ
		DEBUG_APP(3, "444PortId : %d counter = %d\r\n",PortId, GetSensorCounter);
		
		BufLen += Index;
		
		memcpy1(&SendBufs[SendBufsCounter].Buf[BufLen], Temp, TempIndex); ///����������
		
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
 *	SensorQueryPinStaus:		�㲥��ѯ485����ȡ����������
 *	������			  					��
 *	����ֵ��								��ѯ״̬���ɹ�/ʧ��/��ʱ
 */
static HAL_StatusTypeDef SensorQueryPinStaus(void)
{	
	HAL_StatusTypeDef Status = HAL_TIMEOUT;
	
	DEBUG_APP(2,"Start get Rs485 Sensor data, It need some time, waiting......\r\n");
	
	RS485CmdPackage(NBI_RS485_SEARCH_CODE);///��ȡԤ��485�����
		
	for(int id = 0; id < NBI_RS485_PIN_COUNT ; id++)
	{		
		//����plus��6���ڣ��ֱ���ʲô
		//��io��	
		Rs485s.OpenPin(id);
		HAL_Delay(1500);
		//�ж���ʲô�ڣ����һ�ȡ���д������ĵ�ַ
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
 *	SensorQueryType:		�㲥��ѯ����485����
 *	������			  			Rs485�ӿ�
 *	����ֵ��						Rs485���ͣ�RS485_NONE/RS485_EXPAND_BOX/RS485_SIGNAL
 */
static Rstype_t SensorQueryType(int PortId)
{
	uint8_t repbuff[20] = {0};	
	uint8_t expend_sensor = 0;
	
	uint8_t id = PortId;
	id++;

	SaveRs485s[PortId].Type = RS485_NONE;
	
	int len = Rs485s.Cmd(ExpendBoxbuff,7, 2, 1000);   // ��ַ�㲥��get expend return data 	
	
	memcpy1(repbuff,Rs485s.Revbuff,len);
	Rs485s.Print(repbuff, len, 2);
			
  ///�жϹ㲥�ظ���ַ����չ�е�ַӦ��
	
	if(len == CheckRs485s[EXPENDBOX].RevDataLen && repbuff[3] == 0) //��չ�еĵ�ַ Ϊ0
	{
		//���ص����ݳ��ȴ���0����Ϊ���ⲿ����
		expend_sensor = repbuff[4];  	//��չ����Щ�ڽ��봫����
				
		Sensors.ExpendBoxLive(expend_sensor,PortId); ///��չ����Ӧ�ӿ�״̬��־
        
		SaveRs485s[PortId].Type = RS485_EXPAND_BOX;
				
		Sensors.ExpBoxAddr( PortId );		
	}
	else if(len >5) ///����չ�е�ַӦ��
	{				
		//û���ҵ���չ�е��ǵ��������˻ظ�
		DEBUG_APP(3,"siganal ok and has rpy addr = %x",repbuff[3]);		
		int i = 0;

		////�жϽӿڶ�Ӧ����485��������ַ����¼
		for(i = 0; i < ARRAY(CheckRs485s); i ++)
		{
			if(CheckRs485s[i].Addr == repbuff[3]) 
			{					
				SaveRs485s[PortId].Type = RS485_SIGNAL;
				
				SaveRs485s[PortId].MainBox.Index = id;  
				SaveRs485s[PortId].MainBox.CheckIndex = i; ///��¼��ѯ�±�
				SaveRs485s[PortId].MainBox.Identifier = CheckRs485s[i].Identifier; ///�����ʶ
						
				DEBUG_APP(2,"CheckRs485s is ok index = %d",i);		
	
#if CHECKGETDATA		
				
				Sensors.MaBoxData( PortId );

#endif				
				break;
			}
		}
		if(i == ARRAY(CheckRs485s))
		{
				//û���ҵ�
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
		//û���ҵ���չ�У�������Ҳû�лظ�����ʼ�������е��ⲿ�豸������
		DEBUG_WARNING(3,"siganal ok and foreach");		
		uint32_t startTime = HAL_GetTick(  );
		
		uint32_t Rs485TimeOut = 0;
		uint8_t  CheckRs485Index = 0;
		uint8_t	 i = 0;
		
		/*******************��������ģʽѡ��ũ���򲻻�ȡˮ������������*******************/
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

		for(; i <  CheckRs485Index; i ++) ///ֻ��ѯҶ�洫���� 
		{			
			if(i>=14)
			{
				Rs485TimeOut = 400; ///���ٱ���ˮ����������ַ����ǰ������Ч��������
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
				//�ҵ���
				DEBUG_APP(2,"main_box = %d, device had find:0x%02x",++PortId, CheckRs485s[i].Addr);
				--PortId;
				
				SaveRs485s[PortId].MainBox.Index = id;
				SaveRs485s[PortId].MainBox.CheckIndex = i; ///��¼��ѯ�±�
				SaveRs485s[PortId].MainBox.Identifier = CheckRs485s[i].Identifier; ///�����ʶ

				SaveRs485s[PortId].Type = RS485_SIGNAL;				
				
				///��ѯ���ݱ��洦��
				SaveRs485s[PortId].MainBox.SensorToLen = CheckRs485s[i].SensorToLen; ///�������ܳ���		
				
#if CHECKGETDATA				
				Sensors.Counter ++;
#endif
				
				DEBUG_APP(2,"CheckRs485s[i].Identifier = 0x%02x Sensors.Counter = %d",CheckRs485s[i].Identifier,Sensors.Counter);
				
				DEBUG_APP(2,"sensor Rs485 revData :");
				
				memset(SaveRs485s[PortId].MainBox.DataBuff,0,len);
				memcpy1(SaveRs485s[PortId].MainBox.DataBuff,Rs485s.Revbuff,len);
				
				Rs485s.Print(SaveRs485s[PortId].MainBox.DataBuff, len, 2);
				
				/*********************************��ǰ����ˮ�����������Զ���������ʱ���ʶ��Ч**************************************/
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
										
					if(SaveRs485s[PortId].MainBox.SensorToLen == RS485_IDE_LEN+4)  //DO���ݴ���
					{
						for( uint8_t k = 0, j = 3; j < len-2; )
						{
							databuf[k] |= (SaveRs485s[PortId].MainBox.DataBuff[j++]<<8);
							databuf[k++] |= (SaveRs485s[PortId].MainBox.DataBuff[j++]<<0);
						}	
					}
					else  ///RDO���ݴ���
					{
						for( uint8_t k = 0, j = 3; j < len-4; )
						{
							databuf[k] |= (SaveRs485s[PortId].MainBox.DataBuff[j++]<<8);
							databuf[k++] |= (SaveRs485s[PortId].MainBox.DataBuff[j++]<<0);
							j += 2;
						}	
						SaveRs485s[PortId].MainBox.SensorToLen -= 4; ///���˵�4���ֽ��������ͱ�ʾ
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
 *	SensorExpendBoxLive:		�ж���չ����Щ�ڽ��봫����
 *	������			  					��չ�н����ʶ������485ID
 *	����ֵ��								��
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
 *	SensorExpBoxAddr:		��ѯ��չ���봫������ַ
 *	������			  			����485ID
 *	����ֵ��						��ѯ״̬���ɹ�/ʧ��
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
			
		  len = Rs485s.Cmd(ExpendBoxbuff,7, NODEBUG,1000);   // ��ַ�㲥��get expend return data 	
	
			memcpy1(repbuff,Rs485s.Revbuff,len);		
			
			if(len >5) ///�㲥Ӧ��
			{											
				DEBUG_APP(3,"siganal ok and has rpy addr = 0x%02x",repbuff[3]);		
				int i = 0;

				////�жϽӿڶ�Ӧ����485��������ַ����¼
				for(i = 0 ; i < ARRAY(CheckRs485s); i ++)
				{
					if(CheckRs485s[i].Addr == repbuff[3]) 
					{											
						SaveRs485s[index].MainBox.ExpendBox[ExpId].ExpenCheck = true;
						SaveRs485s[index].MainBox.ExpendBox[ExpId].Index = (MainIndex<<4)|ExpId;   
						SaveRs485s[index].MainBox.ExpendBox[ExpId].CheckIndex = i; ///��¼��ѯ�±�
						SaveRs485s[index].MainBox.ExpendBox[ExpId].Identifier = CheckRs485s[i].Identifier; ///�����ʶ

#if CHECKGETDATA
						Sensors.ExpenSigle( index, ExpId );
#endif						
									
						DEBUG_APP(3,"add device ok id = %d",i);		
						break;
					}
				}
				if(i == ARRAY(CheckRs485s))
				{
						//û���ҵ�
						SaveRs485s[index].MainBox.ExpendBox[ExpId].ExpendBoxLive = false;				
						DEBUG_WARNING(2,"ExpendBox address = %d not in array",repbuff[3]);		
				}
			}
			else
			{
				//�㲥��Ϣû�з��أ���Ϊ���������������FE��ѯ
        //����
        DEBUG_WARNING(2,"port=%d sensor don`t return FE statr foreach CheckRs485s = %d",ExpIndex,ARRAY(CheckRs485s));			
				
				uint32_t startTime = HAL_GetTick();
				
				uint32_t Rs485TimeOut = 0;
				uint8_t	 i = 0;
							
				uint8_t  CheckRs485Index = 0;		
		
				/*******************��������ģʽѡ��ũ���򲻻�ȡˮ������������*******************/
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
				  
				for(; i < CheckRs485Index; i ++) ///ֻ��ѯҶ�洫����  4   7
				{
					if(i>=14)
					{
						Rs485TimeOut = 400; ///���ٱ���ˮ����������ַ
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
						SaveRs485s[index].MainBox.ExpendBox[ExpId].CheckIndex = i; ///��¼��ѯ�±�
						SaveRs485s[index].MainBox.ExpendBox[ExpId].Identifier = CheckRs485s[i].Identifier; ///�����ʶ

						///��ѯ���ݱ��洦��
						SaveRs485s[index].MainBox.ExpendBox[ExpId].SensorToLen = CheckRs485s[i].SensorToLen; ///�������ܳ���		
						
#if CHECKGETDATA	
						
						Sensors.Counter ++;
						
#endif						
						memset(SaveRs485s[index].MainBox.ExpendBox[ExpId].DataBuff,0,len);
						
						memcpy1(SaveRs485s[index].MainBox.ExpendBox[ExpId].DataBuff,Rs485s.Revbuff,len);
						
						DEBUG_APP(2,"main_port = %02x, exbox_port=%d sensor Rs485 revData : ",MainIndex,ExpIndex);		
						DEBUG_APP(2,"rs485 ExpendBox get data = ");		
						Rs485s.Print(SaveRs485s[index].MainBox.ExpendBox[ExpId].DataBuff,len, APP); 
						
						/*********************************��ǰ����ˮ�����������Զ���������ʱ���ʶ��Ч**************************************/
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
							
							if(SaveRs485s[index].MainBox.ExpendBox[ExpId].SensorToLen == RS485_IDE_LEN+4)  //DO���ݴ���
							{
								for( uint8_t k = 0, j = 3; j < len-2; )
								{
									databuf[k] |= (SaveRs485s[index].MainBox.ExpendBox[ExpId].DataBuff[j++] << 8);
									databuf[k++] |= (SaveRs485s[index].MainBox.ExpendBox[ExpId].DataBuff[j++] << 0);
								}	
							}
							else  ///RDO���ݴ���
							{
								for( uint8_t k = 0, j = 3; j < len-4; )
								{
									databuf[k] |= (SaveRs485s[index].MainBox.ExpendBox[ExpId].DataBuff[j++] << 8);
									databuf[k++] |= (SaveRs485s[index].MainBox.ExpendBox[ExpId].DataBuff[j++] << 0);
									j += 2;
								}	
								SaveRs485s[index].MainBox.ExpendBox[ExpId].SensorToLen -= 4; ///���˵�4�ֽڣ��������ͱ�ʾ
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
					//û���ҵ�
					SaveRs485s[index].MainBox.ExpendBox[ExpId].ExpendBoxLive = false;				
					DEBUG_ERROR(3,"ExpendBox address = %d not in array",repbuff[3]);	
				}
			}			
		}
	}	
		
	//�ر� ��չ��
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
 *	SensorMaBoxData:		��ȡ����Rs485����������
 *	������			  			����485ID
 *	����ֵ��						��ѯ״̬���ɹ�/ʧ��
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
	//�ȴ��������,				
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
		
		///��ȡ����ʱĳ���ӿڴ������쳣���������ܳ������㣬��Ϊʶ���ʶ
		SaveRs485s[id].MainBox.SensorToLen = 0; ///�������ܳ���		
		Sensors.Counter ++;
		return HAL_ERROR;
	}
	
	SaveRs485s[id].MainBox.SensorToLen = CheckRs485s[SaveRs485s[id].MainBox.CheckIndex].SensorToLen; ///�������ܳ���		
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
//		if(CheckRs485s[SaveRs485s[id].MainBox.CheckIndex].SensorToLen == RS485_IDE_LEN+4)  //DO���ݴ���
		if(len == 9)  //DO���ݴ���
		{
			for( uint8_t k = 0, j = 3; j < len-2; )
			{
				databuf[k] 		= (SaveRs485s[id].MainBox.DataBuff[j++] << 8);
				databuf[k++] |= (SaveRs485s[id].MainBox.DataBuff[j++] << 0);
			}	
		}
		else if(len == 13)  ///RDO���ݴ���
		{
			for( uint8_t k = 0, j = 3; j < len-4; )
			{
				databuf[k] 		= (SaveRs485s[id].MainBox.DataBuff[j++] << 8);
				databuf[k++] |= (SaveRs485s[id].MainBox.DataBuff[j++] << 0);
				j += 2;
			}	
			
//			CheckRs485s[SaveRs485s[id].MainBox.CheckIndex].SensorToLen -= 4; ///���˵�4�ֽڣ��������ͱ�ʾ
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
 *	SensorExpenSigle:		��ȡ��չ��Rs485
 *	������			  			����485ID, ָ����չ�нӿ�
 *	����ֵ��						��ѯ״̬���ɹ�/ʧ��
 */
static HAL_StatusTypeDef SensorExpenSigle(uint8_t index, uint8_t Exid)
{
	uint8_t len = 0;
	
	uint32_t startTime = 0;
	uint32_t getDataTimeOut = 4000;
	
	uint8_t main_box = index;
	uint8_t ex_box   = Exid;
	
	main_box ++;	

	/***********************�ж϶�Ӧ485�ӿ��Ƿ���봫����***********************************/
	if( SaveRs485s[index].MainBox.ExpendBox[Exid].ExpenCheck ) 
	{					
		ex_box ++;
		/***********************����չ�нӿ�״̬***********************************/
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
		//�ȴ��������,				
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
				
//				if(CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].SensorToLen == RS485_IDE_LEN+4)  //DO���ݴ���
				
				if(len == 9)//DO���ݴ��� 
				{
					for( uint8_t k = 0, j = 3; j < len-2; )
					{
						databuf[k] 		= (SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff[j++] << 8);
						databuf[k++] |= (SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff[j++] << 0);
					}	
				}
				else if(len == 13) ///RDO���ݴ���
				{
					for( uint8_t k = 0, j = 3; j < len-4; )
					{
						databuf[k] 		= (SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff[j++] << 8);
						databuf[k++] |= (SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff[j++] << 0);
						j += 2;
					}	
										
//					CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].SensorToLen -= 4; ///���˵�4�ֽڣ��������ͱ�ʾ
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
	
	//�ر� ��չ��
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
 *	SensorExpenData:		��ȡ��չ��Rs485����������
 *	������			  			����485ID
 *	����ֵ��						��ѯ״̬���ɹ�/ʧ��
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
		/***********************�ж϶�Ӧ485�ӿ��Ƿ���봫����***********************************/
		if( SaveRs485s[index].MainBox.ExpendBox[Exid].ExpenCheck ) 
		{								
			/***********************����չ�нӿ�״̬***********************************/
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
			//�ȴ��������,				
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
					
//					if(CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].SensorToLen == RS485_IDE_LEN+4)  //DO���ݴ���
					if(len == 9) //DO���ݴ���
					{
						for( uint8_t k = 0, j = 3; j < len-2; )
						{
							databuf[k] 		= (SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff[j++] << 8);
							databuf[k++] |= (SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff[j++] << 0);
						}	
					}
					else if(len == 13) ///RDO���ݴ���
					{
						for( uint8_t k = 0, j = 3; j < len-4; )
						{
							databuf[k] 		= (SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff[j++] << 8);
							databuf[k++] |= (SaveRs485s[index].MainBox.ExpendBox[Exid].DataBuff[j++] << 0);
							j += 2;
						}	
//						CheckRs485s[SaveRs485s[index].MainBox.ExpendBox[Exid].CheckIndex].SensorToLen -= 4; ///���˵�4�ֽڣ��������ͱ�ʾ
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
	
	//�ر� ��չ��
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
*OpenExpenBox������չ�нӿ�
*ExpId��			 ��չ��ID
*���أ�				 ��չ�з������ݳ���
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
*RS485CmdPackage��Rs485��ѯ���������
*mode��			 			Rs485������
*���أ�				    ��
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


