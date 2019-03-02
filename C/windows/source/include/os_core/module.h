#ifndef _MODULE_H
#define _MODULE_H

#include <double_linked_list.h>
#include <TCB.h>
#include <kakaosstdint.h>

#define RT_DLMODULE_STAT_INIT       0x00
#define RT_DLMODULE_STAT_RUNNING    0x01
#define RT_DLMODULE_STAT_CLOSING    0x02
#define RT_DLMODULE_STAT_CLOSED     0x03

typedef int                             rt_bool_t;      /**< boolean type */
#define RT_TRUE                         1               /**< boolean true  */
#define RT_FALSE                        0               /**< boolean fails */

struct rt_dlmodule;
typedef void* rt_addr_t;

struct rt_module_symtab
{
    void       *addr;
    const char *name;
};

#define RT_NAME_MAX 10

struct rt_object
{
    char       name[RT_NAME_MAX];                       /**< name of kernel object */
    UINT8 type;                                    /**< type of kernel object */
    UINT8 flag;                                    /**< flag of kernel object */
    void      *module_id;                               /**< id of application module */
};

typedef int  (*rt_dlmodule_entry_func_t)(int argc, char** argv);
typedef void (*rt_dlmodule_init_func_t)(struct rt_dlmodule *module);
typedef void (*rt_dlmodule_cleanup_func_t)(struct rt_dlmodule *module);

struct rt_dlmodule
{
    struct rt_object parent;
    struct list_head object_list;  /* objects inside this module */

    UINT8 stat;        /* status of module */

    /* main thread of this module */
    UINT16 priority;
    UINT32 stack_size;
    TCB *main_thread;
    /* the return code */
    int ret_code;

    /* VMA base address for the first LOAD segment */
    UINT32 vstart_addr;

    /* module entry, RT_NULL for dynamic library */
    rt_dlmodule_entry_func_t  entry_addr;
    char *cmd_line;         /* command line */

    rt_addr_t   mem_space;  /* memory space */
    UINT32 mem_size;   /* sizeof memory space */

    /* init and clean function */
    rt_dlmodule_init_func_t     init_func;
    rt_dlmodule_cleanup_func_t  cleanup_func;

    UINT16 nref;       /* reference count */

    UINT16 nsym;       /* number of symbols in the module */
    struct rt_module_symtab *symtab;    /* module symbol table */
};

void put_in_module_buffer(char c);
void clear_module_buffer(void);
struct rt_dlmodule* dlmodule_exec(void);
void set_module_buffer(void *add);

#endif
