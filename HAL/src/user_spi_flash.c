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

/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "user_spi_flash.h"

/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
/*��ֵ����*/
#define WIP_Flag                      0x01    /* Write In Progress (WIP) flag */
#define Dummy_Byte                    0xFF    //������������

#define SPI_FLASH_ID1                 0xc22013                //ʹ��RDIDָ���
#define SPI_FLASH_ID2                 0xc22012                //ʹ��RDIDָ���

/*Flash����ָ��*/
#define MX25_WriteEnable		    			0x06    //дʹ��
#define MX25_WriteDisable		    			0x04    //дʧ��
#define MX25_WriteStatusReg		    		0x01    //д״̬�Ĵ���
#define MX25_ReadStatusReg		    		0x05    //��״̬�Ĵ���
#define MX25_ReadData               	0x03    //������
#define MX25_FastReadData           	0x0B    //���ٶ�����
#define MX25_ReadSFDP               	0x5A
#define MX25_DeviceID               	0x9F    //��ȡ�豸IDָ��      //RDID      //0xc22013
#define MX25_ElectronicID           	0xAB    //���豸����ID        //RES       //0x12
#define MX25_ManufactDeviceID       	0x90    //�������̺��豸ID     //REMS      //0xc212����0x12c2
#define MX25_FastReadDual           	0x3B    //˫�ض�����
#define MX25_SectorErase            	0x20    //ҳ����
#define MX25_BlockErase             	0xD8    //�����
//#define MX25_BlockErase             0x52    //�����
#define MX25_ChipErase              	0xC7    //оƬ����
//#define MX25_ChipErase              0x60    //оƬ����
#define MX25_PageProgram            	0x02    //��¼ѡ���˵�ҳ
#define MX25_PowerDown              	0xB9    //�����������
#define MX25_ReleasePowerDown	    		0xAB    //�˳��������



/* ˽�б��� ------------------------------------------------------------------*/
/* ˽�к��� ------------------------------------------------------------------*/
/* ��չ���� ------------------------------------------------------------------*/
/* ��չ���� ------------------------------------------------------------------*/

/**
  * ��������: ��ʼ��
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
void SpiFlashInit(void)
{		
	SpiFlashCsInit( );
	SpiFlashWAKEUP(  );
	
	SpiFlashReadDeviceID(  );	
	
	/*********�ر�flash��Դ***********/
	SpiFlashPowerDown(  );
}

/**
  * ��������: ��ʼ��SPI_CS
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
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
  * ��������: ��������
  * �������: SectorAddr��������������ַ��Ҫ��Ϊ4096����
  * �� �� ֵ: ��
  * ˵    ��������Flash��С�������СΪ4KB(4096�ֽ�)����һ��������С��Ҫ���������
  *           Ϊ4096��������������FlashоƬд������֮ǰҪ���Ȳ����ռ䡣
  */
void SpiFlashSectorErase(uint32_t SectorAddr)
{
  /* ����FLASHдʹ������ */
  SpiFlashWriteEnable();
  SpiFlashWaitForWriteEnd();
  /* �������� */
  /* ѡ����FLASH: CS�͵�ƽ */
  FLASH_SPI_CS_ENABLE();
  /* ������������ָ��*/
  SpiFlashSendByte(MX25_SectorErase);
  /*���Ͳ���������ַ�ĸ�λ*/
  SpiFlashSendByte((SectorAddr & 0xFF0000) >> 16);
  /* ���Ͳ���������ַ����λ */
  SpiFlashSendByte((SectorAddr & 0xFF00) >> 8);
  /* ���Ͳ���������ַ�ĵ�λ */
  SpiFlashSendByte(SectorAddr & 0xFF);
  /* ���ô���FLASH: CS �ߵ�ƽ */
  FLASH_SPI_CS_DISABLE();
  /* �ȴ��������*/
  SpiFlashWaitForWriteEnd();
}

/**
  * ��������: ������Ƭ
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������������Flash��Ƭ�ռ�
  */
void SpiFlashBulkErase(void)
{
 /* ����FLASHдʹ������ */
  SpiFlashWriteEnable();

  /* ��Ƭ���� Erase */
  /* ѡ����FLASH: CS�͵�ƽ */
  FLASH_SPI_CS_ENABLE();
  /* ������Ƭ����ָ��*/
  SpiFlashSendByte(MX25_ChipErase);
  /* ���ô���FLASH: CS�ߵ�ƽ */
  FLASH_SPI_CS_DISABLE();

  /* �ȴ��������*/
  SpiFlashWaitForWriteEnd();
}

