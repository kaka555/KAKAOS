#include <elf.h>
#include <module.h>
#include <myMicroLIB.h>
#include <osinit.h>
#include <os_schedule.h>
#include <myassert.h>
#include <export.h>

#if CONFIG_MODULE

#define elf_module        ((Elf32_Ehdr *)module_ptr)

static char *module_buffer = NULL;
static unsigned int num = 0;
static struct list_head module_list_head;

void __init_module(void)
{
    INIT_LIST_HEAD(&module_list_head);
}

void shell_modinfo(int argc, char const *argv[])
{
    struct dynamic_module *dynamic_module_ptr;
    unsigned int i;
    list_for_each_entry(dynamic_module_ptr,&module_list_head,module_list)
    {
        ka_printf("module name is %s\n",dynamic_module_ptr->name);
        ka_printf("module state:");
        switch(dynamic_module_ptr->module_state)
        {
            default :
                ASSERT(0);ka_printf("\n"); break;
            case MODULE_STATE_INIT:
                ka_printf("STATE_INIT\n"); break;
            case MODULE_STATE_LOADED:
                ka_printf("STATE_LOADED\n"); break;
            case MODULE_STATE_RUN:
                ka_printf("MODULE_STATE_RUN\n"); break;
        }
        ka_printf("module_space : %p\n",dynamic_module_ptr->module_space);
        ka_printf("module_size : %u\n",dynamic_module_ptr->module_size);
        ka_printf("export symbols : \n");
        for(i=0;i<dynamic_module_ptr->export_symbols_num;++i)
        {
            ka_printf("no.%u : %s\n",i+1,dynamic_module_ptr->export_symbols_array[i].name);
        }
        ka_printf("ref : %u\n",dynamic_module_ptr->ref);
    }
}

void shell_list_module(int argc, char const *argv[])
{
    struct dynamic_module *dynamic_module_ptr;
    unsigned int i = 0;
    list_for_each_entry(dynamic_module_ptr,&module_list_head,module_list)
    {
        ++i;
        ka_printf("module name is %s\n",dynamic_module_ptr->name);
    }
    ka_printf("total %u modules\n",i);
}

int dlmodule_relocate(struct dynamic_module *module, Elf32_Rel *rel, Elf32_Addr sym_val)
{
    Elf32_Addr *where, tmp;
    Elf32_Sword addend, offset;
    UINT32 upper, lower, sign, j1, j2;

    where = (Elf32_Addr *)((UINT8 *)module->module_space
                           + rel->r_offset
                           - module->vstart_addr);
    switch (ELF32_R_TYPE(rel->r_info))
    {
    case R_ARM_NONE:
        break;
    case R_ARM_ABS32:
        *where += (Elf32_Addr)sym_val;
        /*ka_printf("R_ARM_ABS32: %x -> %x\n",
                  where, *where);*/
        break;
    case R_ARM_PC24:
    case R_ARM_PLT32:
    case R_ARM_CALL:
    case R_ARM_JUMP24:
        addend = *where & 0x00ffffff;
        if (addend & 0x00800000)
            addend |= 0xff000000;
        tmp = sym_val - (Elf32_Addr)where + (addend << 2);
        tmp >>= 2;
        *where = (*where & 0xff000000) | (tmp & 0x00ffffff);
        /*ka_printf("R_ARM_PC24: %x -> %x\n",
                  where, *where);*/
        break;
    case R_ARM_REL32:
        *where += sym_val - (Elf32_Addr)where;
        /*ka_printf("R_ARM_REL32: %x -> %x, sym %x, offset %x\n",
                  where, *where, sym_val, rel->r_offset);*/
        break;
    case R_ARM_V4BX:
        *where &= 0xf000000f;
        *where |= 0x01a0f000;
        break;

    case R_ARM_GLOB_DAT:
    case R_ARM_JUMP_SLOT:
        *where = (Elf32_Addr)sym_val;
        /*ka_printf("R_ARM_JUMP_SLOT: 0x%x -> 0x%x 0x%x\n",
                  where, *where, sym_val);*/
        break;
#if 0        /* To do */
    case R_ARM_GOT_BREL:
        temp   = (Elf32_Addr)sym_val;
        *where = (Elf32_Addr)&temp;
        RT_DEBUG_LOG(RT_DEBUG_MODULE, ("R_ARM_GOT_BREL: 0x%x -> 0x%x 0x%x\n",
                                       where, *where, sym_val));
        break;
#endif

    case R_ARM_RELATIVE:
        *where = (Elf32_Addr)sym_val + *where;
        /*ka_printf("R_ARM_RELATIVE: 0x%x -> 0x%x 0x%x\n",
                  where, *where, sym_val);*/
        break;
    case R_ARM_THM_CALL:
    case R_ARM_THM_JUMP24:
        upper  = *(UINT16 *)where;
        lower  = *(UINT16 *)((Elf32_Addr)where + 2);

        sign   = (upper >> 10) & 1;
        j1     = (lower >> 13) & 1;
        j2     = (lower >> 11) & 1;
        offset = (sign << 24) |
                 ((~(j1 ^ sign) & 1) << 23) |
                 ((~(j2 ^ sign) & 1) << 22) |
                 ((upper & 0x03ff) << 12) |
                 ((lower & 0x07ff) << 1);
        if (offset & 0x01000000)
            offset -= 0x02000000;
        offset += sym_val - (Elf32_Addr)where;

        if (!(offset & 1) ||
                offset <= (INT32)0xff000000 ||
                offset >= (INT32)0x01000000)
        {
            ka_printf("Module: Only Thumb addresses allowed\n");

            return -1;
        }

        sign = (offset >> 24) & 1;
        j1   = sign ^ (~(offset >> 23) & 1);
        j2   = sign ^ (~(offset >> 22) & 1);
        *(UINT16 *)where = (UINT16)((upper & 0xf800) |
                                    (sign << 10) |
                                    ((offset >> 12) & 0x03ff));
        *(UINT16 *)(where + 2) = (UINT16)((lower & 0xd000) |
                                          (j1 << 13) | (j2 << 11) |
                                          ((offset >> 1) & 0x07ff));
        upper = *(UINT16 *)where;
        lower = *(UINT16 *)((Elf32_Addr)where + 2);
        break;
    default:
        /*ka_printf("do nothing\n");*/
        return -1;
    }

    return 0;
}

