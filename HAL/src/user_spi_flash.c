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

/* 包含头文件 ----------------------------------------------------------------*/
#include "user_spi_flash.h"

/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
/*数值常量*/
#define WIP_Flag                      0x01    /* Write In Progress (WIP) flag */
#define Dummy_Byte                    0xFF    //发送任意数据

#define SPI_FLASH_ID1                 0xc22013                //使用RDID指令读
#define SPI_FLASH_ID2                 0xc22012                //使用RDID指令读

/*Flash操作指令*/
#define MX25_WriteEnable		    			0x06    //写使能
#define MX25_WriteDisable		    			0x04    //写失能
#define MX25_WriteStatusReg		    		0x01    //写状态寄存器
#define MX25_ReadStatusReg		    		0x05    //读状态寄存器
#define MX25_ReadData               	0x03    //读数据
#define MX25_FastReadData           	0x0B    //快速读数据
#define MX25_ReadSFDP               	0x5A
#define MX25_DeviceID               	0x9F    //读取设备ID指令      //RDID      //0xc22013
#define MX25_ElectronicID           	0xAB    //读设备电子ID        //RES       //0x12
#define MX25_ManufactDeviceID       	0x90    //读制造商和设备ID     //REMS      //0xc212或者0x12c2
#define MX25_FastReadDual           	0x3B    //双重读数据
#define MX25_SectorErase            	0x20    //页擦除
#define MX25_BlockErase             	0xD8    //块擦除
//#define MX25_BlockErase             0x52    //块擦除
#define MX25_ChipErase              	0xC7    //芯片擦除
//#define MX25_ChipErase              0x60    //芯片擦除
#define MX25_PageProgram            	0x02    //烧录选择了的页
#define MX25_PowerDown              	0xB9    //进入深度休眠
#define MX25_ReleasePowerDown	    		0xAB    //退出深度休眠



/* 私有变量 ------------------------------------------------------------------*/
/* 私有函数 ------------------------------------------------------------------*/
/* 扩展变量 ------------------------------------------------------------------*/
/* 扩展函数 ------------------------------------------------------------------*/

/**
  * 函数功能: 初始化
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
void SpiFlashInit(void)
{		
	SpiFlashCsInit( );
	SpiFlashWAKEUP(  );
	
	SpiFlashReadDeviceID(  );	
	
	/*********关闭flash电源***********/
	SpiFlashPowerDown(  );
}

/**
  * 函数功能: 初始化SPI_CS
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
void SpiFlashCsInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	__HAL_RCC_GPIOA_CLK_ENABLE(  );
	
	GPIO_InitStruct.Pin 	= FLASH_SPI_CS_PIN;
  GPIO_InitStruct.Mode 	= GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull 	= GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(FLASH_SPI_CS_PORT, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(FLASH_SPI_CS_PORT, FLASH_SPI_CS_PIN, GPIO_PIN_SET);
}

/**
  * 函数功能: 擦除扇区
  * 输入参数: SectorAddr：待擦除扇区地址，要求为4096倍数
  * 返 回 值: 无
  * 说    明：串行Flash最小擦除块大小为4KB(4096字节)，即一个扇区大小，要求输入参数
  *           为4096倍数。在往串行Flash芯片写入数据之前要求先擦除空间。
  */
void SpiFlashSectorErase(uint32_t SectorAddr)
{
  /* 发送FLASH写使能命令 */
  SpiFlashWriteEnable();
  SpiFlashWaitForWriteEnd();
  /* 擦除扇区 */
  /* 选择串行FLASH: CS低电平 */
  FLASH_SPI_CS_ENABLE();
  /* 发送扇区擦除指令*/
  SpiFlashSendByte(MX25_SectorErase);
  /*发送擦除扇区地址的高位*/
  SpiFlashSendByte((SectorAddr & 0xFF0000) >> 16);
  /* 发送擦除扇区地址的中位 */
  SpiFlashSendByte((SectorAddr & 0xFF00) >> 8);
  /* 发送擦除扇区地址的低位 */
  SpiFlashSendByte(SectorAddr & 0xFF);
  /* 禁用串行FLASH: CS 高电平 */
  FLASH_SPI_CS_DISABLE();
  /* 等待擦除完毕*/
  SpiFlashWaitForWriteEnd();
}

/**
  * 函数功能: 擦除整片
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：擦除串行Flash整片空间
  */
