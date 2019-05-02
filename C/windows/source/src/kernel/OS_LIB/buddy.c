#include <buddy.h>
#include <myassert.h>
#include <os_cpu.h>
#include <myMicroLIB.h>

static struct buddy *os_buddy_ptr_head = NULL;
static struct buddy *current_used_buddy_ptr;

extern int _ebss;

extern int get_set_bit_place(unsigned int num);
extern unsigned long ka_pow(int x,unsigned int y);
extern void dis_page_alloc_record(void);

static void _delete_from_chain(unsigned int level,Page_Num_Type page_num);
#define __debug 
#if CONFIG_ASSERT_DEBUG
static __debug int assert_in_chain(unsigned int level,Page_Num_Type num);
#endif
static inline int _check_flag(unsigned int level,Page_Num_Type page_num);
static void _deal_with_flag_alloc(unsigned int level,Page_Num_Type page_num);
static Page_Num_Type _alloc_page(unsigned int level);

static inline unsigned int get_max_level(unsigned int page_num)
{
	return get_set_bit_place(page_num);
}

const struct buddy *_get_os_buddy_ptr_head(void)
{
	current_used_buddy_ptr = os_buddy_ptr_head;
	return (const struct buddy *)os_buddy_ptr_head;
}

const struct buddy *_get_next_buddy_ptr_head(const struct buddy *buddy_ptr)
{
	if(NULL == buddy_ptr->next)
	{
		current_used_buddy_ptr = os_buddy_ptr_head;
		return NULL;
	}
	current_used_buddy_ptr = buddy_ptr->next;
	return (const struct buddy *)current_used_buddy_ptr;
}

unsigned int _get_current_buddy_space(void)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	unsigned int buffer = current_used_buddy_ptr->current_page_num;
	CPU_CRITICAL_EXIT();
	return buffer;
}

static struct order_link *_alloc_order_link(void) /*allocate a struct order_link entity*/
{
	UINT16 buffer = current_used_buddy_ptr->order_link_first;
	if(NOTHING == buffer)
	{
		return NULL;
	}
	current_used_buddy_ptr->order_link_first = current_used_buddy_ptr->order_link_flag[buffer];
	current_used_buddy_ptr->order_link_flag[buffer] = NOTHING;
	return &current_used_buddy_ptr->link_body[buffer];
}

/*		this function allocate a struct order_link to store the page_num;
 then, insert the struct order_link into corresponding level's order_array*/
static void _add_to_order_array(unsigned int level,Page_Num_Type page_num)
{
	ASSERT((level>=1)&&(level<=current_used_buddy_ptr->info.max_level),ASSERT_INPUT);
	ASSERT(0 == (((0x0001<<(level-1))-1) & page_num),ASSERT_INPUT);
	UINT16 first = current_used_buddy_ptr->order_link_first;
	struct order_link *order_link_ptr = _alloc_order_link();
	ASSERT(NULL != order_link_ptr,ASSERT_PARA_AFFIRM);
	order_link_ptr->num  = page_num;
	order_link_ptr->next = current_used_buddy_ptr->order_array[level-1];
	ASSERT(order_link_ptr->num <= current_used_buddy_ptr->info.page_num-1,ASSERT_PARA_AFFIRM);
	ASSERT((order_link_ptr->next <= current_used_buddy_ptr->info.page_num-1) || (NOTHING == order_link_ptr->next),ASSERT_PARA_AFFIRM);
	current_used_buddy_ptr->order_array[level-1] = first;
}

static Page_Num_Type _buddy_page_num(unsigned int level,Page_Num_Type page_num)
{
	ASSERT((level>=1)&&(level<=current_used_buddy_ptr->info.max_level),ASSERT_INPUT);
	ASSERT(0 == (((0x0001<<(level-1))-1) & page_num),ASSERT_INPUT);
	if(page_num & (0x0001<<(level-1))) /*it's buddy is ahead of it*/
		return (page_num - sizeof_level(level));
	else 
		return (page_num + sizeof_level(level)); /*it's buddy fall behind it*/
}

