# KAKAOS内核代码

## 介绍 ##
* 因为我是卡卡的球迷，故将该OS取名为KAKAOS，虽然名字如此，但这OS用起来一点都不卡。KAKAOS目前超过11000行代码，90个文件，支持的MCU有stm32f103、stm32f407。</br>
* 本项目使用GCC进行编译，测试时使用的GCC版本是gcc-arm-none-eabi-7-2018-q2。</br>
* OS全部编译为库后生成的二进制代码大小约为 100k，通过裁剪保留基本的IPC、调度、内存管理和时间功能最小可以缩小到低于40K</br>
* OS工程里使用了某些公司的BSP提供某些功能的驱动</br>
* 内核的入口函数为start_kernel（）</br>
* 源码中分别包含了windows下和linux下的工程，其中windows下的工程使用keil作为IDE，工程文件位于C\windows\Project\下，linux下使用make工具，以类似于linux工程的形式进行工程的构建，比如编译stm32f103的工程，先输入命令make stm32f103_config，然后make，具体参考主makefile下的各项配置；而且，linux下的工程代码通过shell脚本的帮助可以更轻松地对内核进行配置和裁剪</br>
* 极短的任务切换时间，在stm32f103zet6平台下以两个任务切换测试为例，挂起函数suspend()上下文切换时长为6.94us，延时函数sleep()上下文切换时长为12.35us；作为对比，ucosiii挂起函数上下文切换时长为15.3us，延时函数上下文切换时长为16.3us</br>
* ./KAKAOS.png为本文件导出的思维导图

## 内核包括的模块及功能简介：

### IPC模块： ###
* 信号量MCB，包括二值信号量及计数信号量，基本操作有P、V操作</br>
* 消息队列MQB，基本操作有发送消息msg_send（）和接收消息msg_receive（）</br>
* 互斥锁MUTEX，基本操作包括mutex_lock（），mutex_try_lock（），mutex_unlock（）</br>

### 调度模块：  ###
* 延时函数sleep（）</br>
* 挂起函数suspend（）</br>
* 任务创建函数task_creat_ready（）和task_init_ready（）</br>
* 用新函数覆盖原线程函数exec（）</br>
* 实现了基于抢占式和时间片的调度方法，系统共64级优先级，从0-63，相同优先级的任务按创建任务时设计的时间片调度，不同优先级任务直接按抢占式调度方法调度</br>
* 某些优先级上已有操作系统必要的任务存在</br>
		
### 时间模块 ###
* 时间显示system_time_display（）</br>
* 时间设置set_time（）</br>
* 定时器功能，包括timer_enable（），timer_disable（）</br>
		
### C库模块： ###
* 包括基本的输出函数ka_printf（）</br>
* 基本的字符串操作函数ka_strlen（）、ka_strcpy（）等</br>
		
### 内存管理模块 ###
* 以buddy算法和slab分配器为基础搭建的管理系统</br>
* 包括基本的内存申请及释放函数ka_malloc（）和ka_free（）</br>
* 支持不连续内存的管理</br>
* 另外还有内存池管理系统，可以与buddy相结合或相独立</br>
* 内存池操作函数init_mp（），create_mp（），mp_alloc（），mp_free（）</br>
		
### shell命令窗口 ###
* 该命令窗口经由串口1与用户交互</br>
* 实现的shell命令位于shell.c中</br>
* tab键有加速访问的功能</br>
		
### 动态模块： ###
* 该功能需配合串口2和shell命令实现，相关shell命令为insmod</br>
* 用户根据module_tool目录下的模板生成.so或.ko文件，默认利用串口2将该文件传输至mcu，即可运行动态模块</br>
* 建议一般情况下使用.KO模块，实测时该模块占用空间更小</br>
* 模块中必须包含main函数作为模块的入口函数，而module_init宏和module_exit宏作为模块初始化和退出的函数，在模块运行前后分别被调用</br>
* 模块中参考了RT-thread内核的代码，侵删</br>

### 虚拟文件系统模块： ###
* 实现了一个简单的抽象文件系统，可以mount实际文件系统</br>
* 使用device_register（）相关函数可以向VFS注册设备，设备文件将在/dev/目录下生成，可以通过cat、echo命令操控设备</br>
* 使用fs_register（）函数向系统注册实际文件系统，可以指定mount的位置，笔者已成功适配fat文件系统</br>
* 必须启动虚拟文件系统之后才能挂载实际的文件系统</br>
