SRCS_y = 

ASM_SRCS = 

SUB_DIR_y =

SRCS_$(CONFIG_TIME_EN) += os_time.c

SRCS_$(CONFIG_TIMER_EN) += os_timer.c

$(SUB_TGT): $(SRCS_y:.c=.o) $(ASM_SRCS:.s=.o) $(SUB_DIR_y)
	$(LD) $(SRCS_y:.c=.o) $(ASM_SRCS:.s=.o) $(SUB_DIR_y:=/$(SUB_TGT)) $(LDFLAG)  -r -o $@

%.o: %.c
	@echo "CC $@"
	@$(CC) $(FLAGS) -c $< -o $@ $(DEF)

%.d :%.c
	@$(CC) $(FLAGS) $< -MM > $@ $(DEF)

%.o :%.s
	@echo "CC $@"
	@$(CC) $(ASFLAGS) -c $< -o $@

sinclude $(SRCS_y:.c=.d)



$(SUB_DIR_y) :
	@make -C $@

clean:
	rm -vf $(SUB_TGT) $(SRCS_y:.c=.o) $(ASM_SRCS:.s=.o) $(SRCS_y:.c=.d)
	for dir in $(SUB_DIR_y);do \
	    make -C $$dir clean;\
	done

.PHONY: clean $(SUB_DIR_y)
