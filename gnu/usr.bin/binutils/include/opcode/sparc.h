/* Definitions for opcode table for the sparc.
   Copyright (C) 1989-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler, GDB, the GNU debugger, and
   the GNU Binutils.

   GAS/GDB is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS/GDB is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS or GDB; see the file COPYING3.  If not, write to
   the Free Software Foundation, 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#include "ansidecl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The SPARC opcode table (and other related data) is defined in
   the opcodes library in sparc-opc.c.  If you change anything here, make
   sure you fix up that file, and vice versa.  */

 /* FIXME-someday: perhaps the ,a's and such should be embedded in the
    instruction's name rather than the args.  This would make gas faster, pinsn
    slower, but would mess up some macros a bit.  xoxorich. */

/* List of instruction sets variations.
   These values are such that each element is either a superset of a
   preceding each one or they conflict in which case SPARC_OPCODE_CONFLICT_P
   returns non-zero.
   The values are indices into `sparc_opcode_archs' defined in sparc-opc.c.
   Don't change this without updating sparc-opc.c.  */

enum sparc_opcode_arch_val
{
  SPARC_OPCODE_ARCH_V6 = 0,
  SPARC_OPCODE_ARCH_V7,
  SPARC_OPCODE_ARCH_V8,
  SPARC_OPCODE_ARCH_LEON,
  SPARC_OPCODE_ARCH_SPARCLET,
  SPARC_OPCODE_ARCH_SPARCLITE,
  /* V9 variants must appear last.  */
  SPARC_OPCODE_ARCH_V9,
  SPARC_OPCODE_ARCH_V9A, /* V9 with ultrasparc additions.  */
  SPARC_OPCODE_ARCH_V9B, /* V9 with ultrasparc and cheetah additions.  */
  SPARC_OPCODE_ARCH_V9C, /* V9 with UA2005 and T1 additions.  */
  SPARC_OPCODE_ARCH_V9D, /* V9 with UA2007 and T3 additions.  */
  SPARC_OPCODE_ARCH_V9E, /* V9 with OSA2011 and T4 additions modulus integer multiply-add.  */
  SPARC_OPCODE_ARCH_V9V, /* V9 with OSA2011 and T4 additions, integer
                            multiply and Fujitsu fp multiply-add.  */
  SPARC_OPCODE_ARCH_V9M, /* V9 with OSA2015 and M7 additions.  */
  SPARC_OPCODE_ARCH_M8,  /* V9 with OSA2017 and M8 additions.  */
  SPARC_OPCODE_ARCH_MAX = SPARC_OPCODE_ARCH_M8,
  SPARC_OPCODE_ARCH_BAD  /* Error return from sparc_opcode_lookup_arch.  */
};


/* Given an enum sparc_opcode_arch_val, return the bitmask to use in
   insn encoding/decoding.  */
#define SPARC_OPCODE_ARCH_MASK(arch) (1 << (arch))

/* Given a valid sparc_opcode_arch_val, return non-zero if it's v9.  */
#define SPARC_OPCODE_ARCH_V9_P(arch) ((arch) >= SPARC_OPCODE_ARCH_V9)

/* Table of cpu variants.  */

typedef struct sparc_opcode_arch
{
  const char *name;
  /* Mask of sparc_opcode_arch_val's supported.
     EG: For v7 this would be
     (SPARC_OPCODE_ARCH_MASK (..._V6) | SPARC_OPCODE_ARCH_MASK (..._V7)).
     These are short's because sparc_opcode.architecture is.  */
  short supported;
  /* Bitmaps describing the set of hardware capabilities implemented
     by the opcode arch.  */
  int hwcaps;
  int hwcaps2;
} sparc_opcode_arch;

extern const struct sparc_opcode_arch sparc_opcode_archs[];

/* Given architecture name, look up it's sparc_opcode_arch_val value.  */
extern enum sparc_opcode_arch_val sparc_opcode_lookup_arch (const char *);

/* Return the bitmask of supported architectures for ARCH.  */
#define SPARC_OPCODE_SUPPORTED(ARCH) (sparc_opcode_archs[ARCH].supported)

/* Non-zero if ARCH1 conflicts with ARCH2.
   IE: ARCH1 as a supported bit set that ARCH2 doesn't, and vice versa.  */
