TARGET = kernel.elf

CROSS_COMPILER = arm-none-eabi-

SCRIPT = $(HOME)/src/kernel/OS_CPU/$(PROJECT)/stm32_flash.ld

LIBPATH = -L/home/ka/stm32/gcc-arm-none-eabi-5_4-2016q3/lib/gcc/arm-none-eabi/5.4.1/armv7e-m/softfp/ -L/home/ka/stm32/gcc-arm-none-eabi-5_4-2016q3/arm-none-eabi/lib/armv7e-m/softfp/

export CC = ${CROSS_COMPILER}gcc

export AS = ${CROSS_COMPILER}as

export LD = ${CROSS_COMPILER}ld

export OBJCOPY = ${CROSS_COMPILER}objcopy

export HOME = ${shell pwd}

include $(HOME)/config.mk
###############################################

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

export INCLUDE = -I$(HEAD_DIR1) -I$(HEAD_DIR2) -I$(HEAD_DIR3) -I$(HEAD_DIR4) -I$(HEAD_DIR5) -I$(HEAD_DIR6) -I$(HEAD_DIR7) -I$(HEAD_DIR8) -I$(HEAD_DIR9) -I$(HEAD_DIR10) -I$(HEAD_DIR11) -I$(HEAD_DIR12) -I$(HEAD_DIR13)

OPTIMIZE = O2

export FLAGS = -Wall -$(OPTIMIZE) $(INCLUDE) $(FLAGS_AUXILIARY) #-ffunction-sections -fdata-sections

export ASFLAGS = -Wall -$(OPTIMIZE) $(INCLUDE) $(FLAGS_AUXILIARY) #-ffunction-sections -fdata-sections

export SUB_TGT = built-in.o
########################################################################
SUB_DIR =  src/kernel user

$(TARGET): $(SUB_DIR)
	$(LD)  $(^:=/$(SUB_TGT)) -T $(SCRIPT) -nostartfiles -nodefaultlibs -nostdlib --gc-sections -o $@ -static $(LIBPATH) -lgcc -lc
	$(OBJCOPY) $(TARGET)  $(TARGET:.elf=.bin) -Obinary
	$(OBJCOPY) $(TARGET)  $(TARGET:.elf=.hex) -Oihex
	@echo $(FLAGS)

$(SUB_DIR):
	make -C $@

clean:
	@rm -vf $(TARGET)
	for dir in $(SUB_DIR);do \
            make -C $$dir clean;\
        done
	@rm -vf $(TARGET:.elf=.bin) $(TARGET:.elf=.hex) $(TARGET)
	

cleanconfig:
	@echo "" > $(HOME)/config.mk

.PHONY: clean $(SUB_DIR)

MKCONFIG := $(HOME)/mkconfig

stm32f103_config:
	@$(MKCONFIG) $(@:_config=) "-mcpu=cortex-m3 -mthumb" "-D STM32F10X_HD -D USE_STDPERIPH_DRIVER"

stm32f407_config:
	@$(MKCONFIG) $(@:_config=) "-mcpu=cortex-m4 -mthumb" "-D STM32F40XX -D USE_STDPERIPH_DRIVER"

