/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright 1996-1998 John D. Polstra.
 * All rights reserved.
 *
 * Mach-O support Copyright (C) 2021 by Zoe Knox <zoe@pixin.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <sys/param.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "rtld.h"

#include <sys/mach-o/loader.h>
#include <sys/mach-o/fat.h>

static Elf_Ehdr *get_elf_header(int, const char *, const struct stat *,
    Elf_Phdr **phdr);
static struct mach_header_64 *get_macho_header(int, const char *, const struct stat *);
static Obj_Entry *map_macho_object(int fd, const char *path, const struct stat *sb, struct mach_header_64 *hdr);
static int convert_flags(int); /* Elf flags -> mmap flags */
static int convert_macho_flags(int flags); /* MachO flags -> mmap flags */

extern const char *_progname_so;
const char *_progname_so = "/usr/lib/progname.so"; 

int __getosreldate(void);

static bool
phdr_in_zero_page(const Elf_Ehdr *hdr)
{
	return (hdr->e_phoff + hdr->e_phnum * sizeof(Elf_Phdr) <=
	    (size_t)PAGE_SIZE);
}

/*
 * Map a shared object into memory.  The "fd" argument is a file descriptor,
 * which must be open on the object and positioned at its beginning.
 * The "path" argument is a pathname that is used only for error messages.
 *
 * The return value is a pointer to a newly-allocated Obj_Entry structure
 * for the shared object.  Returns NULL on failure.
 */
