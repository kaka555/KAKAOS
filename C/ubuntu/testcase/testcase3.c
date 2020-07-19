//test timer
#include <user.h>
#include <osinit.h>
#include <os_delay.h>

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;

static void ka(void *para)
{
	ka_printf("task three timer test ka\n");
}

static void kb(void *para)
{
	ka_printf("task four timer test kb\n");
}
struct timer timer1;
struct timer timer0;
void three(void *para)
{
	ka_printf("task three\n");
	timer_init(&timer0, TIMER_ONE_TIME, "T1", ka, 0, 15, 8);
	timer_enable(&timer0);
	suspend();
}

void four(void *para)
{
	ka_printf("task four\n");
	timer_init(&timer1, TIMER_TIME, "T2", kb, 0, 15, 8);
	timer_enable(&timer1);
	suspend();
}
