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
 *	File:	architecture/m88k/fp_regs.h
 *	Author:	Mike DeMoney, NeXT Computer, Inc.
 *
 *	This include file defines Motorola 88K architecturally defined
 *	floating point control and status registers.
 *
 * HISTORY
 * 23-Jan-91  Mike DeMoney (mike@next.com)
 *	Created.
 */

#ifndef	_ARCH_M88K_FP_REGS_H_
#define	_ARCH_M88K_FP_REGS_H_

#import <architecture/m88k/reg_help.h>

/*
 * m88k_xrf_t -- data types that MAY be in extended register file
 * Actual data types supported is implementation dependent
 */
typedef union {
	float		f;		// 32 bit IEEE single
	double		d;		// 64 bit IEEE double
	/*
	 * NOTE: currently compiler implements long double type
	 * simply as double.  In the future, it may implement
	 * this as 80 bit IEEE double extended or 128 bit IEEE quad
	 * as appropriate for the 88K implementation.
	 */
	long double	e;		// 80 or 128 bit IEEE format
	/* Insure compiler aligns struct appropriately */
#ifdef __GNUC__
	unsigned	x[4] __attribute__(( aligned(16) ));
#else
	unsigned	x[4];
#endif
} m88k_xrf_t;

/*
 * FPSR -- Floating Point Status Register
 */
typedef struct {
	unsigned	:BITS_WIDTH(31,17);
	unsigned	xmod:BIT_WIDTH(16);	// extended registers modified
	unsigned	:BITS_WIDTH(15,5);
	unsigned	afinv:BIT_WIDTH(4);	// accumulated invalid flag
	unsigned	afdvz:BIT_WIDTH(3);	// accumulated div by zero flag
	unsigned	afunf:BIT_WIDTH(2);	// accumulated underflow flag
	unsigned	afovf:BIT_WIDTH(1);	// accumulated overflow flag
	unsigned	afinx:BIT_WIDTH(0);	// accumulated inexact flag
} m88k_fpsr_t;

/*
 * FPCR -- Floating Point Control Register
 * 88K architecturally specified form.
 * Does not expose implementation-dependent functions
 */
typedef enum {
	M88K_RM_NEAREST = 0,		// round toward nearest
	M88K_RM_ZERO	= 1,		// round toward zero
	M88K_RM_NEGINF = 2,		// round toward negative infinity
	M88K_RM_POSINF =3		// round toward positive infinity
} m88k_fpcr_rm_t;

typedef struct {
	unsigned	:BITS_WIDTH(31,16);
	m88k_fpcr_rm_t	rm:BITS_WIDTH(15,14);	// rounding mode
	unsigned	:BITS_WIDTH(13,5);
	unsigned	efinv:BIT_WIDTH(4);	// invalid exception enable
	unsigned	efdvz:BIT_WIDTH(3);	// div by zero exception enable
	unsigned	efunf:BIT_WIDTH(2);	// underflow exception enable
	unsigned	efovf:BIT_WIDTH(1);	// overflow exception enable
	unsigned	efinx:BIT_WIDTH(0);	// inexact exception enable
} m88k_fpcr_t;

/*
 * FPCR -- Floating Point Control Register
 * 88110 implementation -- includes "TCFP" features.
 */
typedef struct {
	unsigned	:BITS_WIDTH(31,22);
	unsigned	tcfp:BIT_WIDTH(21);	// def results for INF/NaN
	unsigned	:BITS_WIDTH(20,19);
	unsigned	tcfpunf:BIT_WIDTH(18);	// underflow -> zero
	unsigned	tcfpovf:BIT_WIDTH(17);	// overflow -> inf
	unsigned	:BIT_WIDTH(16);
	m88k_fpcr_rm_t	rm:BITS_WIDTH(15,14);	// rounding mode
	unsigned	:BITS_WIDTH(13,5);
	unsigned	efinv:BIT_WIDTH(4);	// invalid exception enable
	unsigned	efdvz:BIT_WIDTH(3);	// div by zero exception enable
	unsigned	efunf:BIT_WIDTH(2);	// underflow exception enable
	unsigned	efovf:BIT_WIDTH(1);	// overflow exception enable
	unsigned	efinx:BIT_WIDTH(0);	// inexact exception enable
} m88110_fpcr_t;

#ifdef m88k
#ifndef	__STRICT_ANSI__
/*
 * read and write fpsr and fpcr registers
 *
 * FIXME: When the compiler is fixed, convert to style of inlines shown
 * in m88110_sfu0.h which do not use either CONTENTS() macro or
 * *(foo_t *)& casts (and therefore, don't force the compiler to generate
 * a memory reference.
 */
static __inline__ m88k_fpsr_t m88k_get_fpsr()
{
	unsigned	cr_tmp;
	
	__asm__ volatile ("fldcr	 %0,fcr62	; get_fpsr()" : "=r" (cr_tmp));
	return *(m88k_fpsr_t *)&cr_tmp;
}

static __inline__ void m88k_set_fpsr(m88k_fpsr_t fpsr_val)
{
	/*
	 * Must force xmod to 1, since OS uses this to determine
	 * if XRF must be context switched.  Not setting to 1
	 * will NOT corrupt other threads registers, but will
	 * result in loss of this threads register values.
	 */
	fpsr_val.xmod = 1;
	__asm__ volatile ("fstcr	 %0,fcr62	; set_fpsr()"
	  : : "r" (CONTENTS(fpsr_val)));
}

static __inline__ m88k_fpcr_t m88k_get_fpcr()
{
	unsigned	cr_tmp;
	
	__asm__ volatile ("fldcr	 %0,fcr63	; get_fpcr()" : "=r" (cr_tmp));
	return *(m88k_fpcr_t *)&cr_tmp;
}

static __inline__ void m88k_set_fpcr(m88k_fpcr_t fpcr_val)
{
	__asm__ volatile ("fstcr	 %0,fcr63	; set_fpcr()"
	  : : "r" (CONTENTS(fpcr_val)));
}
#endif /* __STRICT_ANSI__ */
#endif /* m88k */

#endif	/* _ARCH_M88K_FP_REGS_H_ */
