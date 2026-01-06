/*
 * Copyright (c) 2004, Apple Computer, Inc. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/* Copyright (c) 1991 NeXT Computer, Inc.  All rights reserved.
 *
 *	File:	architecture/m88k/reg_help.h
 *	Author:	Mike DeMoney, NeXT Computer, Inc.
 *
 *	This header file defines cpp macros useful for defining
 *	machine register and doing machine-level operations.
 *
 * HISTORY
 * 23-Jan-91  Mike DeMoney (mike@next.com)
 *	Created.
 */

#ifndef _ARCH_M88K_REG_HELP_H_
#define _ARCH_M88K_REG_HELP_H_

#import <architecture/nrw/reg_help.h>

/* Stack pointer must always be a multiple of 16 */
#define	STACK_INCR	16
#define	ROUND_FRAME(x)	((((unsigned)(x)) + STACK_INCR - 1) & ~(STACK_INCR-1))

/*
 * REG_PAIR_DEF -- define a register pair
 * Register pairs are appropriately aligned to allow access via
 * ld.d and st.d.
 *
 * Usage:
 *	struct foo {
 *		REG_PAIR_DEF(
 *			bar_t *,	barp,
 *			afu_t,		afu
 *		);
 *	};
 *
 * Access to individual entries of the pair is via the REG_PAIR
 * macro (below).
 */
#define	REG_PAIR_DEF(type0, name0, type1, name1)		\
	struct {						\
		type0	name0 __attribute__(( aligned(8) ));	\
		type1	name1;					\
	} name0##_##name1

/*
 * REG_PAIR -- Macro to define names for accessing individual registers
 * of register pairs.
 *
 * Usage:
 *	arg0 is first element of pair
 *	arg1 is second element of pair
 *	arg2 is desired element of pair
 * eg:
 *	#define	foo_barp	REG_PAIR(barp, afu, afu)
 */
#define	REG_PAIR(name0, name1, the_name)			\
	name0##_##name1.the_name

#endif  /* _ARCH_M88K_REG_HELP_H_ */
