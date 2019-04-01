#ifndef _BSP_H
#define _BSP_H
#include <kakaosstdint.h>

enum device_type{
	DEV_NORMAL	= 0,
	DEV_MEM		= 1
};

struct dev_normal{
	void (*init_fun)(void);
};

struct dev_mem_para{
	unsigned int prio; /*1 is internal memory, others is external memory*/
	void *start;
	UINT32 size; /*KB  must be 2^n*/
};

struct dev_mem{
	void (*init_fun)(const struct dev_mem_para *para);
	const struct dev_mem_para *para;
};

struct device_head{
	char *dev_name;
	char *dev_info;
	enum device_type type;
};

struct device
{
	struct device_head head;
	union{
		struct dev_normal normal;
		struct dev_mem mem;
	}u;
};

void _bsp_init(const struct device *device_array,unsigned int num);

#endif
