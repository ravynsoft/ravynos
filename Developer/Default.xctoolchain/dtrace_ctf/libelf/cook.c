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

/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

#include <string.h>
#include <ar.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include "libelf.h"
#include "decl.h"
#include "msg.h"

#include <sys/mman.h>

/*
 * Cook the input file.
 *	These functions take the input file buffer and extract
 *	the Ehdr, Phdr table, and the Shdr table.  They keep track
 *	of the buffer status as "fresh," "cooked," or "frozen."
 *
 *	fresh	The file buffer is in its original state and
 *		nothing has yet referenced it.
 *
 *	cooked	The application asked for translated data first
 *		and caused the library to return a pointer into
 *		the file buffer.  After this happens, all "raw"
 *		operations must go back to the disk.
 *
 *	frozen	The application first did a "raw" operation that
 *		prohibits reusing the file buffer.  This effectively
 *		freezes the buffer, and all "normal" operations must
 *		duplicate their data.
 *
 *	For archive handling, these functions conspire to align the
 *	file buffer to the host memory format.  Archive members
 *	are guaranteed only even byte alignment, but the file uses
 *	objects at least 4 bytes long.  If an archive member is about
 *	to be cooked and is not aligned in memory, these functions
 *	"slide" the buffer up into the archive member header.
 *	This sliding never occurs for frozen files.
 *
 *	Some processors might not need sliding at all, if they have
 *	no alignment constraints on memory references.  This code
 *	ignores that possibility for two reasons.  First, even machines
 *	that have no constraints usually handle aligned objects faster
 *	than unaligned.  Forcing alignment here probably leads to better
 *	performance.  Second, there's no way to test at run time whether
 *	alignment is required or not.  The safe thing is to align in
 *	all cases.
 *
 *	This sliding relies on the archive header being disposable.
 *	Only archive members that are object files ever slide.
 *	They're also the only ones that ever need to.  Archives never
 *	freeze to make headers disposable.  Any program peculiar enough
 *	to want a frozen archive pays the penalty.
 *
 *	The library itself inspects the Ehdr and the Shdr table
 *	from the file.  Consequently, it converts the file's data
 *	to EV_CURRENT version, not the working version.  This is
 *	transparent to the user.  The library never looks at the
 *	Phdr table; so that's kept in the working version.
 */

static int
_elf_slide(Elf * elf)
{
#pragma unused(elf)
	NOTE(ASSUMING_PROTECTED(*elf))
	Elf		*par = elf->ed_parent;

	if (par == 0 || par->ed_kind != ELF_K_AR)
		return (0);
	return (-1);
}


Okay
_elf_cook(Elf * elf)
{
	NOTE(ASSUMING_PROTECTED(*elf))
	register int	inplace = 1;

	if (elf->ed_kind != ELF_K_ELF && elf->ed_kind != ELF_K_MACHO)
		return (OK_YES);

	if (elf->ed_kind == ELF_K_MACHO)
		inplace = 0; /* Ensures ident structure gets fresh storage */

	if ((elf->ed_status == ES_COOKED) ||
	    ((elf->ed_myflags & EDF_READ) == 0))
		return (OK_YES);

	/*
	 * Here's where the unaligned archive member gets fixed.
	 */
	if (elf->ed_status == ES_FRESH && _elf_slide(elf) != 0)
		return (OK_NO);

	if (elf->ed_status == ES_FROZEN)
		inplace = 0;

	/*
	 * This is the first time we've actually looked at the file
	 * contents.  We need to know whether or not this is an
	 * Elf32 or Elf64 file before we can decode the header.
	 * But it's the header that tells us which is which.
	 *
	 * Resolve the chicken-and-egg problem by peeking at the
	 * 'class' byte in the ident string.
	 */
	if (elf->ed_ident[EI_CLASS] == ELFCLASS32) {
		if (_elf32_ehdr(elf, inplace) != 0)
			return (OK_NO);
		if (_elf32_phdr(elf, inplace) != 0)
			goto xehdr;
		if (_elf32_shdr(elf, inplace) != 0)
			goto xphdr;
		elf->ed_class = ELFCLASS32;
	} else if (elf->ed_ident[EI_CLASS] == ELFCLASS64) {
		if (_elf64_ehdr(elf, inplace) != 0)
			return (OK_NO);
		if (_elf64_phdr(elf, inplace) != 0)
			goto xehdr;
		if (_elf64_shdr(elf, inplace) != 0)
			goto xphdr;
		elf->ed_class = ELFCLASS64;
	} else
		return (OK_NO);

	return (OK_YES);

xphdr:
	if (elf->ed_myflags & EDF_PHALLOC) {
		elf->ed_myflags &= ~EDF_PHALLOC;
		free(elf->ed_phdr);
	}
	elf->ed_phdr = 0;
xehdr:
	if (elf->ed_myflags & EDF_EHALLOC) {
		elf->ed_myflags &= ~EDF_EHALLOC;
		free(elf->ed_ehdr);
	}
	elf->ed_ehdr = 0;

	return (OK_NO);
}


Okay
_elf_cookscn(Elf_Scn * s)
{
	Elf *	elf = s->s_elf;

	if (elf->ed_class == ELFCLASS32) {
		return (_elf32_cookscn(s));
	} else if (elf->ed_class == ELFCLASS64) {
		return (_elf64_cookscn(s));
	}

	_elf_seterr(EREQ_CLASS, 0);
	return (OK_NO);
}
