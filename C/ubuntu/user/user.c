#include <user.h>
#include <osinit.h>
#include <os_delay.h>


extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;

#if 0
void three(void *para)
{
	ka_printf("go into task three\n");
	ka_printf("get out of task three\n");
}
void four(void *para)
{
	while(1)
	{
		ka_printf("four\n");
		suspend();
	}
}

void five(void *para)
{
int i,j;
	while(1)
	{
			//ka_printf("55\n");
		for(i=0;i<5000;++i)
		{
			for(j=0;j<1000;++j)
			{
				;
			}
		}
	}
}

void six(void *para)
{
int i,j;
	while(1)
	{
			ka_printf("six\n");
		for(i=0;i<5000;++i)
		{
			for(j=0;j<1000;++j)
			{
				;
			}
		}
		
	}
}
#endif

#if 0 /* test timer */
static void ka(void *para)
{
	ka_printf("task three timer test\n");
}
struct timer kaka;
void three(void *para)
{
	
	if(timer_init(&kaka,TIMER_PERIODIC,"test",ka,NULL,15,10) < 0)
	{
		ka_printf("fatal error\n");
	}
	if(timer_enable(&kaka) < 0)
	{
		ka_printf("fatal error\n");
	}
	suspend();
	while(1)
	{

	}
}
void four(void *para)
{
	while(1)
	{
		ka_printf("four\n");
		sleep(HZ);
	}
}
#endif

#if 0 //test timer
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

#endif

#if 0 /* test breakpoint */
void three(void *para)
{
	int i = 5;
	int ia = 7;
	double kaka = 6.66;
	shell_insert_variable_INT("i",&i);
	shell_insert_variable_INT("ia",&ia);
	shell_insert_variable_FLOAT("kaka",&kaka);
	ka_printf("address of i is 0x%p\n",&i);
	ka_printf("address of ia is 0x%p\n",&ia);
	ka_printf("address of kaka is 0x%p\n",&kaka);
	ka_printf("i is %d\n",i);
	ka_printf("ia is %d\n",ia);
	ka_printf("kaka is %f\n",kaka);
	INSERT_BREAK_POINT();
	ka_printf("i is %d\n",i);
	ka_printf("ia is %d\n",ia);
	ka_printf("kaka is %f\n",kaka);
	INSERT_BREAK_POINT();
	ka_printf("i is %d\n",i);
	ka_printf("ia is %d\n",ia);
	ka_printf("kaka is %f\n",kaka);
	while(1)
	{
		ka_printf("three\n");	
		sleep(10);	
	}	
}
#endif

#if 0
void six(void *para)
{
	
}
void five(void *para)
{
	while(1)
	{
		ka_printf("five\n");	
		sleep(20);	
	}	
}
void four(void *para)
{
	while(1)
	{
		ka_printf("four\n");	
		sleep(30);	
	}	
}
void three(void *para)
{
		void *ptr[64];
int i;
	for(i=0;i<64;++i)
	{
		ptr[i] = ka_malloc(sizeof(int));
		ka_printf("%d get ptr address is %x \n",i,(int)ptr[i]);
		ka_printf("value %d is %x\n",i,*(int *)ptr[i]);
		*(int *)ptr[i] = 66;
	}
	
	void *kaka = ka_malloc(48);
	ka_printf("ptr--kaka get ptr address is %x \n",(int)kaka);
	
	//sleep(300);
	
	ka_free(kaka);
	ka_printf("free complete\n");
	ka_free(ptr[24]);
	ka_printf("free complete\n");
	
	ka_free(ptr[31]);
	ka_printf("free complete\n");
	ka_free(ptr[7]);
	
	ka_free(ptr[62]);
	
	ka_printf("free complete\n");
	
	kaka = ka_malloc(120);
	ka_printf("ptr--kaka get ptr address is %x \n",(int)kaka);
	
	//sleep(300);
	
	ka_free(kaka);
	ka_printf("free kaka complete\n");
	
	while(1)
	{
	}	
}
#endif

#if 0 // test int sleep(unsigned int)
#include <myassert.h>
void three(void *para)
{
	
	TCB *TCB_ptr4,*TCB_ptr5;
	if(0 != task_creat_ready(256,5,5,"five",five,NULL,&TCB_ptr5))
	{
		ka_printf("os_init_fail...stop booting...\n");
		while(1);
	}
	if(0 != task_creat_ready(256,5,5,"four",four,NULL,&TCB_ptr4))
	{
		ka_printf("os_init_fail...stop booting...\n");
		while(1);
	}
	ka_printf("now task three going to sleep\n");
	int ret = sleep(3*HZ);
	ka_printf("task three ret is %d\n",ret);
/*
	ka_printf("task three ret is %d\n",ret);
	if(0 != _remove_from_delay_heap(TCB_ptr5))
	{
		ka_printf("fatal error\n");
	}
	if(0 != _remove_from_delay_heap(TCB_ptr4))
	{
		ka_printf("fatal error\n");
	}
	suspend();
	while(1);
*/
}

