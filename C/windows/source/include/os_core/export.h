#ifndef _EXPORT_H
#define _EXPORT_H
#include <kakaosstdint.h>

struct ka_module_symtab
{
	void       *addr;
	const char *name;
};

#define EXPORT_SYMBOL(symbol)                                            \
const char __rtmsym_##symbol##_name[] __attribute__((section(".rodata.name"))) = #symbol; \
const struct ka_module_symtab __rtmsym_##symbol __attribute__((section(".RTMSymTab")))= \
{                                                                     \
    .addr = (void *)&symbol,                                                  \
    .name = __rtmsym_##symbol##_name,                                         \
}

UINT32 get_export_function_addr(const char *fun_name);

#endif