# 移植必须实现的函数 #

* void OSStartHighRdy(void); 
	这个函数是系统初始化时最后一个调用的函数</br>
	
* 上下文切换函数

* void set_register(void **stack_ptr,void *entry_ptr,void *return_ptr,void *para);
	这个函数用于配置线程初始时的寄存器值</br>
	
* 时钟初始化函数void __init_systick(void);

* 进出临界区函数 void CPU_CRITICAL_ENTER(void) 和 void CPU_CRITICAL_EXIT(void)

* void bsp_init(void);

* kakaosstdint.h文件里的所有typedef

# 移植可选实现的函数 #
	reboot函数