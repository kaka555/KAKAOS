#include <vfs.h>
#include <myMicroLIB.h>
#include <myassert.h>
#include <os_error.h>
#include <export.h>
#include <os_cpu.h>
#include <sys_init_fun.h>
#include <printf_debug.h>
#include <vector.h>
#include <command_processor.h>
#include <shell.h>
#include <fs.h>

extern int default_open(struct file *file_ptr);
extern int default_close(struct file *file_ptr);
extern int default_read(struct file *file_ptr,void *buffer,unsigned int len,unsigned int offset);
extern int default_write(struct file *file_ptr,void *buffer,unsigned int len,unsigned int offset);
extern int default_llseek(struct file *file_ptr,int offset,enum llseek_from from);
extern int default_ioctl(struct file *file_ptr,int cmd,int args);

static struct file_operations default_file_operations = {
	default_open,
	default_close,
	default_read,
	default_write,
	default_llseek,
	default_ioctl
};
#define defalut_file_operations_ptr (&default_file_operations)

extern int default_inode_open(struct file *file_ptr);
extern int default_inode_close(struct file *file_ptr);
extern int default_change_name(struct inode *inode_ptr,struct dentry *dentry_ptr,const char *new_name);
extern int default_refresh(struct inode *inode_ptr,struct dentry *dentry_ptr);
extern int default_add_sub_file(struct dentry *dentry_ptr,const char *name);
extern int default_add_sub_folder(struct dentry *dentry_ptr,const char *folder_name);
extern int default_read_data(struct file *file_ptr,void *store_ptr,unsigned int len,unsigned int offset);
extern int default_write_data(struct file *file_ptr,void *data_ptr,unsigned int len,unsigned int offset);
extern int default_remove(struct dentry *dentry_ptr);
extern int default_remove_dir(struct dentry *dentry_ptr);
extern int default_get_size(struct file *file_ptr);
static struct inode_operations default_inode_operations = {
	default_inode_open,
	default_inode_close,
	default_change_name,
	default_refresh,
	default_add_sub_file,
	default_add_sub_folder,
	default_read_data,
	default_write_data,
	default_remove,
	default_remove_dir,
	default_get_size
};
#define defalut_inode_operations_ptr (&default_inode_operations)

/* the statement of basic dentry & inode */
static struct dentry root_dentry;
static struct dentry dev_dentry;
static struct dentry *current_dentry_ptr = NULL;
static struct inode root_inode;
static struct inode dev_inode;

static void _init_dentry(
	struct dentry *dentry_ptr,
	struct dentry *parent_ptr,  
	struct inode *inode_ptr,
	const char *name,
	UINT32 flag
	)
{
	ASSERT(NULL != dentry_ptr,ASSERT_INPUT);
	dentry_ptr->d_parent = parent_ptr;
	dentry_ptr->d_inode = inode_ptr;
	dentry_ptr->name = (char *)name;
	dentry_ptr->ref = 0;
	INIT_LIST_HEAD(&dentry_ptr->subdirs);
	INIT_LIST_HEAD(&dentry_ptr->child);
	dentry_ptr->flag = FLAG_DENTRY_DEFAULT | flag;
	if(dentry_ptr == parent_ptr) /* root */
	{
		set_dentry_flag(dentry_ptr,FLAG_DENTRY_ROOT);
	}
	else
	{
		list_add_tail(&dentry_ptr->child,&parent_ptr->subdirs);
	}
}

/**
 * @Author      kaka
 * @DateTime    2019-04-21
 * @description : allocate a struct dentry and init it
 * @param       parent_ptr parent dentry pointer
 * @param       inode_ptr  
 * @param       name       
 * @param       flag       
 * @return                 
 */
struct dentry *_dentry_alloc_and_init(
	struct dentry *parent_ptr,  
	struct inode *inode_ptr,
	const char *name,
	UINT32 flag
	)
{
	ASSERT(NULL != parent_ptr,ASSERT_INPUT);
	ASSERT(NULL != name,ASSERT_INPUT);
	ASSERT(NULL != inode_ptr,ASSERT_INPUT);
	struct dentry *dentry_ptr = ka_malloc(sizeof(struct dentry));
	if(NULL == dentry_ptr)
	{
		return NULL;
	}
	flag &= ~FLAG_DENTRY_NAME_CHANGE_DIS; /* new file should be name-changable */
	_init_dentry(dentry_ptr,parent_ptr,inode_ptr,name,flag);
	return dentry_ptr;
}

/**
 * @Author      kaka
 * @DateTime    2019-04-21
 * @description : allocate a struct file and init it
 * @param       dentry_ptr corresponding dentry
 * @param       flag       
 * @return                 
 */
