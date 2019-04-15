#include <vfs.h>
#include <ff.h>
#include <os_error.h>
#include <printf_debug.h>
#include <sys_init_fun.h>
#include <myassert.h>
#include <fs.h>

#define FAT_ROOT "/fat"
#define FAT_NAME "fat"

static FIL f;

static char fat_path[100] = "1:";
static unsigned int fat_path_len = 2;  /* 2 == strlen("1:") */

static inline void path_add(const char *name)
{
	fat_path[fat_path_len++] = '/';
	ka_strcpy(fat_path + fat_path_len,name);
	fat_path_len += ka_strlen(name);
}

static inline void path_back(const char *name)
{
	unsigned int len = ka_strlen(name);
	fat_path_len -= len+1;
	fat_path[fat_path_len] = '\0';
}

static FRESULT scan_files (struct inode *inode_ptr,struct dentry *dentry_ptr,char* path) 
{ 
    FRESULT res; 	
    FILINFO fno; 
    DIR dir;          
    char *fn;        
	
#if _USE_LFN 
    static char lfn[_MAX_LFN*2 + 1]; 	
    fno.lfname = lfn; 
    fno.lfsize = sizeof(lfn); 
#endif 
    res = f_opendir(&dir, path); 
    if (res == FR_OK) 
	{
	    for (;;) 
		{ 
	    	res = f_readdir(&dir, &fno); 								
	    	if (res != FR_OK || fno.fname[0] == 0) break; 	
#if _USE_LFN 
	    	fn = *fno.lfname ? fno.lfname : fno.fname; 
#else 
	    	fn = fno.fname; 
#endif 	
	    	if (*fn == '.') continue; 	  
	    	if (fno.fattrib & AM_DIR)         
			{
				KA_WARN(DEBUG_FAT,"%s/%s\r\n", path, fn);
				if(has_same_name_file(dentry_ptr,fn))
				{
					continue ;
				}
				res = ___add_folder(dentry_ptr,fn,inode_ptr->i_f_ops);         
	    		if (res != FUN_EXECUTE_SUCCESSFULLY)
	    		{
	    			KA_WARN(DEBUG_FAT,"add folder fail\n");
					break; 
	    		}
	    	} 
			else 
			{
				KA_WARN(DEBUG_FAT,"%s/%s\r\n", path, fn);  
				res = ___add_file(dentry_ptr,fn,inode_ptr->i_f_ops);
				if (res != FUN_EXECUTE_SUCCESSFULLY)
	    		{
	    			KA_WARN(DEBUG_FAT,"add file fail\n");
					break; 
	    		}
	        }
	    } 
  	}
  	else
  	{
  		KA_WARN(DEBUG_FAT,"disk open folder fail\n");
  	}
    return res; 
}

static void _fat_cd(struct dentry *dentry_ptr)
{
	const char *name = dentry_ptr->name;
	if(0 == ka_strcmp(name,FAT_NAME))
	{
		return ;
	}
	_fat_cd(dentry_ptr->d_parent);
	path_add(name);
	return ;
}

static void fat_cd(struct dentry *dentry_ptr)
{
	fat_path_len = 2;
	fat_path[2] = '\0';
	_fat_cd(dentry_ptr);
	KA_WARN(DEBUG_FAT,"fat_path is %s\n",fat_path);
}

static int fat_get_size(struct file *file_ptr)
{
	file_ptr->file_len = f_size((FIL*)(file_ptr->private_data));
	return FUN_EXECUTE_SUCCESSFULLY;
}

static int fat_inode_open(struct file *file_ptr)
{
	ASSERT(NULL != file_ptr,ASSERT_INPUT);
	FIL *fnew = ka_malloc(sizeof(FIL));
	if(NULL == fnew)
	{
		KA_WARN(DEBUG_FAT,"fat_inode_open allocate mem fail\n");
		return -ERROR_NO_MEM;
	}
	fat_cd(file_ptr->f_den);
	FRESULT res_flash = f_open(fnew,fat_path,FA_OPEN_ALWAYS | file_ptr->f_mode);
	if(res_flash != FR_OK)
	{
		KA_WARN(DEBUG_FAT,"fat_inode_open fail\n");
		return -ERROR_DISK;
	}
	file_ptr->private_data = fnew;
	fat_get_size(file_ptr);  /* fill the file_len */
	return FUN_EXECUTE_SUCCESSFULLY;
}

