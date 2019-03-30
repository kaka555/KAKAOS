/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.4.0
  * @date    04-August-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
 

/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

#include <bsp_support.h>
#include <myMicroLIB.h>
#include <TCB.h>
#include <shell.h>
#include <os_cpu.h>

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

  

    

while(1) 

{ 

        ;; 

} 

  

} 




/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
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


void USART1_IRQHandler(void)
{
#if CONFIG_SHELL_EN
	SYS_ENTER_INTERRUPT();

	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)
	{
		_put_in_shell_buffer(USART_ReceiveData(USART1));			
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
	
 

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