struct file *_file_alloc_and_init(
	struct dentry *dentry_ptr,UINT32 flag)
{
	ASSERT(NULL != dentry_ptr,ASSERT_INPUT);
	struct file *file_ptr = ka_malloc(sizeof(struct file));
	if(NULL == file_ptr)
	{
		return NULL;
	}
	ASSERT(NULL != dentry_ptr->d_inode->i_f_ops,ASSERT_PARA_AFFIRM);
	file_ptr->f_op = dentry_ptr->d_inode->i_f_ops;
	file_ptr->offset = 0;
	file_ptr->file_len = 0;
	file_ptr->f_den = dentry_ptr;
	file_ptr->f_mode = FILE_MODE_DEFAULT | flag;
	/*file_ptr->ref = 0;*/
	return file_ptr;
}

static void _inode_init(
	struct inode_operations *inode_operations_ptr,
	struct file_operations *file_operations_ptr,
	UINT32 flag,
	struct inode *inode_ptr)
{
	ASSERT(NULL != inode_ptr,ASSERT_INPUT);
	inode_ptr->ref = 0;
	inode_ptr->inode_ops = inode_operations_ptr;
	inode_ptr->i_f_ops = file_operations_ptr;
	inode_ptr->flag = FLAG_INODE_DEFAULT | flag;
	if(NULL == inode_operations_ptr)
	{
		set_inode_flag(inode_ptr,FLAG_INODE_SOFT);
		inode_ptr->inode_ops = defalut_inode_operations_ptr;
	}
	if(NULL == file_operations_ptr)
	{
		inode_ptr->i_f_ops = defalut_file_operations_ptr;
	}
}

/**
 * @Author      kaka
 * @DateTime    2019-04-21
 * @description : allocate a struct inode and init it
 * @param       inode_operations_ptr [description]
 * @param       file_operations_ptr  [description]
 * @param       flag                 [description]
 * @return                           [description]
 */
struct inode *_inode_alloc_and_init(
	struct inode_operations *inode_operations_ptr,
	struct file_operations *file_operations_ptr,
	UINT32 flag)
{
	struct inode *inode_ptr = ka_malloc(sizeof(struct inode));
	if(NULL == inode_ptr)
	{
		return NULL;
	}
	_inode_init(inode_operations_ptr,file_operations_ptr,flag,inode_ptr);
	return inode_ptr;
}

int _rename(struct dentry *dentry_ptr,const char *name)
{
	ASSERT((NULL != dentry_ptr) && (NULL != name),ASSERT_INPUT);
	int ret = FUN_EXECUTE_SUCCESSFULLY;
	const char *pre_name = dentry_ptr->name;
	if(ret < 0)
	{
		return ret;
	}
	if(dentry_ptr->flag & FLAG_DENTRY_NAME_CHANGE_DIS)
	{
		ret = -ERROR_SYS;
		goto out;
	}
	if(dentry_ptr->d_inode->inode_ops->change_name(dentry_ptr->d_inode,dentry_ptr,name) < 0)
	{
		ka_printf("write disk error\n");
		ret = -ERROR_DISK;
		goto out;
	}
	char *buffer = (char *)ka_malloc(ka_strlen(name));
	if(NULL == buffer)
	{
		ret = -ERROR_NO_MEM;
		dentry_ptr->d_inode->inode_ops->change_name(dentry_ptr->d_inode,dentry_ptr,pre_name);
		goto out;
	}
	ka_strcpy(buffer,name);
	ka_free(dentry_ptr->name);
	dentry_ptr->name = buffer;
out:
	return ret;
}

/**
 * @Author      kaka
 * @DateTime    2019-04-21
 * @description : rename a dentry
 * @param       dentry_ptr [description]
 * @param       name       [description]
 * @return                 [description]
 */
int rename(struct dentry *dentry_ptr,const char *name)
{
	if((NULL == dentry_ptr) || (NULL == name))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _rename(dentry_ptr,name);
}
EXPORT_SYMBOL(rename);

/**
 * @Author      kaka
 * @DateTime    2019-04-21
 * @description : check if there is a same-name-dentry under the parent_dentry
 * @param       parent_dentry_ptr [description]
 * @param       file_name         [description]
 * @return      1 means there is a same-name-dentry, 0 means no
 */
int has_same_name_file(struct dentry *parent_dentry_ptr,const char *file_name)
{
	struct dentry *buffer_ptr;
	struct list_head *head = &parent_dentry_ptr->subdirs;
	list_for_each_entry(buffer_ptr,head,child)
	{
		if(0 == ka_strcmp(file_name,buffer_ptr->name))
		{
			return 1;
		}
	}
	return 0;
}

/**
 * @Author      kaka
 * @DateTime    2019-04-21
 * @description : get the dentry pointer with with name "path_name"
 * @param       parent          [description]
 * @param       path_name       [description]
 * @param       subdir_name_len [description]
 * @return                      [description]
 */
