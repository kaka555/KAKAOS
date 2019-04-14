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
	if(file_ptr->f_den->d_inode->inode_ops->inode_open(file_ptr) < 0)
	{
		KA_WARN(DEBUG_TYPE_VFS,"inode open file fail\n");
		return -ERROR_DISK;
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

int default_close(struct file *file_ptr)
{
	ASSERT(NULL != file_ptr);
	(void)file_ptr;
	KA_WARN(DEBUG_TYPE_VFS,"close file %s\n",file_ptr->f_den->name);
	if(file_ptr->f_den->d_inode->inode_ops->inode_close(file_ptr) < 0)
	{
		KA_WARN(DEBUG_TYPE_VFS,"inode close file fail\n");
		return -ERROR_DISK;
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

int default_read(struct file *file_ptr,void *buffer,unsigned int len,unsigned int offset)
{
	ASSERT((NULL != file_ptr) && (NULL != buffer) && (len > 0));
	ka_memset(buffer,0,len);
	struct inode *inode_ptr = file_ptr->f_den->d_inode;
	int num = inode_ptr->inode_ops->read_data(file_ptr,buffer,len,offset);
	if(num < 0)
	{
		KA_WARN(DEBUG_TYPE_VFS,"default_read fail\n");
		return num;
	}
	file_ptr->offset += num;
	return num;
}

int default_write(struct file *file_ptr,void *buffer,unsigned int len,unsigned int offset)
{
	ASSERT((NULL != file_ptr) && (NULL != buffer) && (len > 0));
	struct inode *inode_ptr = file_ptr->f_den->d_inode;
	int num = inode_ptr->inode_ops->write_data(file_ptr,buffer,len,offset);
	if(num < 0)
	{
		KA_WARN(DEBUG_TYPE_VFS,"default_write fail\n");
		return num;
	}
	file_ptr->offset += num;
	return num;
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
			file_ptr->offset = (unsigned int)offset;
			return FUN_EXECUTE_SUCCESSFULLY;
		case FILE_CURRENT:
			file_ptr->offset = file_ptr->offset + (unsigned int)offset;
			return FUN_EXECUTE_SUCCESSFULLY;
		case FILE_TAIL:
			if(offset > 0)
			{
				return -ERROR_USELESS_INPUT;
			}
			file_ptr->offset = file_ptr->offset + (unsigned int)offset;
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
/*
void default_cd(struct dentry *dentry_ptr)
{
	(void)dentry_ptr;
}
*/

int default_inode_open(struct file *file_ptr)
{
	(void)file_ptr;
	return FUN_EXECUTE_SUCCESSFULLY;
}

int default_inode_close(struct file *file_ptr)
{
	(void)file_ptr;
	return FUN_EXECUTE_SUCCESSFULLY;
}

int default_change_name(struct inode *inode_ptr,struct dentry *dentry_ptr,const char *new_name)
{
	(void)inode_ptr;
	(void)dentry_ptr;
	(void)new_name;
	return FUN_EXECUTE_SUCCESSFULLY;
}

int default_refresh(struct inode *inode_ptr,struct dentry *dentry_ptr)
{
	(void)inode_ptr;
	(void)dentry_ptr;
	return FUN_EXECUTE_SUCCESSFULLY;
}
int default_floader_cmp_file_name(struct inode *inode_ptr,const char *name)
{
	(void)inode_ptr;
	(void)name;
	return FUN_EXECUTE_SUCCESSFULLY;
}

int default_add_sub_file(struct dentry *dentry_ptr,const char *name)
{
	(void)dentry_ptr;
	(void)name;
	return FUN_EXECUTE_SUCCESSFULLY;
}

int default_add_sub_folder(struct dentry *dentry_ptr,const char *folder_name)
{
	(void)dentry_ptr;
	(void)folder_name;
	return FUN_EXECUTE_SUCCESSFULLY;
}

int default_read_data(struct file *file_ptr,void *store_ptr,unsigned int len,unsigned int offset)
{
	(void)file_ptr;
	(void)store_ptr;
	(void)len;
	(void)offset;
	return len;
}

int default_write_data(struct file *file_ptr,void *data_ptr,unsigned int len,unsigned int offset)
{
	(void)file_ptr;
	(void)data_ptr;
	(void)len;
	(void)offset;
	return len;
}

int default_remove(struct dentry *dentry_ptr)
{
	(void)dentry_ptr;
	return FUN_EXECUTE_SUCCESSFULLY;
}

int default_remove_dir(struct dentry *dentry_ptr)
{
	(void)dentry_ptr;
	return FUN_EXECUTE_SUCCESSFULLY;
}

int default_get_size(struct file *file_ptr)
{
	(void)file_ptr;
	return 0;
}

/**  end of inode_operations default function  **/
