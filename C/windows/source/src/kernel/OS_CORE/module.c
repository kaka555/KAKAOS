#include <elf.h>
#include <module.h>
#include <myMicroLIB.h>
#include <osinit.h>
#include <os_schedule.h>
#include <myassert.h>
#include <export.h>

/* RT-Thread error code definitions */
#define RT_EOK                          0               /**< There is no error */
#define RT_ERROR                        1               /**< A generic error happens */
#define RT_ETIMEOUT                     2               /**< Timed out */
#define RT_EFULL                        3               /**< The resource is full */
#define RT_EEMPTY                       4               /**< The resource is empty */
#define RT_ENOMEM                       5               /**< No memory */
#define RT_ENOSYS                       6               /**< No system */
#define RT_EBUSY                        7               /**< Busy */
#define RT_EIO                          8               /**< IO error */
#define RT_EINTR                        9               /**< Interrupted system call */
#define RT_EINVAL                       10              /**< Invalid argument */

#define elf_module        ((Elf32_Ehdr *)module_ptr)

typedef long                       rt_err_t;       /**< Type for error number */

static char *module_buffer = NULL;
static unsigned int num = 0;

int dlmodule_relocate(struct rt_dlmodule *module, Elf32_Rel *rel, Elf32_Addr sym_val)
{
    Elf32_Addr *where, tmp;
    Elf32_Sword addend, offset;
    UINT32 upper, lower, sign, j1, j2;

    where = (Elf32_Addr *)((UINT8 *)module->mem_space
                           + rel->r_offset
                           - module->vstart_addr);
    switch (ELF32_R_TYPE(rel->r_info))
    {
    case R_ARM_NONE:
        break;
    case R_ARM_ABS32:
        *where += (Elf32_Addr)sym_val;
        ka_printf("R_ARM_ABS32: %x -> %x\n",
                  where, *where);
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
        ka_printf("R_ARM_PC24: %x -> %x\n",
                  where, *where);
        break;
    case R_ARM_REL32:
        *where += sym_val - (Elf32_Addr)where;
        ka_printf("R_ARM_REL32: %x -> %x, sym %x, offset %x\n",
                  where, *where, sym_val, rel->r_offset);
        break;
    case R_ARM_V4BX:
        *where &= 0xf000000f;
        *where |= 0x01a0f000;
        break;

    case R_ARM_GLOB_DAT:
    case R_ARM_JUMP_SLOT:
        *where = (Elf32_Addr)sym_val;
        ka_printf("R_ARM_JUMP_SLOT: 0x%x -> 0x%x 0x%x\n",
                  where, *where, sym_val);
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
        ka_printf("R_ARM_RELATIVE: 0x%x -> 0x%x 0x%x\n",
                  where, *where, sym_val);
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
        ka_printf("do nothing\n");
        return -1;
    }

    return 0;
}

