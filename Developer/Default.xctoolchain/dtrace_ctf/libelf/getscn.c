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


Elf_Scn *
elf_getscn(Elf * elf, size_t index)
{
	Elf_Scn	*	s;
	Elf_Scn	*	prev_s;
	size_t		j = index;
	size_t		tabsz;

	if (elf == 0)
		return (0);

	ELFRLOCK(elf)
	tabsz = elf->ed_scntabsz;
	if (elf->ed_hdscn == 0) {
		ELFUNLOCK(elf)
		ELFWLOCK(elf)
		if ((elf->ed_hdscn == 0) && (_elf_cook(elf) != OK_YES)) {
			ELFUNLOCK(elf);
			return (0);
		}
		ELFUNLOCK(elf);
		ELFRLOCK(elf)
	}
	/*
	 * If the section in question is part of a table allocated
	 * from within _elf_prescn() then we can index straight
	 * to it.
	 */
	if (index < tabsz) {
		s = &elf->ed_hdscn[index];
		ELFUNLOCK(elf);
		return (s);
	}

	if (tabsz)
		s = &elf->ed_hdscn[tabsz - 1];
	else
		s = elf->ed_hdscn;

	for (prev_s = 0; s != 0; prev_s = s, s = s->s_next) {
		if (prev_s) {
			SCNUNLOCK(prev_s)
		}
		SCNLOCK(s)
		if (j == 0) {
			if (s->s_index == index) {
				SCNUNLOCK(s)
				ELFUNLOCK(elf);
				return (s);
			}
			_elf_seterr(EBUG_SCNLIST, 0);
			SCNUNLOCK(s)
			ELFUNLOCK(elf)
			return (0);
		}
		--j;
	}
	if (prev_s) {
		SCNUNLOCK(prev_s)
	}
	_elf_seterr(EREQ_NDX, 0);
	ELFUNLOCK(elf);
	return (0);
}
