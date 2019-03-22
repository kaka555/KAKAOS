#include <vfs.h>
#include <myMicroLIB.h>
#include <myassert.h>
#include <os_error.h>
#include <export.h>
#include <os_cpu.h>
#include <sys_init_fun.h>
#include <printf_debug.h>
#include <vector.h>
#include <command_processor.h>
#include <shell.h>

extern int default_open(struct file *file_ptr);
extern int default_close(struct file *file_ptr);
extern int default_read(struct file *file_ptr,void *buffer,unsigned int len,unsigned int offset);
extern int default_write(struct file *file_ptr,void *buffer,unsigned int len,unsigned int offset);
extern int default_llseek(struct file *file_ptr,int offset,enum llseek_from from);
extern int default_ioctl(struct file *file_ptr,int cmd,int args);

static struct file_operations default_file_operations = {
	default_open,
	default_close,
	default_read,
	default_write,
	default_llseek,
	default_ioctl
};
#define defalut_file_operations_ptr (&default_file_operations)

extern int default_change_name(struct inode *inode_ptr,struct dentry *dentry_ptr);
extern int default_refresh(struct inode *inode_ptr,struct dentry *dentry_ptr);
extern int default_floader_cmp_file_name(struct inode *inode_ptr,const char *name);
extern int default_add_sub_file(struct inode *inode_ptr,const char *name);
extern int add_sub_folder(struct inode *inode_ptr,const char *folder_name);
extern int default_read_data(struct inode *inode_ptr,void *store_ptr,unsigned int len,unsigned int offset);
extern int default_write_data(struct inode *inode_ptr,void *data_ptr,unsigned int len,unsigned int offset);
static struct inode_operations default_inode_operations = {
	default_change_name,
	default_refresh,
	default_floader_cmp_file_name,
	default_add_sub_file,
	add_sub_folder,
	default_read_data,
	default_write_data
};
#define defalut_inode_operations_ptr (&default_inode_operations)

/* the statement of basic dentry & inode */
static struct dentry root_dentry;
static struct dentry dev_dentry;
static struct dentry *current_dentry_ptr = NULL;
static struct inode root_inode;
static struct inode dev_inode;

static void update_para_arv_vector(void);

static void _init_dentry(
	struct dentry *dentry_ptr,
	struct dentry *parent_ptr,  
	struct inode *inode_ptr,
	const char *name,
	UINT32 flag
	)
{
	ASSERT(NULL != dentry_ptr);
	dentry_ptr->d_parent = parent_ptr;
	dentry_ptr->d_inode = inode_ptr;
	dentry_ptr->name = (char *)name;
	dentry_ptr->ref = 0;
	INIT_LIST_HEAD(&dentry_ptr->subdirs);
	INIT_LIST_HEAD(&dentry_ptr->child);
	dentry_ptr->flag = FLAG_DENTRY_DEFAULT;
	if((FLAG_FOLDER & flag) == FLAG_FOLDER)
	{
		set_dentry_flag(dentry_ptr,FLAG_DENTRY_FOLDER);
	}
	if(dentry_name_not_changable(dentry_ptr))
	{
		set_dentry_flag(dentry_ptr,FLAG_DENTRY_NAME_CHANGE_DIS);
	}
	if(dentry_ptr == parent_ptr) /* root */
	{
		set_dentry_flag(dentry_ptr,FLAG_DENTRY_ROOT);
	}
	else
	{
		list_add_tail(&dentry_ptr->child,&parent_ptr->subdirs);
	}
	_mutex_init(&dentry_ptr->d_mutex);
}

struct dentry *_dentry_alloc_and_init(
	struct dentry *parent_ptr,  
	struct inode *inode_ptr,
	const char *name,
	UINT32 flag
	)
{
	ASSERT(NULL != parent_ptr);
	ASSERT(NULL != name);
	ASSERT(NULL != inode_ptr);
	struct dentry *dentry_ptr = ka_malloc(sizeof(struct dentry));
	if(NULL == dentry_ptr)
	{
		return NULL;
	}
	_init_dentry(dentry_ptr,parent_ptr,inode_ptr,name,flag);
	return dentry_ptr;
}

