/* ppc.h -- Header file for PowerPC opcode table
   Copyright (C) 1994-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor, Cygnus Support

   This file is part of GDB, GAS, and the GNU binutils.

   GDB, GAS, and the GNU binutils are free software; you can redistribute
   them and/or modify them under the terms of the GNU General Public
   License as published by the Free Software Foundation; either version 3,
   or (at your option) any later version.

   GDB, GAS, and the GNU binutils are distributed in the hope that they
   will be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING3.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef PPC_H
#define PPC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t ppc_cpu_t;
typedef uint16_t ppc_opindex_t;

/* Smaller of ppc_opindex_t and fx_pcrel_adjust maximum.  Note that
   values extracted from fx_pcrel_adjust are masked with this constant,
   effectively making the field unsigned.  */
#define PPC_OPINDEX_MAX 0xffff

/* The opcode table is an array of struct powerpc_opcode.  */

struct powerpc_opcode
{
  /* The opcode name.  */
  const char *name;

  /* The opcode itself.  Those bits which will be filled in with
     operands are zeroes.  */
  uint64_t opcode;

  /* The opcode mask.  This is used by the disassembler.  This is a
     mask containing ones indicating those bits which must match the
     opcode field, and zeroes indicating those bits which need not
     match (and are presumably filled in by operands).  */
  uint64_t mask;

  /* One bit flags for the opcode.  These are used to indicate which
     specific processors support the instructions.  The defined values
     are listed below.  */
  ppc_cpu_t flags;

  /* One bit flags for the opcode.  These are used to indicate which
     specific processors no longer support the instructions.  The defined
     values are listed below.  */
  ppc_cpu_t deprecated;

  /* An array of operand codes.  Each code is an index into the
     operand table.  They appear in the order which the operands must
     appear in assembly code, and are terminated by a zero.  */
  ppc_opindex_t operands[8];
};

/* The table itself is sorted by major opcode number, and is otherwise
   in the order in which the disassembler should consider
   instructions.  */
extern const struct powerpc_opcode powerpc_opcodes[];
extern const unsigned int powerpc_num_opcodes;
extern const struct powerpc_opcode prefix_opcodes[];
extern const unsigned int prefix_num_opcodes;
extern const struct powerpc_opcode vle_opcodes[];
extern const unsigned int vle_num_opcodes;
extern const struct powerpc_opcode lsp_opcodes[];
extern const unsigned int lsp_num_opcodes;
extern const struct powerpc_opcode spe2_opcodes[];
extern const unsigned int spe2_num_opcodes;

/* Values defined for the flags field of a struct powerpc_opcode.  */

/* Opcode is defined for the PowerPC architecture.  */
#define PPC_OPCODE_PPC		       0x1ull

/* Opcode is defined for the POWER (RS/6000) architecture.  */
#define PPC_OPCODE_POWER	       0x2ull

/* Opcode is defined for the POWER2 (Rios 2) architecture.  */
#define PPC_OPCODE_POWER2	       0x4ull

/* Opcode is only defined on 64 bit architectures.  */
#define PPC_OPCODE_64		       0x8ull

/* Opcode is supported by the Motorola PowerPC 601 processor.  The 601
   is assumed to support all PowerPC (PPC_OPCODE_PPC) instructions,
   but it also supports many additional POWER instructions.  */
#define PPC_OPCODE_601		      0x10ull

/* Opcode is supported in both the Power and PowerPC architectures
   (ie, compiler's -mcpu=common or assembler's -mcom).  More than just
   the intersection of PPC_OPCODE_PPC with the union of PPC_OPCODE_POWER
   and PPC_OPCODE_POWER2 because many instructions changed mnemonics
   between POWER and POWERPC.  */
#define PPC_OPCODE_COMMON	      0x20ull

/* Opcode is supported for any Power or PowerPC platform (this is
   for the assembler's -many option, and it eliminates duplicates).  */
#define PPC_OPCODE_ANY		      0x40ull

/* Opcode is supported as part of the 64-bit bridge.  */
#define PPC_OPCODE_64_BRIDGE	      0x80ull

