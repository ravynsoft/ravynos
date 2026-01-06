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


char *
elf_getident(Elf * elf, size_t * ptr)
{
	size_t	sz = 0;
	char *	id = 0;

	if (elf != 0) {
		ELFRLOCK(elf)
		if (elf->ed_identsz != 0) {
			if ((elf->ed_vm == 0) || (elf->ed_status !=
			    ES_COOKED)) {
				/*
				 * We need to upgrade to a Writers
				 * lock
				 */
				ELFUNLOCK(elf)
				ELFWLOCK(elf)
				if ((_elf_cook(elf) == OK_YES) &&
				    (_elf_vm(elf, (size_t)0,
				    elf->ed_identsz) == OK_YES)) {
					id = elf->ed_ident;
					sz = elf->ed_identsz;
				}
			} else {
				id = elf->ed_ident;
				sz = elf->ed_identsz;
			}
		}
		ELFUNLOCK(elf)
	}
	if (ptr != 0)
		*ptr = sz;
	return (id);
}

char *
elf_getimage(Elf * elf, size_t * ptr)
{
	char *image = NULL;

	ELFRLOCK(elf)

	if (ptr) {
		*ptr = elf->ed_imagesz;
	}
	image = elf->ed_image;

	ELFUNLOCK(elf)

	return image;
}
