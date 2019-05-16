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

/* macro of struct dev_mem_para's member-->prio */
enum mem_type
{
	TYPE_NORMAL,
	TYPE_SYS /* the os data store in memory with prio PRIO_SYS */
};

struct dev_mem_para{
	unsigned int prio;
	enum mem_type type;
	UINT32 size; /*KB  must be 2^n*/
	void *start;
};

struct dev_mem{
	void (*init_fun)(const struct dev_mem_para *para);
	const struct dev_mem_para *para;
};

struct device_head{
	const char *dev_name;
	const char *dev_info;
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
