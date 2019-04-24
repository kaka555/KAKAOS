SRCS_y = command_processor.c shell.c shell_fun.c

ASM_SRCS = 

SUB_DIR = 

SRCS_$(CONFIG_SHELL_DEBUG_EN) += shell_debug.c

$(SUB_TGT): $(SRCS_y:.c=.o) $(ASM_SRCS:.s=.o) $(SUB_DIR)
	$(LD) $(SRCS_y:.c=.o) $(ASM_SRCS:.s=.o) $(SUB_DIR:=/$(SUB_TGT)) $(LDFLAG)  -r -o $@

%.o: %.c
	@echo "CC $@"
	@$(CC) $(FLAGS) -c $< -o $@ $(DEF)

%.d :%.c
	@$(CC) $(FLAGS) $< -MM > $@ $(DEF)

%.o :%.s
	@echo "CC $@"
	@$(CC) $(ASFLAGS) -c $< -o $@

sinclude $(SRCS_y:.c=.d)



$(SUB_DIR) :
	@make -C $@

clean:
	rm -vf $(SUB_TGT) $(SRCS_y:.c=.o) $(ASM_SRCS:.s=.o) $(SRCS_y:.c=.d)
	for dir in $(SUB_DIR);do \
	    make -C $$dir clean;\
	done

.PHONY: clean $(SUB_DIR)
