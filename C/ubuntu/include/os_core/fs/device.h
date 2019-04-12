#ifndef _DEVICE_H
#define _DEVICE_H

#include <vfs.h>

#define DEFAULT_DEVICE_PATH "/dev"

int device_register(const char *dev_name,struct file_operations *file_operations_ptr,UINT32 flag);

static inline int readonly_device_register(const char *dev_name,struct file_operations *file_operations_ptr)
{
	return device_register(dev_name,file_operations_ptr,FLAG_INODE_READ);
}

static inline int writeonly_device_register(const char *dev_name,struct file_operations *file_operations_ptr)
{
	return device_register(dev_name,file_operations_ptr,FLAG_INODE_WRITE);
}

static inline int rdwr_device_register(const char *dev_name,struct file_operations *file_operations_ptr)
{
	return device_register(dev_name,file_operations_ptr,FLAG_INODE_READ | FLAG_INODE_WRITE);
}

int device_unregister(const char *dev_name);

#endif
