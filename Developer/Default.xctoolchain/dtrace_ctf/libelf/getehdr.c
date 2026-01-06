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

#include "libelf.h"
#include "decl.h"
#include "msg.h"


static void*
getehdr(Elf * elf, int class)
{
	void *	rc;
	if (elf == 0)
		return (0);
	ELFWLOCK(elf);
	if (elf->ed_class != class) {
		_elf_seterr(EREQ_CLASS, 0);
		ELFUNLOCK(elf);
		return (0);
	}
	if (elf->ed_ehdr == 0)
		(void) _elf_cook(elf);

	rc = elf->ed_ehdr;
	ELFUNLOCK(elf);

	return (rc);
}


Elf32_Ehdr *
elf32_getehdr(Elf * elf)
{
	return ((Elf32_Ehdr*) getehdr(elf, ELFCLASS32));
}


Elf64_Ehdr *
elf64_getehdr(Elf * elf)
{
	return ((Elf64_Ehdr*) getehdr(elf, ELFCLASS64));
}