/* Opcode is supported by Altivec Vector Unit */
#define PPC_OPCODE_ALTIVEC	     0x100ull

/* Opcode is supported by PowerPC 403 processor.  */
#define PPC_OPCODE_403		     0x200ull

/* Opcode is supported by PowerPC BookE processor.  */
#define PPC_OPCODE_BOOKE	     0x400ull

/* Opcode is only supported by Power4 architecture.  */
#define PPC_OPCODE_POWER4	     0x800ull

/* Opcode is only supported by e500x2 Core.
   This bit, PPC_OPCODE_EFS, PPC_OPCODE_VLE, and all those with APU in
   their comment mark opcodes so that when those instructions are used
   an APUinfo entry can be generated.  */
#define PPC_OPCODE_SPE		    0x1000ull

/* Opcode is supported by Integer select APU.  */
#define PPC_OPCODE_ISEL		    0x2000ull

/* Opcode is an e500 SPE floating point instruction.  */
#define PPC_OPCODE_EFS		    0x4000ull

/* Opcode is supported by branch locking APU.  */
#define PPC_OPCODE_BRLOCK	    0x8000ull

/* Opcode is supported by performance monitor APU.  */
#define PPC_OPCODE_PMR		   0x10000ull

/* Opcode is supported by cache locking APU.  */
#define PPC_OPCODE_CACHELCK	   0x20000ull

/* Opcode is supported by machine check APU.  */
#define PPC_OPCODE_RFMCI	   0x40000ull

/* Opcode is supported by PowerPC 440 processor.  */
#define PPC_OPCODE_440		   0x80000ull

/* Opcode is only supported by Power5 architecture.  */
#define PPC_OPCODE_POWER5	  0x100000ull

/* Opcode is supported by PowerPC e300 family.  */
#define PPC_OPCODE_E300		  0x200000ull

/* Opcode is only supported by Power6 architecture.  */
#define PPC_OPCODE_POWER6	  0x400000ull

/* Opcode is only supported by PowerPC Cell family.  */
#define PPC_OPCODE_CELL		  0x800000ull

/* Opcode is supported by CPUs with paired singles support.  */
#define PPC_OPCODE_PPCPS	 0x1000000ull

/* Opcode is supported by Power E500MC */
#define PPC_OPCODE_E500MC	 0x2000000ull

/* Opcode is supported by PowerPC 405 processor.  */
#define PPC_OPCODE_405		 0x4000000ull

/* Opcode is supported by Vector-Scalar (VSX) Unit */
#define PPC_OPCODE_VSX		 0x8000000ull

/* Opcode is only supported by Power7 architecture.  */
#define PPC_OPCODE_POWER7	0x10000000ull

/* Opcode is supported by A2.  */
#define PPC_OPCODE_A2		0x20000000ull

/* Opcode is supported by PowerPC 476 processor.  */
#define PPC_OPCODE_476		0x40000000ull

/* Opcode is supported by AppliedMicro Titan core */
#define PPC_OPCODE_TITAN	0x80000000ull

/* Opcode which is supported by the e500 family */
#define PPC_OPCODE_E500        0x100000000ull

/* Opcode is supported by Power E6500 */
#define PPC_OPCODE_E6500       0x200000000ull

/* Opcode is supported by Thread management APU */
#define PPC_OPCODE_TMR	       0x400000000ull

/* Opcode which is supported by the VLE extension.  */
#define PPC_OPCODE_VLE	       0x800000000ull

/* Opcode is only supported by Power8 architecture.  */
#define PPC_OPCODE_POWER8     0x1000000000ull

/* Opcode is supported by ppc750cl/Gekko/Broadway.  */
#define PPC_OPCODE_750	      0x2000000000ull

/* Opcode is supported by ppc7450.  */
#define PPC_OPCODE_7450       0x4000000000ull

/* Opcode is supported by ppc821/850/860.  */
#define PPC_OPCODE_860	      0x8000000000ull

/* Opcode is only supported by Power9 architecture.  */
#define PPC_OPCODE_POWER9    0x10000000000ull

/* Opcode is supported by e200z4.  */
#define PPC_OPCODE_E200Z4    0x20000000000ull

