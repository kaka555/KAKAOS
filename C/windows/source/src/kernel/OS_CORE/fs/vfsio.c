#include <vfs.h>
#include <myassert.h>
#include <os_error.h>
#include <export.h>

int _open(const char *path,enum FILE_FLAG flag,struct file **file_store_ptr)
{
	ASSERT(NULL != path);
	struct dentry *dentry_ptr = _find_dentry(path);
	if(NULL == dentry_ptr)
	{
		if(NULL != error_ptr)
			*error_ptr = -ERROR_LOGIC;
		return NULL;
	}
	struct file *file_ptr;
	switch(flag)
	{
		case FLAG_READONLY:
			if(!(dentry_ptr->d_inode.flag & FLAG_INODE_READ))
			{
				if(NULL != error_ptr)
					*error_ptr = -ERROR_SYS;
				return NULL;
			}
			if(dentry_ptr->file_ptr)
			{
				file_ptr = dentry_ptr->file_ptr;
				set_file_mode(file_ptr,FILE_MODE_READ);
				break ;
			}
			file_ptr = _file_alloc_and_init_READONLY(dentry_ptr);
			if(NULL == file_ptr)
			{
				if(NULL != error_ptr)
					*error_ptr = -ERROR_NO_MEM;
				return NULL;
			}
			break;
		case FLAG_WRITEONLY:
			if(!(dentry_ptr->d_inode.flag & FLAG_INODE_WRITE))
			{
				if(NULL != error_ptr)
					*error_ptr = -ERROR_SYS;
				return NULL;
			}
			if(dentry_ptr->file_ptr)
			{
				file_ptr = dentry_ptr->file_ptr;
				set_file_mode(file_ptr,FILE_MODE_WRITE);
				break ;
			}
			file_ptr = _file_alloc_and_init_WRITEONLY(dentry_ptr);
			if(NULL == file_ptr)
			{
				if(NULL != error_ptr)
					*error_ptr = -ERROR_NO_MEM;
				return NULL;
			}
			break;
		case FLAG_READ_WRITE:
			if(!(dentry_ptr->d_inode.flag & (FLAG_INODE_WRITE | FLAG_INODE_READ)))
			{
				if(NULL != error_ptr)
					*error_ptr = -ERROR_SYS;
				return NULL;
			}
			if(dentry_ptr->file_ptr)
			{
				file_ptr = dentry_ptr->file_ptr;
				set_file_mode(file_ptr,FILE_MODE_WRITE | FILE_MODE_READ);
				break ;
			}
			file_ptr = _file_alloc_and_init_WRITEONLY(dentry_ptr);
			if(NULL == file_ptr)
			{
				if(NULL != error_ptr)
					*error_ptr = -ERROR_NO_MEM;
				return NULL;
			}
			break;
		default:
			ASSERT(0);
			if(NULL != error_ptr)
				*error_ptr = -ERROR_LOGIC;
			return NULL;
	}
	dentry_ptr->file_ptr = file_ptr;
	_fget(file_ptr);
	if(file_ptr->f_op->open)
	{
		return file_ptr->f_op->open(file_ptr);
	}
	if(NULL != error_ptr)
		*error_ptr = FUN_EXECUTE_SUCCESSFULLY;
	return file_ptr;
}

int open(const char *path,enum FILE_FLAG flag,struct file **file_store_ptr)
{
	if(NULL == path)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(open,path);
		return NULL;
	}
	return _open(path,flag,error_ptr);
}
EXPORT_SYMBOL(open);

int _close(struct file *file_ptr)
{
	ASSERT(NULL != file_ptr);
}

int close(struct file *file_ptr)
{
	if(NULL == file_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(close,file_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	return _close(file_ptr);
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
