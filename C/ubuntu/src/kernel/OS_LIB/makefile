SRCS_y = buddy.c heap_oo.c insert_sort_oo.c malloc.c myassert.c myMicroLIB.c singly_linked_list.c slab.c vector.c

ASM_SRCS = 

SUB_DIR = 

SRCS_$(CONFIG_MEM_POOL) += mem_pool.c

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