static void _add_to_order_array_loop(unsigned int level,Page_Num_Type page_num)
{
	/*non - recursion*/
	unsigned int i;
	Page_Num_Type buddy_page_num;
	ASSERT((level>=1)&&(level<=current_used_buddy_ptr->info.max_level),ASSERT_INPUT);
	ASSERT(0 == (((0x0001<<(level-1))-1) & page_num),ASSERT_INPUT);
	for(i=level;i<=current_used_buddy_ptr->info.max_level;++i)
	{
		ASSERT((i<=current_used_buddy_ptr->info.max_level)&&(i>=1),ASSERT_PARA_AFFIRM);
		ASSERT(0 == (page_num & ((0x0001<<(i-1))-1)),ASSERT_PARA_AFFIRM);
		if(current_used_buddy_ptr->info.max_level == i) /* if it is the max level, just insert it into chain order_array[max_level]*/
		{
			_add_to_order_array(i,page_num);
			break ;
		}
		else 
		{
			buddy_page_num = _buddy_page_num(i,page_num); /* get it's buddy page's num*/
			if(0 == _check_flag(i,page_num)) /* if flag is 0 , means it's buddy is in chain order_array[level]*/
			{
				ASSERT(0 == assert_in_chain(i,buddy_page_num),ASSERT_PARA_AFFIRM); /*assert it's buddy is in chain "order_array"*/
				_delete_from_chain(i,buddy_page_num); /* delete it's buddy from chain order_array[level] so that they can combine into a big block to insert into the next level*/
				if(page_num < buddy_page_num)
				{
					_deal_with_flag_alloc(i+1,page_num); /* XOR the corresponding bit*/
					continue ;
				}
				else /*if(page_num > buddy_page_num) */
				{
					_deal_with_flag_alloc(i+1,buddy_page_num); /* XOR the corresponding bit*/
					page_num = buddy_page_num;
					continue ;
				}
			}
			else /*if(1 == _check_flag(i,page_num))*/  /* if flag is not 0 , means it's buddy is in chain order_array[level]*/
			{
				ASSERT(-1 == assert_in_chain(i,buddy_page_num),ASSERT_PARA_AFFIRM);
				_add_to_order_array(i,page_num); /* insert into chain order_array[level]*/
				break ;
			}
		}
		
	}
}

static inline void *_add_of_num(UINT16 page_num)
{
	return (void *)(&current_used_buddy_ptr->buddy_space_start_ptr[page_num]);
}

/* XOR the corresponding bit*/
static void _deal_with_flag_alloc(unsigned int level,Page_Num_Type page_num)
{
	ASSERT((level>=1)&&(level<=current_used_buddy_ptr->info.max_level),ASSERT_INPUT);
	ASSERT(0 == (((0x0001<<(level-1))-1) & page_num),ASSERT_INPUT);
	unsigned int index = page_num>>level;
	current_used_buddy_ptr->flag[(current_used_buddy_ptr->level_flag_base[level-1]+index)/(sizeof(Flag_Type)*8)] ^= (0x80000000>>((current_used_buddy_ptr->level_flag_base[level-1]+index)%(sizeof(Flag_Type)*8)));
}

static void _return_link_body(Page_Num_Type buffer) /*return struct order_link "link_body[buffer]"*/
{
	ASSERT(buffer<=current_used_buddy_ptr->info.page_num-1,ASSERT_INPUT);
	ASSERT(NOTHING == current_used_buddy_ptr->order_link_flag[buffer],ASSERT_INPUT);
	current_used_buddy_ptr->order_link_flag[buffer] = current_used_buddy_ptr->order_link_first;
	current_used_buddy_ptr->order_link_first = buffer; /*change list's head*/
}

int in_buddy_range(void *ptr)
{
	if(((unsigned int)ptr >= (unsigned int)current_used_buddy_ptr->buddy_space_start_ptr) 
			&& ((unsigned int)ptr < (unsigned int)current_used_buddy_ptr + current_used_buddy_ptr->info.page_num * PAGE_SIZE_BYTE))
	{
		return 0;
	}
	return -1;
} 

static void add_to_os(struct buddy *buddy_ptr)
{
	if(NULL == os_buddy_ptr_head)
	{
		os_buddy_ptr_head = buddy_ptr;
	}
	else if(os_buddy_ptr_head->info.prio > buddy_ptr->info.prio)
	{
		buddy_ptr->next = os_buddy_ptr_head;
		os_buddy_ptr_head = buddy_ptr;
	}
	else
	{
		struct buddy *buffer_ptr = os_buddy_ptr_head;
		while(NULL != buffer_ptr->next)
		{
			if(buffer_ptr->next->info.prio > buddy_ptr->info.prio)
			{
				buddy_ptr->next = buffer_ptr->next;
				buffer_ptr->next = buddy_ptr;
				return ;
			}
			buffer_ptr = buffer_ptr->next;
		}
		buffer_ptr->next = buddy_ptr;
	}
}

