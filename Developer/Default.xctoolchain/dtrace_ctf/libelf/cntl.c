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

int
elf_cntl(Elf * elf, Elf_Cmd cmd)
{

	if (elf == 0)
		return (0);
	ELFWLOCK(elf);
	switch (cmd) {
	case ELF_C_FDREAD:
	{
		int	j = 0;

		if ((elf->ed_myflags & EDF_READ) == 0) {
			_elf_seterr(EREQ_CNTLWRT, 0);
			ELFUNLOCK(elf);
			return (-1);
		}
		if ((elf->ed_status != ES_FROZEN) &&
		    ((_elf_cook(elf) != OK_YES) ||
		    (_elf_vm(elf, (size_t)0, elf->ed_fsz) != OK_YES)))
			j = -1;
		elf->ed_fd = -1;
		ELFUNLOCK(elf);
		return (j);
	}

	case ELF_C_FDDONE:
		if ((elf->ed_myflags & EDF_READ) == 0) {
			_elf_seterr(EREQ_CNTLWRT, 0);
			ELFUNLOCK(elf);
			return (-1);
		}
		elf->ed_fd = -1;
		ELFUNLOCK(elf);
		return (0);

	default:
		_elf_seterr(EREQ_CNTLCMD, 0);
		break;
	}
	ELFUNLOCK(elf);
	return (-1);
}
