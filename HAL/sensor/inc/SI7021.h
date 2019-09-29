/*
**************************************************************************************************************
*	@file			SI7021.c
*	@author 	YSheng
*	@version 
*	@date    
*	@brief	
***************************************************************************************************************
*/
#ifndef __SI7021_H
#define __SI7021_H

#include <stdint.h>
#include "debug.h"
#include "sensor.h"

#ifdef __cplusplus
	extern "C" {
#endif

/*
 *	InitSI7021:		��ʼ��SI7021��ַ
 *	����ֵ: 			
 */
void InitSI7021(void);

/*
 *	SI7021:	��ȡSI7021�Ĺ��ն�����
 *	����ֵ: 			���նȣ��Ŵ�100��
 */
void SI7021ReadTH(int32_t *measure);
		
static void I2C_SI7021_Error (void);		
		
uint8_t SI7021_Configtrue(uint8_t reg);		
		
uint8_t SI7021_WriteOneByte(uint8_t *data, uint8_t number);
		
uint16_t SI7021_ReadOneByte(uint8_t reg);
		
static uint8_t SI7021_CheckCrc(uint8_t data[]);

#ifdef __cplusplus
}
#endif

#endif