/* Disassemble to instructions matching later in the opcode table
   with fewer "mask" bits set rather than the earlist match.  Fewer
   "mask" bits set imply a more general form of the opcode, in fact
   the underlying machine instruction.  */
#define PPC_OPCODE_RAW	     0x40000000000ull

/* Opcode is supported by PowerPC LSP */
#define PPC_OPCODE_LSP	     0x80000000000ull

/* Opcode is only supported by Freescale SPE2 APU.  */
#define PPC_OPCODE_SPE2	    0x100000000000ull

/* Opcode is supported by EFS2.  */
#define PPC_OPCODE_EFS2	    0x200000000000ull

/* Opcode is only supported by power10 architecture.  */
#define PPC_OPCODE_POWER10  0x400000000000ull

/* Opcode is only supported by SVP64 extensions (LibreSOC architecture).  */
#define PPC_OPCODE_SVP64    0x800000000000ull

/* Opcode is only supported by 'future' architecture.  */
#define PPC_OPCODE_FUTURE  0x1000000000000ull

/* A macro to extract the major opcode from an instruction.  */
#define PPC_OP(i) (((i) >> 26) & 0x3f)

/* A macro to determine if the instruction is a 2-byte VLE insn.  */
#define PPC_OP_SE_VLE(m) ((m) <= 0xffff)

/* A macro to extract the major opcode from a VLE instruction.  */
#define VLE_OP(i,m) (((i) >> ((m) <= 0xffff ? 10 : 26)) & 0x3f)

/* A macro to convert a VLE opcode to a VLE opcode segment.  */
#define VLE_OP_TO_SEG(i) ((i) >> 1)

/* Map LSP insn to lookup segment for disassembly.  */
#define LSP_OP_TO_SEG(i) (((i) & 0x7ff) >> 6)

/* A macro to extract the extended opcode from a SPE2 instruction.  */
#define SPE2_XOP(i) ((i) & 0x7ff)

/* A macro to convert a SPE2 extended opcode to a SPE2 xopcode segment.  */
#define SPE2_XOP_TO_SEG(i) ((i) >> 7)

/* A macro to extract the prefix word from an 8-byte PREFIX instruction.  */
#define PPC_GET_PREFIX(i) (((i) >> 32) & ((1LL << 32) - 1))

/* A macro to extract the suffix word from an 8-byte PREFIX instruction.  */
#define PPC_GET_SUFFIX(i) ((i) & ((1LL << 32) - 1))

/* A macro to determine whether insn I is an 8-byte prefix instruction.  */
#define PPC_PREFIX_P(i) (PPC_OP (PPC_GET_PREFIX (i)) == 0x1)

/* A macro used to hash 8-byte PREFIX instructions.  */
#define PPC_PREFIX_SEG(i) (PPC_OP (i) >> 1)


/* The operands table is an array of struct powerpc_operand.  */

struct powerpc_operand
{
  /* A bitmask of bits in the operand.  */
  uint64_t bitm;

  /* The shift operation to be applied to the operand.  No shift
     is made if this is zero.  For positive values, the operand
     is shifted left by SHIFT.  For negative values, the operand
     is shifted right by -SHIFT.  Use PPC_OPSHIFT_INV to indicate
     that BITM and SHIFT cannot be used to determine where the
     operand goes in the insn.  */
  int shift;

  /* Insertion function.  This is used by the assembler.  To insert an
     operand value into an instruction, check this field.

     If it is NULL, execute
	 if (o->shift >= 0)
	   i |= (op & o->bitm) << o->shift;
	 else
	   i |= (op & o->bitm) >> -o->shift;
     (i is the instruction which we are filling in, o is a pointer to
     this structure, and op is the operand value).

     If this field is not NULL, then simply call it with the
     instruction and the operand value.  It will return the new value
     of the instruction.  If the operand value is illegal, *ERRMSG
     will be set to a warning string (the operand will be inserted in
     any case).  If the operand value is legal, *ERRMSG will be
     unchanged (most operands can accept any value).  */
  uint64_t (*insert)
    (uint64_t instruction, int64_t op, ppc_cpu_t dialect, const char **errmsg);