Obj_Entry *
map_object(int fd, const char *path, const struct stat *sb)
{
    Obj_Entry *obj;
    Elf_Ehdr *hdr;
    int i;
    Elf_Phdr *phdr;
    Elf_Phdr *phlimit;
    Elf_Phdr **segs;
    int nsegs;
    Elf_Phdr *phdyn;
    Elf_Phdr *phinterp;
    Elf_Phdr *phtls;
    caddr_t mapbase;
    size_t mapsize;
    Elf_Addr base_vaddr;
    Elf_Addr base_vlimit;
    caddr_t base_addr;
    int base_flags;
    Elf_Off data_offset;
    Elf_Addr data_vaddr;
    Elf_Addr data_vlimit;
    caddr_t data_addr;
    int data_prot;
    int data_flags;
    Elf_Addr clear_vaddr;
    caddr_t clear_addr;
    caddr_t clear_page;
    Elf_Addr phdr_vaddr;
    size_t nclear, phsize;
    Elf_Addr bss_vaddr;
    Elf_Addr bss_vlimit;
    caddr_t bss_addr;
    Elf_Word stack_flags;
    Elf_Addr relro_page;
    size_t relro_size;
    Elf_Addr note_start;
    Elf_Addr note_end;
    char *note_map;
    size_t note_map_len;

    struct mach_header_64 *mhdr = NULL;

    hdr = get_elf_header(fd, path, sb, &phdr);
    if (hdr == NULL)
        mhdr = get_macho_header(fd, path, sb);
    if(hdr == NULL && mhdr == NULL)
	return (NULL);
    if(mhdr != NULL)
        return map_macho_object(fd, path, sb, mhdr);

    /*
     * Scan the program header entries, and save key information.
     * We expect that the loadable segments are ordered by load address.
     */
    phsize  = hdr->e_phnum * sizeof(phdr[0]);
    phlimit = phdr + hdr->e_phnum;
    nsegs = -1;
    phdyn = phinterp = phtls = NULL;
    phdr_vaddr = 0;
    relro_page = 0;
    relro_size = 0;
    note_start = 0;
    note_end = 0;
    note_map = NULL;
    note_map_len = 0;
    segs = alloca(sizeof(segs[0]) * hdr->e_phnum);
    stack_flags = RTLD_DEFAULT_STACK_PF_EXEC | PF_R | PF_W;
    while (phdr < phlimit) {
	switch (phdr->p_type) {

	case PT_INTERP:
	    phinterp = phdr;
	    break;

	case PT_LOAD:
	    segs[++nsegs] = phdr;
    	    if ((segs[nsegs]->p_align & (PAGE_SIZE - 1)) != 0) {
		_rtld_error("%s: PT_LOAD segment %d not page-aligned",
		    path, nsegs);
		goto error;
	    }
	    break;

	case PT_PHDR:
	    phdr_vaddr = phdr->p_vaddr;
	    phsize = phdr->p_memsz;
	    break;

	case PT_DYNAMIC:
	    phdyn = phdr;
	    break;

	case PT_TLS:
	    phtls = phdr;
	    break;

	case PT_GNU_STACK:
	    stack_flags = phdr->p_flags;
	    break;

	case PT_GNU_RELRO:
	    relro_page = phdr->p_vaddr;
	    relro_size = phdr->p_memsz;
	    break;

	case PT_NOTE:
	    if (phdr->p_offset > PAGE_SIZE ||
	      phdr->p_offset + phdr->p_filesz > PAGE_SIZE) {
		note_map_len = round_page(phdr->p_offset +
		  phdr->p_filesz) - trunc_page(phdr->p_offset);
		note_map = mmap(NULL, note_map_len, PROT_READ,
		  MAP_PRIVATE, fd, trunc_page(phdr->p_offset));
		if (note_map == MAP_FAILED) {
		    _rtld_error("%s: error mapping PT_NOTE (%d)", path, errno);
		    goto error;
		}
		note_start = (Elf_Addr)(note_map + phdr->p_offset -
		  trunc_page(phdr->p_offset));
	    } else {
		note_start = (Elf_Addr)(char *)hdr + phdr->p_offset;
	    }
	    note_end = note_start + phdr->p_filesz;
	    break;
	}

	++phdr;
    }
    if (phdyn == NULL) {
	_rtld_error("%s: object is not dynamically-linked", path);
	goto error;
    }

    if (nsegs < 0) {
	_rtld_error("%s: too few PT_LOAD segments", path);
	goto error;
    }

    /*
     * Map the entire address space of the object, to stake out our
     * contiguous region, and to establish the base address for relocation.
     */
    base_vaddr = trunc_page(segs[0]->p_vaddr);
    base_vlimit = round_page(segs[nsegs]->p_vaddr + segs[nsegs]->p_memsz);
    mapsize = base_vlimit - base_vaddr;
    base_addr = (caddr_t) base_vaddr;
    base_flags = __getosreldate() >= P_OSREL_MAP_GUARD ? MAP_GUARD :
	MAP_PRIVATE | MAP_ANON | MAP_NOCORE;
    if (npagesizes > 1 && round_page(segs[0]->p_filesz) >= pagesizes[1])
	base_flags |= MAP_ALIGNED_SUPER;
    if (base_vaddr != 0)
	base_flags |= MAP_FIXED | MAP_EXCL;

    mapbase = mmap(base_addr, mapsize, PROT_NONE, base_flags, -1, 0);
    if (mapbase == MAP_FAILED) {
	_rtld_error("%s: mmap of entire address space failed: %s",
	  path, rtld_strerror(errno));
	goto error;
    }
    if (base_addr != NULL && mapbase != base_addr) {
	_rtld_error("%s: mmap returned wrong address: wanted %p, got %p",
	  path, base_addr, mapbase);
	goto error1;
    }

    for (i = 0; i <= nsegs; i++) {
	/* Overlay the segment onto the proper region. */
	data_offset = trunc_page(segs[i]->p_offset);
	data_vaddr = trunc_page(segs[i]->p_vaddr);
	data_vlimit = round_page(segs[i]->p_vaddr + segs[i]->p_filesz);
	data_addr = mapbase + (data_vaddr - base_vaddr);
	data_prot = convert_prot(segs[i]->p_flags);
	data_flags = convert_flags(segs[i]->p_flags) | MAP_FIXED;
	if (mmap(data_addr, data_vlimit - data_vaddr, data_prot,
	  data_flags | MAP_PREFAULT_READ, fd, data_offset) == (caddr_t) -1) {
	    _rtld_error("%s: mmap of data failed: %s", path,
		rtld_strerror(errno));
	    goto error1;
	}

	/* Do BSS setup */
	if (segs[i]->p_filesz != segs[i]->p_memsz) {

	    /* Clear any BSS in the last page of the segment. */
	    clear_vaddr = segs[i]->p_vaddr + segs[i]->p_filesz;
	    clear_addr = mapbase + (clear_vaddr - base_vaddr);
	    clear_page = mapbase + (trunc_page(clear_vaddr) - base_vaddr);

	    if ((nclear = data_vlimit - clear_vaddr) > 0) {
		/* Make sure the end of the segment is writable */
		if ((data_prot & PROT_WRITE) == 0 && -1 ==
		     mprotect(clear_page, PAGE_SIZE, data_prot|PROT_WRITE)) {
			_rtld_error("%s: mprotect failed: %s", path,
			    rtld_strerror(errno));
			goto error1;
		}

		memset(clear_addr, 0, nclear);

		/* Reset the data protection back */
		if ((data_prot & PROT_WRITE) == 0)
		    mprotect(clear_page, PAGE_SIZE, data_prot);
	    }

	    /* Overlay the BSS segment onto the proper region. */
	    bss_vaddr = data_vlimit;
	    bss_vlimit = round_page(segs[i]->p_vaddr + segs[i]->p_memsz);
	    bss_addr = mapbase +  (bss_vaddr - base_vaddr);
	    if (bss_vlimit > bss_vaddr) {	/* There is something to do */
		if (mmap(bss_addr, bss_vlimit - bss_vaddr, data_prot,
		    data_flags | MAP_ANON, -1, 0) == MAP_FAILED) {
		    _rtld_error("%s: mmap of bss failed: %s", path,
			rtld_strerror(errno));
		    goto error1;
		}
	    }
	}

	if (phdr_vaddr == 0 && data_offset <= hdr->e_phoff &&
	  (data_vlimit - data_vaddr + data_offset) >=
	  (hdr->e_phoff + hdr->e_phnum * sizeof (Elf_Phdr))) {
	    phdr_vaddr = data_vaddr + hdr->e_phoff - data_offset;
	}
    }

    obj = obj_new();
    obj->macho = 0;
    if (sb != NULL) {
	obj->dev = sb->st_dev;
	obj->ino = sb->st_ino;
    }
    obj->mapbase = mapbase;
    obj->mapsize = mapsize;
    obj->textsize = round_page(segs[0]->p_vaddr + segs[0]->p_memsz) -
      base_vaddr;
    obj->vaddrbase = base_vaddr;
    obj->relocbase = mapbase - base_vaddr;
    obj->dynamic = (const Elf_Dyn *)(obj->relocbase + phdyn->p_vaddr);
    if (hdr->e_entry != 0)
	obj->entry = (caddr_t)(obj->relocbase + hdr->e_entry);
    if (phdr_vaddr != 0) {
	obj->phdr = (const Elf_Phdr *)(obj->relocbase + phdr_vaddr);
    } else {
	obj->phdr = malloc(phsize);
	if (obj->phdr == NULL) {
	    obj_free(obj);
	    _rtld_error("%s: cannot allocate program header", path);
	    goto error1;
	}
	memcpy(__DECONST(char *, obj->phdr), (char *)hdr + hdr->e_phoff, phsize);
	obj->phdr_alloc = true;
    }
    obj->phsize = phsize;
    if (phinterp != NULL)
	obj->interp = (const char *)(obj->relocbase + phinterp->p_vaddr);
    if (phtls != NULL) {
	tls_dtv_generation++;
	obj->tlsindex = ++tls_max_index;
	obj->tlssize = phtls->p_memsz;
	obj->tlsalign = phtls->p_align;
	obj->tlspoffset = phtls->p_offset;
	obj->tlsinitsize = phtls->p_filesz;
	obj->tlsinit = mapbase + phtls->p_vaddr;
    }
    obj->stack_flags = stack_flags;
    obj->relro_page = obj->relocbase + trunc_page(relro_page);
    obj->relro_size = round_page(relro_size);
    if (note_start < note_end)
	digest_notes(obj, note_start, note_end);
    if (note_map != NULL)
	munmap(note_map, note_map_len);
    munmap(hdr, PAGE_SIZE);
    return (obj);

error1:
    munmap(mapbase, mapsize);
error:
    if (note_map != NULL && note_map != MAP_FAILED)
	munmap(note_map, note_map_len);
    if (!phdr_in_zero_page(hdr))
	munmap(phdr, hdr->e_phnum * sizeof(phdr[0]));
    munmap(hdr, PAGE_SIZE);
    return (NULL);
}

