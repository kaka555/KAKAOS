TARGET = stm32f103.elf

CROSS_COMPILER = arm-none-eabi-

SCRIPT = stm32_flash.ld

LIBPATH = -L/home/ka/stm32/gcc-arm-none-eabi-5_4-2016q3/lib/gcc/arm-none-eabi/5.4.1/armv7e-m/softfp/ -L/home/ka/stm32/gcc-arm-none-eabi-5_4-2016q3/arm-none-eabi/lib/armv7e-m/softfp/

export CC = ${CROSS_COMPILER}gcc

export AS = ${CROSS_COMPILER}as

export LD = ${CROSS_COMPILER}ld

export OBJCOPY = ${CROSS_COMPILER}objcopy
###############################################
export HOME = ${shell pwd}

export HEAD_DIR1 = ${HOME}/include 
export HEAD_DIR2 = ${HOME}/src/FWlib/inc 
export HEAD_DIR3 = ${HOME}/include/os_base
export HEAD_DIR4 = ${HOME}/include/os_core
export HEAD_DIR5 = ${HOME}/include/os_core/schedule
export HEAD_DIR6 = ${HOME}/include/os_cpu
export HEAD_DIR7 = ${HOME}/include/os_cpu/bsp
export HEAD_DIR8 = ${HOME}/include/os_lib
export HEAD_DIR9 = ${HOME}/include/shell

export INCLUDE = -I$(HEAD_DIR1) -I$(HEAD_DIR2) -I$(HEAD_DIR3) -I$(HEAD_DIR4) -I$(HEAD_DIR5) -I$(HEAD_DIR6) -I$(HEAD_DIR7) -I$(HEAD_DIR8) -I$(HEAD_DIR9)

OPTIMIZE = O2
export FLAGS = -W -Wall -$(OPTIMIZE) $(INCLUDE) -g -mcpu=cortex-m3 -mthumb --specs=nosys.specs -nostartfiles 

export ASFLAGS = -W -Wall -g -mcpu=cortex-m3 -mthumb --specs=nosys.specs

export SUB_TGT = built-in.o
########################################################################
SUB_DIR = user src

$(TARGET): $(SUB_DIR)
	$(LD)  $(^:=/$(SUB_TGT)) -T $(SCRIPT)  -nodefaultlibs -nostdlib -o $@ -static $(LIBPATH) -lgcc -lc
	$(OBJCOPY) $(TARGET)  $(TARGET:.elf=.bin) -Obinary
	$(OBJCOPY) $(TARGET)  $(TARGET:.elf=.hex) -Oihex

$(SUB_DIR):
	make -C $@

clean:
	rm -vf $(TARGET)
	for dir in $(SUB_DIR);do \
            make -C $$dir clean;\
        done
	rm -vf $(TARGET:.elf=.bin) $(TARGET:.elf=.hex) $(TARGET)

.PHONY: clean $(SUB_DIR) 