rt_err_t dlmodule_load_shared_object(struct rt_dlmodule* module, void *module_ptr)
{
    rt_bool_t linked   = RT_FALSE;
    UINT32 index, module_size = 0;
    Elf32_Addr vstart_addr, vend_addr;
    rt_bool_t has_vstart;

    ASSERT(module_ptr != NULL);

    /* get the ELF image size */
    has_vstart = RT_FALSE;
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
            has_vstart = RT_TRUE;
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
        return -RT_ERROR;
    }

    module->vstart_addr = vstart_addr;
    module->nref = 0;

    /* allocate module space */
    module->mem_space = ka_malloc(module_size);
    if (module->mem_space == NULL)
    {
        ka_printf("Module: allocate space failed.\n");
        return -RT_ERROR;
    }
    module->mem_size = module_size;

    /* zero all space */
    ka_memset(module->mem_space, 0, module_size);
    for (index = 0; index < elf_module->e_phnum; index++)
    {
        if (phdr[index].p_type == PT_LOAD)
        {
            ka_memcpy(module->mem_space + phdr[index].p_vaddr - vstart_addr,
                      (UINT8 *)elf_module + phdr[index].p_offset,
                      phdr[index].p_filesz);
        }
    }

    /* set module entry */
    module->entry_addr = module->mem_space + elf_module->e_entry - vstart_addr;
    ka_printf("1\n");
    /* handle relocation section */
    for (index = 0; index < elf_module->e_shnum; index ++)
    {
        UINT32 i, nr_reloc;
        Elf32_Sym *symtab;
        Elf32_Rel *rel;
        UINT8 *strtab;
        static rt_bool_t unsolved = RT_FALSE;

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

            ka_printf("relocate symbol %s shndx %d", strtab + sym->st_name, sym->st_shndx);

            if ((sym->st_shndx != SHT_NULL) ||(ELF_ST_BIND(sym->st_info) == STB_LOCAL))
            {
                Elf32_Addr addr;

                addr = (Elf32_Addr)(module->mem_space + sym->st_value - vstart_addr);
                dlmodule_relocate(module, rel, addr);
            }
            else if (!linked)
            {
                Elf32_Addr addr;

                ka_printf("relocate symbol: %s", strtab + sym->st_name);
                /* need to resolve symbol in kernel symbol table */
                addr = get_export_function_addr((const char *)(strtab + sym->st_name));
                if (addr == 0)
                {
                    ka_printf("Module: can't find %s in kernel symbol table", strtab + sym->st_name);
                    unsolved = RT_TRUE;
                }
                else
                {
                    dlmodule_relocate(module, rel, addr);
                }
            }
            rel ++;
        }

        if (unsolved) 
            return -RT_ERROR;
    }
    ka_printf("2\n");
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
    ka_printf("3\n");
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
                (ELF_ST_TYPE(symtab[i].st_info) == STT_FUNC))
                count ++;
        }

        module->symtab = (struct rt_module_symtab *)ka_malloc
                         (count * sizeof(struct rt_module_symtab));
        module->nsym = count;
        for (i = 0, count = 0; i < shdr[index].sh_size / sizeof(Elf32_Sym); i++)
        {
            unsigned long length;

            if ((ELF_ST_BIND(symtab[i].st_info) != STB_GLOBAL) ||
                (ELF_ST_TYPE(symtab[i].st_info) != STT_FUNC))
                continue;

            length = ka_strlen((const char *)(strtab + symtab[i].st_name)) + 1;

            module->symtab[count].addr =
                (void *)(module->mem_space + symtab[i].st_value - module->vstart_addr);
            module->symtab[count].name = ka_malloc(length);
            ka_memset((void *)module->symtab[count].name, 0, length);
            ka_memcpy((void *)module->symtab[count].name,
                      strtab + symtab[i].st_name,
                      length);
            count ++;
        }
    }

    return RT_EOK;
}

