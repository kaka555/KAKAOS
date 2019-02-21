#ifndef _OS_SUSPEND_H
#define _OS_SUSPEND_H

#include <TCB.h>

void __init_suspend_list(void);
int  insert_into_suspend_list(TCB *const TCB_ptr);
int  remove_from_suspend_list(TCB *const TCB_ptr);

#endif