#define SPARC_OPCODE_CONFLICT_P(ARCH1, ARCH2) \
 (((SPARC_OPCODE_SUPPORTED (ARCH1) & SPARC_OPCODE_SUPPORTED (ARCH2)) \
   != SPARC_OPCODE_SUPPORTED (ARCH1)) \
  && ((SPARC_OPCODE_SUPPORTED (ARCH1) & SPARC_OPCODE_SUPPORTED (ARCH2)) \
     != SPARC_OPCODE_SUPPORTED (ARCH2)))

/* Structure of an opcode table entry.  */

typedef struct sparc_opcode
{
  const char *name;
  unsigned long match;	/* Bits that must be set.  */
  unsigned long lose;	/* Bits that must not be set.  */
  const char *args;
  /* This was called "delayed" in versions before the flags.  */
  unsigned int flags;
  unsigned int hwcaps;
  unsigned int hwcaps2;
  short architecture;	/* Bitmask of sparc_opcode_arch_val's.  */
} sparc_opcode;

/* Struct for ASIs - to handle ASIs introduced in a specific architecture */
typedef struct
{
  int value;
  const char *name;
  short architecture;
} sparc_asi;

/* FIXME: Add F_ANACHRONISTIC flag for v9.  */
#define	F_DELAYED	0x00000001 /* Delayed branch.  */
#define	F_ALIAS		0x00000002 /* Alias for a "real" instruction.  */
#define	F_UNBR		0x00000004 /* Unconditional branch.  */
#define	F_CONDBR	0x00000008 /* Conditional branch.  */
#define	F_JSR		0x00000010 /* Subroutine call.  */
#define F_FLOAT		0x00000020 /* Floating point instruction (not a branch).  */
#define F_FBR		0x00000040 /* Floating point branch.  */
#define F_PREFERRED	0x00000080 /* A preferred alias.  */

#define F_PREF_ALIAS	(F_ALIAS|F_PREFERRED)

/* These must match the ELF_SPARC_HWCAP_* and ELF_SPARC_HWCAP2_*
   values precisely.  See include/elf/sparc.h.  */
#define HWCAP_MUL32	0x00000001 /* umul/umulcc/smul/smulcc insns */
#define HWCAP_DIV32	0x00000002 /* udiv/udivcc/sdiv/sdivcc insns */
#define HWCAP_FSMULD	0x00000004 /* 'fsmuld' insn */
#define HWCAP_V8PLUS	0x00000008 /* v9 insns available to 32bit */
#define HWCAP_POPC	0x00000010 /* 'popc' insn */
#define HWCAP_VIS	0x00000020 /* VIS insns */
#define HWCAP_VIS2	0x00000040 /* VIS2 insns */
#define HWCAP_ASI_BLK_INIT	\
			0x00000080 /* block init ASIs */
#define HWCAP_FMAF	0x00000100 /* fused multiply-add */
#define HWCAP_VIS3	0x00000400 /* VIS3 insns */
#define HWCAP_HPC	0x00000800 /* HPC insns */
#define HWCAP_RANDOM	0x00001000 /* 'random' insn */
#define HWCAP_TRANS	0x00002000 /* transaction insns */
#define HWCAP_FJFMAU	0x00004000 /* unfused multiply-add */
#define HWCAP_IMA	0x00008000 /* integer multiply-add */
#define HWCAP_ASI_CACHE_SPARING \
			0x00010000 /* cache sparing ASIs */
#define HWCAP_AES	0x00020000 /* AES crypto insns */
#define HWCAP_DES	0x00040000 /* DES crypto insns */
#define HWCAP_KASUMI	0x00080000 /* KASUMI crypto insns */
#define HWCAP_CAMELLIA 	0x00100000 /* CAMELLIA crypto insns */
#define HWCAP_MD5	0x00200000 /* MD5 hashing insns */
#define HWCAP_SHA1	0x00400000 /* SHA1 hashing insns */
#define HWCAP_SHA256	0x00800000 /* SHA256 hashing insns */
#define HWCAP_SHA512	0x01000000 /* SHA512 hashing insns */
#define HWCAP_MPMUL	0x02000000 /* Multiple Precision Multiply */
#define HWCAP_MONT	0x04000000 /* Montgomery Mult/Sqrt */
#define HWCAP_PAUSE	0x08000000 /* Pause insn */
#define HWCAP_CBCOND	0x10000000 /* Compare and Branch insns */
#define HWCAP_CRC32C	0x20000000 /* CRC32C insn */

