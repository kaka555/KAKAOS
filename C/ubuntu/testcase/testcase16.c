/* test vfs */
#include <user.h>
#include <osinit.h>
#include <os_delay.h>

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;

#include <bsp_support.h>
static int open(struct file *file_ptr)
{
	ka_printf("open file %s\n", file_ptr->f_den->name);
	USART2_Config();
	return 0;
}

static int close(struct file *file_ptr)
{
	ka_printf("bye bye file %s\n", file_ptr->f_den->name);
	return 0;
}

static int write(struct file *file_ptr, void *buffer, unsigned int len, unsigned int offset)
{
	char *data = (char *)buffer;
	unsigned int i;
	for (i = 0; i < len; ++i)
	{
		ka_printf("round %u,write %c\n", i, data[i]);
		Usart_SendByte(USART2, data[i]);
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
	if (writeonly_device_register("usart2", &fop) < 0)
	{
		ka_printf("device register error\n");
	}
	else
	{
		ka_printf("device register successfully\n");
	}
	sleep(20 * HZ);
	if (device_unregister("usart2") < 0)
	{
		ka_printf("unregister fail\n");
	}
	else
	{
		ka_printf("unregister success\n");
	}
}
