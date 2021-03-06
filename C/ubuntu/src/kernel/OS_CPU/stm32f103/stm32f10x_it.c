/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTI
  
  AL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <bsp_support.h>
#include <myMicroLIB.h>
#include <TCB.h>
#include <shell.h>
#include <os_cpu.h>
#include <os_TCB_list.h>
#include <shell_fun.h>
#include <buddy.h>
#include <slab.h>
#include <os_time.h>
/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

void hard_fault_handler_c(unsigned int * hardfault_args)   

{   

unsigned int stacked_r0;   

unsigned int stacked_r1;   

unsigned int stacked_r2;   

unsigned int stacked_r3;   

unsigned int stacked_r12;   

unsigned int stacked_lr;   

unsigned int stacked_pc;   

unsigned int stacked_psr;   

  

stacked_r0 = ((unsigned long) hardfault_args[0]);   

stacked_r1 = ((unsigned long) hardfault_args[1]);   

stacked_r2 = ((unsigned long) hardfault_args[2]);   

stacked_r3 = ((unsigned long) hardfault_args[3]);   

  

stacked_r12 = ((unsigned long) hardfault_args[4]);   

stacked_lr = ((unsigned long) hardfault_args[5]);   

stacked_pc = ((unsigned long) hardfault_args[6]);   

stacked_psr = ((unsigned long) hardfault_args[7]);   

  

ka_printf ("[Hard fault handler]\n");   

ka_printf ("R0 = 0x%x\r\n", stacked_r0);   

ka_printf ("R1 = 0x%x\r\n", stacked_r1);   

ka_printf ("R2 = 0x%x\r\n", stacked_r2);   

ka_printf ("R3 = 0x%x\r\n", stacked_r3);   

ka_printf ("R12 = 0x%x\r\n", stacked_r12);   

ka_printf ("LR = 0x%x\r\n", stacked_lr);   

ka_printf ("PC = 0x%x\r\n", stacked_pc);   

ka_printf ("PSR = 0x%x\r\n", stacked_psr);   

ka_printf ("BFAR = 0x%lx\r\n", (*((volatile unsigned long *)(0xE000ED38))));   

ka_printf ("CFSR = 0x%lx\r\n", (*((volatile unsigned long *)(0xE000ED28))));   

ka_printf ("HFSR = 0x%lx\r\n", (*((volatile unsigned long *)(0xE000ED2C))));   

ka_printf ("DFSR = 0x%lx\r\n", (*((volatile unsigned long *)(0xE000ED30))));   

ka_printf ("AFSR = 0x%lx\r\n", (*((volatile unsigned long *)(0xE000ED3C))));  
/*
#if CONFIG_SHELL_EN
  shell_stack_check(1,NULL);
  shell_TCB_check(1,NULL);
  shell_buddy_debug(1,NULL);
  shell_check_slab(1,NULL);
  shell_memory(1,NULL);
#if CONFIG_TIME_EN
  system_time_display();
#endif
  ka_printf("tick is %llu\n",get_tick());
#endif
#if CONFIG_POWER_MANAGEMENT
extern void sys_sleep(void);
    sys_sleep();
#endif
*/
while(1) 
{ 
        ;; 
} 

  

} 

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */

void SVC_Handler(void)
{

}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}


void DEBUG_USART_IRQHandler(void)
{	
#if CONFIG_SHELL_EN
	SYS_ENTER_INTERRUPT();

	if(USART_GetITStatus(DEBUG_USARTx,USART_IT_RXNE)!=RESET)
	{
		_put_in_shell_buffer(USART_ReceiveData(DEBUG_USARTx));			
	}

	SYS_EXIT_INTERRUPT();
#endif
}

#include <module.h>
void USART2_IRQHandler(void)
{
  SYS_ENTER_INTERRUPT();
  if(USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET)
  {
#if CONFIG_MODULE
    _put_in_module_buffer(USART_ReceiveData(USART2));
#endif
  }   
  SYS_EXIT_INTERRUPT();
}


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
