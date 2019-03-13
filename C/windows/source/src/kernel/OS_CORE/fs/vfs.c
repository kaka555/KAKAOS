#include <vfs.h>
#include <myMicroLIB.h>
#include <myassert.h>
#include <os_error.h>

static struct dentry_operations default_dentry_file_operations = {};
#define default_dentry_file_op_ptr (&default_dentry_file_operations)

static struct dentry_operations default_dentry_folder_operations = {};
#define default_dentry_folder_op_ptr (&default_dentry_folder_operations)

/* the statement of basic dentry */
static struct dentry root_dentry;
static struct dentry dev_dentry;

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
}

void __init_vfs(void)
{
	_init_dentry(&root_dentry,NULL,default_dentry_file_op_ptr,NULL,"/",FLAG_FOLDER);
	_init_dentry(&dev_dentry,&root_dentry,default_dentry_file_op_ptr,NULL,"dev",FLAG_FOLDER);
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
