#ifndef _MUTEX_H
#define _MUTEX_H

#include <insert_sort_oo.h>
typedef struct task_control_block_struct TCB;

typedef struct mutex_block{
	unsigned int mutex_flag; /* should be 0 or 1*/
	TCB *owner_TCB_ptr;
	struct insert_sort_entity mutex_insert_sort_TCB_list;
}MUTEX;

enum mutex_state{
	MUTEX_LOCK = 0,
	MUTEX_UNLOCK
};

int _mutex_init(MUTEX *MUTEX_ptr);
int mutex_init(MUTEX *MUTEX_ptr);
int _mutex_lock(MUTEX *MUTEX_ptr);
int mutex_lock(MUTEX *MUTEX_ptr);
int _mutex_try_lock(MUTEX *MUTEX_ptr);
int mutex_try_lock(MUTEX *MUTEX_ptr);
int _mutex_unlock(MUTEX *MUTEX_ptr);
int mutex_unlock(MUTEX *MUTEX_ptr);
int _mutex_del(MUTEX *MUTEX_ptr);
int mutex_del(MUTEX *MUTEX_ptr);

#endif
