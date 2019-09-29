
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

nmea_msg gpsx; 											//GPS��Ϣ

void GpsInit(void)
{	
	GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_GPIOB_CLK_ENABLE();           //����GPIOBʱ��

	GPIO_Initure.Pin=GPS_Power_ON;  
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //�������
	GPIO_Initure.Pull=GPIO_PULLUP;          //����
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //����
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
	__HAL_RCC_GPIOB_CLK_ENABLE();           //����GPIOBʱ��

	GPIO_Initure.Pin=GPS_Power_ON;  
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_OD;  //�������
	GPIO_Initure.Pull=GPIO_NOPULL;          //����
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //����
	HAL_GPIO_Init(GPS_IO,&GPIO_Initure);

	HAL_GPIO_WritePin(GPS_IO,GPS_Power_ON,GPIO_PIN_SET);
}


/*
*GpsSet: ����GPS
*���أ�  �ɹ���1  ʧ�ܣ�0
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
*GpsGetPosition: ��ȡGPS����,������
*���أ�					 �ɹ���1  ʧ�ܣ�0
*/
void GpsGetPosition(uint8_t *GpsBuf)
{	
	uint8_t len = 0;
	
	GpsBuf[len++] = 0XFD; //GPS	
 
	if(( SetGpsAck.GetPation == PATIONDONE ) && SetGpsAck.Posfix) ///�ж��Ƿ���GPS����ȡGPS��Ϣ  
	{	        		 
		NMEA_GPGLL_Analysis(&gpsx, (uint8_t *)SetGpsAck.GLL); ///��γ��  "$GPGLL,2233.1773,N,11356.7148,E,094100.210,A,A*5E\r\n"

		DEBUG(3,"11---%.5f %1c, %.5f %1c\r\n", (double)gpsx.latitude,gpsx.nshemi, (double)gpsx.longitude,gpsx.ewhemi);
		DEBUG(2,"22---%.5f %1c, %.5f %1c\r\n", (double)gpsx.latitude/100000,gpsx.nshemi, (double)gpsx.longitude/100000,gpsx.ewhemi);
		uint32_t temp[4] = {gpsx.latitude,gpsx.nshemi,gpsx.longitude,gpsx.ewhemi};

		uint8_t i = 0;
		
		for(; len < GPSLEN; i++)
		{
			if(i%2==1)
			{
				GpsBuf[len++] = (temp[i])&0xff;	///��γ�ַ�
			}					
			else
			{
				GpsBuf[len++] = (temp[i] >> 24)&0xff;
				GpsBuf[len++] = (temp[i] >> 16)&0xff;
				GpsBuf[len++] = (temp[i] >> 8)&0xff;
				GpsBuf[len++] = (temp[i])&0xff;			
			}						
		}					
		SetGpsAck.GpsDone = true; ///GPS������ɱ��	 
		gpsx.gpssta = 1;
		SetGpsAck.Len = len;

		SetLedStates(NoneCare);
		
		HAL_TIM_Base_Stop_IT(&htim2);
				
	} 
	else if(((HAL_GetTick( ) - SetGpsAck.GpsOverTime) > 300000) && SetGpsAck.Posfix && (SetGpsAck.GetPation == PATIONNULL))  ///GPS 5�����ڶ�λʧ�ܣ�Ĭ��GPS�쳣���ٶ�λ 300000
 {	 
		DEBUG(2,"GPS_TIME22 : %d\r\n",HAL_GetTick( ) - SetGpsAck.GpsOverTime);
		 
		SetGpsAck.GetPation = PATIONFAIL;

		Gps.Disable(  ); ///�ر�GPS
		 
		SetGpsAck.Start = false;

		memset(SetGpsAck.GLL, 0, strlen(SetGpsAck.GLL));
	 
		SetGpsAck.GpsDone = true; ///GPS������ɱ��	 
		gpsx.gpssta = 0;	
		
		GpsBuf[len++] = 0x01;
	 
		SetGpsAck.Len = len;
	 	 
		SetLedStates(NoneCare);
	 	HAL_TIM_Base_Stop_IT(&htim2);
	}
}

