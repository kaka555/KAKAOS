#include <os_cpu_stm32.h>
#include <ka_configuration.h>
#include <stm32f10x.h>
#include <osinit.h>
#include "stm32f10x_pwr.h" 
#include "stm32f10x_rcc.h" 
#include <myMicroLIB.h>



#if (32 == CPU_BIT)
	typedef unsigned int* STACK_P;
#else
	#error "CPU_BIT not be setted,end compile"
#endif

void set_register(void **stack_ptr,void *entry_ptr,void *return_ptr,void *para)
{
	STACK_P stack = *stack_ptr;
	*stack = (unsigned int)0x01000000uL;		//xPSR
	*(--stack) = (unsigned int)entry_ptr;  		//R15 PC
	*(--stack) = (unsigned int)return_ptr;		//R14 LR	
	*(--stack) = (unsigned int)0x12121212uL;	//R12
	*(--stack) = (unsigned int)0x03030303uL;	//R3
	*(--stack) = (unsigned int)0x02020202uL;	//R2
	*(--stack) = (unsigned int)0x01010101uL;	//R1
	*(--stack) = (unsigned int)para;			//R0
	*(--stack) = (unsigned int)0x11111111uL;	//R11
	*(--stack) = (unsigned int)0x10101010uL;	//R10
	*(--stack) = (unsigned int)0x09090909uL;	//R9
	*(--stack) = (unsigned int)0x08080808uL;	//R8
	*(--stack) = (unsigned int)0x07070707uL;	//R7
	*(--stack) = (unsigned int)0x06060606uL;	//R6
	*(--stack) = (unsigned int)0x05050505uL;	//R5
	*(--stack) = (unsigned int)0x04040404uL;	//R4
	*stack_ptr = stack;
}

void __init_systick(void)
{
	CPU_INT32U  prio;
	CPU_REG_NVIC_ST_RELOAD = SystemCoreClock / TICK_PER_SEC  -1 ; 
    /* Set SysTick handler prio */
    prio  = CPU_REG_NVIC_SHPRI3;
    prio &= DEF_BIT_FIELD(24, 0);
    prio |= DEF_BIT_MASK(OS_CPU_CFG_SYSTICK_PRIO, 24);

    CPU_REG_NVIC_SHPRI3 = prio; 

                                                            /* Enable timer.                                   */
    CPU_REG_NVIC_ST_CTRL |= CPU_REG_NVIC_ST_CTRL_CLKSOURCE |
                            CPU_REG_NVIC_ST_CTRL_ENABLE;
                                                            /* Enable timer interrupt.                                */
    CPU_REG_NVIC_ST_CTRL |= CPU_REG_NVIC_ST_CTRL_TICKINT;
}

extern void Reset_Handler();
void ReBoot()
{
	asm volatile(
	    "CPSID   I\n\t"
	    "MRS R0,CONTROL\n\t"
	    "AND R0,#0x01\n\t"
	    "MSR CONTROL,R0\n\t"
	    "LDR SP, =_estack\n\t"
	    "LDR R1,=0xE000E010\n\t"
	    "LDR R0,[R1]\n\t"
	    "AND R0,#0xFFFFFFFD\n\t"
	    "STR R0,[R1]\n\t"
	    "B   Reset_Handler\n\t"
	);
}

#if CONFIG_POWER_MANAGEMENT
void sys_sleep(void)
{

	CPU_REG_NVIC_ST_CTRL &= ~(CPU_REG_NVIC_ST_CTRL_TICKINT | CPU_REG_NVIC_ST_CTRL_ENABLE);
	asm volatile("wfi");

	CPU_REG_NVIC_ST_CTRL |= CPU_REG_NVIC_ST_CTRL_CLKSOURCE |
                            CPU_REG_NVIC_ST_CTRL_ENABLE;
                                                            /* Enable timer interrupt.                                */
    CPU_REG_NVIC_ST_CTRL |= CPU_REG_NVIC_ST_CTRL_TICKINT;

}

void sys_shutdown(void)
{
	PWR_EnterSTANDBYMode();
}
#endif

/*
void __init_svc(void)
{
	CPU_INT32U  prio;
	prio = CPU_REG_NVIC_SHPRI2;
	prio &= 0x00ffffff;
	CPU_REG_NVIC_SHPRI2 = prio;
}

inline void set_return_value(unsigned int *stack_ptr,unsigned int value)
{
	stack_ptr[8] = value;
}
*/
