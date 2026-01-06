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
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#include <string.h>
#include "decl.h"
#include "msg.h"


/*
 * Find elf or it's class from a pointer to an Elf_Data struct.
 * Warning:  this Assumes that the Elf_Data is part of a libelf
 * Dnode structure, which is expected to be true for any Elf_Data
 * passed into libelf *except* for the xlatetof() and xlatetom() functions.
 */
#define	EDATA_CLASS(edata) \
	(((Dnode *)(edata))->db_scn->s_elf->ed_class)

#define	EDATA_ELF(edata) \
	(((Dnode *)(edata))->db_scn->s_elf)

#define	EDATA_SCN(edata) \
	(((Dnode *)(edata))->db_scn)

#define	EDATA_READLOCKS(edata) \
	READLOCKS(EDATA_ELF((edata)), EDATA_SCN((edata)))

#define	EDATA_READUNLOCKS(edata) \
	READUNLOCKS(EDATA_ELF((edata)), EDATA_SCN((edata)))

int
gelf_getclass(Elf *elf)
{
	if (elf == NULL)
		return (0);

	/*
	 * Don't rely on the idents, a new ehdr doesn't have it!
	 */
	return (elf->ed_class);
}


GElf_Ehdr *
gelf_getehdr(Elf *elf, GElf_Ehdr *dst)
{
	int class;

	if (elf == NULL)
		return (NULL);

	class = gelf_getclass(elf);
	if (class == ELFCLASS32) {
		Elf32_Ehdr * e		= elf32_getehdr(elf);

		if (e == NULL)
			return (NULL);

		ELFRLOCK(elf);
		(void) memcpy(dst->e_ident, e->e_ident, EI_NIDENT);
		dst->e_type		= e->e_type;
		dst->e_machine		= e->e_machine;
		dst->e_version		= e->e_version;
		dst->e_entry		= (Elf64_Addr)e->e_entry;
		dst->e_phoff		= (Elf64_Off)e->e_phoff;
		dst->e_shoff		= (Elf64_Off)e->e_shoff;
		dst->e_flags		= e->e_flags;
		dst->e_ehsize		= e->e_ehsize;
		dst->e_phentsize	= e->e_phentsize;
		dst->e_phnum		= e->e_phnum;
		dst->e_shentsize	= e->e_shentsize;
		dst->e_shnum		= e->e_shnum;
		dst->e_shstrndx		= e->e_shstrndx;
		ELFUNLOCK(elf);

		return (dst);
	} else if (class == ELFCLASS64) {
		Elf64_Ehdr * e		= elf64_getehdr(elf);

		if (e == NULL)
			return (NULL);

		ELFRLOCK(elf);
		*dst			= *e;
		ELFUNLOCK(elf);

		return (dst);
	}

	_elf_seterr(EREQ_CLASS, 0);
	return (NULL);
}

GElf_Shdr *
gelf_getshdr(Elf_Scn *scn,  GElf_Shdr *dst)
{
	if (scn == NULL)
		return (NULL);

	if (scn->s_elf->ed_class == ELFCLASS32) {
		Elf32_Shdr *s		= elf32_getshdr(scn);

		if (s == NULL)
			return (NULL);

		READLOCKS(scn->s_elf, scn);
		dst->sh_name		= s->sh_name;
		dst->sh_type		= s->sh_type;
		dst->sh_flags		= (Elf64_Xword)s->sh_flags;
		dst->sh_addr		= (Elf64_Addr)s->sh_addr;
		dst->sh_offset		= (Elf64_Off)s->sh_offset;
		dst->sh_size		= (Elf64_Xword)s->sh_size;
		dst->sh_link		= s->sh_link;
		dst->sh_info		= s->sh_info;
		dst->sh_addralign	= (Elf64_Xword)s->sh_addralign;
		dst->sh_entsize		= (Elf64_Xword)s->sh_entsize;
		READUNLOCKS(scn->s_elf, scn);

		return (dst);
	} else if (scn->s_elf->ed_class == ELFCLASS64) {
		Elf64_Shdr *s		= elf64_getshdr(scn);

		if (s == NULL)
			return (NULL);

		READLOCKS(scn->s_elf, scn);
		*dst			= *(Elf64_Shdr *)s;
		READUNLOCKS(scn->s_elf, scn);

		return (dst);
	}

	_elf_seterr(EREQ_CLASS, 0);
	return (NULL);
}

GElf_Sym *
gelf_getsym(Elf_Data * data, int ndx, GElf_Sym * dst)
{
	int	class;
	size_t	entsize;

	if (data == NULL)
		return (NULL);

	class = EDATA_CLASS(data);
	if (class == ELFCLASS32)
		entsize = sizeof (Elf32_Sym);
	else if (class == ELFCLASS64)
		entsize = sizeof (GElf_Sym);
	else {
		_elf_seterr(EREQ_CLASS, 0);
		return (NULL);
	}

	EDATA_READLOCKS(data);

	if ((entsize * ndx) >= data->d_size) {
		_elf_seterr(EREQ_RAND, 0);
		dst = NULL;
	} else if (class == ELFCLASS32) {
		Elf32_Sym	*s;
		s		= &(((Elf32_Sym *)data->d_buf)[ndx]);
		dst->st_name	= s->st_name;
		dst->st_value	= (Elf64_Addr)s->st_value;
		dst->st_size	= (Elf64_Xword)s->st_size;
		dst->st_info	= ELF64_ST_INFO(ELF32_ST_BIND(s->st_info),
					ELF32_ST_TYPE(s->st_info));
		dst->st_other	= s->st_other;
		dst->st_shndx	= s->st_shndx;
	} else
		*dst = ((GElf_Sym *)data->d_buf)[ndx];

	EDATA_READUNLOCKS(data);
	return (dst);
}
