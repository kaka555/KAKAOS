//test delete_MCB()
#include <user.h>
#include <osinit.h>
#include <os_delay.h>

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;

MCB kaka;

void three(void *para)
{
	init_MCB(&kaka, 0);
	ka_printf("task three going to p()\n");
	int ret = p(&kaka, MCB_FLAG_WAIT, 0);
	ka_printf("task three ret is %d\n", ret);
}

void four(void *para)
{
	ka_printf("task four going to p()\n");
	int ret = p(&kaka, MCB_FLAG_WAIT, 0);
	ka_printf("task four ret is %d\n", ret);
}

void five(void *para)
{
	ka_printf("task five going to delete_MCB()\n");
	delete_MCB(&kaka);
	ka_printf("delete completed\n");
}
