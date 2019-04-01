#ifndef _MODULE_H
#define _MODULE_H

#include <double_linked_list.h>
#include <TCB.h>
#include <kakaosstdint.h>
#include <export.h>

#define RT_DLMODULE_STAT_INIT       0x00
#define RT_DLMODULE_STAT_RUNNING    0x01
#define RT_DLMODULE_STAT_CLOSING    0x02
#define RT_DLMODULE_STAT_CLOSED     0x03

#define D_MODULE_NAME_MAX               32
#define D_MODULE_DEFAULT_NAME           "dynamicmodule"
#define D_MODULE_DEFAULT_PRIO           (PRIO_MAX-2)
#define D_MODULE_DEFAULT_STACK_SIZE     2048

#define module_init(initfn)                 \
    int init_module(void) __attribute__((alias(#initfn)))
    
#define module_exit(initfn)                 \
    int exit_module(void) __attribute__((alias(#initfn)))

/*===================*/
/*module state*/

#define MODULE_STATE_DEFAULT    0x00
#define MODULE_STATE_INIT       MODULE_STATE_DEFAULT
#define MODULE_STATE_LOADED     0x01
#define MODULE_STATE_RUN        0x02
/*======================================================*/

struct dynamic_module
{
    char name[D_MODULE_NAME_MAX];
    struct list_head module_list;   /*link all modules*/
    UINT32 module_state;
    int (*init)(void);              /*this function will be called before module execute*/
    void (*exit)(void);             /*this function will be called after module return */
    void *module_space;
    unsigned int module_size;       /*the room size that module finally use*/
    void *init_space;               /* not use*/
    void *entry;                    /*module entry point*/
    UINT32 vstart_addr;             /*VMA base address for the first LOAD segment*/
    struct ka_module_symtab *export_symbols_array;
    unsigned int export_symbols_num;
    unsigned int ref;
    TCB *thread_TCB_ptr;
};

typedef struct dynamic_module d_module;

static inline void set_module_state(struct dynamic_module *dynamic_module_ptr,UINT32 state)
{
    dynamic_module_ptr->module_state = state;
}

void _put_in_module_buffer(char c);
void _clear_module_buffer(void);
int _dlmodule_exec(
    unsigned int stack_size,
    TASK_PRIO_TYPE prio,
    const char *name);
void _set_module_buffer(void *add);
int remove_module(struct dynamic_module *dynamic_module_ptr);
void shell_modinfo(int argc, char const *argv[]);
void shell_list_module(int argc, char const *argv[]);
int _check_same_mod_name(const char *name);
void _restart_module(
    unsigned int stack_size,
    TASK_PRIO_TYPE prio,
    const char *name);

#endif
