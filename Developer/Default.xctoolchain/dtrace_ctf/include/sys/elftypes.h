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
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/


/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef _SYS_ELFTYPES_H
#define	_SYS_ELFTYPES_H
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

typedef uint32_t	Elf32_Addr;	/* Program address. */
typedef uint8_t		Elf32_Byte;	/* Unsigned tiny integer. */
typedef uint16_t	Elf32_Half;	/* Unsigned medium integer. */
typedef uint32_t	Elf32_Off;	/* File offset. */
typedef uint16_t	Elf32_Section;	/* Section index. */
typedef int32_t		Elf32_Sword;	/* Signed integer. */
typedef uint32_t	Elf32_Word;	/* Unsigned integer. */
typedef uint64_t	Elf32_Lword;	/* Unsigned long integer. */

typedef uint64_t	Elf64_Addr;	/* Program address. */
typedef uint8_t		Elf64_Byte;	/* Unsigned tiny integer. */
typedef uint16_t	Elf64_Half;	/* Unsigned medium integer. */
typedef uint64_t	Elf64_Off;	/* File offset. */
typedef uint16_t	Elf64_Section;	/* Section index. */
typedef int32_t		Elf64_Sword;	/* Signed integer. */
typedef uint32_t	Elf64_Word;	/* Unsigned integer. */
typedef uint64_t	Elf64_Lword;	/* Unsigned long integer. */
typedef uint64_t	Elf64_Xword;	/* Unsigned long integer. */
typedef int64_t		Elf64_Sxword;	/* Signed long integer. */

#ifdef	__cplusplus
}
#endif

#endif	/* _SYS_ELFTYPES_H */
