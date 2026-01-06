/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/* 
 * Mach Operating System
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 **********************************************************************
 * HISTORY
 * 13-Feb-88  John Seamons (jks) at NeXT
 *	NeXT_MOD: use same hack as Sun for machine type.
 *
 * 12-Sep-86  Jonathan J. Chew (jjc) at Carnegie-Mellon University
 *	Changed first long word to two shorts for the Sun to give
 *	the machine type along with the magic number.
 *	Added machine types for Sun.
 *	Made file includable more than once.
 *
 **********************************************************************
 */
 
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)exec.h	7.1 (Berkeley) 6/4/86
 */

#ifndef	_EXEC_
#define	_EXEC_	1

#include <stdint.h>

#ifndef NeXT_MOD
#define NeXT_MOD 1
#endif

/*
 * Header prepended to each a.out file.
 */
struct exec {
#if	sun || NeXT_MOD
unsigned short  a_machtype;     /* machine type */
unsigned short  a_magic;        /* magic number */
#else	/* sun || NeXT_MOD */
	int32_t	a_magic;	/* magic number */
#endif	/* sun || NeXT_MOD */
uint32_t	a_text;		/* size of text segment */
uint32_t	a_data;		/* size of initialized data */
uint32_t	a_bss;		/* size of uninitialized data */
uint32_t	a_syms;		/* size of symbol table */
uint32_t	a_entry;	/* entry point */
uint32_t	a_trsize;	/* size of text relocation */
uint32_t	a_drsize;	/* size of data relocation */
};

#define	OMAGIC	0407		/* old impure format */
#define	NMAGIC	0410		/* read-only text */
#define	ZMAGIC	0413		/* demand load format */

#ifdef sun
/* Sun machine types */

#define M_OLDSUN2	0	/* old sun-2 executable files */
#define M_68010		1	/* runs on either 68010 or 68020 */
#define M_68020		2	/* runs only on 68020 */
#endif /* sun */

#endif	/* _EXEC_ */
