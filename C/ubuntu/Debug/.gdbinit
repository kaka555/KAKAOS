target remote :2331
set mem inaccessible-by-default off
monitor speed auto
monitor endian little
monitor reset
monitor flash device = STM32F103ZE
monitor flash breakpoints = 1
monitor flash download = 1
load
monitor reg sp = (0x08000000)
monitor reg pc = (0x08000004)
break Reset_Handler
continue