  /* Extraction function.  This is used by the disassembler.  To
     extract this operand type from an instruction, check this field.

     If it is NULL, compute
	 if (o->shift >= 0)
	   op = (i >> o->shift) & o->bitm;
	 else
	   op = (i << -o->shift) & o->bitm;
	 if ((o->flags & PPC_OPERAND_SIGNED) != 0)
	   sign_extend (op);
     (i is the instruction, o is a pointer to this structure, and op
     is the result).

     If this field is not NULL, then simply call it with the
     instruction value.  It will return the value of the operand.
     *INVALID will be set to one by the extraction function if this
     operand type can not be extracted from this operand (i.e., the
     instruction does not match).  If the operand is valid, *INVALID
     will not be changed.  *INVALID will always be non-negative when
     used to extract a field from an instruction.

     The extraction function is also called by both the assembler and
     disassembler if an operand is optional, in which case the
     function should return the default value of the operand.
     *INVALID is negative in this case, and is the negative count of
     omitted optional operands up to and including this operand.  */
  int64_t (*extract) (uint64_t instruction, ppc_cpu_t dialect, int *invalid);

  /* One bit syntax flags.  */
  unsigned long flags;
};

/* Elements in the table are retrieved by indexing with values from
   the operands field of the powerpc_opcodes table.  */

extern const struct powerpc_operand powerpc_operands[];
extern const unsigned int num_powerpc_operands;

/* Use with the shift field of a struct powerpc_operand to indicate
   that BITM and SHIFT cannot be used to determine where the operand
   goes in the insn.  */
#define PPC_OPSHIFT_INV (1U << 30)
/* A special case, 6-bit SH field.  */
#define PPC_OPSHIFT_SH6 (2U << 30)

/* Values defined for the flags field of a struct powerpc_operand.
   Keep the register bits low:  They need to fit in an unsigned short.  */

/* This operand names a register.  The disassembler uses this to print
   register names with a leading 'r'.  */
#define PPC_OPERAND_GPR (0x1)

/* Like PPC_OPERAND_GPR, but don't print a leading 'r' for r0.  */
#define PPC_OPERAND_GPR_0 (0x2)

/* This operand names a floating point register.  The disassembler
   prints these with a leading 'f'.  */
#define PPC_OPERAND_FPR (0x4)

/* This operand names a vector unit register.  The disassembler
   prints these with a leading 'v'.  */
#define PPC_OPERAND_VR (0x8)

/* This operand names a vector-scalar unit register.  The disassembler
   prints these with a leading 'vs'.  */
#define PPC_OPERAND_VSR (0x10)

/* This operand names a VSX accumulator.  */
#define PPC_OPERAND_ACC (0x20)

/* This operand names a dense math register.  */
#define PPC_OPERAND_DMR (0x40)

/* This operand may use the symbolic names for the CR fields (even
   without -mregnames), which are
       lt  0	gt  1	eq  2	so  3	un  3
       cr0 0	cr1 1	cr2 2	cr3 3
       cr4 4	cr5 5	cr6 6	cr7 7
   These may be combined arithmetically, as in cr2*4+gt.  These are
   only supported on the PowerPC, not the POWER.  */
#define PPC_OPERAND_CR_BIT (0x80)

/* This is a CR FIELD that does not use symbolic names (unless
   -mregnames is in effect).  If both PPC_OPERAND_CR_BIT and
   PPC_OPERAND_CR_REG are set then treat the field as per
   PPC_OPERAND_CR_BIT for assembly, but as if neither of these
   bits are set for disassembly.  */
#define PPC_OPERAND_CR_REG (0x100)

/* This operand names a special purpose register.  */
#define PPC_OPERAND_SPR (0x200)

/* This operand names a paired-single graphics quantization register.  */
#define PPC_OPERAND_GQR (0x400)

/* This operand is a relative branch displacement.  The disassembler
   prints these symbolically if possible.  */
#define PPC_OPERAND_RELATIVE (0x800)

/* This operand is an absolute branch address.  The disassembler
   prints these symbolically if possible.  */
#define PPC_OPERAND_ABSOLUTE (0x1000)

