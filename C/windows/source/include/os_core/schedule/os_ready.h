#ifndef _OS_READY_H
#define _OS_READY_H

#include <TCB.h>

int insert_ready_TCB(TCB *TCB_ptr);
int delete_TCB_from_ready(TCB *TCB_ptr);
TCB *get_highest_prio_ready_TCB(void);
void __init_ready_group(void);
void shell_check_os_ready(void);

#endif
