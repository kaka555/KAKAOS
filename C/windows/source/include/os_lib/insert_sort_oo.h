#ifndef _INSERT_SORT_OO_H
#define _INSERT_SORT_OO_H

#define PUBLIC 
#define PRIVATE 

#include <double_linked_list.h>
#include <kakaosstdint.h>

#define DECLEAR_INSERT_SORT_DATA(name) \
	struct insert_sort_data name

typedef UINT64 id_type;

struct insert_sort_data
{
	void *data_ptr;
	id_type identify_data;  /*if data is equal,use this data to identify the data's owner*/
	struct list_head data_list;
};

void init_insert_sort_data(struct insert_sort_data *insert_sort_data_ptr,void *data_ptr);
void init_insert_sort_data_with_id(struct insert_sort_data *insert_sort_data_ptr,void *data_ptr,id_type identify_data);

/********************************************
*     insert data into insert-sort-list
*     base on the sequence "<="
*********************************************/
struct insert_sort_entity
{
PUBLIC
/*
	int (*insert_into)(struct insert_sort_data *data_ptr,struct insert_sort_entity *insert_sort_entity_ptr);
	struct insert_sort_data *(*find_data_with_id)(struct insert_sort_entity *insert_sort_entity_ptr,void *data_ptr,id_type identify_data);
	struct insert_sort_data *(*find_data)(struct insert_sort_entity *insert_sort_entity_ptr,void *data_ptr);
	struct insert_sort_data * (*delete_head)(struct insert_sort_entity *insert_sort_entity_ptr);
	struct insert_sort_data * (*delete_data)(struct insert_sort_entity *insert_sort_entity_ptr,void *data_ptr);
	struct insert_sort_data * (*delete_data_with_id)(struct insert_sort_entity *insert_sort_entity_ptr,void *data_ptr,id_type identify_data);
*/
PRIVATE
	struct list_head data_list_head;
	unsigned int data_num;
	/*if return value is negative,means data1<data2*/
	/*if return value is zero    ,means data1==data2*/
	/*if return value is positive,means data1>data2*/
	int (*compare)(struct insert_sort_data *data1,struct insert_sort_data *data2);		/*must realize*/
	int (*get_data_ptr)(struct insert_sort_data *insert_sort_data_ptr,void *data_ptr); 	/* can be NULL*/
	int (*value_cmp)(void *data1,void *data2);//must realize
};

int init_insert_sort_entity(
	struct insert_sort_entity *insert_sort_entity_ptr,
	int (*compare)(struct insert_sort_data *data1,struct insert_sort_data *data2),
	int (*get_data_ptr)(struct insert_sort_data *insert_sort_data_ptr,void *data_ptr),
	int (*value_cmp)(void *data1,void *data2)
	);

int insert_sort_insert_into(struct insert_sort_data *data_ptr,struct insert_sort_entity *insert_sort_entity_ptr);
struct insert_sort_data * insert_sort_find_data(struct insert_sort_entity *insert_sort_entity_ptr,void *data_ptr);
struct insert_sort_data * insert_sort_find_data_with_id(struct insert_sort_entity *insert_sort_entity_ptr,void *data_ptr,id_type identify_data);
struct insert_sort_data * insert_sort_delete_head(struct insert_sort_entity *insert_sort_entity_ptr);
struct insert_sort_data * insert_sort_delete_data(struct insert_sort_entity *insert_sort_entity_ptr,void *data_ptr);
struct insert_sort_data * insert_sort_delete_data_with_id(struct insert_sort_entity *insert_sort_entity_ptr,void *data_ptr,id_type identify_data);
/*struct insert_sort_data * insert_sort_find_first_ge(struct insert_sort_entity *insert_sort_entity_ptr,void *data_ptr);*/
/*struct insert_sort_data * insert_sort_get_next_data_ptr(struct insert_sort_data *data_ptr);*/
struct insert_sort_data * insert_sort_get_first_data_ptr(struct insert_sort_entity *insert_sort_entity_ptr);

#endif