/* Map a MachO object into memory. Returns an Obj_Entry pointer for
 * the shared object or NULL on failure. This is called from map_object()
 * when it detects that the fd or path is a MachO object instead of ELF
 */
static Obj_Entry *map_macho_object(int fd, const char *path, const struct stat *sb, struct mach_header_64 *hdr) {
    int i, nsegs = -1;
    Obj_Entry *obj;
    struct load_command *lc;
    struct segment_command_64 *ls;
    struct entry_point_command *lm;
    struct dylinker_command *ldy;
    struct dylib_command *ld;
    char *dylinker = NULL;
    struct segment_command_64 *segs[16]; // I think 16 is the max
    Needed_Entry **needed_tail;
    struct dyld_info_command *ldi = NULL;
    Elf_Addr offset;
    caddr_t mapbase;
    size_t mapsize;
    Elf_Addr base_vaddr;
    Elf_Addr base_vlimit;
    Elf_Word stack_flags;
    caddr_t base_addr;
    int base_flags;
    Elf_Off data_offset;
    Elf_Addr data_vaddr;
    Elf_Addr data_vlimit;
    caddr_t data_addr;
    int data_prot;
    int data_flags;
    Elf_Addr main_addr = 0;
    int textseg = -1;
    int lowseg = -1; // first segment which is not __PAGEZERO

    // We already mmapped 4*PAGE_SIZE bytes of the file into *hdr
    // so we can read the commands unless they exceed 4*PAGE_SIZE
    offset = (ptrdiff_t)hdr;
    offset += sizeof(struct mach_header_64);

    if(hdr->sizeofcmds + sizeof(struct mach_header_64) > 4*PAGE_SIZE) {
        _rtld_error("%s: commands exceed 4*PAGE_SIZE (%d) bytes", path, 4*PAGE_SIZE);
        return NULL;
    }

    stack_flags = RTLD_DEFAULT_STACK_PF_EXEC | PF_R | PF_W;

    obj = obj_new();
    obj->macho = 1;
    needed_tail = &obj->needed;

    for(i = 0; i < (int)hdr->ncmds; i++) {
        lc = (struct load_command *)offset;
        uint32_t cmd = lc->cmd;
        //dbg("%d. lc.cmd %x lc.cmdsize %d", i, cmd, lc->cmdsize);
        switch(cmd) {
            case LC_SEGMENT_64:
                ls = (struct segment_command_64 *)lc;
                dbg("LC_SEGMENT_64 %s vmaddr %lx size %lx fileoff %lx size %lx",ls->segname,
                    ls->vmaddr, ls->vmsize, ls->fileoff, ls->filesize);
                segs[++nsegs] = ls;
                if(!strcmp(SEG_TEXT, ls->segname))
                    textseg = i;
                if(lowseg < 0 && strcmp(SEG_PAGEZERO, ls->segname))
                    lowseg = i;

		unsigned int x = sizeof(struct segment_command_64);
		while(ls->nsects && x < ls->cmdsize) {
		    struct section_64 *sect = (struct section_64 *)(offset+x);
		    dbg("  section %s addr %lx size %lx offset %x res1 %x",
		    	sect->sectname, sect->addr, sect->size, sect->offset,
			sect->reserved1);
		    if(!strcmp(SECT_TEXT, sect->sectname) &&
			!strcmp(SEG_TEXT, ls->segname) && main_addr == 0)
		    {
			unsigned long addr = sect->offset;
			main_addr = (uint64_t)addr;
		    }
		    if(!strcmp("__la_symbol_ptr", sect->sectname)) {
		    	unsigned long addr = sect->offset;
			obj->macho_la_syms = (uint64_t *)addr;
		    }
		    if(!strcmp("__got", sect->sectname)
			&& (!strcmp("__DATA_CONST", ls->segname)
			|| !strcmp(SEG_DATA, ls->segname)))
		    {
		    	unsigned long addr = sect->offset;
			obj->pltgot = (Elf_Addr *)addr;
		    }
		    x += sizeof(struct section_64);
		}
                break;
            case LC_MAIN:
                lm = (struct entry_point_command *)lc;
                dbg("LC_MAIN entry point %lx stacksize %lx",lm->entryoff,lm->stacksize);
                main_addr = lm->entryoff;
                break;
	    case LC_LOAD_DYLINKER:
		ldy = (struct dylinker_command *)lc;
		dylinker = (char *)lc + ldy->name.offset;
		dbg("LC_LOAD_DYLINKER name=%s", dylinker);
		break;
	    case LC_LOAD_DYLIB:
		ld = (struct dylib_command *)lc;
		char *p = (char *)lc + ld->dylib.name.offset;
		dbg("LC_LOAD_DYLIB %s cur ver = %08x compat ver = %08x",p,
			ld->dylib.current_version, ld->dylib.compatibility_version);
		if (!obj->rtld) {
		    Needed_Entry *nep = NEW(Needed_Entry);
		    nep->name = offset + ld->dylib.name.offset - (uint64_t)hdr;
		    nep->obj = NULL;
		    nep->next = NULL;

		    *needed_tail = nep;
		    needed_tail = &nep->next;
		}
		break;
	    case LC_SYMTAB:
	    {
	    	struct symtab_command *sym = (struct symtab_command *)lc;
		dbg("LC_SYMTAB sym off %x count %x string offset %x size %x",
		    sym->symoff, sym->nsyms, sym->stroff, sym->strsize);
		unsigned long addr = sym->stroff;
		obj->strtab = (const char *)addr;
		obj->strsize = sym->strsize;
		addr = sym->symoff;
		obj->symtab = (Elf_Sym *)addr;
		obj->symcount = sym->nsyms;
		break;
	    }
	    case LC_DYSYMTAB:
	    {
		struct dysymtab_command *sym = (struct dysymtab_command *)lc;
	    	dbg("LC_DYSYMTAB indirect offset %x count %x", sym->indirectsymoff,
		    sym->nindirectsyms);
		obj->dynsymcount = sym->nindirectsyms;
		break;
	    }
	    case LC_DYLD_INFO_ONLY:
	    	ldi = (struct dyld_info_command *)lc;
	    	dbg("LC_DYLD_INFO_ONLY");
		obj->dynamic = (Elf_Dyn *)(offset - (uint64_t)hdr);
		break;
        }
        offset += lc->cmdsize;
    }

    if(hdr->filetype == MH_EXECUTE && textseg < 0) {
        _rtld_error("%s: no text segment found", path);
	obj_free(obj);
        goto error;
    }

    /*
     * Map the entire address space of the object, to stake out our
     * contiguous region, and to establish the base address for relocation.
     */
    base_vaddr = trunc_page(segs[0]->vmaddr);
    base_vlimit = round_page(segs[nsegs]->vmaddr + segs[nsegs]->vmsize);
    mapsize = base_vlimit - base_vaddr - segs[0]->vmsize + PAGE_SIZE;
    base_addr = (caddr_t) base_vaddr;

    base_flags = __getosreldate() >= P_OSREL_MAP_GUARD ? MAP_GUARD :
	MAP_PRIVATE | MAP_ANON | MAP_NOCORE;
    if (npagesizes > 1 && round_page(segs[0]->filesize) >= pagesizes[1])
	base_flags |= MAP_ALIGNED_SUPER;
    if (base_vaddr != 0)
	base_flags |= MAP_FIXED | MAP_EXCL;

    mapbase = mmap(base_addr, mapsize, PROT_NONE, base_flags, -1, 0);
    if (mapbase == MAP_FAILED) {
	_rtld_error("%s: mmap of entire address space failed: %s",
	  path, rtld_strerror(errno));
	obj_free(obj);
	goto error;
    }
    if (base_addr != NULL && mapbase != base_addr) {
	_rtld_error("%s: mmap returned wrong address: wanted %p, got %p",
	  path, base_addr, mapbase);
	obj_free(obj);
	goto error1;
    }

    obj->nsegs = nsegs;
    for (i = 0; i <= nsegs; i++) {
        if(!strcmp(segs[i]->segname, SEG_PAGEZERO))
            continue;
	/* Overlay the segment onto the proper region. */
	data_offset = trunc_page(segs[i]->fileoff);
	data_vaddr = trunc_page(segs[i]->vmaddr - segs[0]->vmsize);
	data_vlimit = round_page(segs[i]->vmaddr - segs[0]->vmsize + segs[i]->filesize);
	data_addr = mapbase + (data_vaddr - base_vaddr);
        dbg("mapbase %p data_vaddr %lx base %lx",mapbase, data_vaddr,base_vaddr);
	data_prot = convert_macho_prot(segs[i]->initprot);
	data_flags = convert_macho_flags(segs[i]->initprot) | MAP_FIXED;
        dbg("Overlaying segment %d @0x%p sz 0x%lx %x %x off 0x%lx %lx", i, data_addr,
            data_vlimit - data_vaddr, data_prot, data_flags, data_offset, segs[i]->fileoff);
	if (mmap(data_addr, data_vlimit - data_vaddr, data_prot,
	  data_flags | MAP_PREFAULT_READ, fd, data_offset) == (caddr_t) -1) {
	    _rtld_error("%s: mmap of data failed: %s", path,
		rtld_strerror(errno));
	    obj_free(obj);
	    goto error1;
	}
	if(!strcmp(SEG_LINKEDIT, segs[i]->segname))
	    obj->linkedit = data_addr;
	obj->segs[i] = data_addr;
#if 0
	/* Do BSS setup */
	if (segs[i]->filesize != segs[i]->vmsize) {

	    /* Clear any BSS in the last page of the segment. */
	    clear_vaddr = segs[i]->vmaddr + segs[i]->filesize;
	    clear_addr = mapbase + (clear_vaddr - base_vaddr);
	    clear_page = mapbase + (trunc_page(clear_vaddr) - base_vaddr);

	    if ((nclear = data_vlimit - clear_vaddr) > 0) {
		/* Make sure the end of the segment is writable */
		if ((data_prot & PROT_WRITE) == 0 && -1 ==
		     mprotect(clear_page, PAGE_SIZE, data_prot|PROT_WRITE)) {
			_rtld_error("%s: mprotect failed: %s", path,
			    rtld_strerror(errno));
			goto error1;
		}

		memset(clear_addr, 0, nclear);

		/* Reset the data protection back */
		if ((data_prot & PROT_WRITE) == 0)
		    mprotect(clear_page, PAGE_SIZE, data_prot);
	    }

	    /* Overlay the BSS segment onto the proper region. */
	    bss_vaddr = data_vlimit;
	    bss_vlimit = round_page(segs[i]->vmaddr + segs[i]->vmsize);
	    bss_addr = mapbase +  (bss_vaddr - base_vaddr);
	    if (bss_vlimit > bss_vaddr) {	/* There is something to do */
		if (mmap(bss_addr, bss_vlimit - bss_vaddr, data_prot,
		    data_flags | MAP_ANON, -1, 0) == MAP_FAILED) {
		    _rtld_error("%s: mmap of bss failed: %s", path,
			rtld_strerror(errno));
		    goto error1;
		}
	    }
	}
#endif
    }
    if (sb != NULL) {
	obj->dev = sb->st_dev;
	obj->ino = sb->st_ino;
    }
    obj->mapbase = mapbase;
    obj->mapsize = mapsize;
    if(textseg >= 0)
        obj->textsize = round_page(segs[textseg]->vmaddr
            + segs[textseg]->vmsize) - base_vaddr - segs[0]->vmsize;
    obj->vaddrbase = base_vaddr;
    obj->relocbase = mapbase - base_vaddr;
    obj->interp = xstrdup(dylinker);

    /* Convert all offsets to actual addresses */
    if(obj->dynamic)
	obj->dynamic = (Elf_Dyn *)(((ptrdiff_t)obj->relocbase)
	    + ((ptrdiff_t)obj->dynamic));
    if(obj->macho_la_syms)
	obj->macho_la_syms = (uint64_t *)(((ptrdiff_t)obj->relocbase) +
	    ((ptrdiff_t)obj->macho_la_syms));
    if(obj->pltgot)
	obj->pltgot = (Elf_Addr *)(((ptrdiff_t)obj->relocbase) +
	    ((ptrdiff_t)obj->pltgot));
    if(obj->symtab)
	obj->symtab = (Elf_Sym *)(((ptrdiff_t)obj->relocbase) +
	    ((ptrdiff_t)obj->symtab));
    if(obj->strtab)
	obj->strtab = (const char *)(((ptrdiff_t)obj->relocbase) +
	    ((ptrdiff_t)obj->strtab));

    Needed_Entry *nep;
    for(nep = needed_tail; nep != NULL; nep = nep->next) {
	nep->name = nep->name + (unsigned long)(obj->relocbase);
    }

    /* use the extra page mapped at the very end to hold a copy
     * of our progname.so string for the Needed_Entry
     */
    char *page = mmap((void *)trunc_page(mapbase + mapsize - PAGE_SIZE), PAGE_SIZE,
    	PROT_READ|PROT_WRITE, MAP_ANON|MAP_FIXED, -1, 0);
    if(page == NULL) {
	_rtld_error("mmap failed");
	rtld_die();
    }
    memset(page, 0, PAGE_SIZE);
    memcpy(page, _progname_so, strlen(_progname_so)); 

    nep = NEW(Needed_Entry);
    nep->name = (unsigned long)(mapsize - PAGE_SIZE);
    nep->obj = NULL;
    nep->next = NULL;
    *needed_tail = nep;
    needed_tail = &nep->next;

    obj->entry = (caddr_t)(obj->relocbase + main_addr);
// FIXME: what should we put here?
//     if (phdr_vaddr != 0) {
// 	obj->phdr = (const Elf_Phdr *)(obj->relocbase + phdr_vaddr);
//     } else {
// 	obj->phdr = malloc(phsize);
// 	if (obj->phdr == NULL) {
// 	    obj_free(obj);
// 	    _rtld_error("%s: cannot allocate program header", path);
// 	    goto error1;
// 	}
// 	memcpy(__DECONST(char *, obj->phdr), (char *)hdr + hdr->e_phoff, phsize);
// 	obj->phdr_alloc = true;
//     }
//     obj->phsize = phsize;
//     if (phtls != NULL) {
// 	tls_dtv_generation++;
// 	obj->tlsindex = ++tls_max_index;
// 	obj->tlssize = phtls->p_memsz;
// 	obj->tlsalign = phtls->p_align;
// 	obj->tlspoffset = phtls->p_offset;
// 	obj->tlsinitsize = phtls->p_filesz;
// 	obj->tlsinit = mapbase + phtls->p_vaddr;
//     }
    obj->stack_flags = stack_flags;
//     obj->relro_page = obj->relocbase + trunc_page(relro_page);
//     obj->relro_size = round_page(relro_size);
//     if (note_start < note_end)
// 	digest_notes(obj, note_start, note_end);
//     if (note_map != NULL)
// 	munmap(note_map, note_map_len);
    munmap(hdr, 4*PAGE_SIZE);
    dbg("%s: base %p sz %lx vbase %lx tsz %lx entry %p reloc %p",
        path, obj->mapbase, obj->mapsize, obj->vaddrbase, obj->textsize,
        obj->entry, obj->relocbase);
    return (obj);

error1:
    munmap(mapbase, mapsize);
error:
    munmap(hdr, 4*PAGE_SIZE);
    return (NULL);
}

