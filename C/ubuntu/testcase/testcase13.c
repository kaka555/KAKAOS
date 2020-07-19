//test mutex_del()
#include <user.h>
#include <osinit.h>
#include <os_delay.h>

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;

MUTEX kaka;

void three(void *para)
{
	mutex_init(&kaka);
	sleep(5 * HZ);
	ka_printf("task three going to mutex_lock()\n");
	int ret = mutex_lock(&kaka);
	ka_printf("task three ret is %d\n", ret);
}

void four(void *para)
{
	sleep(5 * HZ);
	ka_printf("task four going to mutex_lock()\n");
	int ret = mutex_lock(&kaka);
	ka_printf("task four ret is %d\n", ret);
}

void five(void *para)
{
	ka_printf("task five going to mutex_lock()\n");
	mutex_lock(&kaka);
	sleep(10 * HZ);
	ka_printf("task five going to mutex_del()\n");
	mutex_del(&kaka);
	ka_printf("delete completed\n");
}
