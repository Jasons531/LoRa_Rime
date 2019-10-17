/*
**************************************************************************************************************
*	@file	main.c
*	@author Jason_531@163.com
*	@version V1.1
*	@date    2017/12/13
*	@brief	NBI_LoRaWAN功能代码: add OTAA
***************************************************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <math.h>
#include "stm32l0xx_hal.h"
#include "usart.h"
#include "rtc-board.h"
#include "delay.h"
#include "board.h"
#include "etimer.h"
#include "autostart.h"

/*******************************************************************************
  * @函数名称	main
  * @函数说明   主函数 
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无

	版本说明：

	优化功能：
  *****************************************************************************/
/* variable functions ---------------------------------------------------------*/	

int main(void)
{	
	BoardInitMcu(  );
	
	DEBUG_APP(2,"TIME : %s  DATE : %s\r\n",__TIME__, __DATE__);
				
	uint8_t TxData = 0X42;
    uint8_t pRxData = SX1276Read( TxData );

	DEBUG_APP(2, "SX1276 ID = 0x%x",pRxData);  ///读取到0x12则正确，否则错误	
		
	process_init(  );
	autostart_start(autostart_processes);///自动包含下面的线程
	
	while (1)
	{		
		do
		{
		}while(process_run() > 0); 
	}
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{ 
	DEBUG_APP(2,"error\r\n");
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
