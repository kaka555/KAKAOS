#include <vfs.h>
#include <myMicroLIB.h>
#include <myassert.h>
#include <os_error.h>
#include <export.h>

static struct dentry_operations default_dentry_file_operations = {};
#define default_dentry_file_op_ptr (&default_dentry_file_operations)

static struct dentry_operations default_dentry_folder_operations = {};
#define default_dentry_folder_op_ptr (&default_dentry_folder_operations)

static struct file_operations default_file_operations = {};
#define defalut_file_operations_ptr (&default_file_operations)

/* the statement of basic dentry */
static struct dentry root_dentry;
static struct dentry dev_dentry;
static struct dentry *current_dentry_ptr = NULL;

static void _init_dentry(
	struct dentry *dentry_ptr,
	struct dentry *parent_ptr,  
	struct dentry_operations *dentry_operations_ptr,
	struct inode *inode_ptr,
	const char *name,
	enum DENTRY_INIT_FLAG flag
	)
{
	ASSERT(NULL != dentry_ptr);
	dentry_ptr->parent = parent_ptr;
	dentry_ptr->d_op = dentry_operations_ptr;
	dentry_ptr->d_inode = inode_ptr;
	dentry_ptr->name = name;
	INIT_LIST_HEAD(&dentry_ptr->subdirs);
	INIT_LIST_HEAD(&dentry_ptr->child);
	dentry_ptr->flag = FLAG_DENTRY_DEFAULT;
	if(FLAG_FOLDER == flag)
	{
		set_dentry_flag(dentry_ptr,FLAG_DENTRY_FOLDER);
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
		if(FLAG_FOLDER == flag)
		{
			dentry_ptr->d_op = default_dentry_folder_op_ptr;
		}
		else
		{
			dentry_ptr->d_op = default_dentry_file_op_ptr;
		}
	}
	if(!inode_ptr)
	{
		set_dentry_flag(dentry_ptr,FLAG_DENTRY_HARD);
	}
	_mutex_init(&dentry_ptr->d_mutex);
}

void __init_vfs(void)
{
	_init_dentry(&root_dentry,NULL,default_dentry_file_op_ptr,NULL,"/",FLAG_FOLDER);
	_init_dentry(&dev_dentry,&root_dentry,default_dentry_file_op_ptr,NULL,"dev",FLAG_FOLDER);
	current_dentry_ptr = &root_dentry; // set pwd
}

struct dentry *_dentry_alloc_and_init(
	struct dentry *parent_ptr,  
	struct dentry_operations *dentry_operations_ptr,
	struct inode *inode_ptr,
	const char *name,
	enum DENTRY_INIT_FLAG flag
	)
{
	ASSERT(NULL != parent_ptr);
	ASSERT(NULL != name);
	struct dentry *dentry_ptr = ka_malloc(sizeof(struct dentry));
	if(NULL == dentry_ptr)
	{
		return NULL;
	}
	_init_dentry(dentry_ptr,parent_ptr,dentry_operations_ptr,inode_ptr,name,flag);
	return dentry_ptr;
}

struct file *_file_alloc_and_init(
	struct file_operations *file_operations_ptr,
	struct dentry *dentry_ptr)
{
	ASSERT(NULL != dentry_ptr);
	struct file *file_ptr = ka_malloc(sizeof(struct file));
	if(NULL == file_ptr)
	{
		return NULL;
	}
	file_ptr->f_op = file_operations_ptr;
	if(!file_operations_ptr)
	{
		file_ptr->f_op = defalut_file_operations_ptr;
	}
	file_ptr->offset = 0;
	file_ptr->f_den = dentry_ptr;
	file_ptr->f_mode = FILE_MODE_DEFAULT;
	return file_ptr;
}

struct inode *_inode_alloc_and_init(
	unsigned long file_size)
{
	struct inode *inode_ptr = ka_malloc(sizeof(struct inode));
	if(NULL == inode_ptr)
	{
		return NULL;
	}
	inode_ptr->file_size = file_size;
	inode_ptr->ref = 0;
}

int change_name(struct file *file_ptr,const char *name)
{

}
EXPORT_SYMBOL(change_name);

int add_folder(const char *path,const char *folder_name)
{

}
EXPORT_SYMBOL(add_folder);

int add_file(const char *path,struct file_operations *f_op,const char *file_name)
{

}
EXPORT_SYMBOL(add_file);

struct file *open(const char *path,enum FILE_FLAG flag)
{

}
EXPORT_SYMBOL(open);

int close(struct file *file_ptr)
{

}
EXPORT_SYMBOL(close);

int read(struct file *file_ptr,void *buffer,unsigned int len)
{

}
EXPORT_SYMBOL(read);

int write(struct file *file_ptr,void *buffer,unsigned int len)
{

}
EXPORT_SYMBOL(write);

int lseek(struct file *file_ptr,int offset)
{

}
EXPORT_SYMBOL(lseek);

int ioctl(struct file *file_ptr,int cmd,int args)
{

}
EXPORT_SYMBOL(ioctl);
