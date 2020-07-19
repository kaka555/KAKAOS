/* test timer */
#include <user.h>
#include <osinit.h>
#include <os_delay.h>

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;

static void ka(void *para)
{
	ka_printf("task three timer test\n");
}
struct timer kaka;
void three(void *para)
{

	if (timer_init(&kaka, TIMER_PERIODIC, "test", ka, NULL, 15, 10) < 0)
	{
		ka_printf("fatal error\n");
	}
	if (timer_enable(&kaka) < 0)
	{
		ka_printf("fatal error\n");
	}
	suspend();
	while (1)
	{

	}
}
void four(void *para)
{
	while (1)
	{
		ka_printf("four\n");
		sleep(HZ);
	}
}
