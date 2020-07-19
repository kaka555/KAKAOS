//test msg_del()
#include <user.h>
#include <osinit.h>
#include <os_delay.h>

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;

MQB kaka;

void three(void *para)
{
	struct message *a;
	msg_init(&kaka, "kaka", 8);
	ka_printf("task three going to msg_receive()\n");
	int ret = msg_receive(&kaka, &a, MSG_FLAG_WAIT, 0);
	ka_printf("task three ret is %d\n", ret);
}

void four(void *para)
{
	struct message *a;
	ka_printf("task four going to msg_receive()\n");
	int ret = msg_receive(&kaka, &a, MSG_FLAG_WAIT, 0);
	ka_printf("task four ret is %d\n", ret);
}

void five(void *para)
{
	ka_printf("task five going to msg_del()\n");
	msg_del(&kaka);
	ka_printf("delete completed\n");
}
