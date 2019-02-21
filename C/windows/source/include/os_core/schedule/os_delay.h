#ifndef _OS_DELAY_H
#define _OS_DELAY_H

#include "heap_oo.h"
#include "TCB.h"

void __init_delay_heap(void);
int  insert_into_delay_heap(TCB *TCB_ptr);
int  remove_from_delay_heap(TCB *TCB_ptr);
TCB* delay_heap_get_top_TCB(void);
TCB* delay_heap_remove_top_TCB(void);
void shell_delay_heap_check(void);

#endif
