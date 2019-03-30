#ifndef _OS_CPU_STM32_H
#define _OS_CPU_STM32_H

#include <barrier.h>

typedef  unsigned  int         CPU_INT32U;
typedef  volatile  CPU_INT32U  CPU_REG32;
typedef  CPU_INT32U            CPU_SR;

#define  NVIC_INT_CTRL         *((CPU_REG32 *)0xE000ED04)
#define  NVIC_PENDSVSET        0x10000000   
#define  NVIC_SYSPRI14         *((CPU_REG32 *)0xE000ED22)
#define  NVIC_PENDSV_PRI       0xFF 
#define  OSIntCtxSw()          NVIC_INT_CTRL = NVIC_PENDSVSET

#define  DEF_INT_32U_MAX_VAL        4294967295u
#define  DEF_OCTET_NBR_BITS         8u
#define  DEF_OCTET_MASK             0xFFu
#define  OS_CPU_CFG_SYSTICK_PRIO    0u

#define  CPU_REG_NVIC_ST_CTRL                     (*((CPU_REG32 *)(0xE000E010))) 
#define  CPU_REG_NVIC_ST_CTRL_COUNTFLAG           0x00010000
#define  CPU_REG_NVIC_ST_CTRL_CLKSOURCE           0x00000004
#define  CPU_REG_NVIC_ST_CTRL_TICKINT             0x00000002
#define  CPU_REG_NVIC_ST_CTRL_ENABLE              0x00000001
#define  DEF_INT_CPU_U_MAX_VAL                    DEF_INT_32U_MAX_VAL
#define  DEF_INT_CPU_NBR_BITS                     (CPU_CFG_DATA_SIZE     * DEF_OCTET_NBR_BITS)
#define  CPU_WORD_SIZE_32                              4        /* 32-bit word size (in octets).                        */
#define  CPU_CFG_DATA_SIZE              CPU_WORD_SIZE_32


#define  DEF_INT_32U_MAX_VAL        4294967295u
#define  DEF_OCTET_NBR_BITS         8u
#define  CPU_REG_NVIC_ST_RELOAD      (*((CPU_REG32 *)(0xE000E014)))             /* SysTick Reload      Value Reg.       */
#define  CPU_REG_NVIC_SHPRI3         (*((CPU_REG32 *)(0xE000ED20)))             /* System Handlers 12 to 15 Prio.       */
#define  CPU_REG_NVIC_SHPRI2         (*((CPU_REG32 *)(0xE000ED1C)))             /* System Handlers  8 to 11 Prio.       */
#define  DEF_BIT(bit)                (1u << (bit))
#define  DEF_BIT_FIELD(bit_field, bit_shift)                                 ((((bit_field) >= DEF_INT_CPU_NBR_BITS) ? (DEF_INT_CPU_U_MAX_VAL)     \
                                                                                                                     : (DEF_BIT(bit_field) - 1uL)) \
                                                                                                                            << (bit_shift))
#define  DEF_BIT_MASK(bit_mask, bit_shift)                                     ((bit_mask) << (bit_shift))


#define  CPU_SR_ALLOC()             CPU_SR  cpu_sr = (CPU_SR)0
#define  CPU_INT_DIS()         do { cpu_sr = CPU_SR_Save(); } while (0) /* Save    CPU status word & disable interrupts.*/
#define  CPU_INT_EN()          do { CPU_SR_Restore(cpu_sr); } while (0) /* Restore CPU status word.                     */
#define  CPU_CRITICAL_ENTER()  do { CPU_INT_DIS(); barrier();} while (0)          /* Disable   interrupts */
#define  CPU_CRITICAL_EXIT()   do { barrier();CPU_INT_EN();  } while (0)          /* Re-enable interrupts */


void CPU_IntDis(void);
void CPU_IntEn(void);
void set_register(void **stack_ptr,void *entry_ptr,void *return_ptr,void *para);
void __init_systick(void);
void __init_svc(void);
CPU_SR      CPU_SR_Save      (void);
void        CPU_SR_Restore   (CPU_SR   cpu_sr);
void bsp_init(void);

#endif
