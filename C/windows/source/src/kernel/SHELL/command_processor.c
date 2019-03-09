#include <command_processor.h>
#include <myassert.h>
#include <myMicroLIB.h>
#include <kakaosstdint.h>

#if CONFIG_SHELL_EN

/******
*if you need to add command:  step 1: add the following three place if necessary
*                             step 2: add struct command in shell.c
*/
/*1.hash array to store command*/
static struct singly_list_head command_1_ptr_hash_array[ARRAY_SIZE];
static struct singly_list_head command_2_ptr_hash_array[ARRAY_SIZE];
static struct singly_list_head command_3_ptr_hash_array[ARRAY_SIZE];
static struct singly_list_head command_4_ptr_hash_array[ARRAY_SIZE];
static struct singly_list_head command_5_ptr_hash_array[ARRAY_SIZE];
static struct singly_list_head command_6_ptr_hash_array[ARRAY_SIZE];
static struct singly_list_head command_7_ptr_hash_array[ARRAY_SIZE];
static struct singly_list_head command_8_ptr_hash_array[ARRAY_SIZE];
/*2.struct command_processer entity*/
static struct command_processer command_1_processer = {1,command_1_ptr_hash_array};
static struct command_processer command_2_processer = {2,command_2_ptr_hash_array};
static struct command_processer command_3_processer = {3,command_3_ptr_hash_array};
static struct command_processer command_4_processer = {4,command_4_ptr_hash_array};
static struct command_processer command_5_processer = {5,command_5_ptr_hash_array};
static struct command_processer command_6_processer = {6,command_6_ptr_hash_array};
static struct command_processer command_7_processer = {7,command_7_ptr_hash_array};
static struct command_processer command_8_processer = {8,command_8_ptr_hash_array};
/*3.array to store command_processor_ptr*/
static struct command_processer* command_processer_ptr_array[] = {&command_1_processer,&command_2_processer,
	&command_3_processer,&command_4_processer,&command_5_processer,&command_6_processer,&command_7_processer,
	&command_8_processer};

struct command_processer *_get_command_processer(unsigned int num)
{
	if(!(num <= sizeof(command_processer_ptr_array)/sizeof(struct command_processer *)) || (num<1))
	{
		return NULL;
	}
	return command_processer_ptr_array[num-1];
}

static unsigned int command_list_hash(const char *command_ptr)
{
	unsigned int sum = 0;
	ASSERT(NULL != command_ptr);
	while((*command_ptr != ' ') && (*command_ptr != 0x0d) && (*command_ptr != 0x0a) && ('\0' != *command_ptr))
	{
		sum += *command_ptr++ - 'a';
	}
#if DEBUG_SHELL
	ka_printf("hash is %u\n",sum % ARRAY_SIZE);
#endif
	return sum % ARRAY_SIZE;
}

void __init_command_n_ptr_hash_array(void)
{
	unsigned int i,j;
	for(i=0;i<sizeof(command_processer_ptr_array)/sizeof(struct command_processer *);++i)
	{
		for(j=0;j<ARRAY_SIZE;++j)
		{
			INIT_SINGLY_LIST_HEAD(&(command_processer_ptr_array[i]->command_list_address[j]));
		}
	}
}

int _match_and_execute_command(
	int num,
	char const *argv[],
	struct command_processer * const command_processer_ptr)
{
	if(NULL == command_processer_ptr)
	{
		ka_printf("%s\n","command not found");
		return -1;
	}
	struct command *struct_command_ptr;
	struct singly_list_head *pos;
	singly_list_for_each(pos,&command_processer_ptr->command_list_address[command_list_hash(argv[0])])
	{
		struct_command_ptr = singly_list_entry(pos,struct command,list);
		if(0 == ka_strncmp(argv[0],struct_command_ptr->command_name,command_processer_ptr->command_length))
		{
			struct_command_ptr->f(num,argv);
			if(0 == ka_strcmp(argv[0],"r"))
			{
				return 1;
			}
			return 0;
		}
	}
	ka_printf("%s\n","command not found");
	return -1;
}

inline static void insert_struct_command(struct command *command_ptr,struct singly_list_head *hash_array)
{
	ASSERT(NULL != command_ptr);
	unsigned int hash = command_list_hash(command_ptr->command_name);
	singly_list_add(&command_ptr->list,&hash_array[hash]);
}

void _insert_struct_command_1(struct command *ptr)
{
	insert_struct_command(ptr,command_1_ptr_hash_array);
}
void _insert_struct_command_2(struct command *ptr)
{
	insert_struct_command(ptr,command_2_ptr_hash_array);
}
void _insert_struct_command_3(struct command *ptr)
{
	insert_struct_command(ptr,command_3_ptr_hash_array);
}
void _insert_struct_command_4(struct command *ptr)
{
	insert_struct_command(ptr,command_4_ptr_hash_array);
}

void _insert_struct_command_5(struct command *ptr)
{
	insert_struct_command(ptr,command_5_ptr_hash_array);
}

void _insert_struct_command_6(struct command *ptr)
{
	insert_struct_command(ptr,command_6_ptr_hash_array);
}
void _insert_struct_command_7(struct command *ptr)
{
	insert_struct_command(ptr,command_7_ptr_hash_array);
}
void _insert_struct_command_8(struct command *ptr)
{
	insert_struct_command(ptr,command_8_ptr_hash_array);
}

#endif