static int fat_inode_close(struct file *file_ptr)
{
	ASSERT(NULL != file_ptr,ASSERT_INPUT);
	f_close(file_ptr->private_data);
	ka_free(file_ptr->private_data);
	return FUN_EXECUTE_SUCCESSFULLY;
}

static int fat_refresh(struct inode *inode_ptr,struct dentry *dentry_ptr)
{
	ASSERT(NULL != inode_ptr,ASSERT_INPUT);
	ASSERT(NULL != dentry_ptr,ASSERT_INPUT);
	KA_WARN(DEBUG_FAT,"fat_refresh\n");
	fat_cd(dentry_ptr);
	if(is_folder(dentry_ptr))
	{
		scan_files(inode_ptr,dentry_ptr,fat_path);
	}
	KA_WARN(DEBUG_FAT,"fat_path is %s\n",fat_path);
	return FUN_EXECUTE_SUCCESSFULLY;
}

static int fat_change_name(struct inode *inode_ptr,struct dentry *dentry_ptr,const char *new_name)
{
	ASSERT(NULL != inode_ptr,ASSERT_INPUT);
	ASSERT(NULL != dentry_ptr,ASSERT_INPUT);
	ASSERT(NULL != new_name,ASSERT_INPUT);
	FRESULT error;
	KA_WARN(DEBUG_FAT,"fat_change_name\n");
	fat_cd(dentry_ptr->d_parent);
	unsigned int new_name_len = fat_path_len + ka_strlen(new_name) + 1;
	char *name = ka_malloc(new_name_len);
	if(NULL == name)
	{
		KA_WARN(DEBUG_FAT,"fat_change_name allocate room fail\n");
		return -ERROR_NO_MEM;
	}
	ka_strcpy(name,fat_path);
	ka_strcpy(name+fat_path_len,new_name);
	name[new_name_len-1] = '\0';
	path_add(dentry_ptr->name);
	KA_WARN(DEBUG_FAT,"fat_path is %s\n",fat_path);
	error = f_rename(fat_path,name);
	path_back(dentry_ptr->name);
	ka_free(name);
	if(FR_OK == error)
	{
		return FUN_EXECUTE_SUCCESSFULLY;
	}
	KA_WARN(DEBUG_FAT,"fat_change_name fail\n");
	return -error;
}

static int fat_add_sub_file(struct dentry *dentry_ptr,const char *file_name)
{
	ASSERT(NULL != dentry_ptr,ASSERT_INPUT);
	ASSERT(NULL != file_name,ASSERT_INPUT);
	KA_WARN(DEBUG_FAT,"fat_add_sub_file\n");
	fat_cd(dentry_ptr);
	path_add(file_name);
	KA_WARN(DEBUG_FAT,"fat_path is %s\n",fat_path);
	FRESULT error = f_open(&f,fat_path,FA_CREATE_ALWAYS);
	if(FR_OK != error)
	{
		ka_printf("add_sub_file error,code is %u\n",error);
		return -ERROR_DISK;
	}
	error = f_close(&f); 
	if(FR_OK != error)
	{
		ka_printf("f_close error,code is %u\n",error);
		return -ERROR_DISK;
	}
	path_back(file_name);
	KA_WARN(DEBUG_FAT,"fat_add_sub_file succeed\n");
	return FUN_EXECUTE_SUCCESSFULLY;
}

static int fat_add_sub_folder(struct dentry *dentry_ptr,const char *folder_name)
{
	ASSERT(NULL != dentry_ptr,ASSERT_INPUT);
	ASSERT(NULL != folder_name,ASSERT_INPUT);
	KA_WARN(DEBUG_FAT,"fat_add_sub_file\n");
	fat_cd(dentry_ptr);
	path_add(folder_name);
	KA_WARN(DEBUG_FAT,"fat_path is %s\n",fat_path);
	FRESULT error = f_mkdir(fat_path);
	if(FR_OK != error)
	{
		ka_printf("fat_add_sub_folder error,code is %u\n",error);
		return -ERROR_DISK;
	}
	path_back(folder_name);
	KA_WARN(DEBUG_FAT,"fat_add_sub_folder succeed\n");
	return FUN_EXECUTE_SUCCESSFULLY;
}

