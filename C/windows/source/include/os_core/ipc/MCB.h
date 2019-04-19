#ifndef _MCB_H
#define _MCB_H
#include <TCB.h>
#include <double_linked_list.h>
#include <insert_sort_oo.h>

typedef enum MCB_WAIT_FLAG {
	MCB_FLAG_WAIT 				= 0,
	MCB_FLAG_NON_BLOCKING 		= 1,
	MCB_FLAG_BINARY				= 2 
}MCB_WAIT_FLAG;

#define MCB_TYPE_FLAG_DEFAULT	0
/* member-->flag attribution */
/*bit 0*/
#define MCB_TYPE_FLAG_COUNT		(0X00<<0)
#define MCB_TYPE_FLAG_BINARY	(0X01<<0)
#define MCB_type_is_binary(ptr)	((ptr)->flag & MCB_TYPE_FLAG_BINARY)

typedef struct message_struct{ 
	struct insert_sort_entity MCB_insert_sort_list;
	int   resource_num;	
	unsigned int flag;
}MCB;/*signal control block*/

#define MCB_OUT_OF_RESOURCE  	1
#define MCB_OUT_OF_TIME			2

/***********************/
int init_MCB(MCB *MCB_ptr,int m,unsigned int flag);
#define init_MCB_count(MCB_ptr,m) 	init_MCB(MCB_ptr,m,MCB_TYPE_FLAG_COUNT)
#define init_MCB_binary(MCB_ptr,m) 	init_MCB(MCB_ptr,m,MCB_TYPE_FLAG_BINARY)
int delete_MCB(MCB *MCB_ptr);
int _p(MCB *MCB_ptr,MCB_WAIT_FLAG flag,unsigned int time);
int p(MCB *MCB_ptr,MCB_WAIT_FLAG flag,unsigned int time);
int _v(MCB *MCB_ptr);
int v(MCB *MCB_ptr);
int clear_MCB_index(MCB *MCB_ptr);

/*****************************end of MCB******************/
#endif