/*
*GpsGetPositionAgain: GPS�ٴζ�λ
*���أ�					 �ɹ���1  ʧ�ܣ�0
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

const uint32_t BAUD_id[9]={4800,9600,19200,38400,57600,115200,230400,460800,921600};//ģ��֧�ֲ���������
//��buf����õ���cx���������ڵ�λ��
//����ֵ:0~0XFE,����������λ�õ�ƫ��.
//       0XFF,�������ڵ�cx������							  
uint8_t NMEA_Comma_Pos(uint8_t *buf,uint8_t cx)
{	 		    
	uint8_t *p=buf;
	while(cx)
	{		 
		if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//����'*'���߷Ƿ��ַ�,�򲻴��ڵ�cx������
		if(*buf==',')cx--;
		buf++;
	}
	return buf-p;	 
}
//m^n����
//����ֵ:m^n�η�.
uint32_t NMEA_Pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}
//strת��Ϊ����,��','����'*'����
//buf:���ִ洢��
//dx:С����λ��,���ظ����ú���
//����ֵ:ת�������ֵ
int NMEA_Str2num(uint8_t *buf,uint8_t*dx)
{
	uint8_t *p=buf;
	uint32_t ires=0,fres=0;
	uint8_t ilen=0,flen=0,i;
	uint8_t mask=0;
	int res;
	while(1) //�õ�������С���ĳ���
	{
		if(*p=='-'){mask|=0X02;p++;}//�Ǹ���
		if(*p==','||(*p=='*'))break;//����������
		if(*p=='.'){mask|=0X01;p++;}//����С������
		else if(*p>'9'||(*p<'0'))	//�зǷ��ַ�
		{	
			ilen=0;
			flen=0;
			break;
		}	
		if(mask&0X01)flen++;
		else ilen++;
		p++;
	}
	if(mask&0X02)buf++;	//ȥ������
	for(i=0;i<ilen;i++)	//�õ�������������
	{  
		ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	}
	if(flen>5)flen=5;	//���ȡ5λС��
	*dx=flen;	 		//С����λ��
	for(i=0;i<flen;i++)	//�õ�С����������
	{  
		fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
	} 
	res=ires*NMEA_Pow(10,flen)+fres;
	if(mask&0X02)res=-res;		   
	return res;
}	  							 
//����GPGSV��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GPGSV_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p,*p1,dx;
	uint8_t len,i,j,slx=0;
	uint8_t posx;   	 
	p=buf;
	p1=(uint8_t*)strstr((const char *)p,"$GPGSV");
	len=p1[7]-'0';								//�õ�GPGSV������
	posx=NMEA_Comma_Pos(p1,3); 					//�õ��ɼ���������
	if(posx!=0XFF)gpsx->svnum=NMEA_Str2num(p1+posx,&dx);
	for(i=0;i<len;i++)
	{	 
		p1=(uint8_t*)strstr((const char *)p,"$GPGSV");  
		for(j=0;j<4;j++)
		{	  
			posx=NMEA_Comma_Pos(p1,4+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].num=NMEA_Str2num(p1+posx,&dx);	//�õ����Ǳ��
			else break; 
			posx=NMEA_Comma_Pos(p1,5+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].eledeg=NMEA_Str2num(p1+posx,&dx);//�õ��������� 
			else break;
			posx=NMEA_Comma_Pos(p1,6+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].azideg=NMEA_Str2num(p1+posx,&dx);//�õ����Ƿ�λ��
			else break; 
			posx=NMEA_Comma_Pos(p1,7+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].sn=NMEA_Str2num(p1+posx,&dx);	//�õ����������
			else break;
			slx++;	   
		}   
 		p=p1+1;//�л�����һ��GPGSV��Ϣ
	}   
}
//����BDGSV��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_BDGSV_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p,*p1,dx;
	uint8_t len,i,j,slx=0;
	uint8_t posx;   	 
	p=buf;
	p1=(uint8_t*)strstr((const char *)p,"$BDGSV");
	len=p1[7]-'0';								//�õ�BDGSV������
	posx=NMEA_Comma_Pos(p1,3); 					//�õ��ɼ�������������
	if(posx!=0XFF)gpsx->beidou_svnum=NMEA_Str2num(p1+posx,&dx);
	for(i=0;i<len;i++)
	{	 
		p1=(uint8_t*)strstr((const char *)p,"$BDGSV");  
		for(j=0;j<4;j++)
		{	  
			posx=NMEA_Comma_Pos(p1,4+j*4);
			if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_num=NMEA_Str2num(p1+posx,&dx);	//�õ����Ǳ��
			else break; 
			posx=NMEA_Comma_Pos(p1,5+j*4);
			if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_eledeg=NMEA_Str2num(p1+posx,&dx);//�õ��������� 
			else break;
			posx=NMEA_Comma_Pos(p1,6+j*4);
			if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_azideg=NMEA_Str2num(p1+posx,&dx);//�õ����Ƿ�λ��
			else break; 
			posx=NMEA_Comma_Pos(p1,7+j*4);
			if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_sn=NMEA_Str2num(p1+posx,&dx);	//�õ����������
			else break;
			slx++;	   
		}   
 		p=p1+1;//�л�����һ��BDGSV��Ϣ
	}   
}
//����GNGGA��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GNGGA_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;			 
	uint8_t posx;    
	p1=(uint8_t*)strstr((const char *)buf,"$GNGGA");
	posx=NMEA_Comma_Pos(p1,6);								//�õ�GPS״̬
	if(posx!=0XFF)gpsx->gpssta=NMEA_Str2num(p1+posx,&dx);	
	posx=NMEA_Comma_Pos(p1,7);								//�õ����ڶ�λ��������
	if(posx!=0XFF)gpsx->posslnum=NMEA_Str2num(p1+posx,&dx); 
	posx=NMEA_Comma_Pos(p1,9);								//�õ����θ߶�
	if(posx!=0XFF)gpsx->altitude=NMEA_Str2num(p1+posx,&dx);  
}
//����GNGSA��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GNGSA_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;			 
	uint8_t posx; 
	uint8_t i;   
	p1=(uint8_t*)strstr((const char *)buf,"$GNGSA");
	posx=NMEA_Comma_Pos(p1,2);								//�õ���λ����
	if(posx!=0XFF)gpsx->fixmode=NMEA_Str2num(p1+posx,&dx);	
	for(i=0;i<12;i++)										//�õ���λ���Ǳ��
	{
		posx=NMEA_Comma_Pos(p1,3+i);					 
		if(posx!=0XFF)gpsx->possl[i]=NMEA_Str2num(p1+posx,&dx);
		else break; 
	}				  
	posx=NMEA_Comma_Pos(p1,15);								//�õ�PDOPλ�þ�������
	if(posx!=0XFF)gpsx->pdop=NMEA_Str2num(p1+posx,&dx);  
	posx=NMEA_Comma_Pos(p1,16);								//�õ�HDOPλ�þ�������
	if(posx!=0XFF)gpsx->hdop=NMEA_Str2num(p1+posx,&dx);  
	posx=NMEA_Comma_Pos(p1,17);								//�õ�VDOPλ�þ�������
	if(posx!=0XFF)gpsx->vdop=NMEA_Str2num(p1+posx,&dx);  
}
//����GNRMC��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GNRMC_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;			 
	uint8_t posx;     
	uint32_t temp;	   
	float rs;  
	p1=(uint8_t*)strstr((const char *)buf,"$GNRMC");//"$GNRMC",������&��GNRMC�ֿ������,��ֻ�ж�GPRMC.
	posx=NMEA_Comma_Pos(p1,1);								//�õ�UTCʱ��
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//�õ�UTCʱ��,ȥ��ms
		gpsx->utc.hour=temp/10000;
		gpsx->utc.min=(temp/100)%100;
		gpsx->utc.sec=temp%100;	 	 
	}	
	posx=NMEA_Comma_Pos(p1,3);								//�õ�γ��
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//�õ���
		rs=temp%NMEA_Pow(10,dx+2);				//�õ�'		 
		gpsx->latitude=gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//ת��Ϊ�� 
	}
	posx=NMEA_Comma_Pos(p1,4);								//��γ���Ǳ�γ 
	if(posx!=0XFF)gpsx->nshemi=*(p1+posx);					 
 	posx=NMEA_Comma_Pos(p1,5);								//�õ�����
	if(posx!=0XFF)
	{												  
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//�õ���
		rs=temp%NMEA_Pow(10,dx+2);				//�õ�'		 
		gpsx->longitude=gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//ת��Ϊ�� 
	}
	posx=NMEA_Comma_Pos(p1,6);								//������������
	if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);		 
	posx=NMEA_Comma_Pos(p1,9);								//�õ�UTC����
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 				//�õ�UTC����
		gpsx->utc.date=temp/10000;
		gpsx->utc.month=(temp/100)%100;
		gpsx->utc.year=2000+temp%100;	 	 
	} 
}

//����GPGLL��Ϣ   $GPGLL,2233.1493,N,11356.6989,E,072246.000,A,A*5D
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GPGLL_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;			 
	uint8_t posx;     
	uint32_t temp;	   
	float rs;  
	p1=(uint8_t*)strstr((const char *)buf,"$GPGLL");//"$GPGLL",������&��GNRMC�ֿ������,��ֻ�ж�GPGLL.
	
	posx=NMEA_Comma_Pos(p1,1);								//�õ�γ��
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//�õ���
		rs=temp%NMEA_Pow(10,dx+2);				//�õ�'		 
		gpsx->latitude=gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//ת��Ϊ�� 
	}
	posx=NMEA_Comma_Pos(p1,2);								//��γ���Ǳ�γ 
	
	if(posx!=0XFF)gpsx->nshemi=*(p1+posx);	
	
 	posx=NMEA_Comma_Pos(p1,3);								//�õ�����
	if(posx!=0XFF)
	{												  
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//�õ���
		rs=temp%NMEA_Pow(10,dx+2);				//�õ�'		 
		gpsx->longitude=gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//ת��Ϊ�� 
	}
	posx=NMEA_Comma_Pos(p1,4);								//������������
	if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);		

	posx=NMEA_Comma_Pos(p1,5);								//�õ�UTCʱ��
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//�õ�UTCʱ��,ȥ��ms
		gpsx->utc.hour=temp/10000;
		gpsx->utc.min=(temp/100)%100;
		gpsx->utc.sec=temp%100;	 	 
	}		
}

//����GNVTG��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GNVTG_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;			 
	uint8_t posx;    
	p1=(uint8_t*)strstr((const char *)buf,"$GNVTG");							 
	posx=NMEA_Comma_Pos(p1,7);								//�õ���������
	if(posx!=0XFF)
	{
		gpsx->speed=NMEA_Str2num(p1+posx,&dx);
		if(dx<3)gpsx->speed*=NMEA_Pow(10,3-dx);	 	 		//ȷ������1000��
	}
}  
//��ȡNMEA-0183��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void GPS_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	NMEA_GPGSV_Analysis(gpsx,buf);	//GPGSV����
	NMEA_BDGSV_Analysis(gpsx,buf);	//BDGSV����
	NMEA_GNGGA_Analysis(gpsx,buf);	//GNGGA���� 	
	NMEA_GNGSA_Analysis(gpsx,buf);	//GPNSA����
	NMEA_GNRMC_Analysis(gpsx,buf);	//GPNMC����
	NMEA_GNVTG_Analysis(gpsx,buf);	//GPNTG����
}