rt_err_t dlmodule_load_relocated_object(struct rt_dlmodule* module, void *module_ptr)
{
    UINT32 index, rodata_addr = 0, bss_addr = 0, data_addr = 0;
    UINT32 module_addr = 0, module_size = 0;
    UINT8 *ptr, *strtab, *shstrab;

    /* get the ELF image size */
    for (index = 0; index < elf_module->e_shnum; index ++)
    {
        /* text */
        if (IS_PROG(shdr[index]) && IS_AX(shdr[index]))
        {
            module_size += shdr[index].sh_size;
            module_addr = shdr[index].sh_addr;
        }
        /* rodata */
        if (IS_PROG(shdr[index]) && IS_ALLOC(shdr[index]))
        {
            module_size += shdr[index].sh_size;
        }
        /* data */
        if (IS_PROG(shdr[index]) && IS_AW(shdr[index]))
        {
            module_size += shdr[index].sh_size;
        }
        /* bss */
        if (IS_NOPROG(shdr[index]) && IS_AW(shdr[index]))
        {
            module_size += shdr[index].sh_size;
        }
    }

    /* no text, data and bss on image */
    if (module_size == 0) return NULL;

    module->vstart_addr = 0;

    /* allocate module space */
    ka_printf("module size is %u\n",module_size);
    module->mem_space = ka_malloc(module_size);
    if (module->mem_space == NULL)
    {
        ka_printf("Module: allocate space failed.\n");
        return -RT_ERROR;
    }
    module->mem_size = module_size;
    ka_printf("mem_space is %p\n",module->mem_space);

    /* zero all space */
    ptr = module->mem_space;
    ka_memset(ptr, 0, module_size);

    /* load text and data section */
    for (index = 0; index < elf_module->e_shnum; index ++)
    {
        /* load text section */
        if (IS_PROG(shdr[index]) && IS_AX(shdr[index]))
        {
            ka_memcpy(ptr,
                      (UINT8 *)elf_module + shdr[index].sh_offset,
                      shdr[index].sh_size);
            ka_printf("load text 0x%x, size %d\n", ptr, shdr[index].sh_size);
            ptr += shdr[index].sh_size;
        }

        /* load rodata section */
        if (IS_PROG(shdr[index]) && IS_ALLOC(shdr[index]))
        {
            ka_memcpy(ptr,
                      (UINT8 *)elf_module + shdr[index].sh_offset,
                      shdr[index].sh_size);
            rodata_addr = (UINT32)ptr;
            ka_printf("load rodata 0x%x, size %d, rodata 0x%x\n", ptr,
                      shdr[index].sh_size, *(UINT32 *)data_addr);
            ptr += shdr[index].sh_size;
        }

        /* load data section */
        if (IS_PROG(shdr[index]) && IS_AW(shdr[index]))
        {
            ka_memcpy(ptr,
                      (UINT8 *)elf_module + shdr[index].sh_offset,
                      shdr[index].sh_size);
            data_addr = (UINT32)ptr;
            ka_printf("load data 0x%x, size %d, data 0x%x\n", ptr,
                      shdr[index].sh_size, *(UINT32 *)data_addr);
            ptr += shdr[index].sh_size;
        }

        /* load bss section */
        if (IS_NOPROG(shdr[index]) && IS_AW(shdr[index]))
        {
            ka_memset(ptr, 0, shdr[index].sh_size);
            bss_addr = (UINT32)ptr;
            ka_printf("load bss 0x%x, size %d\n", ptr, shdr[index].sh_size);
        }
    }

    /* set module entry */
    module->entry_addr = (rt_dlmodule_entry_func_t)((UINT8 *)module->mem_space + elf_module->e_entry - module_addr);
    ka_printf("module entry point is %p\n",module->entry_addr);
    /* handle relocation section */
    for (index = 0; index < elf_module->e_shnum; index ++) //number of section headers
    {
        UINT32 i, nr_reloc;
        Elf32_Sym *symtab;
        Elf32_Rel *rel;

        if (!IS_REL(shdr[index]))
        {
            ka_printf("type is %u\n",shdr[index]);
            continue;
        }

        /* get relocate item */
        rel = (Elf32_Rel *)((UINT8 *)module_ptr + shdr[index].sh_offset);

        /* locate .dynsym and .dynstr */
        symtab   = (Elf32_Sym *)((UINT8 *)module_ptr +
                                 shdr[shdr[index].sh_link].sh_offset);
        strtab   = (UINT8 *)module_ptr +
                   shdr[shdr[shdr[index].sh_link].sh_link].sh_offset;
        shstrab  = (UINT8 *)module_ptr +
                   shdr[elf_module->e_shstrndx].sh_offset;
        nr_reloc = (UINT32)(shdr[index].sh_size / sizeof(Elf32_Rel));

        /* relocate every items */
        for (i = 0; i < nr_reloc; i ++)
        {
            ka_printf("relocate items round %d\n",i);

            Elf32_Sym *sym = &symtab[ELF32_R_SYM(rel->r_info)];

            ka_printf("relocate symbol: %s\n", strtab + sym->st_name);

            if (sym->st_shndx != STN_UNDEF)  //符号所在段
            {

                ka_printf("!STN_UNDEF\n");

                Elf32_Addr addr = 0;

                if ((ELF_ST_TYPE(sym->st_info) == STT_SECTION) ||
                        (ELF_ST_TYPE(sym->st_info) == STT_OBJECT))
                {
                    if (ka_strncmp((const char *)(shstrab +
                                                  shdr[sym->st_shndx].sh_name), ELF_RODATA, 8) == 0)
                    {
                        /* relocate rodata section */
                        ka_printf("rodata\n");
                        addr = (Elf32_Addr)(rodata_addr + sym->st_value);
                    }
                    else if (ka_strncmp((const char *)
                                        (shstrab + shdr[sym->st_shndx].sh_name), ELF_BSS, 5) == 0)
                    {
                        /* relocate bss section */
                        ka_printf("bss\n");
                        addr = (Elf32_Addr)bss_addr + sym->st_value;
                    }
                    else if (ka_strncmp((const char *)(shstrab + shdr[sym->st_shndx].sh_name),
                                        ELF_DATA, 6) == 0)
                    {
                        /* relocate data section */
                        ka_printf("data\n");
                        addr = (Elf32_Addr)data_addr + sym->st_value;
                    }

                    if (addr != 0) dlmodule_relocate(module, rel, addr);
                }
                else if (ELF_ST_TYPE(sym->st_info) == STT_FUNC)
                {
                    ka_printf("function\n");

                    addr = (Elf32_Addr)((UINT8 *) module->mem_space - module_addr + sym->st_value);

                    /* relocate function */
                    dlmodule_relocate(module, rel, addr);
                }
            }
            else if (ELF_ST_TYPE(sym->st_info) == STT_FUNC)
            {
                ka_printf("STT_FUNC\n");

                /* relocate function */
                dlmodule_relocate(module, rel,
                                  (Elf32_Addr)((UINT8 *)
                                               module->mem_space
                                               - module_addr
                                               + sym->st_value));
            }
            else
            {
                ka_printf("other\n");

                Elf32_Addr addr = NULL;

                if (ELF32_R_TYPE(rel->r_info) != R_ARM_V4BX)
                {
                    ka_printf("relocate symbol: %s\n", strtab + sym->st_name);

                    /* need to resolve symbol in kernel symbol table */
                    addr = get_export_function_addr((const char *)(strtab + sym->st_name));
                    ka_printf("get addr is %p\n",addr);
                    if (addr != (Elf32_Addr)NULL)
                    {
                        dlmodule_relocate(module, rel, addr);
                        ka_printf("symbol addr 0x%x\n", addr);
                    }
                    else
                        ka_printf("Module: can't find %s in kernel symbol table\n",
                                  strtab + sym->st_name);
                }
                else
                {
                    addr = (Elf32_Addr)((UINT8 *) module->mem_space - module_addr + sym->st_value);
                    dlmodule_relocate(module, rel, addr);
                }
            }

            rel ++;
        }
    }

    return RT_EOK;
}