/*
*************************************
*			memory map :
*
|-------------------------------|  low  <--memory start address
|	struct buddy  				|	|
|-------------------------------|	|
|	flag[FLAG_ARRAY_NUM]		|	|
|-------------------------------|	|
|	link_body[PAGE_NUM]			|	|
|-------------------------------|	|
|	order_link_flag[PAGE_NUM]	|	|
|-------------------------------|	|
|	order_array[MAX_LEVEL]		|	|
|-------------------------------|	|
|		room					|	|
|		for 					|	|
|	 allocation 				|	|
|		 . 						|	|
|		 .						|	|
|		 .						|	V
|-------------------------------|  high
 */
/**
 * @Author      kaka
 * @DateTime    2019-05-02
 * @description : this function init the buddy system, it use the front memory 
 * to record the flag for management
 * @param       dev_mem_para_ptr [description]
 */
void __buddy_init(const struct dev_mem_para *dev_mem_para_ptr)
{
	unsigned int size;
	unsigned int i;
	unsigned int buffer;
	UINT32 size_sum = 0;
	struct buddy *buddy_ptr;
	if(TYPE_NORMAL == dev_mem_para_ptr->type)
	{
	 	buddy_ptr = (struct buddy *)(dev_mem_para_ptr->start);
	}
	else
	{
		ASSERT(TYPE_SYS == dev_mem_para_ptr->type,ASSERT_PARA_AFFIRM);
		buddy_ptr = (struct buddy *)(&_ebss + 1);
	}
	current_used_buddy_ptr = buddy_ptr;
/*1.allocate room for flag[current_used_buddy_ptr->info.flag_array_num]*/
	buddy_ptr->flag = (Flag_Type *)((char *)buddy_ptr + sizeof(struct buddy));
	size = dev_mem_para_ptr->size / (PAGE_SIZE_KB * 8 * sizeof(Flag_Type));
	if(dev_mem_para_ptr->size % (PAGE_SIZE_KB * 8 * sizeof(Flag_Type)))
	{
		++size;
	}
	buddy_ptr->info.flag_array_num = size;
	size_sum += size*sizeof(Flag_Type);
	buddy_ptr->link_body = (struct order_link *)((char *)buddy_ptr->flag + size*sizeof(Flag_Type));
/*allocate room for link_body[current_used_buddy_ptr->info.page_num]*/
	size = dev_mem_para_ptr->size / PAGE_SIZE_KB;
	buddy_ptr->info.page_num = size;
	size_sum += size * sizeof(struct order_link);
	buddy_ptr->order_link_flag = (Page_Num_Type *)((char *)buddy_ptr->link_body + size * sizeof(struct order_link));
/*allocate room for order_link_flag[current_used_buddy_ptr->info.page_num]	*/
	size_sum += size * sizeof(Page_Num_Type);
	buddy_ptr->order_array = (Page_Num_Type *)((char *)buddy_ptr->order_link_flag + size * sizeof(Page_Num_Type));
/*allocate room for order_array[current_used_buddy_ptr->info.max_level]	*/
	size = get_max_level(buddy_ptr->info.page_num);
	buddy_ptr->info.max_level = size;
	size_sum += size * sizeof(Page_Num_Type);
	buddy_ptr->level_flag_base = (UINT16 *)((char *)buddy_ptr->order_array + size * sizeof(Page_Num_Type));
/*allocate room for level_flag_base[buddy_ptr->info.level_flag_base_size]*/
	size = buddy_ptr->info.max_level;

	buddy_ptr->info.level_flag_base_size = size;
	size_sum += size * sizeof(UINT16);
	buddy_ptr->buddy_struct_size = size_sum;
	buddy_ptr->current_page_num = dev_mem_para_ptr->size / PAGE_SIZE_KB;
	buddy_ptr->buddy_space_start_ptr = (Buddy_Space_Type *)(dev_mem_para_ptr->start);
	buddy_ptr->next = NULL;
	buddy_ptr->info.prio = dev_mem_para_ptr->prio;
/*init the data*/
	for(i=0;i<buddy_ptr->info.flag_array_num;++i)
		buddy_ptr->flag[i] = 0;

	buddy_ptr->order_link_first = 0;

	for(i=0;i<buddy_ptr->info.page_num-1;++i)
		buddy_ptr->order_link_flag[i] = i+1;
	buddy_ptr->order_link_flag[i] = NOTHING;

	for(i=0;i<buddy_ptr->info.max_level;++i)
		buddy_ptr->order_array[i] = NOTHING;

	buffer = ka_pow(2,buddy_ptr->info.max_level - 1);
	buddy_ptr->level_flag_base[0] = 0;
	for(i=1;i<buddy_ptr->info.level_flag_base_size;++i,buffer/=2)
		buddy_ptr->level_flag_base[i] = buddy_ptr->level_flag_base[i-1] + buffer;
/*set the order_array[current_used_buddy_ptr->info.max_level],and then allocate a room for struct buddy's member*/
/*first deal with the max level*/
	_add_to_order_array(buddy_ptr->info.max_level,sizeof_level(buddy_ptr->info.max_level));
	_add_to_order_array(buddy_ptr->info.max_level,0);
/*mark the space used by struct buddy*/
	size = (unsigned int)buddy_ptr - (unsigned int)dev_mem_para_ptr->start + buddy_ptr->buddy_struct_size + sizeof(struct buddy);
	ka_printf("space used by os is %d\n",size);
	if(size % PAGE_SIZE_BYTE)
	{
		size = size/PAGE_SIZE_BYTE + 1;
	}
	else
	{
		size /= PAGE_SIZE_BYTE;
	}
	for(i=0;i<current_used_buddy_ptr->info.max_level;++i)
	{
		if(((0x01u)<<i) >= size)
		{
			ka_printf("allocate level %d\n",i+1);
			_alloc_page(i+1);
			current_used_buddy_ptr->current_page_num -= 0x01<<i;
			goto final_step;
		}
	}
	ASSERT(0,ASSERT_BAD_EXE_LOCATION);/*allocation fail*/
final_step:
/*add the struct buddy to OS*/
	add_to_os(buddy_ptr);
}

