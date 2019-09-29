
#include "gps.h" 					   
#include "stdio.h"	 
#include "stdarg.h"	 
#include "string.h"	 
#include "math.h" 
#include "gpio.h" 
#include "led.h" 
#include "stm32l0xx_hal.h"


#define MTK_COULD			"$PMTK103*30\r\n"
#define MTK_HOST       	"$PMTK101*32\r\n"

#define MTK_POS_FIX		"$PMTK220,1000*1F\r\n" //  $PMTK220,3000*1D
#define MTK_GLL			"$PMTK314,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0*28\r\n"

SetGpsAck_t SetGpsAck = {false, false, false, false, false, {0}, PATIONNULL, 0, {0}, 0, 0, 0};

const Gps_t Gps = {GpsInit, GpsEnable, GpsDisable, GpsSet, GpsGetPosition, GpsGetPositionAgain};

nmea_msg gpsx; 											//GPS信息

void GpsInit(void)
{	
	GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_GPIOB_CLK_ENABLE();           //开启GPIOB时钟

	GPIO_Initure.Pin=GPS_Power_ON;  
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
	GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
	HAL_GPIO_Init(GPS_IO,&GPIO_Initure);
	
	MX_USART2_UART_Init(  ); 
}

void GpsEnable(void)
{
	HAL_GPIO_WritePin(GPS_IO,GPS_Power_ON,GPIO_PIN_SET);
}

void GpsDisable(void)
{
  GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_GPIOB_CLK_ENABLE();           //开启GPIOB时钟

	GPIO_Initure.Pin=GPS_Power_ON;  
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_OD;  //推挽输出
	GPIO_Initure.Pull=GPIO_NOPULL;          //上拉
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
	HAL_GPIO_Init(GPS_IO,&GPIO_Initure);

	HAL_GPIO_WritePin(GPS_IO,GPS_Power_ON,GPIO_PIN_SET);
}


/*
*GpsSet: 配置GPS
*返回：  成功：1  失败：0
*/
uint8_t GpsSet(void)
{
	Gps.Enable( );  
	uint32_t GPS_TIME = 0;
	
	do
	{	
		HAL_NVIC_DisableIRQ(USART2_IRQn);
		HAL_UART_Transmit(&huart2, (uint8_t *)MTK_COULD, sizeof(MTK_COULD), 0xFFFFFFFF);
		HAL_NVIC_EnableIRQ(USART2_IRQn);
		HAL_Delay(1000);	
		GPS_TIME ++;
		DEBUG(2,"line = %d\r\n",__LINE__);
	}
	while(!SetGpsAck.Start && ( GPS_TIME <= 10 ));
    
	SetGpsAck.GetPationTime = HAL_GetTick();

	if(!SetGpsAck.Start && ( GPS_TIME > 10 ))
	{
		DEBUG_ERROR(2,"Hardware_Exist_GPS ERROR\r\n");
		return 0;
	}	
	if(SetGpsAck.Start)
	{
		do
		{
			HAL_NVIC_DisableIRQ(USART2_IRQn);
			HAL_UART_Transmit(&huart2, (uint8_t *)MTK_GLL, sizeof(MTK_GLL), 0xFFFFFFFF);
			HAL_NVIC_EnableIRQ(USART2_IRQn);		
			HAL_Delay(200);	
			DEBUG(3,"line = %d\r\n",__LINE__);
		}while(!SetGpsAck.Gpll);
		
		do
		{
			SetGpsAck.Start = false;
			SetGpsAck.Gpll = false;
			HAL_NVIC_DisableIRQ(USART2_IRQn);
			HAL_UART_Transmit(&huart2, (uint8_t *)MTK_POS_FIX, sizeof(MTK_POS_FIX), 0xFFFFFFFF);
			HAL_NVIC_EnableIRQ(USART2_IRQn);	
			HAL_Delay(200);	
			DEBUG(3,"line = %d\r\n",__LINE__);
		}while(!SetGpsAck.Posfix);
				
		return 1;
	}
	
	return 0;
}

