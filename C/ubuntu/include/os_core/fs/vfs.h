#ifndef _VFS_H
#define _VFS_H
#include <kakaosstdint.h>
#include <double_linked_list.h>
#include <mutex.h>

//#define BLOCK_SIZE 1024 /* bytes */

struct file;
struct dentry_operations;
struct inode;
struct dentry;

enum llseek_from{
	FILE_START,
	FILE_CURRENT,
	FILE_TAIL
};

struct file_operations
{
	int (*open)(struct file *file_ptr);
	int (*close)(struct file *file_ptr);
	int (*read)(struct file *file_ptr,void *buffer,unsigned int len,unsigned int offset);
	int (*write)(struct file *file_ptr,void *buffer,unsigned int len,unsigned int offset);
	int (*llseek)(struct file *file_ptr,int offset,enum llseek_from from);
	int (*ioctl)(struct file *file_ptr,int cmd,int args);
};

/*
	realize by bottom file system
	if the file system does not support some operation,just give
	the corresponding element a empty function pointer
 */
struct inode_operations 
{
	int (*inode_open)(struct file *file_ptr);
	int (*inode_close)(struct file *file_ptr);
	int (*change_name)(struct inode *inode_ptr,struct dentry *dentry_ptr,const char *new_name);
	int (*refresh)(struct inode *inode_ptr,struct dentry *dentry_ptr);
	int (*add_sub_file)(struct dentry *dentry_ptr,const char *file_name);
	int (*add_sub_folder)(struct dentry *dentry_ptr,const char *folder_name);
	int (*read_data)(struct file *file_ptr,void *store_ptr,unsigned int len,unsigned int offset);
	int (*write_data)(struct file *file_ptr,void *data_ptr,unsigned int len,unsigned int offset);
	int (*remove)(struct dentry *dentry_ptr);
	int (*remove_dir)(struct dentry *dentry_ptr);
	int (*get_size)(struct file *file_ptr); /* fill the file_len of struct file */
};

/*************the element 'flag' of struct dentry*************/
#define FLAG_DENTRY_DEFAULT 			0X00
/* bit 0 */ 			
#define FLAG_DENTRY_NORMAL 				0X00
#define FLAG_DENTRY_NOT_RELEASED 		0X01
/* bit 1 */ 			
#define FLAG_DENTRY_FILE 				0X00
#define FLAG_DENTRY_FOLDER 				0X02
/* bit 2 */ 			
#define FLAG_NEED_REFLASH				0X04
/* bit 3 */	
#define FLAG_DENTRY_NAME_CHANGE_EN 		0X00
#define FLAG_DENTRY_NAME_CHANGE_DIS 	0X08
/* bit 4 */
#define FLAG_DENTRY_ROOT 				0X10
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
};

static inline void _dget(struct dentry *dentry_ptr)
{
	++(dentry_ptr->ref);
}

static inline void _dput(struct dentry *dentry_ptr)
{
	--(dentry_ptr->ref);
}

static inline unsigned int _dref(struct dentry *dentry_ptr)
{
	return dentry_ptr->ref;
}

static inline void set_dentry_flag(struct dentry *dentry_ptr,UINT32 flag)
{
	dentry_ptr->flag |= flag;
}

static inline UINT32 get_dentry_flag(struct dentry *dentry_ptr)
{
	return dentry_ptr->flag;
}

static inline int dentry_not_releasse(struct dentry *dentry_ptr)
{
	return (get_dentry_flag(dentry_ptr) & FLAG_DENTRY_NOT_RELEASED);
}

static inline int is_folder(struct dentry *dentry_ptr)
{
	return (get_dentry_flag(dentry_ptr) & FLAG_DENTRY_FOLDER);
}

static inline int is_file(struct dentry *dentry_ptr)
{
	return (!(get_dentry_flag(dentry_ptr)& FLAG_DENTRY_FOLDER));
}