/*  if allocation success,get a page num,which scope is 0 to 1023
**  else, get NOTHING*/
static Page_Num_Type _alloc_page(unsigned int level)
{
	ASSERT((level>=1)&&(level<=current_used_buddy_ptr->info.max_level),ASSERT_INPUT);
	Page_Num_Type buffer;
	if(NOTHING != current_used_buddy_ptr->order_array[level-1]) /* if this level has free page */
	{
		Page_Num_Type index = current_used_buddy_ptr->order_array[level-1];
		buffer = current_used_buddy_ptr->link_body[index].num;
		current_used_buddy_ptr->order_array[level-1] = current_used_buddy_ptr->link_body[index].next;
		_return_link_body(index);
	}
	else /*if this level doesn't have free page*/
	{
		if(current_used_buddy_ptr->info.max_level == level)
		{
			return NOTHING;
		}
		buffer = _alloc_page(level+1); /*get a free page from the higher level */
		if(NOTHING == buffer)
		{
			return NOTHING;
		}
		_add_to_order_array(level,buffer+sizeof_level(level)); /* add the second half of the block to present level*/
	}
	_deal_with_flag_alloc(level,buffer); /* XOR the corresponding bit*/
	return buffer;
}

/*get 1 page*/
void *_alloc_power1_page(void)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	Page_Num_Type num = _alloc_page(1);
	if(NOTHING != num)
	{
		/* decrease the space of this buddy*/
		current_used_buddy_ptr->current_page_num -= PAGE_SIZE_KB;
		CPU_CRITICAL_EXIT();
		return _add_of_num(num);
	}
	CPU_CRITICAL_EXIT();
	return NULL;
}

void *_alloc_power2_page(void)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	Page_Num_Type num = _alloc_page(2);
	if(NOTHING != num)
	{
		/* decrease the space of this buddy*/
		current_used_buddy_ptr->current_page_num -= 2 * PAGE_SIZE_KB;
		CPU_CRITICAL_EXIT();
		return _add_of_num(num);
	}
	CPU_CRITICAL_EXIT();
	return NULL;
}

void *_alloc_power3_page(void)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	Page_Num_Type num = _alloc_page(3);
	if(NOTHING != num)
	{
		/* decrease the space of this buddy*/
		current_used_buddy_ptr->current_page_num -= 4 * PAGE_SIZE_KB;
		CPU_CRITICAL_EXIT();
		return _add_of_num(num);
	}
	CPU_CRITICAL_EXIT();
	return NULL;
}

