#include <vfs.h>
#include <myMicroLIB.h>
#include <myassert.h>
#include <os_error.h>
#include <export.h>
#include <os_cpu.h>
#include <sys_init_fun.h>

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
extern int default_refresh(struct inode *inode_ptr);
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
	INIT_LIST_HEAD(&dentry_ptr->subdirs);
	INIT_LIST_HEAD(&dentry_ptr->child);
	dentry_ptr->flag = FLAG_DENTRY_DEFAULT;
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
	file_ptr->ref = 0;
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

static struct dentry *_get_subdir(struct dentry *parent,const char *path_name,unsigned int subdir_name_len)
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
	char *cur_path = (char *)(path+1);
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
	return _get_subdir(dentry_ptr,cur_path,name_len);
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
	if(target_dentry_ptr->d_inode->inode_ops->add_sub_folder(target_dentry_ptr->d_inode,folder_name) < 0)
	{
		return -ERROR_DISK;
	}
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
		return -ERROR_NO_MEM;
	}
	struct dentry *buffer = _folder_dentry_alloc_and_init(target_dentry_ptr,inode_ptr,folder_name,FLAG_DEFAULT);
	if(NULL == buffer)
	{
		ka_free(inode_ptr);
		return -ERROR_NO_MEM;
	}
	return FUN_EXECUTE_SUCCESSFULLY;
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

int _add_file(const char *path,const char *file_name,struct file_operations *file_operations_ptr)
{
	ASSERT((NULL != path) && (NULL != file_name));
	/* find the corresponding dentry */
	struct dentry *target_dentry_ptr = _find_dentry(path);
	if(NULL == target_dentry_ptr)
	{
		return -ERROR_LOGIC;
	}
	if(target_dentry_ptr->d_inode->inode_ops->add_sub_file(target_dentry_ptr->d_inode,file_name) < 0)
	{
		return -ERROR_DISK;
	}
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
		return -ERROR_NO_MEM;
	}
	struct dentry *buffer = _file_dentry_alloc_and_init(target_dentry_ptr,inode_ptr,file_name,FLAG_DEFAULT);
	if(NULL == buffer)
	{
		ka_free(inode_ptr);
		return -ERROR_NO_MEM;
	}
	return FUN_EXECUTE_SUCCESSFULLY;
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

static void __init_vfs(void)
{
	_inode_init(0,NULL,NULL,FLAG_INODE_SOFT|FLAG_INODE_READ|FLAG_INODE_WRITE,&root_inode);
	_inode_init(0,NULL,NULL,FLAG_INODE_SOFT|FLAG_INODE_READ|FLAG_INODE_WRITE,&dev_inode);
	_init_dentry(&root_dentry,NULL,&root_inode,"/",FLAG_FOLDER|FLAG_NAME_CHANGE_DIS);
	_init_dentry(&dev_dentry,&root_dentry,&dev_inode,"dev",FLAG_FOLDER|FLAG_NAME_CHANGE_DIS);
	current_dentry_ptr = &root_dentry; // set pwd
}
INIT_FUN(__init_vfs);
