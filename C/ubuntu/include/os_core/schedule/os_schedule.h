#ifndef _OS_SCHEDULE_H
#define _OS_SCHEDULE_H

#include <task_state.h>
#include <TCB.h>
#include <os_error.h>

#define sleep(x) 		sys_delay((x),STATE_DELAY)
#define suspend()   	sys_suspend(STATE_SUSPEND_NORMAL)

#define SYS_ENTER_CRITICAL()   do { g_schedule_lock++;} while(0)
#define SYS_EXIT_CRITICAL()   do { g_schedule_lock--;} while(0)

int sys_delay(unsigned int delay_ticks_num,TASK_STATE state);
int sys_suspend(TASK_STATE state);

void schedule(void);

int _must_check task_creat_ready(
	unsigned int stack_size,
	TASK_PRIO_TYPE prio,
	unsigned int timeslice_hope_time,
	const char *name,
	functionptr function,
	void *para,
	TCB **ptr);

int _must_check task_init_ready(
	TCB *TCB_ptr,
	unsigned int stack_size,
	TASK_PRIO_TYPE prio,
	unsigned int timeslice_hope_time,
	const char *name,
	functionptr function,
	void *para);

void sys_schedule_lock(void);
void sys_schedule_unlock(void);
int sys_schedule_islock(void);

#endif
