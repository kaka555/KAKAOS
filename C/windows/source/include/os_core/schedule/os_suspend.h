#ifndef _OS_SUSPEND_H
#define _OS_SUSPEND_H

#include <TCB.h>

void _insert_into_suspend_list(TCB *const TCB_ptr);
int  _remove_from_suspend_list(TCB *const TCB_ptr);

#endif
