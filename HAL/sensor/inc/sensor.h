/*
**************************************************************************************************************
*	@author 	Jason
*	@version 	V0.1
*	@date     2018/07/13
*	@brief		�������ɼ�
***************************************************************************************************************
*/
#ifndef __SENSOR_H
#define __SENSOR_H

#include <stdint.h>	
#include <stdbool.h>	
#include "rs485.h"
#include "power.h"

#ifdef __cplusplus
	extern "C" {
#endif	 
		
#define CHECKGETDATA						0		 ///�ϵ��ѯ��ַ��ͬʱ��ȡ����
		
#define RS485_IDE_LEN       			2     ///485�ӿ�ID+��������ʶռ�ó���

#define WATERSENSORNUMBER				3
		
#define GET_CRC(__X__,DATA)    	((__X__)[1] = ((DATA & 0xff00) >> 8), (__X__)[0] = (DATA & 0x00ff))
#define ARRAY(__X__)   				(sizeof(__X__)/sizeof(__X__[0]))
		

typedef enum
{
	EXPENDBOX = 0,	
	SWR = 1,
	PH,
	GH,
	ST_YMW,
	ST_YMS,
	ST_TW,
	ST_EC,
	ST_CO2,
	OXY,
	AnDan,			
	Water_EC,
	Water_T,
	Water_K,
	Air_Ill,
}enum_RS485_index;
		
/*
*Rs485�ӿ����ͱ�־λ
*/
typedef enum u_Rstype
{
	RS485_NULL = 0,
	RS485_SIGNAL = 1,
	RS485_EXPAND_BOX = 2,
	RS485_NONE,
	
}Rstype_t;		
		
/*
*��ѯRs485�ӿ�
*/
typedef struct u_CheckRs485
{
	/*
	*Rs485��ַ
	*/
	uint8_t	 		Addr;
	
	/*
	*Rs485�����ʶ
	*/
	uint8_t			Identifier;
	
	/*
	*Rs485��������ַ
	*/
	uint8_t  		RegAddress;
	
	/*
	*Rs485��������������
	*/
	uint8_t  		RegDatalen; 
	
	/*
	*���Ͳ�ѯ����
	*/
	uint8_t 		SendDataLen;
	
	/*
	*Rs485�������ݳ���
	*/
	uint8_t 		RevDataLen;

	/*
	*�������ܳ���: ��չ��ID+�����������ʶ+��Ч����
	*/
	uint8_t 		SensorToLen;
	
	/*
	*Rs485�������ȶ�ʱ��
	*/
	uint32_t 		TimeOut;
	
	/*
	*Rs485��ѯ���ͻ�����
	*/
	uint8_t 		SendBuff[NBI_RS485_SEND_DATA_LEN];
	
	/*
	*Rs485�������ݻ�����
	*/
	uint8_t 		RevBuff[NBI_RS485_REV_DATA_LEN];	
	
	/*
	*Rs485����������
	*/
	char 				Name[10];
}CheckRs485_t;

/*
*Rs485�ӿ����ͣ���չ�нӿ�
*/
typedef struct u_ExInterfaceType
{
	/*
	*Rs485��չ�н����ʶ��485��չ��5PIN�ž��У�4PIN���߱�
	*/
	bool 				ExpendBoxLive;
	
	/*
	*Rs485��չ�н����ʶ������4PIN Rs485��������չ�ڲ��ϱ��ӿ�״̬
	*/
	bool 				ExpenCheck;
	
	/*
	*Rs485�ӿ�ID����Ҫ�ϱ�������
	*/
	uint8_t			Index;
	
	/*
	*��¼��ѯ����Rs485�����б��±�
	*/
	uint8_t	 		CheckIndex;
	
	/*
	*Rs485�����ʶ����Ҫ�ϱ�������
	*/
	uint8_t			Identifier;
		
	/*
	*�������ܳ���: ��չ��ID+�����������ʶ+��Ч���ݣ���Ϊ�ְ�����
	*/
	uint8_t 		SensorToLen;
	
	/*
	*����485��������
	*/
	uint8_t 		DataBuff[NBI_RS485_REV_DATA_LEN];
	
	/*
	*���洫��������
	*/
	uint8_t			SensorBuff[NBI_RS485_SEND_DATA_LEN];

}ExInterfaceType_t;

/*
*Rs485�ӿ����ͣ�����ӿ�
*/
typedef struct u_MainInterfaceType
{	
	/*
	*Rs485�ӿ�ID����Ҫ�ϱ�������
	*/
	uint8_t							Index;
	
	/*
	*��¼��ѯ����Rs485�����б��±�
	*/
	uint8_t	 						CheckIndex;
	
	/*
	*Rs485�����ʶ����Ҫ�ϱ�������
	*/
	uint8_t							Identifier;
		
	/*
	*�������ܳ���: ��չ��ID+�����������ʶ+��Ч���ݣ���Ϊ�ְ�����
	*/
	uint8_t 						SensorToLen;
	
	/*
	*����485��������
	*/
	uint8_t 						DataBuff[NBI_RS485_REV_DATA_LEN];
	
	/*
	*���洫��������
	*/
	uint8_t							SensorBuff[NBI_RS485_SEND_DATA_LEN];
	
	/*
	*������չ����Ϣ
	*/
	ExInterfaceType_t		ExpendBox[6];

}MainInterfaceType_t;


/*
*ֻ����ӿ�ID,�����ʶ����������ַ���������ܳ��ȣ�����������
*/
typedef struct u_SaveRs485
{
	/*
	*Rs485�ӿ�����,MainBox�ž��У���Type = RS485_EXPAND_BOX��ExpendBox����Ч
	*/
	Rstype_t							Type;
	
	/*
	*��������485��Ϣ
	*/
	MainInterfaceType_t		MainBox;

}SaveRs485_t;

/*
*���������ݴ���
*/
typedef struct u_SendBuf
{
	uint8_t						Port[64];
	uint8_t						Buf[64];
	uint8_t						Len;
}SendBuf_t;

/*
*Sensor������
*/
typedef struct u_Sensor
{
	/**************���봫����������*****************/
	uint8_t						Counter;
	
	/**************�豸��������ѡ��1��ũ����2��ˮ��*****************/
	uint8_t				    SceneSelection;

	/**************ˮ����������ʶ����Ϊ�Զ����ķ���ʱ����*****************/
	bool 							WaterSensor;    
		
	void 							(*Handle)(void);
	
	void              (*CheckHandle)(void);
	
	void 							(*DataProces)(void);

	void 							(*ExpendBoxLive)(int expend_sensor,int index);
	
	Rstype_t 					(*GetRs485Type)(int index);

	Rstype_t 					(*QueryType)(int PortId);
	
	HAL_StatusTypeDef (*QueryPinStaus)(void);
	
	HAL_StatusTypeDef (*GetData)(int id);

	HAL_StatusTypeDef (*ExpBoxAddr)(int index);
	
	HAL_StatusTypeDef  (*ExpenSigle)(uint8_t index, uint8_t Exid);

	HAL_StatusTypeDef (*MaBoxData)(uint8_t id);

	HAL_StatusTypeDef (*ExpenData)(uint8_t index);
	
}Sensor_t;


extern CheckRs485_t CheckRs485s[];
extern SaveRs485_t  SaveRs485s[3];
extern SendBuf_t		SendBufs[10];
extern Sensor_t     Sensors;

extern volatile 	uint8_t SendBufsCounter;

void SensorsInit(void);

static void SensorHandle(void);

static void SensorCheckHandle(void);

static void SensorDataProces(void);

static HAL_StatusTypeDef SensorQueryPinStaus(void);

static Rstype_t GetRs485Type(int index);

static Rstype_t SensorQueryType(int PortId);

static HAL_StatusTypeDef SensorGetData(int id);

static void SensorExpendBoxLive(int expend_sensor,int index);

static HAL_StatusTypeDef SensorExpBoxAddr(int index);

static HAL_StatusTypeDef SensorExpenSigle(uint8_t index, uint8_t Exid);

static HAL_StatusTypeDef SensorMaBoxData(uint8_t id);

static HAL_StatusTypeDef SensorExpenData(uint8_t index);

static uint8_t OpenExpenBox(uint8_t ExpId);

static void RS485CmdPackage(char mode);

#ifdef __cplusplus
}
#endif

#endif