/**
  * ��������: ������FLASH��ҳд�����ݣ����ñ�����д������ǰ��Ҫ�Ȳ�������
  * �������: pBuffer����д�����ݵ�ָ��
  *           WriteAddr��д���ַ
  *           NumByteToWrite��д�����ݳ��ȣ�����С�ڵ���SPI_FLASH_PERWRITEPAGESIZE
  * �� �� ֵ: ��
  * ˵    ��������Flashÿҳ��СΪ256���ֽ�
  */
void SpiFlashPageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
  /* ����FLASHдʹ������ */
  SpiFlashWriteEnable();

   /* Ѱ�Ҵ���FLASH: CS�͵�ƽ */
  FLASH_SPI_CS_ENABLE();
  /* д��дָ��*/
  SpiFlashSendByte(MX25_PageProgram);
  /*����д��ַ�ĸ�λ*/
  SpiFlashSendByte((WriteAddr & 0xFF0000) >> 16);
  /*����д��ַ����λ*/
  SpiFlashSendByte((WriteAddr & 0xFF00) >> 8);
  /*����д��ַ�ĵ�λ*/
  SpiFlashSendByte(WriteAddr & 0xFF);

  if(NumByteToWrite > SPI_FLASH_PERWRITEPAGESIZE)
  {
     NumByteToWrite = SPI_FLASH_PERWRITEPAGESIZE;
     //printf("Err: SpiFlashPageWrite too large!\n");
  }

  /* д������*/
  while (NumByteToWrite--)
  {
     /* ���͵�ǰҪд����ֽ����� */
    SpiFlashSendByte(*pBuffer);
     /* ָ����һ�ֽ����� */
    pBuffer++;
  }

  /* ���ô���FLASH: CS �ߵ�ƽ */
  FLASH_SPI_CS_DISABLE();

  /* �ȴ�д�����*/
  SpiFlashWaitForWriteEnd();
}

/**
  * ��������: ������FLASHд�����ݣ����ñ�����д������ǰ��Ҫ�Ȳ�������
  * �������: pBuffer����д�����ݵ�ָ��
  *           WriteAddr��д���ַ
  *           NumByteToWrite��д�����ݳ���
  * �� �� ֵ: ��
  * ˵    �����ú���������������д�����ݳ���
  */
void SpiFlashBufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
  uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

  Addr = WriteAddr % SPI_FLASH_PAGESIZE;
  count = SPI_FLASH_PAGESIZE - Addr;
  NumOfPage =  NumByteToWrite / SPI_FLASH_PAGESIZE;
  NumOfSingle = NumByteToWrite % SPI_FLASH_PAGESIZE;
  
//  printf("Addr=%d,count=%d,NumOfPage=%d,NumOfSingle=%d\r\n",Addr,count,NumOfPage,NumOfSingle);

  if (Addr == 0) /* ����ַ�� SPI_FLASH_PAGESIZE ����  */
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
  else /* ����ַ�� SPI_FLASH_PAGESIZE ������ */
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
  * ��������: �Ӵ���Flash��ȡ����
  * �������: pBuffer����Ŷ�ȡ�����ݵ�ָ��
  *           ReadAddr����ȡ����Ŀ���ַ
  *           NumByteToRead����ȡ���ݳ���
  * �� �� ֵ: ��
  * ˵    �����ú����������������ȡ���ݳ���
  */
void SpiFlashBufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
  /* ѡ����FLASH: CS�͵�ƽ */
  FLASH_SPI_CS_ENABLE();

  /* ���� �� ָ�� */
  SpiFlashSendByte(MX25_ReadData);

  /* ���� �� ��ַ��λ */
  SpiFlashSendByte((ReadAddr & 0xFF0000) >> 16);
  /* ���� �� ��ַ��λ */
  SpiFlashSendByte((ReadAddr& 0xFF00) >> 8);
  /* ���� �� ��ַ��λ */
  SpiFlashSendByte(ReadAddr & 0xFF);

  while (NumByteToRead--) /* ��ȡ���� */
  {
     /* ��ȡһ���ֽ�*/
    *pBuffer = SpiFlashSendByte(Dummy_Byte);
    /* ָ����һ���ֽڻ����� */
    pBuffer++;
  }

  /* ���ô���FLASH: CS �ߵ�ƽ */
  FLASH_SPI_CS_DISABLE();
}

