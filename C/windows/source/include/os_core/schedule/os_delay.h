#ifndef _OS_DELAY_H
#define _OS_DELAY_H

#include "heap_oo.h"
#include "TCB.h"

int  _insert_into_delay_heap(TCB *TCB_ptr);
int  _remove_from_delay_heap(TCB *TCB_ptr);
TCB* _delay_heap_get_top_TCB(void);
TCB* _delay_heap_remove_top_TCB(void);
void shell_delay_heap_check(void);

#endif
