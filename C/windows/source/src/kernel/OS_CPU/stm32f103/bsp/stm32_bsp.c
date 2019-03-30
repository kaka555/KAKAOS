#include <bsp.h>
#include <buddy.h>
#include <bsp_support.h>
#include <myMicroLIB.h>

static const struct dev_mem_para para1 = {
	.start = (void *)0x20000000,
	.size = 64, /* KB*/
	.prio = 1,
};

static const struct dev_mem_para para2 = {
	.start = (void *)0x68000000,
	.size = 1024, /*KB*/
	.prio = 2,
};

static const struct device device_array[] = {

	{
		.head.dev_name = "USART1",
		.head.dev_info = NULL,
		.head.type = DEV_NORMAL,
		.u.normal.init_fun = USART_Config,
	},

	{
		.head.dev_name = "SRAM",
		.head.dev_info = NULL,
		.head.type = DEV_NORMAL,
		.u.normal.init_fun = FSMC_SRAM_Init,
	},

	{
		.head.dev_name = "USART2",
		.head.dev_info = NULL,
		.head.type = DEV_NORMAL,
		.u.normal.init_fun = USART2_Config,
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
	USART_SendData(DEBUG_USARTx,ch);
	while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);
}

void bsp_init(void)
{
	USART_Config();
	_bsp_init(device_array,sizeof(device_array)/sizeof(struct device));
}