static struct dentry *_get_subdir(struct dentry *parent,const char *path_name,unsigned int subdir_name_len)
{
	ASSERT((NULL != parent) && (NULL != path_name),ASSERT_INPUT);
	if('\0' == *path_name)
	{
		return parent;
	}
	struct list_head *head = &parent->subdirs;
	struct dentry *subdir;
	list_for_each_entry(subdir,head,child)
	{
		if(ka_strlen(subdir->name) != subdir_name_len)
		{
			continue ;
		}
		if(0 == ka_strcmp(subdir->name,path_name))
		{
			return subdir;
		}
	}
	return NULL;
}

/*
	return 0 means that it is the last dentry of the path  
 */
static unsigned int _get_subdir_name_len(const char *path_name)
{
	ASSERT(NULL != path_name,ASSERT_INPUT);
	unsigned int i = 0;
	while('/' != *path_name)
	{
		++i;
		if('\0' == *path_name)
		{
			return 0;
		}
		++path_name;
	}
	return i;
}

/**
 * @Author      kaka
 * @DateTime    2019-04-21
 * @description : get the dentry pointer of path
 * @param       path       the full path of file
 * @return      the dentry pointer 
 */
struct dentry *_find_dentry(const char *path)
{
	ASSERT(NULL != path,ASSERT_INPUT);
	struct dentry *dentry_ptr;
	char *cur_path;
	if('/' == *path)
	{
		dentry_ptr = &root_dentry;
		cur_path = (char *)(path+1);
	}
	else
	{
		dentry_ptr = current_dentry_ptr;
		cur_path = (char *)(path);
	}
	unsigned int name_len = _get_subdir_name_len(cur_path);
	while(0 != name_len) /* not the last dentry */
	{
		dentry_ptr = _get_subdir(dentry_ptr,cur_path,name_len);
		if(NULL == dentry_ptr) /* no such dentry */
		{
			return NULL;
		}
		cur_path += name_len + 1;
		name_len = _get_subdir_name_len(cur_path);
	}
	return _get_subdir(dentry_ptr,cur_path,ka_strlen(cur_path));
}

int ___add_folder(struct dentry *target_dentry_ptr,const char *folder_name,
	struct file_operations *file_operations_ptr)
{
	ASSERT(is_folder(target_dentry_ptr),"make sure is folder\n");
	unsigned int len = ka_strlen(folder_name)+1;
	char *name_buffer = ka_malloc(len);
	if(NULL == name_buffer)
	{
		KA_WARN(DEBUG_TYPE_VFS,"name_buffer malloc fail\n");
		goto out3;
	}
	ka_strcpy(name_buffer,folder_name);
	name_buffer[len-1] = '\0';
	struct inode *inode_ptr;
	if(file_operations_ptr)
	{
		inode_ptr = _inode_alloc_and_init(target_dentry_ptr->d_inode->inode_ops,
			file_operations_ptr,target_dentry_ptr->d_inode->flag);
	}
	else
	{
		inode_ptr = _inode_alloc_and_init(target_dentry_ptr->d_inode->inode_ops,
			target_dentry_ptr->d_inode->i_f_ops,target_dentry_ptr->d_inode->flag);
	}
	if(NULL == inode_ptr)
	{
		KA_WARN(DEBUG_TYPE_VFS,"inode malloc fail\n");
		goto out2;
	}
	struct dentry *buffer = _folder_dentry_alloc_and_init(target_dentry_ptr,inode_ptr,name_buffer,FLAG_DENTRY_DEFAULT|FLAG_NEED_REFLASH);
	if(NULL == buffer)
	{
		KA_WARN(DEBUG_TYPE_VFS,"dentry malloc fail\n");
		goto out1;
	}
	return FUN_EXECUTE_SUCCESSFULLY;
out1:
	ka_free(inode_ptr);
out2:
	ka_free(name_buffer);
out3:
	return -ERROR_NO_MEM;
}

int __add_folder(struct dentry *target_dentry_ptr,const char *folder_name,
	struct file_operations *file_operations_ptr)
{
	ASSERT(is_folder(target_dentry_ptr),"make sure is folder\n");
	if(has_same_name_file(target_dentry_ptr,folder_name))
	{
		return -ERROR_LOGIC;
	}
	if(target_dentry_ptr->d_inode->inode_ops->add_sub_folder(target_dentry_ptr,folder_name) < 0)
	{
		return -ERROR_DISK;
	}
	return ___add_folder(target_dentry_ptr,folder_name,file_operations_ptr);
}

int _add_folder(const char *path,const char *folder_name,
	struct file_operations *file_operations_ptr)
{
	ASSERT((NULL != path) && (NULL != folder_name),ASSERT_INPUT);
	/* find the corresponding dentry */
	struct dentry *target_dentry_ptr = _find_dentry(path);
	if(NULL == target_dentry_ptr)
	{
		return -ERROR_LOGIC;
	}
	return __add_folder(target_dentry_ptr,folder_name,file_operations_ptr);
}