#define HWCAP2_FJATHPLUS 0x00000001 /* Fujitsu Athena+ */
#define HWCAP2_VIS3B     0x00000002 /* Subset of VIS3 present on sparc64 X+.  */
#define HWCAP2_ADP       0x00000004 /* Application Data Protection */
#define HWCAP2_SPARC5    0x00000008 /* The 29 new fp and sub instructions */
#define HWCAP2_MWAIT     0x00000010 /* mwait instruction and load/monitor ASIs */
#define HWCAP2_XMPMUL    0x00000020 /* XOR multiple precision multiply */
#define HWCAP2_XMONT     0x00000040 /* XOR Montgomery mult/sqr instructions */
#define HWCAP2_NSEC      \
                         0x00000080 /* pause insn with support for nsec timings */
#define HWCAP2_FJATHHPC  0x00001000 /* Fujitsu HPC instrs */
#define HWCAP2_FJDES     0x00002000 /* Fujitsu DES instrs */
#define HWCAP2_FJAES     0x00010000 /* Fujitsu AES instrs */

#define HWCAP2_SPARC6    0x00020000 /* OSA2017 new instructions */
#define HWCAP2_ONADDSUB  0x00040000 /* Oracle Number add/subtract */
#define HWCAP2_ONMUL     0x00080000 /* Oracle Number multiply */
#define HWCAP2_ONDIV     0x00100000 /* Oracle Number divide */
#define HWCAP2_DICTUNP   0x00200000 /* Dictionary unpack instruction */
#define HWCAP2_FPCMPSHL  0x00400000 /* Partition compare with shifted result */
#define HWCAP2_RLE       0x00800000 /* Run-length encoded burst and length */
#define HWCAP2_SHA3      0x01000000 /* SHA3 instruction */


/* All sparc opcodes are 32 bits, except for the `set' instruction (really a
   macro), which is 64 bits. It is handled as a special case.

   The match component is a mask saying which bits must match a particular
   opcode in order for an instruction to be an instance of that opcode.

   The args component is a string containing one character for each operand of the
   instruction.

   Kinds of operands:
	#	Number used by optimizer.	It is ignored.
	1	rs1 register.
	2	rs2 register.
	d	rd register.
	e	frs1 floating point register.
	v	frs1 floating point register (double/even).
	V	frs1 floating point register (quad/multiple of 4).
	;	frs1 floating piont register (multiple of 8).
	f	frs2 floating point register.
	B	frs2 floating point register (double/even).
	R	frs2 floating point register (quad/multiple of 4).
	:	frs2 floating point register (multiple of 8).
	'	rs2m floating point register (double/even) in FPCMPSHL. (m8)
	4	frs3 floating point register.
	5	frs3 floating point register (doube/even).
	g	frsd floating point register.
	H	frsd floating point register (double/even).
	J	frsd floating point register (quad/multiple of 4).
	}       frsd floating point register (double/even) that is == frs2
	^	frsd floating piont register in ON instructions.
	b	crs1 coprocessor register
	c	crs2 coprocessor register
	D	crsd coprocessor register
	m	alternate space register (asr) in rd
	M	alternate space register (asr) in rs1
	h	22 high bits.
	X	5 bit unsigned immediate
	Y	6 bit unsigned immediate
	3	SIAM mode (3 bits). (v9b)
	K	MEMBAR mask (7 bits). (v9)
	j	10 bit Immediate. (v9)
	I	11 bit Immediate. (v9)
	i	13 bit Immediate.
	n	22 bit immediate.
	k	2+14 bit PC relative immediate. (v9)
	G	19 bit PC relative immediate. (v9)
	l	22 bit PC relative immediate.
	L	30 bit PC relative immediate.
	a	Annul.	The annul bit is set.
	A	Alternate address space. Stored as 8 bits.
	C	Coprocessor state register.
	F	floating point state register.
	p	Processor state register.
	N	Branch predict clear ",pn" (v9)
	T	Branch predict set ",pt" (v9)
	z	%icc. (v9)
	Z	%xcc. (v9)
	q	Floating point queue.
	r	Single register that is both rs1 and rd.
	O	Single register that is both rs2 and rd.
	Q	Coprocessor queue.
	S	Special case.
	t	Trap base register.
	w	Window invalid mask register.
	y	Y register.
	u	sparclet coprocessor registers in rd position
	U	sparclet coprocessor registers in rs1 position
	E	%ccr. (v9)
	s	%fprs. (v9)
	P	%pc.  (v9)
	W	%tick.	(v9)
	{	%mcdper. (v9b)
	&	%entropy.  (m8)
	o	%asi. (v9)
	6	%fcc0. (v9)
	7	%fcc1. (v9)
	8	%fcc2. (v9)
	9	%fcc3. (v9)
	!	Privileged Register in rd (v9)
	?	Privileged Register in rs1 (v9)
	%	Hyperprivileged Register in rd (v9b)
	$	Hyperprivileged Register in rs1 (v9b)
	*	Prefetch function constant. (v9)
	x	OPF field (v9 impdep).
	0	32/64 bit immediate for set or setx (v9) insns
	_	Ancillary state register in rd (v9a)
	/	Ancillary state register in rs1 (v9a)
	(	entire floating point state register (%efsr)
	)	5 bit immediate placed in RS3 field
	=	2+8 bit PC relative immediate. (v9)
	|	FPCMPSHL 2 bit immediate. (m8)  */