static Elf_Ehdr *
get_elf_header(int fd, const char *path, const struct stat *sbp,
    Elf_Phdr **phdr_p)
{
	Elf_Ehdr *hdr;
	Elf_Phdr *phdr;

	/* Make sure file has enough data for the ELF header */
	if (sbp != NULL && sbp->st_size < (off_t)sizeof(Elf_Ehdr)) {
		_rtld_error("%s: invalid file format", path);
		return (NULL);
	}

	hdr = mmap(NULL, PAGE_SIZE, PROT_READ, MAP_PRIVATE | MAP_PREFAULT_READ,
	    fd, 0);
	if (hdr == MAP_FAILED) {
		_rtld_error("%s: read error: %s", path, rtld_strerror(errno));
		return (NULL);
	}

	/* Make sure the file is valid */
	if (!IS_ELF(*hdr)) {
		_rtld_error("%s: invalid file format", path);
		goto error;
	}
	if (hdr->e_ident[EI_CLASS] != ELF_TARG_CLASS ||
	    hdr->e_ident[EI_DATA] != ELF_TARG_DATA) {
		_rtld_error("%s: unsupported file layout", path);
		goto error;
	}
	if (hdr->e_ident[EI_VERSION] != EV_CURRENT ||
	    hdr->e_version != EV_CURRENT) {
		_rtld_error("%s: unsupported file version", path);
		goto error;
	}
	if (hdr->e_type != ET_EXEC && hdr->e_type != ET_DYN) {
		_rtld_error("%s: unsupported file type", path);
		goto error;
	}
	if (hdr->e_machine != ELF_TARG_MACH) {
		_rtld_error("%s: unsupported machine", path);
		goto error;
	}

	/*
	 * We rely on the program header being in the first page.  This is
	 * not strictly required by the ABI specification, but it seems to
	 * always true in practice.  And, it simplifies things considerably.
	 */
	if (hdr->e_phentsize != sizeof(Elf_Phdr)) {
		_rtld_error(
	    "%s: invalid shared object: e_phentsize != sizeof(Elf_Phdr)", path);
		goto error;
	}
	if (phdr_in_zero_page(hdr)) {
		phdr = (Elf_Phdr *)((char *)hdr + hdr->e_phoff);
	} else {
		phdr = mmap(NULL, hdr->e_phnum * sizeof(phdr[0]),
		    PROT_READ, MAP_PRIVATE | MAP_PREFAULT_READ, fd,
		    hdr->e_phoff);
		if (phdr == MAP_FAILED) {
			_rtld_error("%s: error mapping phdr: %s", path,
			    rtld_strerror(errno));
			goto error;
		}
	}
	*phdr_p = phdr;
	return (hdr);

error:
	munmap(hdr, PAGE_SIZE);
	return (NULL);
}

