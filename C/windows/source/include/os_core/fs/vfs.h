#ifndef _VFS_H
#define _VFS_H
#include <kakaosstdint.h>
#include <double_linked_list.h>
#include <mutex.h>

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

struct inode_operations
{
	int (*change_name)(struct inode *inode_ptr,struct dentry *dentry_ptr);
	int (*refresh)(struct inode *inode_ptr);
	int (*cmp_name)(struct inode *inode_ptr,const char *name);
};

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
	struct inode *d_inode; /* if NULL, means that this dentry not exists in hardware */
	char *name;
	UINT32 flag; /* allocated? sysfs? normal? release? */
	unsigned int ref;
	struct list_head subdirs; /* this is the list head of it's subdirs */
	struct list_head child;	/* this is the node of the subdirs's list */
	MUTEX d_mutex;
};

static inline void set_dentry_flag(struct dentry *dentry_ptr,UINT32 flag)
{
	dentry_ptr->flag |= flag;
}

typedef UINT32 f_mode_t;
/*************the element 'f_mode' of struct file*************/
#define FILE_MODE_DEFAULT (FILE_MODE_READ | FILE_MODE_WRITE)
#define FILE_MODE_READ    0x01
#define FILE_MODE_WRITE   0x02
#define FILE_MODE_EXE     0x04
/*************************************************************/

struct file
{
	struct file_operations *f_op;
	unsigned int offset;
	struct dentry *f_den;
	f_mode_t f_mode;
};

static inline void set_file_mode(struct file *file_ptr,f_mode_t mode)
{
	file_ptr->f_mode |= mode;
}

static inline void clear_file_mode(struct file *file_ptr,f_mode_t mode)
{
	file_ptr->f_mode &= ~mode;
}

struct inode
{
	unsigned long file_size; /* bytes */
	unsigned int ref;
	struct inode_operations *inode_ops; /*realize by bottom file system*/
};

/* add the ref of dentry,means that a task use this dentry,
 os should not release it. */
void dget(struct dentry *d_parent); 
int dput(struct dentry *d_parent); 

/* add the ref of inode,means that a task use this inode,
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

struct file *_file_alloc_and_init(
	struct file_operations *file_operations_ptr,
	struct dentry *dentry_ptr);

struct inode *_inode_alloc_and_init(
	unsigned long file_size);

void __init_vfs(void);

int change_name(struct file *file_ptr,const char *name);

int add_folder(const char *path,const char *folder_name);
int add_file(const char *path,struct file_operations *f_op,const char *file_name);

enum FILE_FLAG {
	FLAG_READONLY = 0,
	FLAG_WRITEONLY,
	FLAG_READ_WRITE
};
struct file *open(const char *path,enum FILE_FLAG flag);
int close(struct file *file_ptr);
int read(struct file *file_ptr,void *buffer,unsigned int len);
int write(struct file *file_ptr,void *buffer,unsigned int len);
int lseek(struct file *file_ptr,int offset);
int ioctl(struct file *file_ptr,int cmd,int args);

#endif
