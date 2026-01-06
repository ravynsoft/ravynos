/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
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
 * Copyright 2002 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#include <string.h>
#include "gelf.h"
#include "decl.h"
#include "msg.h"

int
elf_getshstrndx(Elf *elf, size_t *shstrndx)
{
	GElf_Ehdr	ehdr;
	Elf_Scn		*scn;
	GElf_Shdr	shdr0;

	if (gelf_getehdr(elf, &ehdr) == 0)
		return (0);
	if (ehdr.e_shstrndx != SHN_XINDEX) {
		*shstrndx = ehdr.e_shstrndx;
		return (1);
	}
	if ((scn = elf_getscn(elf, 0)) == 0)
		return (0);
	if (gelf_getshdr(scn, &shdr0) == 0)
		return (0);
	*shstrndx = shdr0.sh_link;
	return (1);
}