void *_alloc_power4_page(void)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	Page_Num_Type num = _alloc_page(4);
	if(NOTHING != num)
	{
		/* decrease the space of this buddy*/
		current_used_buddy_ptr->current_page_num -= 8 * PAGE_SIZE_KB;
		CPU_CRITICAL_EXIT();
		return _add_of_num(num);
	}
	CPU_CRITICAL_EXIT();
	return NULL;
}

void *_alloc_power5_page(void)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	Page_Num_Type num = _alloc_page(5);
	if(NOTHING != num)
	{
		/* decrease the space of this buddy*/
		current_used_buddy_ptr->current_page_num -= 16 * PAGE_SIZE_KB;
		CPU_CRITICAL_EXIT();
		return _add_of_num(num);
	}
	CPU_CRITICAL_EXIT();
	return NULL;
}

void *_alloc_power6_page(void)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	Page_Num_Type num = _alloc_page(6);
	if(NOTHING != num)
	{
		/* decrease the space of this buddy*/
		current_used_buddy_ptr->current_page_num -= 32 * PAGE_SIZE_KB;
		CPU_CRITICAL_EXIT();
		return _add_of_num(num);
	}
	CPU_CRITICAL_EXIT();
	return NULL;
}

void *_alloc_power7_page(void)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	Page_Num_Type num = _alloc_page(7);
	if(NOTHING != num)
	{
		/* decrease the space of this buddy*/
		current_used_buddy_ptr->current_page_num -= 64 * PAGE_SIZE_KB;
		CPU_CRITICAL_EXIT();
		return _add_of_num(num);
	}
	CPU_CRITICAL_EXIT();
	return NULL;
}

void *_alloc_power8_page(void)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	Page_Num_Type num = _alloc_page(8);
	if(NOTHING != num)
	{
		/* decrease the space of this buddy*/
		current_used_buddy_ptr->current_page_num -= 128 * PAGE_SIZE_KB;
		CPU_CRITICAL_EXIT();
		return _add_of_num(num);
	}
	CPU_CRITICAL_EXIT();
	return NULL;
}

void *_alloc_power9_page(void)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	Page_Num_Type num = _alloc_page(9);
	if(NOTHING != num)
	{
		/* decrease the space of this buddy*/
		current_used_buddy_ptr->current_page_num -= 256 * PAGE_SIZE_KB;
		CPU_CRITICAL_EXIT();
		return _add_of_num(num);
	}
	CPU_CRITICAL_EXIT();
	return NULL;
}

void *_alloc_power10_page(void)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	Page_Num_Type num = _alloc_page(10);
	if(NOTHING != num)
	{
		/* decrease the space of this buddy*/
		current_used_buddy_ptr->current_page_num -= 512 * PAGE_SIZE_KB;
		CPU_CRITICAL_EXIT();
		return _add_of_num(num);
	}
	CPU_CRITICAL_EXIT();
	return NULL;
}

#if CONFIG_ASSERT_DEBUG
/*if in chain,return 0;else, return -1*/
static __debug int assert_in_chain(unsigned int level,Page_Num_Type page_num)
{
	if(NOTHING == current_used_buddy_ptr->order_array[level-1])
		return -1;
	UINT16 buffer;
	buffer = current_used_buddy_ptr->order_array[level-1];
	while(buffer != NOTHING)
	{
		if(page_num == current_used_buddy_ptr->link_body[buffer].num)
			return 0;
		buffer = current_used_buddy_ptr->link_body[buffer].next;
	}
	return -1;
}
#endif

static inline Page_Num_Type _get_page_num(void *ptr) /*according to adress*/
{
	return (Page_Num_Type)(((UINT32)ptr - (UINT32)current_used_buddy_ptr->buddy_space_start_ptr)/PAGE_SIZE_BYTE);
}

static inline int _check_flag(unsigned int level,Page_Num_Type page_num) 
{
	ASSERT((level>=1)&&(level<=current_used_buddy_ptr->info.max_level),ASSERT_INPUT);
	ASSERT(0 == (((0x0001<<(level-1))-1) & page_num),ASSERT_INPUT);
	int index = page_num>>level;
	return (current_used_buddy_ptr->flag[(current_used_buddy_ptr->level_flag_base[level-1]+index)/(sizeof(Flag_Type)*8)]
		&(0x80000000>>((current_used_buddy_ptr->level_flag_base[level-1]+index)%(sizeof(Flag_Type)*8))));
}

