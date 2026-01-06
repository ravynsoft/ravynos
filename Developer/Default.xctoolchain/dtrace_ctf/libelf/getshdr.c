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


static void *
getshdr(Elf_Scn * scn, int class)
{
	void *	rc;
	Elf *	elf;
	if (scn == 0)
		return (0);
	elf = scn->s_elf;
	READLOCKS(elf, scn)
	if (elf->ed_class != class) {
		READUNLOCKS(elf, scn)
		_elf_seterr(EREQ_CLASS, 0);
		return (0);
	}

	rc = scn->s_shdr;
	READUNLOCKS(elf, scn)
	return (rc);
}

Elf32_Shdr *
elf32_getshdr(Elf_Scn * scn)
{
	return ((Elf32_Shdr*) getshdr(scn, ELFCLASS32));
}

Elf64_Shdr *
elf64_getshdr(Elf_Scn * scn)
{
	return ((Elf64_Shdr*) getshdr(scn, ELFCLASS64));
}
