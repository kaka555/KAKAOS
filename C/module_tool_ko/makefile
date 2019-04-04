src = kaka.c

HOME = /home/ka/KAKAOS_0.09

PROJECT = stm32f407

export HEAD_DIR1  = ${HOME}/include 
export HEAD_DIR2  = ${HOME}/src/kernel/OS_CPU/$(PROJECT)/FWlib/inc 
export HEAD_DIR3  = ${HOME}/include/os_base
export HEAD_DIR4  = ${HOME}/include/os_core
export HEAD_DIR5  = ${HOME}/include/os_core/schedule
export HEAD_DIR6  = ${HOME}/include/os_cpu/$(PROJECT)
export HEAD_DIR7  = ${HOME}/include/os_cpu/$(PROJECT)/bsp
export HEAD_DIR8  = ${HOME}/include/os_lib
export HEAD_DIR9  = ${HOME}/include/shell
export HEAD_DIR10 = ${HOME}/include/os_core/dmodule
export HEAD_DIR11 = ${HOME}/include/os_core/fs
export HEAD_DIR12 = ${HOME}/include/os_core/ipc
export HEAD_DIR13 = ${HOME}/include/os_core/time

INCLUDE = -I$(HEAD_DIR1) -I$(HEAD_DIR2) -I$(HEAD_DIR3) -I$(HEAD_DIR4) -I$(HEAD_DIR5) -I$(HEAD_DIR6) -I$(HEAD_DIR7) -I$(HEAD_DIR8) -I$(HEAD_DIR9) 

FLAGS = -W -Wall $(INCLUDE) -mcpu=cortex-m4 -mthumb -fno-builtin -O0

LIBPATH = /home/ka/stm32/gcc-arm-none-eabi-5_4-2016q3/arm-none-eabi/lib/armv7-m/ -L/home/ka/stm32/gcc-arm-none-eabi-5_4-2016q3/lib/gcc/arm-none-eabi/5.4.1/armv7-m/

LFLAGS = -z max-page-size=0x4 -Bsymbolic  -nodefaultlibs -nostartfiles -nostdlib -r -L$(LIBPATH) -T stm32_flash.ld -e main -static -lgcc

all:module.ko

module.ko : $(src:.c=.o)
	arm-none-eabi-ld $(LFLAGS) $^ -o $@

%.o : %.c
	arm-none-eabi-gcc $(FLAGS)  -mword-relocations -mlong-calls -c $< -o $@

clean:
	rm -rf *.o *.ko

