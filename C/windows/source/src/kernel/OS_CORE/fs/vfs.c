#include <vfs.h>
#include <myMicroLIB.h>
#include <myassert.h>
#include <os_error.h>
#include <export.h>
#include <os_cpu.h>

static struct file_operations default_file_operations = {};
#define defalut_file_operations_ptr (&default_file_operations)

/* the statement of basic dentry & inode */
static struct dentry root_dentry;
static struct dentry dev_dentry;
static struct dentry *current_dentry_ptr = NULL;
static struct inode root_inode;
static struct inode dev_inode;

static void _init_dentry(
	struct dentry *dentry_ptr,
	struct dentry *parent_ptr,  
	struct inode *inode_ptr,
	const char *name,
	UINT32 flag
	)
{
	ASSERT(NULL != dentry_ptr);
	dentry_ptr->parent = parent_ptr;
	dentry_ptr->d_inode = inode_ptr;
	dentry_ptr->name = name;
	INIT_LIST_HEAD(&dentry_ptr->subdirs);
	INIT_LIST_HEAD(&dentry_ptr->child);
	dentry_ptr->flag = FLAG_DENTRY_DEFAULT;
	dentry_ptr->file_ptr = NULL;
	if((FLAG_FOLDER & flag) == flag)
	{
		set_dentry_flag(dentry_ptr,FLAG_DENTRY_FOLDER);
	}
	if((FLAG_NAME_CHANGE_DIS & flag) == flag)
	{
		set_dentry_flag(dentry_ptr,FLAG_DENTRY_NAME_CHANGE_DIS);
	}
	if(!parent_ptr) /* root */
	{
		set_dentry_flag(dentry_ptr,FLAG_DENTRY_ROOT);
	}
	else
	{
		list_add_tail(&dentry_ptr->child,&parent_ptr->subdirs);
	}
	if(!dentry_operations_ptr) /* use defalut operations */
	{
		if((FLAG_FOLDER & flag) == flag)
		{
			dentry_ptr->d_op = default_dentry_folder_op_ptr;
		}
		else
		{
			dentry_ptr->d_op = default_dentry_file_op_ptr;
		}
	}
	_mutex_init(&dentry_ptr->d_mutex);
}

void __init_vfs(void)
{
	_init_dentry(&root_dentry,NULL,default_dentry_file_op_ptr,NULL,"/",FLAG_FOLDER|FLAG_NAME_CHANGE_DIS);
	_init_dentry(&dev_dentry,&root_dentry,default_dentry_file_op_ptr,NULL,"dev",FLAG_FOLDER|FLAG_NAME_CHANGE_DIS);
	current_dentry_ptr = &root_dentry; // set pwd
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
	ASSERT(NULL != dentry_ptr->d_inode.i_f_ops);
	file_ptr->f_op = dentry_ptr->d_inode.i_f_ops;
	file_ptr->offset = 0;
	file_ptr->f_den = dentry_ptr;
	file_ptr->f_mode = FILE_MODE_DEFAULT | flag;
	file_ptr->ref = 0;
	return file_ptr;
}

static void _inode_init(
	unsigned long file_size,
	struct inode_operations *inode_operations_ptr,
	struct file_operations *file_operations_ptr,
	UINT32 flag,struct inode *inode_ptr)
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

int _change_name(struct file *file_ptr,const char *name)
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
	if(dentry_ptr->d_inode.inode_ops->change_name(&dentry_ptr->d_inode,dentry_ptr) < 0)
	{
		ka_printf("write disk error\n");
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

int change_name(struct file *file_ptr,const char *name)
{
	if((NULL == path) || (NULL == folder_name))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _change_name(struct file *file_ptr,const char *name)
}
EXPORT_SYMBOL(change_name);

static struct dentry *_get_subdir(const struct dentry *parent,const char *path_name,unsigned int subdir_name_len)
{
	ASSERT((NULL != parent) && (NULL != path_name));
	struct list_head *head = &parent->subdirs;
	struct dentry *subdir;
	list_for_each_entry(subdir,head,child)
	{
		if(0 == ka_strncmp(subdir->name,path_name,subdir_name_len))
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
	if('/' == *path)
	{
		dentry_ptr = &root_dentry;
	}
	else
	{
		dentry_ptr = current_dentry_ptr;
	}
	char *cur_path = path+1;
	unsigned int name_len = _get_subdir_name_len(cur_path);
	while(0 != name_len) /* not the last dentry */
	{
		dentry_ptr = _get_subdir(dentry_ptr,cur_path,name_len);
		if(NULL == dentry_ptr) /* no such dentry */
		{
			return -NULL;
		}
		cur_path += name_len + 1;
		name_len = _get_subdir_name_len(cur_path);
	}
	dentry_ptr = _get_subdir(dentry_ptr,cur_path,name_len);
	if(NULL == dentry_ptr) /* no such dentry */
	{
		return -NULL;
	}
}

int _add_folder(const char *path,const char *folder_name)
{
	ASSERT((NULL != path) && (NULL != folder_name));
	/* find the corresponding dentry */
	struct dentry *target_dentry = _find_dentry(path);
	if(NULL == target_dentry)
	{
		return -ERROR_LOGIC;
	}
	struct dentry *buffer = _folder_dentry_alloc_and_init(dentry_ptr,NULL,folder_name,FLAG_DEFAULT);
	if(NULL == buffer)
	{
		return -ERROR_NO_MEM;
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

int add_folder(const char *path,const char *folder_name)
{
	if((NULL == path) || (NULL == folder_name))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _add_folder(path,folder_name);
}
EXPORT_SYMBOL(add_folder);

int _add_file(const char *path,struct file_operations *f_op,const char *file_name)
{
	ASSERT((NULL != path) && (NULL != file_name));
	/* find the corresponding dentry */
	struct dentry *target_dentry = _find_dentry(path);
	if(NULL == target_dentry)
	{
		return -ERROR_LOGIC;
	}
	struct dentry *buffer = _file_dentry_alloc_and_init(dentry_ptr,NULL,folder_name,FLAG_DEFAULT);
	if(NULL == buffer)
	{
		return -ERROR_NO_MEM;
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

int add_file(const char *path,struct file_operations *f_op,const char *file_name)
{
	if((NULL == path) || (NULL == file_name))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _add_file(path,f_op,file_name);
}
EXPORT_SYMBOL(add_file);

void _fget(struct file *file_ptr)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	++(file_ptr->ref);
	CPU_CRITICAL_EXIT();
}

int _fput(struct file *file_ptr)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	--(file_ptr->ref);
	CPU_CRITICAL_EXIT();
}
