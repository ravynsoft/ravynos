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

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/


#ifndef _SYS_ELF_H
#define	_SYS_ELF_H
#include "elftypes.h" /* In lieu of Solaris <sys/elftypes.h> */

#ifdef	__cplusplus
extern "C" {
#endif
/*
 *	"Enumerations" below use ...NUM as the number of
 *	values in the list.  It should be 1 greater than the
 *	highest "real" value.
 */

/*
 *	ELF header
 */

#define	EI_NIDENT	16

typedef struct {
	unsigned char	e_ident[EI_NIDENT];	/* ident bytes */
	Elf32_Half	e_type;			/* file type */
	Elf32_Half	e_machine;		/* target machine */
	Elf32_Word	e_version;		/* file version */
	Elf32_Addr	e_entry;		/* start address */
	Elf32_Off	e_phoff;		/* phdr file offset */
	Elf32_Off	e_shoff;		/* shdr file offset */
	Elf32_Word	e_flags;		/* file flags */
	Elf32_Half	e_ehsize;		/* sizeof ehdr */
	Elf32_Half	e_phentsize;		/* sizeof phdr */
	Elf32_Half	e_phnum;		/* number phdrs */
	Elf32_Half	e_shentsize;		/* sizeof shdr */
	Elf32_Half	e_shnum;		/* number shdrs */
	Elf32_Half	e_shstrndx;		/* shdr string index */
} Elf32_Ehdr;

typedef struct {
	unsigned char	e_ident[EI_NIDENT];	/* ident bytes */
	Elf64_Half	e_type;			/* file type */
	Elf64_Half	e_machine;		/* target machine */
	Elf64_Word	e_version;		/* file version */
	Elf64_Addr	e_entry;		/* start address */
	Elf64_Off	e_phoff;		/* phdr file offset */
	Elf64_Off	e_shoff;		/* shdr file offset */
	Elf64_Word	e_flags;		/* file flags */
	Elf64_Half	e_ehsize;		/* sizeof ehdr */
	Elf64_Half	e_phentsize;		/* sizeof phdr */
	Elf64_Half	e_phnum;		/* number phdrs */
	Elf64_Half	e_shentsize;		/* sizeof shdr */
	Elf64_Half	e_shnum;		/* number shdrs */
	Elf64_Half	e_shstrndx;		/* shdr string index */
} Elf64_Ehdr;


#define	EI_MAG0		0	/* e_ident[] indexes */
#define	EI_MAG1		1
#define	EI_MAG2		2
#define	EI_MAG3		3
#define	EI_CLASS	4	/* File class */
#define	EI_DATA		5	/* Data encoding */
#define	EI_VERSION	6	/* File version */
#define	EI_OSABI	7	/* Operating system/ABI identification */
#define	EI_ABIVERSION	8	/* ABI version */

#define	ELFMAG0		0x7f		/* EI_MAG */
#define	ELFMAG1		'E'
#define	ELFMAG2		'L'
#define	ELFMAG3		'F'
#define	ELFMAG		"\177ELF"
#define	SELFMAG		4

#define	ELFCLASSNONE	0		/* EI_CLASS */
#define	ELFCLASS32	1
#define	ELFCLASS64	2
#define	ELFCLASSNUM	3

#define	ELFDATANONE	0		/* EI_DATA */
#define	ELFDATA2LSB	1
#define	ELFDATA2MSB	2
#define	ELFDATANUM	3

#define	ET_NONE		0		/* e_type */
#define	ET_REL		1
#define	ET_EXEC		2
#define	ET_DYN		3

#define	EM_NONE		0		/* e_machine */

#define	EV_NONE		0		/* e_version, EI_VERSION */
#define	EV_CURRENT	1
#define	EV_NUM		2

#define	ELFOSABI_NONE		0	/* No extensions or unspecified */

/*
 *	Program header
 */

typedef struct {
	Elf32_Word	p_type;		/* entry type */
	Elf32_Off	p_offset;	/* file offset */
	Elf32_Addr	p_vaddr;	/* virtual address */
	Elf32_Addr	p_paddr;	/* physical address */
	Elf32_Word	p_filesz;	/* file size */
	Elf32_Word	p_memsz;	/* memory size */
	Elf32_Word	p_flags;	/* entry flags */
	Elf32_Word	p_align;	/* memory/file alignment */
} Elf32_Phdr;

typedef struct {
	Elf64_Word	p_type;		/* entry type */
	Elf64_Word	p_flags;	/* entry flags */
	Elf64_Off	p_offset;	/* file offset */
	Elf64_Addr	p_vaddr;	/* virtual address */
	Elf64_Addr	p_paddr;	/* physical address */
	Elf64_Xword	p_filesz;	/* file size */
	Elf64_Xword	p_memsz;	/* memory size */
	Elf64_Xword	p_align;	/* memory/file alignment */
} Elf64_Phdr;

/*
 *	Section header
 */

typedef struct {
	Elf32_Word	sh_name;	/* section name */
	Elf32_Word	sh_type;	/* SHT_... */
	Elf32_Word	sh_flags;	/* SHF_... */
	Elf32_Addr	sh_addr;	/* virtual address */
	Elf32_Off	sh_offset;	/* file offset */
	Elf32_Word	sh_size;	/* section size */
	Elf32_Word	sh_link;	/* misc info */
	Elf32_Word	sh_info;	/* misc info */
	Elf32_Word	sh_addralign;	/* memory alignment */
	Elf32_Word	sh_entsize;	/* entry size if table */
} Elf32_Shdr;

typedef struct {
	Elf64_Word	sh_name;	/* section name */
	Elf64_Word	sh_type;	/* SHT_... */
	Elf64_Xword	sh_flags;	/* SHF_... */
	Elf64_Addr	sh_addr;	/* virtual address */
	Elf64_Off	sh_offset;	/* file offset */
	Elf64_Xword	sh_size;	/* section size */
	Elf64_Word	sh_link;	/* misc info */
	Elf64_Word	sh_info;	/* misc info */
	Elf64_Xword	sh_addralign;	/* memory alignment */
	Elf64_Xword	sh_entsize;	/* entry size if table */
} Elf64_Shdr;

#define	SHT_NULL		0		/* sh_type */
#define	SHT_PROGBITS		1
#define	SHT_SYMTAB		2
#define	SHT_STRTAB		3
#define	SHT_RELA		4
#define	SHT_HASH		5
#define	SHT_DYNAMIC		6
#define	SHT_NOTE		7
#define	SHT_NOBITS		8
#define	SHT_REL			9
#define	SHT_SHLIB		10
#define	SHT_DYNSYM		11
#define	SHT_UNKNOWN12		12
#define	SHT_UNKNOWN13		13
#define	SHT_INIT_ARRAY		14
#define	SHT_FINI_ARRAY		15
#define	SHT_PREINIT_ARRAY	16
#define	SHT_GROUP		17
#define	SHT_SYMTAB_SHNDX	18
#define	SHT_NUM			19

#define	SHF_WRITE		0x01		/* sh_flags */
#define	SHF_ALLOC		0x02

#define	SHN_UNDEF	0		/* special section numbers */
#define	SHN_ABS		0xfff1
#define SHN_MACHO_64	0xfffd		/* Mach-o_64 direct string access */
#define SHN_MACHO	0xfffe		/* Mach-o direct string access */
#define	SHN_XINDEX	0xffff		/* extended sect index */

/*
 *	Symbol table
 */

typedef struct {
	Elf32_Word	st_name;
	Elf32_Addr	st_value;
	Elf32_Word	st_size;
	unsigned char	st_info;	/* bind, type: ELF_32_ST_... */
	unsigned char	st_other;
	Elf32_Half	st_shndx;	/* SHN_... */
#if defined(__arm__) || defined(__arm64__)
	uint32_t	st_arch_subinfo;
#endif
} Elf32_Sym;

typedef struct {
	Elf64_Sxword	st_name;
	unsigned char	st_info;	/* bind, type: ELF_64_ST_... */
	unsigned char	st_other;
	Elf64_Half	st_shndx;	/* SHN_... */
	Elf64_Addr	st_value;
	Elf64_Xword	st_size;
#if defined(__arm__) || defined(__arm64__)
	uint32_t	st_arch_subinfo;
#endif
} Elf64_Sym;

#define	STN_UNDEF	0

/*
 *	The macros compose and decompose values for S.st_info
 *
 *	bind = ELF32_ST_BIND(S.st_info)
 *	type = ELF32_ST_TYPE(S.st_info)
 *	S.st_info = ELF32_ST_INFO(bind, type)
 */

#define	ELF32_ST_BIND(info)		((info) >> 4)
#define	ELF32_ST_TYPE(info)		((info) & 0xf)
#define	ELF32_ST_INFO(bind, type)	(((bind)<<4)+((type)&0xf))

#define	ELF64_ST_BIND(info)		((info) >> 4)
#define	ELF64_ST_TYPE(info)		((info) & 0xf)
#define	ELF64_ST_INFO(bind, type)	(((bind)<<4)+((type)&0xf))


#define	STB_LOCAL	0		/* BIND */
#define	STB_GLOBAL	1
#define	STB_WEAK	2

#define	STT_NOTYPE	0		/* TYPE */
#define	STT_OBJECT	1
#define	STT_FUNC	2
#define	STT_SECTION	3
#define	STT_FILE	4
#define	STT_COMMON	5
#define	STT_TLS		6
#define	STT_NUM		7

#ifdef	__cplusplus
}
#endif
#endif	/* _SYS_ELF_H */