static void _delete_from_chain(unsigned int level,Page_Num_Type page_num)
{
	Page_Num_Type buffer1,buffer2;
	ASSERT((level>=1)&&(level<=current_used_buddy_ptr->info.max_level),ASSERT_INPUT);
	ASSERT(0 == (((0x0001<<(level-1))-1) & page_num),ASSERT_INPUT);
	ASSERT(NOTHING != current_used_buddy_ptr->order_array[level-1],ASSERT_PARA_AFFIRM);
	buffer1 = current_used_buddy_ptr->order_array[level-1];
	if(page_num == current_used_buddy_ptr->link_body[buffer1].num) /*delete from linked-list*/
	{
		current_used_buddy_ptr->order_array[level-1] = current_used_buddy_ptr->link_body[buffer1].next;
		_return_link_body(buffer1);
		return ;
	}
	else
	{
		buffer2 = current_used_buddy_ptr->link_body[buffer1].next;
		while(NOTHING != buffer2)
		{
			if(page_num == current_used_buddy_ptr->link_body[buffer2].num)
			{
				current_used_buddy_ptr->link_body[buffer1].next = current_used_buddy_ptr->link_body[buffer2].next;
				_return_link_body(buffer2);
				return ;
			}
			buffer1 = buffer2;
			buffer2 = current_used_buddy_ptr->link_body[buffer2].next;
		}
		ASSERT(0,ASSERT_BAD_EXE_LOCATION);
	}
}

static void _return_page(void *ptr,unsigned int level) /*return page to system*/
{
	ASSERT(NULL != ptr,ASSERT_INPUT);
	if(NULL == ptr)
	{
		return ;
	}
	ASSERT((level>=1)&&(level<=current_used_buddy_ptr->info.max_level),ASSERT_INPUT);
	ASSERT(((UINT32)current_used_buddy_ptr->buddy_space_start_ptr <= (UINT32)ptr)
		&&
		((UINT32)&current_used_buddy_ptr->buddy_space_start_ptr[BUDDY_SPACE_SIZE] >= (UINT32)ptr),ASSERT_INPUT);
	ASSERT(0 == in_buddy_range(ptr),ASSERT_INPUT);
	Page_Num_Type page_num = _get_page_num(ptr);
	_deal_with_flag_alloc(level,page_num);  /*XOR the flag bit*/
	_add_to_order_array_loop(level,page_num); /* then going to loop to finish the return*/
}

void _return_power1_page(void *ptr)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	/*increase the space of the buddy*/
	current_used_buddy_ptr->current_page_num += PAGE_SIZE_KB;
	_return_page(ptr,1);
	CPU_CRITICAL_EXIT();
}

void _return_power2_page(void *ptr)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	/*increase the space of the buddy*/
	current_used_buddy_ptr->current_page_num += 2 * PAGE_SIZE_KB;
	_return_page(ptr,2);
	CPU_CRITICAL_EXIT();
}

void _return_power3_page(void *ptr)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	/*increase the space of the buddy*/
	current_used_buddy_ptr->current_page_num += 4 * PAGE_SIZE_KB;
	_return_page(ptr,3);
	CPU_CRITICAL_EXIT();
}

void _return_power4_page(void *ptr)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	/*increase the space of the buddy*/
	current_used_buddy_ptr->current_page_num += 8 * PAGE_SIZE_KB;
	_return_page(ptr,4);
	CPU_CRITICAL_EXIT();
}

void _return_power5_page(void *ptr)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	/*increase the space of the buddy*/
	current_used_buddy_ptr->current_page_num += 16 * PAGE_SIZE_KB;
	_return_page(ptr,5);
	CPU_CRITICAL_EXIT();
}

void _return_power6_page(void *ptr)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	/*increase the space of the buddy*/
	current_used_buddy_ptr->current_page_num += 32 * PAGE_SIZE_KB;
	_return_page(ptr,6);
	CPU_CRITICAL_EXIT();
}

void _return_power7_page(void *ptr)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	/*increase the space of the buddy*/
	current_used_buddy_ptr->current_page_num += 64 * PAGE_SIZE_KB;
	_return_page(ptr,7);
	CPU_CRITICAL_EXIT();
}

void _return_power8_page(void *ptr)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	/*increase the space of the buddy*/
	current_used_buddy_ptr->current_page_num += 128 * PAGE_SIZE_KB;
	_return_page(ptr,8);
	CPU_CRITICAL_EXIT();
}

