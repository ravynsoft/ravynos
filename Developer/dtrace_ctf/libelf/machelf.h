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

#ifndef	_SYS_MACHELF_H
#define	_SYS_MACHELF_H

#ifdef	__cplusplus
extern "C" {
#endif
#include <sys/elf.h>

/*
 * Make machine class dependent data types transparent to the common code
 */
#if defined(_ELF64) && !defined(_ELF32_COMPAT)
typedef	Elf64_Ehdr	Ehdr;
typedef	Elf64_Shdr	Shdr;
#else	/* _ILP32 */
typedef	Elf32_Ehdr	Ehdr;
typedef	Elf32_Shdr	Shdr;
#endif	/* _ILP32 */

#ifdef	__cplusplus
}
#endif

#ifdef __linux__
# define __nonnull_attribute__(...) __attribute__ ((__nonnull__ (__VA_ARGS__)))
# define __deprecated_attribute__ __attribute__ ((__deprecated__))
# define __attribute_pure__ __attribute__ ((__pure__))
# define __const_attribute__ __attribute__ ((__const__))
#endif

#endif	/* _SYS_MACHELF_H */