static inline int dentry_name_not_changable(struct dentry *dentry_ptr)
{
	return (FLAG_DENTRY_NAME_CHANGE_DIS & dentry_ptr->flag);
}

static inline int dentry_need_refresh(struct dentry *dentry_ptr)
{
	return (FLAG_NEED_REFLASH & dentry_ptr->flag);
}

static inline void dentry_clear_refresh_flag(struct dentry *dentry_ptr)
{
	dentry_ptr->flag &= ~FLAG_NEED_REFLASH;
}

typedef UINT32 f_mode_t;
/*************the element 'f_mode' of struct file*************/
#define FILE_MODE_DEFAULT 0x00
#define FILE_MODE_READ    0x01
#define FILE_MODE_WRITE   0x02
#define FILE_MODE_EXE     0x04
/*************************************************************/

struct file
{
	struct file_operations *f_op;
	unsigned int offset;
	unsigned int file_len; /* bytes */
	struct dentry *f_den;
	f_mode_t f_mode;
	/*unsigned int ref;*/
	void *private_data;
};

/*
static inline void _fget(struct file *file_ptr)
{
	++(file_ptr->ref);
}
*/
/* add the ref of file,means that a task use this file,
 os should not release it. */
/*
static inline void _fput(struct file *file_ptr)
{
	--(file_ptr->ref);
}
*/
/*
static inline unsigned int get_file_ref(struct file *file_ptr)
{
	return file_ptr->ref;
}
*/
static inline void set_file_mode(struct file *file_ptr,f_mode_t mode)
{
	file_ptr->f_mode |= mode;
}

static inline void clear_file_mode(struct file *file_ptr,f_mode_t mode)
{
	file_ptr->f_mode &= ~mode;
}

/*************the element 'flag' of struct inode*************/
#define FLAG_INODE_DEFAULT 			0X00
/* bit 0 */ 			/* is it exist on disk? */
#define FLAG_INODE_HARD 			0X00
#define FLAG_INODE_SOFT				0X01
/* bit 1 */				/* represent a block or a device */
#define FLAG_INODE_DATA 			0X00
#define FLAG_INODE_DEVICE 			0X02
/* bit 2 */	
#define FLAG_INODE_NOT_READ 		0X00
#define FLAG_INODE_READ 			0X04
/* bit 3 */	
#define FLAG_INODE_NOT_WRITE 		0X00
#define FLAG_INODE_WRITE 			0X08
/* bit 4 */
#define FLAG_NOT_DIRTY				0X00
#define FLAG_DIRTY					0X10
/************************************************************/

/*
inode用来存储关于这个文件的静态信息(不变的信息)，
包括这个设备文件对应的设备号，文件的路径以及对应的驱动对象etc
 */
struct inode
{
	unsigned int ref;
	struct inode_operations *inode_ops; /*realize by bottom file system*/
	struct file_operations *i_f_ops;
	UINT32 flag;
	/* struct list_head file_list; string the data block together */
};

static inline void set_inode_flag(struct inode *inode_ptr,UINT32 flag)
{
	inode_ptr->flag |= flag;
}

static inline int inode_is_soft(struct inode *inode_ptr)
{
	return (inode_ptr->flag & FLAG_INODE_SOFT);
}

static inline int inode_is_dev(struct inode *inode_ptr)
{
	return (inode_ptr->flag & FLAG_INODE_DEVICE);
}

static inline int inode_readable(struct inode *inode_ptr)
{
	return (inode_ptr->flag & FLAG_INODE_READ);
}

static inline int inode_writable(struct inode *inode_ptr)
{
	return (inode_ptr->flag & FLAG_INODE_WRITE);
}

static inline int inode_is_dirty(struct inode *inode_ptr)
{
	return (inode_ptr->flag & FLAG_DIRTY);
}

/* add the ref of inode,means that a task use this inode,
 os should not release it. */
int _iget(struct dentry *d_parent); 
int _iput(struct dentry *d_parent); 

