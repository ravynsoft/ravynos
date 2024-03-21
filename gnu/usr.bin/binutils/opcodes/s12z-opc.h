/* s12z-dis.h -- Header file for s12z-dis.c and s12z-decode.c
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#ifndef S12Z_OPC_H
#define S12Z_OPC_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* An abstraction used to read machine code from a source.  */
struct mem_read_abstraction_base
{
  int (*read) (struct mem_read_abstraction_base *, int, size_t, bfd_byte *);
  void (*advance) (struct mem_read_abstraction_base *);
  bfd_vma (*posn) (struct mem_read_abstraction_base *);
};


/* Machine code operators.
   These *roughly* correspond to opcodes.
   But describe their purpose rather than their form.  */
enum optr
  {
    OP_INVALID = 0,

    OP_push,
    OP_pull,
    /* Test and branch.  */
    OP_tbNE, OP_tbEQ, OP_tbPL, OP_tbMI, OP_tbGT, OP_tbLE,
    /* Decrement and branch.  */
    OP_dbNE, OP_dbEQ, OP_dbPL, OP_dbMI, OP_dbGT, OP_dbLE,

    /* Note: sex and exg are the same opcode.
       They are mnemonic changes according to the operands.  */
    OP_sex,
    OP_exg,

    /* Shifters.  */
    OP_lsl, OP_lsr,
    OP_asl, OP_asr,
    OP_rol, OP_ror,
    /* Bit field operations.  */
    OP_bfins, OP_bfext,
    OP_trap,

    OP_ld,
    OP_st,
    OP_cmp,

    OP_stop,
    OP_wai,
    OP_sys,

    OP_minu,
    OP_mins,
    OP_maxu,
    OP_maxs,

    OP_abs,
    OP_adc,
    OP_bit,
    OP_sbc,
    OP_rti,
    OP_clb,
    OP_eor,

    OP_sat,

    OP_nop,
    OP_bgnd,
    OP_brclr,
    OP_brset,
    OP_rts,
    OP_lea,
    OP_mov,

    OP_bra,
    OP_bsr,
    OP_bhi,
    OP_bls,
    OP_bcc,
    OP_bcs,
    OP_bne,
    OP_beq,
    OP_bvc,
    OP_bvs,
    OP_bpl,
    OP_bmi,
    OP_bge,
    OP_blt,
    OP_bgt,
    OP_ble,
    OP_inc,
    OP_clr,
    OP_dec,

    OP_add,
    OP_sub,
    OP_and,
    OP_or,

    OP_tfr,
    OP_jmp,
    OP_jsr,
    OP_com,
    OP_andcc,
    OP_neg,
    OP_orcc,
    OP_bclr,
    OP_bset,
    OP_btgl,
    OP_swi,

    OP_mulu,
    OP_divu,
    OP_modu,
    OP_macu,
    OP_qmulu,

    OP_muls,
    OP_divs,
    OP_mods,
    OP_macs,
    OP_qmuls,

    OPBASE_mul = 0x4000,
    OPBASE_div,
    OPBASE_mod,
    OPBASE_mac,
    OPBASE_qmul,

    n_OPS
  };


/* Used for operands which mutate their index/base registers.
   Eg  ld d0, (s+).  */
enum op_reg_mutation
  {
    OPND_RM_NONE,
    OPND_RM_PRE_DEC,
    OPND_RM_PRE_INC,
    OPND_RM_POST_DEC,
    OPND_RM_POST_INC
  };

/* The class of an operand.  */
enum opnd_class
  {
    OPND_CL_IMMEDIATE,
    OPND_CL_MEMORY,
    OPND_CL_REGISTER,
    OPND_CL_REGISTER_ALL,   /* Used only for psh/pul.  */
    OPND_CL_REGISTER_ALL16, /* Used only for psh/pul.  */
    OPND_CL_SIMPLE_MEMORY,
    OPND_CL_BIT_FIELD
  };


/* Base structure of all operands.  */
struct operand
{
  enum opnd_class cl;

  /* OSIZE determines the size of memory access for
     the  operation in which the operand participates.
     It may be -1 which indicates either unknown
     (must be determined by other operands) or if
     it is not applicable for this operation.  */
  int osize;
};

/* Immediate operands.  Eg: #23  */
struct immediate_operand
{
  struct operand parent;
  int value;
};

/* Bitfield operands.   Used only in bfext and bfins
   instructions.  */
struct bitfield_operand
{
  struct operand parent;
  int width;
  int offset;
};

/* Register operands.  */
struct register_operand
{
  struct operand parent;
  int reg;
};


/* Simple memory operands.  ie, direct memory,
   no index, no pre/post inc/dec.  May be either relative or absolute.
   Eg st d0, 0x123456  */
struct simple_memory_operand
{
  struct operand parent;

  bfd_vma addr;
  bfd_vma base;
  bool relative;
};


/* Memory operands.    Should be able to represent all memory
   operands in the S12Z instruction set which are not simple
   memory operands.  */
struct memory_operand
{
  struct operand parent;

  /* True for indirect operands: eg [0x123456]   */
  bool indirect;

  /* The value of any offset.  eg 45 in (45,d7) */
    int base_offset;

  /* Does this operand increment or decrement
     its participating registers.  Eg (-s) */
  enum op_reg_mutation mutation;

  /* The number of registers participating in this operand.
     For S12Z this is always in the range [0, 6] (but for most
     instructions it's <= 2).  */
  int n_regs;

  /* The participating registers.  */
  int regs[6];
};


/* Decode a single instruction.
   OPERATOR, OSIZE, N_OPERANDS and OPERANDS are pointers to
   variables which must be provided by the caller.
   N_OPERANDS will be incremented by the number of operands read, so
   you should assign it to something before calling this function.
   OPERANDS must be large enough to contain all operands read
   (which may be up to 6).
   It is the responsibility of the caller to free all operands
   when they are no longer needed.
   Returns the number of bytes read.  */
int decode_s12z (enum optr *myoperator, short *osize,
		 int *n_operands, struct operand **operands,
		 struct mem_read_abstraction_base *);
#ifdef __cplusplus
}
#endif

#endif /* S12Z_OPC_H  */
