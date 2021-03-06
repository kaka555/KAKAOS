#include <vfs.h>
#include <myassert.h>
#include <os_error.h>
#include <export.h>
#include <printf_debug.h>
#include <os_cpu.h>

/* 
user should use the function in this file to operate the file
 */

int __open(struct dentry *dentry_ptr,enum FILE_FLAG flag,const struct file **file_store_ptr)
{
	if(get_dentry_flag(dentry_ptr) & FLAG_DENTRY_FOLDER)
	{
		KA_WARN(DEBUG_TYPE_VFS,"'%s' is not a file\n",dentry_ptr->name);
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
			file_ptr = _file_alloc_and_init_RDWR(dentry_ptr);
			if(NULL == file_ptr)
			{
				return -ERROR_NO_MEM;
			}
			break;
		default:
			KA_WARN(DEBUG_TYPE_VFS,"function open flag error\n");
			ASSERT(0,ASSERT_BAD_EXE_LOCATION);
			return -ERROR_LOGIC;
	}
	ASSERT(NULL != file_ptr,ASSERT_PARA_AFFIRM);
	if(dentry_ptr->d_inode->inode_ops->get_size(file_ptr))
	{
		KA_WARN(DEBUG_TYPE_VFS,"function open get_size error\n");
		return -ERROR_DISK;
	}
	ASSERT(file_ptr->offset <= file_ptr->file_len,ASSERT_PARA_AFFIRM);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	_dget(file_ptr->f_den);
	CPU_CRITICAL_EXIT();
	if(file_ptr->f_op->open)
	{
		int error = file_ptr->f_op->open(file_ptr);
		if(error < 0)
		{
			return error;
		}
	}
	*file_store_ptr = file_ptr;
	return FUN_EXECUTE_SUCCESSFULLY;
}

int _open(const char *path,enum FILE_FLAG flag,const struct file **file_store_ptr)
{
	ASSERT(NULL != path,ASSERT_INPUT);
	struct dentry *dentry_ptr = _find_dentry(path);
	if(NULL == dentry_ptr)
	{
		return -ERROR_LOGIC;
	}
	return __open(dentry_ptr,flag,file_store_ptr);
}

/* open a file */
int ka_open(const char *path,enum FILE_FLAG flag,const struct file **file_store_ptr)
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
	ASSERT(NULL != file_ptr,ASSERT_INPUT);
	ASSERT(file_ptr->offset <= file_ptr->file_len,ASSERT_PARA_AFFIRM);
	if(file_ptr->f_op->close)
	{
		file_ptr->f_op->close(file_ptr);
	}
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	ASSERT(_dref(file_ptr->f_den) > 0,ASSERT_PARA_AFFIRM);
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

int _read(struct file *file_ptr,void *buffer,unsigned int len,enum llseek_from offset_flag)
{
	ASSERT((NULL != file_ptr) && (NULL != buffer) && (len > 0),ASSERT_INPUT);
	ASSERT(file_ptr->offset <= file_ptr->file_len,ASSERT_PARA_AFFIRM);
	if(!(file_ptr->f_mode & FILE_MODE_READ))
	{
		KA_WARN(CONFIG_VFS,"file %s cannot be read\n",file_ptr->f_den->name);
		return -ERROR_LOGIC;
	}
	if(file_ptr->f_op->read)
	{
		unsigned int offset_backup = file_ptr->offset;
		switch (offset_flag)
		{
			case FILE_START :
				file_ptr->offset = 0;
				break ;
			case FILE_TAIL :
				file_ptr->offset = file_ptr->file_len;
				break ;
			default :
				ASSERT(FILE_CURRENT == offset_flag,ASSERT_PARA_AFFIRM);
				break ;
		}
		int offset = file_ptr->f_op->read(file_ptr,buffer,len,file_ptr->offset);
		if(offset < 0)
		{
			file_ptr->offset = offset_backup;
			KA_WARN(CONFIG_VFS,"f_op->read fail\n");
			return offset;
		}
		KA_WARN(CONFIG_VFS,"read data is %s\n",(char *)buffer);
		if(inode_is_soft(file_ptr->f_den->d_inode))
		{
			file_ptr->offset += offset;
		}
		ASSERT(file_ptr->offset <= file_ptr->file_len,ASSERT_PARA_AFFIRM);
		return offset;
	}
	return 0;
}

int ka_read(struct file *file_ptr,void *buffer,unsigned int len,enum llseek_from offset_flag)
{
	if((NULL == file_ptr) || (NULL == buffer))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	if(0 == len)
	{
		return -ERROR_USELESS_INPUT;
	}
	return _read(file_ptr,buffer,len,offset_flag);
}
EXPORT_SYMBOL(ka_read);

int _write(struct file *file_ptr,void *buffer,unsigned int len,enum llseek_from offset_flag)
{
	ASSERT((NULL != file_ptr) && (NULL != buffer) && (len > 0),ASSERT_INPUT);
	ASSERT(file_ptr->offset <= file_ptr->file_len,ASSERT_PARA_AFFIRM);
	if(!(file_ptr->f_mode & FILE_MODE_WRITE))
	{
		KA_WARN(CONFIG_VFS,"file %s cannot be written\n",file_ptr->f_den->name);
		return -ERROR_LOGIC;
	}
	if(file_ptr->f_op->write)
	{
		unsigned int offset_backup = file_ptr->offset;
		switch (offset_flag)
		{
			case FILE_START :
				file_ptr->offset = 0;
				break ;
			case FILE_TAIL :
				file_ptr->offset = file_ptr->file_len;
				break ;
			default :
				ASSERT(FILE_CURRENT == offset_flag,ASSERT_PARA_AFFIRM);
				break ;
		}
		int offset = file_ptr->f_op->write(file_ptr,buffer,len,file_ptr->offset);
		if(offset < 0)
		{
			KA_WARN(CONFIG_VFS,"f_op->write fail\n");
			file_ptr->offset = offset_backup;

			return offset;
		}
		file_ptr->f_den->d_inode->inode_ops->get_size(file_ptr);
		ASSERT(file_ptr->offset <= file_ptr->file_len,ASSERT_PARA_AFFIRM);
		return offset;
	}
	return 0;
}

int ka_write(struct file *file_ptr,void *buffer,unsigned int len,enum llseek_from offset_flag)
{
	if((NULL == file_ptr) || (NULL == buffer))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	if(0 == len)
	{
		return -ERROR_USELESS_INPUT;
	}
	return _write(file_ptr,buffer,len,offset_flag);
}
EXPORT_SYMBOL(ka_write);

int _llseek(struct file *file_ptr,int offset,enum llseek_from from)
{
	ASSERT(NULL != file_ptr,ASSERT_INPUT);
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
	ASSERT(NULL != file_ptr,ASSERT_INPUT);
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
