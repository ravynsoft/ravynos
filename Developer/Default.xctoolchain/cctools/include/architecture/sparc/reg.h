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
/*	@(#)reg.h 1.16 92/04/28 SMI	*/

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#ifndef _sparc_reg_h
#define	_sparc_reg_h

/*
 * Location of the users' stored
 * registers relative to R0.
 * Usage is u.u_ar0[XX].
 */
#define	PSR	(0)
#define	SPARC_PC	(1)
#define	nPC	(2)
#define	SPARC_Y	(3)
#define	G1	(4)
#define	G2	(5)
#define	G3	(6)
#define	G4	(7)
#define	G5	(8)
#define	G6	(9)
#define	G7	(10)
#define	O0	(11)
#define	O1	(12)
#define	O2	(13)
#define	O3	(14)
#define	O4	(15)
#define	O5	(16)
#define	O6	(17)
#define	O7	(18)

/* the following defines are for portability */
#define	PS	PSR
#define	SP	O6
#define	R0	O0
#define	R1	O1

/*
 * And now for something completely the same...
 */
#ifndef __ASSEMBLER__
struct regs {
	int	r_psr;		/* processor status register */
	int	r_pc;		/* program counter */
	int	r_npc;		/* next program counter */
	int	r_y;		/* the y register */
	int	r_g1;		/* user global regs */
	int	r_g2;
	int	r_g3;
	int	r_g4;
	int	r_g5;
	int	r_g6;
	int	r_g7;
	int	r_o0;
	int	r_o1;
	int	r_o2;
	int	r_o3;
	int	r_o4;
	int	r_o5;
	int	r_o6;
	int	r_o7;
};

#define	r_ps	r_psr		/* for portablility */
#define	r_r0	r_o0
#define	r_sp	r_o6

#endif /* !__ASSEMBLER__ */

/*
 * Floating point definitions.
 */

#define	FPU			/* we have an external float unit */

#ifndef __ASSEMBLER__

#define	FQ_DEPTH	16		/* maximum instuctions in FQ */

/*
 * struct fpu_status is the floating point processor state
 * struct fpu is the sum total of all possible floating point state
 * which includes the state of external floating point hardware,
 * fpa registers, etc..., if it exists.
 */
struct fpq {
	unsigned int *addr;		/* address */
	unsigned int instr;		/* instruction */
};
struct	fq {
	union {				/* FPU inst/addr queue */
		double	whole;
		struct  fpq fpq;
	} FQu;
};


#define	FPU_REGS_TYPE unsigned
#define	FPU_FSR_TYPE unsigned

struct	fpu {
	union {				 /* FPU floating point regs */
		FPU_REGS_TYPE Fpu_regs[32];	/* 32 singles */
		double	Fpu_dregs[16];		/* 16 doubles */
	} fpu_fr;
	FPU_FSR_TYPE Fpu_fsr;		/* FPU status register */
	unsigned Fpu_flags;		/* control flags */
	unsigned Fpu_extra;		/* extra word */
	unsigned Fpu_qcnt;		/* count of valid entries in fps_q */
	struct fq Fpu_q[FQ_DEPTH];	/* FPU instruction address queue */
};

#define	fpu_regs	fpu_fr.Fpu_regs
#define	fpu_dregs	fpu_fr.Fpu_dregs
#define	fpu_fsr		Fpu_fsr
#define	fpu_flags	Fpu_flags
#define	fpu_extra	Fpu_extra
#define	fpu_q		Fpu_q
#define	fpu_qcnt	Fpu_qcnt

#endif /* !__ASSEMBLER__ */


/*
 * Definition of bits in the Sun-4 FSR (Floating-point Status Register)
 *   ________________________________________________________________________
 *  |  RD |  RP | TEM | NS | res | vers | FTT | QNE | PR | FCC | AEXC | CEXC |
 *  |-----|---- |-----|----|-----|------|-----|-----|----|-----|------|------|
 *   31 30 29 28 27 23  22  21 20 19  17 16 14   13   12  11 10 9    5 4    0
 */
#define	FSR_CEXC	0x0000001f	/* Current Exception */
#define	FSR_AEXC	0x000003e0	/* ieee accrued exceptions */
#define	FSR_FCC		0x00000c00	/* Floating-point Condition Codes */
#define	FSR_PR		0x00001000	/* Partial Remainder */
#define	FSR_QNE		0x00002000	/* Queue not empty */
#define	FSR_FTT		0x0001c000	/* Floating-point Trap Type */
#define FSR_VERS	0x000e0000	/* version field */
#define FSR_RESV	0x00300000	/* reserved */
#define FSR_NS		0x00400000	/* non-standard fp */
#define FSR_TEM		0x0f800000	/* ieee Trap Enable Mask */
#define	FSR_RP		0x30000000	/* Rounding Precision */
#define	FSR_RD		0xc0000000	/* Rounding Direction */

#define FSR_VERS_SHIFT	(17)		/* amount to shift version field */

/*
 * Definition of CEXC (Current EXCeption) bit field of fsr
 */
#define	FSR_CEXC_NX	0x00000001	/* inexact */
#define	FSR_CEXC_DZ	0x00000002	/* divide-by-zero */
#define	FSR_CEXC_UF	0x00000004	/* underflow */.
#define	FSR_CEXC_OF	0x00000008	/* overflow */
#define	FSR_CEXC_NV	0x00000010	/* invalid */

