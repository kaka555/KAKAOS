#include <bsp.h>
#include <buddy.h>
#include <bsp_support.h>
#include <myMicroLIB.h>

static const struct dev_mem_para para1 = {
	.start = (void *)0x10000000,
	.size = 64, /* KB*/
	.prio = 2,
	.type = TYPE_SYS,
};

static const struct dev_mem_para para2 = {
	.start = (void *)0x20000000,
	.size = 128, /* KB*/
	.prio = 1,
	.type = TYPE_NORMAL,
};

static const struct device device_array[] = {

	{
		.head.dev_name = "USART1",
		.head.dev_info = NULL,
		.head.type = DEV_NORMAL,
		.u.normal.init_fun = uart1_init,
	},

	{
		.head.dev_name = "USART2",
		.head.dev_info = NULL,
		.head.type = DEV_NORMAL,
		.u.normal.init_fun = uart2_init,
	},

	{	
		.head.dev_name = "first mem",
		.head.dev_info = NULL,
		.head.type = DEV_MEM,
		.u.mem.init_fun = __buddy_init,
		.u.mem.para = &para1,
	},

	{	
		.head.dev_name = "second mem",
		.head.dev_info = NULL,
		.head.type = DEV_MEM,
		.u.mem.init_fun = __buddy_init,
		.u.mem.para = &para2,
	}
	
};

void ka_putchar(const char ch)  
{
	USART_SendData(USART1,ch);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void bsp_init(void)
{
	uart1_init();
	_bsp_init(device_array,sizeof(device_array)/sizeof(struct device));
}
