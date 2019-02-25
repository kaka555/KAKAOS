src = kaka.c

INCLUDE = 

FLAGS = -W -Wall $(INCLUDE) -mcpu=cortex-m3 -mthumb -specs=nano.specs -specs=nosys.specs -fdata-sections -ffunction-sections -nostartfiles 

LIBPATH = -L/home/ka/stm32/gcc-arm-none-eabi-5_4-2016q3/arm-none-eabi/lib/armv7-m/ -L/home/ka/stm32/gcc-arm-none-eabi-5_4-2016q3/lib/gcc/arm-none-eabi/5.4.1/armv7-m/

all:libkaka.so

libkaka.so:$(src)
	arm-none-eabi-gcc $(FLAGS) -mlong-calls -fPIC -c $< -o $(src:.c=.o)
	#arm-none-eabi-ld -T stm32_flash.ld --gc-sections -z max-page-size=0x4 -shared -fPIC -nostartfiles -nostdlib $(src:.c=.o) -o $@ -static $(LIBPATH) -lgcc
	#arm-none-eabi-strip -R .hash libkaka.so
	arm-none-eabi-strip -R .hash kaka.o

clean:
	rm -rf *.o *.so
