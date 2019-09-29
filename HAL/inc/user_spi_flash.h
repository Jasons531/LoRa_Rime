/**
  ******************************************************************************
  * 文件名程: 
  * 作    者: 农博嵌入式开发团队(LWY)
  * 版    本: V1.0
  * 编写日期: 
  * 功    能: 
  ******************************************************************************
  * 简介：
  *
  *
  ******************************************************************************
  * 版本修改记录：
  * v1.0    (1)
  * 
  * v1.1    (1)
  *         (2)
  *
  * v1.2    (1)
  *
  * 版权归农博嵌入式开发团队所有，请勿商用。
  ******************************************************************************
  */
  
/* 预定义 ---------------------------------------------------------------------*/
#ifndef __USER_SPI_FLASH_H__
#define __USER_SPI_FLASH_H__

#ifdef __cplusplus
 extern "C" {
#endif
	 
#include "gpio.h"
	 

/* 包含头文件 ------------------------------------------------------------------*/

/* 拓展类型 --------------------------------------------------------------------*/

/* 宏定义（拓展常量） -----------------------------------------------------------*/
#define hspiflash                       hspi1

#define FLASH_ID                        0xc22013                //使用RDID指令读
#define ELECTRONIC_ID                   0x12                    //RES指令读
#define RESID0                          0xc212                  //REMS指令读
#define RESID1                          0x12c2
#define SPI_FLASH_SIZE                  0x80000                 //512KB，4Mb 512K字节  4M位
#define SPI_FLASH_PAGESIZE              256
#define SPI_FLASH_PERWRITEPAGESIZE      256
#define SPI_FLASH_SECTOR_SIZE           4096                    //每个扇区4096字节

/* 宏定义（拓展函数） -----------------------------------------------------------*/
#define FLASH_SPI_CS_ENABLE()                      HAL_GPIO_WritePin(FLASH_SPI_CS_PORT, FLASH_SPI_CS_PIN, GPIO_PIN_RESET)
#define FLASH_SPI_CS_DISABLE()                     HAL_GPIO_WritePin(FLASH_SPI_CS_PORT, FLASH_SPI_CS_PIN, GPIO_PIN_SET)

/* 扩展变量 ------------------------------------------------------------------*/
extern SPI_HandleTypeDef hspiflash;

/* 函数声明  -------------------------------------------------------------------*/

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

/************************ (C) COPYRIGHT 农博嵌入式开发团队 *****END OF FILE****/
