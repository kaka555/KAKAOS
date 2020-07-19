//test MCB
#include <user.h>
#include <osinit.h>
#include <os_delay.h>

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;

MCB kaka;

void three(void *para)
{
	init_MCB_binary(&kaka, 0);
//	ka_printf("task three p()\n");
//	p(&kaka,MCB_FLAG_WAIT,0);
//	ka_printf("task three p()\n");
//	p(&kaka,MCB_FLAG_WAIT,0);
//	ka_printf("task three sleep()\n");
//	sleep(50);
	ka_printf("task three v()\n");
	v(&kaka);
	ka_printf("task three v()\n");
	v(&kaka);
}

void four(void *para)
{
//	ka_printf("task four p()\n");
//	p(&kaka,MCB_FLAG_WAIT,0);
	ka_printf("task four p()\n");
	p(&kaka, MCB_FLAG_WAIT, 0);
	ka_printf("task four sleep()\n");
	sleep(50);
	ka_printf("task four v()\n");
	v(&kaka);
	ka_printf("task four v()\n");
	v(&kaka);
}

void five(void *para)
{
//	ka_printf("task five p()\n");
//	p(&kaka,MCB_FLAG_WAIT,0);
	ka_printf("task five p()\n");
	p(&kaka, MCB_FLAG_WAIT, 0);
	ka_printf("task five sleep()\n");
	sleep(50);
	ka_printf("task five v()\n");
	v(&kaka);
	ka_printf("task five v()\n");
	v(&kaka);
}
