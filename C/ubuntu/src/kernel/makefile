SRCS = 

ASM_SRCS = 

SUB_DIR_y = OS_CORE OS_CPU/$(PROJECT) OS_LIB

SUB_DIR_$(CONFIG_SHELL_EN) += SHELL

$(SUB_TGT): $(SRCS:.c=.o) $(ASM_SRCS:.s=.o) $(SUB_DIR_y)
	$(LD) $(SRCS:.c=.o) $(ASM_SRCS:.s=.o) $(SUB_DIR_y:=/$(SUB_TGT)) -r -o $@

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@ $(DEF)

%.d :%.c
	$(CC) $(FLAGS) $< -MM > $@ $(DEF)

%.o :%.s
	#@echo "execute .s -> .o"
	$(CC) $(ASFLAGS) -c $< -o $@

sinclude $(SRCS:.c=.d)



$(SUB_DIR_y) :
	make -C $@

clean:
	rm -vf $(SUB_TGT) $(SRCS:.c=.o) $(ASM_SRCS:.s=.o) $(SRCS:.c=.d)
	for dir in $(SUB_DIR_y);do \
	    make -C $$dir clean;\
	done

.PHONY: clean $(SUB_DIR_y)