int dlmodule_load_shared_object(struct dynamic_module* module, void *module_ptr)
{
    int linked   = KA_FALSE;
    UINT32 index, module_size = 0;
    Elf32_Addr vstart_addr, vend_addr;
    int has_vstart = KA_FALSE;

    ASSERT(module_ptr != NULL);

    /* get the ELF image size */
    vstart_addr = vend_addr = NULL;
    for (index = 0; index < elf_module->e_phnum; index++)
    {
        if (phdr[index].p_type != PT_LOAD)
            continue;

        ka_printf("LOAD segment: %d, 0x%p, 0x%x\n", index, phdr[index].p_vaddr, phdr[index].p_memsz);

        if (phdr[index].p_memsz < phdr[index].p_filesz)
        {
            ka_printf("invalid elf: segment %d: p_memsz: %d, p_filesz: %d\n",
                       index, phdr[index].p_memsz, phdr[index].p_filesz);
            return NULL;
        }
        if (!has_vstart)
        {
            vstart_addr = phdr[index].p_vaddr;
            vend_addr = phdr[index].p_vaddr + phdr[index].p_memsz;
            has_vstart = KA_TRUE;
            if (vend_addr < vstart_addr)
            {
                ka_printf("invalid elf: segment %d: p_vaddr: %d, p_memsz: %d\n",
                           index, phdr[index].p_vaddr, phdr[index].p_memsz);
                return NULL;
            }
        }
        else
        {
            if (phdr[index].p_vaddr < vend_addr)
            {
                ka_printf("invalid elf: segment should be sorted and not overlapped\n");
                return NULL;
            }
            if (phdr[index].p_vaddr > vend_addr + 16)
            {
                /* There should not be too much padding in the object files. */
                ka_printf("warning: too much padding before segment %d\n", index);
            }

            vend_addr = phdr[index].p_vaddr + phdr[index].p_memsz;
            if (vend_addr < phdr[index].p_vaddr)
            {
                ka_printf("invalid elf: "
                           "segment %d address overflow\n", index);
                return NULL;
            }
        }
    }

    module_size = vend_addr - vstart_addr;
    ka_printf("module size: %d, vstart_addr: 0x%p\n", module_size, vstart_addr);
    if (module_size == 0)
    {
        ka_printf("Module: size error\n");
        return -ERROR_LOGIC;
    }

    module->vstart_addr = vstart_addr;
    module->ref = 0;

    /* allocate module space */
    module->module_space = ka_malloc(module_size);
    if (NULL == module->module_space)
    {
        ka_printf("Module: allocate space failed.\n");
        return -ERROR_NO_MEM;
    }
    module->module_size = module_size;

    /* zero all space */
    ka_memset(module->module_space, 0, module_size);
    /*load program*/
    for (index = 0; index < elf_module->e_phnum; index++)
    {
        if (phdr[index].p_type == PT_LOAD)
        {
            ka_memcpy(module->module_space + phdr[index].p_vaddr - vstart_addr,
                      (UINT8 *)elf_module + phdr[index].p_offset,
                      phdr[index].p_filesz);
        }
    }

    /* set module entry */
    module->entry = module->module_space + elf_module->e_entry - vstart_addr;
    /* handle relocation section */
    for (index = 0; index < elf_module->e_shnum; index ++)
    {
        UINT32 i, nr_reloc;
        Elf32_Sym *symtab;
        Elf32_Rel *rel;
        UINT8 *strtab;
        static int unsolved = KA_FALSE;

        if (!IS_REL(shdr[index]))
            continue;

        /* get relocate item */
        rel = (Elf32_Rel *)((UINT8 *)module_ptr + shdr[index].sh_offset);

        /* locate .rel.plt and .rel.dyn section */
        symtab = (Elf32_Sym *)((UINT8 *)module_ptr +
                               shdr[shdr[index].sh_link].sh_offset);
        strtab = (UINT8 *)module_ptr +
                 shdr[shdr[shdr[index].sh_link].sh_link].sh_offset;
        nr_reloc = (UINT32)(shdr[index].sh_size / sizeof(Elf32_Rel));

        /* relocate every items */
        for (i = 0; i < nr_reloc; i ++)
        {
            Elf32_Sym *sym = &symtab[ELF32_R_SYM(rel->r_info)];

            //ka_printf("relocate symbol %s shndx %d", strtab + sym->st_name, sym->st_shndx);

            if ((sym->st_shndx != SHT_NULL) ||(ELF_ST_BIND(sym->st_info) == STB_LOCAL))
            {
                Elf32_Addr addr;

                addr = (Elf32_Addr)(module->module_space + sym->st_value - vstart_addr);
                if(dlmodule_relocate(module, rel, addr) < 0)
                {
                    return -ERROR_LOGIC;
                }
            }
            else if (!linked)
            {
                Elf32_Addr addr;

                //ka_printf("relocate symbol: %s", strtab + sym->st_name);
                /* need to resolve symbol in kernel symbol table */
                addr = get_export_function_addr((const char *)(strtab + sym->st_name));
                if (addr == 0)
                {
                    ka_printf("Module: can't find %s in kernel symbol table", strtab + sym->st_name);
                    unsolved = KA_TRUE;
                    break;
                }
                else
                {
                    if(dlmodule_relocate(module, rel, addr) < 0)
                    {
                        return -ERROR_LOGIC;
                    }
                }
            }
            rel ++;
        }

        if (unsolved) 
            return -ERROR_LOGIC;
    }
    /* construct module symbol table */
    for (index = 0; index < elf_module->e_shnum; index ++)
    {
        /* find .dynsym section */
        UINT8 *shstrab;
        shstrab = (UINT8 *)module_ptr +
                  shdr[elf_module->e_shstrndx].sh_offset;
        if (ka_strcmp((const char *)(shstrab + shdr[index].sh_name), ELF_DYNSYM) == 0)
            break;
    }
    /* found .dynsym section */
    if (index != elf_module->e_shnum)
    {
        int i, count = 0;
        Elf32_Sym  *symtab = NULL;
        UINT8 *strtab = NULL;

        symtab = (Elf32_Sym *)((UINT8 *)module_ptr + shdr[index].sh_offset);
        strtab = (UINT8 *)module_ptr + shdr[shdr[index].sh_link].sh_offset;

        for (i = 0; i < shdr[index].sh_size / sizeof(Elf32_Sym); i++)
        {
            if ((ELF_ST_BIND(symtab[i].st_info) == STB_GLOBAL) &&
                (ELF_ST_TYPE(symtab[i].st_info) == STT_FUNC))   //export function only
            {
                ++count;
                if(0 == ka_strcmp("init_module",(const char *)(strtab + symtab[i].st_name)))
                {
                    --count;
                    module->init = module->module_space + symtab[i].st_value - module->vstart_addr;
                }
            }
        }

        module->export_symbols_array = (struct ka_module_symtab *)ka_malloc
                                        (count * sizeof(struct ka_module_symtab));
        module->export_symbols_num = count;
        for (i = 0, count = 0; i < shdr[index].sh_size / sizeof(Elf32_Sym); i++)
        {
            unsigned long length;

            if ((ELF_ST_BIND(symtab[i].st_info) != STB_GLOBAL) ||
                (ELF_ST_TYPE(symtab[i].st_info) != STT_FUNC)   ||
                (0 == ka_strcmp("init_module",(const char *)(strtab + symtab[i].st_name))))
                continue;

            length = ka_strlen((const char *)(strtab + symtab[i].st_name)) + 1;

            module->export_symbols_array[count].addr =
                (void *)(module->module_space + symtab[i].st_value - module->vstart_addr);
            module->export_symbols_array[count].name = ka_malloc(length);
            ka_memset((void *)module->export_symbols_array[count].name, 0, length);
            ka_memcpy((void *)module->export_symbols_array[count].name,
                      strtab + symtab[i].st_name,
                      length);
            count ++;
        }
    }

    return FUN_EXECUTE_SUCCESSFULLY;
}

