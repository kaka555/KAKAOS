SRCS = device.c vfs.c vfs_default_fun.c vfsio.c

ASM_SRCS = 

SUB_DIR = 

ifeq (stm32f103,$(PROJECT))
SRCS += fat.c
SUB_DIR += FATFS
endif

$(SUB_TGT): $(SRCS:.c=.o) $(ASM_SRCS:.s=.o) $(SUB_DIR)
	$(LD) $(SRCS:.c=.o) $(ASM_SRCS:.s=.o) $(SUB_DIR:=/$(SUB_TGT)) $(LDFLAG)  -r -o $@

%.o: %.c
	@echo "CC $@"
	@$(CC) $(FLAGS) -c $< -o $@ $(DEF)

%.d :%.c
	@$(CC) $(FLAGS) $< -MM > $@ $(DEF)

%.o :%.s
	@echo "CC $@"
	@$(CC) $(ASFLAGS) -c $< -o $@

sinclude $(SRCS:.c=.d)



$(SUB_DIR) :
	@make -C $@

clean:
	rm -vf $(SUB_TGT) $(SRCS:.c=.o) $(ASM_SRCS:.s=.o) $(SRCS:.c=.d)
	for dir in $(SUB_DIR);do \
	    make -C $$dir clean;\
	done

.PHONY: clean $(SUB_DIR)