/*
 * Definition of AEXC (Accrued EXCeption) bit field of fsr
 */
#define	FSR_AEXC_NX	(0x1 << 5)	/* inexact */
#define	FSR_AEXC_DZ	(0x2 << 5)	/* divide-by-zero */
#define	FSR_AEXC_UF	(0x4 << 5)	/* underflow */.
#define	FSR_AEXC_OF	(0x8 << 5)	/* overflow */
#define	FSR_AEXC_NV	(0x10 << 5)	/* invalid */

/*
 * Defintion of FTT (Floating-point Trap Type) field within the FSR
 */
#define	FTT_NONE	0		/* no excepitons */
#define	FTT_IEEE	1		/* IEEE exception */
#define	FTT_UNFIN	2		/* unfinished fpop */
#define	FTT_UNIMP	3		/* unimplemented fpop */
#define	FTT_SEQ		4		/* sequence error */
#define	FTT_ALIGN	5	/* alignment, by software convention */
#define	FTT_DFAULT	6	/* data fault, by software convention */
#define	FSR_FTT_SHIFT	14	/* shift needed to justfy ftt field */
#define	FSR_FTT_IEEE	(FTT_IEEE   << FSR_FTT_SHIFT)
#define	FSR_FTT_UNFIN	(FTT_UNFIN  << FSR_FTT_SHIFT)
#define	FSR_FTT_UNIMP	(FTT_UNIMP  << FSR_FTT_SHIFT)
#define	FSR_FTT_SEQ	(FTT_SEQ    << FSR_FTT_SHIFT)
#define	FSR_FTT_ALIGN	(FTT_ALIGN  << FSR_FTT_SHIFT)
#define	FSR_FTT_DFAULT	(FTT_DFAULT << FSR_FTT_SHIFT)

/*
 * Values of VERS (version) field within the FSR
 * NOTE: these values are overloaded; the cpu type must be used to
 * further discriminate amongst these.  For that reason, no #defines are
 * provided.
 *
 * Version	cpu = 21-22, 51-54		cpu = 23-24, 55-57
 *	0	Weitek 1164/5 (FAB 1-4)		TI 8847
 *	1	Weitek 1164/5 (FAB 5-6)		LSI L64814
 *	2	TI 8847				TI TMS390C602A
 *	3	Weitek 3170			Weitek 3171
 *	4	Meiko				?
 *	5	?				?
 *	6	?				?
 *	7	No FP Hardware			No FP Hardware
 */


/*
 * Definition of TEM (Trap Enable Mask) bit field of fsr
 */
#define	FSR_TEM_NX	(0x1 << 23)	/* inexact */
#define	FSR_TEM_DZ	(0x2 << 23)	/* divide-by-zero */
#define	FSR_TEM_UF	(0x4 << 23)	/* underflow */.
#define	FSR_TEM_OF	(0x8 << 23)	/* overflow */
#define	FSR_TEM_NV	(0x10 << 23)	/* invalid */

/*
 * Definition of RP (Rounding Precision) field of fsr
 */
#define	RP_DBLEXT	0		/* double-extended */
#define	RP_SINGLE	1		/* single */
#define	RP_DOUBLE	2		/* double */
#define	RP_RESERVED	3		/* unused and reserved */

/*
 * Defintion of RD (Rounding Direction) field of fsr
 */
#define	RD_NEAR		0		/* nearest or even if tie */
#define	RD_ZER0		1		/* to zero */
#define	RD_POSINF	2		/* positive infinity */
#define	RD_NEGINF	3		/* negative infinity */

/*
 * Definition of the FP enable flags of the pcb struct
 * Normal operation, all flags are zero
 */
#define	FP_UNINITIALIZED	1
#define	FP_STARTSIG		2
#define	FP_DISABLE		4
#define	FP_ENABLE		8

#ifndef __ASSEMBLER__
/*
 * How a register window looks on the stack.
 */
struct rwindow {
	int	rw_local[8];		/* locals */
	int	rw_in[8];		/* ins */
};

#define	rw_fp	rw_in[6]		/* frame pointer */
#define	rw_rtn	rw_in[7]		/* return address */

#endif /* !__ASSEMBLER__ */


/*
 * Definition of bits in the Sun-4 PSR (Processor Status Register)
 *   ____________________________________________________________________
 *  |    IMPL   | VER |  ICC | res | EC | EF |  PIL  | S | PS | ET | CWP |
 *  |-----------|-----|------|-----|----|----|-------|---|----|----|-----|
 *   31       28 27  24 23  20 19 14 13   12  11    8  7    6    5  4    0
 */


struct p_status {
  union {
    unsigned int psr;
    struct {
	unsigned int 
	impl:4,
	ver:4,
	icc:4,
	reserved:6,
	ec:1,
	ef:1,
	pil:4,
	s:1,
	ps:1,
	et:1,
	cwp:5;
      } psr_bits;
  } PSRREG;
};

struct f_status {
  union {
    FPU_FSR_TYPE Fpu_fsr;		/* FPU status register */
    struct {
      unsigned int
      rd:2,
      rp:2,
      tem:5,
      res:6,
      ftt:3,
      qne:1,
      pr:1,
      fcc:2,
      aexc:5,
      cexc:5;
    } Fpu_fsr_bits;
  } FPUREG;
};

#endif /*!_sparc_reg_h*/