struct file *_file_alloc_and_init(
	struct dentry *dentry_ptr,UINT32 flag)
{
	ASSERT(NULL != dentry_ptr);
	struct file *file_ptr = ka_malloc(sizeof(struct file));
	if(NULL == file_ptr)
	{
		return NULL;
	}
	ASSERT(NULL != dentry_ptr->d_inode->i_f_ops);
	file_ptr->f_op = dentry_ptr->d_inode->i_f_ops;
	file_ptr->offset = 0;
	file_ptr->f_den = dentry_ptr;
	file_ptr->f_mode = FILE_MODE_DEFAULT | flag;
	/*file_ptr->ref = 0;*/
	return file_ptr;
}

static void _inode_init(
	unsigned long file_size, /* 0 means that this is a recently created file or a special device */
	struct inode_operations *inode_operations_ptr,
	struct file_operations *file_operations_ptr,
	UINT32 flag,
	struct inode *inode_ptr)
{
	ASSERT(NULL != inode_ptr);
	inode_ptr->file_size = file_size;
	inode_ptr->ref = 0;
	inode_ptr->inode_ops = inode_operations_ptr;
	inode_ptr->i_f_ops = file_operations_ptr;
	inode_ptr->flag = FLAG_INODE_DEFAULT | flag;
	if(NULL == inode_operations_ptr)
	{
		set_inode_flag(inode_ptr,FLAG_INODE_SOFT);
		inode_ptr->inode_ops = defalut_inode_operations_ptr;
	}
	if(NULL == file_operations_ptr)
	{
		inode_ptr->i_f_ops = defalut_file_operations_ptr;
	}
}

struct inode *_inode_alloc_and_init(
	unsigned long file_size,
	struct inode_operations *inode_operations_ptr,
	struct file_operations *file_operations_ptr,
	UINT32 flag)
{
	struct inode *inode_ptr = ka_malloc(sizeof(struct inode));
	if(NULL == inode_ptr)
	{
		return NULL;
	}
	_inode_init(file_size,inode_operations_ptr,file_operations_ptr,flag,inode_ptr);
	return inode_ptr;
}

