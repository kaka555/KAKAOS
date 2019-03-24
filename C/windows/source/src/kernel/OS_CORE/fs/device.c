#include <device.h>
#include <vfs.h>
#include <os_error.h>
#include <export.h>
#include <myassert.h>
#include <printf_debug.h>

int _device_register(const char *dev_name,struct file_operations *file_operations_ptr,UINT32 flag)
{
	ASSERT((NULL != dev_name) && (NULL != file_operations_ptr));
	struct dentry *dev_dentry_ptr = _find_dentry(DEFAULT_DEVICE_PATH);
	ASSERT(NULL != dev_dentry_ptr);
	ASSERT(is_folder(dev_dentry_ptr));
	if(has_same_name_file(dev_dentry_ptr,dev_name))
	{
		return -ERROR_LOGIC;
	}
	struct inode *inode_ptr = _inode_alloc_and_init(NULL,file_operations_ptr,FLAG_INODE_DEVICE | flag);
	if(NULL == inode_ptr)
	{
		return -ERROR_NO_MEM;
	}
	struct dentry *buffer = _file_dentry_alloc_and_init(dev_dentry_ptr,inode_ptr,dev_name,FLAG_DEFAULT);
	if(NULL == buffer)
	{
		ka_free(inode_ptr);
		return -ERROR_NO_MEM;
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

int device_register(const char *dev_name,struct file_operations *file_operations_ptr,UINT32 flag)
{
	if((NULL == dev_name) || (NULL == file_operations_ptr))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _device_register(dev_name,file_operations_ptr,flag);
}
EXPORT_SYMBOL(device_register);

int _device_unregister(const char *dev_name)
{
	ASSERT(NULL != dev_name);
	struct dentry *dev_dentry_ptr = _find_dentry(DEFAULT_DEVICE_PATH);
	ASSERT(NULL != dev_dentry_ptr);
	ASSERT(is_folder(dev_dentry_ptr));
	struct dentry *buffer_ptr;
	struct list_head *head = &dev_dentry_ptr->subdirs;
	list_for_each_entry(buffer_ptr,head,child)
	{
		if(0 == ka_strcmp(dev_name,buffer_ptr->name))
		{
			ASSERT((!dentry_not_releasse(buffer_ptr)) && is_file(buffer_ptr)
							&& inode_is_soft(buffer_ptr->d_inode) && inode_is_dev(buffer_ptr->d_inode));
			__delete_file(buffer_ptr);
			KA_WARN(DEBUG_TYPE_VFS,"remove device %s\n",dev_name);
			return FUN_EXECUTE_SUCCESSFULLY;
		}
	}
	return -ERROR_LOGIC;
}

int device_unregister(const char *dev_name)
{
	if(NULL == dev_name)
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _device_unregister(dev_name);
}
EXPORT_SYMBOL(device_unregister);
