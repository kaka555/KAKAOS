#ifndef _EXPORT_H
#define _EXPORT_H

struct rt_module_symtab
{
	void       *addr;
	const char *name;
};

#define RTM_EXPORT(symbol)                                            \
const char __rtmsym_##symbol##_name[] __attribute__((section(".rodata.name"))) = #symbol; \
const struct rt_module_symtab __rtmsym_##symbol __attribute__((section(".RTMSymTab")))= \
{                                                                     \
    (void *)&symbol,                                                  \
    __rtmsym_##symbol##_name                                          \
};


#endif