// test message queue
#include <user.h>
#include <osinit.h>
#include <os_delay.h>

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;


MQB MQB_test;

void three(void *para)
{
	int i;
	msg_init(&MQB_test, "test", 3);
	struct message array[10];
	array[9].tick = 1;
//	for(i=0;i<4;++i)
//	{
//		if(0 != message_init(array+i,4,(void *)i))
//		{
//			ka_printf("message_init error\n");
//		}
//	}
//	for(i=0;i<3;++i)
//	{
//		if(0 != msg_send(&MQB_test,array+i,MSG_FLAG_NON_BLOCKING,456))
//		{
//			ka_printf("msg_send error\n");
//		}
//	}
//	if(0 != msg_send(&MQB_test,array+i,MSG_FLAG_WAIT,20))
//	{
//		ka_printf("msg_send error\n");
//	}
	ka_printf("complete\n");
	suspend();
	while (1);
}


void four(void *para)
{
	struct message *message_ptr;
	while (1)
	{
		if (0 != msg_receive(&MQB_test, &message_ptr, MSG_FLAG_WAIT, 0))
		{
			ka_printf("msg_send error\n");
		}
		else
		{
			ka_printf("task four get a message\n");
			ka_printf("size is %u\n", message_ptr->message_size);
			ka_printf("data is %u\n", (unsigned int)(message_ptr->message_data_ptr));
			ka_printf("tick is %l\n", message_ptr->tick);
			ka_printf("=====================================");
		}
	}
}
