#include <vfs.h>
#include <ff.h>
#include <os_error.h>
#include <printf_debug.h>
#include <sys_init_fun.h>

static FIL f;

static int fat_change_name(struct inode *inode_ptr,struct dentry *dentry_ptr)
{
	FRESULT error;
	KA_WARN(DEBUG_FAT,"fat_change_name\n");
	error = f_rename(dentry_ptr->name,"kaka");
	if(FR_OK == error)
	{
		return FUN_EXECUTE_SUCCESSFULLY;
	}
	return -error;
}

static int fat_add_sub_file(struct inode *inode_ptr,const char *file_name)
{
	KA_WARN(DEBUG_FAT,"fat_add_sub_file\n");
	char *str = ka_malloc(2+ka_strlen(file_name));
	ka_strcpy(str,"1:");
	ka_strcpy(str+2,file_name);
	FRESULT error = f_open(&f,str,FA_CREATE_ALWAYS);
	ka_free(str);
	if(FR_OK != error)
	{
		ka_printf("add_sub_file error,code is %u\n",error);
		return -1;
	}
	error = f_close(&f); 
	if(FR_OK != error)
	{
		ka_printf("f_close error,code is %u\n",error);
		return -1;
	}
	KA_WARN(DEBUG_FAT,"fat_add_sub_file succeed\n");
	return FUN_EXECUTE_SUCCESSFULLY;
}

static int fat_read_data(struct inode *inode_ptr,void *store_ptr,unsigned int len,unsigned int offset)
{
	FRESULT error;
	int num;
	KA_WARN(DEBUG_FAT,"fat_read_data\n");
	error = f_read(&f,store_ptr,len,(unsigned int *)&num);
	ka_printf("read error code is %u\n",error);
	return num;
}

static int fat_write_data(struct inode *inode_ptr,void *data_ptr,unsigned int len,unsigned int offset)
{
	FRESULT error;
	int num;
	KA_WARN(DEBUG_FAT,"fat_write_data\n");
	error = f_write(&f,data_ptr,len,(unsigned int *)&num);
	ka_printf("write error code is %u\n",error);
	return num;
}

struct inode_operations fat_inode_operations = {
	.change_name = fat_change_name,
	.add_sub_file = fat_add_sub_file,
	.read_data = fat_read_data,
	.write_data = fat_write_data,
};

void __init_fat(void)
{
	FATFS fs;
	FRESULT res_flash;
	res_flash = f_mount(&fs,"1:",1);
	if(res_flash!=FR_OK)
	{
		KA_WARN(DEBUG_FAT,"fat init fail\n");
	}
	else
	{
		KA_WARN(DEBUG_FAT,"fat init succeed\n");
	}
}
INIT_FUN(__init_fat,1);
