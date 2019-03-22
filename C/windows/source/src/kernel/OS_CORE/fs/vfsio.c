#include <vfs.h>
#include <myassert.h>
#include <os_error.h>
#include <export.h>
#include <printf_debug.h>
#include <os_cpu.h>

int _open(const char *path,enum FILE_FLAG flag,struct file **file_store_ptr)
{
	ASSERT(NULL != path);
	struct dentry *dentry_ptr = _find_dentry(path);
	if(NULL == dentry_ptr)
	{
		return -ERROR_LOGIC;
	}
	if(get_dentry_flag(dentry_ptr) & FLAG_DENTRY_FOLDER)
	{
		KA_WARN(DEBUG_TYPE_VFS,"the path '%s' is not a file\n",path);
		return -ERROR_LOGIC;
	}
	struct file *file_ptr;
	switch(flag)
	{
		case FILE_FLAG_READONLY:
			if(!(dentry_ptr->d_inode->flag & FLAG_INODE_READ))
			{
				return -ERROR_SYS;
			}
			file_ptr = _file_alloc_and_init_READONLY(dentry_ptr);
			if(NULL == file_ptr)
			{
				return -ERROR_NO_MEM;
			}
			break;
		case FILE_FLAG_WRITEONLY:
			if(!(dentry_ptr->d_inode->flag & FLAG_INODE_WRITE))
			{
				return -ERROR_SYS;
			}
			file_ptr = _file_alloc_and_init_WRITEONLY(dentry_ptr);
			if(NULL == file_ptr)
			{
				return -ERROR_NO_MEM;
			}
			break;
		case FILE_FLAG_READ_WRITE:
			if(!(dentry_ptr->d_inode->flag & (FLAG_INODE_WRITE | FLAG_INODE_READ)))
			{
				return -ERROR_SYS;
			}
			file_ptr = _file_alloc_and_init_WRITEONLY(dentry_ptr);
			if(NULL == file_ptr)
			{
				return -ERROR_NO_MEM;
			}
			break;
		default:
			KA_WARN(DEBUG_TYPE_VFS,"function open flag error\n");
			ASSERT(0);
			return -ERROR_LOGIC;
	}
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	_dget(file_ptr->f_den);
	CPU_CRITICAL_EXIT();
	if(file_ptr->f_op->open)
	{
		return file_ptr->f_op->open(file_ptr);
	}
	*file_store_ptr = file_ptr;
	return FUN_EXECUTE_SUCCESSFULLY;
}

/* open a file */
int ka_open(const char *path,enum FILE_FLAG flag,struct file **file_store_ptr)
{
	if((NULL == path) || (NULL == file_store_ptr))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(open,path);
		OS_ERROR_PARA_MESSAGE_DISPLAY(open,file_store_ptr);
		return NULL;
	}
	return _open(path,flag,file_store_ptr);
}
EXPORT_SYMBOL(ka_open);

int _close(struct file *file_ptr)
{
	ASSERT(NULL != file_ptr);
	if(file_ptr->f_op->close)
	{
		file_ptr->f_op->close(file_ptr);
	}
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	ASSERT(_dref(file_ptr->f_den) > 0);
	_dput(file_ptr->f_den);
	ka_free(file_ptr);
	CPU_CRITICAL_EXIT();
	return FUN_EXECUTE_SUCCESSFULLY;
}

int ka_close(struct file *file_ptr)
{
	if(NULL == file_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(close,file_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	return _close(file_ptr);
}
EXPORT_SYMBOL(ka_close);

int _read(struct file *file_ptr,void *buffer,unsigned int len)
{
	ASSERT((NULL != file_ptr) && (NULL != buffer) && (len > 0));
	if(!(file_ptr->f_mode & FILE_MODE_READ))
	{
		return -ERROR_LOGIC;
	}
	if(file_ptr->f_op->read)
	{
		int offset = file_ptr->f_op->read(file_ptr,buffer,len,file_ptr->offset);
		if(offset < 0)
		{
			return offset;
		}
		file_ptr->offset += offset;
		return offset;
	}
	else
	{
		file_ptr->offset += len;  /* a bug here of file->offset */
		return len;
	}
}

int ka_read(struct file *file_ptr,void *buffer,unsigned int len)
{
	if((NULL == file_ptr) || (NULL == buffer))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	if(0 == len)
	{
		return -ERROR_USELESS_INPUT;
	}
	return _read(file_ptr,buffer,len);
}
EXPORT_SYMBOL(ka_read);

int _write(struct file *file_ptr,void *buffer,unsigned int len)
{
	ASSERT((NULL != file_ptr) && (NULL != buffer) && (len > 0));
	if(!(file_ptr->f_mode & FILE_MODE_WRITE))
	{
		return -ERROR_LOGIC;
	}
	if(file_ptr->f_op->write)
	{
		int offset = file_ptr->f_op->write(file_ptr,buffer,len,file_ptr->offset);
		if(offset < 0)
		{
			return offset;
		}
		file_ptr->offset += offset;
		return offset;
	}
	else
	{
		file_ptr->offset += len; /* a bug here of file->offset */
		return len;
	}
}

int ka_write(struct file *file_ptr,void *buffer,unsigned int len)
{
	if((NULL == file_ptr) || (NULL == buffer))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	if(0 == len)
	{
		return -ERROR_USELESS_INPUT;
	}
	return _write(file_ptr,buffer,len);
}
EXPORT_SYMBOL(ka_write);

int _llseek(struct file *file_ptr,int offset,enum llseek_from from)
{
	ASSERT(NULL != file_ptr);
	if(file_ptr->f_op->llseek)
	{
		return file_ptr->f_op->llseek(file_ptr,offset,from);
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

int ka_llseek(struct file *file_ptr,int offset,enum llseek_from from)
{
	if(NULL == file_ptr)
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _llseek(file_ptr,offset,from);
}
EXPORT_SYMBOL(ka_llseek);

int _ioctl(struct file *file_ptr,int cmd,int args)
{
	ASSERT(NULL != file_ptr);
	if(file_ptr->f_op->ioctl)
	{
		return file_ptr->f_op->ioctl(file_ptr,cmd,args);
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

int ka_ioctl(struct file *file_ptr,int cmd,int args)
{
	if(NULL == file_ptr)
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _ioctl(file_ptr,cmd,args);
}
EXPORT_SYMBOL(ka_ioctl);