int add_folder(const char *path,const char *folder_name,
	struct file_operations *file_operations_ptr)
{
	if((NULL == path) || (NULL == folder_name))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _add_folder(path,folder_name,file_operations_ptr);
}
EXPORT_SYMBOL(add_folder);

int ___add_file(struct dentry *target_dentry_ptr,const char *file_name,struct file_operations *file_operations_ptr)
{
	ASSERT(is_folder(target_dentry_ptr),"make sure is folder\n");
	unsigned int len = ka_strlen(file_name)+1;
	char *name_buffer = ka_malloc(len);
	if(NULL == name_buffer)
	{
		goto out3;
	}
	ka_strcpy(name_buffer,file_name);
	name_buffer[len-1] = '\0';
	struct inode *inode_ptr;
	if(file_operations_ptr)
	{
		inode_ptr = _inode_alloc_and_init(target_dentry_ptr->d_inode->inode_ops,file_operations_ptr,target_dentry_ptr->d_inode->flag);
	}
	else
	{
		inode_ptr = _inode_alloc_and_init(target_dentry_ptr->d_inode->inode_ops,target_dentry_ptr->d_inode->i_f_ops,target_dentry_ptr->d_inode->flag);
	}
	if(NULL == inode_ptr)
	{
		goto out2;
	}
	struct dentry *buffer = _file_dentry_alloc_and_init(target_dentry_ptr,inode_ptr,name_buffer,FLAG_DENTRY_DEFAULT);
	if(NULL == buffer)
	{
		goto out1;
	}
	return FUN_EXECUTE_SUCCESSFULLY;
out1:
	ka_free(inode_ptr);
out2:
	ka_free(name_buffer);
out3:
	return -ERROR_NO_MEM;
}

static int __add_file(struct dentry *target_dentry_ptr,const char *file_name,struct file_operations *file_operations_ptr)
{
	ASSERT(is_folder(target_dentry_ptr),"make sure is folder\n");
	KA_WARN(DEBUG_TYPE_VFS,"__add_file() get dentry name is %s\nfile name is %s\n",target_dentry_ptr->name,file_name);
	if(has_same_name_file(target_dentry_ptr,file_name))
	{
		KA_WARN(DEBUG_TYPE_VFS,"same name file already exist\n");
		return -ERROR_LOGIC;
	}
	if(target_dentry_ptr->d_inode->inode_ops->add_sub_file(target_dentry_ptr,file_name) < 0)
	{
		KA_WARN(DEBUG_TYPE_VFS,"disk add fail\n");
		return -ERROR_DISK;
	}
	return ___add_file(target_dentry_ptr,file_name,file_operations_ptr);
}

int _add_file(const char *path,const char *file_name,struct file_operations *file_operations_ptr)
{
	ASSERT((NULL != path) && (NULL != file_name),ASSERT_INPUT);
	/* find the corresponding dentry */
	struct dentry *target_dentry_ptr = _find_dentry(path);
	if(NULL == target_dentry_ptr)
	{
		return -ERROR_LOGIC;
	}
	return __add_file(target_dentry_ptr,file_name,file_operations_ptr);
}

int add_file(const char *path,const char *file_name,struct file_operations *file_operations_ptr)
{
	if((NULL == path) || (NULL == file_name))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _add_file(path,file_name,file_operations_ptr);
}
EXPORT_SYMBOL(add_file);

int __delete_file(struct dentry *dentry_ptr)
{
	ASSERT(NULL != dentry_ptr,ASSERT_INPUT);
	ASSERT(list_empty(&dentry_ptr->subdirs),ASSERT_PARA_AFFIRM);
	ASSERT(is_file(dentry_ptr),ASSERT_PARA_AFFIRM);
	if(0 == _dref(dentry_ptr))
	{
		dentry_ptr->d_inode->inode_ops->remove(dentry_ptr);
		list_del(&dentry_ptr->child);
		ka_free(dentry_ptr);
		KA_WARN(DEBUG_TYPE_VFS,"remove file %s\n",dentry_ptr->name);
		return FUN_EXECUTE_SUCCESSFULLY;
	}
	else
	{
		ka_printf("file is currently used by task\n");
		return -ERROR_SYS;
	}
}

int _delete_file(const char *path)
{
	ASSERT(NULL != path,ASSERT_INPUT);
	struct dentry *target_dentry_ptr = _find_dentry(path);
	if((NULL == target_dentry_ptr) || (!is_file(target_dentry_ptr)))
	{
		ka_printf("file path error\n");
		return -ERROR_LOGIC;
	}
	if(dentry_not_releasse(target_dentry_ptr))
	{
		ka_printf("remove refused\n");
		return -ERROR_SYS;
	}
	return __delete_file(target_dentry_ptr);
}