static bool is_macho_header(struct mach_header_64 *hdr)
{
    if(hdr->magic != MH_MAGIC_64 && hdr->magic != FAT_MAGIC_64)
        return false;

    dbg("MachO object\nhdr.magic 0x%x", hdr->magic);
    //dbg("hdr.cputype %x", hdr->cputype);
    //dbg("hdr.cpusubtype 0x%x", hdr->cpusubtype);
    //dbg("hdr.filetype 0x%x", hdr->filetype);
    //dbg("hdr.ncmds %d", hdr->ncmds);
    //dbg("hdr.sizeofcmds %d", hdr->sizeofcmds);
    //dbg("hdr.flags 0x%x", hdr->flags);
    return true;
}

static struct mach_header_64 *get_macho_header(int fd, const char *path, const struct stat *sbp)
{
	struct mach_header_64 *hdr;

	/* Make sure file has enough data for the Mach header */
	if (sbp != NULL && sbp->st_size < (off_t)sizeof(struct mach_header_64)) {
		_rtld_error("%s: invalid MachO format", path);
		return (NULL);
	}

	hdr = mmap(NULL, 4*PAGE_SIZE, PROT_READ, MAP_PRIVATE | MAP_PREFAULT_READ,
	    fd, 0);
	if (hdr == MAP_FAILED) {
		_rtld_error("%s: read error: %s", path, rtld_strerror(errno));
		return (NULL);
	}

