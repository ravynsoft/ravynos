/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/*
 * This file gives definitions supplementing <a.out.h>
 * for debugging symbol table entries.
 * These entries must have one of the N_STAB bits on,
 * and are subject to relocation according to the masks in <a.out.h>
 * on 4.x (stabs not relocated on SVR4).
 */

#ifndef _STAB_H
#define _STAB_H

/* this file also contains fragments of a.out.h relevant to
 * support of stab processing within ELF files
 * (when a.out.h is not available)
 */
struct stab
{
  unsigned n_strx;      /* index into file string table */
  unsigned char n_type; /* type flag (N_TEXT,..)  */
  char n_other;         /* used by N_SLINE stab */
  short n_desc;         /* see stabs documentation */
  unsigned n_value;     /* value of symbol (or sdb offset) */
};

/* patchtypes for N_PATCH stab (n_desc field) */
#define P_BITFIELD          0x1
#define P_SPILL             0x2
#define P_SCOPY             0x3

/* markers for N_CODETAG stab (n_other field) */
#define CODETAG_BITFIELD    0x1 /* load/store of a bit field */
#define CODETAG_SPILL       0x2 /* spill of registers */
#define CODETAG_SCOPY       0x3 /* structure copy load/store */
#define CODETAG_FSTART      0x4 /* points to first inst of new frame (0==leaf)*/
#define CODETAG_END_CTORS   0x5 /* end of calls to super-class constructors */
/* UNUSED 0x6 DW_ATCF_SUN_branch_target in dwarf, not used in stabs */
#define CODETAG_STACK_PROBE 0x7 /* marks insns which probe the stack memory */

/*
 * Simple values for n_type.
 */
#define N_UNDF      0x0     /* undefined */
#define N_ABS       0x2     /* absolute */
#define N_TEXT      0x4     /* text */
#define N_DATA      0x6     /* data */
#define N_BSS       0x8     /* bss */
#define N_COMM      0x12    /* common (internal to ld) */
#define N_FN        0x1f    /* file name symbol */
#define N_EXT       01      /* external bit, or'ed in */
#define N_TYPE      0x1e    /* mask for all the type bits */

/*
 * maximum length of stab string before using continuation stab.
 *   (this is just a suggested limit), assembler has no limit.
 */
#define MAX_STAB_STR_LEN 250

/*
 * for symbolic debuggers:
 */
#define N_GSYM      0x20  /* global symbol: name,,0,type,0 */
#define N_FNAME     0x22  /* procedure name (f77 kludge): name,,0 */
#define N_FUN       0x24  /* procedure: name,,0,linenumber,0 */
#define N_OUTL      0x25  /* outlined func: name,,0,linenumber,0 */
#define N_STSYM     0x26  /* static symbol: name,,0,type,0 or section relative */
#define N_TSTSYM    0x27  /* thread static symbol: Ttdata.data */
#define N_LCSYM     0x28  /* .lcomm symbol: name,,0,type,0 or section relative */
#define N_TLCSYM    0x29  /* thread local symbol: Ttbss.bss */
#define N_MAIN      0x2a  /* name of main routine : name,,0,0,0 */
#define N_ROSYM     0x2c  /* ro_data: name,,0,type,0 or section relative */
#define N_FLSYM     0x2e  /* fragmented data: name,,0,type,0 */
#define N_TFLSYM    0x2f  /* thread fragmented data: name,,0,type,0 */
#define N_PC        0x30  /* global pascal symbol: name,,0,subtype,line */
#define N_CMDLINE   0x34  /* command line info */
#define N_OBJ       0x38  /* object file path or name */
#define N_OPT       0x3c  /* compiler options */
#define N_RSYM      0x40  /* register sym: name,,0,type,register */
#define N_SLINE     0x44  /* src line: 0,,0,linenumber,function relative */
#define N_XLINE     0x45  /* h.o. src line: 0,,0,linenumber>>16,0 */
#define N_ILDPAD    0x4c  /* now used as ild pad stab value=strtab delta
			   * was designed for "function start.end" */
