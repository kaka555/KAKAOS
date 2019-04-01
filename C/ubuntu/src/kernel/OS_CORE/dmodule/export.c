#include <export.h>
#include <myMicroLIB.h>
#include <myassert.h>
#include <ka_configuration.h>

#if CONFIG_MODULE

extern unsigned long _ka_module_symtab_begin;
extern unsigned long _ka_module_symtab_end;

UINT32 _get_sys_export_function_addr(const char *fun_name)
{
	ASSERT(NULL != fun_name);
	struct ka_module_symtab *index;

    for (index = (struct ka_module_symtab *)(&_ka_module_symtab_begin);
            index != (struct ka_module_symtab *)(&_ka_module_symtab_end); index++)
    {
        if (0 == ka_strcmp(index->name, fun_name))
            return (UINT32)index->addr;
    }

    return 0;
}

void shell_symbol_list_display(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	struct ka_module_symtab *index;
	ka_printf("export symbol list:\n");
	for (index = (struct ka_module_symtab *)(&_ka_module_symtab_begin);
            index != (struct ka_module_symtab *)(&_ka_module_symtab_end); index++)
	{
		ka_printf("%s\n",index->name);
	}
}

#endif
