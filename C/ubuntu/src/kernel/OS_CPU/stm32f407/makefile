SRCS = ${shell ls *.c}

ASM_SRCS = CPU_A.S

SUB_DIR = bsp CMSIS FWlib/src

ifeq ($(CONFIG_NAME), stm32f407_config_no_fpu)
#do not use fpu
ASM_SRCS += OS_CPU_A_NO_FPU.S
else
#use fpu
ASM_SRCS += OS_CPU_A.S
endif

$(SUB_TGT): $(SRCS:.c=.o) $(ASM_SRCS:.S=.o) $(SUB_DIR)
	$(LD) $(SRCS:.c=.o) $(ASM_SRCS:.S=.o) $(SUB_DIR:=/$(SUB_TGT)) $(LDFLAG)  -r -o $@

%.o: %.c
	@echo "CC $@"
	@$(CC) $(FLAGS) -c $< -o $@ $(DEF)

%.d :%.c
	@$(CC) $(FLAGS) $< -MM > $@ $(DEF)

%.o :%.S
	@echo "AS $@"
	@$(AS) $(ASFLAGS) -c $< -o $@ 

sinclude $(SRCS:.c=.d)



$(SUB_DIR) :
	@make -C $@

clean:
	rm -vf $(SUB_TGT) $(SRCS:.c=.o) $(ASM_SRCS:.S=.o) $(SRCS:.c=.d)
	for dir in $(SUB_DIR);do \
	    make -C $$dir clean;\
	done

.PHONY: clean $(SUB_DIR)