int delete_file(const char *path)
{
	if(NULL == path)
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _delete_file(path);
}
EXPORT_SYMBOL(delete_file);

/*
 remove all the folder and file under the root_dentry_ptr,
 and then remove the root_dentry_ptr
 */
static int remove_all_dentry(struct dentry *root_dentry_ptr)
{
	ASSERT(NULL != root_dentry_ptr,ASSERT_INPUT);
	struct dentry *dentry_buffer_ptr,*n;
	if(dentry_need_refresh(root_dentry_ptr))
	{
		if(root_dentry_ptr->d_inode->inode_ops->refresh(root_dentry_ptr->d_inode,root_dentry_ptr) < 0)
		{
			ka_printf("disk refresh error\n");
			return -ERROR_DISK;
		}
		KA_WARN(DEBUG_TYPE_VFS,"dentry refresh\n");
		dentry_clear_refresh_flag(root_dentry_ptr);
	}
	list_for_each_entry_safe(dentry_buffer_ptr,n,&root_dentry_ptr->subdirs,child)
	{
		if(is_folder(dentry_buffer_ptr))
		{
			KA_WARN(DEBUG_TYPE_VFS,"remove floder %s\n",dentry_buffer_ptr->name);
			if(remove_all_dentry(dentry_buffer_ptr) < 0)
			{
				KA_WARN(DEBUG_TYPE_VFS,"remove_all_dentry error,current dentry is %s\n"
					,dentry_buffer_ptr->name);
				return -ERROR_SYS;
			}
		}
		else
		{
			ASSERT(is_file(dentry_buffer_ptr),ASSERT_PARA_AFFIRM);
			if(__delete_file(dentry_buffer_ptr) < 0)
			{
				KA_WARN(DEBUG_TYPE_VFS,"__delete_file error,current dentry is %s\n"
					,dentry_buffer_ptr->name);
				return -ERROR_SYS;
			}
		}
	}
	root_dentry_ptr->d_inode->inode_ops->remove_dir(root_dentry_ptr);
	list_del(&root_dentry_ptr->child);
	ka_free(root_dentry_ptr);
	return FUN_EXECUTE_SUCCESSFULLY;
}

int _delete_folder(const char *path)
{
	ASSERT(NULL != path,ASSERT_INPUT);
	struct dentry *target_dentry_ptr = _find_dentry(path);
	if((NULL == target_dentry_ptr) || (!is_folder(target_dentry_ptr)))
	{
		ka_printf("folder path error\n");
		return -ERROR_LOGIC;
	}
	if(dentry_not_releasse(target_dentry_ptr))
	{
		ka_printf("remove refused\n");
		return -ERROR_SYS;
	}
	return remove_all_dentry(target_dentry_ptr);
}

int delete_folder(const char *path)
{
	if(NULL == path)
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _delete_folder(path);
}
EXPORT_SYMBOL(delete_folder);

#if CONFIG_SHELL_EN

static Vector para_arv_vector;
static struct command *command_cd_ptr = NULL;
static struct command *command_rm_ptr = NULL;
static struct command *command_rmdir_ptr = NULL;
static struct command *command_ls_ptr = NULL;
static struct command *command_cat_ptr = NULL;
static struct command *command_echo_ptr = NULL;
static struct command *command_rename_ptr = NULL;

static void pwd(struct dentry *dentry_ptr)
{
	ASSERT(is_folder(dentry_ptr),ASSERT_PARA_AFFIRM);
	if(dentry_ptr == (dentry_ptr->d_parent)) /* root */
	{
		ka_printf("/");
		return ;
	}
	pwd(dentry_ptr->d_parent);
	ka_printf("%s/",dentry_ptr->name);
	return ;
}

/**
 * @Author      kaka
 * @DateTime    2019-04-21
 * @description : show the current path, this function should only be 
 * called by thread "shell"
 * @param       argc       insignificant
 * @param       argv       insignificant
 */
void shell_pwd(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	struct dentry *dentry_ptr = current_dentry_ptr;
	ASSERT(NULL != dentry_ptr,ASSERT_PARA_AFFIRM);
	if(dentry_ptr == (dentry_ptr->d_parent)) /* root */
	{
		ka_printf("/\n");
		return ;
	}
	pwd(dentry_ptr->d_parent);
	ka_printf("%s\n",dentry_ptr->name);
	return ;
}

/* if the input >0, return 1, else, return 0 */
static inline unsigned int shell_ls_helper(unsigned int para)
{
	return (para>0)?1:0;
}

/*
	ls
	ls filename
output:
	filename ref not_release is_folder need_refresh name_not_changable is_soft is_dev \
	read wirte dirty
 */
