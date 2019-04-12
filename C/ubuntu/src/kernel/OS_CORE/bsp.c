#include <bsp.h>
#include <myMicroLIB.h>


void _bsp_init(const struct device *device_array,unsigned int num)
{
	unsigned int i;
	for(i=0;i<num;++i)
	{
		switch(device_array[i].head.type)
		{
			case DEV_NORMAL:
				device_array[i].u.normal.init_fun();
				ka_printf("type is ");
				ka_printf("DEV_NORMAL\n");
				break;
			case DEV_MEM:
				device_array[i].u.mem.init_fun(device_array[i].u.mem.para);
				ka_printf("type is ");
				ka_printf("DEV_MEM\n");
				break;
			default :
				ka_printf("error!!!\n");break;
		}
		ka_printf("dev_name is %s\n",device_array[i].head.dev_name);
		if(NULL != device_array[i].head.dev_info)
		{
			ka_printf("dev_info is %s\n",device_array[i].head.dev_info);
		}
	}
}