#define OP2(x)		(((x) & 0x7) << 22)  /* Op2 field of format2 insns.  */
#define OP3(x)		(((x) & 0x3f) << 19) /* Op3 field of format3 insns.  */
#define OP(x)		((unsigned) ((x) & 0x3) << 30) /* Op field of all insns.  */
#define OPF(x)		(((x) & 0x1ff) << 5) /* Opf field of float insns.  */
#define OPF_LOW5(x)	OPF ((x) & 0x1f)     /* V9.  */
#define OPF_LOW4(x)	OPF ((x) & 0xf)      /* V9.  */
#define OPM(x)		(((x) & 0x7) << 10)  /* opm field of misaligned load/store insns.  */
#define OPMI(x)	(((x) & 0x1) << 9)   /* opm i field of misaligned load/store insns.  */
#define ONFCN(x)	(((x) & 0x3) << 26)  /* fcn field of Oracle Number insns.  */
#define REVFCN(x)	(((x) & 0x3) << 0)   /* fcn field of REV* insns.  */
#define F3F(x, y, z)	(OP (x) | OP3 (y) | OPF (z)) /* Format3 float insns.  */
#define F3F4(x, y, z)	(OP (x) | OP3 (y) | OPF_LOW4 (z))
#define F3I(x)		(((x) & 0x1) << 13)  /* Immediate field of format 3 insns.  */
#define F2(x, y)	(OP (x) | OP2(y))    /* Format 2 insns.  */
#define F3(x, y, z)	(OP (x) | OP3(y) | F3I(z)) /* Format3 insns.  */
#define F1(x)		(OP (x))
#define DISP30(x)	((x) & 0x3fffffff)
#define ASI(x)		(((x) & 0xff) << 5)  /* Asi field of format3 insns.  */
#define RS2(x)		((x) & 0x1f)         /* Rs2 field.  */
#define SIMM13(x)	((x) & 0x1fff)       /* Simm13 field.  */
#define SIMM10(x)	((x) & 0x3ff)	     /* Simm10 field.  */
#define RD(x)		(((x) & 0x1f) << 25) /* Destination register field.  */
#define RS1(x)		(((x) & 0x1f) << 14) /* Rs1 field.  */
#define RS3(x)		(((x) & 0x1f) << 9)  /* Rs3 field.  */
#define ASI_RS2(x)	(SIMM13 (x))
#define MEMBAR(x)	((x) & 0x7f)
#define SLCPOP(x)	(((x) & 0x7f) << 6)  /* Sparclet cpop.  */

#define ANNUL	(1 << 29)
#define BPRED	(1 << 19)	/* V9.  */
#define	IMMED	F3I (1)
#define RD_G0	RD (~0)
#define	RS1_G0	RS1 (~0)
#define	RS2_G0	RS2 (~0)

extern const struct sparc_opcode sparc_opcodes[];
extern const int sparc_num_opcodes;

extern const sparc_asi *sparc_encode_asi (const char *);
extern const char *sparc_decode_asi (int);
extern int sparc_encode_membar (const char *);
extern const char *sparc_decode_membar (int);
extern int sparc_encode_prefetch (const char *);
extern const char *sparc_decode_prefetch (int);
extern int sparc_encode_sparclet_cpreg (const char *);
extern const char *sparc_decode_sparclet_cpreg (int);

/* Local Variables:
   fill-column: 131
   comment-column: 0
   End: */

#ifdef __cplusplus
}
#endif