/*
*GpsGetPosition: 获取GPS数据,并发送
*返回：					 成功：1  失败：0
*/
void GpsGetPosition(uint8_t *GpsBuf)
{	
	uint8_t len = 0;
	
	GpsBuf[len++] = 0XFD; //GPS	
 
	if(( SetGpsAck.GetPation == PATIONDONE ) && SetGpsAck.Posfix) ///判断是否有GPS、获取GPS信息  
	{	        		 
		NMEA_GPGLL_Analysis(&gpsx, (uint8_t *)SetGpsAck.GLL); ///经纬度  "$GPGLL,2233.1773,N,11356.7148,E,094100.210,A,A*5E\r\n"

		DEBUG(3,"11---%.5f %1c, %.5f %1c\r\n", (double)gpsx.latitude,gpsx.nshemi, (double)gpsx.longitude,gpsx.ewhemi);
		DEBUG(2,"22---%.5f %1c, %.5f %1c\r\n", (double)gpsx.latitude/100000,gpsx.nshemi, (double)gpsx.longitude/100000,gpsx.ewhemi);
		uint32_t temp[4] = {gpsx.latitude,gpsx.nshemi,gpsx.longitude,gpsx.ewhemi};

		uint8_t i = 0;
		
		for(; len < GPSLEN; i++)
		{
			if(i%2==1)
			{
				GpsBuf[len++] = (temp[i])&0xff;	///经纬字符
			}					
			else
			{
				GpsBuf[len++] = (temp[i] >> 24)&0xff;
				GpsBuf[len++] = (temp[i] >> 16)&0xff;
				GpsBuf[len++] = (temp[i] >> 8)&0xff;
				GpsBuf[len++] = (temp[i])&0xff;			
			}						
		}					
		SetGpsAck.GpsDone = true; ///GPS发送完成标记	 
		gpsx.gpssta = 1;
		SetGpsAck.Len = len;

		SetLedStates(NoneCare);
		
		HAL_TIM_Base_Stop_IT(&htim2);
				
	} 
	else if(((HAL_GetTick( ) - SetGpsAck.GpsOverTime) > 300000) && SetGpsAck.Posfix && (SetGpsAck.GetPation == PATIONNULL))  ///GPS 5分钟内定位失败，默认GPS异常不再定位 300000
 {	 
		DEBUG(2,"GPS_TIME22 : %d\r\n",HAL_GetTick( ) - SetGpsAck.GpsOverTime);
		 
		SetGpsAck.GetPation = PATIONFAIL;

		Gps.Disable(  ); ///关闭GPS
		 
		SetGpsAck.Start = false;

		memset(SetGpsAck.GLL, 0, strlen(SetGpsAck.GLL));
	 
		SetGpsAck.GpsDone = true; ///GPS发送完成标记	 
		gpsx.gpssta = 0;	
		
		GpsBuf[len++] = 0x01;
	 
		SetGpsAck.Len = len;
	 	 
		SetLedStates(NoneCare);
	 	HAL_TIM_Base_Stop_IT(&htim2);
	}
}

/*
*GpsGetPositionAgain: GPS再次定位
*返回：					 成功：1  失败：0
*/
void GpsGetPositionAgain(void)
{
	DEBUG_APP(2,"%s",__func__);

	SetGpsAck.Start = false;
	SetGpsAck.Posfix = false;
	SetGpsAck.Gpll	= false;      

	Gps.Init(  );
	
	HAL_TIM_Base_Start_IT(&htim2);

	Gps.Set(  );	
	
	SetLedStates(GpsLocation);

	SetGpsAck.GpsOverTime = HAL_GetTick( );
}

