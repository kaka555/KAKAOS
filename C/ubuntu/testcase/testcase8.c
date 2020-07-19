//test mutex
#include <user.h>
#include <osinit.h>
#include <os_delay.h>

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;

MUTEX kaka;

void three(void *para)
{
	sleep(10 * HZ);
	mutex_lock(&kaka);
	ka_printf("task three success to lock mutex\n");
	ka_printf("now task three prio is %u\n", OSTCBCurPtr->prio);
	ka_printf("release lock\n");
	int ret = mutex_unlock(&kaka);
	ka_printf("task three unlock ret is %d\n", ret);
	ka_printf("now task three prio is %u\n", OSTCBCurPtr->prio);
}

void four(void *para)
{
	sleep(5 * HZ);
	mutex_lock(&kaka);
	ka_printf("task four success to lock mutex\n");
	ka_printf("now task four prio is %u\n", OSTCBCurPtr->prio);
	ka_printf("release lock\n");
	int ret = mutex_unlock(&kaka);
	ka_printf("task four unlock ret is %d\n", ret);
	ka_printf("now task four prio is %u\n", OSTCBCurPtr->prio);
}

void five(void *para)
{
	mutex_init(&kaka);
	int ret = mutex_lock(&kaka);
	if (0 > ret)
	{
		ka_printf("lock fail\n");
	}
	else
	{
		ka_printf("task five success to lock mutex\n");
	}
	sleep(7 * HZ);
	ka_printf("now task five prio is %u\n", OSTCBCurPtr->prio);
	sleep(5 * HZ);
	ka_printf("now task five prio is %u\n", OSTCBCurPtr->prio);
	ret = mutex_unlock(&kaka);
	ka_printf("task five unlock ret is %d\n", ret);
	ka_printf("now task five prio is %u\n", OSTCBCurPtr->prio);
}
