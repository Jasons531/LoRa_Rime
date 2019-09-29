/*
**************************************************************************************************************
*	@author 	Jason
*	@version 	V0.1
*	@date     2018/07/13
*	@brief		传感器采集
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
		
#define CHECKGETDATA						0		 ///上电查询地址，同时获取数据
		
#define RS485_IDE_LEN       			2     ///485接口ID+传感器标识占用长度

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
*Rs485接口类型标志位
*/
typedef enum u_Rstype
{
	RS485_NULL = 0,
	RS485_SIGNAL = 1,
	RS485_EXPAND_BOX = 2,
	RS485_NONE,
	
}Rstype_t;		
		
/*
*查询Rs485接口
*/
typedef struct u_CheckRs485
{
	/*
	*Rs485地址
	*/
	uint8_t	 		Addr;
	
	/*
	*Rs485解码标识
	*/
	uint8_t			Identifier;
	
	/*
	*Rs485传感器地址
	*/
	uint8_t  		RegAddress;
	
	/*
	*Rs485传感器参数个数
	*/
	uint8_t  		RegDatalen; 
	
	/*
	*发送查询长度
	*/
	uint8_t 		SendDataLen;
	
	/*
	*Rs485接收数据长度
	*/
	uint8_t 		RevDataLen;

	/*
	*传感器总长度: 扩展口ID+传感器解码标识+有效数据
	*/
	uint8_t 		SensorToLen;
	
	/*
	*Rs485传感器稳定时间
	*/
	uint32_t 		TimeOut;
	
	/*
	*Rs485查询发送缓存区
	*/
	uint8_t 		SendBuff[NBI_RS485_SEND_DATA_LEN];
	
	/*
	*Rs485接收数据缓存区
	*/
	uint8_t 		RevBuff[NBI_RS485_REV_DATA_LEN];	
	
	/*
	*Rs485传感器名称
	*/
	char 				Name[10];
}CheckRs485_t;

/*
*Rs485接口类型：扩展盒接口
*/
typedef struct u_ExInterfaceType
{
	/*
	*Rs485扩展盒接入标识：485扩展口5PIN才具有，4PIN不具备
	*/
	bool 				ExpendBoxLive;
	
	/*
	*Rs485扩展盒接入标识：兼容4PIN Rs485传感器扩展口不上报接口状态
	*/
	bool 				ExpenCheck;
	
	/*
	*Rs485接口ID：需要上报服务器
	*/
	uint8_t			Index;
	
	/*
	*记录查询到的Rs485所在列表下标
	*/
	uint8_t	 		CheckIndex;
	
	/*
	*Rs485解码标识：需要上报服务器
	*/
	uint8_t			Identifier;
		
	/*
	*传感器总长度: 扩展口ID+传感器解码标识+有效数据：作为分包依据
	*/
	uint8_t 		SensorToLen;
	
	/*
	*保存485接收数据
	*/
	uint8_t 		DataBuff[NBI_RS485_REV_DATA_LEN];
	
	/*
	*保存传感器数据
	*/
	uint8_t			SensorBuff[NBI_RS485_SEND_DATA_LEN];

}ExInterfaceType_t;

/*
*Rs485接口类型：主板接口
*/
typedef struct u_MainInterfaceType
{	
	/*
	*Rs485接口ID：需要上报服务器
	*/
	uint8_t							Index;
	
	/*
	*记录查询到的Rs485所在列表下标
	*/
	uint8_t	 						CheckIndex;
	
	/*
	*Rs485解码标识：需要上报服务器
	*/
	uint8_t							Identifier;
		
	/*
	*传感器总长度: 扩展口ID+传感器解码标识+有效数据：作为分包依据
	*/
	uint8_t 						SensorToLen;
	
	/*
	*保存485接收数据
	*/
	uint8_t 						DataBuff[NBI_RS485_REV_DATA_LEN];
	
	/*
	*保存传感器数据
	*/
	uint8_t							SensorBuff[NBI_RS485_SEND_DATA_LEN];
	
	/*
	*保存扩展盒信息
	*/
	ExInterfaceType_t		ExpendBox[6];

}MainInterfaceType_t;


/*
*只保存接口ID,解码标识，传感器地址，传感器总长度，传感器数据
*/
typedef struct u_SaveRs485
{
	/*
	*Rs485接口类型,MainBox才具有，当Type = RS485_EXPAND_BOX，ExpendBox才生效
	*/
	Rstype_t							Type;
	
	/*
	*保存主板485信息
	*/
	MainInterfaceType_t		MainBox;

}SaveRs485_t;

/*
*传感器数据处理
*/
typedef struct u_SendBuf
{
	uint8_t						Port[64];
	uint8_t						Buf[64];
	uint8_t						Len;
}SendBuf_t;

/*
*Sensor处理函数
*/
typedef struct u_Sensor
{
	/**************接入传感器总数量*****************/
	uint8_t						Counter;
	
	/**************设备工作场景选择：1：农场，2：水产*****************/
	uint8_t				    SceneSelection;

	/**************水产传感器标识，作为自动更改发送时间间隔*****************/
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
