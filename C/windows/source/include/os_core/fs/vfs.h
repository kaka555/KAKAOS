#ifndef _VFS_H
#define _VFS_H

#include <double_linked_list.h>

struct file;
struct dentry_operations;
struct inode;

struct file_operations
{
	int (*open)(struct file *file_ptr);
	int (*close)(struct file *file_ptr);
	int (*read)(struct file *file_ptr,void *buffer,unsigned int len);
	int (*write)(struct file *file_ptr,void *buffer,unsigned int len);
	int (*lseek)(struct file *file_ptr,int offset);
	int (*ioctl)(struct file *file_ptr,int cmd,int args);
};


/*
The word dentry is short for 'directory entry'. 
A dentry is nothing but a specific component in the path from the root. 
They (directory name or file name) provide for accessing files or directories[.]

A directory entry is used to describe the properties, size, creation time,
 modification time, and so on of a file or folder
*/
struct dentry
{
	struct dentry *d_parent;
	struct list_head subdirs;
	struct list_head child;
	const struct dentry_operations *d_op;
	struct inode *d_inode;
	const char *name;
};

struct file
{
	struct file_operations *f_op;
	unsigned int offset;
	struct dentry *f_den;
	unsigned long f_mode;
};

#endif