static int fat_read_data(struct file *file_ptr,void *store_ptr,unsigned int len,unsigned int offset)
{
	ASSERT(NULL != file_ptr,ASSERT_INPUT);
	ASSERT(NULL != store_ptr,ASSERT_INPUT);
	FRESULT error;
	int num;
	KA_WARN(DEBUG_FAT,"fat_read_data\n");
	ASSERT(offset <= file_ptr->file_len,ASSERT_PARA_AFFIRM);
	if(f_lseek(file_ptr->private_data,offset))
	{
		ASSERT(0,ASSERT_BAD_EXE_LOCATION);
		return -ERROR_DISK;
	}
	error = f_read(file_ptr->private_data,store_ptr,len,(unsigned int *)&num);
	KA_WARN(DEBUG_FAT,"read error code is %u\n",error);
	return num;
}

static int fat_write_data(struct file *file_ptr,void *data_ptr,unsigned int len,unsigned int offset)
{
	ASSERT(NULL != file_ptr,ASSERT_INPUT);
	ASSERT(NULL != data_ptr,ASSERT_INPUT);
	FRESULT error;
	int num;
	KA_WARN(DEBUG_FAT,"fat_write_data\n");
	ASSERT(offset <= file_ptr->file_len,ASSERT_PARA_AFFIRM);
	if(f_lseek(file_ptr->private_data,offset))
	{
		ASSERT(0,ASSERT_BAD_EXE_LOCATION);
		return -ERROR_DISK;
	}
	error = f_write(file_ptr->private_data,data_ptr,len,(unsigned int *)&num);
	KA_WARN(DEBUG_FAT,"write error code is %u\n",error);
	return num;
}

static int fat_remove(struct dentry *dentry_ptr)
{
	ASSERT(NULL != dentry_ptr,ASSERT_INPUT);
	fat_cd(dentry_ptr->d_parent);
	path_add(dentry_ptr->name);
	int error = f_unlink(fat_path);
	if(FR_OK != error)
	{
		KA_WARN(DEBUG_FAT,"fat_remove error code is %u\n",error);
	}
	path_back(dentry_ptr->name);
	return FUN_EXECUTE_SUCCESSFULLY;
}

static int fat_remove_dir(struct dentry *dentry_ptr)
{
	ASSERT(NULL != dentry_ptr,ASSERT_INPUT);
	fat_cd(dentry_ptr->d_parent);
	path_add(dentry_ptr->name);
	int error = f_unlink(fat_path);
	if(FR_OK != error)
	{
		KA_WARN(DEBUG_FAT,"fat_remove_dir error code is %u\n",error);
	}
	path_back(dentry_ptr->name);
	return FUN_EXECUTE_SUCCESSFULLY;
}

static struct inode_operations fat_inode_operations = {
	.inode_open = fat_inode_open,
	.inode_close = fat_inode_close,
	.change_name = fat_change_name,
	.refresh = fat_refresh,
	.add_sub_file = fat_add_sub_file,
	.add_sub_folder = fat_add_sub_folder,
	.read_data = fat_read_data,
	.write_data = fat_write_data,
	.remove = fat_remove,
	.remove_dir = fat_remove_dir,
	.get_size = fat_get_size,
};

static int init_fat(void *para)
{
	(void)para;
	FATFS fs;
	FRESULT res_flash;
	res_flash = f_mount(&fs,"1:",1);
	if(res_flash!=FR_OK)
	{
		KA_WARN(DEBUG_FAT,"fat init fail\n");
		return -ERROR_DISK;
	}
	else
	{
		KA_WARN(DEBUG_FAT,"fat init succeed\n");
		return FUN_EXECUTE_SUCCESSFULLY;
	}
}

void __init_fat(void)
{
	int error = fs_register(FAT_ROOT,init_fat,NULL,&fat_inode_operations);
	if(error < 0)
	{
		ka_printf("fat init fail,error code is %d\n",error);
		ASSERT(0,ASSERT_BAD_EXE_LOCATION);
		panic("fat initialization fail\n");
	}
}
INIT_FUN(__init_fat,3); /* excute after initialization of vfs */
