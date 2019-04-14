#ifndef _FS_H
#define _FS_H

typedef int (*fs_init_fun)(void *para);
struct inode_operations;
/* 
	the para mount_point should be given like this: 
	"/fat"
	init is the function pointer that will be called and check
	while registering the file system
*/
int fs_register(const char *mount_point,fs_init_fun init,void *para,struct inode_operations *i_opts_ptr);

#endif
