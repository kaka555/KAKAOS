#include <export.h>
#include <myMicroLIB.h>

extern unsigned long _ka_module_symtab_begin;
extern unsigned long _ka_module_symtab_end;

UINT32 get_export_function_addr(const char *fun_name)
{
	struct ka_module_symtab *index;

    for (index = (struct ka_module_symtab *)(&_ka_module_symtab_begin);
            index != (struct ka_module_symtab *)(&_ka_module_symtab_end); index ++)
    {
        if (0 == ka_strcmp(index->name, fun_name))
            return (UINT32)index->addr;
    }

    return 0;
}
