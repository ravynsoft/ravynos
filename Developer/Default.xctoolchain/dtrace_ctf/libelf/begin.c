/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*
 *	Copyright (c) 1988 AT&T
 *	All Rights Reserved
 */

#include <ar.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include "libelf.h"
#include <sys/mman.h>
#include "decl.h"
#include "msg.h"

static const char	armag[] = ARMAG;

#include <crt_externs.h>
#include <mach/mach.h>
#include <mach-o/loader.h>
#include <mach-o/dyld.h>
#include <mach-o/fat.h>
#ifndef __linux__
#include <sys/sysctl.h>
#endif

static cpu_type_t current_program_arch(void)
{
#ifdef __linux__
	cpu_type_t current_arch = CPU_TYPE_X86_64;
#else // !__linux__
        cpu_type_t current_arch = (_NSGetMachExecuteHeader())->cputype;
#endif // __linux__
        return current_arch;
}

static cpu_type_t current_kernel_arch(void)
{
#ifdef __linux__
	cpu_type_t current_arch = CPU_TYPE_X86_64;
#else // !__linux__
        struct host_basic_info  hi;
        unsigned int            size;
        kern_return_t           kret;
        cpu_type_t                                current_arch;
        int                                                ret, mib[4];
        size_t                                        len;
        struct kinfo_proc                kp;

        size = sizeof(hi)/sizeof(int);
        kret = host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t)&hi, &size);
        if (kret != KERN_SUCCESS) {
                return 0;
        }
        current_arch = hi.cpu_type;
        /* Now determine if the kernel is running in 64-bit mode */
        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_PID;
        mib[3] = 0; /* kernproc, pid 0 */
        len = sizeof(kp);
        ret = sysctl(mib, sizeof(mib)/sizeof(mib[0]), &kp, &len, NULL, 0);
        if (ret == -1) {
                return 0;
        }
        if (kp.kp_proc.p_flag & P_LP64) {
                current_arch |= CPU_ARCH_ABI64;
        }
#endif // __linux__
        return current_arch;
}

static Elf *
_elf_regular(int fd, unsigned flags)		/* initialize regular file */
{
	Elf		*elf;

	if ((elf = (Elf *)calloc(1, sizeof (Elf))) == 0) {
		_elf_seterr(EMEM_ELF, errno);
		return (0);
	}

	NOTE(NOW_INVISIBLE_TO_OTHER_THREADS(*elf))
	elf->ed_fd = fd;
	elf->ed_myflags |= flags;
	if (_elf_inmap(elf) != OK_YES) {
		free(elf);
		return (0);
	}
	NOTE(NOW_VISIBLE_TO_OTHER_THREADS(*elf))
	return (elf);
}