struct dentry *_dentry_alloc_and_init(
	struct dentry *parent_ptr,  
	struct inode *inode_ptr,
	const char *name,
	UINT32 flag
	);
#define _file_dentry_alloc_and_init(parent_ptr,inode_ptr,name,flag) \
	_dentry_alloc_and_init(parent_ptr,inode_ptr,name,FLAG_DENTRY_FILE|flag)
#define _folder_dentry_alloc_and_init(parent_ptr,inode_ptr,name,flag) \
	_dentry_alloc_and_init(parent_ptr,inode_ptr,name,FLAG_DENTRY_FOLDER|flag)

struct file *_file_alloc_and_init(
	struct dentry *dentry_ptr,UINT32 flag);
#define _file_alloc_and_init_READONLY(dentry_ptr) \
	_file_alloc_and_init(dentry_ptr,FILE_MODE_READ)
#define _file_alloc_and_init_WRITEONLY(dentry_ptr) \
	_file_alloc_and_init(dentry_ptr,FILE_MODE_WRITE)
#define _file_alloc_and_init_RDWR(dentry_ptr) \
	_file_alloc_and_init(dentry_ptr,FILE_MODE_READ | FILE_MODE_WRITE)

struct inode *_inode_alloc_and_init(
	struct inode_operations *inode_operations_ptr,
	struct file_operations *file_operations_ptr,
	UINT32 flag);

int rename(struct dentry *dentry_ptr,const char *name);

int has_same_name_file(struct dentry *dentry_ptr,const char *file_name);
struct dentry *_find_dentry(const char *path);
int ___add_file(struct dentry *target_dentry_ptr,const char *file_name,
	struct file_operations *file_operations_ptr);
int ___add_folder(struct dentry *target_dentry_ptr,const char *folder_name,
	struct file_operations *file_operations_ptr);
int add_folder(const char *path,const char *folder_name,
	struct file_operations *file_operations_ptr);
int add_file(const char *path,const char *file_name,struct file_operations *file_operations_ptr);
int delete_file(const char *path);
int delete_folder(const char *path);
int __delete_file(struct dentry *dentry_ptr);

enum FILE_FLAG{
	FILE_FLAG_READONLY = FILE_MODE_READ,
	FILE_FLAG_WRITEONLY = FILE_MODE_WRITE,
	FILE_FLAG_READ_WRITE = (FILE_MODE_READ | FILE_MODE_WRITE)
};

int ka_open(const char *path,enum FILE_FLAG flag,const struct file **file_store_ptr); /* open a file */
int ka_close(struct file *file_ptr);
int ka_read(struct file *file_ptr,void *buffer,unsigned int len,enum llseek_from offset_flag);
int ka_write(struct file *file_ptr,void *buffer,unsigned int len,enum llseek_from offset_flag);
int ka_lseek(struct file *file_ptr,int offset,enum llseek_from from);
int ka_ioctl(struct file *file_ptr,int cmd,int args);

int __open(struct dentry *dentry_ptr,enum FILE_FLAG flag,const struct file **file_store_ptr);
int _write(struct file *file_ptr,void *buffer,unsigned int len,enum llseek_from offset_flag);
int _close(struct file *file_ptr);
int _read(struct file *file_ptr,void *buffer,unsigned int len,enum llseek_from offset_flag);

void shell_pwd(int argc, char const *argv[]);
void shell_ls(int argc, char const *argv[]);
void shell_cd(int argc, char const *argv[]);
void shell_cat(int argc, char const *argv[]);
void shell_touch(int argc, char const *argv[]);
void shell_mkdir(int argc, char const *argv[]);
void shell_rm(int argc, char const *argv[]);
void shell_rmdir(int argc, char const *argv[]);
void shell_vfs_echo(char const *argv[]); /* argc == 4 argc[3] == ">" or ">>" */
void shell_rename(int argc, char const *argv[]);

void update_para_arv_vector(void);

#endif
