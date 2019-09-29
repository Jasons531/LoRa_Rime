/**
  ******************************************************************************
  * �ļ�����: 
  * ��    ��: ũ��Ƕ��ʽ�����Ŷ�(LWY)
  * ��    ��: V1.0
  * ��д����: 
  * ��    ��: 
  ******************************************************************************
  * ��飺
  *
  *
  ******************************************************************************
  * �汾�޸ļ�¼��
  * v1.0    (1)
  * 
  * v1.1    (1)
  *         (2)
  *
  * v1.2    (1)
  *
  * ��Ȩ��ũ��Ƕ��ʽ�����Ŷ����У��������á�
  ******************************************************************************
  */
  
/* Ԥ���� ---------------------------------------------------------------------*/
#ifndef __USER_SPI_FLASH_H__
#define __USER_SPI_FLASH_H__

#ifdef __cplusplus
 extern "C" {
#endif
	 
#include "gpio.h"
	 

/* ����ͷ�ļ� ------------------------------------------------------------------*/

/* ��չ���� --------------------------------------------------------------------*/

/* �궨�壨��չ������ -----------------------------------------------------------*/
#define hspiflash                       hspi1

#define FLASH_ID                        0xc22013                //ʹ��RDIDָ���
#define ELECTRONIC_ID                   0x12                    //RESָ���
#define RESID0                          0xc212                  //REMSָ���
#define RESID1                          0x12c2
#define SPI_FLASH_SIZE                  0x80000                 //512KB��4Mb 512K�ֽ�  4Mλ
#define SPI_FLASH_PAGESIZE              256
#define SPI_FLASH_PERWRITEPAGESIZE      256
#define SPI_FLASH_SECTOR_SIZE           4096                    //ÿ������4096�ֽ�

/* �궨�壨��չ������ -----------------------------------------------------------*/
#define FLASH_SPI_CS_ENABLE()                      HAL_GPIO_WritePin(FLASH_SPI_CS_PORT, FLASH_SPI_CS_PIN, GPIO_PIN_RESET)
#define FLASH_SPI_CS_DISABLE()                     HAL_GPIO_WritePin(FLASH_SPI_CS_PORT, FLASH_SPI_CS_PIN, GPIO_PIN_SET)

/* ��չ���� ------------------------------------------------------------------*/
extern SPI_HandleTypeDef hspiflash;

/* ��������  -------------------------------------------------------------------*/

void SpiFlashInit(void);
void SpiFlashCsInit(void);
void SpiFlashSectorErase(uint32_t SectorAddr);
void SpiFlashBulkErase(void);
void SpiFlashPageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void SpiFlashBufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void SpiFlashBufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
//uint32_t SpiFlashReadID(void);
uint32_t SpiFlashReadDeviceID(void);
void SpiFlashStartReadSequence(uint32_t ReadAddr);  //
void SpiFlashPowerDown(void);
void SpiFlashWAKEUP(void);

uint8_t SpiFlashReadByte(void);
uint8_t SpiFlashSendByte(uint8_t byte);
void SpiFlashWriteEnable(void);
void SpiFlashWaitForWriteEnd(void);
#ifdef __cplusplus
}
#endif

#endif  /* __USER_SPI_FLASH_H__ */

/************************ (C) COPYRIGHT ũ��Ƕ��ʽ�����Ŷ� *****END OF FILE****/