static void __init_d_module(struct dynamic_module *dynamic_module_ptr)
{
    ka_strcpy(dynamic_module_ptr->name,D_MODULE_DEFAULT_NAME);
    INIT_LIST_HEAD(&(dynamic_module_ptr->module_list));
    dynamic_module_ptr->module_state = MODULE_STATE_INIT;
    dynamic_module_ptr->init                    = NULL;
    dynamic_module_ptr->exit                    = NULL;
    dynamic_module_ptr->module_space            = NULL;
    dynamic_module_ptr->module_size             = 0;
    dynamic_module_ptr->init_space              = NULL;
    dynamic_module_ptr->entry                   = NULL;
    dynamic_module_ptr->vstart_addr             = NULL;
    dynamic_module_ptr->export_symbols_array    = NULL;
    dynamic_module_ptr->export_symbols_num      = 0;
    dynamic_module_ptr->ref                     = 0;
}

struct dynamic_module* dlmodule_load(void)
{
    int error = FUN_EXECUTE_SUCCESSFULLY;
    UINT8 *module_ptr = (UINT8 *)module_buffer;
    struct dynamic_module *module = NULL;

    /* check ELF header */
    if (ka_strncmp((const char *)elf_module->e_ident, (const char *)RTMMAG, SELFMAG) != 0 &&
            ka_strncmp((const char *)elf_module->e_ident, (const char *)ELFMAG, SELFMAG) != 0)
    {
        ka_printf("Module: magic error\n");
        goto __exit;
    }