void shell_ls(int argc, char const *argv[])
{
	ASSERT(NULL != current_dentry_ptr,ASSERT_PARA_AFFIRM);
	struct dentry *dentry_ptr;
	struct list_head *head = &current_dentry_ptr->subdirs;
	if(1 == argc)
	{
		list_for_each_entry(dentry_ptr,head,child)
		{
			ka_printf("%s\t",dentry_ptr->name);
		}
	}
	else if(2 == argc)
	{
		list_for_each_entry(dentry_ptr,head,child)
		{
			if(0 == ka_strcmp(argv[1],dentry_ptr->name))
			{
				ka_printf("%s\t%u|%u|%u|%u|%u|%u|%u\t%u|%u\t%u",dentry_ptr->name,dentry_ptr->ref,
					shell_ls_helper(dentry_not_releasse(dentry_ptr)),
					shell_ls_helper(is_folder(dentry_ptr)),
					shell_ls_helper(dentry_need_refresh(dentry_ptr)),
					shell_ls_helper(dentry_name_not_changable(dentry_ptr)),
					shell_ls_helper(inode_is_soft(dentry_ptr->d_inode)),
					shell_ls_helper(inode_is_dev(dentry_ptr->d_inode)),
					shell_ls_helper(inode_readable(dentry_ptr->d_inode)),
					shell_ls_helper(inode_writable(dentry_ptr->d_inode)),
					shell_ls_helper(inode_is_dirty(dentry_ptr->d_inode))
					);
				break ;
			}
		}
	}
	else
	{
		ka_printf("parameter error");
	}
	ka_printf("\n");
}

void shell_cd(int argc, char const *argv[])
{
	if(2 != argc)
	{
		ka_printf("command error\n");
		return ;
	}
	if(0 == ka_strcmp("..",argv[1]))
	{
		current_dentry_ptr = current_dentry_ptr->d_parent;
		update_para_arv_vector();
		return ;
	}
	else if(0 == ka_strcmp(".",argv[1]))
	{
		return ;
	}
	struct dentry *dentry_ptr = _find_dentry(argv[1]);
	if((NULL == dentry_ptr) || (!is_folder(dentry_ptr)))
	{
		ka_printf("path error\n");
		return ;
	}
	current_dentry_ptr = dentry_ptr;
	if(dentry_need_refresh(dentry_ptr))
	{
		if(dentry_ptr->d_inode->inode_ops->refresh(dentry_ptr->d_inode,dentry_ptr) < 0)
		{
			ka_printf("disk refresh error\n");
			return ;
		}
		KA_WARN(DEBUG_TYPE_VFS,"dentry refresh\n");
		dentry_clear_refresh_flag(dentry_ptr);
	}
	update_para_arv_vector();
	return ;
}

/*
	touch filename;
	create a file
 */
void shell_touch(int argc, char const *argv[])
{
	if(2 != argc)
	{
		ka_printf("command error\n");
		return ;
	}
	int ret = __add_file(current_dentry_ptr,argv[1],NULL);
	if((ret < 0) && (-ERROR_LOGIC != ret))
	{
		ka_printf("create file fail\n");
		return ;
	}
	update_para_arv_vector();
}

/*
	cat filename
 */
void shell_cat(int argc, char const *argv[])
{
	if(2 != argc)
	{
		ka_printf("command error\n");
		return ;
	}
	struct file *file_ptr = NULL;
	struct dentry *dentry_ptr = _find_dentry(argv[1]);
	if(NULL == dentry_ptr)
	{
		ka_printf("path error\n");
		return ;
	}
	int error = __open(dentry_ptr,FILE_FLAG_READONLY,(const struct file **)&file_ptr);
	if(error < 0)
	{
		ka_printf("open file fail\n");
		return ;
	}
	ASSERT(NULL != file_ptr,ASSERT_PARA_AFFIRM);
	if(0 == file_ptr->file_len)
	{
		_close(file_ptr);
		return ;
	}
	void *buffer = ka_malloc(file_ptr->file_len);
	if(NULL == buffer)
	{
		ka_printf("allocate room for display error\n");
		return ;
	}
	error = _read(file_ptr,buffer,file_ptr->file_len,FILE_CURRENT);
	ka_printf("%s\n",(char *)buffer);
	if(error < 0)
	{
		ka_printf("read fail\n");
	}
	ka_free(buffer);
	_close(file_ptr);
}

void shell_rename(int argc, char const *argv[])
{
	if(3 != argc)
	{
		ka_printf("command error\n");
		return ;
	}
	struct dentry *dentry_ptr = _get_subdir(current_dentry_ptr,argv[1],ka_strlen(argv[2]));
	if(NULL == dentry_ptr)
	{
		ka_printf("file name error\n");
		return ;
	}
	if(_rename(dentry_ptr,argv[2]) < 0)
	{
		ka_printf("rename fail\n");
	}
}

/*
	mkdir foldername
 */