int _rename(struct file *file_ptr,const char *name)
{
	ASSERT((NULL != file_ptr) && (NULL != name));
	struct dentry *dentry_ptr = file_ptr->f_den;
	if(dentry_ptr->flag | FLAG_DENTRY_NAME_CHANGE_DIS)
	{
		return -ERROR_SYS;
	}
	char *buffer = (char *)ka_malloc(ka_strlen(name));
	if(NULL == buffer)
	{
		return -ERROR_NO_MEM;
	}
	ka_strcpy(buffer,name);
	ka_free(dentry_ptr->name);
	dentry_ptr->name = buffer;
	if(dentry_ptr->d_inode->inode_ops->change_name(dentry_ptr->d_inode,dentry_ptr) < 0)
	{
		ka_printf("write disk error\n");
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

int rename(struct file *file_ptr,const char *name)
{
	if((NULL == file_ptr) || (NULL == name))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _rename(file_ptr,name);
}
EXPORT_SYMBOL(rename);

static int has_same_name_file(struct dentry *dentry_ptr,const char *file_name)
{
	struct dentry *buffer_ptr;
	struct list_head *head = &dentry_ptr->subdirs;
	list_for_each_entry(buffer_ptr,head,child)
	{
		if(0 == ka_strcmp(file_name,buffer_ptr->name))
		{
			return 1;
		}
	}
	return 0;
}

static struct dentry *_get_subdir(struct dentry *parent,const char *path_name,unsigned int subdir_name_len)
{
	ASSERT((NULL != parent) && (NULL != path_name));
	if('\0' == *path_name)
	{
		return parent;
	}
	struct list_head *head = &parent->subdirs;
	struct dentry *subdir;
	list_for_each_entry(subdir,head,child)
	{
		if(ka_strlen(subdir->name) != subdir_name_len)
		{
			continue ;
		}
		if(0 == ka_strcmp(subdir->name,path_name))
		{
			return subdir;
		}
	}
	return NULL;
}

/*
	return 0 means that it is the last dentry of the path  
 */
static unsigned int _get_subdir_name_len(const char *path_name)
{
	ASSERT(NULL != path_name);
	unsigned int i = 0;
	while('/' != *path_name)
	{
		++i;
		if('\0' == *path_name)
		{
			return 0;
		}
		++path_name;
	}
	return i;
}

struct dentry *_find_dentry(const char *path)
{
	ASSERT(NULL != path);
	struct dentry *dentry_ptr;
	char *cur_path;
	if('/' == *path)
	{
		dentry_ptr = &root_dentry;
		cur_path = (char *)(path+1);
	}
	else
	{
		dentry_ptr = current_dentry_ptr;
		cur_path = (char *)(path);
	}
	if(dentry_need_refresh(dentry_ptr))
	{
		if(dentry_ptr->d_inode->inode_ops->refresh(dentry_ptr->d_inode,dentry_ptr) < 0)
		{
			ka_printf("disk error\n");
			return NULL;
		}
	}
	unsigned int name_len = _get_subdir_name_len(cur_path);
	while(0 != name_len) /* not the last dentry */
	{
		dentry_ptr = _get_subdir(dentry_ptr,cur_path,name_len);
		if(NULL == dentry_ptr) /* no such dentry */
		{
			return NULL;
		}
		cur_path += name_len + 1;
		name_len = _get_subdir_name_len(cur_path);
	}
	return _get_subdir(dentry_ptr,cur_path,ka_strlen(cur_path));
}

int __add_folder(struct dentry *target_dentry_ptr,const char *folder_name,struct file_operations *file_operations_ptr)
{
	ASSERT(is_folder(target_dentry_ptr));
	if(has_same_name_file(target_dentry_ptr,folder_name))
	{
		return -ERROR_LOGIC;
	}
	if(target_dentry_ptr->d_inode->inode_ops->add_sub_folder(target_dentry_ptr->d_inode,folder_name) < 0)
	{
		return -ERROR_DISK;
	}
	unsigned int len = ka_strlen(folder_name)+1;
	char *name_buffer = ka_malloc(len);
	if(NULL == name_buffer)
	{
		KA_WARN(DEBUG_TYPE_VFS,"name_buffer malloc fail\n");
		return -ERROR_NO_MEM;
	}
	ka_strcpy(name_buffer,folder_name);
	name_buffer[len-1] = '\0';
	struct inode *inode_ptr;
	if(file_operations_ptr)
	{
		inode_ptr = _inode_alloc_and_init(0,target_dentry_ptr->d_inode->inode_ops,file_operations_ptr,target_dentry_ptr->d_inode->flag);
	}
	else
	{
		inode_ptr = _inode_alloc_and_init(0,target_dentry_ptr->d_inode->inode_ops,target_dentry_ptr->d_inode->i_f_ops,target_dentry_ptr->d_inode->flag);
	}
	if(NULL == inode_ptr)
	{
		KA_WARN(DEBUG_TYPE_VFS,"inode malloc fail\n");
		ka_free(name_buffer);
		return -ERROR_NO_MEM;
	}
	struct dentry *buffer = _folder_dentry_alloc_and_init(target_dentry_ptr,inode_ptr,name_buffer,FLAG_DEFAULT);
	if(NULL == buffer)
	{
		ka_free(name_buffer);
		ka_free(inode_ptr);
		return -ERROR_NO_MEM;
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

int _add_folder(const char *path,const char *folder_name,struct file_operations *file_operations_ptr)
{
	ASSERT((NULL != path) && (NULL != folder_name));
	/* find the corresponding dentry */
	struct dentry *target_dentry_ptr = _find_dentry(path);
	if(NULL == target_dentry_ptr)
	{
		return -ERROR_LOGIC;
	}
	return __add_folder(target_dentry_ptr,folder_name,file_operations_ptr);
}

int add_folder(const char *path,const char *folder_name,struct file_operations *file_operations_ptr)
{
	if((NULL == path) || (NULL == folder_name))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _add_folder(path,folder_name,file_operations_ptr);
}
EXPORT_SYMBOL(add_folder);

static int __add_file(struct dentry *target_dentry_ptr,const char *file_name,struct file_operations *file_operations_ptr)
{
	ASSERT(is_folder(target_dentry_ptr));
	KA_WARN(DEBUG_TYPE_VFS,"__add_file() get dentry name is %s\nfile name is %s\n",target_dentry_ptr->name,file_name);
	if(has_same_name_file(target_dentry_ptr,file_name))
	{
		return -ERROR_LOGIC;
	}
	if(target_dentry_ptr->d_inode->inode_ops->add_sub_file(target_dentry_ptr->d_inode,file_name) < 0)
	{
		return -ERROR_DISK;
	}
	unsigned int len = ka_strlen(file_name)+1;
	char *name_buffer = ka_malloc(len);
	if(NULL == name_buffer)
	{
		return -ERROR_NO_MEM;
	}
	ka_strcpy(name_buffer,file_name);
	name_buffer[len-1] = '\0';
	struct inode *inode_ptr;
	if(file_operations_ptr)
	{
		inode_ptr = _inode_alloc_and_init(0,target_dentry_ptr->d_inode->inode_ops,file_operations_ptr,target_dentry_ptr->d_inode->flag);
	}
	else
	{
		inode_ptr = _inode_alloc_and_init(0,target_dentry_ptr->d_inode->inode_ops,target_dentry_ptr->d_inode->i_f_ops,target_dentry_ptr->d_inode->flag);
	}
	if(NULL == inode_ptr)
	{
		ka_free(name_buffer);
		return -ERROR_NO_MEM;
	}
	struct dentry *buffer = _file_dentry_alloc_and_init(target_dentry_ptr,inode_ptr,name_buffer,FLAG_DEFAULT);
	if(NULL == buffer)
	{
		ka_free(name_buffer);
		ka_free(inode_ptr);
		return -ERROR_NO_MEM;
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

int _add_file(const char *path,const char *file_name,struct file_operations *file_operations_ptr)
{
	ASSERT((NULL != path) && (NULL != file_name));
	/* find the corresponding dentry */
	struct dentry *target_dentry_ptr = _find_dentry(path);
	if(NULL == target_dentry_ptr)
	{
		return -ERROR_LOGIC;
	}
	return __add_file(target_dentry_ptr,file_name,file_operations_ptr);
}

int add_file(const char *path,const char *file_name,struct file_operations *file_operations_ptr)
{
	if((NULL == path) || (NULL == file_name))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _add_file(path,file_name,file_operations_ptr);
}
EXPORT_SYMBOL(add_file);

static int __delete_file(struct dentry *dentry_ptr)
{
	ASSERT(NULL != dentry_ptr);
	ASSERT(list_empty(&dentry_ptr->subdirs));
	ASSERT(is_file(dentry_ptr));
	if(0 == _dref(dentry_ptr))
	{
		list_del(&dentry_ptr->child);
		ka_free(dentry_ptr);
		KA_WARN(DEBUG_TYPE_VFS,"remove file %s\n",dentry_ptr->name);
		return FUN_EXECUTE_SUCCESSFULLY;
	}
	else
	{
		ka_printf("file is currently used by task\n");
		return -ERROR_SYS;
	}
}

int _delete_file(const char *path)
{
	ASSERT((NULL != path) && (0 != ka_strcmp("/",path)));
	struct dentry *target_dentry_ptr = _find_dentry(path);
	if((NULL == target_dentry_ptr) || (!is_file(target_dentry_ptr)))
	{
		ka_printf("file path error\n");
		return -ERROR_LOGIC;
	}
	return __delete_file(target_dentry_ptr);
}

int delete_file(const char *path)
{
	if((NULL == path) || (0 == ka_strcmp("/",path)))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _delete_file(path);
}
EXPORT_SYMBOL(delete_file);

static int remove_all_dentry(struct dentry *root_dentry_ptr)
{
	ASSERT(NULL != root_dentry_ptr);
	struct dentry *dentry_buffer_ptr,*n;
	list_for_each_entry_safe(dentry_buffer_ptr,n,&root_dentry_ptr->subdirs,child)
	{
		if(is_folder(dentry_buffer_ptr))
		{
			if(remove_all_dentry(dentry_buffer_ptr) < 0)
			{
				return -ERROR_SYS;
			}
			list_del(&dentry_buffer_ptr->child);
			ka_free(dentry_buffer_ptr);
			KA_WARN(DEBUG_TYPE_VFS,"remove floder %s\n",dentry_buffer_ptr->name);
		}
		else
		{
			ASSERT(is_file(dentry_buffer_ptr));
			if(__delete_file(dentry_buffer_ptr) < 0)
			{
				return -ERROR_SYS;
			}
		}
	}
	list_del(&root_dentry_ptr->child);
	ka_free(root_dentry_ptr);
	return FUN_EXECUTE_SUCCESSFULLY;
}

int _delete_folder(const char *path)
{
	ASSERT((NULL != path) && (0 != ka_strcmp("/",path)));
	struct dentry *target_dentry_ptr = _find_dentry(path);
	if((NULL == target_dentry_ptr) || (!is_folder(target_dentry_ptr)))
	{
		ka_printf("folder path error\n");
		return -ERROR_LOGIC;
	}
	return remove_all_dentry(target_dentry_ptr);
}

int delete_folder(const char *path)
{
	if((NULL == path) || (0 == ka_strcmp("/",path)))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _delete_folder(path);
}
EXPORT_SYMBOL(delete_folder);

#if CONFIG_SHELL_EN

static Vector para_arv_vector;
static struct command *command_cd_ptr = NULL;
static struct command *command_rm_ptr = NULL;
static struct command *command_rmdir_ptr = NULL;

static void pwd(struct dentry *dentry_ptr)
{
	ASSERT(is_folder(dentry_ptr));
	if(dentry_ptr == (dentry_ptr->d_parent)) /* root */
	{
		ka_printf("/");
		return ;
	}
	pwd(dentry_ptr->d_parent);
	ka_printf("%s/",dentry_ptr->name);
	return ;
}

void shell_pwd(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	struct dentry *dentry_ptr = current_dentry_ptr;
	ASSERT(NULL != dentry_ptr);
	if(dentry_ptr == (dentry_ptr->d_parent)) /* root */
	{
		ka_printf("/\n");
		return ;
	}
	pwd(dentry_ptr->d_parent);
	ka_printf("%s\n",dentry_ptr->name);
	return ;
}

void shell_ls(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	ASSERT(NULL != current_dentry_ptr);
	struct dentry *dentry_ptr;
	struct list_head *head = &current_dentry_ptr->subdirs;
	list_for_each_entry(dentry_ptr,head,child)
	{
		ka_printf("%s\t",dentry_ptr->name);
	}
	ka_printf("\n");
}

void shell_cd(int argc, char const *argv[])
{
	if(2 != argc)
	{
		ka_printf("command error\n");
		return ;
	}
	if(0 == ka_strcmp("..",argv[1]))
	{
		current_dentry_ptr = current_dentry_ptr->d_parent;
		update_para_arv_vector();
		return ;
	}
	else if(0 == ka_strcmp(".",argv[1]))
	{
		return ;
	}
	struct dentry *dentry_ptr = _find_dentry(argv[1]);
	if((NULL == dentry_ptr) || (!is_folder(dentry_ptr)))
	{
		ka_printf("path error\n");
		return ;
	}
	current_dentry_ptr = dentry_ptr;
	update_para_arv_vector();
	return ;
}

/*
	touch filename;
	create a file
 */
void shell_touch(int argc, char const *argv[])
{
	if(2 != argc)
	{
		ka_printf("command error\n");
		return ;
	}
	int ret = __add_file(current_dentry_ptr,argv[1],NULL);
	if((ret < 0) && (-ERROR_LOGIC != ret))
	{
		ka_printf("create file fail\n");
		return ;
	}
	update_para_arv_vector();
}

void shell_cat(int argc, char const *argv[])
{

}

/*
	mkdir foldername
 */
void shell_mkdir(int argc, char const *argv[])
{
	if(2 != argc)
	{
		ka_printf("command error\n");
		return ;
	}
	int ret = __add_folder(current_dentry_ptr,argv[1],NULL);
	if((ret < 0) && (-ERROR_LOGIC != ret))
	{
		ka_printf("create folder fail\n");
		return ;
	}
	update_para_arv_vector();
}

void shell_rm(int argc, char const *argv[])
{
	if(2 != argc)
	{
		ka_printf("command error\n");
		return ;
	}
	delete_file(argv[1]);
	return ;
}

void shell_rmdir(int argc, char const *argv[])
{
	if(2 != argc)
	{
		ka_printf("command error\n");
		return ;
	}
	delete_folder(argv[1]);
	return ;
}

static void update_para_arv_vector(void)
{
	ASSERT(NULL != current_dentry_ptr);
	Vector_clean_up(&para_arv_vector);
	struct dentry *dentry_ptr;
	list_for_each_entry(dentry_ptr,&current_dentry_ptr->subdirs,child)
	{
		Vector_push_back(&para_arv_vector,dentry_ptr->name);
	}
	_set_command_para_list(command_cd_ptr,(char **)para_arv_vector.data_ptr,para_arv_vector.cur_len);
	_set_command_para_list(command_rm_ptr,(char **)para_arv_vector.data_ptr,para_arv_vector.cur_len);
	_set_command_para_list(command_rmdir_ptr,(char **)para_arv_vector.data_ptr,para_arv_vector.cur_len);
}

#endif

static void __init_vfs(void)
{
#if CONFIG_SHELL_EN
	int error;
	error = Vector_init(&para_arv_vector,16,MKVFADD(16));
	if(error < 0)
	{
		ka_printf("vfs init fail\n");
		while(1);
	}
	ASSERT(FUN_EXECUTE_SUCCESSFULLY == error);
#endif
	_inode_init(0,NULL,NULL,FLAG_INODE_SOFT|FLAG_INODE_READ|FLAG_INODE_WRITE,&root_inode);
	_inode_init(0,NULL,NULL,FLAG_INODE_SOFT|FLAG_INODE_READ|FLAG_INODE_WRITE,&dev_inode);
	_init_dentry(&root_dentry,&root_dentry,&root_inode,"/",FLAG_FOLDER|FLAG_NAME_CHANGE_DIS);
	root_dentry.d_parent = &root_dentry;
	_init_dentry(&dev_dentry,&root_dentry,&dev_inode,"dev",FLAG_FOLDER|FLAG_NAME_CHANGE_DIS);
	current_dentry_ptr = &root_dentry; /* set pwd */
#if CONFIG_SHELL_EN
/* add tab feature */
/* command cd */
	command_cd_ptr = _get_command_ptr("cd");
	ASSERT(NULL != command_cd_ptr);
/* command rm */
	command_rm_ptr = _get_command_ptr("rm");
	ASSERT(NULL != command_rm_ptr);
/* command rmdir */
	command_rmdir_ptr = _get_command_ptr("rmdir");
	ASSERT(NULL != command_rmdir_ptr);	
/* finally */
	update_para_arv_vector(); /* set tab list */
#endif
}
INIT_FUN(__init_vfs,2);
