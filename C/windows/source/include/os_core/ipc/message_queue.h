#ifndef _MESSAGE_QUEUE_H
#define _MESSAGE_QUEUE_H

#include <double_linked_list.h>
#include <insert_sort_oo.h>

typedef enum MSG_WAIT_FLAG {
	MSG_FLAG_WAIT				= 0,
	MSG_FLAG_NON_BLOCKING 		= 1
}MSG_FLAG;

/*use KA_MALLOC to allocate the room for message*/
struct message{
	struct list_head message_list;
	unsigned int message_size;
	void *message_data_ptr;
	UINT64 tick;
};

typedef struct message_queue_block{
	struct list_head message_list;
	struct insert_sort_entity message_array_insert_sort_wait_TCB_list;
	struct insert_sort_entity message_array_insert_sort_put_TCB_list;
	unsigned int put_num; /* the task num that wait for put message*/
	unsigned int wait_num;/* the task num that wait for get message*/
	char *name;
	unsigned int current_message_num; /* current message's num; should <= max_message_num*/
	unsigned int max_message_num;	  /* the max message's num that this MQB can hold */
}MQB;

#define ERROR_MSG_FULL 		1
#define MSG_OUT_OF_TIME		2

int _msg_init(MQB *MQB_ptr,char *name,unsigned int max_message_num);
int msg_init(MQB *MQB_ptr,char *name,unsigned int max_message_num);
int _msg_send(MQB *MQB_ptr,struct message *message_ptr,MSG_FLAG flag,unsigned int time);
int msg_send(MQB *MQB_ptr,struct message *message_ptr,MSG_FLAG flag,unsigned int time);
int _msg_receive(MQB *MQB_ptr,struct message **message_ptr,MSG_FLAG flag,unsigned int time);
int msg_receive(MQB *MQB_ptr,struct message **message_ptr,MSG_FLAG flag,unsigned int time);
int _msg_del(MQB *MQB_ptr);
int msg_del(MQB *MQB_ptr);

int message_init(struct message *message_ptr,unsigned int message_size,void *data);

#endif
