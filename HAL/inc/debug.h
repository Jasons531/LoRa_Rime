/*
**************************************************************************************************************
*	@file		debug.h
*	@author Ysheng
*	@version 
*	@date    
*	@brief	debug
***************************************************************************************************************
*/
#ifndef __DEBUG_H
#define __DEBUG_H	 
#include <stdio.h>
#include <stdint.h>
#include "stm32l0xx_hal.h"
#include "usart.h"

#ifdef __cplusplus
	extern "C" {
#endif
			
#define DEBUG__						1
#define DEBUG_LEVEL	  		2					//���Եȼ������DEBUG���Ժ���Ƶ��������Χ,���ڸõȼ��ĵ��Բ����
		
		
#ifdef DEBUG__				  	//���Ժ궨��  
#include <stdio.h>
#include <string.h>  
		
#define NODEBUG		 3
		
#define APP        2

#define NORMAL     2

#define WARNING    2

#define ERROR      2

#define DEBUG(level, fmt, arg...)  if(level <= DEBUG_LEVEL)  printf(fmt,##arg); 	
	
#define DEBUG_APP(level, fmt, arg...)  if(level <= APP)	printf("App:"__FILE__",line : %d,"fmt"\r\n",__LINE__,##arg); 	
#define DEBUG_WARNING(level, fmt, arg...)  if(level <= WARNING)	printf("Warning:"__FILE__",line : %d,"fmt"\r\n",__LINE__,##arg); 
#define DEBUG_ERROR(level, fmt, arg...)  if(level <= ERROR)	printf("Error:"__FILE__",line : %d,"fmt"\r\n",__LINE__,##arg); 

#endif //end of DEBUG__							

#ifdef __cplusplus
}
#endif

#endif