void five(void *para)
{
	int ret = sleep(5*HZ);
	ka_printf("task five ret is %d\n",ret);
	suspend();
	while(1);
}
void four(void *para)
{
	int ret = sleep(4*HZ);
	ka_printf("task four ret is %d\n",ret);
	suspend();
	while(1);
}
#endif

#if 0 // test message queue
MQB MQB_test;

void three(void *para)
{
	int i;
	msg_init(&MQB_test,"test",3);
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
	while(1);
}
	

void four(void *para)
{
	struct message *message_ptr;
	while(1)
	{
		if(0 != msg_receive(&MQB_test,&message_ptr,MSG_FLAG_WAIT,0))
		{
			ka_printf("msg_send error\n");
		}
		else
		{
			ka_printf("task four get a message\n");
			ka_printf("size is %u\n",message_ptr->message_size);
			ka_printf("data is %u\n",(unsigned int)(message_ptr->message_data_ptr));
			ka_printf("tick is %l\n",message_ptr->tick);
			ka_printf("=====================================");
		}
	}
}
#endif


#if 0 //test mutex
MUTEX kaka;

void three(void *para)
{
	sleep(10*HZ);
	mutex_lock(&kaka);
	ka_printf("task three success to lock mutex\n");
	ka_printf("now task three prio is %u\n",OSTCBCurPtr->prio);
	ka_printf("release lock\n");
	int ret = mutex_unlock(&kaka);
	ka_printf("task three unlock ret is %d\n",ret);
	ka_printf("now task three prio is %u\n",OSTCBCurPtr->prio);
}

void four(void *para)
{
	sleep(5*HZ);
	mutex_lock(&kaka);
	ka_printf("task four success to lock mutex\n");
	ka_printf("now task four prio is %u\n",OSTCBCurPtr->prio);
	ka_printf("release lock\n");
	int ret = mutex_unlock(&kaka);
	ka_printf("task four unlock ret is %d\n",ret);
	ka_printf("now task four prio is %u\n",OSTCBCurPtr->prio);
}

void five(void *para)
{
	mutex_init(&kaka);
	int ret = mutex_lock(&kaka);
	if(0 > ret)
	{
		ka_printf("lock fail\n");
	}
	else
	{
		ka_printf("task five success to lock mutex\n");
	}
	sleep(7*HZ);
	ka_printf("now task five prio is %u\n",OSTCBCurPtr->prio);
	sleep(5*HZ);
	ka_printf("now task five prio is %u\n",OSTCBCurPtr->prio);
	ret = mutex_unlock(&kaka);
	ka_printf("task five unlock ret is %d\n",ret);
	ka_printf("now task five prio is %u\n",OSTCBCurPtr->prio);
}

#endif
#if 0 //test delay_ms
#include "bsp_led.h"  
void three(void *para)
{
	LED_GPIO_Config();

	LED1_OFF;
	delay_ms(10);
	LED1_ON;

}
#endif

#if 0 //test context switch's time
#include "bsp_led.h"
void three(void *para)
{
	LED_GPIO_Config();
	LED1_OFF;
	sleep(100);
	//suspend();
	while(1);
}

void four(void *para)
{
	LED1_ON;
	while(1);
}
//void five(void *para)
//{
//	while(1);
//}
//void six(void *para)
//{
//	while(1);
//}
#endif

#if 0 //test new memory management

void three(void *para)
{
	void *ptr[31];
	int i;
	_get_os_buddy_ptr_head();
	ka_printf("%p\n",_alloc_power2_page());
	ka_printf("%p\n",_alloc_power3_page());
	ka_printf("%p\n",_alloc_power4_page());
	ka_printf("%p\n",_alloc_power6_page());
	while(1)
	{
		for(i=0;i<31;++i)
		{
			ptr[i] = ka_malloc(500);
			ka_printf("ptr[%d] is %p\n",i,ptr[i]);
		}
		sleep(200 * HZ);
		for(i=0;i<31;++i)
		{
			ka_free(ptr[i]);
		}
		ka_printf("free complete\n");
		sleep(500 * HZ);
	}
}

#endif

#if 0 //test delete_MCB()

MCB kaka;

void three(void *para)
{
	init_MCB(&kaka,0);
	ka_printf("task three going to p()\n");
	int ret = p(&kaka,MCB_FLAG_WAIT,0);
	ka_printf("task three ret is %d\n",ret);
}

void four(void *para)
{
	ka_printf("task four going to p()\n");
	int ret = p(&kaka,MCB_FLAG_WAIT,0);
	ka_printf("task four ret is %d\n",ret);
}

