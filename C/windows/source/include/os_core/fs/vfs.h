#ifndef _VFS_H
#define _VFS_H
#include <kakaosstdint.h>
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

struct dentry_operations
{
	int (*release_inode)(struct dentry *dentry_ptr);
	int (*change_name)(struct dentry *dentry_ptr);
	int (*del_dentry)(struct dentry *dentry_ptr);
	int (*cmp_name)(struct dentry *dentry_ptr,const char *name);
}；

/*************the element 'flag' of struct dentry*************/
#define FLAG_DENTRY_DEFAULT 0X00
/* bit 0 */ 
#define FLAG_DENTRY_NORMAL 	0X00
#define FLAG_DENTRY_ROOT 	0X01
/* bit 1 */ 
#define FLAG_DENTRY_FILE 	0X00
#define FLAG_DENTRY_FOLDER 	0X02
/* bit 2 */ 
#define FLAG_DENTRY_SOFE 	0X00
#define FLAG_DENTRY_HARD 	0X04
/*************************************************************/

/*
The word dentry is short for 'directory entry'. 
A dentry is nothing but a specific component in the path from the root. 
They (directory name or file name) provide for accessing files or directories[.]

A directory entry is used to describe the properties, size, creation time,
 modification time, and so on of a file or folder
*/
struct dentry
{
	struct dentry *d_parent; /* if NULL, means it is the dentry '/' */
	struct dentry_operations *d_op; /*realize by bottom file system*/
	struct inode *d_inode; /* if NULL, means that this dentry not exists in hardware */
	char *name;
	UINT32 flag; /* allocated? sysfs? normal? release? */
	unsigned int ref;
	struct list_head subdirs; /* this is the list head of it's subdirs */
	struct list_head child;	/* this is the node of the subdirs's list */
};

static inline void set_dentry_flag(struct dentry *dentry_ptr,UINT32 flag)
{
	dentry_ptr->flag |= flag;
}

typedef UINT32 f_mode_t;

struct file
{
	struct file_operations *f_op;
	unsigned int offset;
	struct dentry *f_den;
	f_mode_t f_mode;
};

struct inode
{
	unsigned long file_size;
	unsigned int ref;
};

/* add the ref of dentry,means that a task use this dentry,
 os should not release it. */
int dget(struct dentry *d_parent); 

int dput(struct dentry *d_parent); 

enum DENTRY_INIT_FLAG {
	FLAG_FILE = 0,
	FLAG_FOLDER
};

struct dentry *_dentry_alloc_and_init(
	struct dentry *parent_ptr,  
	struct dentry_operations *dentry_operations_ptr,
	struct inode *inode_ptr,
	const char *name,
	enum DENTRY_INIT_FLAG flag
	);

#define _file_dentry_alloc_and_init(parent_ptr,inode_ptr,name) \
	_dentry_alloc_and_init(parent_ptr,inode_ptr,name,FLAG_FILE)

#define _folder_dentry_alloc_and_init(parent_ptr,inode_ptr,name) \
	_dentry_alloc_and_init(parent_ptr,inode_ptr,name,FLAG_FOLDER)

void __init_vfs(void);

int add_folder(const char *path,const char *folder_name);
int add_file(const char *path,struct file_operations *f_op,const char *file_name);

#endif