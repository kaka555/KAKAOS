#include <os_suspend.h>
#include <double_linked_list.h>
#include <ka_configuration.h>
#include <myassert.h>
#include <os_ready.h>
#include <sys_init_fun.h>

static struct list_head suspend_list_head;

static void __INIT __init_suspend_list(void)
{
	INIT_LIST_HEAD(&suspend_list_head);
}
INIT_FUN(__init_suspend_list,1);

void _insert_into_suspend_list(TCB *const TCB_ptr)
{
	ASSERT(NULL != TCB_ptr,ASSERT_INPUT);
	list_add_tail(&TCB_ptr->suspend_list,&suspend_list_head);
}

int _remove_from_suspend_list(TCB *TCB_ptr)
{
	ASSERT(NULL != TCB_ptr,ASSERT_INPUT);
	struct list_head *pos;
	list_for_each(pos,&suspend_list_head)
	{
		if(&TCB_ptr->suspend_list == pos)
		{
			list_del(pos);
			_insert_ready_TCB(TCB_ptr);
			return FUN_EXECUTE_SUCCESSFULLY;
		}
	}
	return -ERROR_USELESS_INPUT;
}