static Elf *
_elf_config(Elf * elf)
{
	char *		base;
	unsigned	encode;

	ELFRWLOCKINIT(&elf->ed_rwlock);

	/*
	 * Determine if this is a ELF file.
	 */
	base = elf->ed_ident;
	if ((elf->ed_fsz >= EI_NIDENT) &&
	    (_elf_vm(elf, (size_t)0, (size_t)EI_NIDENT) == OK_YES) &&
	    (base[EI_MAG0] == ELFMAG0) &&
	    (base[EI_MAG1] == ELFMAG1) &&
	    (base[EI_MAG2] == ELFMAG2) &&
	    (base[EI_MAG3] == ELFMAG3)) {
		_elf_seterr(EREQ_NOTSUP, 0);
		return (0);
	}

	/*
	 * Determine if this is a Mach-o file.
	 */
	if ((elf->ed_fsz >= sizeof(struct fat_header)) &&
	    (_elf_vm(elf, (size_t)0, (size_t)sizeof(struct fat_header)) == OK_YES) &&
	    (FAT_MAGIC == *(unsigned int *)(elf->ed_ident) ||
		 FAT_CIGAM == *(unsigned int *)(elf->ed_ident)))
	{
		struct fat_header *fat_header = (struct fat_header *)(elf->ed_ident);
		int nfat_arch = OSSwapBigToHostInt32(fat_header->nfat_arch);
		int end_of_archs = sizeof(struct fat_header) + nfat_arch * sizeof(struct fat_arch);
		struct fat_arch *arch = (struct fat_arch *)(elf->ed_ident + sizeof(struct fat_header));

		cpu_type_t cputype = (elf->ed_myflags & EDF_RDKERNTYPE) ? current_kernel_arch() :current_program_arch();

		if (end_of_archs > elf->ed_fsz) {
			_elf_seterr(EIO_VM, errno);
			return 0;
		}

		for (; nfat_arch-- > 0; arch++) {
			if(((cpu_type_t)OSSwapBigToHostInt32(arch->cputype)) == cputype) {
				elf->ed_ident += OSSwapBigToHostInt32(arch->offset);
				elf->ed_image += OSSwapBigToHostInt32(arch->offset);
				elf->ed_fsz -= OSSwapBigToHostInt32(arch->offset);
				elf->ed_imagesz -= OSSwapBigToHostInt32(arch->offset);
				break;
			}
		}
		/* Fall through positioned at mach_header for "thin" architecture matching host endian-ness */
	}

	if ((elf->ed_fsz >= sizeof(struct mach_header)) &&
	    (_elf_vm(elf, (size_t)0, (size_t)sizeof(struct mach_header)) == OK_YES) &&
	    (MH_MAGIC == *(unsigned int *)(elf->ed_image) ||
		 MH_CIGAM == *(unsigned int *)(elf->ed_image))) {

		struct mach_header *mh = (struct mach_header *)elf->ed_image;
		struct load_command *thisLC = (struct load_command *)(&(mh[1]));
		int i, n = 0;

		for (i = 0; i < mh->ncmds; i++) {
			int cmd = thisLC->cmd, cmdsize = thisLC->cmdsize;

			switch(cmd) {
				case LC_SEGMENT:
				{
					struct segment_command *thisSG = (struct segment_command *)thisLC;


					n += thisSG->nsects;
					break;
				}

				case LC_SYMTAB:
					n += 2;
					break;

				default:
					break;
			}
			thisLC = (struct load_command *) ((caddr_t) thisLC + cmdsize);
		}

		if (0 == (elf->ed_ident = malloc(sizeof(Elf32_Ehdr)))) {
			_elf_seterr(EMEM_ELF, errno);
			return (0);
		}

		((Elf32_Ehdr *)(elf->ed_ident))->e_ident[EI_MAG0] = 'M';
		((Elf32_Ehdr *)(elf->ed_ident))->e_ident[EI_MAG1] = 'a';
		((Elf32_Ehdr *)(elf->ed_ident))->e_ident[EI_MAG2] = 'c';
		((Elf32_Ehdr *)(elf->ed_ident))->e_ident[EI_MAG3] = 'h';
		((Elf32_Ehdr *)(elf->ed_ident))->e_ident[EI_CLASS] = ELFCLASS32;
#if defined(__BIG_ENDIAN__)
		((Elf32_Ehdr *)(elf->ed_ident))->e_ident[EI_DATA] = ELFDATA2MSB;
#else
		((Elf32_Ehdr *)(elf->ed_ident))->e_ident[EI_DATA] = ELFDATA2LSB;
#endif
		((Elf32_Ehdr *)(elf->ed_ident))->e_ident[EI_VERSION] = EV_CURRENT;
		((Elf32_Ehdr *)(elf->ed_ident))->e_ident[EI_OSABI] = ELFOSABI_NONE;
		((Elf32_Ehdr *)(elf->ed_ident))->e_ident[EI_ABIVERSION] = 0;
		((Elf32_Ehdr *)(elf->ed_ident))->e_type = ET_NONE;
		((Elf32_Ehdr *)(elf->ed_ident))->e_machine = EM_NONE;
		((Elf32_Ehdr *)(elf->ed_ident))->e_version = EV_CURRENT;
		((Elf32_Ehdr *)(elf->ed_ident))->e_phoff = 0;
		((Elf32_Ehdr *)(elf->ed_ident))->e_shoff = sizeof(struct mach_header);
		((Elf32_Ehdr *)(elf->ed_ident))->e_ehsize = sizeof(Elf32_Ehdr);
		((Elf32_Ehdr *)(elf->ed_ident))->e_phentsize = sizeof(Elf32_Phdr);
		((Elf32_Ehdr *)(elf->ed_ident))->e_phnum = 0;
		((Elf32_Ehdr *)(elf->ed_ident))->e_shentsize = sizeof(Elf32_Shdr);
		((Elf32_Ehdr *)(elf->ed_ident))->e_shnum = n + 1;
		((Elf32_Ehdr *)(elf->ed_ident))->e_shstrndx = SHN_MACHO;

		elf->ed_kind = ELF_K_MACHO;
		elf->ed_class = ((Elf32_Ehdr *)(elf->ed_ident))->e_ident[EI_CLASS];
#if defined(__BIG_ENDIAN__)
		elf->ed_encode = ELFDATA2MSB;
#else
		elf->ed_encode = ELFDATA2LSB;
#endif
		elf->ed_version = ((Elf32_Ehdr *)(elf->ed_ident))->e_ident[EI_VERSION];
		elf->ed_identsz = EI_NIDENT;

		/*
		 * Allow writing only if originally specified read only.
		 * This is only necessary if the file must be translating
		 * from one encoding to another.
		 */
 		ELFACCESSDATA(encode, _elf_encode)
		if ((elf->ed_vm == 0) && ((elf->ed_myflags & EDF_WRITE) == 0) &&
		    (elf->ed_encode != encode)) {
			if (mprotect((char *)elf->ed_image, elf->ed_imagesz,
			    PROT_READ|PROT_WRITE) == -1) {
				_elf_seterr(EIO_VM, errno);
				return (0);
			}
		}
		return (elf);
	}

	if ((elf->ed_fsz >= sizeof(struct mach_header_64)) &&
	    (_elf_vm(elf, (size_t)0, (size_t)sizeof(struct mach_header_64)) == OK_YES) &&
	    (MH_MAGIC_64 == *(unsigned int *)(elf->ed_image) ||
		 MH_CIGAM_64 == *(unsigned int *)(elf->ed_image))) {

		struct mach_header_64 *mh64 = (struct mach_header_64 *)elf->ed_image;
		struct load_command *thisLC = (struct load_command *)(&(mh64[1]));
		int i, n = 0;

		for (i = 0; i < mh64->ncmds; i++) {
			int cmd = thisLC->cmd, cmdsize = thisLC->cmdsize;

			switch(cmd) {
				case LC_SEGMENT_64:
				{
					struct segment_command_64 *thisSG64 = (struct segment_command_64 *)thisLC;
					n += thisSG64->nsects;
					break;
				}

				case LC_SYMTAB:
					n += 2;
					break;

				default:
					break;
			}
			thisLC = (struct load_command *) ((caddr_t) thisLC + cmdsize);
		}

		if (0 == (elf->ed_ident = malloc(sizeof(Elf64_Ehdr)))) {
			_elf_seterr(EMEM_ELF, errno);
			return (0);
		}

		((Elf64_Ehdr *)(elf->ed_ident))->e_ident[EI_MAG0] = 'M';
		((Elf64_Ehdr *)(elf->ed_ident))->e_ident[EI_MAG1] = 'a';
		((Elf64_Ehdr *)(elf->ed_ident))->e_ident[EI_MAG2] = 'c';
		((Elf64_Ehdr *)(elf->ed_ident))->e_ident[EI_MAG3] = 'h';
		((Elf64_Ehdr *)(elf->ed_ident))->e_ident[EI_CLASS] = ELFCLASS64;
#if defined(__BIG_ENDIAN__)
		((Elf64_Ehdr *)(elf->ed_ident))->e_ident[EI_DATA] = ELFDATA2MSB;
#else
		((Elf64_Ehdr *)(elf->ed_ident))->e_ident[EI_DATA] = ELFDATA2LSB;
#endif
		((Elf64_Ehdr *)(elf->ed_ident))->e_ident[EI_VERSION] = EV_CURRENT;
		((Elf64_Ehdr *)(elf->ed_ident))->e_ident[EI_OSABI] = ELFOSABI_NONE;
		((Elf64_Ehdr *)(elf->ed_ident))->e_ident[EI_ABIVERSION] = 0;
		((Elf64_Ehdr *)(elf->ed_ident))->e_type = ET_NONE;
		((Elf64_Ehdr *)(elf->ed_ident))->e_machine = EM_NONE;
		((Elf64_Ehdr *)(elf->ed_ident))->e_version = EV_CURRENT;
		((Elf64_Ehdr *)(elf->ed_ident))->e_phoff = 0;
		((Elf64_Ehdr *)(elf->ed_ident))->e_shoff = sizeof(struct mach_header_64);
		((Elf64_Ehdr *)(elf->ed_ident))->e_ehsize = sizeof(Elf64_Ehdr);
		((Elf64_Ehdr *)(elf->ed_ident))->e_phentsize = sizeof(Elf64_Phdr);
		((Elf64_Ehdr *)(elf->ed_ident))->e_phnum = 0;
		((Elf64_Ehdr *)(elf->ed_ident))->e_shentsize = sizeof(Elf64_Shdr);
		((Elf64_Ehdr *)(elf->ed_ident))->e_shnum = n + 1;
		((Elf64_Ehdr *)(elf->ed_ident))->e_shstrndx = SHN_MACHO_64;

		elf->ed_kind = ELF_K_MACHO;
		elf->ed_class = ((Elf64_Ehdr *)(elf->ed_ident))->e_ident[EI_CLASS];
#if defined(__BIG_ENDIAN__)
		elf->ed_encode = ELFDATA2MSB;
#else
		elf->ed_encode = ELFDATA2LSB;
#endif
		elf->ed_version = ((Elf64_Ehdr *)(elf->ed_ident))->e_ident[EI_VERSION];
		elf->ed_identsz = EI_NIDENT;

		/*
		 * Allow writing only if originally specified read only.
		 * This is only necessary if the file must be translating
		 * from one encoding to another.
		 */
		ELFACCESSDATA(encode, _elf_encode)
		if ((elf->ed_vm == 0) && ((elf->ed_myflags & EDF_WRITE) == 0) &&
		    (elf->ed_encode != encode)) {
			if (mprotect((char *)elf->ed_image, elf->ed_imagesz,
			    PROT_READ|PROT_WRITE) == -1) {
				_elf_seterr(EIO_VM, errno);
				return (0);
			}
		}
		return (elf);
	}

    /*
	 * Determine if this is an Archive
	 */
	if ((elf->ed_fsz >= SARMAG) &&
	    (_elf_vm(elf, (size_t)0, (size_t)SARMAG) == OK_YES) &&
	    (memcmp(base, armag, SARMAG) == 0)) {
		_elf_seterr(EREQ_NOTSUP, 0);
		return (0);
	}

	/*
	 *	Return a few ident bytes, but not so many that
	 *	getident() must read a large file.  512 is arbitrary.
	 */

	elf->ed_kind = ELF_K_NONE;
	if ((elf->ed_identsz = elf->ed_fsz) > 512)
		elf->ed_identsz = 512;

	return (elf);
}