#define N_SSYM      0x60  /* structure elt: name,,0,type,struct_offset */
#define N_ENDM      0x62  /* last stab emitted for object module */
#define N_SO        0x64  /* source file name: name,,0,0,0 */
#define N_MOD       0x66  /* f90 module: name,,0,0,0 */
#define N_EMOD      0x68  /* end of f90 module: name,,0,0,0 */
#define N_READ_MOD  0x6a  /* use of f90 module: name;locallist,,0,0,0 */
#define N_ALIAS     0x6c  /* alias name: name,,0,0,0 */
#define N_LSYM      0x80  /* local sym: name,,0,type,offset */
#define N_BINCL     0x82  /* header file: name,,0,0,0 */
#define N_SOL       0x84  /* #included file name: name,,0,0,0 */
#define N_PSYM      0xa0  /* parameter: name,,0,type,offset */
#define N_EINCL     0xa2  /* end of include file */
#define N_ENTRY     0xa4  /* alternate entry: name,linenumber,0 */
#define N_SINCL     0xa6  /* shared include file */
#define N_LBRAC     0xc0  /* left bracket: 0,,0,nesting level,function relative */
#define N_EXCL      0xc2  /* excluded include file */
#define N_USING     0xc4  /* C++ using command */
#define N_ISYM      0xc6  /* position independent type symbol, internal */
#define N_ESYM      0xc8  /* position independent type symbol, external */
#define N_PATCH     0xd0  /* Instruction to be ignored by run-time checking. */
#define N_CONSTRUCT 0xd2  /* C++ constructor call. */
#define N_DESTRUCT  0xd4  /* C++ destructor call. */
#define N_CODETAG   0xd8  /* Generic code tag */
#define N_FUN_CHILD 0xd9  /* Identifies a child function */
#define N_RBRAC     0xe0  /* right bracket: 0,,0,nesting level,function relative */
#define N_BCOMM     0xe2  /* begin common: name,, */
#define N_TCOMM     0xe3  /* begin task common: name,, */
#define N_ECOMM     0xe4  /* end task_common/common: name,, */
#define N_XCOMM     0xe6  /* excluded common block */
#define N_ECOML     0xe8  /* end common (local name): ,,address */
#define N_WITH      0xea  /* pascal with statement: type,,0,0,offset */
#define N_LENG      0xfe  /* second stab entry with length information */

/*
 * for analyzer (cache profile feedback support)
 */
#define N_CPROF     0xf0  /* annotation for cache profile feedback */

/*
 * n_descr values used in N_CPROF stabs.  The n_descr field of
 * an N_CPROF stab identifies the type of table whose location
 * is defined by the N_CPROF stab.
 */
typedef enum n_cprof_instr_type_t
{
  N_CPROF_INSTR_TYPE_LOAD = 0,  /* profiled load ops */
  N_CPROF_INSTR_TYPE_STORE,     /* profiled store ops */
  N_CPROF_INSTR_TYPE_PREFETCH,  /* profiled prefetch ops */
  N_CPROF_INSTR_TYPE_BRTARGET,  /* branch target locations */
  N_CPROF_INSTR_TYPE_NTYPES     /* number of types */
} n_cprof_instr_type_t;

/*
 * for code browser only
 */
#define N_BROWS 0x48  /* path to associated .cb file */

/*
 * for functions -- n_other bits for N_FUN stab
 */
#define N_FUN_PURE              (1 << 0)
#define N_FUN_ELEMENTAL         (1 << 1)
#define N_FUN_RECURSIVE         (1 << 2)
#define N_FUN_AMD64_PARMDUMP    (1 << 3)

/*
 * for variables -- n_other bits for N_LSYM, N_GSYM, N_LCSYM, N_STSYM, ...
 */
#define N_SYM_OMP_TLS       (1 << 3)

/*
 * Optional language designations for N_SO (n_desc field)
 */
#define N_SO_AS             1   /* Assembler  */
#define N_SO_C              2   /* C          */
#define N_SO_ANSI_C         3   /* ANSI C     */
#define N_SO_CC             4   /* C++        */
#define N_SO_FORTRAN        5   /* Fortran 77 */
#define N_SO_FORTRAN77      5   /* Fortran 77 */
#define N_SO_PASCAL         6   /* Pascal     */
#define N_SO_FORTRAN90      7   /* Fortran 90 */
#define N_SO_JAVA           8   /* Java       */
#define N_SO_C99            9   /* C99        */

/*
 * Floating point type values (encoded in "R" type specification string)
 */
#define NF_NONE             0   /* Undefined type */
#define NF_SINGLE           1   /* Float IEEE 32 bit floating point */
#define NF_DOUBLE           2   /* Double IEEE 64 bit floating point */
#define NF_COMPLEX          3   /* Complex (2 32bit floats) */
#define NF_COMPLEX16        4   /* Complex (2 64bit doubles) */
#define NF_COMPLEX32        5   /* Complex (2 128bit long doubles) */
#define NF_LDOUBLE          6   /* Long double 128 bit floating point */
#define NF_INTERARITH       7   /* Interval (2 32bit floats) */
#define NF_DINTERARITH      8   /* Interval (2 64bit doubles) */
#define NF_QINTERARITH      9   /* Interval (2 128bit long doubles) */
#define NF_IMAGINARY        10  /* Imaginary (1 32bit floats) */
#define NF_DIMAGINARY       11  /* Imaginary (1 64bit doubles) */
#define NF_QIMAGINARY       12  /* Imaginary (1 128bit long doubles) */

#endif