/**
  * ��������: ��ȡ����Flash�ͺŵ�ID
  * �������: ��
  * �� �� ֵ: uint32_t������Flash���ͺ�ID
  * ˵    ����  
  */
//uint32_t SpiFlashReadID(void)
//{
//  uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

//  /* ѡ����FLASH: CS�͵�ƽ */
//  FLASH_SPI_CS_ENABLE();

//  /* ���������ȡоƬ�ͺ�ID */
//  SpiFlashSendByte(MX25_ManufactDeviceID);

//  /* �Ӵ���Flash��ȡһ���ֽ����� */
//  Temp0 = SpiFlashSendByte(Dummy_Byte);

//  /* �Ӵ���Flash��ȡһ���ֽ����� */
//  Temp1 = SpiFlashSendByte(Dummy_Byte);

//  /* �Ӵ���Flash��ȡһ���ֽ����� */
//  Temp2 = SpiFlashSendByte(Dummy_Byte);

//  /* ���ô���Flash��CS�ߵ�ƽ */
//  FLASH_SPI_CS_DISABLE();
//  
//  Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
//  return Temp;
//}

/**
  * ��������: ��ȡ����Flash�豸ID
  * �������: ��
  * �� �� ֵ: uint32_t������Flash���豸ID
  * ˵    ����
  */
uint32_t SpiFlashReadDeviceID(void)
{
  uint32_t Temp = 0;

  /* ѡ����FLASH: CS�͵�ƽ */
  FLASH_SPI_CS_ENABLE();

  /* ���������ȡоƬ�豸ID * */
  SpiFlashSendByte(MX25_DeviceID);  
  Temp |= SpiFlashSendByte(Dummy_Byte)<<16;
  Temp |= SpiFlashSendByte(Dummy_Byte)<<8;
  Temp |= SpiFlashSendByte(Dummy_Byte)<<0;
  
  /* ���ô���Flash��CS�ߵ�ƽ */
  FLASH_SPI_CS_DISABLE();

  DEBUG_APP(2,"SPIDeviceId=0x%04x\r\n", Temp);
	
	if( Temp != SPI_FLASH_ID1 && Temp != SPI_FLASH_ID2 )
	{
    DEBUG_APP(2, "SPIоƬ�ͺŴ���\r\n");
	}
  return Temp;
}

/**
  * ��������: ����������ȡ���ݴ�
  * �������: ReadAddr����ȡ��ַ
  * �� �� ֵ: ��
  * ˵    ����Initiates a read data byte (READ) sequence from the Flash.
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
  * ��������: �Ӵ���Flash��ȡһ���ֽ�����
  * �������: ��
  * �� �� ֵ: uint8_t����ȡ��������
  * ˵    ����This function must be used only if the Start_Read_Sequence
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
  * ��������: ������Flash��ȡд��һ���ֽ����ݲ�����һ���ֽ�����
  * �������: byte������������
  * �� �� ֵ: uint8_t�����յ�������
  * ˵    ������
  */
uint8_t SpiFlashSendByte(uint8_t byte)
{
  uint8_t d_read,d_send=byte;
  if(HAL_SPI_TransmitReceive(&hspiflash,&d_send,&d_read,1,0xFFFFFF)!=HAL_OK)
    d_read=Dummy_Byte;
  
  return d_read; 
}

/**
  * ��������: ������Flash��ȡд��һ���ֽ����ݲ�����һ���ֽ�����
  * �������: byte������������
  * �� �� ֵ: uint8_t�����յ�������
  * ˵    ������
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
  * ��������: ʹ�ܴ���Flashд����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
void SpiFlashWriteEnable(void)
{
  /* ѡ����FLASH: CS�͵�ƽ */
  FLASH_SPI_CS_ENABLE();

  /* �������дʹ�� */
  SpiFlashSendByte(MX25_WriteEnable);

  /* ���ô���Flash��CS�ߵ�ƽ */
  FLASH_SPI_CS_DISABLE();
}

/**
  * ��������: �ȴ�����д�����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ����Polls the status of the Write In Progress (WIP) flag in the
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
  * ��������: �������ģʽ
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
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
  * ��������: ���Ѵ���Flash
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
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
/************************ (C) COPYRIGHT ũ��Ƕ��ʽ�����Ŷ� *****END OF FILE****/
