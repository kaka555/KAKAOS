#ifndef _SYSFS_H
#define _SYSFS_H

struct kobject
{
	
};

struct kset
{
	
};

struct kobject *kobject_alloc_and_init();
struct kobject *kset_alloc_and_init();

int sysfs_add_kobject(struct kobject *kobject_ptr);
int sysfs_add_kset(struct kset *kset_ptr);

#endif
