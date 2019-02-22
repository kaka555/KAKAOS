#include <os_suspend.h>
#include <double_linked_list.h>
#include <ka_configuration.h>
#include <myassert.h>
#include <os_ready.h>

static struct list_head suspend_list_head;

void __init_suspend_list(void)
{
	INIT_LIST_HEAD(&suspend_list_head);
}

inline int  insert_into_suspend_list(TCB *TCB_ptr)
{
	ASSERT(NULL != TCB_ptr);
#if CONFIG_PARA_CHECK
	if(NULL == TCB_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(insert_into_suspend_list,TCB_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	list_add_tail(&TCB_ptr->suspend_list,&suspend_list_head);
	return FUN_EXECUTE_SUCCESSFULLY;
}

int remove_from_suspend_list(TCB *TCB_ptr)
{
	ASSERT(NULL != TCB_ptr);
#if CONFIG_PARA_CHECK
	if(NULL == TCB_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(remove_from_suspend_list,TCB_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	struct list_head *pos;
	list_for_each(pos,&suspend_list_head)
	{
		if(&TCB_ptr->suspend_list == pos)
		{
			list_del(pos);
			insert_ready_TCB(TCB_ptr);
			return FUN_EXECUTE_SUCCESSFULLY;
		}
	}
	return -ERROR_VALUELESS_INPUT;
}