const uint32_t BAUD_id[9]={4800,9600,19200,38400,57600,115200,230400,460800,921600};//模块支持波特率数组
//从buf里面得到第cx个逗号所在的位置
//返回值:0~0XFE,代表逗号所在位置的偏移.
//       0XFF,代表不存在第cx个逗号							  
uint8_t NMEA_Comma_Pos(uint8_t *buf,uint8_t cx)
{	 		    
	uint8_t *p=buf;
	while(cx)
	{		 
		if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//遇到'*'或者非法字符,则不存在第cx个逗号
		if(*buf==',')cx--;
		buf++;
	}
	return buf-p;	 
}
//m^n函数
//返回值:m^n次方.
uint32_t NMEA_Pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}
//str转换为数字,以','或者'*'结束
//buf:数字存储区
//dx:小数点位数,返回给调用函数
//返回值:转换后的数值
int NMEA_Str2num(uint8_t *buf,uint8_t*dx)
{
	uint8_t *p=buf;
	uint32_t ires=0,fres=0;
	uint8_t ilen=0,flen=0,i;
	uint8_t mask=0;
	int res;
	while(1) //得到整数和小数的长度
	{
		if(*p=='-'){mask|=0X02;p++;}//是负数
		if(*p==','||(*p=='*'))break;//遇到结束了
		if(*p=='.'){mask|=0X01;p++;}//遇到小数点了
		else if(*p>'9'||(*p<'0'))	//有非法字符
		{	
			ilen=0;
			flen=0;
			break;
		}	
		if(mask&0X01)flen++;
		else ilen++;
		p++;
	}
	if(mask&0X02)buf++;	//去掉负号
	for(i=0;i<ilen;i++)	//得到整数部分数据
	{  
		ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	}
	if(flen>5)flen=5;	//最多取5位小数
	*dx=flen;	 		//小数点位数
	for(i=0;i<flen;i++)	//得到小数部分数据
	{  
		fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
	} 
	res=ires*NMEA_Pow(10,flen)+fres;
	if(mask&0X02)res=-res;		   
	return res;
}	  							 
//分析GPGSV信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPGSV_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p,*p1,dx;
	uint8_t len,i,j,slx=0;
	uint8_t posx;   	 
	p=buf;
	p1=(uint8_t*)strstr((const char *)p,"$GPGSV");
	len=p1[7]-'0';								//得到GPGSV的条数
	posx=NMEA_Comma_Pos(p1,3); 					//得到可见卫星总数
	if(posx!=0XFF)gpsx->svnum=NMEA_Str2num(p1+posx,&dx);
	for(i=0;i<len;i++)
	{	 
		p1=(uint8_t*)strstr((const char *)p,"$GPGSV");  
		for(j=0;j<4;j++)
		{	  
			posx=NMEA_Comma_Pos(p1,4+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].num=NMEA_Str2num(p1+posx,&dx);	//得到卫星编号
			else break; 
			posx=NMEA_Comma_Pos(p1,5+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].eledeg=NMEA_Str2num(p1+posx,&dx);//得到卫星仰角 
			else break;
			posx=NMEA_Comma_Pos(p1,6+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].azideg=NMEA_Str2num(p1+posx,&dx);//得到卫星方位角
			else break; 
			posx=NMEA_Comma_Pos(p1,7+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].sn=NMEA_Str2num(p1+posx,&dx);	//得到卫星信噪比
			else break;
			slx++;	   
		}   
 		p=p1+1;//切换到下一个GPGSV信息
	}   
}
//分析BDGSV信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_BDGSV_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p,*p1,dx;
	uint8_t len,i,j,slx=0;
	uint8_t posx;   	 
	p=buf;
	p1=(uint8_t*)strstr((const char *)p,"$BDGSV");
	len=p1[7]-'0';								//得到BDGSV的条数
	posx=NMEA_Comma_Pos(p1,3); 					//得到可见北斗卫星总数
	if(posx!=0XFF)gpsx->beidou_svnum=NMEA_Str2num(p1+posx,&dx);
	for(i=0;i<len;i++)
	{	 
		p1=(uint8_t*)strstr((const char *)p,"$BDGSV");  
		for(j=0;j<4;j++)
		{	  
			posx=NMEA_Comma_Pos(p1,4+j*4);
			if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_num=NMEA_Str2num(p1+posx,&dx);	//得到卫星编号
			else break; 
			posx=NMEA_Comma_Pos(p1,5+j*4);
			if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_eledeg=NMEA_Str2num(p1+posx,&dx);//得到卫星仰角 
			else break;
			posx=NMEA_Comma_Pos(p1,6+j*4);
			if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_azideg=NMEA_Str2num(p1+posx,&dx);//得到卫星方位角
			else break; 
			posx=NMEA_Comma_Pos(p1,7+j*4);
			if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_sn=NMEA_Str2num(p1+posx,&dx);	//得到卫星信噪比
			else break;
			slx++;	   
		}   
 		p=p1+1;//切换到下一个BDGSV信息
	}   
}
//分析GNGGA信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GNGGA_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;			 
	uint8_t posx;    
	p1=(uint8_t*)strstr((const char *)buf,"$GNGGA");
	posx=NMEA_Comma_Pos(p1,6);								//得到GPS状态
	if(posx!=0XFF)gpsx->gpssta=NMEA_Str2num(p1+posx,&dx);	
	posx=NMEA_Comma_Pos(p1,7);								//得到用于定位的卫星数
	if(posx!=0XFF)gpsx->posslnum=NMEA_Str2num(p1+posx,&dx); 
	posx=NMEA_Comma_Pos(p1,9);								//得到海拔高度
	if(posx!=0XFF)gpsx->altitude=NMEA_Str2num(p1+posx,&dx);  
}
//分析GNGSA信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GNGSA_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;			 
	uint8_t posx; 
	uint8_t i;   
	p1=(uint8_t*)strstr((const char *)buf,"$GNGSA");
	posx=NMEA_Comma_Pos(p1,2);								//得到定位类型
	if(posx!=0XFF)gpsx->fixmode=NMEA_Str2num(p1+posx,&dx);	
	for(i=0;i<12;i++)										//得到定位卫星编号
	{
		posx=NMEA_Comma_Pos(p1,3+i);					 
		if(posx!=0XFF)gpsx->possl[i]=NMEA_Str2num(p1+posx,&dx);
		else break; 
	}				  
	posx=NMEA_Comma_Pos(p1,15);								//得到PDOP位置精度因子
	if(posx!=0XFF)gpsx->pdop=NMEA_Str2num(p1+posx,&dx);  
	posx=NMEA_Comma_Pos(p1,16);								//得到HDOP位置精度因子
	if(posx!=0XFF)gpsx->hdop=NMEA_Str2num(p1+posx,&dx);  
	posx=NMEA_Comma_Pos(p1,17);								//得到VDOP位置精度因子
	if(posx!=0XFF)gpsx->vdop=NMEA_Str2num(p1+posx,&dx);  
}
//分析GNRMC信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GNRMC_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;			 
	uint8_t posx;     
	uint32_t temp;	   
	float rs;  
	p1=(uint8_t*)strstr((const char *)buf,"$GNRMC");//"$GNRMC",经常有&和GNRMC分开的情况,故只判断GPRMC.
	posx=NMEA_Comma_Pos(p1,1);								//得到UTC时间
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//得到UTC时间,去掉ms
		gpsx->utc.hour=temp/10000;
		gpsx->utc.min=(temp/100)%100;
		gpsx->utc.sec=temp%100;	 	 
	}	
	posx=NMEA_Comma_Pos(p1,3);								//得到纬度
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//得到°
		rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
		gpsx->latitude=gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//转换为° 
	}
	posx=NMEA_Comma_Pos(p1,4);								//南纬还是北纬 
	if(posx!=0XFF)gpsx->nshemi=*(p1+posx);					 
 	posx=NMEA_Comma_Pos(p1,5);								//得到经度
	if(posx!=0XFF)
	{												  
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//得到°
		rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
		gpsx->longitude=gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//转换为° 
	}
	posx=NMEA_Comma_Pos(p1,6);								//东经还是西经
	if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);		 
	posx=NMEA_Comma_Pos(p1,9);								//得到UTC日期
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 				//得到UTC日期
		gpsx->utc.date=temp/10000;
		gpsx->utc.month=(temp/100)%100;
		gpsx->utc.year=2000+temp%100;	 	 
	} 
}