void shell_mkdir(int argc, char const *argv[])
{
	if(2 != argc)
	{
		ka_printf("command error\n");
		return ;
	}
	int ret = __add_folder(current_dentry_ptr,argv[1],NULL);
	if((ret < 0) && (-ERROR_LOGIC != ret))
	{
		ka_printf("create folder fail\n");
		return ;
	}
	update_para_arv_vector();
}

void shell_rm(int argc, char const *argv[])
{
	if(2 != argc)
	{
		ka_printf("command error\n");
		return ;
	}
	delete_file(argv[1]);
	return ;
}

void shell_rmdir(int argc, char const *argv[])
{
	if(2 != argc)
	{
		ka_printf("command error\n");
		return ;
	}
	delete_folder(argv[1]);
	return ;
}

/**
 * @Author      kaka
 * @DateTime    2019-04-13
 * @description : this function update the file and folder name into a 
 * vector so we can use command like "cd" "rm" ... with TAB easily
 */
void update_para_arv_vector(void)
{
	ASSERT(NULL != current_dentry_ptr,ASSERT_PARA_AFFIRM);
	Vector_clean_up(&para_arv_vector);
	struct dentry *dentry_ptr;
	list_for_each_entry(dentry_ptr,&current_dentry_ptr->subdirs,child)
	{
		Vector_push_back(&para_arv_vector,dentry_ptr->name);
	}
	_set_command_para_list(command_cd_ptr,(char **)para_arv_vector.data_ptr,para_arv_vector.cur_len);
	_set_command_para_list(command_rm_ptr,(char **)para_arv_vector.data_ptr,para_arv_vector.cur_len);
	_set_command_para_list(command_rmdir_ptr,(char **)para_arv_vector.data_ptr,para_arv_vector.cur_len);
	_set_command_para_list(command_ls_ptr,(char **)para_arv_vector.data_ptr,para_arv_vector.cur_len);
	_set_command_para_list(command_cat_ptr,(char **)para_arv_vector.data_ptr,para_arv_vector.cur_len);
	_set_command_para_list(command_echo_ptr,(char **)para_arv_vector.data_ptr,para_arv_vector.cur_len);
	_set_command_para_list(command_rename_ptr,(char **)para_arv_vector.data_ptr,para_arv_vector.cur_len);
}

void shell_vfs_echo(char const *argv[])
{
	int error;
	struct file *file_ptr = NULL;
	if((0 != ka_strcmp(">",argv[2])) && (0 != ka_strcmp(">>",argv[2])))
	{
		ka_printf("command error\n");
		return ;
	}
	if(0 == ka_strlen(argv[1]))
	{
		ka_printf("parameter error\n");
		return ;
	}
	struct dentry *dentry_ptr = _find_dentry(argv[3]);
	if(NULL == dentry_ptr)
	{
		ka_printf("path error\n");
		return ;
	}
	error = __open(dentry_ptr,FILE_FLAG_WRITEONLY,(const struct file **)&file_ptr);
	if(error < 0)
	{
		ka_printf("open file fail\n");
		return ;
	}
	ASSERT(NULL != file_ptr,ASSERT_PARA_AFFIRM);
	if(0 == ka_strcmp(">",argv[2]))
	{
		error = _write(file_ptr,(void *)argv[1],ka_strlen(argv[1]),FILE_CURRENT);
	}
	else
	{
		ASSERT(0 == ka_strcmp(">>",argv[2]),ASSERT_PARA_AFFIRM);
		error = _write(file_ptr,(void *)argv[1],ka_strlen(argv[1]),FILE_TAIL);
	}
	if(error < 0)
	{
		ka_printf("write fail\n");
	}
	_close(file_ptr);
}

#endif