    /* check ELF class */
    if (elf_module->e_ident[EI_CLASS] != ELFCLASS32)
    {
        ka_printf("Module: ELF class error\n");
        goto __exit;
    }

    module = (struct dynamic_module *)ka_malloc(sizeof(struct dynamic_module));
    if (!module) goto __exit;
    __init_d_module(module);
    
    /* set initial priority and stack size */
    //module->priority = PRIO_MAX - 10;
    //module->stack_size = 2048;

    if (elf_module->e_type == ET_DYN)
    {
        ka_printf("load shared file\n");
        error = dlmodule_load_shared_object(module, module_ptr);
    }
    else
    {
        ka_printf("Module: unsupported elf type\n");
        goto __exit;
    }

    /* check return value */
    if (FUN_EXECUTE_SUCCESSFULLY != error) goto __exit;

    /* increase module reference count */
    module->ref ++;
    list_add(&module->module_list,&module_list_head);

    /* set module initialization and cleanup function */
    //module->init = dlsym(module, "init");
    //module->exit = dlsym(module, "exit");

    set_module_state(module,MODULE_STATE_LOADED);
    /* do module initialization */
    if (module->init)
    {
        error = module->init();
        if(FUN_EXECUTE_SUCCESSFULLY != error)
        {
            ka_printf("init error\n");
            goto __exit;
        }
    }

    return module;

__exit:
    if (module_ptr) ka_free(module_ptr);
		
	if(module) 
    {
        if(!list_empty(&module->module_list))
        {
            list_del(&module->module_list);
        }
        ka_free(module);
    }

    clear_module_buffer();
		
    return NULL;
}


int dlmodule_exec(
    unsigned int stack_size,
    TASK_PRIO_TYPE prio,
    const char *name)
{
    struct dynamic_module *module = NULL;
    int error;
    TCB *TCB_ptr;

    module = dlmodule_load();
    if ((module) && (module->entry))
    {
        if(name)
        {
            ka_strcpy(module->name,name);
        }
        if(prio >= PRIO_MAX)
        {
            prio = D_MODULE_DEFAULT_PRIO;
        }
        error = task_creat_ready(stack_size, prio, HZ / 10, module->name,
                                 (functionptr)(module->entry), NULL, &TCB_ptr);
        if (!error)
        {
            ka_printf("module_thread run\n");
        }
        else
        {
            ka_printf("module_thread fail,error code is %d\n",error);
            return error;
        }
        if(module->init)
        {
            error = module->init();
            if(error < 0)
            {
                ka_printf("module init error\n");
                task_delete(TCB_ptr);
                //remove_a_module(module);
            }
        }
    }
    else
    {
        return -ERROR_LOGIC;
    }

    return FUN_EXECUTE_SUCCESSFULLY;
}

void put_in_module_buffer(char c)
{
	if(module_buffer)
	{
		*(module_buffer + num++) = c;
	}
    else
    {
        ka_printf("no room for module\n");
    }
}

void show_get_size(void)
{
    ka_printf("get %u bytes\n",num);
}

void clear_module_buffer(void)
{
	module_buffer = NULL;
	num = 0;
}

void set_module_buffer(void *add)
{
	module_buffer = (char *)add;
}

#endif
