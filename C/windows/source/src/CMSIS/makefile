SRCS = ${shell ls *.c}

ASM_SRCS = 

SUB_DIR = startup

$(SUB_TGT): $(SRCS:.c=.o) $(ASM_SRCS:.s=.o) $(SUB_DIR)
	$(LD) $(SRCS:.c=.o) $(ASM_SRCS:.s=.o) $(SUB_DIR:=/$(SUB_TGT)) -r -o $@

%.o: %.c
	@$(CC) $(FLAGS) -c $< -o $@ -D STM32F10X_HD -D USE_STDPERIPH_DRIVER

%.d :%.c
	@$(CC) $(FLAGS) $< -MM > $@ -D STM32F10X_HD -D USE_STDPERIPH_DRIVER

%.o :%.s
	#@echo "execute .s -> .o"
	@$(CC) $(ASFLAGS) -c $< -o $@

sinclude $(SRCS:.c=.d)



$(SUB_DIR) :
	make -C $@

clean:
	rm -vf $(SUB_TGT) $(SRCS:.c=.o) $(ASM_SRCS:.s=.o) $(SRCS:.c=.d)
	for dir in $(SUB_DIR);do \
	    make -C $$dir clean;\
	done

.PHONY: clean $(SUB_DIR)
