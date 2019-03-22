#include <vfs.h>
#include <os_error.h>
#include <printf_debug.h>
#include <myassert.h>

/**   file_operations default function   **/

int default_open(struct file *file_ptr)
{
	ASSERT(NULL != file_ptr);
	(void)file_ptr;
	KA_WARN(DEBUG_TYPE_VFS,"open file %s\n",file_ptr->f_den->name);
	return FUN_EXECUTE_SUCCESSFULLY;
}

int default_close(struct file *file_ptr)
{
	ASSERT(NULL != file_ptr);
	(void)file_ptr;
	KA_WARN(DEBUG_TYPE_VFS,"close file %s\n",file_ptr->f_den->name);
	return FUN_EXECUTE_SUCCESSFULLY;
}

int default_read(struct file *file_ptr,void *buffer,unsigned int len,unsigned int offset)
{
	ASSERT((NULL != file_ptr) && (NULL != buffer) && (len > 0) && (offset <= BLOCK_SIZE));
	if(offset + len > BLOCK_SIZE)
	{
		len = BLOCK_SIZE - offset;
	}
	struct inode *inode_ptr = file_ptr->f_den->d_inode;
	return inode_ptr->inode_ops->read_data(inode_ptr,buffer,len,offset);
}

int default_write(struct file *file_ptr,void *buffer,unsigned int len,unsigned int offset)
{
	ASSERT((NULL != file_ptr) && (NULL != buffer) && (len > 0) && (offset <= BLOCK_SIZE));
	if(offset + len > BLOCK_SIZE)
	{
		len = BLOCK_SIZE - offset;
	}
	struct inode *inode_ptr = file_ptr->f_den->d_inode;
	return inode_ptr->inode_ops->write_data(inode_ptr,buffer,len,offset);
}

int default_llseek(struct file *file_ptr,int offset,enum llseek_from from)
{
	ASSERT(NULL != file_ptr);
	switch (from)
	{
		case FILE_START:
			if(offset < 0)
			{
				return -ERROR_USELESS_INPUT;
			}
			if((unsigned int)offset + file_ptr->offset > BLOCK_SIZE)
			{
				return -ERROR_USELESS_INPUT;
			}
			file_ptr->offset = (unsigned int)offset;
			ASSERT((file_ptr->offset >=0) && (file_ptr->offset <= BLOCK_SIZE));
			return FUN_EXECUTE_SUCCESSFULLY;
		case FILE_CURRENT:
			if((file_ptr->offset + offset) > BLOCK_SIZE)
			{
				return -ERROR_USELESS_INPUT;
			}
			file_ptr->offset = file_ptr->offset + (unsigned int)offset;
			ASSERT((file_ptr->offset >=0) && (file_ptr->offset <= BLOCK_SIZE));
			return FUN_EXECUTE_SUCCESSFULLY;
		case FILE_TAIL:
			if((offset > 0) || (-offset > BLOCK_SIZE))
			{
				return -ERROR_USELESS_INPUT;
			}
			file_ptr->offset = file_ptr->offset + (unsigned int)offset;
			ASSERT((file_ptr->offset >=0) && (file_ptr->offset <= BLOCK_SIZE));
			return FUN_EXECUTE_SUCCESSFULLY;
		default :
			KA_WARN(DEBUG_TYPE_VFS,"llseek flag error\n");
			ASSERT(0);
	}
	return -ERROR_LOGIC;
}

int default_ioctl(struct file *file_ptr,int cmd,int args)
{
	ASSERT(NULL != file_ptr);
	(void)file_ptr;
	(void)cmd;
	(void)args;
	return FUN_EXECUTE_SUCCESSFULLY;
}

/**  end of file_operations default function   **/

/**  inode_operations default function  **/

int default_change_name(struct inode *inode_ptr,struct dentry *dentry_ptr)
{
	return FUN_EXECUTE_SUCCESSFULLY;
}

int default_refresh(struct inode *inode_ptr,struct dentry *dentry_ptr)
{
	return FUN_EXECUTE_SUCCESSFULLY;
}
int default_floader_cmp_file_name(struct inode *inode_ptr,const char *name)
{
	return FUN_EXECUTE_SUCCESSFULLY;
}

int default_add_sub_file(struct inode *inode_ptr,const char *name)
{
	return FUN_EXECUTE_SUCCESSFULLY;
}

int add_sub_folder(struct inode *inode_ptr,const char *folder_name)
{
	return FUN_EXECUTE_SUCCESSFULLY;
}

int default_read_data(struct inode *inode_ptr,void *store_ptr,unsigned int len,unsigned int offset)
{
	return len;
}

int default_write_data(struct inode *inode_ptr,void *data_ptr,unsigned int len,unsigned int offset)
{
	return len;
}

/**  end of inode_operations default function  **/
