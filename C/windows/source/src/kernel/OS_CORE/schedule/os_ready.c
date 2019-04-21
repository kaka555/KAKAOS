#include <os_ready.h>
#include <heap_oo.h>
#include <myassert.h>
#include <os_TCB_list.h>
#include <os_error.h>
#include <myMicroLIB.h>
#include <task_state.h>
#include <sys_init_fun.h>

/**
 * use array to manage ready TCB
 */

/*==============================================*/
/*this typedef associate with macro PRIO_MAX in TCB.h*/
typedef unsigned char READY_GROUP_TYPE;
typedef unsigned char READY_TABLE_TYPE;

static const READY_TABLE_TYPE map[8*sizeof(READY_GROUP_TYPE)] = {
	0x01,
	0x02,
	0x04,
	0x08,
	0x10,
	0x20,
	0x40,
	0x80
};
static const INT8 unmap[256] = { 
    0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x00 to 0x0F  */ 
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x10 to 0x1F  */ 
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x20 to 0x2F  */ 
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x30 to 0x3F  */ 
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x40 to 0x4F  */ 
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x50 to 0x5F  */ 
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x60 to 0x6F  */ 
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x70 to 0x7F  */ 
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x80 to 0x8F  */ 
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x90 to 0x9F  */ 
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xA0 to 0xAF  */ 
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xB0 to 0xBF  */ 
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xC0 to 0xCF  */ 
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xD0 to 0xDF  */ 
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xE0 to 0xEF  */ 
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0        /* 0xF0 to 0xFF  */ 
}; 

static READY_GROUP_TYPE ready_group;
static READY_TABLE_TYPE ready_table[PRIO_MAX/(sizeof(READY_TABLE_TYPE)*8)];
/*==============================================*/

static void __INIT __init_ready_group(void)
{
	unsigned int i;
	ready_group = 0;
	for(i=0;i<PRIO_MAX/(sizeof(READY_TABLE_TYPE)*8);++i)
	{
		ready_table[i] = 0;
	}
}
INIT_FUN(__init_ready_group,1);

int _insert_ready_TCB(TCB *TCB_ptr)
{
	ASSERT(NULL != TCB_ptr,ASSERT_INPUT);
	TCB_ptr->task_state = STATE_READY;
	_increase_ready_num(TCB_ptr->prio);
	/*deal with the ready_group and ready_table*/
	ready_group |= map[TCB_ptr->prio / (8*sizeof(READY_GROUP_TYPE))];
	ready_table[TCB_ptr->prio / (8*sizeof(READY_GROUP_TYPE))] |= map[TCB_ptr->prio & 0x07];/*0x07 is associated with READY_TABLE_TYPE*/
	return FUN_EXECUTE_SUCCESSFULLY;
}

/*if os delete a task,use _delete_from_TCB_list() first*/
int _delete_TCB_from_ready(TCB *TCB_ptr)
{
	ASSERT(NULL != TCB_ptr,ASSERT_INPUT);
	_decrease_ready_num(TCB_ptr->prio);
	if(0 != _get_ready_num_from_TCB_list(TCB_ptr->prio))
	{
		return FUN_EXECUTE_SUCCESSFULLY;
	}
	/*first deal with the ready_table*/
	ready_table[TCB_ptr->prio / (8*sizeof(READY_GROUP_TYPE))] &= ~map[TCB_ptr->prio & 0x07];/*0x07 is associated with READY_TABLE_TYPE*/
	/*if the ready_table[index] is 0 after resetting,deal with ready_group*/
	if(0 == ready_table[TCB_ptr->prio / (8*sizeof(READY_GROUP_TYPE))])
	{
		ready_group &= ~map[TCB_ptr->prio / (8*sizeof(READY_GROUP_TYPE))];
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

TCB *_get_highest_prio_ready_TCB(void)
{
	int y;
	struct list_head *pos,*head;
	TCB *TCB_ptr;
	/*according to the ready_group and ready_table,get the highest priority
	**than, get the respoding head of the same-priority TCB_list*/
	y = unmap[ready_group]; /* the lowest '1' bit */
	head = _get_from_TCB_list(y*8*sizeof(READY_TABLE_TYPE) + unmap[ready_table[y]]);
	list_for_each(pos,head)
	{
		TCB_ptr = list_entry(pos,TCB,same_prio_list);
		if(STATE_READY == TCB_ptr->task_state)/*take the first TCB whose state is ready*/
		{
			return TCB_ptr;
		}
	}
	/*should not go here*/
	ASSERT(0,ASSERT_BAD_EXE_LOCATION);
	return NULL;
}

#if CONFIG_SHELL_EN
void shell_check_os_ready(void)
{
	unsigned int i,j;
	unsigned int num = 0;
	struct list_head *pos,*head;
	TCB *TCB_ptr;
	unsigned int ready_TCB_num;
	unsigned int TCB_num;
	/* check relation between ready_group and ready_table*/
	for(i=0;i<sizeof(READY_GROUP_TYPE)*8;++i)
	{
		if((ready_group >> i) & (0x01))
		{
			if(!ready_table[i])
			{
				++num;
				ka_printf("ready_group's bit %d error with ready_table[%d]\n",i,i);
			}
		}
	}
	/*according to TCB_list's information, check ready_table*/
	for(i=0;i<PRIO_MAX;++i)
	{
		head = _get_from_TCB_list(i);
		if(!list_empty(head))
		{
			ready_TCB_num = 0;
			TCB_num = 0;
			list_for_each(pos,head)
			{
				++TCB_num;
				TCB_ptr = list_entry(pos,TCB,same_prio_list);
				if(STATE_READY == TCB_ptr->task_state)
				{
					++ready_TCB_num;
					if(!(0x01 & (ready_table[i/(8*sizeof(READY_TABLE_TYPE))] >> (i%(8*sizeof(READY_TABLE_TYPE))))))
					{
						++num;
						ka_printf("priority %d's map error\n",i);
					}
				}
			}
			if(list_entry(head,struct TCB_list,head)->ready_num != ready_TCB_num)
			{
				ka_printf("priority %d's ready_num error\n",i);
			}
			if(list_entry(head,struct TCB_list,head)->TCB_num != TCB_num)
			{
				ka_printf("priority %d's TCB_num error\n",i);
			}
		}
	}
	/*according to ready_table's information, check TCB_list*/
	for(i=0;i<sizeof(READY_GROUP_TYPE)*8;++i)
	{
		for(j=0;j<8*sizeof(READY_TABLE_TYPE);++j)
		{
			if(0x01 & (ready_table[i] >> j))
			{
				head = _get_from_TCB_list(j + i*8*sizeof(READY_TABLE_TYPE));
				list_for_each(pos,head)
				{
					TCB_ptr = list_entry(pos,TCB,same_prio_list);
					if(STATE_READY == TCB_ptr->task_state)
					{
						goto out;
					}
				}
				++num;
				ka_printf("priority %d error\n",j + i*8*sizeof(READY_TABLE_TYPE));
			}
out:
		;
		}
	}
	/*conclusion*/
	if(num)
	{
		ka_printf("checking complete....%u error\n",num);
	}
}
#endif