//分析GPGLL信息   $GPGLL,2233.1493,N,11356.6989,E,072246.000,A,A*5D
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPGLL_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;			 
	uint8_t posx;     
	uint32_t temp;	   
	float rs;  
	p1=(uint8_t*)strstr((const char *)buf,"$GPGLL");//"$GPGLL",经常有&和GNRMC分开的情况,故只判断GPGLL.
	
	posx=NMEA_Comma_Pos(p1,1);								//得到纬度
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//得到°
		rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
		gpsx->latitude=gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//转换为° 
	}
	posx=NMEA_Comma_Pos(p1,2);								//南纬还是北纬 
	
	if(posx!=0XFF)gpsx->nshemi=*(p1+posx);	
	
 	posx=NMEA_Comma_Pos(p1,3);								//得到经度
	if(posx!=0XFF)
	{												  
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//得到°
		rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
		gpsx->longitude=gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//转换为° 
	}
	posx=NMEA_Comma_Pos(p1,4);								//东经还是西经
	if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);		

	posx=NMEA_Comma_Pos(p1,5);								//得到UTC时间
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//得到UTC时间,去掉ms
		gpsx->utc.hour=temp/10000;
		gpsx->utc.min=(temp/100)%100;
		gpsx->utc.sec=temp%100;	 	 
	}		
}

//分析GNVTG信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GNVTG_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;			 
	uint8_t posx;    
	p1=(uint8_t*)strstr((const char *)buf,"$GNVTG");							 
	posx=NMEA_Comma_Pos(p1,7);								//得到地面速率
	if(posx!=0XFF)
	{
		gpsx->speed=NMEA_Str2num(p1+posx,&dx);
		if(dx<3)gpsx->speed*=NMEA_Pow(10,3-dx);	 	 		//确保扩大1000倍
	}
}  
//提取NMEA-0183信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void GPS_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	NMEA_GPGSV_Analysis(gpsx,buf);	//GPGSV解析
	NMEA_BDGSV_Analysis(gpsx,buf);	//BDGSV解析
	NMEA_GNGGA_Analysis(gpsx,buf);	//GNGGA解析 	
	NMEA_GNGSA_Analysis(gpsx,buf);	//GPNSA解析
	NMEA_GNRMC_Analysis(gpsx,buf);	//GPNMC解析
	NMEA_GNVTG_Analysis(gpsx,buf);	//GPNTG解析
}