	/* Make sure the file is valid */
	if (!is_macho_header(hdr)) {
		_rtld_error("%s: invalid MachO format", path);
		goto error;
	}

	switch(hdr->filetype) {
            case MH_BUNDLE:
            case MH_PRELOAD:
            case MH_EXECUTE:
            case MH_DYLINKER:
            case MH_DYLIB: break;
            default: _rtld_error("%s: unsupported file type", path);
                     goto error;
	}

        if(hdr->cputype != CPU_TYPE_X86_64
            && hdr->cputype != (unsigned)CPU_TYPE_ANY) {
                _rtld_error("%s: unsupported machine", path);
                goto error;
        }

	return (hdr);

error:
	munmap(hdr, 4*PAGE_SIZE);
	return (NULL);
}

void
obj_free(Obj_Entry *obj)
{
    Objlist_Entry *elm;

    if (obj->tls_done)
	free_tls_offset(obj);
    while (obj->needed != NULL) {
	Needed_Entry *needed = obj->needed;
	obj->needed = needed->next;
	free(needed);
    }
    while (!STAILQ_EMPTY(&obj->names)) {
	Name_Entry *entry = STAILQ_FIRST(&obj->names);
	STAILQ_REMOVE_HEAD(&obj->names, link);
	free(entry);
    }
    while (!STAILQ_EMPTY(&obj->dldags)) {
	elm = STAILQ_FIRST(&obj->dldags);
	STAILQ_REMOVE_HEAD(&obj->dldags, link);
	free(elm);
    }
    while (!STAILQ_EMPTY(&obj->dagmembers)) {
	elm = STAILQ_FIRST(&obj->dagmembers);
	STAILQ_REMOVE_HEAD(&obj->dagmembers, link);
	free(elm);
    }
    if (obj->vertab)
	free(obj->vertab);
    if (obj->origin_path)
	free(obj->origin_path);
    if (obj->z_origin)
	free(__DECONST(void*, obj->rpath));
    if (obj->priv)
	free(obj->priv);
    if (obj->path)
	free(obj->path);
    if (obj->phdr_alloc)
	free(__DECONST(void *, obj->phdr));
    free(obj);
}

