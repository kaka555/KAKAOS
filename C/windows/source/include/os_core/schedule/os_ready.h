#ifndef _OS_READY_H
#define _OS_READY_H

#include <TCB.h>

int _insert_ready_TCB(TCB *TCB_ptr);
int _delete_TCB_from_ready(TCB *TCB_ptr);
TCB *_get_highest_prio_ready_TCB(void);
void __init_ready_group(void);
void shell_check_os_ready(void);

#endif