Elf *
elf_begin(int fd, Elf_Cmd cmd, Elf *ref)
{
	register Elf	*elf;
	unsigned	work;
	unsigned	flags = 0;

	ELFACCESSDATA(work, _elf_work)
	if (work == EV_NONE)	/* version() not called yet */
	{
		_elf_seterr(ESEQ_VER, 0);
		return (0);
	}
	switch (cmd) {
	default:
		_elf_seterr(EREQ_BEGIN, 0);
		return (0);

	case ELF_C_NULL:
		return (0);

	case ELF_C_WRITE:
		if ((elf = (Elf *)calloc(1, sizeof (Elf))) == 0) {
			_elf_seterr(EMEM_ELF, errno);
			return (0);
		}
		NOTE(NOW_INVISIBLE_TO_OTHER_THREADS(*elf))
		ELFRWLOCKINIT(&elf->ed_rwlock);
		elf->ed_fd = fd;
		elf->ed_activ = 1;
		elf->ed_myflags |= EDF_WRITE;
		NOTE(NOW_VISIBLE_TO_OTHER_THREADS(*elf))
		return (elf);
	case ELF_C_RDWR:
		flags = EDF_WRITE | EDF_READ;
		break;

	case ELF_C_READ:
		flags = EDF_READ;
		break;

	case ELF_C_RDKERNTYPE:
		flags = EDF_READ | EDF_RDKERNTYPE;
		break;
	}

	/*
	 *	A null ref asks for a new file
	 *	Non-null ref bumps the activation count
	 *		or gets next archive member
	 */

	if (ref == 0) {
		if ((elf = _elf_regular(fd, flags)) == 0)
			return (0);
	} else {
		ELFWLOCK(ref);
		if ((ref->ed_myflags & flags) != flags) {
			_elf_seterr(EREQ_RDWR, 0);
			ELFUNLOCK(ref);
			return (0);
		}
		/*
		 * new activation ?
		 */
		if (ref->ed_kind != ELF_K_AR) {
			++ref->ed_activ;
			ELFUNLOCK(ref);
			return (ref);
		}
		_elf_seterr(EREQ_NOTSUP, 0);
		ELFUNLOCK(ref);
		return (0);
	}

	NOTE(NOW_INVISIBLE_TO_OTHER_THREADS(*elf))
	elf->ed_activ = 1;
	elf = _elf_config(elf);
	NOTE(NOW_VISIBLE_TO_OTHER_THREADS(*elf))

	return (elf);
}
