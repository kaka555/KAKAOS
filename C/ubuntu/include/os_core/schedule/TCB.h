#ifndef _TCB_H
#define _TCB_H

#include <kakaosstdint.h>
#include <double_linked_list.h>
#include <task_state.h>
#include <os_error.h>
#include <os_cpu.h>

typedef  unsigned  int         STACK_TYPE;

typedef  unsigned  int         TASK_PRIO_TYPE;
typedef  void                  (*functionptr)(void *para);

#define SYS_ENTER_INTERRUPT()  do { CPU_IntDis();++g_interrupt_count;CPU_IntEn();} while (0)
#define SYS_EXIT_INTERRUPT()   do { CPU_IntDis();--g_interrupt_count;CPU_IntEn();} while (0)
#define task_change_prio_self(prio)	task_change_prio((TCB *)OSTCBCurPtr,(prio))

extern volatile int g_interrupt_count;

#define PRIO_MAX 64

#define RESERVED_PRIO	0

/**** attribution macro ****/
#define DEFAULT_ATTRIBUTION 	0
/**** bit 0 ****/
/* identify the struct TCB is created or init */
#define TCB_ATTRIBUTION_INIT 				(0X00<<0)
#define TCB_ATTRIBUTION_CREATE 				(0X01<<0)
#define TCB_IS_CREATED(TCB_ptr) 			((TCB_ptr)->attribution & TCB_ATTRIBUTION_CREATE)
/**** bit 1 ****/			
#define TCB_ATTRIBUTION_OK					(0X00<<1)
#define TCB_ATTRIBUTION_BAD					(0X01<<1)
#define TCB_state_is_bad(TCB_ptr)			((TCB_ptr)->attribution & TCB_ATTRIBUTION_BAD)
#define clear_bad_state(TCB_ptr)			((TCB_ptr)->attribution &= ~TCB_ATTRIBUTION_BAD)
#define set_bad_state(TCB_ptr)				((TCB_ptr)->attribution |= TCB_ATTRIBUTION_BAD)
/**** bit 2 ****/	
#define TCB_ATTRIBUTION_NO_NEED_RESCHEDULED (0X00<<2)
#define TCB_ATTRIBUTION_NEED_RESCHEDULED 	(0X01<<2)
#define need_rescheduled()					(OSTCBCurPtr->attribution & TCB_ATTRIBUTION_NEED_RESCHEDULED)
#define clear_rescheduled_flag()			(OSTCBCurPtr->attribution &= ~TCB_ATTRIBUTION_NEED_RESCHEDULED)
#define set_rescheduled_flag()				(OSTCBCurPtr->attribution |= TCB_ATTRIBUTION_NEED_RESCHEDULED)
/**** bit 3 ****/	
#define TCB_ATTRIBUTION_IS_NOT_MODULE 		(0X00<<3)
#define TCB_ATTRIBUTION_IS_DYNAMIC_MODULE 	(0X01<<3)
#define is_module(TCB_ptr)					((TCB_ptr)->attribution & TCB_ATTRIBUTION_IS_DYNAMIC_MODULE)
#define set_module_flag(TCB_ptr)			((TCB_ptr)->attribution |= TCB_ATTRIBUTION_IS_DYNAMIC_MODULE)
#define clear_module_flag(TCB_ptr)			((TCB_ptr)->attribution &= ~TCB_ATTRIBUTION_IS_DYNAMIC_MODULE)
/****end of attribution macro ****/

struct dynamic_module; 
typedef struct task_control_block_struct{
	STACK_TYPE *stack; 						/*the stack top of the task*/
	unsigned int stack_size;				/*bytes*/
	STACK_TYPE *stack_end; 					/*the stack tail of the task*/
	TASK_PRIO_TYPE reserve_prio;			/*used for Priority inversion problem*/
	TASK_STATE task_state;					/*priority 0-PRIO_MAX-1*/
	TASK_PRIO_TYPE prio;
	unsigned int delay_heap_position;
	struct list_head same_prio_list;
	struct list_head suspend_list;
	const char *name;
	unsigned int timeslice_hope_time;
	unsigned int timeslice_rest_time;
	UINT32 attribution;							/*each bit of this element present an attribution*/
	struct dynamic_module *dynamic_module_ptr;	/* the related dynamic module*/
	UINT64 delay_reach_time; 					/* use when task is delayed*/
}TCB;

/*To creat a task, use one of the two following function */
TCB * _must_check _task_creat(
	unsigned int stack_size,
	TASK_PRIO_TYPE prio,
	unsigned int timeslice_hope_time,
	const char *name,
	functionptr function,
	void *para,
	TASK_STATE state);/*this function use malloc to creat room for TCB*/

int _must_check _task_init(
	TCB *TCB_ptr,
	unsigned int stack_size,
	TASK_PRIO_TYPE prio,
	unsigned int timeslice_hope_time,
	const char *name,
	functionptr function,
	void *para,
	TASK_STATE state);

int _task_change_prio(TCB *TCB_ptr,TASK_PRIO_TYPE prio);
int task_change_prio(TCB *TCB_ptr,TASK_PRIO_TYPE prio);
int _task_delete(TCB *TCB_ptr);
int task_delete(TCB *TCB_ptr);

static inline void set_tcb_module(TCB *TCB_ptr,struct dynamic_module *mod_ptr)
{
	TCB_ptr->dynamic_module_ptr = mod_ptr;
}

#endif
