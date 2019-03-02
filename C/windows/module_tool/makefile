src = kaka.c led.c

HOME = /home/ka/KAKAOS-0.07

HEAD_DIR1 = ${HOME}/include 
HEAD_DIR2 = ${HOME}/src/FWlib/inc 
HEAD_DIR3 = ${HOME}/include/os_base
HEAD_DIR4 = ${HOME}/include/os_core
HEAD_DIR5 = ${HOME}/include/os_core/schedule
HEAD_DIR6 = ${HOME}/include/os_cpu
HEAD_DIR7 = ${HOME}/include/os_cpu/bsp
HEAD_DIR8 = ${HOME}/include/os_lib
HEAD_DIR9 = ${HOME}/include/shell

INCLUDE = -I$(HEAD_DIR1) -I$(HEAD_DIR2) -I$(HEAD_DIR3) -I$(HEAD_DIR4) -I$(HEAD_DIR5) -I$(HEAD_DIR6) -I$(HEAD_DIR7) -I$(HEAD_DIR8) -I$(HEAD_DIR9) 

FLAGS = -W -Wall $(INCLUDE) -mcpu=cortex-m3 -mthumb -fno-builtin -ffunction-sections -fdata-sections  

LIBPATH = /home/ka/stm32/gcc-arm-none-eabi-5_4-2016q3/arm-none-eabi/lib/armv7-m/ -L/home/ka/stm32/gcc-arm-none-eabi-5_4-2016q3/lib/gcc/arm-none-eabi/5.4.1/armv7-m/

LFLAGS = -z max-page-size=0x4 -Bsymbolic -fno-common -nostartfiles -nostdlib -shared -L$(LIBPATH) -T stm32_flash.ld -e main -static -lgcc

all:module.so

module.so : $(src:.c=.o)
	arm-none-eabi-ld $(LFLAGS) $^ -o $@
	arm-none-eabi-strip -R .hash $@

%.o : %.c
	arm-none-eabi-gcc $(FLAGS)  -fPIC -mlong-calls -c $< -o $@
	#arm-none-eabi-gcc $(FLAGS)  -mword-relocations -mlong-calls -c $< -o $@

clean:
	rm -rf *.o *.so