void five(void *para)
{
	ka_printf("task five going to delete_MCB()\n");
	delete_MCB(&kaka);
	ka_printf("delete completed\n");
}

#endif

#if 0 //test msg_del()

MQB kaka;

void three(void *para)
{
	struct message *a;
	msg_init(&kaka,"kaka",8);
	ka_printf("task three going to msg_receive()\n");
	int ret = msg_receive(&kaka,&a,MSG_FLAG_WAIT,0);
	ka_printf("task three ret is %d\n",ret);
}

void four(void *para)
{
	struct message *a;
	ka_printf("task four going to msg_receive()\n");
	int ret = msg_receive(&kaka,&a,MSG_FLAG_WAIT,0);
	ka_printf("task four ret is %d\n",ret);
}

void five(void *para)
{
	ka_printf("task five going to msg_del()\n");
	msg_del(&kaka);
	ka_printf("delete completed\n");
}

#endif

#if 0 //test mutex_del()

MUTEX kaka;

void three(void *para)
{
	mutex_init(&kaka);
	sleep(5*HZ);
	ka_printf("task three going to mutex_lock()\n");
	int ret = mutex_lock(&kaka);
	ka_printf("task three ret is %d\n",ret);
}

void four(void *para)
{
	sleep(5*HZ);
	ka_printf("task four going to mutex_lock()\n");
	int ret = mutex_lock(&kaka);
	ka_printf("task four ret is %d\n",ret);
}

void five(void *para)
{
	ka_printf("task five going to mutex_lock()\n");
	mutex_lock(&kaka);
	sleep(10*HZ);
	ka_printf("task five going to mutex_del()\n");
	mutex_del(&kaka);
	ka_printf("delete completed\n");
}

#endif

#if 0 //test MCB

MCB kaka;

void three(void *para)
{
	init_MCB_binary(&kaka,0);
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
	p(&kaka,MCB_FLAG_WAIT,0);
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
	p(&kaka,MCB_FLAG_WAIT,0);
	ka_printf("task five sleep()\n");
	sleep(50);
	ka_printf("task five v()\n");
	v(&kaka);
	ka_printf("task five v()\n");
	v(&kaka);
}

#endif

#if 0//test malloc
void three(void *para)
{
	void *ptr[10];
	unsigned int i;
	ka_printf("going to task three\n");
	for(i=0;i<10;++i)
	{
		ptr[i] = ka_malloc(700);
		ka_printf("i is %u,address is %p\n",i,ptr[i]);
	}
	INSERT_BREAK_POINT();
	ka_printf("release address\n");
	for(i=0;i<10;++i)
	{
		ka_free(ptr[i]);
		INSERT_BREAK_POINT();
	}
	INSERT_BREAK_POINT();
	ka_printf("end\n");
}

#endif

#if 0 /* test vfs */

#include <bsp_support.h>
static int open(struct file *file_ptr)
{
	ka_printf("open file %s\n",file_ptr->f_den->name);
	USART2_Config();
	return 0;
}

static int close(struct file *file_ptr)
{
	ka_printf("bye bye file %s\n",file_ptr->f_den->name);
	return 0;
}

static int write(struct file *file_ptr,void *buffer,unsigned int len,unsigned int offset)
{
	char *data = (char *)buffer;
	unsigned int i;
	for(i=0;i<len;++i)
	{
		ka_printf("round %u,write %c\n",i,data[i]);
		Usart_SendByte(USART2,data[i]);
	}
	return len;
}

static struct file_operations fop = {
	.open = open,
	.close = close,
	.write = write,
};

void three(void *para)
{
	if(writeonly_device_register("usart2",&fop) < 0)
	{
		ka_printf("device register error\n");
	}
	else
	{
		ka_printf("device register successfully\n");
	}
	sleep(20 * HZ);
	if(device_unregister("usart2") < 0)
	{
		ka_printf("unregister fail\n");
	}
	else
	{
		ka_printf("unregister success\n");
	}
}

#endif

#if 1 /* test mem pool */

void three(void *para)
{
	mp *mp_ptr;
	int error = create_mp("kaka",11,100,&mp_ptr);
	if(error < 0)
	{
		ka_printf("error is %d\n",error);
		return ;
	}
	void *ptr[30];
	unsigned int i;
	for(i=0;i<30;++i)
	{
		ptr[i] = mp_alloc(mp_ptr,3 * HZ);
		ka_printf("%u\n",i);
	}
	for(i=0;i<30;++i)
	{
		ka_printf("ptr[%u] is %p\n",i,ptr[i]);
	}
	sleep(10 * HZ);
	ka_printf("release pools\n");
	for(i=0;i<30;++i)
	{
		mp_free(ptr[i]);
	}
	ka_printf("complete\n");
	error = create_mp("qq",11,100,&mp_ptr);
}

#endif
