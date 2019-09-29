/*
*Function:
*Programed by:Jason_531@163.com
*Complete date:
*Modified by:
*Modified date:
*Remarks:
*/

#include "stm32l0xx_hal.h"
#include "contiki.h"
#include "sys/clock.h"
#include "sys/cc.h"
#include "sys/etimer.h"
#include "debug-uart.h"

#define RELOAD_VALUE 100000-1    /* 1 ms with a 24 MHz clock */

#define	F_CPU		16000000UL

static volatile clock_time_t current_clock = 0;
static volatile unsigned long current_seconds = 0;
static unsigned int second_countdown = CLOCK_SECOND;

uint32_t TimerOverTime = 0; ///timer2超时机制

void SysTick_Handler(void)
{	
  current_clock++;
  if(etimer_pending()) {
    etimer_request_poll();
  }
  if (--second_countdown == 0) {
    current_seconds++;
    second_countdown = CLOCK_SECOND;
  }
  
  TimerOverTime ++;
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}


void
clock_init()
{  
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/CLOCK_SECOND);
	
	/**Configure the Systick 
	*/
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
		/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

clock_time_t
clock_time(void)
{
  return current_clock;
}

void clock_delay_usec(uint16_t usec)
{
  /* Delay by watching the SysTick value change. */
  int32_t remaining = (int32_t)usec * F_CPU / 1000000;
  int32_t old = SysTick->VAL;
  while(remaining > 0) {
    int32_t new = SysTick->VAL;
    if(new > old) { /* wraparound */
      old += SysTick->LOAD;
    }
    remaining -= (old - new);
    old = new;
  }
	
//	uint32_t ticks;
//	uint32_t told,tnow,tcnt=0;
//	uint32_t reload=SysTick->LOAD;				//LOAD的值	    	 
//	ticks=(int32_t)usec * F_CPU / 1000000; 						//需要的节拍数 
//	told=SysTick->VAL;        				//刚进入时的计数器值
//	while(1)
//	{
//		tnow=SysTick->VAL;	
//		if(tnow!=told)
//		{	    
//			if(tnow<told)tcnt+=told-tnow;	//这里注意一下SYSTICK是一个递减的计数器就可以了.
//			else tcnt+=reload-tnow+told;	    
//			told=tnow;
//			if(tcnt>=ticks)break;			//时间超过/等于要延迟的时间,则退出.
//		}  
//	};
}


#if 0
/* The inner loop takes 4 cycles. The outer 5+SPIN_COUNT*4. */

#define SPIN_TIME 2 /* us */
#define SPIN_COUNT (((MCK*SPIN_TIME/1000000)-5)/4)

#ifndef __MAKING_DEPS__

void
clock_delay(unsigned int t)
{
#ifdef __THUMBEL__ 
  asm volatile("1: mov r1,%2\n2:\tsub r1,#1\n\tbne 2b\n\tsub %0,#1\n\tbne 1b\n":"=l"(t):"0"(t),"l"(SPIN_COUNT));
#else
#error Must be compiled in thumb mode
#endif
}
#endif
#endif /* __MAKING_DEPS__ */

unsigned long
clock_seconds(void)
{
  return current_seconds;
}