void SpiFlashBulkErase(void)
{
 /* 发送FLASH写使能命令 */
  SpiFlashWriteEnable();

  /* 整片擦除 Erase */
  /* 选择串行FLASH: CS低电平 */
  FLASH_SPI_CS_ENABLE();
  /* 发送整片擦除指令*/
  SpiFlashSendByte(MX25_ChipErase);
  /* 禁用串行FLASH: CS高电平 */
  FLASH_SPI_CS_DISABLE();

  /* 等待擦除完毕*/
  SpiFlashWaitForWriteEnd();
}

/**
  * 函数功能: 往串行FLASH按页写入数据，调用本函数写入数据前需要先擦除扇区
  * 输入参数: pBuffer：待写入数据的指针
  *           WriteAddr：写入地址
  *           NumByteToWrite：写入数据长度，必须小于等于SPI_FLASH_PERWRITEPAGESIZE
  * 返 回 值: 无
  * 说    明：串行Flash每页大小为256个字节
  */
void SpiFlashPageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
  /* 发送FLASH写使能命令 */
  SpiFlashWriteEnable();

   /* 寻找串行FLASH: CS低电平 */
  FLASH_SPI_CS_ENABLE();
  /* 写送写指令*/
  SpiFlashSendByte(MX25_PageProgram);
  /*发送写地址的高位*/
  SpiFlashSendByte((WriteAddr & 0xFF0000) >> 16);
  /*发送写地址的中位*/
  SpiFlashSendByte((WriteAddr & 0xFF00) >> 8);
  /*发送写地址的低位*/
  SpiFlashSendByte(WriteAddr & 0xFF);

  if(NumByteToWrite > SPI_FLASH_PERWRITEPAGESIZE)
  {
     NumByteToWrite = SPI_FLASH_PERWRITEPAGESIZE;
     //printf("Err: SpiFlashPageWrite too large!\n");
  }

  /* 写入数据*/
  while (NumByteToWrite--)
  {
     /* 发送当前要写入的字节数据 */
    SpiFlashSendByte(*pBuffer);
     /* 指向下一字节数据 */
    pBuffer++;
  }

  /* 禁用串行FLASH: CS 高电平 */
  FLASH_SPI_CS_DISABLE();

  /* 等待写入完毕*/
  SpiFlashWaitForWriteEnd();
}

/**
  * 函数功能: 往串行FLASH写入数据，调用本函数写入数据前需要先擦除扇区
  * 输入参数: pBuffer：待写入数据的指针
  *           WriteAddr：写入地址
  *           NumByteToWrite：写入数据长度
  * 返 回 值: 无
  * 说    明：该函数可以设置任意写入数据长度
  */
void SpiFlashBufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
  uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

  Addr = WriteAddr % SPI_FLASH_PAGESIZE;
  count = SPI_FLASH_PAGESIZE - Addr;
  NumOfPage =  NumByteToWrite / SPI_FLASH_PAGESIZE;
  NumOfSingle = NumByteToWrite % SPI_FLASH_PAGESIZE;
  