void _return_power9_page(void *ptr)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	/*increase the space of the buddy*/
	current_used_buddy_ptr->current_page_num += 256 * PAGE_SIZE_KB;
	_return_page(ptr,9);
	CPU_CRITICAL_EXIT();
}

void _return_power10_page(void *ptr)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	/*increase the space of the buddy*/
	current_used_buddy_ptr->current_page_num += 512 * PAGE_SIZE_KB;
	_return_page(ptr,10);
	CPU_CRITICAL_EXIT();
}

#if CONFIG_SHELL_EN

#if CONFIG_DEBUG_ON
static int _check_buddy_flag_level(unsigned int level,unsigned int offset)
{
	ASSERT((level>=1) && (level<=10),ASSERT_INPUT);
	ASSERT((offset<((current_used_buddy_ptr->info.page_num>>1)/sizeof_level(level))),ASSERT_INPUT);
	unsigned int num = 0;
	unsigned int page_num = sizeof_level(level)*2*offset;
	if(NOTHING == current_used_buddy_ptr->order_array[level-1])
	{
		return -1;
	}
	struct order_link buffer = current_used_buddy_ptr->link_body[current_used_buddy_ptr->order_array[level-1]];
	while(buffer.next != NOTHING)
	{
		if((page_num == buffer.num) || (page_num+sizeof_level(level) == buffer.num))
		{
			++num;
			if(2 < num)
			{
				return -1;
			}
		}
		buffer = current_used_buddy_ptr->link_body[buffer.next];
	}
	if((page_num == buffer.num) || (page_num+sizeof_level(level) == buffer.num))
	{
		++num;
		if(num>=2)
		{
			return -1;
		}
	}
	if(1 == num)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

static unsigned int _get_level(unsigned int array_index,unsigned int array_offset)
{
	unsigned int num = array_index * 8 * sizeof(Flag_Type) + array_offset;
	ASSERT(num < 1024,ASSERT_INPUT);
	unsigned int i;
	for(i=0;i<current_used_buddy_ptr->info.level_flag_base_size;++i)
	{
		if(num < current_used_buddy_ptr->level_flag_base[i])
		{
			return i;
		}
	}
	ASSERT(current_used_buddy_ptr->info.page_num - 2 == num,ASSERT_PARA_AFFIRM);
	return current_used_buddy_ptr->info.max_level;
}

static int _check_buddy_level_flag(void)
{
	unsigned int i;
	unsigned int num;
	struct order_link buffer;
	unsigned int offset;
	for(i=0;i<current_used_buddy_ptr->info.max_level;++i) /*each level*/
	{
		if(NOTHING != current_used_buddy_ptr->order_array[i])
		{
			if(i != current_used_buddy_ptr->info.max_level-1)
			{
				buffer = current_used_buddy_ptr->link_body[current_used_buddy_ptr->order_array[i]];
				while(NOTHING != buffer.next)
				{
					num = buffer.num;
					offset = num>>(i+1);
					num = current_used_buddy_ptr->level_flag_base[i] + offset;
					/*ka_printf("num is %d\n",num);*/
					if(!(0x01 & current_used_buddy_ptr->flag[num/(sizeof(Flag_Type)*8)]>>(sizeof(Flag_Type)*8 - num%(sizeof(Flag_Type)*8)-1)))
					{
						ka_printf("level %d,page_num %d error\n",i+1,num<<(i+1));
						return -1;
					}
					buffer = current_used_buddy_ptr->link_body[buffer.next];
				}
				num = buffer.num;
				offset = num>>(i+1);
				num = current_used_buddy_ptr->level_flag_base[i] + offset;
				/*ka_printf("num is %d\n",num);*/
				if(!(0x01 & current_used_buddy_ptr->flag[num/(sizeof(Flag_Type)*8)]>>(31-(num%(sizeof(Flag_Type)*8)))))
				{
					ka_printf("level %d,page_num %d error\n",i+1,num<<(i+1));
					return -1;
				}
			}
			else
			{
				buffer = current_used_buddy_ptr->link_body[current_used_buddy_ptr->order_array[i]];
				num = buffer.num;
				offset = num>>(i+1);
				num = current_used_buddy_ptr->level_flag_base[i] + offset;
				if(NOTHING != buffer.next)
				{
					if(current_used_buddy_ptr->link_body[buffer.next].num != sizeof_level(current_used_buddy_ptr->info.max_level))
					{
						/*ka_printf("here\n");*/
						ka_printf("level %d,page_num %d error\n",i+1,num);
						return -1;
					}
					if(0x01 & current_used_buddy_ptr->flag[num/(sizeof(Flag_Type)*8)]>>(31-(num%(sizeof(Flag_Type)*8))))
					{
						/*ka_printf("there\n");*/
						ka_printf("level %d,page_num %d error\n",i+1,num);
						return -1;
					}
				}
				else
				{
					/*ka_printf("num is %d\n",num);*/
					if(!(0x01 & current_used_buddy_ptr->flag[num/(sizeof(Flag_Type)*8)]>>(31-(num%(sizeof(Flag_Type)*8)))))
					{
						ka_printf("level %d,page_num %d error\n",i+1,num);
						return -1;
					}
				}
			}
		}
	}
	return 0;
}

static int check_buddy(void)
{
	unsigned int i,j;
	for(i=0;i<current_used_buddy_ptr->info.flag_array_num;++i)
	{
		if(0 != current_used_buddy_ptr->flag[i])
		{
			for(j=0;j<8*sizeof(Flag_Type);++j)
			{
				if(0x01 & (current_used_buddy_ptr->flag[i]>>(31-j))) /* corresponding bit is setted*/
				{
					unsigned int level = _get_level(i,j);
					if(0 != _check_buddy_flag_level(level,i*sizeof(Flag_Type)*8+j-current_used_buddy_ptr->level_flag_base[level-1]))
					{
						ka_printf("num %d,level %d error!\n",i*8*sizeof(Flag_Type)+j,level);
						return -1;
					}
				}
			}
		}
	}

	/*then check from level to flag*/
	return _check_buddy_level_flag();
}

#endif

void shell_buddy_debug(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	unsigned int i;
	unsigned int j;
	struct order_link buffer;
	struct buddy *buddy_ptr = (struct buddy *)_get_os_buddy_ptr_head();
	ASSERT(NULL != buddy_ptr,ASSERT_PARA_AFFIRM);
	j = 1;
	while(NULL != buddy_ptr)
	{
		ka_printf("NO.%u memory : \n",j++);
		ka_printf("=====================================================\n");
		ka_printf("the following informations are the buddy debug infomations\n");
		ka_printf("rest buddy space is %uKB\n",_get_current_buddy_space());
		ka_printf("the total space is %uKB\n",PAGE_SIZE_KB * current_used_buddy_ptr->info.page_num);
		ka_printf("now there are %u%% left\n",100*_get_current_buddy_space()/(PAGE_SIZE_KB * current_used_buddy_ptr->info.page_num));
	/*==========================================================================	*/
	/*flag info*/
		ka_printf("=====================================================\n");
		ka_printf("buddy flag infomation :\n");
		for(i=0;i<current_used_buddy_ptr->info.flag_array_num;++i)
		{
			ka_printf("flag %d to %d:",i<<5,(i<<5)+31);
			ka_printf("0x%x\n",current_used_buddy_ptr->flag[i]);
		}
	/*==========================================================================*/		
	/*level info*/
		ka_printf("=====================================================\n");
		ka_printf("each level's infomation :\n");
		for(i=0;i<current_used_buddy_ptr->info.max_level;++i)
		{
			ka_printf("level %d : ---",i+1);
			if(NOTHING == current_used_buddy_ptr->order_array[i])
			{
				ka_printf("nothing here\n");
			}
			else
			{
				buffer = current_used_buddy_ptr->link_body[current_used_buddy_ptr->order_array[i]];
				while(1)
				{
					ka_printf("num-%d--->",buffer.num);
					if(NOTHING == buffer.next)
					{
						ka_printf("end\n");
						break ;
					}
					buffer = current_used_buddy_ptr->link_body[buffer.next];
				}
			}
		}
		ka_printf("=====================================================\n");
	/*=========================================================================*/
#if CONFIG_DEBUG_ON
	/*start check the buddy info*/
		if(0 == check_buddy())
		{
			
		}
		else
		{
			ka_printf("something wrong!\n");
		}
#endif
		buddy_ptr = (struct buddy *)_get_next_buddy_ptr_head(buddy_ptr);
	}
	ka_printf("now going to display the page allocation's info\n");
	dis_page_alloc_record();	
}
#endif