Obj_Entry *
obj_new(void)
{
    Obj_Entry *obj;

    obj = CNEW(Obj_Entry);
    STAILQ_INIT(&obj->dldags);
    STAILQ_INIT(&obj->dagmembers);
    STAILQ_INIT(&obj->names);
    return obj;
}

/*
 * Given a set of ELF protection flags, return the corresponding protection
 * flags for MMAP.
 */
int
convert_prot(int elfflags)
{
    int prot = 0;
    if (elfflags & PF_R)
	prot |= PROT_READ;
    if (elfflags & PF_W)
	prot |= PROT_WRITE;
    if (elfflags & PF_X)
	prot |= PROT_EXEC;
    return prot;
}

int convert_macho_prot(int flags) {
    int prot = 0;
    if(flags & VM_PROT_READ) prot |= PROT_READ;
    if(flags & VM_PROT_WRITE) prot |= PROT_WRITE;
    if(flags & VM_PROT_EXECUTE) prot |= PROT_EXEC;
    return prot;
}

static int
convert_flags(int elfflags)
{
    int flags = MAP_PRIVATE; /* All mappings are private */

    /*
     * Readonly mappings are marked "MAP_NOCORE", because they can be
     * reconstructed by a debugger.
     */
    if (!(elfflags & PF_W))
	flags |= MAP_NOCORE;
    return flags;
}

static int convert_macho_flags(int flags) {
    int newflags = MAP_PRIVATE;
    if(!(flags & VM_PROT_WRITE))
        newflags |= MAP_NOCORE;
    return newflags;
}