int _fs_register(const char *mount_point,fs_init_fun init,void *para,struct inode_operations *i_opts_ptr)
{
	ASSERT(NULL != mount_point,ASSERT_INPUT);
	ASSERT(NULL != init,ASSERT_INPUT);
	struct dentry *dentry_ptr = &root_dentry;
	const char *cur_path = mount_point + 1;
	unsigned int name_len = _get_subdir_name_len(cur_path);
	while(0 != name_len) /* not the last dentry */
	{
		dentry_ptr = _get_subdir(dentry_ptr,cur_path,name_len);
		if(NULL == dentry_ptr) /* no such dentry */
		{
			KA_WARN(DEBUG_TYPE_VFS,"_fs_register mount_point error\n");
			return -ERROR_LOGIC;
		}
		cur_path += name_len + 1;
		name_len = _get_subdir_name_len(cur_path);
	}
	if(NULL != _get_subdir(dentry_ptr,cur_path,ka_strlen(cur_path)))
	{
		KA_WARN(DEBUG_TYPE_VFS,"mount_point exists\n");
		return -ERROR_LOGIC;
	}
	unsigned int len = ka_strlen(cur_path)+1;
	char *name_buffer = ka_malloc(len);
	if(NULL == name_buffer)
	{
		KA_WARN(DEBUG_TYPE_VFS,"name_buffer malloc fail\n");
		return -ERROR_NO_MEM;
	}
	ka_strcpy(name_buffer,cur_path);
	name_buffer[len-1] = '\0';
	struct inode *inode_ptr = _inode_alloc_and_init(i_opts_ptr,defalut_file_operations_ptr,
		FLAG_INODE_HARD|FLAG_INODE_READ|FLAG_INODE_WRITE);
	if(NULL == inode_ptr)
	{
		KA_WARN(DEBUG_TYPE_VFS,"inode malloc fail\n");
		ka_free(name_buffer);
		return -ERROR_NO_MEM;
	}
	struct dentry *buffer = _folder_dentry_alloc_and_init(dentry_ptr,inode_ptr,name_buffer,
		FLAG_DENTRY_FOLDER|FLAG_DENTRY_NAME_CHANGE_DIS|FLAG_DENTRY_NOT_RELEASED|FLAG_NEED_REFLASH);
	if(NULL == buffer)
	{
		ka_free(name_buffer);
		ka_free(inode_ptr);
		return -ERROR_NO_MEM;
	}
	if(init(para) < 0)
	{
		KA_WARN(DEBUG_TYPE_VFS,"fs init fail\n");
		return -ERROR_DISK;
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

/**
 * @Author      kaka
 * @DateTime    2019-04-21
 * @description : use this function to register a real file system
 * into VFS
 * @param       mount_point the root path of the fs
 * @param       init        [description]
 * @param       para        [description]
 * @param       i_opts_ptr  [description]
 * @return                  [description]
 */
int fs_register(const char *mount_point,fs_init_fun init,void *para,struct inode_operations *i_opts_ptr)
{
	if((NULL == mount_point) || (NULL == init))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	if('/' != *mount_point)
	{
		return -ERROR_LOGIC;
	}
	return _fs_register(mount_point,init,i_opts_ptr,i_opts_ptr);
}

/*
 init the VFS
 */
extern struct inode_operations fat_inode_operations;
static void __init_vfs(void)
{
#if CONFIG_SHELL_EN
	int error;
	error = Vector_init(&para_arv_vector,16,MKVFADD(16));
	if(error < 0)
	{
		panic("vfs init fail\n");
	}
	ASSERT(FUN_EXECUTE_SUCCESSFULLY == error,ASSERT_PARA_AFFIRM);
#endif
	_inode_init(NULL,NULL,FLAG_INODE_SOFT|FLAG_INODE_READ|FLAG_INODE_WRITE,&root_inode);
	_inode_init(NULL,NULL,FLAG_INODE_SOFT|FLAG_INODE_READ|FLAG_INODE_WRITE,&dev_inode);
	_init_dentry(&root_dentry,&root_dentry,
		&root_inode,"/",
		FLAG_DENTRY_FOLDER|FLAG_DENTRY_NAME_CHANGE_DIS|FLAG_DENTRY_NOT_RELEASED);
	root_dentry.d_parent = &root_dentry;
	_init_dentry(&dev_dentry,&root_dentry,
		&dev_inode,"dev",
		FLAG_DENTRY_FOLDER|FLAG_DENTRY_NAME_CHANGE_DIS|FLAG_DENTRY_NOT_RELEASED);
	current_dentry_ptr = &root_dentry; /* set pwd */
#if CONFIG_SHELL_EN
/* add tab feature */
/* command cd */
	command_cd_ptr = _get_command_ptr("cd");
	ASSERT(NULL != command_cd_ptr,ASSERT_PARA_AFFIRM);
/* command rm */
	command_rm_ptr = _get_command_ptr("rm");
	ASSERT(NULL != command_rm_ptr,ASSERT_PARA_AFFIRM);
/* command rmdir */
	command_rmdir_ptr = _get_command_ptr("rmdir");
	ASSERT(NULL != command_rmdir_ptr,ASSERT_PARA_AFFIRM);	
/* command ls */
	command_ls_ptr = _get_command_ptr("ls");
	ASSERT(NULL != command_ls_ptr,ASSERT_PARA_AFFIRM);
/* command cat */
	command_cat_ptr = _get_command_ptr("cat");
	ASSERT(NULL != command_cat_ptr,ASSERT_PARA_AFFIRM);
/* command echo */
	command_echo_ptr = _get_command_ptr("echo");
	ASSERT(NULL != command_echo_ptr,ASSERT_PARA_AFFIRM);
/* command rename */
	command_rename_ptr = _get_command_ptr("rename");
	ASSERT(NULL != command_rename_ptr,ASSERT_PARA_AFFIRM);	
/* last step */
	update_para_arv_vector(); /* set tab list */
#endif
}
INIT_FUN(__init_vfs,2);
