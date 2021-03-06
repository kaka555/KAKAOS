#ifndef _TASK_STATE_H
#define _TASK_STATE_H

typedef enum Task_State {
	STATE_READY 						= 1,
	STATE_DELAY							= 2,
	STATE_SUSPEND_NORMAL 				= 3,
	STATE_WAIT_MCB_FOREVER 				= 4,
	STATE_WAIT_MCB_TIMEOUT 				= 5,
	STATE_WAIT_MESSAGE_QUEUE_FOREVER	= 6,
	STATE_WAIT_MESSAGE_QUEUE_TIMEOUT 	= 7,
	STATE_PUT_MESSAGE_QUEUE_FOREVER 	= 8,
	STATE_PUT_MESSAGE_QUEUE_TIMEOUT 	= 9,
	STATE_WAIT_MUTEX_FOREVER			= 10,
	STATE_WAIT_MEM_POOL_TIMEOUT			= 11
}TASK_STATE;

#endif