//  printf("Addr=%d,count=%d,NumOfPage=%d,NumOfSingle=%d\r\n",Addr,count,NumOfPage,NumOfSingle);

  if (Addr == 0) /* 若地址与 SPI_FLASH_PAGESIZE 对齐  */
  {
    if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PAGESIZE */
    {
      SpiFlashPageWrite(pBuffer, WriteAddr, NumByteToWrite);
    }
    else /* NumByteToWrite > SPI_FLASH_PAGESIZE */
    {
      while (NumOfPage--)
      {
        SpiFlashPageWrite(pBuffer, WriteAddr, SPI_FLASH_PAGESIZE);
        WriteAddr +=  SPI_FLASH_PAGESIZE;
        pBuffer += SPI_FLASH_PAGESIZE;
      }

      SpiFlashPageWrite(pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else /* 若地址与 SPI_FLASH_PAGESIZE 不对齐 */
  {
    if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PAGESIZE */
    {
      if (NumOfSingle > count) /* (NumByteToWrite + WriteAddr) > SPI_FLASH_PAGESIZE */
      {
        temp = NumOfSingle - count;

        SpiFlashPageWrite(pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count;

        SpiFlashPageWrite(pBuffer, WriteAddr, temp);
      }
      else
      {
        SpiFlashPageWrite(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /* NumByteToWrite > SPI_FLASH_PAGESIZE */
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / SPI_FLASH_PAGESIZE;
      NumOfSingle = NumByteToWrite % SPI_FLASH_PAGESIZE;

      SpiFlashPageWrite(pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;

      while (NumOfPage--)
      {
        SpiFlashPageWrite(pBuffer, WriteAddr, SPI_FLASH_PAGESIZE);
        WriteAddr +=  SPI_FLASH_PAGESIZE;
        pBuffer += SPI_FLASH_PAGESIZE;
      }

      if (NumOfSingle != 0)
      {
        SpiFlashPageWrite(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}

/**
  * 函数功能: 从串行Flash读取数据
  * 输入参数: pBuffer：存放读取到数据的指针
  *           ReadAddr：读取数据目标地址
  *           NumByteToRead：读取数据长度
  * 返 回 值: 无
  * 说    明：该函数可以设置任意读取数据长度
  */
void SpiFlashBufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
  /* 选择串行FLASH: CS低电平 */
  FLASH_SPI_CS_ENABLE();

  /* 发送 读 指令 */
  SpiFlashSendByte(MX25_ReadData);

  /* 发送 读 地址高位 */
  SpiFlashSendByte((ReadAddr & 0xFF0000) >> 16);
  /* 发送 读 地址中位 */
  SpiFlashSendByte((ReadAddr& 0xFF00) >> 8);
  /* 发送 读 地址低位 */
  SpiFlashSendByte(ReadAddr & 0xFF);

  while (NumByteToRead--) /* 读取数据 */
  {
     /* 读取一个字节*/
    *pBuffer = SpiFlashSendByte(Dummy_Byte);
    /* 指向下一个字节缓冲区 */
    pBuffer++;
  }

  /* 禁用串行FLASH: CS 高电平 */
  FLASH_SPI_CS_DISABLE();
}

/**
  * 函数功能: 读取串行Flash型号的ID
  * 输入参数: 无
  * 返 回 值: uint32_t：串行Flash的型号ID
  * 说    明：  
  */
//uint32_t SpiFlashReadID(void)
//{
//  uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

//  /* 选择串行FLASH: CS低电平 */
//  FLASH_SPI_CS_ENABLE();

//  /* 发送命令：读取芯片型号ID */
//  SpiFlashSendByte(MX25_ManufactDeviceID);

//  /* 从串行Flash读取一个字节数据 */
//  Temp0 = SpiFlashSendByte(Dummy_Byte);

//  /* 从串行Flash读取一个字节数据 */
//  Temp1 = SpiFlashSendByte(Dummy_Byte);

//  /* 从串行Flash读取一个字节数据 */
//  Temp2 = SpiFlashSendByte(Dummy_Byte);

//  /* 禁用串行Flash：CS高电平 */
//  FLASH_SPI_CS_DISABLE();
//  
//  Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
//  return Temp;
//}

/**
  * 函数功能: 读取串行Flash设备ID
  * 输入参数: 无
  * 返 回 值: uint32_t：串行Flash的设备ID
  * 说    明：
  */
uint32_t SpiFlashReadDeviceID(void)
{
  uint32_t Temp = 0;

  /* 选择串行FLASH: CS低电平 */
  FLASH_SPI_CS_ENABLE();

  /* 发送命令：读取芯片设备ID * */
  SpiFlashSendByte(MX25_DeviceID);  
  Temp |= SpiFlashSendByte(Dummy_Byte)<<16;
  Temp |= SpiFlashSendByte(Dummy_Byte)<<8;
  Temp |= SpiFlashSendByte(Dummy_Byte)<<0;
  
  /* 禁用串行Flash：CS高电平 */
  FLASH_SPI_CS_DISABLE();

  DEBUG_APP(2,"SPIDeviceId=0x%04x\r\n", Temp);
	
	if( Temp != SPI_FLASH_ID1 && Temp != SPI_FLASH_ID2 )
	{
    DEBUG_APP(2, "SPI芯片型号错误\r\n");
	}
  return Temp;
}

/**
  * 函数功能: 启动连续读取数据串
  * 输入参数: ReadAddr：读取地址
  * 返 回 值: 无
  * 说    明：Initiates a read data byte (READ) sequence from the Flash.
  *           This is done by driving the /CS line low to select the device,
  *           then the READ instruction is transmitted followed by 3 bytes
  *           address. This function exit and keep the /CS line low, so the
  *           Flash still being selected. With this technique the whole
  *           content of the Flash is read with a single READ instruction.
  */
void SpiFlashStartReadSequence(uint32_t ReadAddr)
{
  /* Select the FLASH: Chip Select low */
  FLASH_SPI_CS_ENABLE();

  /* Send "Read from Memory " instruction */
  SpiFlashSendByte(MX25_ReadData);

  /* Send the 24-bit address of the address to read from -----------------------*/
  /* Send ReadAddr high nibble address byte */
  SpiFlashSendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte */
  SpiFlashSendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte */
  SpiFlashSendByte(ReadAddr & 0xFF);
}

/**
  * 函数功能: 从串行Flash读取一个字节数据
  * 输入参数: 无
  * 返 回 值: uint8_t：读取到的数据
  * 说    明：This function must be used only if the Start_Read_Sequence
  *           function has been previously called.
  */
uint8_t SpiFlashReadByte(void)
{
  uint8_t d_read,d_send=Dummy_Byte;
  if(HAL_SPI_TransmitReceive(&hspiflash,&d_send,&d_read,1,0xFFFFFF)!=HAL_OK)
    d_read=Dummy_Byte;
  
  return d_read;    
}

/**
  * 函数功能: 往串行Flash读取写入一个字节数据并接收一个字节数据
  * 输入参数: byte：待发送数据
  * 返 回 值: uint8_t：接收到的数据
  * 说    明：无
  */
uint8_t SpiFlashSendByte(uint8_t byte)
{
  uint8_t d_read,d_send=byte;
  if(HAL_SPI_TransmitReceive(&hspiflash,&d_send,&d_read,1,0xFFFFFF)!=HAL_OK)
    d_read=Dummy_Byte;
  
  return d_read; 
}

/**
  * 函数功能: 往串行Flash读取写入一个字节数据并接收一个字节数据
  * 输入参数: byte：待发送数据
  * 返 回 值: uint8_t：接收到的数据
  * 说    明：无
  */
uint8_t SpiFlashSendBytes(uint8_t byte)
{
  uint8_t d_read[3],d_send=byte;
  if(HAL_SPI_TransmitReceive(&hspiflash,&d_send,d_read,3,0xFFFFFF)!=HAL_OK)
  {
    printf("read spi fail\r\n");
  }
  
  printf("DerveID: ");
  for(uint8_t i = 0; i < 3; i++)
  {
    printf("0x%02x ",d_read[i]);
  }
  printf("\r\n");
  
  return *d_read; 
}

/**
  * 函数功能: 使能串行Flash写操作
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
void SpiFlashWriteEnable(void)
{
  /* 选择串行FLASH: CS低电平 */
  FLASH_SPI_CS_ENABLE();

  /* 发送命令：写使能 */
  SpiFlashSendByte(MX25_WriteEnable);

  /* 禁用串行Flash：CS高电平 */
  FLASH_SPI_CS_DISABLE();
}

/**
  * 函数功能: 等待数据写入完成
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：Polls the status of the Write In Progress (WIP) flag in the
  *           FLASH's status  register  and  loop  until write  opertaion
  *           has completed.
  */
void SpiFlashWaitForWriteEnd(void)
{
  uint8_t FLASH_Status = 0;

  /* Select the FLASH: Chip Select low */
  FLASH_SPI_CS_ENABLE();

  /* Send "Read Status Register" instruction */
  SpiFlashSendByte(MX25_ReadStatusReg);

  /* Loop as long as the memory is busy with a write cycle */
  do
  {
    /* Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    FLASH_Status = SpiFlashSendByte(Dummy_Byte);	 
  }
  while ((FLASH_Status & WIP_Flag) == SET); /* Write in progress */

  /* Deselect the FLASH: Chip Select high */
  FLASH_SPI_CS_DISABLE();
}


/**
  * 函数功能: 进入掉电模式
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
void SpiFlashPowerDown(void)   
{ 
  /* Select the FLASH: Chip Select low */
  FLASH_SPI_CS_ENABLE();

  /* Send "Power Down" instruction */
  SpiFlashSendByte(MX25_PowerDown);

  /* Deselect the FLASH: Chip Select high */
  FLASH_SPI_CS_DISABLE();
}   

/**
  * 函数功能: 唤醒串行Flash
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
void SpiFlashWAKEUP(void)   
{
  /* Select the FLASH: Chip Select low */
  FLASH_SPI_CS_ENABLE();

  /* Send "Power Down" instruction */
  SpiFlashSendByte(MX25_ReleasePowerDown);

  /* Deselect the FLASH: Chip Select high */
  FLASH_SPI_CS_DISABLE(); 
}   
/************************ (C) COPYRIGHT 农博嵌入式开发团队 *****END OF FILE****/
