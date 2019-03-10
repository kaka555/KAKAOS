#include <insert_sort_oo.h>
#include <myassert.h>
#include <ka_configuration.h>
#include <os_error.h>

/*public function for insert sort*/
int insert_sort_insert_into(struct insert_sort_data *data_ptr,struct insert_sort_entity *insert_sort_entity_ptr)
{
	ASSERT(NULL != data_ptr);
	ASSERT(NULL != insert_sort_entity_ptr);
	struct insert_sort_data *buffer;
	struct list_head *pos;
	/* find a suitable position to insert it*/
	list_for_each(pos,&insert_sort_entity_ptr->data_list_head)
	{
		buffer = list_entry(pos,struct insert_sort_data,data_list);
		if(insert_sort_entity_ptr->compare(buffer,data_ptr) > 0) // data_ptr<=buffer
		{
			/*insert it here*/
			list_add_tail(&data_ptr->data_list,pos);
			++(insert_sort_entity_ptr->data_num);
			ASSERT(insert_sort_entity_ptr->data_num > 0);
			return FUN_EXECUTE_SUCCESSFULLY;
		}
	}
	/*insert it at the end*/
	list_add_tail(&data_ptr->data_list,pos);
	++(insert_sort_entity_ptr->data_num);
	ASSERT(insert_sort_entity_ptr->data_num > 0);
	return FUN_EXECUTE_SUCCESSFULLY;
}

/*if (*data_ptr) is found(no need to match id),return it's struct insert_sort_data's ptr
**else , return NULL*/
struct insert_sort_data * insert_sort_find_data(struct insert_sort_entity *insert_sort_entity_ptr,void *data_ptr)
{
	ASSERT(NULL != data_ptr);
	ASSERT(NULL != insert_sort_entity_ptr);
	struct list_head *pos;
	struct insert_sort_data *buffer;
	/*find from start to the end*/
	list_for_each(pos,&insert_sort_entity_ptr->data_list_head)
	{
		buffer = list_entry(pos,struct insert_sort_data,data_list);
		if(0 == insert_sort_entity_ptr->value_cmp(buffer->data_ptr,data_ptr))
		{
			return buffer;
		}
	}
	return NULL;
}

/*the same function with insert_sort_find_data() but also match id*/
struct insert_sort_data * insert_sort_find_data_with_id(struct insert_sort_entity *insert_sort_entity_ptr,void *data_ptr,id_type identify_data)
{
	ASSERT(NULL != data_ptr);
	ASSERT(NULL != insert_sort_entity_ptr);
	struct list_head *pos;
	struct insert_sort_data *buffer;
	list_for_each(pos,&insert_sort_entity_ptr->data_list_head)
	{
		buffer = list_entry(pos,struct insert_sort_data,data_list);
		if((0 == insert_sort_entity_ptr->value_cmp(buffer->data_ptr,data_ptr)) && (buffer->identify_data == identify_data))
		{
			return buffer;
		}
	}
	return NULL;
}

/*remove the first(smallest) data from the list and return it's struct insert_sort_data's ptr
**if there is no data in the list , then return NULL*/
struct insert_sort_data * insert_sort_delete_head(struct insert_sort_entity *insert_sort_entity_ptr)
{
	ASSERT(NULL != insert_sort_entity_ptr);
	if(list_empty(&insert_sort_entity_ptr->data_list_head))
	{
		return NULL;
	}
	struct list_head *pos;
	pos = insert_sort_entity_ptr->data_list_head.next;
	list_del(pos);
	ASSERT(insert_sort_entity_ptr->data_num > 0);
	--(insert_sort_entity_ptr->data_num);
	return list_entry(pos,struct insert_sort_data,data_list);
}

/*remove the data match value (*data_ptr) from the list and return it's struct insert_sort_data's ptr
**if there is no data matched , then return NULL*/
struct insert_sort_data * insert_sort_delete_data(struct insert_sort_entity *insert_sort_entity_ptr,void *data_ptr)
{
	ASSERT(NULL != insert_sort_entity_ptr);
	struct insert_sort_data *buffer_ptr;
	buffer_ptr = insert_sort_find_data(insert_sort_entity_ptr,data_ptr);
	if(NULL == buffer_ptr)/*no such data*/
	{
		return NULL;
	}
	else
	{
		list_del(&buffer_ptr->data_list);
		ASSERT(insert_sort_entity_ptr->data_num > 0);
		--(insert_sort_entity_ptr->data_num);
		return buffer_ptr;
	}
}

/*the same as function insert_sort_delete_data() but also match id*/
struct insert_sort_data * insert_sort_delete_data_with_id(
	struct insert_sort_entity *insert_sort_entity_ptr,
	void *data_ptr,
	id_type identify_data)
{
	ASSERT(NULL != insert_sort_entity_ptr);
	struct insert_sort_data *buffer_ptr;
	buffer_ptr = insert_sort_find_data_with_id(insert_sort_entity_ptr,data_ptr,identify_data);
	if(NULL == buffer_ptr)/*no such data*/
	{
		return NULL;
	}
	else
	{
		list_del(&buffer_ptr->data_list);
		ASSERT(insert_sort_entity_ptr->data_num > 0);
		--(insert_sort_entity_ptr->data_num);
		return buffer_ptr;
	}
}

/*initialize the struct insert_sort_data with data (*data_ptr)*/
void init_insert_sort_data(struct insert_sort_data *insert_sort_data_ptr,void *data_ptr)
{
	ASSERT(NULL != insert_sort_data_ptr);
	ASSERT(NULL != data_ptr);
	insert_sort_data_ptr->data_ptr = data_ptr;
	INIT_LIST_HEAD(&insert_sort_data_ptr->data_list);
}

/*initialize the struct insert_sort_data with data (*data_ptr) and id*/
void init_insert_sort_data_with_id(struct insert_sort_data *insert_sort_data_ptr,void *data_ptr,id_type identify_data)
{
	init_insert_sort_data(insert_sort_data_ptr,data_ptr);
	insert_sort_data_ptr->identify_data = identify_data;
}

/*initialize the struct insert_sort_entity*/
/*user have to realize three funtion in the following parameter*/
int init_insert_sort_entity(
	struct insert_sort_entity *insert_sort_entity_ptr,
	int (*compare)(struct insert_sort_data *data1,struct insert_sort_data *data2),
	int (*get_data_ptr)(struct insert_sort_data *insert_sort_data_ptrm,void *data_ptr),
	int (*value_cmp)(void *data1,void *data2)
)
{
	ASSERT(NULL != insert_sort_entity_ptr);
PRIVATE
	insert_sort_entity_ptr->data_num             = 0;
	insert_sort_entity_ptr->compare              = compare;
	insert_sort_entity_ptr->get_data_ptr         = get_data_ptr;
	insert_sort_entity_ptr->value_cmp            = value_cmp;
	INIT_LIST_HEAD(&insert_sort_entity_ptr->data_list_head);
	return FUN_EXECUTE_SUCCESSFULLY;
}


struct insert_sort_data * insert_sort_get_first_data_ptr(struct insert_sort_entity *insert_sort_entity_ptr)
{
	ASSERT(NULL != insert_sort_entity_ptr);
	if(list_empty(&insert_sort_entity_ptr->data_list_head))
	{
		return NULL;
	}
	return list_entry(insert_sort_entity_ptr->data_list_head.next,struct insert_sort_data,data_list);
}