struct rt_dlmodule* dlmodule_load(void)
{
    rt_err_t ret = RT_EOK;
    UINT8 *module_ptr = (UINT8 *)module_buffer;
    struct rt_dlmodule *module = NULL;

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

    module = ka_malloc(sizeof(struct rt_dlmodule));
    if (!module) goto __exit;
    module->stat = RT_DLMODULE_STAT_INIT;

    /* set initial priority and stack size */
    module->priority = PRIO_MAX - 10;
    module->stack_size = 2048;

    INIT_LIST_HEAD(&(module->object_list));

    /* set the name of module */
    //_dlmodule_set_name(module, filename);

    if (elf_module->e_type == ET_REL)
    {
        ka_printf("load relocated file\n");
        ret = dlmodule_load_relocated_object(module, module_ptr);
    }
    /*else if (elf_module->e_type == ET_DYN)
    {
        ka_printf("load shared file\n");
        ret = dlmodule_load_shared_object(module, module_ptr);
    }*/
    else
    {
        ka_printf("Module: unsupported elf type\n");
        goto __exit;
    }

    /* check return value */
    if (ret != RT_EOK) goto __exit;
    ka_printf("4\n");
    /* release module data */
    ka_free(module_ptr);
    ka_printf("5\n");
    /* increase module reference count */
    module->nref ++;

    /* deal with cache */
//#ifdef RT_USING_CACHE
//    rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, module->mem_space, module->mem_size);
//    rt_hw_cpu_icache_ops(RT_HW_CACHE_INVALIDATE, module->mem_space, module->mem_size);
//#endif

    /* set module initialization and cleanup function */
    //module->init_func = dlsym(module, "module_init");
    //module->cleanup_func = dlsym(module, "module_cleanup");
    module->init_func = NULL;
    module->cleanup_func = NULL;
    module->stat = RT_DLMODULE_STAT_INIT;
    /* do module initialization */
    if (module->init_func)
    {
        module->init_func(module);
    }

    return module;

__exit:
    ka_printf("6\n");
    if (module_ptr) ka_free(module_ptr);
		
    //if (module) dlmodule_destroy(module);
		if(module) {ASSERT(0);}
		
    return NULL;
}


struct rt_dlmodule* dlmodule_exec(void)
{
    struct rt_dlmodule *module = NULL;
    int error;

    module = dlmodule_load();
    if (module)
    {
        if (module->entry_addr)
        {

            /* check stack size and priority */
            if (module->priority > PRIO_MAX) module->priority = PRIO_MAX - 1;
            if (module->stack_size < 2048 || module->stack_size > (1024 * 32)) module->stack_size = 2048;

            error = task_creat_ready(module->stack_size, module->priority, HZ / 10, "module_thread",
                                     (functionptr)(module->entry_addr), NULL, NULL);
            if (!error)
            {
                ka_printf("module_thread run\n");
            }
            else
            {
                ka_printf("module_thread fail,error code is %d\n",error);
                module = NULL;
            }
        }
    }

    return module;
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