/* This operand takes signed values.  */
#define PPC_OPERAND_SIGNED (0x2000)

/* This operand takes signed values, but also accepts a full positive
   range of values when running in 32 bit mode.  That is, if bits is
   16, it takes any value from -0x8000 to 0xffff.  In 64 bit mode,
   this flag is ignored.  */
#define PPC_OPERAND_SIGNOPT (0x4000)

/* The next operand should be wrapped in parentheses rather than
   separated from this one by a comma.  This is used for the load and
   store instructions which want their operands to look like
       reg,displacement(reg)
   */
#define PPC_OPERAND_PARENS (0x8000)

/* This operand is for the DS field in a DS form instruction.  */
#define PPC_OPERAND_DS (0x10000)

/* This operand is for the DQ field in a DQ form instruction.  */
#define PPC_OPERAND_DQ (0x20000)

/* This operand should be regarded as a negative number for the
   purposes of overflow checking (i.e., the normal most negative
   number is disallowed and one more than the normal most positive
   number is allowed).  This flag will only be set for a signed
   operand.  */
#define PPC_OPERAND_NEGATIVE (0x40000)

/* Valid range of operand is 0..n rather than 0..n-1.  */
#define PPC_OPERAND_PLUS1 (0x80000)

/* This operand is optional, and is zero if omitted.  This is used for
   example, in the optional BF field in the comparison instructions.  The
   assembler must count the number of operands remaining on the line,
   and the number of operands remaining for the opcode, and decide
   whether this operand is present or not.  The disassembler should
   print this operand out only if it is not zero.  */
#define PPC_OPERAND_OPTIONAL (0x100000)

/* This flag is only used with PPC_OPERAND_OPTIONAL.  If this operand
   is omitted, then for the next operand use this operand value plus
   1, ignoring the next operand field for the opcode.  This wretched
   hack is needed because the Power rotate instructions can take
   either 4 or 5 operands.  The disassembler should print this operand
   out regardless of the PPC_OPERAND_OPTIONAL field.  */
#define PPC_OPERAND_NEXT (0x200000)

/* This flag is only used with PPC_OPERAND_OPTIONAL.  The operand is
   only optional when generating 32-bit code.  */
#define PPC_OPERAND_OPTIONAL32 (0x400000)

/* Xilinx APU and FSL related operands */
#define PPC_OPERAND_FSL (0x800000)
#define PPC_OPERAND_FCR (0x1000000)
#define PPC_OPERAND_UDI (0x2000000)

/* Valid range of operand is 1..n rather than 0..n-1.
   Before encoding, the operand value is decremented.
   After decoding, the operand value is incremented.  */
#define PPC_OPERAND_NONZERO (0x4000000)

extern ppc_cpu_t ppc_parse_cpu (ppc_cpu_t, ppc_cpu_t *, const char *);

static inline int64_t
ppc_optional_operand_value (const struct powerpc_operand *operand,
			    uint64_t insn,
			    ppc_cpu_t dialect,
			    int num_optional)
{
  if (operand->extract)
    return (*operand->extract) (insn, dialect, &num_optional);
  return 0;
}

/* PowerPC VLE insns.  */
#define E_OPCODE_MASK		0xfc00f800

/* Form I16L, uses 16A relocs.  */
#define E_OR2I_INSN		0x7000C000
#define E_AND2I_DOT_INSN	0x7000C800
#define E_OR2IS_INSN		0x7000D000
#define E_LIS_INSN		0x7000E000
#define	E_AND2IS_DOT_INSN	0x7000E800

/* Form I16A, uses 16D relocs.  */
#define E_ADD2I_DOT_INSN	0x70008800
#define E_ADD2IS_INSN		0x70009000
#define E_CMP16I_INSN		0x70009800
#define E_MULL2I_INSN		0x7000A000
#define E_CMPL16I_INSN		0x7000A800
#define E_CMPH16I_INSN		0x7000B000
#define E_CMPHL16I_INSN		0x7000B800

#define E_LI_INSN		0x70000000
#define E_LI_MASK		0xfc008000

#ifdef __cplusplus
}
#endif

#endif /* PPC_H */
