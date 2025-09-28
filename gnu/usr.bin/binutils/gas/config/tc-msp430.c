/* tc-msp430.c -- Assembler code for the Texas Instruments MSP430

  Copyright (C) 2002-2023 Free Software Foundation, Inc.
  Contributed by Dmitry Diky <diwil@mail.ru>

  This file is part of GAS, the GNU Assembler.

  GAS is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3, or (at your option)
  any later version.

  GAS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with GAS; see the file COPYING.  If not, write to
  the Free Software Foundation, 51 Franklin Street - Fifth Floor,
  Boston, MA 02110-1301, USA.  */

#include "as.h"
#include <limits.h>
#include "subsegs.h"
#include "opcode/msp430.h"
#include "safe-ctype.h"
#include "dwarf2dbg.h"
#include "elf/msp430.h"
#include "libiberty.h"

/* We will disable polymorphs by default because it is dangerous.
   The potential problem here is the following: assume we got the
   following code:

	jump .l1
	nop
	jump  subroutine	; external symbol
      .l1:
	nop
	ret

   In case of assembly time relaxation we'll get:
	0: jmp .l1 <.text +0x08> (reloc deleted)
	2: nop
	4: br subroutine
    .l1:
	8: nop
	10: ret

   If the 'subroutine' is within +-1024 bytes range then linker
   will produce:
	0: jmp .text +0x08
	2: nop
	4: jmp subroutine
	.l1:
	6: nop
	8: ret	; 'jmp .text +0x08' will land here. WRONG!!!

   The workaround is the following:
   1. Declare global var enable_polymorphs which set to 1 via option -mp.
   2. Declare global var enable_relax	which set to 1 via option -mQ.

   If polymorphs are enabled, and relax isn't, treat all jumps as long jumps,
   do not delete any relocs and leave them for linker.

   If relax is enabled, relax at assembly time and kill relocs as necessary.  */

int msp430_enable_relax;
int msp430_enable_polys;

/* GCC uses the some condition codes which we'll
   implement as new polymorph instructions.

   COND	EXPL	   SHORT JUMP	LONG JUMP
   ===============================================
   eq	==	   jeq 		jne +4; br lab
   ne	!=	   jne 		jeq +4; br lab

   ltn honours no-overflow flag
   ltn	<	   jn 		jn +2;  jmp +4; br lab

   lt	<	   jl 		jge +4;	br lab
   ltu	<	   jlo 		lhs +4; br lab
   le	<= see below
   leu	<= see below

   gt	>  see below
   gtu	>  see below
   ge	>=	   jge 		jl +4; br lab
   geu	>=	   jhs 		jlo +4; br lab
   ===============================================

   Therefore, new opcodes are (BranchEQ -> beq; and so on...)
   beq,bne,blt,bltn,bltu,bge,bgeu
   'u' means unsigned compares

   Also, we add 'jump' instruction:
   jump	UNCOND	-> jmp		br lab

   They will have fmt == 4, and insn_opnumb == number of instruction.  */

struct rcodes_s
{
  const char * name;
  int    index;	/* Corresponding insn_opnumb.  */
  int    sop;	/* Opcode if jump length is short.  */
  long   lpos;	/* Label position.  */
  long   lop0;	/* Opcode 1 _word_ (16 bits).  */
  long   lop1;	/* Opcode second word.  */
  long   lop2;	/* Opcode third word.  */
};

#define MSP430_RLC(n,i,sop,o1) \
  {#n, i, sop, 2, (o1 + 2), 0x4010, 0}

static struct rcodes_s msp430_rcodes[] =
{
  MSP430_RLC (beq,  0, 0x2400, 0x2000),
  MSP430_RLC (bne,  1, 0x2000, 0x2400),
  MSP430_RLC (blt,  2, 0x3800, 0x3400),
  MSP430_RLC (bltu, 3, 0x2800, 0x2c00),
  MSP430_RLC (bge,  4, 0x3400, 0x3800),
  MSP430_RLC (bgeu, 5, 0x2c00, 0x2800),
  {"bltn",          6, 0x3000, 3, 0x3000 + 1, 0x3c00 + 2,0x4010},
  {"jump",          7, 0x3c00, 1, 0x4010, 0, 0},
  {0,0,0,0,0,0,0}
};

#undef  MSP430_RLC
#define MSP430_RLC(n,i,sop,o1) \
  {#n, i, sop, 2, (o1 + 2), 0x0030, 0}

static struct rcodes_s msp430x_rcodes[] =
{
  MSP430_RLC (beq,  0, 0x2400,    0x2000),
  MSP430_RLC (bne,  1, 0x2000,    0x2400),
  MSP430_RLC (blt,  2, 0x3800,    0x3400),
  MSP430_RLC (bltu, 3, 0x2800,    0x2c00),
  MSP430_RLC (bge,  4, 0x3400,    0x3800),
  MSP430_RLC (bgeu, 5, 0x2c00,    0x2800),
  {"bltn",          6, 0x3000, 3, 0x0030 + 1, 0x3c00 + 2, 0x3000},
  {"jump",          7, 0x3c00, 1, 0x0030,     0,          0},
  {0,0,0,0,0,0,0}
};
#undef MSP430_RLC

/* More difficult than above and they have format 5.

   COND	EXPL	SHORT			LONG
   =================================================================
   gt	>	jeq +2; jge label	jeq +6; jl  +4; br label
   gtu	>	jeq +2; jhs label	jeq +6; jlo +4; br label
   leu	<=	jeq label; jlo label	jeq +2; jhs +4; br label
   le	<=	jeq label; jl  label	jeq +2; jge +4; br label
   =================================================================  */

struct hcodes_s
{
  const char * name;
  int    index;		/* Corresponding insn_opnumb.  */
  int    tlab;		/* Number of labels in short mode.  */
  int    op0;		/* Opcode for first word of short jump.  */
  int    op1;		/* Opcode for second word of short jump.  */
  int    lop0;		/* Opcodes for long jump mode.  */
  int    lop1;
  int    lop2;
};

static struct hcodes_s msp430_hcodes[] =
{
  {"bgt",  0, 1, 0x2401, 0x3400, 0x2403, 0x3802, 0x4010 },
  {"bgtu", 1, 1, 0x2401, 0x2c00, 0x2403, 0x2802, 0x4010 },
  {"bleu", 2, 2, 0x2400, 0x2800, 0x2401, 0x2c02, 0x4010 },
  {"ble",  3, 2, 0x2400, 0x3800, 0x2401, 0x3402, 0x4010 },
  {0,0,0,0,0,0,0,0}
};

static struct hcodes_s msp430x_hcodes[] =
{
  {"bgt",  0, 1, 0x2401, 0x3400, 0x2403, 0x3802, 0x0030 },
  {"bgtu", 1, 1, 0x2401, 0x2c00, 0x2403, 0x2802, 0x0030 },
  {"bleu", 2, 2, 0x2400, 0x2800, 0x2401, 0x2c02, 0x0030 },
  {"ble",  3, 2, 0x2400, 0x3800, 0x2401, 0x3402, 0x0030 },
  {0,0,0,0,0,0,0,0}
};

const char comment_chars[] = ";";
const char line_comment_chars[] = "#";
const char line_separator_chars[] = "{";
const char EXP_CHARS[] = "eE";
const char FLT_CHARS[] = "dD";

/* Handle  long expressions.  */
extern LITTLENUM_TYPE generic_bignum[];

static htab_t msp430_hash;

/* Relaxations.  */
#define STATE_UNCOND_BRANCH	1	/* jump */
#define STATE_NOOV_BRANCH	3	/* bltn */
#define STATE_SIMPLE_BRANCH	2	/* bne, beq, etc... */
#define STATE_EMUL_BRANCH	4

#define CNRL	2
#define CUBL	4
#define CNOL	8
#define CSBL	6
#define CEBL	4

/* Length.  */
#define STATE_BITS10	1	/* Wild guess. short jump.  */
#define STATE_WORD	2	/* 2 bytes pc rel. addr. more.  */
#define STATE_UNDEF	3	/* Cannot handle this yet. convert to word mode.  */

#define ENCODE_RELAX(what,length) (((what) << 2) + (length))
#define RELAX_STATE(s)            ((s) & 3)
#define RELAX_LEN(s)	          ((s) >> 2)
#define RELAX_NEXT(a,b)	          ENCODE_RELAX (a, b + 1)

relax_typeS md_relax_table[] =
{
  /* Unused.  */
  {1, 1, 0, 0},
  {1, 1, 0, 0},
  {1, 1, 0, 0},
  {1, 1, 0, 0},

  /* Unconditional jump.  */
  {1, 1, 8, 5},
  {1024, -1024, CNRL, RELAX_NEXT (STATE_UNCOND_BRANCH, STATE_BITS10)},	/* state 10 bits displ */
  {0, 0, CUBL, RELAX_NEXT (STATE_UNCOND_BRANCH, STATE_WORD)},		/* state word */
  {1, 1, CUBL, 0},							/* state undef */

  /* Simple branches.  */
  {0, 0, 8, 9},
  {1024, -1024, CNRL, RELAX_NEXT (STATE_SIMPLE_BRANCH, STATE_BITS10)},	/* state 10 bits displ */
  {0, 0, CSBL, RELAX_NEXT (STATE_SIMPLE_BRANCH, STATE_WORD)},		/* state word */
  {1, 1, CSBL, 0},

  /* blt no overflow branch.  */
  {1, 1, 8, 13},
  {1024, -1024, CNRL, RELAX_NEXT (STATE_NOOV_BRANCH, STATE_BITS10)},	/* state 10 bits displ */
  {0, 0, CNOL, RELAX_NEXT (STATE_NOOV_BRANCH, STATE_WORD)},		/* state word */
  {1, 1, CNOL, 0},

  /* Emulated branches.  */
  {1, 1, 8, 17},
  {1020, -1020, CEBL, RELAX_NEXT (STATE_EMUL_BRANCH, STATE_BITS10)},	/* state 10 bits displ */
  {0, 0, CNOL, RELAX_NEXT (STATE_EMUL_BRANCH, STATE_WORD)},		/* state word */
  {1, 1, CNOL, 0}
};


#define MAX_OP_LEN	4096

typedef enum msp_isa
{
  MSP_ISA_430,
  MSP_ISA_430X,
  MSP_ISA_430Xv2
} msp_isa;

static enum msp_isa selected_isa = MSP_ISA_430Xv2;

static inline bool
target_is_430x (void)
{
  return selected_isa >= MSP_ISA_430X;
}

static inline bool
target_is_430xv2 (void)
{
  return selected_isa == MSP_ISA_430Xv2;
}

/* Generate an absolute 16-bit relocation, for 430 (!extended_op) instructions
   only.
   For the 430X we generate a 430 relocation only for the case where part of an
   expression is being extracted (e.g. #hi(EXP), #lo(EXP). Otherwise generate
   a 430X relocation.
   For the 430 we generate a relocation without assembler range checking
   if we are handling an immediate value or a byte-width instruction.  */

#undef  CHECK_RELOC_MSP430
#define CHECK_RELOC_MSP430(OP)				\
  (target_is_430x ()					\
   ? ((OP).expp == MSP_EXPP_ALL				\
       ? BFD_RELOC_MSP430X_ABS16			\
       : ((OP).vshift == 1				\
	  ? BFD_RELOC_MSP430_ABS_HI16 : BFD_RELOC_16))	\
   : ((imm_op || byte_op)				\
      ? BFD_RELOC_MSP430_16_BYTE : BFD_RELOC_MSP430_16))

/* Generate a 16-bit pc-relative relocation.
   For the 430X we generate a relocation without linker range checking.
   For the 430 we generate a relocation without assembler range checking
   if we are handling an immediate value or a byte-width instruction.  */
#undef  CHECK_RELOC_MSP430_PCREL
#define CHECK_RELOC_MSP430_PCREL			     \
  (target_is_430x ()					     \
   ? BFD_RELOC_MSP430X_PCR16				     \
   : (imm_op || byte_op)				     \
   ? BFD_RELOC_MSP430_16_PCREL_BYTE : BFD_RELOC_MSP430_16_PCREL)

/* Profiling capability:
   It is a performance hit to use gcc's profiling approach for this tiny target.
   Even more -- jtag hardware facility does not perform any profiling functions.
   However we've got gdb's built-in simulator where we can do anything.
   Therefore my suggestion is:

   We define new section ".profiler" which holds all profiling information.
   We define new pseudo operation .profiler which will instruct assembler to
   add new profile entry to the object file. Profile should take place at the
   present address.

   Pseudo-op format:

      .profiler flags,function_to_profile [, cycle_corrector, extra]

   where 'flags' is a combination of the following chars:
	    s - function Start
	    x - function eXit
	    i - function is in Init section
	    f - function is in Fini section
	    l - Library call
	    c - libC standard call
	    d - stack value Demand (saved at run-time in simulator)
	    I - Interrupt service routine
	    P - Prologue start
	    p - Prologue end
	    E - Epilogue start
	    e - Epilogue end
	    j - long Jump/ sjlj unwind
	    a - an Arbitrary code fragment
	    t - exTra parameter saved (constant value like frame size)
	  '""' optional: "sil" == sil

      function_to_profile - function's address
      cycle_corrector     - a value which should be added to the cycle
			      counter, zero if omitted
      extra - some extra parameter, zero if omitted.

      For example:
      ------------------------------
	.global fxx
	.type fxx,@function
      fxx:
      .LFrameOffset_fxx=0x08
      .profiler "scdP", fxx	; function entry.
				; we also demand stack value to be displayed
	push r11
	push r10
	push r9
	push r8
      .profiler "cdp",fxx,0, .LFrameOffset_fxx	; check stack value at this point
						; (this is a prologue end)
						; note, that spare var filled with the frame size
	mov r15,r8
	....
      .profiler cdE,fxx		; check stack
	pop r8
	pop r9
	pop r10
	pop r11
      .profiler xcde,fxx,3	; exit adds 3 to the cycle counter
      ret			; cause 'ret' insn takes 3 cycles
      -------------------------------

      This profiling approach does not produce any overhead and
      absolutely harmless.
      So, even profiled code can be uploaded to the MCU.  */
#define MSP430_PROFILER_FLAG_ENTRY	1	/* s */
#define MSP430_PROFILER_FLAG_EXIT	2	/* x */
#define MSP430_PROFILER_FLAG_INITSECT	4	/* i */
#define MSP430_PROFILER_FLAG_FINISECT	8	/* f */
#define MSP430_PROFILER_FLAG_LIBCALL	0x10	/* l */
#define MSP430_PROFILER_FLAG_STDCALL	0x20	/* c */
#define MSP430_PROFILER_FLAG_STACKDMD	0x40	/* d */
#define MSP430_PROFILER_FLAG_ISR	0x80	/* I */
#define MSP430_PROFILER_FLAG_PROLSTART	0x100	/* P */
#define MSP430_PROFILER_FLAG_PROLEND	0x200	/* p */
#define MSP430_PROFILER_FLAG_EPISTART	0x400	/* E */
#define MSP430_PROFILER_FLAG_EPIEND	0x800	/* e */
#define MSP430_PROFILER_FLAG_JUMP	0x1000	/* j */
#define MSP430_PROFILER_FLAG_FRAGMENT	0x2000	/* a */
#define MSP430_PROFILER_FLAG_EXTRA	0x4000	/* t */
#define MSP430_PROFILER_FLAG_notyet	0x8000	/* ? */

static int
pow2value (int y)
{
  int n = 0;
  unsigned int x;

  x = y;

  if (!x)
    return 1;

  for (; x; x = x >> 1)
    if (x & 1)
      n++;

  return n == 1;
}

/* Parse ordinary expression.  */

static char *
parse_exp (char * s, expressionS * op)
{
  input_line_pointer = s;
  expression (op);
  if (op->X_op == O_absent)
    as_bad (_("missing operand"));
  else
    resolve_register (op);

  /* Our caller is likely to check that the entire expression was parsed.
     If we have found a hex constant with an 'h' suffix, ilp will be left
     pointing at the 'h', so skip it here.  */
  if (input_line_pointer != NULL
      && op->X_op == O_constant
      && (*input_line_pointer == 'h' || *input_line_pointer == 'H'))
    ++ input_line_pointer;
  return input_line_pointer;
}


/* Delete spaces from s: X ( r 1  2)  => X(r12).  */

static void
del_spaces (char * s)
{
  while (*s)
    {
      if (ISSPACE (*s))
	{
	  char *m = s + 1;

	  while (ISSPACE (*m) && *m)
	    m++;
	  memmove (s, m, strlen (m) + 1);
	}
      else
	s++;
    }
}

static inline char *
skip_space (char * s)
{
  while (ISSPACE (*s))
    ++s;
  return s;
}

/* Extract one word from FROM and copy it to TO. Delimiters are ",;\n"  */

static char *
extract_operand (char * from, char * to, int limit)
{
  int size = 0;

  /* Drop leading whitespace.  */
  from = skip_space (from);

  while (size < limit && *from)
    {
      *(to + size) = *from;
      if (*from == ',' || *from == ';' || *from == '\n')
	break;
      from++;
      size++;
    }

  *(to + size) = 0;
  del_spaces (to);

  from++;

  return from;
}

static void
msp430_profiler (int dummy ATTRIBUTE_UNUSED)
{
  char   buffer[1024];
  char   f[32];
  char * str = buffer;
  char * flags = f;
  int    p_flags = 0;
  char * halt;
  int    ops = 0;
  int    left;
  char * s;
  segT   seg;
  int    subseg;
  char * end = 0;
  expressionS exp;
  expressionS exp1;

  s = input_line_pointer;
  end = input_line_pointer;

  while (*end && *end != '\n')
    end++;

  while (*s && *s != '\n')
    {
      if (*s == ',')
	ops++;
      s++;
    }

  left = 3 - ops;

  if (ops < 1)
    {
      as_bad (_(".profiler pseudo requires at least two operands."));
      input_line_pointer = end;
      return;
    }

  input_line_pointer = extract_operand (input_line_pointer, flags, 32);

  while (*flags)
    {
      switch (*flags)
	{
	case '"':
	  break;
	case 'a':
	  p_flags |= MSP430_PROFILER_FLAG_FRAGMENT;
	  break;
	case 'j':
	  p_flags |= MSP430_PROFILER_FLAG_JUMP;
	  break;
	case 'P':
	  p_flags |= MSP430_PROFILER_FLAG_PROLSTART;
	  break;
	case 'p':
	  p_flags |= MSP430_PROFILER_FLAG_PROLEND;
	  break;
	case 'E':
	  p_flags |= MSP430_PROFILER_FLAG_EPISTART;
	  break;
	case 'e':
	  p_flags |= MSP430_PROFILER_FLAG_EPIEND;
	  break;
	case 's':
	  p_flags |= MSP430_PROFILER_FLAG_ENTRY;
	  break;
	case 'x':
	  p_flags |= MSP430_PROFILER_FLAG_EXIT;
	  break;
	case 'i':
	  p_flags |= MSP430_PROFILER_FLAG_INITSECT;
	  break;
	case 'f':
	  p_flags |= MSP430_PROFILER_FLAG_FINISECT;
	  break;
	case 'l':
	  p_flags |= MSP430_PROFILER_FLAG_LIBCALL;
	  break;
	case 'c':
	  p_flags |= MSP430_PROFILER_FLAG_STDCALL;
	  break;
	case 'd':
	  p_flags |= MSP430_PROFILER_FLAG_STACKDMD;
	  break;
	case 'I':
	  p_flags |= MSP430_PROFILER_FLAG_ISR;
	  break;
	case 't':
	  p_flags |= MSP430_PROFILER_FLAG_EXTRA;
	  break;
	default:
	  as_warn (_("unknown profiling flag - ignored."));
	  break;
	}
      flags++;
    }

  if (p_flags
      && (   ! pow2value (p_flags & (  MSP430_PROFILER_FLAG_ENTRY
				     | MSP430_PROFILER_FLAG_EXIT))
	  || ! pow2value (p_flags & (  MSP430_PROFILER_FLAG_PROLSTART
				     | MSP430_PROFILER_FLAG_PROLEND
				     | MSP430_PROFILER_FLAG_EPISTART
				     | MSP430_PROFILER_FLAG_EPIEND))
	  || ! pow2value (p_flags & (  MSP430_PROFILER_FLAG_INITSECT
				     | MSP430_PROFILER_FLAG_FINISECT))))
    {
      as_bad (_("ambiguous flags combination - '.profiler' directive ignored."));
      input_line_pointer = end;
      return;
    }

  /* Generate temp symbol which denotes current location.  */
  if (now_seg == absolute_section)	/* Paranoia ?  */
    {
      exp1.X_op = O_constant;
      exp1.X_add_number = abs_section_offset;
      as_warn (_("profiling in absolute section?"));
    }
  else
    {
      exp1.X_op = O_symbol;
      exp1.X_add_symbol = symbol_temp_new_now ();
      exp1.X_add_number = 0;
    }

  /* Generate a symbol which holds flags value.  */
  exp.X_op = O_constant;
  exp.X_add_number = p_flags;

  /* Save current section.  */
  seg = now_seg;
  subseg = now_subseg;

  /* Now go to .profiler section.  */
  obj_elf_change_section (".profiler", SHT_PROGBITS, 0, 0, 0, 0, 0);

  /* Save flags.  */
  emit_expr (& exp, 2);

  /* Save label value.  */
  emit_expr (& exp1, 2);

  while (ops--)
    {
      /* Now get profiling info.  */
      halt = extract_operand (input_line_pointer, str, 1024);
      /* Process like ".word xxx" directive.  */
      (void) parse_exp (str, & exp);
      emit_expr (& exp, 2);
      input_line_pointer = halt;
    }

  /* Fill the rest with zeros.  */
  exp.X_op = O_constant;
  exp.X_add_number = 0;
  while (left--)
    emit_expr (& exp, 2);

  /* Return to current section.  */
  subseg_set (seg, subseg);
}

static char *
extract_word (char * from, char * to, int limit)
{
  char *op_end;
  int size = 0;

  /* Drop leading whitespace.  */
  from = skip_space (from);
  *to = 0;

  /* Find the op code end.  */
  for (op_end = from; *op_end != 0 && is_part_of_name (*op_end);)
    {
      to[size++] = *op_end++;
      if (size + 1 >= limit)
	break;
    }

  to[size] = 0;
  return op_end;
}

#define OPTION_MMCU 'm'
#define OPTION_RELAX 'Q'
#define OPTION_POLYMORPHS 'P'
#define OPTION_LARGE 'l'
static bool large_model = false;
#define OPTION_NO_INTR_NOPS 'N'
#define OPTION_INTR_NOPS 'n'
static bool gen_interrupt_nops = false;
#define OPTION_WARN_INTR_NOPS 'y'
#define OPTION_NO_WARN_INTR_NOPS 'Y'
static bool warn_interrupt_nops = true;
#define OPTION_UNKNOWN_INTR_NOPS 'u'
#define OPTION_NO_UNKNOWN_INTR_NOPS 'U'
static bool do_unknown_interrupt_nops = true;
#define OPTION_MCPU 'c'
#define OPTION_DATA_REGION 'r'
static bool upper_data_region_in_use = false;
/* The default is to use the lower region only.  */
static bool lower_data_region_only = true;

/* Deprecated option, silently ignore it for compatibility with GCC <= 10.  */
#define OPTION_MOVE_DATA 'd'

enum
{
  OPTION_SILICON_ERRATA = OPTION_MD_BASE,
  OPTION_SILICON_ERRATA_WARN,
};

static unsigned int silicon_errata_fix = 0;
static unsigned int silicon_errata_warn = 0;
#define SILICON_ERRATA_CPU4 		(1 << 0)
#define SILICON_ERRATA_CPU8		(1 << 1)
#define SILICON_ERRATA_CPU11 		(1 << 2)
#define SILICON_ERRATA_CPU12 		(1 << 3)
#define SILICON_ERRATA_CPU13 		(1 << 4)
#define SILICON_ERRATA_CPU19 		(1 << 5)

static void
msp430_set_arch (int option)
{
  char str[32];	/* 32 for good measure.  */

  input_line_pointer = extract_word (input_line_pointer, str, 32);

  md_parse_option (option, str);
  bfd_set_arch_mach (stdoutput, TARGET_ARCH,
		     target_is_430x () ? bfd_mach_msp430x : bfd_mach_msp11);
}

/* This is a copy of the same data structure found in gcc/config/msp430/msp430.c
   Keep these two structures in sync.
   The data in this structure has been extracted from version 1.194 of the
   devices.csv file released by TI in September 2016.  */

struct msp430_mcu_data
{
  const char * name;
  unsigned int revision; /* 0=> MSP430, 1=>MSP430X, 2=> MSP430Xv2.  */
  unsigned int hwmpy;    /* 0=>none, 1=>16-bit, 2=>16-bit w/sign extend, 4=>32-bit, 8=> 32-bit (5xx).  */
}
msp430_mcu_data [] =
{
  { "cc430f5123",2,8 },
  { "cc430f5125",2,8 },
  { "cc430f5133",2,8 },
  { "cc430f5135",2,8 },
  { "cc430f5137",2,8 },
  { "cc430f5143",2,8 },
  { "cc430f5145",2,8 },
  { "cc430f5147",2,8 },
  { "cc430f6125",2,8 },
  { "cc430f6126",2,8 },
  { "cc430f6127",2,8 },
  { "cc430f6135",2,8 },
  { "cc430f6137",2,8 },
  { "cc430f6143",2,8 },
  { "cc430f6145",2,8 },
  { "cc430f6147",2,8 },
  { "msp430afe221",0,2 },
  { "msp430afe222",0,2 },
  { "msp430afe223",0,2 },
  { "msp430afe231",0,2 },
  { "msp430afe232",0,2 },
  { "msp430afe233",0,2 },
  { "msp430afe251",0,2 },
  { "msp430afe252",0,2 },
  { "msp430afe253",0,2 },
  { "msp430bt5190",2,8 },
  { "msp430c091",0,0 },
  { "msp430c092",0,0 },
  { "msp430c111",0,0 },
  { "msp430c1111",0,0 },
  { "msp430c112",0,0 },
  { "msp430c1121",0,0 },
  { "msp430c1331",0,0 },
  { "msp430c1351",0,0 },
  { "msp430c311s",0,0 },
  { "msp430c312",0,0 },
  { "msp430c313",0,0 },
  { "msp430c314",0,0 },
  { "msp430c315",0,0 },
  { "msp430c323",0,0 },
  { "msp430c325",0,0 },
  { "msp430c336",0,1 },
  { "msp430c337",0,1 },
  { "msp430c412",0,0 },
  { "msp430c413",0,0 },
  { "msp430cg4616",1,1 },
  { "msp430cg4617",1,1 },
  { "msp430cg4618",1,1 },
  { "msp430cg4619",1,1 },
  { "msp430e112",0,0 },
  { "msp430e313",0,0 },
  { "msp430e315",0,0 },
  { "msp430e325",0,0 },
  { "msp430e337",0,1 },
  { "msp430f110",0,0 },
  { "msp430f1101",0,0 },
  { "msp430f1101a",0,0 },
  { "msp430f1111",0,0 },
  { "msp430f1111a",0,0 },
  { "msp430f112",0,0 },
  { "msp430f1121",0,0 },
  { "msp430f1121a",0,0 },
  { "msp430f1122",0,0 },
  { "msp430f1132",0,0 },
  { "msp430f122",0,0 },
  { "msp430f1222",0,0 },
  { "msp430f123",0,0 },
  { "msp430f1232",0,0 },
  { "msp430f133",0,0 },
  { "msp430f135",0,0 },
  { "msp430f147",0,1 },
  { "msp430f1471",0,1 },
  { "msp430f148",0,1 },
  { "msp430f1481",0,1 },
  { "msp430f149",0,1 },
  { "msp430f1491",0,1 },
  { "msp430f155",0,0 },
  { "msp430f156",0,0 },
  { "msp430f157",0,0 },
  { "msp430f1610",0,1 },
  { "msp430f1611",0,1 },
  { "msp430f1612",0,1 },
  { "msp430f167",0,1 },
  { "msp430f168",0,1 },
  { "msp430f169",0,1 },
  { "msp430f2001",0,0 },
  { "msp430f2002",0,0 },
  { "msp430f2003",0,0 },
  { "msp430f2011",0,0 },
  { "msp430f2012",0,0 },
  { "msp430f2013",0,0 },
  { "msp430f2101",0,0 },
  { "msp430f2111",0,0 },
  { "msp430f2112",0,0 },
  { "msp430f2121",0,0 },
  { "msp430f2122",0,0 },
  { "msp430f2131",0,0 },
  { "msp430f2132",0,0 },
  { "msp430f2232",0,0 },
  { "msp430f2234",0,0 },
  { "msp430f2252",0,0 },
  { "msp430f2254",0,0 },
  { "msp430f2272",0,0 },
  { "msp430f2274",0,0 },
  { "msp430f233",0,2 },
  { "msp430f2330",0,2 },
  { "msp430f235",0,2 },
  { "msp430f2350",0,2 },
  { "msp430f2370",0,2 },
  { "msp430f2410",0,2 },
  { "msp430f2416",1,2 },
  { "msp430f2417",1,2 },
  { "msp430f2418",1,2 },
  { "msp430f2419",1,2 },
  { "msp430f247",0,2 },
  { "msp430f2471",0,2 },
  { "msp430f248",0,2 },
  { "msp430f2481",0,2 },
  { "msp430f249",0,2 },
  { "msp430f2491",0,2 },
  { "msp430f2616",1,2 },
  { "msp430f2617",1,2 },
  { "msp430f2618",1,2 },
  { "msp430f2619",1,2 },
  { "msp430f412",0,0 },
  { "msp430f413",0,0 },
  { "msp430f4132",0,0 },
  { "msp430f415",0,0 },
  { "msp430f4152",0,0 },
  { "msp430f417",0,0 },
  { "msp430f423",0,1 },
  { "msp430f423a",0,1 },
  { "msp430f425",0,1 },
  { "msp430f4250",0,0 },
  { "msp430f425a",0,1 },
  { "msp430f4260",0,0 },
  { "msp430f427",0,1 },
  { "msp430f4270",0,0 },
  { "msp430f427a",0,1 },
  { "msp430f435",0,0 },
  { "msp430f4351",0,0 },
  { "msp430f436",0,0 },
  { "msp430f4361",0,0 },
  { "msp430f437",0,0 },
  { "msp430f4371",0,0 },
  { "msp430f438",0,0 },
  { "msp430f439",0,0 },
  { "msp430f447",0,1 },
  { "msp430f448",0,1 },
  { "msp430f4481",0,1 },
  { "msp430f449",0,1 },
  { "msp430f4491",0,1 },
  { "msp430f4616",1,1 },
  { "msp430f46161",1,1 },
  { "msp430f4617",1,1 },
  { "msp430f46171",1,1 },
  { "msp430f4618",1,1 },
  { "msp430f46181",1,1 },
  { "msp430f4619",1,1 },
  { "msp430f46191",1,1 },
  { "msp430f47126",1,4 },
  { "msp430f47127",1,4 },
  { "msp430f47163",1,4 },
  { "msp430f47166",1,4 },
  { "msp430f47167",1,4 },
  { "msp430f47173",1,4 },
  { "msp430f47176",1,4 },
  { "msp430f47177",1,4 },
  { "msp430f47183",1,4 },
  { "msp430f47186",1,4 },
  { "msp430f47187",1,4 },
  { "msp430f47193",1,4 },
  { "msp430f47196",1,4 },
  { "msp430f47197",1,4 },
  { "msp430f477",0,0 },
  { "msp430f478",0,0 },
  { "msp430f4783",0,4 },
  { "msp430f4784",0,4 },
  { "msp430f479",0,0 },
  { "msp430f4793",0,4 },
  { "msp430f4794",0,4 },
  { "msp430f5131",2,8 },
  { "msp430f5132",2,8 },
  { "msp430f5151",2,8 },
  { "msp430f5152",2,8 },
  { "msp430f5171",2,8 },
  { "msp430f5172",2,8 },
  { "msp430f5212",2,8 },
  { "msp430f5213",2,8 },
  { "msp430f5214",2,8 },
  { "msp430f5217",2,8 },
  { "msp430f5218",2,8 },
  { "msp430f5219",2,8 },
  { "msp430f5222",2,8 },
  { "msp430f5223",2,8 },
  { "msp430f5224",2,8 },
  { "msp430f5227",2,8 },
  { "msp430f5228",2,8 },
  { "msp430f5229",2,8 },
  { "msp430f5232",2,8 },
  { "msp430f5234",2,8 },
  { "msp430f5237",2,8 },
  { "msp430f5239",2,8 },
  { "msp430f5242",2,8 },
  { "msp430f5244",2,8 },
  { "msp430f5247",2,8 },
  { "msp430f5249",2,8 },
  { "msp430f5252",2,8 },
  { "msp430f5253",2,8 },
  { "msp430f5254",2,8 },
  { "msp430f5255",2,8 },
  { "msp430f5256",2,8 },
  { "msp430f5257",2,8 },
  { "msp430f5258",2,8 },
  { "msp430f5259",2,8 },
  { "msp430f5304",2,8 },
  { "msp430f5308",2,8 },
  { "msp430f5309",2,8 },
  { "msp430f5310",2,8 },
  { "msp430f5324",2,8 },
  { "msp430f5325",2,8 },
  { "msp430f5326",2,8 },
  { "msp430f5327",2,8 },
  { "msp430f5328",2,8 },
  { "msp430f5329",2,8 },
  { "msp430f5333",2,8 },
  { "msp430f5335",2,8 },
  { "msp430f5336",2,8 },
  { "msp430f5338",2,8 },
  { "msp430f5340",2,8 },
  { "msp430f5341",2,8 },
  { "msp430f5342",2,8 },
  { "msp430f5358",2,8 },
  { "msp430f5359",2,8 },
  { "msp430f5418",2,8 },
  { "msp430f5418a",2,8 },
  { "msp430f5419",2,8 },
  { "msp430f5419a",2,8 },
  { "msp430f5435",2,8 },
  { "msp430f5435a",2,8 },
  { "msp430f5436",2,8 },
  { "msp430f5436a",2,8 },
  { "msp430f5437",2,8 },
  { "msp430f5437a",2,8 },
  { "msp430f5438",2,8 },
  { "msp430f5438a",2,8 },
  { "msp430f5500",2,8 },
  { "msp430f5501",2,8 },
  { "msp430f5502",2,8 },
  { "msp430f5503",2,8 },
  { "msp430f5504",2,8 },
  { "msp430f5505",2,8 },
  { "msp430f5506",2,8 },
  { "msp430f5507",2,8 },
  { "msp430f5508",2,8 },
  { "msp430f5509",2,8 },
  { "msp430f5510",2,8 },
  { "msp430f5513",2,8 },
  { "msp430f5514",2,8 },
  { "msp430f5515",2,8 },
  { "msp430f5517",2,8 },
  { "msp430f5519",2,8 },
  { "msp430f5521",2,8 },
  { "msp430f5522",2,8 },
  { "msp430f5524",2,8 },
  { "msp430f5525",2,8 },
  { "msp430f5526",2,8 },
  { "msp430f5527",2,8 },
  { "msp430f5528",2,8 },
  { "msp430f5529",2,8 },
  { "msp430f5630",2,8 },
  { "msp430f5631",2,8 },
  { "msp430f5632",2,8 },
  { "msp430f5633",2,8 },
  { "msp430f5634",2,8 },
  { "msp430f5635",2,8 },
  { "msp430f5636",2,8 },
  { "msp430f5637",2,8 },
  { "msp430f5638",2,8 },
  { "msp430f5658",2,8 },
  { "msp430f5659",2,8 },
  { "msp430f5xx_6xxgeneric",2,8 },
  { "msp430f6433",2,8 },
  { "msp430f6435",2,8 },
  { "msp430f6436",2,8 },
  { "msp430f6438",2,8 },
  { "msp430f6458",2,8 },
  { "msp430f6459",2,8 },
  { "msp430f6630",2,8 },
  { "msp430f6631",2,8 },
  { "msp430f6632",2,8 },
  { "msp430f6633",2,8 },
  { "msp430f6634",2,8 },
  { "msp430f6635",2,8 },
  { "msp430f6636",2,8 },
  { "msp430f6637",2,8 },
  { "msp430f6638",2,8 },
  { "msp430f6658",2,8 },
  { "msp430f6659",2,8 },
  { "msp430f6720",2,8 },
  { "msp430f6720a",2,8 },
  { "msp430f6721",2,8 },
  { "msp430f6721a",2,8 },
  { "msp430f6723",2,8 },
  { "msp430f6723a",2,8 },
  { "msp430f6724",2,8 },
  { "msp430f6724a",2,8 },
  { "msp430f6725",2,8 },
  { "msp430f6725a",2,8 },
  { "msp430f6726",2,8 },
  { "msp430f6726a",2,8 },
  { "msp430f6730",2,8 },
  { "msp430f6730a",2,8 },
  { "msp430f6731",2,8 },
  { "msp430f6731a",2,8 },
  { "msp430f6733",2,8 },
  { "msp430f6733a",2,8 },
  { "msp430f6734",2,8 },
  { "msp430f6734a",2,8 },
  { "msp430f6735",2,8 },
  { "msp430f6735a",2,8 },
  { "msp430f6736",2,8 },
  { "msp430f6736a",2,8 },
  { "msp430f6745",2,8 },
  { "msp430f67451",2,8 },
  { "msp430f67451a",2,8 },
  { "msp430f6745a",2,8 },
  { "msp430f6746",2,8 },
  { "msp430f67461",2,8 },
  { "msp430f67461a",2,8 },
  { "msp430f6746a",2,8 },
  { "msp430f6747",2,8 },
  { "msp430f67471",2,8 },
  { "msp430f67471a",2,8 },
  { "msp430f6747a",2,8 },
  { "msp430f6748",2,8 },
  { "msp430f67481",2,8 },
  { "msp430f67481a",2,8 },
  { "msp430f6748a",2,8 },
  { "msp430f6749",2,8 },
  { "msp430f67491",2,8 },
  { "msp430f67491a",2,8 },
  { "msp430f6749a",2,8 },
  { "msp430f67621",2,8 },
  { "msp430f67621a",2,8 },
  { "msp430f67641",2,8 },
  { "msp430f67641a",2,8 },
  { "msp430f6765",2,8 },
  { "msp430f67651",2,8 },
  { "msp430f67651a",2,8 },
  { "msp430f6765a",2,8 },
  { "msp430f6766",2,8 },
  { "msp430f67661",2,8 },
  { "msp430f67661a",2,8 },
  { "msp430f6766a",2,8 },
  { "msp430f6767",2,8 },
  { "msp430f67671",2,8 },
  { "msp430f67671a",2,8 },
  { "msp430f6767a",2,8 },
  { "msp430f6768",2,8 },
  { "msp430f67681",2,8 },
  { "msp430f67681a",2,8 },
  { "msp430f6768a",2,8 },
  { "msp430f6769",2,8 },
  { "msp430f67691",2,8 },
  { "msp430f67691a",2,8 },
  { "msp430f6769a",2,8 },
  { "msp430f6775",2,8 },
  { "msp430f67751",2,8 },
  { "msp430f67751a",2,8 },
  { "msp430f6775a",2,8 },
  { "msp430f6776",2,8 },
  { "msp430f67761",2,8 },
  { "msp430f67761a",2,8 },
  { "msp430f6776a",2,8 },
  { "msp430f6777",2,8 },
  { "msp430f67771",2,8 },
  { "msp430f67771a",2,8 },
  { "msp430f6777a",2,8 },
  { "msp430f6778",2,8 },
  { "msp430f67781",2,8 },
  { "msp430f67781a",2,8 },
  { "msp430f6778a",2,8 },
  { "msp430f6779",2,8 },
  { "msp430f67791",2,8 },
  { "msp430f67791a",2,8 },
  { "msp430f6779a",2,8 },
  { "msp430fe423",0,0 },
  { "msp430fe4232",0,0 },
  { "msp430fe423a",0,0 },
  { "msp430fe4242",0,0 },
  { "msp430fe425",0,0 },
  { "msp430fe4252",0,0 },
  { "msp430fe425a",0,0 },
  { "msp430fe427",0,0 },
  { "msp430fe4272",0,0 },
  { "msp430fe427a",0,0 },
  { "msp430fg4250",0,0 },
  { "msp430fg4260",0,0 },
  { "msp430fg4270",0,0 },
  { "msp430fg437",0,0 },
  { "msp430fg438",0,0 },
  { "msp430fg439",0,0 },
  { "msp430fg4616",1,1 },
  { "msp430fg4617",1,1 },
  { "msp430fg4618",1,1 },
  { "msp430fg4619",1,1 },
  { "msp430fg477",0,0 },
  { "msp430fg478",0,0 },
  { "msp430fg479",0,0 },
  { "msp430fg6425",2,8 },
  { "msp430fg6426",2,8 },
  { "msp430fg6625",2,8 },
  { "msp430fg6626",2,8 },
  { "msp430fr2032",2,0 },
  { "msp430fr2033",2,0 },
  { "msp430fr2110",2,0 },
  { "msp430fr2111",2,0 },
  { "msp430fr2310",2,0 },
  { "msp430fr2311",2,0 },
  { "msp430fr2433",2,8 },
  { "msp430fr2532",2,8 },
  { "msp430fr2533",2,8 },
  { "msp430fr2632",2,8 },
  { "msp430fr2633",2,8 },
  { "msp430fr2xx_4xxgeneric",2,8 },
  { "msp430fr4131",2,0 },
  { "msp430fr4132",2,0 },
  { "msp430fr4133",2,0 },
  { "msp430fr5720",2,8 },
  { "msp430fr5721",2,8 },
  { "msp430fr5722",2,8 },
  { "msp430fr5723",2,8 },
  { "msp430fr5724",2,8 },
  { "msp430fr5725",2,8 },
  { "msp430fr5726",2,8 },
  { "msp430fr5727",2,8 },
  { "msp430fr5728",2,8 },
  { "msp430fr5729",2,8 },
  { "msp430fr5730",2,8 },
  { "msp430fr5731",2,8 },
  { "msp430fr5732",2,8 },
  { "msp430fr5733",2,8 },
  { "msp430fr5734",2,8 },
  { "msp430fr5735",2,8 },
  { "msp430fr5736",2,8 },
  { "msp430fr5737",2,8 },
  { "msp430fr5738",2,8 },
  { "msp430fr5739",2,8 },
  { "msp430fr57xxgeneric",2,8 },
  { "msp430fr5847",2,8 },
  { "msp430fr58471",2,8 },
  { "msp430fr5848",2,8 },
  { "msp430fr5849",2,8 },
  { "msp430fr5857",2,8 },
  { "msp430fr5858",2,8 },
  { "msp430fr5859",2,8 },
  { "msp430fr5867",2,8 },
  { "msp430fr58671",2,8 },
  { "msp430fr5868",2,8 },
  { "msp430fr5869",2,8 },
  { "msp430fr5870",2,8 },
  { "msp430fr5872",2,8 },
  { "msp430fr58721",2,8 },
  { "msp430fr5887",2,8 },
  { "msp430fr5888",2,8 },
  { "msp430fr5889",2,8 },
  { "msp430fr58891",2,8 },
  { "msp430fr5922",2,8 },
  { "msp430fr59221",2,8 },
  { "msp430fr5947",2,8 },
  { "msp430fr59471",2,8 },
  { "msp430fr5948",2,8 },
  { "msp430fr5949",2,8 },
  { "msp430fr5957",2,8 },
  { "msp430fr5958",2,8 },
  { "msp430fr5959",2,8 },
  { "msp430fr5962",2,8 },
  { "msp430fr5964",2,8 },
  { "msp430fr5967",2,8 },
  { "msp430fr5968",2,8 },
  { "msp430fr5969",2,8 },
  { "msp430fr59691",2,8 },
  { "msp430fr5970",2,8 },
  { "msp430fr5972",2,8 },
  { "msp430fr59721",2,8 },
  { "msp430fr5986",2,8 },
  { "msp430fr5987",2,8 },
  { "msp430fr5988",2,8 },
  { "msp430fr5989",2,8 },
  { "msp430fr59891",2,8 },
  { "msp430fr5992",2,8 },
  { "msp430fr5994",2,8 },
  { "msp430fr59941",2,8 },
  { "msp430fr5xx_6xxgeneric",2,8 },
  { "msp430fr6820",2,8 },
  { "msp430fr6822",2,8 },
  { "msp430fr68221",2,8 },
  { "msp430fr6870",2,8 },
  { "msp430fr6872",2,8 },
  { "msp430fr68721",2,8 },
  { "msp430fr6877",2,8 },
  { "msp430fr6879",2,8 },
  { "msp430fr68791",2,8 },
  { "msp430fr6887",2,8 },
  { "msp430fr6888",2,8 },
  { "msp430fr6889",2,8 },
  { "msp430fr68891",2,8 },
  { "msp430fr6920",2,8 },
  { "msp430fr6922",2,8 },
  { "msp430fr69221",2,8 },
  { "msp430fr6927",2,8 },
  { "msp430fr69271",2,8 },
  { "msp430fr6928",2,8 },
  { "msp430fr6970",2,8 },
  { "msp430fr6972",2,8 },
  { "msp430fr69721",2,8 },
  { "msp430fr6977",2,8 },
  { "msp430fr6979",2,8 },
  { "msp430fr69791",2,8 },
  { "msp430fr6987",2,8 },
  { "msp430fr6988",2,8 },
  { "msp430fr6989",2,8 },
  { "msp430fr69891",2,8 },
  { "msp430fw423",0,0 },
  { "msp430fw425",0,0 },
  { "msp430fw427",0,0 },
  { "msp430fw428",0,0 },
  { "msp430fw429",0,0 },
  { "msp430g2001",0,0 },
  { "msp430g2101",0,0 },
  { "msp430g2102",0,0 },
  { "msp430g2111",0,0 },
  { "msp430g2112",0,0 },
  { "msp430g2113",0,0 },
  { "msp430g2121",0,0 },
  { "msp430g2131",0,0 },
  { "msp430g2132",0,0 },
  { "msp430g2152",0,0 },
  { "msp430g2153",0,0 },
  { "msp430g2201",0,0 },
  { "msp430g2202",0,0 },
  { "msp430g2203",0,0 },
  { "msp430g2210",0,0 },
  { "msp430g2211",0,0 },
  { "msp430g2212",0,0 },
  { "msp430g2213",0,0 },
  { "msp430g2221",0,0 },
  { "msp430g2230",0,0 },
  { "msp430g2231",0,0 },
  { "msp430g2232",0,0 },
  { "msp430g2233",0,0 },
  { "msp430g2252",0,0 },
  { "msp430g2253",0,0 },
  { "msp430g2302",0,0 },
  { "msp430g2303",0,0 },
  { "msp430g2312",0,0 },
  { "msp430g2313",0,0 },
  { "msp430g2332",0,0 },
  { "msp430g2333",0,0 },
  { "msp430g2352",0,0 },
  { "msp430g2353",0,0 },
  { "msp430g2402",0,0 },
  { "msp430g2403",0,0 },
  { "msp430g2412",0,0 },
  { "msp430g2413",0,0 },
  { "msp430g2432",0,0 },
  { "msp430g2433",0,0 },
  { "msp430g2444",0,0 },
  { "msp430g2452",0,0 },
  { "msp430g2453",0,0 },
  { "msp430g2513",0,0 },
  { "msp430g2533",0,0 },
  { "msp430g2544",0,0 },
  { "msp430g2553",0,0 },
  { "msp430g2744",0,0 },
  { "msp430g2755",0,0 },
  { "msp430g2855",0,0 },
  { "msp430g2955",0,0 },
  { "msp430i2020",0,2 },
  { "msp430i2021",0,2 },
  { "msp430i2030",0,2 },
  { "msp430i2031",0,2 },
  { "msp430i2040",0,2 },
  { "msp430i2041",0,2 },
  { "msp430i2xxgeneric",0,2 },
  { "msp430l092",0,0 },
  { "msp430p112",0,0 },
  { "msp430p313",0,0 },
  { "msp430p315",0,0 },
  { "msp430p315s",0,0 },
  { "msp430p325",0,0 },
  { "msp430p337",0,1 },
  { "msp430sl5438a",2,8 },
  { "msp430tch5e",0,0 },
  { "msp430xgeneric",2,8 },
  { "rf430f5144",2,8 },
  { "rf430f5155",2,8 },
  { "rf430f5175",2,8 },
  { "rf430frl152h",0,0 },
  { "rf430frl152h_rom",0,0 },
  { "rf430frl153h",0,0 },
  { "rf430frl153h_rom",0,0 },
  { "rf430frl154h",0,0 },
  { "rf430frl154h_rom",0,0 }
};  

int
md_parse_option (int c, const char * arg)
{
  switch (c)
    {
    case OPTION_SILICON_ERRATA:
    case OPTION_SILICON_ERRATA_WARN:
      {
	signed int i;
	const struct
	{
	  const char *       name;
	  unsigned int length;
	  unsigned int bitfield;
	} erratas[] =
	{
	  { STRING_COMMA_LEN ("cpu4"), SILICON_ERRATA_CPU4 },
	  { STRING_COMMA_LEN ("cpu8"), SILICON_ERRATA_CPU8 },
	  { STRING_COMMA_LEN ("cpu11"), SILICON_ERRATA_CPU11 },
	  { STRING_COMMA_LEN ("cpu12"), SILICON_ERRATA_CPU12 },
	  { STRING_COMMA_LEN ("cpu13"), SILICON_ERRATA_CPU13 },
	  { STRING_COMMA_LEN ("cpu19"), SILICON_ERRATA_CPU19 },
	};

	do
	  {
	    for (i = ARRAY_SIZE (erratas); i--;)
	      if (strncasecmp (arg, erratas[i].name, erratas[i].length) == 0)
		{
		  if (c == OPTION_SILICON_ERRATA)
		    silicon_errata_fix |= erratas[i].bitfield;
		  else
		    silicon_errata_warn |= erratas[i].bitfield;
		  arg += erratas[i].length;
		  break;
		}
	    if (i < 0)
	      {
		as_warn (_("Unrecognised CPU errata name starting here: %s"), arg);
		break;
	      }
	    if (*arg == 0)
	      break;
	    if (*arg != ',')
	      as_warn (_("Expecting comma after CPU errata name, not: %s"), arg);
	    else
	      arg ++;
	  }
	while (*arg != 0);
      }
      return 1;

    case OPTION_MMCU:
      if (arg == NULL)
	as_fatal (_("MCU option requires a name\n"));

      if (strcasecmp ("msp430", arg) == 0)
	selected_isa = MSP_ISA_430;
      else if (strcasecmp ("msp430xv2", arg) == 0)
	selected_isa = MSP_ISA_430Xv2;
      else if (strcasecmp ("msp430x", arg) == 0)
	selected_isa = MSP_ISA_430X;
      else
	{
	  int i;

	  for (i = ARRAY_SIZE (msp430_mcu_data); i--;)
	    if (strcasecmp (msp430_mcu_data[i].name, arg) == 0)
	      {
		switch (msp430_mcu_data[i].revision)
		  {
		  case 0: selected_isa = MSP_ISA_430; break;
		  case 1: selected_isa = MSP_ISA_430X; break;
		  case 2: selected_isa = MSP_ISA_430Xv2; break;
		  }
		break;
	    }
	}
      /* It is not an error if we do not match the MCU name.  */
      return 1;

    case OPTION_MCPU:
      if (strcmp (arg, "430") == 0
	  || strcasecmp (arg, "msp430") == 0)
	selected_isa = MSP_ISA_430;
      else if (strcasecmp (arg, "430x") == 0
	       || strcasecmp (arg, "msp430x") == 0)
	selected_isa = MSP_ISA_430X;
      else if (strcasecmp (arg, "430xv2") == 0
	       || strcasecmp (arg, "msp430xv2") == 0)
	selected_isa = MSP_ISA_430Xv2;
      else
	as_fatal (_("unrecognised argument to -mcpu option '%s'"), arg);
      return 1;

    case OPTION_RELAX:
      msp430_enable_relax = 1;
      return 1;

    case OPTION_POLYMORPHS:
      msp430_enable_polys = 1;
      return 1;

    case OPTION_LARGE:
      large_model = true;
      return 1;

    case OPTION_NO_INTR_NOPS:
      gen_interrupt_nops = false;
      return 1;
    case OPTION_INTR_NOPS:
      gen_interrupt_nops = true;
      return 1;

    case OPTION_WARN_INTR_NOPS:
      warn_interrupt_nops = true;
      return 1;
    case OPTION_NO_WARN_INTR_NOPS:
      warn_interrupt_nops = false;
      return 1;

    case OPTION_UNKNOWN_INTR_NOPS:
      do_unknown_interrupt_nops = true;
      return 1;
    case OPTION_NO_UNKNOWN_INTR_NOPS:
      do_unknown_interrupt_nops = false;
      return 1;

    case OPTION_MOVE_DATA:
      /* Silently ignored.  */
      return 1;

    case OPTION_DATA_REGION:
      if (strcmp (arg, "upper") == 0
	  || strcmp (arg, "either") == 0)
	upper_data_region_in_use = true;
      if (strcmp (arg, "upper") == 0
	  || strcmp (arg, "either") == 0
	  /* With data-region=none, the compiler has generated code assuming
	     data could be in the upper region, but nothing has been explicitly
	     placed there.  */
	  || strcmp (arg, "none") == 0)
	lower_data_region_only = false;
      return 1;
    }

  return 0;
}

/* The intention here is to have the mere presence of these sections
   cause the object to have a reference to a well-known symbol.  This
   reference pulls in the bits of the runtime (crt0) that initialize
   these sections.  Thus, for example, the startup code to call
   memset() to initialize .bss will only be linked in when there is a
   non-empty .bss section.  Otherwise, the call would exist but have a
   zero length parameter, which is a waste of memory and cycles.

   The code which initializes these sections should have a global
   label for these symbols, and should be marked with KEEP() in the
   linker script.  */

static void
msp430_make_init_symbols (const char * name)
{
  if (startswith (name, ".bss")
      || startswith (name, ".lower.bss")
      || startswith (name, ".either.bss")
      || startswith (name, ".gnu.linkonce.b."))
    (void) symbol_find_or_make ("__crt0_init_bss");

  if (startswith (name, ".data")
      || startswith (name, ".lower.data")
      || startswith (name, ".either.data")
      || startswith (name, ".gnu.linkonce.d."))
    (void) symbol_find_or_make ("__crt0_movedata");
  /* Note - data assigned to the .either.data section may end up being
     placed in the .upper.data section if the .lower.data section is
     full.  Hence the need to define the crt0 symbol.
     The linker may create upper or either data sections, even when none exist
     at the moment, so use the value of the data-region flag to determine if
     the symbol is needed.  */
  if (startswith (name, ".either.data")
      || startswith (name, ".upper.data")
      || upper_data_region_in_use)
    (void) symbol_find_or_make ("__crt0_move_highdata");

  /* See note about .either.data above.  */
  if (startswith (name, ".upper.bss")
      || startswith (name, ".either.bss")
      || upper_data_region_in_use)
    (void) symbol_find_or_make ("__crt0_init_highbss");

  /* The following symbols are for the crt0 functions that run through
     the different .*_array sections and call the functions placed there.
     - init_array stores global static C++ constructors to run before main.
     - preinit_array is not expected to ever be used for MSP430.
     GCC only places initialization functions for runtime "sanitizers"
     (i.e. {a,l,t,u}san) and "virtual table verification" in preinit_array.
     - fini_array stores global static C++ destructors to run after calling
     exit() or returning from main.
     __crt0_run_array is required to actually call the functions in the above
     arrays.  */
  if (startswith (name, ".init_array"))
    {
      (void) symbol_find_or_make ("__crt0_run_init_array");
      (void) symbol_find_or_make ("__crt0_run_array");
    }
  else if (startswith (name, ".preinit_array"))
    {
      (void) symbol_find_or_make ("__crt0_run_preinit_array");
      (void) symbol_find_or_make ("__crt0_run_array");
    }
  else if (startswith (name, ".fini_array"))
    {
      (void) symbol_find_or_make ("__crt0_run_fini_array");
      (void) symbol_find_or_make ("__crt0_run_array");
    }
}

static void
msp430_section (int arg)
{
  char * saved_ilp = input_line_pointer;
  const char * name = obj_elf_section_name ();

  msp430_make_init_symbols (name);

  input_line_pointer = saved_ilp;
  obj_elf_section (arg);
}

void
msp430_frob_section (asection *sec)
{
  const char *name = sec->name;

  if (sec->size == 0)
    return;

  msp430_make_init_symbols (name);
}

static void
msp430_lcomm (int ignore ATTRIBUTE_UNUSED)
{
  symbolS *symbolP = s_comm_internal (0, s_lcomm_internal);

  if (symbolP)
    symbol_get_bfdsym (symbolP)->flags |= BSF_OBJECT;
  (void) symbol_find_or_make ("__crt0_init_bss");
}

static void
msp430_comm (int needs_align)
{
  s_comm_internal (needs_align, elf_common_parse);
  (void) symbol_find_or_make ("__crt0_init_bss");
}

static void
msp430_refsym (int arg ATTRIBUTE_UNUSED)
{
  char sym_name[1024];
  input_line_pointer = extract_word (input_line_pointer, sym_name, 1024);

  (void) symbol_find_or_make (sym_name);
}

/* Handle a .mspabi_attribute or .gnu_attribute directive.
   attr_type is 0 for .mspabi_attribute or 1 for .gnu_attribute.
   This is only used for validating the attributes in the assembly file against
   the options gas has been invoked with.  If the attributes and options are
   compatible then we add the attributes to the assembly file in
   msp430_md_finish.  */
static void
msp430_object_attribute (int attr_type)
{
  char tag_name_s[32];
  char tag_value_s[32];
  int tag_name, tag_value;
  /* First operand is the tag name, second is the tag value e.g.
     ".mspabi_attribute 4, 2".  */
  input_line_pointer = extract_operand (input_line_pointer, tag_name_s, 32);
  input_line_pointer = extract_operand (input_line_pointer, tag_value_s, 32);
  tag_name = atoi (tag_name_s);
  tag_value = atoi (tag_value_s);
  /* If the attribute directive is present, the tag_value should never be set
     to 0.  */
  if (tag_name == 0 || tag_value == 0)
    as_bad (_("bad arguments \"%s\" and/or \"%s\" in %s directive"),
	      tag_name_s, tag_value_s, (attr_type ? ".gnu_attribute"
					: ".mspabi_attribute"));
  else if (attr_type == 0)
    /* Handle .mspabi_attribute.  */
    switch (tag_name)
      {
      case OFBA_MSPABI_Tag_ISA:
	switch (tag_value)
	  {
	  case OFBA_MSPABI_Val_ISA_MSP430:
	    if (target_is_430x ())
	      as_bad (_("file was compiled for the 430 ISA but the %s ISA is "
			"selected"), (target_is_430xv2 () ? "430X" : "430Xv2"));
	    break;
	  case OFBA_MSPABI_Val_ISA_MSP430X:
	    if (!target_is_430x ())
	      as_bad (_("file was compiled for the 430X ISA but the 430 ISA is "
			"selected"));
	    break;
	  default:
	    as_bad (_("unknown MSPABI build attribute value '%d' for "
		      "OFBA_MSPABI_Tag_ISA(%d) in .mspabi_attribute directive"),
		    tag_value, OFBA_MSPABI_Tag_ISA);
	    break;
	  }
	break;
      case OFBA_MSPABI_Tag_Code_Model:
	/* Fall through.  */
      case OFBA_MSPABI_Tag_Data_Model:
	/* FIXME: Might we want to set the memory model to large if the assembly
	   file has the large model attribute, but -ml has not been passed?  */
	switch (tag_value)
	  {
	  case OFBA_MSPABI_Val_Code_Model_SMALL:
	    if (large_model)
	      as_bad (_("file was compiled for the small memory model, but the "
			"large memory model is selected"));
	    break;
	  case OFBA_MSPABI_Val_Code_Model_LARGE:
	    if (!large_model)
	      as_bad (_("file was compiled for the large memory model, "
			"but the small memory model is selected"));
	    break;
	  default:
	    as_bad (_("unknown MSPABI build attribute value '%d' for %s(%d) "
		      "in .mspabi_attribute directive"), tag_value,
		    (tag_name == OFBA_MSPABI_Tag_Code_Model
		     ? "OFBA_MSPABI_Tag_Code_Model"
		     : "OFBA_MSPABI_Tag_Data_Model"),
		    (tag_name == OFBA_MSPABI_Tag_Code_Model
		     ? OFBA_MSPABI_Tag_Code_Model
		     : OFBA_MSPABI_Tag_Data_Model));
	    break;
	  }
	break;
      default:
	as_bad (_("unknown MSPABI build attribute tag '%d' in "
		  ".mspabi_attribute directive"), tag_name);
	break;
      }
  else if (attr_type == 1)
    /* Handle .gnu_attribute.  */
    switch (tag_name)
      {
      case Tag_GNU_MSP430_Data_Region:
	/* This attribute is only applicable in the large memory model.  */
	if (!large_model)
	  break;
	switch (tag_value)
	  {
	  case Val_GNU_MSP430_Data_Region_Lower:
	    if (!lower_data_region_only)
	      as_bad (_("file was compiled assuming all data will be in the "
			"lower memory region, but the upper region is in use"));
	    break;
	  case Val_GNU_MSP430_Data_Region_Any:
	    if (lower_data_region_only)
	      as_bad (_("file was compiled assuming data could be in the upper "
			"memory region, but the lower data region is "
			"exclusively in use"));
	    break;
	  default:
	    as_bad (_("unknown GNU build attribute value '%d' for "
		      "Tag_GNU_MSP430_Data_Region(%d) in .gnu_attribute "
		      "directive"), tag_value, Tag_GNU_MSP430_Data_Region);
	  }
      }
  else
    as_bad (_("internal: unexpected argument '%d' to msp430_object_attribute"),
	    attr_type);
}

const pseudo_typeS md_pseudo_table[] =
{
  {"arch", msp430_set_arch, OPTION_MMCU},
  {"cpu", msp430_set_arch, OPTION_MCPU},
  {"profiler", msp430_profiler, 0},
  {"section", msp430_section, 0},
  {"section.s", msp430_section, 0},
  {"sect", msp430_section, 0},
  {"sect.s", msp430_section, 0},
  {"pushsection", msp430_section, 1},
  {"refsym", msp430_refsym, 0},
  {"comm", msp430_comm, 0},
  {"lcomm", msp430_lcomm, 0},
  {"mspabi_attribute", msp430_object_attribute, 0},
  {"gnu_attribute", msp430_object_attribute, 1},
  {NULL, NULL, 0}
};

const char *md_shortopts = "mm:,mP,mQ,ml,mN,mn,my,mY,mu,mU";

struct option md_longopts[] =
{
  {"msilicon-errata", required_argument, NULL, OPTION_SILICON_ERRATA},
  {"msilicon-errata-warn", required_argument, NULL, OPTION_SILICON_ERRATA_WARN},
  {"mmcu", required_argument, NULL, OPTION_MMCU},
  {"mcpu", required_argument, NULL, OPTION_MCPU},
  {"mP", no_argument, NULL, OPTION_POLYMORPHS},
  {"mQ", no_argument, NULL, OPTION_RELAX},
  {"ml", no_argument, NULL, OPTION_LARGE},
  {"mN", no_argument, NULL, OPTION_NO_INTR_NOPS},
  {"mn", no_argument, NULL, OPTION_INTR_NOPS},
  {"mY", no_argument, NULL, OPTION_NO_WARN_INTR_NOPS},
  {"my", no_argument, NULL, OPTION_WARN_INTR_NOPS},
  {"mu", no_argument, NULL, OPTION_UNKNOWN_INTR_NOPS},
  {"mU", no_argument, NULL, OPTION_NO_UNKNOWN_INTR_NOPS},
  {"md", no_argument, NULL, OPTION_MOVE_DATA},
  {"mdata-region", required_argument, NULL, OPTION_DATA_REGION},
  {NULL, no_argument, NULL, 0}
};

size_t md_longopts_size = sizeof (md_longopts);

void
md_show_usage (FILE * stream)
{
  fprintf (stream,
	   _("MSP430 options:\n"
	     "  -mmcu=<msp430-name>     - select microcontroller type\n"
             "  -mcpu={430|430x|430xv2} - select microcontroller architecture\n"));
  fprintf (stream,
	   _("  -msilicon-errata=<name>[,<name>...] - enable fixups for silicon errata\n"
	     "  -msilicon-errata-warn=<name>[,<name>...] - warn when a fixup might be needed\n"
	     "   supported errata names: cpu4, cpu8, cpu11, cpu12, cpu13, cpu19\n"));
  fprintf (stream,
	   _("  -mQ - enable relaxation at assembly time. DANGEROUS!\n"
	     "  -mP - enable polymorph instructions\n"));
  fprintf (stream,
	   _("  -ml - enable large code model\n"));
  fprintf (stream,
	   _("  -mN - do not insert NOPs after changing interrupts (default)\n"));
  fprintf (stream,
	   _("  -mn - insert a NOP after changing interrupts\n"));
  fprintf (stream,
	   _("  -mY - do not warn about missing NOPs after changing interrupts\n"));
  fprintf (stream,
	   _("  -my - warn about missing NOPs after changing interrupts (default)\n"));
  fprintf (stream,
	   _("  -mU - for an instruction which changes interrupt state, but where it is not\n"
	     "        known how the state is changed, do not warn/insert NOPs\n"));
  fprintf (stream,
	   _("  -mu - for an instruction which changes interrupt state, but where it is not\n"
	     "        known how the state is changed, warn/insert NOPs (default)\n"
	     "        -mn and/or -my are required for this to have any effect\n"));
  fprintf (stream,
	   _("  -mdata-region={none|lower|upper|either} - select region data will be\n"
	     "    placed in.\n"));
}

symbolS *
md_undefined_symbol (char * name ATTRIBUTE_UNUSED)
{
  return NULL;
}

static char *
extract_cmd (char * from, char * to, int limit)
{
  int size = 0;

  while (*from && ! ISSPACE (*from) && *from != '.' && limit > size)
    {
      *(to + size) = *from;
      from++;
      size++;
    }

  *(to + size) = 0;

  return from;
}

const char *
md_atof (int type, char * litP, int * sizeP)
{
  return ieee_md_atof (type, litP, sizeP, false);
}

void
md_begin (void)
{
  struct msp430_opcode_s * opcode;
  msp430_hash = str_htab_create ();

  for (opcode = msp430_opcodes; opcode->name; opcode++)
    str_hash_insert (msp430_hash, opcode->name, opcode, 0);

  bfd_set_arch_mach (stdoutput, TARGET_ARCH,
		     target_is_430x () ? bfd_mach_msp430x : bfd_mach_msp11);

  /*  Set linkrelax here to avoid fixups in most sections.  */
  linkrelax = 1;
}

static inline bool
is_regname_end (char c)
{
  return (c == 0 || ! ISALNUM (c));
}
  
/* Returns the register number equivalent to the string T.
   Returns -1 if there is no such register.
   Skips a leading 'r' or 'R' character if there is one.
   Handles the register aliases PC and SP.  */

static signed int
check_reg (char * t)
{
  char * endt;
  signed long int val;

  if (t == NULL || t[0] == 0)
    return -1;

  if (*t == 'r' || *t == 'R')
    ++t;

  if (strncasecmp (t, "pc", 2) == 0 && is_regname_end (t[2]))
    return 0;

  if (strncasecmp (t, "sp", 2) == 0 && is_regname_end (t[2]))
    return 1;

  if (strncasecmp (t, "sr", 2) == 0 && is_regname_end (t[2]))
    return 2;

  if (*t == '0' && is_regname_end (t[1]))
    return 0;

  val = strtol (t, & endt, 0);

  if (val < 1 || val > 15)
    return -1;

  if (is_regname_end (*endt))
    return val;

  return -1;
}

static int
msp430_srcoperand (struct msp430_operand_s * op,
		   char * l,
		   int bin,
		   bool * imm_op,
		   bool allow_20bit_values,
		   bool constants_allowed)
{
  char * end;
  char *__tl = l;

  /* Check if an immediate #VALUE.  The hash sign should be only at the beginning!  */
  if (*l == '#')
    {
      char *h = l;
      int vshift = -1;
      int rval = 0;
      /* Use all parts of the constant expression by default.  */
      enum msp430_expp_e expp = MSP_EXPP_ALL;

      /* Check if there is:
	 llo(x) - least significant 16 bits, x &= 0xffff
	 lhi(x) - x = (x >> 16) & 0xffff,
	 hlo(x) - x = (x >> 32) & 0xffff,
	 hhi(x) - x = (x >> 48) & 0xffff
	 The value _MUST_ be an immediate expression: #hlo(1231231231).  */

      *imm_op = true;

      if (strncasecmp (h, "#llo(", 5) == 0)
	{
	  vshift = 0;
	  rval = 3;
	  expp = MSP_EXPP_LLO;
	}
      else if (strncasecmp (h, "#lhi(", 5) == 0)
	{
	  vshift = 1;
	  rval = 3;
	  expp = MSP_EXPP_LHI;
	}
      else if (strncasecmp (h, "#hlo(", 5) == 0)
	{
	  vshift = 2;
	  rval = 3;
	  expp = MSP_EXPP_HLO;
	}
      else if (strncasecmp (h, "#hhi(", 5) == 0)
	{
	  vshift = 3;
	  rval = 3;
	  expp = MSP_EXPP_HHI;
	}
      else if (strncasecmp (h, "#lo(", 4) == 0)
	{
	  vshift = 0;
	  rval = 2;
	  expp = MSP_EXPP_LO;
	}
      else if (strncasecmp (h, "#hi(", 4) == 0)
	{
	  vshift = 1;
	  rval = 2;
	  expp = MSP_EXPP_HI;
	}

      op->reg = 0;		/* Reg PC.  */
      op->am = 3;
      op->ol = 1;		/* Immediate will follow an instruction.  */
      __tl = h + 1 + rval;
      op->mode = OP_EXP;
      op->vshift = vshift;
      op->expp = expp;

      end = parse_exp (__tl, &(op->exp));
      if (end != NULL && *end != 0 && *end != ')' )
	{
	  as_bad (_("extra characters '%s' at end of immediate expression '%s'"), end, l);
	  return 1;
	}
      if (op->exp.X_op == O_constant)
	{
	  int x = op->exp.X_add_number;

	  if (vshift == 0)
	    {
	      x = x & 0xffff;
	      op->exp.X_add_number = x;
	    }
	  else if (vshift == 1)
	    {
	      x = (x >> 16) & 0xffff;
	      op->exp.X_add_number = x;
	      op->vshift = 0;
	    }
	  else if (vshift > 1)
	    {
	      if (x < 0)
		op->exp.X_add_number = -1;
	      else
		op->exp.X_add_number = 0;	/* Nothing left.  */
	      x = op->exp.X_add_number;
	      op->vshift = 0;
	    }

	  if (allow_20bit_values)
	    {
	      if (op->exp.X_add_number > 0xfffff || op->exp.X_add_number < -524288)
		{
		  as_bad (_("value 0x%x out of extended range."), x);
		  return 1;
		}
	    }
	  else if (op->exp.X_add_number > 65535 || op->exp.X_add_number < -32768)
	    {
	      as_bad (_("value %d out of range. Use #lo() or #hi()"), x);
	      return 1;
	    }

	  /* Now check constants.  */
	  /* Substitute register mode with a constant generator if applicable.  */

	  if (!allow_20bit_values)
	    x = (short) x;	/* Extend sign.  */

	  if (! constants_allowed)
	    ;
	  else if (x == 0)
	    {
	      op->reg = 3;
	      op->am = 0;
	      op->ol = 0;
	      op->mode = OP_REG;
	    }
	  else if (x == 1)
	    {
	      op->reg = 3;
	      op->am = 1;
	      op->ol = 0;
	      op->mode = OP_REG;
	    }
	  else if (x == 2)
	    {
	      op->reg = 3;
	      op->am = 2;
	      op->ol = 0;
	      op->mode = OP_REG;
	    }
	  else if (x == -1)
	    {
	      op->reg = 3;
	      op->am = 3;
	      op->ol = 0;
	      op->mode = OP_REG;
	    }
	  else if (x == 4)
	    {
	      if (bin == 0x1200 && ! target_is_430x ())
		{
		  /* CPU4: The shorter form of PUSH #4 is not supported on MSP430.  */
		  if (silicon_errata_warn & SILICON_ERRATA_CPU4)
		    as_warn (_("cpu4: not converting PUSH #4 to shorter form"));
		  /* No need to check silicon_errata_fixes - this fix is always implemented.  */
		}
	      else
		{
		  op->reg = 2;
		  op->am = 2;
		  op->ol = 0;
		  op->mode = OP_REG;
		}
	    }
	  else if (x == 8)
	    {
	      if (bin == 0x1200 && ! target_is_430x ())
		{
		  /* CPU4: The shorter form of PUSH #8 is not supported on MSP430.  */
		  if (silicon_errata_warn & SILICON_ERRATA_CPU4)
		    as_warn (_("cpu4: not converting PUSH #8 to shorter form"));
		}
	      else
		{
		  op->reg = 2;
		  op->am = 3;
		  op->ol = 0;
		  op->mode = OP_REG;
		}
	    }
	}
      else if (op->exp.X_op == O_symbol)
	{
	  if (vshift > 1)
	    as_bad (_("error: unsupported #foo() directive used on symbol"));
	  op->mode = OP_EXP;
	}
      else if (op->exp.X_op == O_big)
	{
	  short x;

	  if (vshift != -1)
	    {
	      op->exp.X_op = O_constant;
	      op->exp.X_add_number = 0xffff & generic_bignum[vshift];
	      x = op->exp.X_add_number;
	      op->vshift = 0;
	    }
	  else
	    {
	      as_bad (_
		      ("unknown expression in operand %s.  Use #llo(), #lhi(), #hlo() or #hhi()"),
		      l);
	      return 1;
	    }

	  if (x == 0)
	    {
	      op->reg = 3;
	      op->am = 0;
	      op->ol = 0;
	      op->mode = OP_REG;
	    }
	  else if (x == 1)
	    {
	      op->reg = 3;
	      op->am = 1;
	      op->ol = 0;
	      op->mode = OP_REG;
	    }
	  else if (x == 2)
	    {
	      op->reg = 3;
	      op->am = 2;
	      op->ol = 0;
	      op->mode = OP_REG;
	    }
	  else if (x == -1)
	    {
	      op->reg = 3;
	      op->am = 3;
	      op->ol = 0;
	      op->mode = OP_REG;
	    }
	  else if (x == 4)
	    {
	      op->reg = 2;
	      op->am = 2;
	      op->ol = 0;
	      op->mode = OP_REG;
	    }
	  else if (x == 8)
	    {
	      op->reg = 2;
	      op->am = 3;
	      op->ol = 0;
	      op->mode = OP_REG;
	    }
	}
      /* Redundant (yet) check.  */
      else if (op->exp.X_op == O_register)
	as_bad
	  (_("Registers cannot be used within immediate expression [%s]"), l);
      else
	as_bad (_("unknown operand %s"), l);

      return 0;
    }

  /* Check if absolute &VALUE (assume that we can construct something like ((a&b)<<7 + 25).  */
  if (*l == '&')
    {
      char *h = l;

      op->reg = 2;		/* Reg 2 in absolute addr mode.  */
      op->am = 1;		/* Mode As == 01 bin.  */
      op->ol = 1;		/* Immediate value followed by instruction.  */
      __tl = h + 1;
      end = parse_exp (__tl, &(op->exp));
      if (end != NULL && *end != 0)
	{
	  as_bad (_("extra characters '%s' at the end of absolute operand '%s'"), end, l);
	  return 1;
	}
      op->mode = OP_EXP;
      op->vshift = 0;
      op->expp = MSP_EXPP_ALL;
      if (op->exp.X_op == O_constant)
	{
	  int x = op->exp.X_add_number;

	  if (allow_20bit_values)
	    {
	      if (x > 0xfffff || x < -(0x7ffff))
		{
		  as_bad (_("value 0x%x out of extended range."), x);
		  return 1;
		}
	    }
	  else if (x > 65535 || x < -32768)
	    {
	      as_bad (_("value out of range: 0x%x"), x);
	      return 1;
	    }
	}
      else if (op->exp.X_op == O_symbol)
	;
      else
	{
	  /* Redundant (yet) check.  */
	  if (op->exp.X_op == O_register)
	    as_bad
	      (_("Registers cannot be used within absolute expression [%s]"), l);
	  else
	    as_bad (_("unknown expression in operand %s"), l);
	  return 1;
	}
      return 0;
    }

  /* Check if indirect register mode @Rn / postincrement @Rn+.  */
  if (*l == '@')
    {
      char *t = l;
      char *m = strchr (l, '+');

      if (t != l)
	{
	  as_bad (_("unknown addressing mode %s"), l);
	  return 1;
	}

      t++;

      if ((op->reg = check_reg (t)) == -1)
	{
	  as_bad (_("Bad register name %s"), t);
	  return 1;
	}

      op->mode = OP_REG;
      op->am = m ? 3 : 2;
      op->ol = 0;

      /* PC cannot be used in indirect addressing.  */
      if (target_is_430xv2 () && op->reg == 0)
	{
	  as_bad (_("cannot use indirect addressing with the PC"));
	  return 1;
	}

      return 0;
    }

  /* Check if register indexed X(Rn).  */
  do
    {
      char *h = strrchr (l, '(');
      char *m = strrchr (l, ')');
      char *t;

      *imm_op = true;

      if (!h)
	break;
      if (!m)
	{
	  as_bad (_("')' required"));
	  return 1;
	}

      t = h;
      op->am = 1;
      op->ol = 1;

      /* Extract a register.  */
      if ((op->reg = check_reg (t + 1)) == -1)
	{
	  as_bad (_
		  ("unknown operator %s. Did you mean X(Rn) or #[hl][hl][oi](CONST) ?"),
		  l);
	  return 1;
	}

      if (op->reg == 2)
	{
	  as_bad (_("r2 should not be used in indexed addressing mode"));
	  return 1;
	}

      /* Extract constant.  */
      __tl = l;
      *h = 0;
      op->mode = OP_EXP;
      op->vshift = 0;
      op->expp = MSP_EXPP_ALL;
      end = parse_exp (__tl, &(op->exp));
      if (end != NULL && *end != 0)
	{
	  as_bad (_("extra characters '%s' at end of operand '%s'"), end, l);
	  return 1;
	}
      if (op->exp.X_op == O_constant)
	{
	  int x = op->exp.X_add_number;

	  if (allow_20bit_values)
	    {
	      if (x > 0xfffff || x < - (0x7ffff))
		{
		  as_bad (_("value 0x%x out of extended range."), x);
		  return 1;
		}
	    }
	  else if (x > 65535 || x < -32768)
	    {
	      as_bad (_("value out of range: 0x%x"), x);
	      return 1;
	    }

	  if (x == 0)
	    {
	      op->mode = OP_REG;
	      op->am = 2;
	      op->ol = 0;
	      return 0;
	    }

	  if (op->reg == 1 && (x & 1))
	    {
	      if (silicon_errata_fix & SILICON_ERRATA_CPU8)
		as_bad (_("CPU8: Stack pointer accessed with an odd offset"));
	      else if (silicon_errata_warn & SILICON_ERRATA_CPU8)
		as_warn (_("CPU8: Stack pointer accessed with an odd offset"));
	    }
	}
      else if (op->exp.X_op == O_symbol)
	;
      else
	{
	  /* Redundant (yet) check.  */
	  if (op->exp.X_op == O_register)
	    as_bad
	      (_("Registers cannot be used as a prefix of indexed expression [%s]"), l);
	  else
	    as_bad (_("unknown expression in operand %s"), l);
	  return 1;
	}

      return 0;
    }
  while (0);

  /* Possibly register mode 'mov r1,r2'.  */
  if ((op->reg = check_reg (l)) != -1)
    {
      op->mode = OP_REG;
      op->am = 0;
      op->ol = 0;
      return 0;
    }

  /* Symbolic mode 'mov a, b' == 'mov x(pc), y(pc)'.  */
  op->mode = OP_EXP;
  op->reg = 0;		/* PC relative... be careful.  */
  /* An expression starting with a minus sign is a constant, not an address.  */
  op->am = (*l == '-' ? 3 : 1);
  op->ol = 1;
  op->vshift = 0;
  op->expp = MSP_EXPP_ALL;
  __tl = l;
  end = parse_exp (__tl, &(op->exp));
  if (end != NULL && * end != 0)
    {
      as_bad (_("extra characters '%s' at end of operand '%s'"), end, l);
      return 1;
    }
  return 0;
}


static int
msp430_dstoperand (struct msp430_operand_s * op,
		   char * l,
		   int bin,
		   bool allow_20bit_values,
		   bool constants_allowed)
{
  bool dummy;
  int ret = msp430_srcoperand (op, l, bin, & dummy,
			       allow_20bit_values,
			       constants_allowed);

  if (ret)
    return ret;

  if (op->am == 2)
    {
      char *__tl = (char *) "0";

      op->mode = OP_EXP;
      op->am = 1;
      op->ol = 1;
      op->vshift = 0;
      op->expp = MSP_EXPP_ALL;
      (void) parse_exp (__tl, &(op->exp));

      if (op->exp.X_op != O_constant || op->exp.X_add_number != 0)
	{
	  as_bad (_("Internal bug. Try to use 0(r%d) instead of @r%d"),
		  op->reg, op->reg);
	  return 1;
	}
      return 0;
    }

  if (op->am > 1)
    {
      as_bad (_
	      ("this addressing mode is not applicable for destination operand"));
      return 1;
    }
  return 0;
}

/* Attempt to encode a MOVA instruction with the given operands.
   Returns the length of the encoded instruction if successful
   or 0 upon failure.  If the encoding fails, an error message
   will be returned if a pointer is provided.  */

static int
try_encode_mova (bool imm_op,
		 int bin,
		 struct msp430_operand_s * op1,
		 struct msp430_operand_s * op2,
		 const char ** error_message_return)
{
  short ZEROS = 0;
  char *frag;
  int where;

  /* Only a restricted subset of the normal MSP430 addressing modes
     are supported here, so check for the ones that are allowed.  */
  if (imm_op)
    {
      if (op1->mode == OP_EXP)
	{
	  if (op2->mode != OP_REG)
	    {
	      if (error_message_return != NULL)
		* error_message_return = _("expected register as second argument of %s");
	      return 0;
	    }

	  if (op1->am == 3)
	    {
	      /* MOVA #imm20, Rdst.  */
	      bin |= 0x80 | op2->reg;
	      frag = frag_more (4);
	      where = frag - frag_now->fr_literal;
	      if (op1->exp.X_op == O_constant)
		{
		  bin |= ((op1->exp.X_add_number >> 16) & 0xf) << 8;
		  bfd_putl16 ((bfd_vma) bin, frag);
		  bfd_putl16 (op1->exp.X_add_number & 0xffff, frag + 2);
		}
	      else
		{
		  bfd_putl16 ((bfd_vma) bin, frag);
		  fix_new_exp (frag_now, where, 4, &(op1->exp), false,
			       BFD_RELOC_MSP430X_ABS20_ADR_SRC);
		  bfd_putl16 ((bfd_vma) ZEROS, frag + 2);
		}

	      return 4;
	    }
	  else if (op1->am == 1)
	    {
	      /* MOVA z16(Rsrc), Rdst.  */
	      bin |= 0x30 | (op1->reg << 8) | op2->reg;
	      frag = frag_more (4);
	      where = frag - frag_now->fr_literal;
	      bfd_putl16 ((bfd_vma) bin, frag);
	      if (op1->exp.X_op == O_constant)
		{
		  if (op1->exp.X_add_number > 0xffff
		      || op1->exp.X_add_number < -(0x7fff))
		    {
		      if (error_message_return != NULL)
			* error_message_return = _("index value too big for %s");
		      return 0;
		    }
		  bfd_putl16 (op1->exp.X_add_number & 0xffff, frag + 2);
		}
	      else
		{
		  bfd_putl16 ((bfd_vma) ZEROS, frag + 2);
		  fix_new_exp (frag_now, where + 2, 2, &(op1->exp), false,
			       op1->reg == 0 ?
			       BFD_RELOC_MSP430X_PCR16 :
			       BFD_RELOC_MSP430X_ABS16);
		}
	      return 4;
	    }

	  if (error_message_return != NULL)
	    * error_message_return = _("unexpected addressing mode for %s");
	  return 0;
	}
      else if (op1->am == 0)
	{
	  /* MOVA Rsrc, ... */
	  if (op2->mode == OP_REG)
	    {
	      bin |= 0xc0 | (op1->reg << 8) | op2->reg;
	      frag = frag_more (2);
	      where = frag - frag_now->fr_literal;
	      bfd_putl16 ((bfd_vma) bin, frag);
	      return 2;
	    }
	  else if (op2->am == 1)
	    {
	      if (op2->reg == 2)
		{
		  /* MOVA Rsrc, &abs20.  */
		  bin |= 0x60 | (op1->reg << 8);
		  frag = frag_more (4);
		  where = frag - frag_now->fr_literal;
		  if (op2->exp.X_op == O_constant)
		    {
		      bin |= (op2->exp.X_add_number >> 16) & 0xf;
		      bfd_putl16 ((bfd_vma) bin, frag);
		      bfd_putl16 (op2->exp.X_add_number & 0xffff, frag + 2);
		    }
		  else
		    {
		      bfd_putl16 ((bfd_vma) bin, frag);
		      bfd_putl16 ((bfd_vma) ZEROS, frag + 2);
		      fix_new_exp (frag_now, where, 4, &(op2->exp), false,
				   BFD_RELOC_MSP430X_ABS20_ADR_DST);
		    }
		  return 4;
		}

	      /* MOVA Rsrc, z16(Rdst).  */
	      bin |= 0x70 | (op1->reg << 8) | op2->reg;
	      frag = frag_more (4);
	      where = frag - frag_now->fr_literal;
	      bfd_putl16 ((bfd_vma) bin, frag);
	      if (op2->exp.X_op == O_constant)
		{
		  if (op2->exp.X_add_number > 0xffff
		      || op2->exp.X_add_number < -(0x7fff))
		    {
		      if (error_message_return != NULL)
			* error_message_return = _("index value too big for %s");
		      return 0;
		    }
		  bfd_putl16 (op2->exp.X_add_number & 0xffff, frag + 2);
		}
	      else
		{
		  bfd_putl16 ((bfd_vma) ZEROS, frag + 2);
		  fix_new_exp (frag_now, where + 2, 2, &(op2->exp), false,
			       op2->reg == 0 ?
			       BFD_RELOC_MSP430X_PCR16 :
			       BFD_RELOC_MSP430X_ABS16);
		}
	      return 4;
	    }

	  if (error_message_return != NULL)
	    * error_message_return = _("unexpected addressing mode for %s");
	  return 0;
	}
    }

  /* imm_op == false.  */

  if (op1->reg == 2 && op1->am == 1 && op1->mode == OP_EXP)
    {
      /* MOVA &abs20, Rdst.  */
      if (op2->mode != OP_REG)
	{
	  if (error_message_return != NULL)
	    * error_message_return = _("expected register as second argument of %s");
	  return 0;
	}

      if (op2->reg == 2 || op2->reg == 3)
	{
	  if (error_message_return != NULL)
	    * error_message_return = _("constant generator destination register found in %s");
	  return 0;
	}

      bin |= 0x20 | op2->reg;
      frag = frag_more (4);
      where = frag - frag_now->fr_literal;
      if (op1->exp.X_op == O_constant)
	{
	  bin |= ((op1->exp.X_add_number >> 16) & 0xf) << 8;
	  bfd_putl16 ((bfd_vma) bin, frag);
	  bfd_putl16 (op1->exp.X_add_number & 0xffff, frag + 2);
	}
      else
	{
	  bfd_putl16 ((bfd_vma) bin, frag);
	  bfd_putl16 ((bfd_vma) ZEROS, frag + 2);
	  fix_new_exp (frag_now, where, 4, &(op1->exp), false,
		       BFD_RELOC_MSP430X_ABS20_ADR_SRC);
	}
      return 4;
    }
  else if (op1->mode == OP_REG)
    {
      if (op1->am == 3)
	{
	  /* MOVA @Rsrc+, Rdst.  */
	  if (op2->mode != OP_REG)
	    {
	      if (error_message_return != NULL)
		* error_message_return = _("expected register as second argument of %s");
	      return 0;
	    }

	  if (op2->reg == 2 || op2->reg == 3)
	    {
	      if (error_message_return != NULL)
		* error_message_return = _("constant generator destination register found in %s");
	      return 0;
	    }

	  if (op1->reg == 2 || op1->reg == 3)
	    {
	      if (error_message_return != NULL)
		* error_message_return = _("constant generator source register found in %s");
	      return 0;
	    }

	  bin |= 0x10 | (op1->reg << 8) | op2->reg;
	  frag = frag_more (2);
	  where = frag - frag_now->fr_literal;
	  bfd_putl16 ((bfd_vma) bin, frag);
	  return 2;
	}
      else if (op1->am == 2)
	{
	  /* MOVA @Rsrc,Rdst */
	  if (op2->mode != OP_REG)
	    {
	      if (error_message_return != NULL)
		* error_message_return = _("expected register as second argument of %s");
	      return 0;
	    }

	  if (op2->reg == 2 || op2->reg == 3)
	    {
	      if (error_message_return != NULL)
		* error_message_return = _("constant generator destination register found in %s");
	      return 0;
	    }

	  if (op1->reg == 2 || op1->reg == 3)
	    {
	      if (error_message_return != NULL)
		* error_message_return = _("constant generator source register found in %s");
	      return 0;
	    }

	  bin |= (op1->reg << 8) | op2->reg;
	  frag = frag_more (2);
	  where = frag - frag_now->fr_literal;
	  bfd_putl16 ((bfd_vma) bin, frag);
	  return 2;
	}
    }

  if (error_message_return != NULL)
    * error_message_return = _("unexpected addressing mode for %s");

  return 0;
}

#define NOP_CHECK_INTERRUPT  (1 << 0)
#define NOP_CHECK_CPU12      (1 << 1)
#define NOP_CHECK_CPU19      (1 << 2)

static signed int check_for_nop = 0;

#define is_opcode(NAME) (strcmp (opcode->name, NAME) == 0)

/* is_{e,d}int only check the explicit enabling/disabling of interrupts.
   For MOV insns, more sophisticated processing is needed to determine if they
   result in enabling/disabling interrupts.  */
#define is_dint(OPCODE, BIN) ((strcmp (OPCODE, "dint") == 0) \
				   || ((strcmp (OPCODE, "bic") == 0) \
				       && BIN == 0xc232) \
				   || ((strcmp (OPCODE, "clr") == 0) \
				       && BIN == 0x4302))

#define is_eint(OPCODE, BIN) ((strcmp (OPCODE, "eint") == 0) \
				   || ((strcmp (OPCODE, "bis") == 0) \
				       && BIN == 0xd232))

const char * const INSERT_NOP_BEFORE_EINT = "NOP inserted here, before an interrupt enable instruction";
const char * const INSERT_NOP_AFTER_DINT = "NOP inserted here, after an interrupt disable instruction";
const char * const INSERT_NOP_AFTER_EINT = "NOP inserted here, after an interrupt enable instruction";
const char * const INSERT_NOP_BEFORE_UNKNOWN = "NOP inserted here, before this interrupt state change";
const char * const INSERT_NOP_AFTER_UNKNOWN ="NOP inserted here, after the instruction that changed interrupt state";
const char * const INSERT_NOP_AT_EOF = "NOP inserted after the interrupt state change at the end of the file";

const char * const WARN_NOP_BEFORE_EINT = "a NOP might be needed here, before an interrupt enable instruction";
const char * const WARN_NOP_AFTER_DINT = "a NOP might be needed here, after an interrupt disable instruction";
const char * const WARN_NOP_AFTER_EINT = "a NOP might be needed here, after an interrupt enable instruction";
const char * const WARN_NOP_BEFORE_UNKNOWN = "a NOP might be needed here, before this interrupt state change";
const char * const WARN_NOP_AFTER_UNKNOWN = "a NOP might also be needed here, after the instruction that changed interrupt state";
const char * const WARN_NOP_AT_EOF = "a NOP might be needed after the interrupt state change at the end of the file";

static void
gen_nop (void)
{
  char *frag;
  frag = frag_more (2);
  bfd_putl16 ((bfd_vma) 0x4303 /* NOP */, frag);
  dwarf2_emit_insn (2);
}

/* Insert/inform about adding a NOP if this insn enables interrupts.  */

static void
warn_eint_nop (bool prev_insn_is_nop, bool prev_insn_is_dint)
{
  if (prev_insn_is_nop
      /* If the last insn was a DINT, we will have already warned that a NOP is
	 required after it.  */
      || prev_insn_is_dint
      /* 430 ISA does not require a NOP before EINT.  */
      || (! target_is_430x ()))
    return;

  if (gen_interrupt_nops)
    {
      gen_nop ();
      if (warn_interrupt_nops)
	as_warn (_(INSERT_NOP_BEFORE_EINT));
    }
  else if (warn_interrupt_nops)
    as_warn (_(WARN_NOP_BEFORE_EINT));
}

/* Use when unsure what effect the insn will have on the interrupt status,
   to insert/warn about adding a NOP before the current insn.  */

static void
warn_unsure_interrupt (bool prev_insn_is_nop,
		       bool prev_insn_is_dint)
{
  if (prev_insn_is_nop
      /* If the last insn was a DINT, we will have already warned that a NOP is
	 required after it.  */
      || prev_insn_is_dint
      /* 430 ISA does not require a NOP before EINT or DINT.  */
      || (! target_is_430x ()))
    return;

  if (gen_interrupt_nops)
    {
      gen_nop ();
      if (warn_interrupt_nops)
	as_warn (_(INSERT_NOP_BEFORE_UNKNOWN));
    }
  else if (warn_interrupt_nops)
    as_warn (_(WARN_NOP_BEFORE_UNKNOWN));
}

/* Parse instruction operands.
   Return binary opcode.  */

static unsigned int
msp430_operands (struct msp430_opcode_s * opcode, char * line)
{
  int bin = opcode->bin_opcode;	/* Opcode mask.  */
  int insn_length = 0;
  char l1[MAX_OP_LEN], l2[MAX_OP_LEN];
  char *frag;
  char *end;
  int where;
  struct msp430_operand_s op1, op2;
  int res = 0;
  static short ZEROS = 0;
  bool byte_op, imm_op;
  int op_length = 0;
  int fmt;
  int extended = 0x1800;
  bool extended_op = false;
  bool addr_op;
  const char * error_message;
  static signed int repeat_count = 0;
  static bool prev_insn_is_nop = false;
  static bool prev_insn_is_dint = false;
  static bool prev_insn_is_eint = false;
  /* We might decide before the end of the function that the current insn is
     equivalent to DINT/EINT.  */
  bool this_insn_is_dint = false;
  bool this_insn_is_eint = false;
  bool fix_emitted;

  /* Opcode is the one from opcodes table
     line contains something like
     [.w] @r2+, 5(R1)
     or
     .b @r2+, 5(R1).  */

  byte_op = false;
  addr_op = false;
  if (*line == '.')
    {
      bool check = false;
      ++ line;

      switch (TOLOWER (* line))
	{
	case 'b':
	  /* Byte operation.  */
	  bin |= BYTE_OPERATION;
	  byte_op = true;
	  check = true;
	  break;

	case 'a':
	  /* "Address" ops work on 20-bit values.  */
	  addr_op = true;
	  bin |= BYTE_OPERATION;
	  check = true;
	  break;

	case 'w':
	  /* Word operation - this is the default.  */
	  check = true;
	  break;

	case 0:
	case ' ':
	case '\n':
	case '\r':
	  as_warn (_("no size modifier after period, .w assumed"));
	  break;

	default:
	  as_bad (_("unrecognised instruction size modifier .%c"),
		   * line);
	  return 0;
	}

      if (check)
	{
	  ++ line;

	}
    }

  if (*line && ! ISSPACE (*line))
    {
      as_bad (_("junk found after instruction: %s.%s"),
	      opcode->name, line);
      return 0;
    }

  /* Catch the case where the programmer has used a ".a" size modifier on an
     instruction that does not support it.  Look for an alternative extended
     instruction that has the same name without the period.  Eg: "add.a"
     becomes "adda".  Although this not an officially supported way of
     specifying instruction aliases other MSP430 assemblers allow it.  So we
     support it for compatibility purposes.  */
  if (addr_op && opcode->fmt >= 0)
    {
      const char * old_name = opcode->name;
      char real_name[32];

      sprintf (real_name, "%sa", old_name);
      opcode = str_hash_find (msp430_hash, real_name);
      if (opcode == NULL)
	{
	  as_bad (_("instruction %s.a does not exist"), old_name);
	  return 0;
	}
#if 0 /* Enable for debugging.  */
      as_warn ("treating %s.a as %s", old_name, real_name);
#endif
      addr_op = false;
      bin = opcode->bin_opcode;
    }

  if (opcode->fmt != -1
      && opcode->insn_opnumb
      && (!*line || *line == '\n'))
    {
      as_bad (ngettext ("instruction %s requires %d operand",
			"instruction %s requires %d operands",
			opcode->insn_opnumb),
	      opcode->name, opcode->insn_opnumb);
      return 0;
    }

  memset (l1, 0, sizeof (l1));
  memset (l2, 0, sizeof (l2));
  memset (&op1, 0, sizeof (op1));
  memset (&op2, 0, sizeof (op2));

  imm_op = false;

  if ((fmt = opcode->fmt) < 0)
    {
      if (! target_is_430x ())
	{
	  as_bad (_("instruction %s requires MSP430X mcu"),
		  opcode->name);
	  return 0;
	}

      fmt = (-fmt) - 1;
      extended_op = true;
    }

  if (repeat_count)
    {
      /* If requested set the extended instruction repeat count.  */
      if (extended_op)
	{
	  if (repeat_count > 0)
	    extended |= (repeat_count - 1);
	  else
	    extended |= (1 << 7) | (- repeat_count);
	}
      else
	as_bad (_("unable to repeat %s insn"), opcode->name);

      repeat_count = 0;
    }

  /* The previous instruction set this flag if it wants to check if this insn
     is a NOP.  */
  if (check_for_nop)
    {
      if (! is_opcode ("nop"))
	{
	  do
	    {
	      switch (check_for_nop & - check_for_nop)
		{
		case NOP_CHECK_INTERRUPT:
		  /* NOP_CHECK_INTERRUPT rules:
		     1.  430 and 430x ISA require a NOP after DINT.
		     2.  Only the 430x ISA requires NOP before EINT (this has
			been dealt with in the previous call to this function).
		     3.  Only the 430x ISA requires NOP after every EINT.
			CPU42 errata.  */
		  if (gen_interrupt_nops || warn_interrupt_nops)
		    {
		      if (prev_insn_is_dint)
			{
			  if (gen_interrupt_nops)
			    {
			      gen_nop ();
			      if (warn_interrupt_nops)
				as_warn (_(INSERT_NOP_AFTER_DINT));
			    }
			  else
			    as_warn (_(WARN_NOP_AFTER_DINT));
			}
		      else if (prev_insn_is_eint)
			{
			  if (gen_interrupt_nops)
			    {
			      gen_nop ();
			      if (warn_interrupt_nops)
				as_warn (_(INSERT_NOP_AFTER_EINT));
			    }
			  else
			    as_warn (_(WARN_NOP_AFTER_EINT));
			}
		      /* If we get here it's because the last instruction was
			 determined to either disable or enable interrupts, but
			 we're not sure which.
			 We have no information yet about what effect the
			 current instruction has on interrupts, that has to be
			 sorted out later.
			 The last insn may have required a NOP after it, so we
			 deal with that now.  */
		      else
			{
			  if (gen_interrupt_nops)
			    {
			      gen_nop ();
			      if (warn_interrupt_nops)
				as_warn (_(INSERT_NOP_AFTER_UNKNOWN));
			    }
			  else
			    /* warn_unsure_interrupt was called on the previous
			       insn.  */
			    as_warn (_(WARN_NOP_AFTER_UNKNOWN));
			}
		    }
		  break;

		case NOP_CHECK_CPU12:
		  if (silicon_errata_warn & SILICON_ERRATA_CPU12)
		    as_warn (_("CPU12: CMP/BIT with PC destination ignores next instruction"));

		  if (silicon_errata_fix & SILICON_ERRATA_CPU12)
		    gen_nop ();
		  break;

		case NOP_CHECK_CPU19:
		  if (silicon_errata_warn & SILICON_ERRATA_CPU19)
		    as_warn (_("CPU19: Instruction setting CPUOFF must be followed by a NOP"));

		  if (silicon_errata_fix & SILICON_ERRATA_CPU19)
		    gen_nop ();
		  break;
		  
		default:
		  as_bad (_("internal error: unknown nop check state"));
		  break;
		}
	      check_for_nop &= ~ (check_for_nop & - check_for_nop);
	    }
	  while (check_for_nop);
	}
      check_for_nop = 0;
    }

  switch (fmt)
    {
    case 0:
      /* Emulated.  */
      switch (opcode->insn_opnumb)
	{
	case 0:
	  if (is_opcode ("eint"))
	    warn_eint_nop (prev_insn_is_nop, prev_insn_is_dint);

	  /* Set/clear bits instructions.  */
	  if (extended_op)
	    {
	      if (!addr_op)
		extended |= BYTE_OPERATION;

	      /* Emit the extension word.  */
	      insn_length += 2;
	      frag = frag_more (2);
	      bfd_putl16 (extended, frag);
	    }

	  insn_length += 2;
	  frag = frag_more (2);
	  bfd_putl16 ((bfd_vma) bin, frag);
	  dwarf2_emit_insn (insn_length);
	  break;

	case 1:
	  /* Something which works with destination operand.  */
	  line = extract_operand (line, l1, sizeof (l1));
	  res = msp430_dstoperand (&op1, l1, opcode->bin_opcode, extended_op, true);
	  if (res)
	    break;

	  bin |= (op1.reg | (op1.am << 7));

	  /* If the PC is the destination...  */
	  if (op1.am == 0 && op1.reg == 0
	      /* ... and the opcode alters the SR.  */
	      && !(is_opcode ("bic") || is_opcode ("bis") || is_opcode ("mov")
		   || is_opcode ("bicx") || is_opcode ("bisx") || is_opcode ("movx")))
	    {
	      if (silicon_errata_fix & SILICON_ERRATA_CPU11)
		as_bad (_("CPU11: PC is destination of SR altering instruction"));
	      else if (silicon_errata_warn & SILICON_ERRATA_CPU11)
		as_warn (_("CPU11: PC is destination of SR altering instruction"));
	    }
	  
	  /* If the status register is the destination...  */
	  if (op1.am == 0 && op1.reg == 2
	      /* ... and the opcode alters the SR.  */
	      && (is_opcode ("adc") || is_opcode ("dec") || is_opcode ("decd")
		  || is_opcode ("inc") || is_opcode ("incd") || is_opcode ("inv")
		  || is_opcode ("sbc") || is_opcode ("sxt")
		  || is_opcode ("adcx") || is_opcode ("decx") || is_opcode ("decdx")
		  || is_opcode ("incx") || is_opcode ("incdx") || is_opcode ("invx")
		  || is_opcode ("sbcx")
		  ))
	    {
	      if (silicon_errata_fix & SILICON_ERRATA_CPU13)
		as_bad (_("CPU13: SR is destination of SR altering instruction"));
	      else if (silicon_errata_warn & SILICON_ERRATA_CPU13)
		as_warn (_("CPU13: SR is destination of SR altering instruction"));
	    }
	  
	  /* Compute the entire instruction length, in bytes.  */
	  op_length = (extended_op ? 2 : 0) + 2 + (op1.ol * 2);
	  insn_length += op_length;
	  frag = frag_more (op_length);
	  where = frag - frag_now->fr_literal;

	  if (extended_op)
	    {
	      if (!addr_op)
		extended |= BYTE_OPERATION;

	      if (op1.ol != 0 && ((extended & 0xf) != 0))
		{
		  as_bad (_("repeat instruction used with non-register mode instruction"));
		  extended &= ~ 0xf;
		}

	      if (op1.mode == OP_EXP)
		{
		  if (op1.exp.X_op == O_constant)
		    extended |= ((op1.exp.X_add_number >> 16) & 0xf) << 7;

		  else if (op1.reg || op1.am == 3)	/* Not PC relative.  */
		    fix_new_exp (frag_now, where, 6, &(op1.exp), false,
				 BFD_RELOC_MSP430X_ABS20_EXT_SRC);
		  else
		    fix_new_exp (frag_now, where, 6, &(op1.exp), false,
				 BFD_RELOC_MSP430X_PCR20_EXT_SRC);
		}

	      /* Emit the extension word.  */
	      bfd_putl16 (extended, frag);
	      frag += 2;
	      where += 2;
	    }

	  bfd_putl16 ((bfd_vma) bin, frag);
	  frag += 2;
	  where += 2;

	  if (op1.mode == OP_EXP)
	    {
	      if (op1.exp.X_op == O_constant)
		{
		  bfd_putl16 (op1.exp.X_add_number & 0xffff, frag);
		}
	      else
		{
		  bfd_putl16 ((bfd_vma) ZEROS, frag);

		  if (!extended_op)
		    {
		      if (op1.reg)
			fix_new_exp (frag_now, where, 2,
				     &(op1.exp), false, CHECK_RELOC_MSP430 (op1));
		      else
			fix_new_exp (frag_now, where, 2,
				     &(op1.exp), true, CHECK_RELOC_MSP430_PCREL);
		    }
		}
	    }

	  dwarf2_emit_insn (insn_length);
	  break;

	case 2:
	  /* Shift instruction.  */
	  line = extract_operand (line, l1, sizeof (l1));
	  strncpy (l2, l1, sizeof (l2));
	  l2[sizeof (l2) - 1] = '\0';
	  res = msp430_srcoperand (&op1, l1, opcode->bin_opcode, &imm_op, extended_op, true);
	  res += msp430_dstoperand (&op2, l2, opcode->bin_opcode, extended_op, true);

	  if (res)
	    break;	/* An error occurred.  All warnings were done before.  */

	  insn_length = (extended_op ? 2 : 0) + 2 + (op1.ol * 2) + (op2.ol * 2);
	  frag = frag_more (insn_length);
	  where = frag - frag_now->fr_literal;

	  if (target_is_430xv2 ()
	      && op1.mode == OP_REG
	      && op1.reg == 0
	      && (is_opcode ("rlax")
		  || is_opcode ("rlcx")
		  || is_opcode ("rla")
		  || is_opcode ("rlc")))
	    {
	      as_bad (_("%s: attempt to rotate the PC register"), opcode->name);
	      break;
	    }

	  /* If the status register is the destination...  */
	  if (op1.am == 0 && op1.reg == 2
	      /* ... and the opcode alters the SR.  */
	      && (is_opcode ("rla") || is_opcode ("rlc")
		  || is_opcode ("rlax") || is_opcode ("rlcx")
		  || is_opcode ("sxt") || is_opcode ("sxtx")
		  || is_opcode ("swpb")
		  ))
	    {
	      if (silicon_errata_fix & SILICON_ERRATA_CPU13)
		as_bad (_("CPU13: SR is destination of SR altering instruction"));
	      else if (silicon_errata_warn & SILICON_ERRATA_CPU13)
		as_warn (_("CPU13: SR is destination of SR altering instruction"));
	    }
	  
	  if (extended_op)
	    {
	      if (!addr_op)
		extended |= BYTE_OPERATION;

	      if ((op1.ol != 0 || op2.ol != 0) && ((extended & 0xf) != 0))
		{
		  as_bad (_("repeat instruction used with non-register mode instruction"));
		  extended &= ~ 0xf;
		}

	      if (op1.mode == OP_EXP)
		{
		  if (op1.exp.X_op == O_constant)
		    extended |= ((op1.exp.X_add_number >> 16) & 0xf) << 7;

		  else if (op1.reg || op1.am == 3)	/* Not PC relative.  */
		    fix_new_exp (frag_now, where, 6, &(op1.exp), false,
				 BFD_RELOC_MSP430X_ABS20_EXT_SRC);
		  else
		    fix_new_exp (frag_now, where, 6, &(op1.exp), false,
				 BFD_RELOC_MSP430X_PCR20_EXT_SRC);
		}

	      if (op2.mode == OP_EXP)
		{
		  if (op2.exp.X_op == O_constant)
		    extended |= (op2.exp.X_add_number >> 16) & 0xf;

		  else if (op1.mode == OP_EXP)
		    fix_new_exp (frag_now, where, 8, &(op2.exp), false,
				 op2.reg ? BFD_RELOC_MSP430X_ABS20_EXT_ODST
				 : BFD_RELOC_MSP430X_PCR20_EXT_ODST);
		  else
		    fix_new_exp (frag_now, where, 6, &(op2.exp), false,
				 op2.reg ? BFD_RELOC_MSP430X_ABS20_EXT_DST
				 : BFD_RELOC_MSP430X_PCR20_EXT_DST);
		}

	      /* Emit the extension word.  */
	      bfd_putl16 (extended, frag);
	      frag += 2;
	      where += 2;
	    }

	  bin |= (op2.reg | (op1.reg << 8) | (op1.am << 4) | (op2.am << 7));
	  bfd_putl16 ((bfd_vma) bin, frag);
	  frag += 2;
	  where += 2;

	  if (op1.mode == OP_EXP)
	    {
	      if (op1.exp.X_op == O_constant)
		{
		  bfd_putl16 (op1.exp.X_add_number & 0xffff, frag);
		}
	      else
		{
		  bfd_putl16 ((bfd_vma) ZEROS, frag);

		  if (!extended_op)
		    {
		      if (op1.reg || op1.am == 3)	/* Not PC relative.  */
			fix_new_exp (frag_now, where, 2,
				     &(op1.exp), false, CHECK_RELOC_MSP430 (op1));
		      else
			fix_new_exp (frag_now, where, 2,
				     &(op1.exp), true, CHECK_RELOC_MSP430_PCREL);
		    }
		}
	      frag += 2;
	      where += 2;
	    }

	  if (op2.mode == OP_EXP)
	    {
	      if (op2.exp.X_op == O_constant)
		{
		  bfd_putl16 (op2.exp.X_add_number & 0xffff, frag);
		}
	      else
		{
		  bfd_putl16 ((bfd_vma) ZEROS, frag);

		  if (!extended_op)
		    {
		      if (op2.reg)	/* Not PC relative.  */
			fix_new_exp (frag_now, where, 2,
				     &(op2.exp), false, CHECK_RELOC_MSP430 (op2));
		      else
			fix_new_exp (frag_now, where, 2,
				     &(op2.exp), true, CHECK_RELOC_MSP430_PCREL);
		    }
		}
	    }

	  dwarf2_emit_insn (insn_length);
	  break;

	case 3:
	  /* Branch instruction => mov dst, r0.  */
	  if (extended_op)
	    {
	      as_bad ("Internal error: state 0/3 not coded for extended instructions");
	      break;
	    }

	  line = extract_operand (line, l1, sizeof (l1));
	  res = msp430_srcoperand (&op1, l1, opcode->bin_opcode, &imm_op, extended_op, false);
	  if (res)
	    break;

	  byte_op = false;
	  imm_op = false;
	  bin |= ((op1.reg << 8) | (op1.am << 4));
	  op_length = 2 + 2 * op1.ol;
	  frag = frag_more (op_length);
	  where = frag - frag_now->fr_literal;
	  bfd_putl16 ((bfd_vma) bin, frag);

	  if (op1.mode == OP_EXP)
	    {
	      if (op1.exp.X_op == O_constant)
		{
		  bfd_putl16 (op1.exp.X_add_number & 0xffff, frag + 2);
		}
	      else
		{
		  where += 2;

		  bfd_putl16 ((bfd_vma) ZEROS, frag + 2);

		  if (op1.reg || op1.am == 3)
		    fix_new_exp (frag_now, where, 2,
				 &(op1.exp), false, CHECK_RELOC_MSP430 (op1));
		  else
		    fix_new_exp (frag_now, where, 2,
				 &(op1.exp), true, CHECK_RELOC_MSP430_PCREL);
		}
	    }

	  dwarf2_emit_insn (insn_length + op_length);
	  break;

	case 4:
	  /* CALLA instructions.  */
	  fix_emitted = false;

	  line = extract_operand (line, l1, sizeof (l1));
	  imm_op = false;

	  res = msp430_srcoperand (&op1, l1, opcode->bin_opcode, &imm_op,
				   extended_op, false);
	  if (res)
	    break;

	  byte_op = false;

	  op_length = 2 + 2 * op1.ol;
	  frag = frag_more (op_length);
	  where = frag - frag_now->fr_literal;

	  if (imm_op)
	    {
	      if (op1.am == 3)
		{
		  bin |= 0xb0;

		  fix_new_exp (frag_now, where, 4, &(op1.exp), false,
			       BFD_RELOC_MSP430X_ABS20_ADR_DST);
		  fix_emitted = true;
		}
	      else if (op1.am == 1)
		{
		  if (op1.reg == 0)
		    {
		      bin |=  0x90;

		      fix_new_exp (frag_now, where, 4, &(op1.exp), false,
				   BFD_RELOC_MSP430X_PCR20_CALL);
		      fix_emitted = true;
		    }
		  else
		    bin |=  0x50 | op1.reg;
		}
	      else if (op1.am == 0)
		bin |= 0x40 | op1.reg;
	    }
	  else if (op1.am == 1)
	    {
	      bin |= 0x80;

	      fix_new_exp (frag_now, where, 4, &(op1.exp), false,
			   BFD_RELOC_MSP430X_ABS20_ADR_DST);
	      fix_emitted = true;
	    }
	  else if (op1.am == 2)
	    bin |= 0x60 | op1.reg;
	  else if (op1.am == 3)
	    bin |= 0x70 | op1.reg;

	  bfd_putl16 ((bfd_vma) bin, frag);

	  if (op1.mode == OP_EXP)
	    {
	      if (op1.ol != 1)
		{
		  as_bad ("Internal error: unexpected CALLA instruction length: %d\n", op1.ol);
		  break;
		}

	      bfd_putl16 ((bfd_vma) ZEROS, frag + 2);

	      if (! fix_emitted)
		fix_new_exp (frag_now, where + 2, 2,
			     &(op1.exp), false, BFD_RELOC_16);
	    }

	  dwarf2_emit_insn (insn_length + op_length);
	  break;

	case 5:
	  {
	    int n;
	    int reg;

	    /* [POP|PUSH]M[.A] #N, Rd */
	    line = extract_operand (line, l1, sizeof (l1));
	    line = extract_operand (line, l2, sizeof (l2));

	    if (*l1 != '#')
	      {
		as_bad (_("expected #n as first argument of %s"), opcode->name);
		break;
	      }
	    end = parse_exp (l1 + 1, &(op1.exp));
	    if (end != NULL && *end != 0)
	      {
		as_bad (_("extra characters '%s' at end of constant expression '%s'"), end, l1);
		break;
	      }
	    if (op1.exp.X_op != O_constant)
	      {
		as_bad (_("expected constant expression as first argument of %s"),
			opcode->name);
		break;
	      }

	    if ((reg = check_reg (l2)) == -1)
	      {
		as_bad (_("expected register as second argument of %s"),
			opcode->name);
		break;
	      }

	    op_length = 2;
	    frag = frag_more (op_length);
	    where = frag - frag_now->fr_literal;
	    bin = opcode->bin_opcode;
	    if (! addr_op)
	      bin |= 0x100;
	    n = op1.exp.X_add_number;
	    bin |= (n - 1) << 4;
	    if (is_opcode ("pushm"))
	      bin |= reg;
	    else
	      {
		if (reg - n + 1 < 0)
		  {
		    as_bad (_("Too many registers popped"));
		    break;
		  }

		/* CPU21 errata: cannot use POPM to restore the SR register.  */
		if (target_is_430xv2 ()
		    && (reg - n + 1 < 3)
		    && reg >= 2
		    && is_opcode ("popm"))
		  {
		    as_bad (_("Cannot use POPM to restore the SR register"));
		    break;
		  }

		bin |= (reg - n + 1);
	      }

	    bfd_putl16 ((bfd_vma) bin, frag);
	    dwarf2_emit_insn (op_length);
	    break;
	  }

	case 6:
	  {
	    int n;
	    int reg;

	    /* Bit rotation instructions. RRCM, RRAM, RRUM, RLAM.  */
	    if (extended & 0xff)
	      {
		as_bad (_("repeat count cannot be used with %s"), opcode->name);
		break;
	      }

	    line = extract_operand (line, l1, sizeof (l1));
	    line = extract_operand (line, l2, sizeof (l2));

	    if (*l1 != '#')
	      {
		as_bad (_("expected #n as first argument of %s"), opcode->name);
		break;
	      }
	    end = parse_exp (l1 + 1, &(op1.exp));
	    if (end != NULL && *end != 0)
	      {
		as_bad (_("extra characters '%s' at end of operand '%s'"), end, l1);
		break;
	      }
	    if (op1.exp.X_op != O_constant)
	      {
		as_bad (_("expected constant expression as first argument of %s"),
			opcode->name);
		break;
	      }
	    n = op1.exp.X_add_number;
	    if (n > 4 || n < 1)
	      {
		as_bad (_("expected first argument of %s to be in the range 1-4"),
			opcode->name);
		break;
	      }

	    if ((reg = check_reg (l2)) == -1)
	      {
		as_bad (_("expected register as second argument of %s"),
			opcode->name);
		break;
	      }

	    if (target_is_430xv2 () && reg == 0)
	      {
		as_bad (_("%s: attempt to rotate the PC register"), opcode->name);
		break;
	      }

	    op_length = 2;
	    frag = frag_more (op_length);
	    where = frag - frag_now->fr_literal;

	    bin = opcode->bin_opcode;
	    if (! addr_op)
	      bin |= 0x10;
	    bin |= (n - 1) << 10;
	    bin |= reg;

	    bfd_putl16 ((bfd_vma) bin, frag);
	    dwarf2_emit_insn (op_length);
	    break;
	  }

	case 8:
	  {
	    bool need_reloc = false;
	    int n;
	    int reg;

	    /* ADDA, CMPA and SUBA address instructions.  */
	    if (extended & 0xff)
	      {
		as_bad (_("repeat count cannot be used with %s"), opcode->name);
		break;
	      }

	    line = extract_operand (line, l1, sizeof (l1));
	    line = extract_operand (line, l2, sizeof (l2));

	    bin = opcode->bin_opcode;

	    if (*l1 == '#')
	      {
		end = parse_exp (l1 + 1, &(op1.exp));
		if (end != NULL && *end != 0)
		  {
		    as_bad (_("extra characters '%s' at end of operand '%s'"), end, l1);
		    break;
		  }

		if (op1.exp.X_op == O_constant)
		  {
		    n = op1.exp.X_add_number;
		    if (n > 0xfffff || n < - (0x7ffff))
		      {
			as_bad (_("expected value of first argument of %s to fit into 20-bits"),
				opcode->name);
			break;
		      }

		    bin |= ((n >> 16) & 0xf) << 8;
		  }
		else
		  {
		    n = 0;
		    need_reloc = true;
		  }

		op_length = 4;
	      }
	    else
	      {
		if ((n = check_reg (l1)) == -1)
		  {
		    as_bad (_("expected register name or constant as first argument of %s"),
			    opcode->name);
		    break;
		  }

		bin |= (n << 8) | (1 << 6);
		op_length = 2;
	      }

	    if ((reg = check_reg (l2)) == -1)
	      {
		as_bad (_("expected register as second argument of %s"),
			opcode->name);
		break;
	      }

	    frag = frag_more (op_length);
	    where = frag - frag_now->fr_literal;
	    bin |= reg;
	    if (need_reloc)
	      fix_new_exp (frag_now, where, 4, &(op1.exp), false,
			   BFD_RELOC_MSP430X_ABS20_ADR_SRC);

	    bfd_putl16 ((bfd_vma) bin, frag);
	    if (op_length == 4)
	      bfd_putl16 ((bfd_vma) (n & 0xffff), frag + 2);
	    dwarf2_emit_insn (op_length);
	    break;
	  }

	case 9: /* MOVA, BRA, RETA.  */
	  imm_op = false;
	  bin = opcode->bin_opcode;

	  if (is_opcode ("reta"))
	    {
	      /* The RETA instruction does not take any arguments.
		 The implicit first argument is @SP+.
		 The implicit second argument is PC.  */
	      op1.mode = OP_REG;
	      op1.am = 3;
	      op1.reg = 1;

	      op2.mode = OP_REG;
	      op2.reg = 0;
	    }
	  else
	    {
	      line = extract_operand (line, l1, sizeof (l1));
	      res = msp430_srcoperand (&op1, l1, opcode->bin_opcode,
				       &imm_op, extended_op, false);

	      if (is_opcode ("bra"))
		{
		  /* This is the BRA synthetic instruction.
		     The second argument is always PC.  */
		  op2.mode = OP_REG;
		  op2.reg = 0;
		}
	      else
		{
		  line = extract_operand (line, l2, sizeof (l2));
		  res += msp430_dstoperand (&op2, l2, opcode->bin_opcode,
					    extended_op, true);
		}

	      if (res)
		break;	/* Error occurred.  All warnings were done before.  */
	    }

	  /* Only a restricted subset of the normal MSP430 addressing modes
	     are supported here, so check for the ones that are allowed.  */
	  if ((op_length = try_encode_mova (imm_op, bin, & op1, & op2,
					    & error_message)) == 0)
	    {
	      as_bad (error_message, opcode->name);
	      break;
	    }
	  dwarf2_emit_insn (op_length);
	  break;

	case 10: /* RPT */
	  line = extract_operand (line, l1, sizeof l1);
	  /* The RPT instruction only accepted immediates and registers.  */
	  if (*l1 == '#')
	    {
	      end = parse_exp (l1 + 1, &(op1.exp));
	      if (end != NULL && *end != 0)
		{
		  as_bad (_("extra characters '%s' at end of operand '%s'"), end, l1);
		  break;
		}
	      if (op1.exp.X_op != O_constant)
		{
		  as_bad (_("expected constant value as argument to RPT"));
		  break;
		}
	      if (op1.exp.X_add_number < 1
		  || op1.exp.X_add_number > (1 << 4))
		{
		  as_bad (_("expected constant in the range 2..16"));
		  break;
		}

	      /* We silently accept and ignore a repeat count of 1.  */
	      if (op1.exp.X_add_number > 1)
		repeat_count = op1.exp.X_add_number;
	    }
	  else
	    {
	      int reg;

	      if ((reg = check_reg (l1)) != -1)
		{
		  if (reg == 0)
		    as_warn (_("PC used as an argument to RPT"));
		  else
		    repeat_count = - reg;
		}
	      else
		{
		  as_bad (_("expected constant or register name as argument to RPT insn"));
		  break;
		}
	    }
	  break;

	default:
	  as_bad (_("Illegal emulated instruction"));
	  break;
	}
      break;

      /* FIXME: Emit warning when dest reg SR(R2) is addressed with .B or .A.
	 From f5 ref man 6.3.3:
	   The 16-bit Status Register (SR, also called R2), used as a source or
	   destination register, can only be used in register mode addressed
	   with word instructions.  */

    case 1:			/* Format 1, double operand.  */
      line = extract_operand (line, l1, sizeof (l1));
      line = extract_operand (line, l2, sizeof (l2));
      res = msp430_srcoperand (&op1, l1, opcode->bin_opcode, &imm_op, extended_op, true);
      res += msp430_dstoperand (&op2, l2, opcode->bin_opcode, extended_op, true);

      if (res)
	break;			/* Error occurred.  All warnings were done before.  */

      if (extended_op
	  && is_opcode ("movx")
	  && addr_op
	  && msp430_enable_relax)
	{
	  /* This is the MOVX.A instruction.  See if we can convert
	     it into the MOVA instruction instead.  This saves 2 bytes.  */
	  if ((op_length = try_encode_mova (imm_op, 0x0000, & op1, & op2,
					    NULL)) != 0)
	    {
	      dwarf2_emit_insn (op_length);
	      break;
	    }
	}

      bin |= (op2.reg | (op1.reg << 8) | (op1.am << 4) | (op2.am << 7));

      /* If the PC is the destination...  */
      if (op2.am == 0 && op2.reg == 0
	  /* ... and the opcode alters the SR.  */
	  && !(is_opcode ("bic") || is_opcode ("bis") || is_opcode ("mov")
	       || is_opcode ("bicx") || is_opcode ("bisx") || is_opcode ("movx")))
	{
	  if (silicon_errata_fix & SILICON_ERRATA_CPU11)
	    as_bad (_("CPU11: PC is destination of SR altering instruction"));
	  else if (silicon_errata_warn & SILICON_ERRATA_CPU11)
	    as_warn (_("CPU11: PC is destination of SR altering instruction"));
	}
	  
      /* If the status register is the destination...  */
      if (op2.am == 0 && op2.reg == 2
	  /* ... and the opcode alters the SR.  */
	  && (is_opcode ("add") || is_opcode ("addc") || is_opcode ("and")
	      || is_opcode ("dadd") || is_opcode ("sub") || is_opcode ("subc")
	      || is_opcode ("xor")
	      || is_opcode ("addx") || is_opcode ("addcx") || is_opcode ("andx")
	      || is_opcode ("daddx") || is_opcode ("subx") || is_opcode ("subcx")
	      || is_opcode ("xorx")
	      ))
	{
	  if (silicon_errata_fix & SILICON_ERRATA_CPU13)
	    as_bad (_("CPU13: SR is destination of SR altering instruction"));
	  else if (silicon_errata_warn & SILICON_ERRATA_CPU13)
	    as_warn (_("CPU13: SR is destination of SR altering instruction"));
	}

      /* Chain these checks for SR manipulations so we can warn if they are not
	 caught.  */
      if (((is_opcode ("bis") && bin == 0xd032)
	   || (is_opcode ("mov") && bin == 0x4032)
	   || (is_opcode ("xor") && bin == 0xe032))
	  && op1.mode == OP_EXP
	  && op1.exp.X_op == O_constant
	  && (op1.exp.X_add_number & 0x10) == 0x10)
	check_for_nop |= NOP_CHECK_CPU19;
      else if ((is_opcode ("mov") && op2.mode == OP_REG && op2.reg == 2))
	{
	  /* Any MOV with the SR as the destination either enables or disables
	     interrupts.  */
	  if (op1.mode == OP_EXP
	      && op1.exp.X_op == O_constant)
	    {
	      if ((op1.exp.X_add_number & 0x8) == 0x8)
		{
		  /* The GIE bit is being set.  */
		  warn_eint_nop (prev_insn_is_nop, prev_insn_is_dint);
		  this_insn_is_eint = true;
		}
	      else
		/* The GIE bit is being cleared.  */
		this_insn_is_dint = true;
	    }
	  /* If an immediate value which is covered by the constant generator
	     is the src, then op1 will have been changed to either R2 or R3 by
	     this point.
	     The only constants covered by CG1 and CG2, which have bit 3 set
	     and therefore would enable interrupts when writing to the SR, are
	     R2 with addresing mode 0b11 and R3 with 0b11.
	     The addressing mode is in bits 5:4 of the binary opcode.  */
	  else if (op1.mode == OP_REG
		   && (op1.reg == 2 || op1.reg == 3)
		   && (bin & 0x30) == 0x30)
	    {
	      warn_eint_nop (prev_insn_is_nop, prev_insn_is_dint);
	      this_insn_is_eint = true;
	    }
	  /* Any other use of the constant generator with destination R2, will
	     disable interrupts.  */
	  else if (op1.mode == OP_REG
		   && (op1.reg == 2 || op1.reg == 3))
	    this_insn_is_dint = true;
	  else if (do_unknown_interrupt_nops)
	    {
	      /* FIXME: Couldn't work out whether the insn is enabling or
		 disabling interrupts, so for safety need to treat it as both
		 a DINT and EINT.  */
	      warn_unsure_interrupt (prev_insn_is_nop, prev_insn_is_dint);
	      check_for_nop |= NOP_CHECK_INTERRUPT;
	    }
	}
      else if (is_eint (opcode->name, bin))
	warn_eint_nop (prev_insn_is_nop, prev_insn_is_dint);
      else if ((bin & 0x32) == 0x32)
	{
	  /* Double-operand insn with the As==0b11 and Rdst==0x2 will result in
	   * an interrupt state change if a write happens.  */
	  /* FIXME: How strict to be here? */
	  ;
	}

      /* Compute the entire length of the instruction in bytes.  */
      op_length = (extended_op ? 2 : 0)	/* The extension word.  */
	+ 2 			/* The opcode */
	+ (2 * op1.ol)		/* The first operand. */
	+ (2 * op2.ol);		/* The second operand.  */

      insn_length += op_length;
      frag = frag_more (op_length);
      where = frag - frag_now->fr_literal;

      if (extended_op)
	{
	  if (!addr_op)
	    extended |= BYTE_OPERATION;

	  if ((op1.ol != 0 || op2.ol != 0) && ((extended & 0xf) != 0))
	    {
	      as_bad (_("repeat instruction used with non-register mode instruction"));
	      extended &= ~ 0xf;
	    }

	  /* If necessary, emit a reloc to update the extension word.  */
	  if (op1.mode == OP_EXP)
	    {
	      if (op1.exp.X_op == O_constant)
		extended |= ((op1.exp.X_add_number >> 16) & 0xf) << 7;

	      else  if (op1.reg || op1.am == 3)	/* Not PC relative.  */
		fix_new_exp (frag_now, where, 6, &(op1.exp), false,
			     BFD_RELOC_MSP430X_ABS20_EXT_SRC);
	      else
		fix_new_exp (frag_now, where, 6, &(op1.exp), false,
			     BFD_RELOC_MSP430X_PCR20_EXT_SRC);
	    }

	  if (op2.mode == OP_EXP)
	    {
	      if (op2.exp.X_op == O_constant)
		extended |= (op2.exp.X_add_number >> 16) & 0xf;

	      else if (op1.mode == OP_EXP)
		fix_new_exp (frag_now, where, 8, &(op2.exp), false,
			     op2.reg ? BFD_RELOC_MSP430X_ABS20_EXT_ODST
			     : BFD_RELOC_MSP430X_PCR20_EXT_ODST);

	      else
		fix_new_exp (frag_now, where, 6, &(op2.exp), false,
			     op2.reg ? BFD_RELOC_MSP430X_ABS20_EXT_DST
			     : BFD_RELOC_MSP430X_PCR20_EXT_DST);
	    }

	  /* Emit the extension word.  */
	  bfd_putl16 (extended, frag);
	  where += 2;
	  frag += 2;
	}

      bfd_putl16 ((bfd_vma) bin, frag);
      where += 2;
      frag += 2;

      if (op1.mode == OP_EXP)
	{
	  if (op1.exp.X_op == O_constant)
	    {
	      bfd_putl16 (op1.exp.X_add_number & 0xffff, frag);
	    }
	  else
	    {
	      bfd_putl16 ((bfd_vma) ZEROS, frag);

	      if (!extended_op)
		{
		  if (op1.reg || op1.am == 3)	/* Not PC relative.  */
		    fix_new_exp (frag_now, where, 2,
				 &(op1.exp), false, CHECK_RELOC_MSP430 (op1));
		  else
		    fix_new_exp (frag_now, where, 2,
				 &(op1.exp), true, CHECK_RELOC_MSP430_PCREL);
		}
	    }

	  where += 2;
	  frag += 2;
	}

      if (op2.mode == OP_EXP)
	{
	  if (op2.exp.X_op == O_constant)
	    {
	      bfd_putl16 (op2.exp.X_add_number & 0xffff, frag);
	    }
	  else
	    {
	      bfd_putl16 ((bfd_vma) ZEROS, frag);

	      if (!extended_op)
		{
		  if (op2.reg)		/* Not PC relative.  */
		    fix_new_exp (frag_now, where, 2,
				 &(op2.exp), false, CHECK_RELOC_MSP430 (op2));
		  else
		    fix_new_exp (frag_now, where, 2,
				 &(op2.exp), true, CHECK_RELOC_MSP430_PCREL);
		}
	    }
	}

      dwarf2_emit_insn (insn_length);

      /* If the PC is the destination...  */
      if (op2.am == 0 && op2.reg == 0
	  /* ... but the opcode does not alter the destination.  */
	  && (is_opcode ("cmp") || is_opcode ("bit") || is_opcode ("cmpx")))
	check_for_nop |= NOP_CHECK_CPU12;
      break;

    case 2:			/* Single-operand mostly instr.  */
      if (opcode->insn_opnumb == 0)
	{
	  /* reti instruction.  */
	  insn_length += 2;
	  frag = frag_more (2);
	  bfd_putl16 ((bfd_vma) bin, frag);
	  dwarf2_emit_insn (insn_length);
	  break;
	}

      line = extract_operand (line, l1, sizeof (l1));
      res = msp430_srcoperand (&op1, l1, opcode->bin_opcode,
			       &imm_op, extended_op, true);
      if (res)
	break;		/* Error in operand.  */

      if (target_is_430xv2 ()
	  && op1.mode == OP_REG
	  && op1.reg == 0
	  && (is_opcode ("rrax")
	      || is_opcode ("rrcx")
	      || is_opcode ("rra")
	      || is_opcode ("rrc")))
	{
	  as_bad (_("%s: attempt to rotate the PC register"), opcode->name);
	  break;
	}

      /* If the status register is the destination...  */
      if (op1.am == 0 && op1.reg == 2
	  /* ... and the opcode alters the SR.  */
	  && (is_opcode ("rra") || is_opcode ("rrc") || is_opcode ("sxt")))
	{
	  if (silicon_errata_fix & SILICON_ERRATA_CPU13)
	    as_bad (_("CPU13: SR is destination of SR altering instruction"));
	  else if (silicon_errata_warn & SILICON_ERRATA_CPU13)
	    as_warn (_("CPU13: SR is destination of SR altering instruction"));
	}
	  
      insn_length = (extended_op ? 2 : 0) + 2 + (op1.ol * 2);
      frag = frag_more (insn_length);
      where = frag - frag_now->fr_literal;

      if (extended_op)
	{
	  if (is_opcode ("swpbx") || is_opcode ("sxtx"))
	    {
	      /* These two instructions use a special
		 encoding of the A/L and B/W bits.  */
	      bin &= ~ BYTE_OPERATION;

	      if (byte_op)
		{
		  as_bad (_("%s instruction does not accept a .b suffix"),
			  opcode->name);
		  break;
		}
	      else if (! addr_op)
		extended |= BYTE_OPERATION;
	    }
	  else if (! addr_op)
	    extended |= BYTE_OPERATION;

	  if (is_opcode ("rrux"))
	    extended |= IGNORE_CARRY_BIT;
	  
	  if (op1.ol != 0 && ((extended & 0xf) != 0))
	    {
	      as_bad (_("repeat instruction used with non-register mode instruction"));
	      extended &= ~ 0xf;
	    }

	  if (op1.mode == OP_EXP)
	    {
	      if (op1.exp.X_op == O_constant)
		extended |= ((op1.exp.X_add_number >> 16) & 0xf) << 7;

	      else if (op1.reg || op1.am == 3)	/* Not PC relative.  */
		fix_new_exp (frag_now, where, 6, &(op1.exp), false,
			     BFD_RELOC_MSP430X_ABS20_EXT_SRC);
	      else
		fix_new_exp (frag_now, where, 6, &(op1.exp), false,
			     BFD_RELOC_MSP430X_PCR20_EXT_SRC);
	    }

	  /* Emit the extension word.  */
	  bfd_putl16 (extended, frag);
	  frag += 2;
	  where += 2;
	}

      bin |= op1.reg | (op1.am << 4);
      bfd_putl16 ((bfd_vma) bin, frag);
      frag += 2;
      where += 2;

      if (op1.mode == OP_EXP)
	{
	  if (op1.exp.X_op == O_constant)
	    {
	      bfd_putl16 (op1.exp.X_add_number & 0xffff, frag);
	    }
	  else
	    {
	      bfd_putl16 ((bfd_vma) ZEROS, frag);

	      if (!extended_op)
		{
		  if (op1.reg || op1.am == 3)	/* Not PC relative.  */
		    fix_new_exp (frag_now, where, 2,
				 &(op1.exp), false, CHECK_RELOC_MSP430 (op1));
		  else
		    fix_new_exp (frag_now, where, 2,
				 &(op1.exp), true, CHECK_RELOC_MSP430_PCREL);
		}
	    }
	}

      dwarf2_emit_insn (insn_length);
      break;

    case 3:			/* Conditional jumps instructions.  */
      line = extract_operand (line, l1, sizeof (l1));
      /* l1 is a label.  */
      if (l1[0])
	{
	  char *m = l1;
	  expressionS exp;

	  if (*m == '$')
	    m++;

	  end = parse_exp (m, &exp);
	  if (end != NULL && *end != 0)
	    {
	      as_bad (_("extra characters '%s' at end of operand '%s'"), end, l1);
	      break;
	    }

	  /* In order to handle something like:

	     and #0x8000, r5
	     tst r5
	     jz   4     ;       skip next 4 bytes
	     inv r5
	     inc r5
	     nop        ;       will jump here if r5 positive or zero

	     jCOND      -n      ;assumes jump n bytes backward:

	     mov r5,r6
	     jmp -2

	     is equal to:
	     lab:
	     mov r5,r6
	     jmp lab

	     jCOND      $n      ; jump from PC in either direction.  */

	  if (exp.X_op == O_constant)
	    {
	      int x = exp.X_add_number;

	      if (x & 1)
		{
		  as_warn (_("Even number required. Rounded to %d"), x + 1);
		  x++;
		}

	      if ((*l1 == '$' && x > 0) || x < 0)
		x -= 2;

	      x >>= 1;

	      if (x > 512 || x < -511)
		{
		  as_bad (_("Wrong displacement %d"), x << 1);
		  break;
		}

	      insn_length += 2;
	      frag = frag_more (2);	/* Instr size is 1 word.  */

	      bin |= x & 0x3ff;
	      bfd_putl16 ((bfd_vma) bin, frag);
	    }
	  else if (exp.X_op == O_symbol && *l1 != '$')
	    {
	      insn_length += 2;
	      frag = frag_more (2);	/* Instr size is 1 word.  */
	      where = frag - frag_now->fr_literal;
	      fix_new_exp (frag_now, where, 2,
			   &exp, true, BFD_RELOC_MSP430_10_PCREL);

	      bfd_putl16 ((bfd_vma) bin, frag);
	    }
	  else if (*l1 == '$')
	    {
	      as_bad (_("instruction requires label sans '$'"));
	    }
	  else
	    as_bad (_
		    ("instruction requires label or value in range -511:512"));
	  dwarf2_emit_insn (insn_length);
	  break;
	}
      else
	{
	  as_bad (_("instruction requires label"));
	  break;
	}
      break;

    case 4:	/* Extended jumps.  */
      if (!msp430_enable_polys)
	{
	  as_bad (_("polymorphs are not enabled. Use -mP option to enable."));
	  break;
	}

      line = extract_operand (line, l1, sizeof (l1));
      if (l1[0])
	{
	  char *m = l1;
	  expressionS exp;

	  /* Ignore absolute addressing. make it PC relative anyway.  */
	  if (*m == '#' || *m == '$')
	    m++;

	  end = parse_exp (m, & exp);
	  if (end != NULL && *end != 0)
	    {
	      as_bad (_("extra characters '%s' at end of operand '%s'"), end, l1);
	      break;
	    }
	  if (exp.X_op == O_symbol)
	    {
	      /* Relaxation required.  */
	      struct rcodes_s rc = msp430_rcodes[opcode->insn_opnumb];

	      if (target_is_430x ())
		rc = msp430x_rcodes[opcode->insn_opnumb];

	      /* The parameter to dwarf2_emit_insn is actually the offset to
		 the start of the insn from the fix piece of instruction that
		 was emitted.  Since next fragments may have variable size we
		 tie debug info to the beginning of the instruction.  */
	      insn_length += 8;
	      frag = frag_more (8);
	      dwarf2_emit_insn (0);
	      bfd_putl16 ((bfd_vma) rc.sop, frag);
	      frag = frag_variant (rs_machine_dependent, 8, 2,
				    /* Wild guess.  */
				   ENCODE_RELAX (rc.lpos, STATE_BITS10),
				   exp.X_add_symbol,
				   0,	/* Offset is zero if jump dist less than 1K.  */
				   (char *) frag);
	      break;
	    }
	}

      as_bad (_("instruction requires label"));
      break;

    case 5:	/* Emulated extended branches.  */
      if (!msp430_enable_polys)
	{
	  as_bad (_("polymorphs are not enabled. Use -mP option to enable."));
	  break;
	}
      line = extract_operand (line, l1, sizeof (l1));
      if (l1[0])
	{
	  char * m = l1;
	  expressionS exp;

	  /* Ignore absolute addressing. make it PC relative anyway.  */
	  if (*m == '#' || *m == '$')
	    m++;

	  end = parse_exp (m, & exp);
	  if (end != NULL && *end != 0)
	    {
	      as_bad (_("extra characters '%s' at end of operand '%s'"), end, l1);
	      break;
	    }
	  if (exp.X_op == O_symbol)
	    {
	      /* Relaxation required.  */
	      struct hcodes_s hc = msp430_hcodes[opcode->insn_opnumb];

	      if (target_is_430x ())
		hc = msp430x_hcodes[opcode->insn_opnumb];

	      insn_length += 8;
	      frag = frag_more (8);
	      dwarf2_emit_insn (0);
	      bfd_putl16 ((bfd_vma) hc.op0, frag);
	      bfd_putl16 ((bfd_vma) hc.op1, frag+2);

	      frag = frag_variant (rs_machine_dependent, 8, 2,
				   ENCODE_RELAX (STATE_EMUL_BRANCH, STATE_BITS10), /* Wild guess.  */
				   exp.X_add_symbol,
				   0,	/* Offset is zero if jump dist less than 1K.  */
				   (char *) frag);
	      break;
	    }
	}

      as_bad (_("instruction requires label"));
      break;

    default:
      as_bad (_("Illegal instruction or not implemented opcode."));
    }

    if (is_opcode ("nop"))
      {
	prev_insn_is_nop = true;
	prev_insn_is_dint = false;
	prev_insn_is_eint = false;
      }
    else if (this_insn_is_dint || is_dint (opcode->name, bin))
      {
	prev_insn_is_dint = true;
	prev_insn_is_eint = false;
	prev_insn_is_nop = false;
	check_for_nop |= NOP_CHECK_INTERRUPT;
      }
    /* NOP is not needed after EINT for 430 ISA.  */
    else if (target_is_430x () && (this_insn_is_eint || is_eint (opcode->name, bin)))
      {
	prev_insn_is_eint = true;
	prev_insn_is_nop = false;
	prev_insn_is_dint = false;
	check_for_nop |= NOP_CHECK_INTERRUPT;
      }
    else
      {
	prev_insn_is_nop = false;
	prev_insn_is_dint = false;
	prev_insn_is_eint = false;
      }

  input_line_pointer = line;
  return 0;
}

void
md_assemble (char * str)
{
  struct msp430_opcode_s * opcode;
  char cmd[32];
  unsigned int i = 0;

  str = skip_space (str);	/* Skip leading spaces.  */
  str = extract_cmd (str, cmd, sizeof (cmd) - 1);

  while (cmd[i])
    {
      char a = TOLOWER (cmd[i]);
      cmd[i] = a;
      i++;
    }

  if (!cmd[0])
    {
      as_bad (_("can't find opcode"));
      return;
    }

  opcode = (struct msp430_opcode_s *) str_hash_find (msp430_hash, cmd);

  if (opcode == NULL)
    {
      as_bad (_("unknown opcode `%s'"), cmd);
      return;
    }

  {
    char *__t = input_line_pointer;

    msp430_operands (opcode, str);
    input_line_pointer = __t;
  }
}

/* GAS will call this function for each section at the end of the assembly,
   to permit the CPU backend to adjust the alignment of a section.  */

valueT
md_section_align (asection * seg, valueT addr)
{
  int align = bfd_section_alignment (seg);

  return ((addr + (1 << align) - 1) & -(1 << align));
}

/* If you define this macro, it should return the offset between the
   address of a PC relative fixup and the position from which the PC
   relative adjustment should be made.  On many processors, the base
   of a PC relative instruction is the next instruction, so this
   macro would return the length of an instruction.  */

long
md_pcrel_from_section (fixS * fixp, segT sec)
{
  if (fixp->fx_addsy != (symbolS *) NULL
      && (!S_IS_DEFINED (fixp->fx_addsy)
	  || (S_GET_SEGMENT (fixp->fx_addsy) != sec)))
    return 0;

  return fixp->fx_frag->fr_address + fixp->fx_where;
}

/* Addition to the standard TC_FORCE_RELOCATION_LOCAL.
   Now it handles the situation when relocations
   have to be passed to linker.  */
int
msp430_force_relocation_local (fixS *fixp)
{
  if (fixp->fx_r_type == BFD_RELOC_MSP430_10_PCREL)
    return 1;
  if (fixp->fx_pcrel)
    return 1;
  if (msp430_enable_polys
        && !msp430_enable_relax)
    return 1;

  return 0;
}


/* GAS will call this for each fixup.  It should store the correct
   value in the object file.  */
void
md_apply_fix (fixS * fixp, valueT * valuep, segT seg)
{
  unsigned char * where;
  unsigned long insn;
  long value;

  if (fixp->fx_addsy == (symbolS *) NULL)
    {
      value = *valuep;
      fixp->fx_done = 1;
    }
  else if (fixp->fx_pcrel)
    {
      segT s = S_GET_SEGMENT (fixp->fx_addsy);

      if (fixp->fx_addsy && (s == seg || s == absolute_section))
	{
	  /* FIXME: We can appear here only in case if we perform a pc
	     relative jump to the label which is i) global, ii) locally
	     defined or this is a jump to an absolute symbol.
	     If this is an absolute symbol -- everything is OK.
	     If this is a global label, we've got a symbol value defined
	     twice:
               1. S_GET_VALUE (fixp->fx_addsy) will contain a symbol offset
	          from this section start
               2. *valuep will contain the real offset from jump insn to the
	          label
	     So, the result of S_GET_VALUE (fixp->fx_addsy) + (* valuep);
	     will be incorrect. Therefore remove s_get_value.  */
	  value = /* S_GET_VALUE (fixp->fx_addsy) + */ * valuep;
	  fixp->fx_done = 1;
	}
      else
	value = *valuep;
    }
  else
    {
      value = fixp->fx_offset;

      if (fixp->fx_subsy != (symbolS *) NULL)
	{
	  if (S_GET_SEGMENT (fixp->fx_subsy) == absolute_section)
	    {
	      value -= S_GET_VALUE (fixp->fx_subsy);
	      fixp->fx_done = 1;
	    }
	}
    }

  fixp->fx_no_overflow = 1;

  /* If polymorphs are enabled and relax disabled.
     do not kill any relocs and pass them to linker.  */
  if (msp430_enable_polys
      && !msp430_enable_relax)
    {
      if (!fixp->fx_addsy
	  || S_GET_SEGMENT (fixp->fx_addsy) == absolute_section)
	fixp->fx_done = 1;	/* It is ok to kill 'abs' reloc.  */
      else
      	fixp->fx_done = 0;
    }

  if (fixp->fx_done)
    {
      /* Fetch the instruction, insert the fully resolved operand
	 value, and stuff the instruction back again.  */
      where = (unsigned char *) fixp->fx_frag->fr_literal + fixp->fx_where;

      insn = bfd_getl16 (where);

      switch (fixp->fx_r_type)
	{
	case BFD_RELOC_MSP430_10_PCREL:
	  if (value & 1)
	    as_bad_where (fixp->fx_file, fixp->fx_line,
			  _("odd address operand: %ld"), value);

	  /* Jumps are in words.  */
	  value >>= 1;
	  --value;		/* Correct PC.  */

	  if (value < -512 || value > 511)
	    as_bad_where (fixp->fx_file, fixp->fx_line,
			  _("operand out of range: %ld"), value);

	  value &= 0x3ff;	/* get rid of extended sign */
	  bfd_putl16 ((bfd_vma) (value | insn), where);
	  break;

	case BFD_RELOC_MSP430X_PCR16:
	case BFD_RELOC_MSP430_RL_PCREL:
	case BFD_RELOC_MSP430_16_PCREL:
	  if (value & 1)
	    as_bad_where (fixp->fx_file, fixp->fx_line,
			  _("odd address operand: %ld"), value);
	  /* Fall through.  */

	case BFD_RELOC_MSP430_16_PCREL_BYTE:
	  /* Nothing to be corrected here.  */
	  if (value < -32768 || value > 65536)
	    as_bad_where (fixp->fx_file, fixp->fx_line,
			  _("operand out of range: %ld"), value);
	  /* Fall through.  */

	case BFD_RELOC_MSP430X_ABS16:
	case BFD_RELOC_MSP430_16:
	case BFD_RELOC_16:
	case BFD_RELOC_MSP430_16_BYTE:
	  value &= 0xffff;	/* Get rid of extended sign.  */
	  bfd_putl16 ((bfd_vma) value, where);
	  break;

	case BFD_RELOC_MSP430_ABS_HI16:
	  value >>= 16;
	  value &= 0xffff;	/* Get rid of extended sign.  */
	  bfd_putl16 ((bfd_vma) value, where);
	  break;

	case BFD_RELOC_32:
	  bfd_putl16 ((bfd_vma) value, where);
	  break;

	case BFD_RELOC_MSP430_ABS8:
	case BFD_RELOC_8:
	  bfd_put_8 (NULL, (bfd_vma) value, where);
	  break;

	case BFD_RELOC_MSP430X_ABS20_EXT_SRC:
	case BFD_RELOC_MSP430X_PCR20_EXT_SRC:
	  bfd_putl16 ((bfd_vma) (value & 0xffff), where + 4);
	  value >>= 16;
	  bfd_putl16 ((bfd_vma) (((value & 0xf) << 7) | insn), where);
	  break;

	case BFD_RELOC_MSP430X_ABS20_ADR_SRC:
	  bfd_putl16 ((bfd_vma) (value & 0xffff), where + 2);
	  value >>= 16;
	  bfd_putl16 ((bfd_vma) (((value & 0xf) << 8) | insn), where);
	  break;

	case BFD_RELOC_MSP430X_ABS20_EXT_ODST:
	  bfd_putl16 ((bfd_vma) (value & 0xffff), where + 6);
	  value >>= 16;
	  bfd_putl16 ((bfd_vma) ((value & 0xf) | insn), where);
	  break;

	case BFD_RELOC_MSP430X_PCR20_CALL:
	  bfd_putl16 ((bfd_vma) (value & 0xffff), where + 2);
	  value >>= 16;
	  bfd_putl16 ((bfd_vma) ((value & 0xf) | insn), where);
	  break;

	case BFD_RELOC_MSP430X_ABS20_EXT_DST:
	case BFD_RELOC_MSP430X_PCR20_EXT_DST:
	  bfd_putl16 ((bfd_vma) (value & 0xffff), where + 4);
	  value >>= 16;
	  bfd_putl16 ((bfd_vma) ((value & 0xf) | insn), where);
	  break;

	case BFD_RELOC_MSP430X_PCR20_EXT_ODST:
	  bfd_putl16 ((bfd_vma) (value & 0xffff), where + 6);
	  value >>= 16;
	  bfd_putl16 ((bfd_vma) ((value & 0xf) | insn), where);
	  break;

	case BFD_RELOC_MSP430X_ABS20_ADR_DST:
	  bfd_putl16 ((bfd_vma) (value & 0xffff), where + 2);
	  value >>= 16;
	  bfd_putl16 ((bfd_vma) ((value & 0xf) | insn), where);
	  break;

	default:
	  as_fatal (_("line %d: unknown relocation type: 0x%x"),
		    fixp->fx_line, fixp->fx_r_type);
	  break;
	}
    }
  else
    {
      fixp->fx_addnumber = value;
    }
}

static bool
S_IS_GAS_LOCAL (symbolS * s)
{
  const char * name;
  unsigned int len;

  if (s == NULL)
    return false;
  name = S_GET_NAME (s);
  len = strlen (name) - 1;

  return name[len] == 1 || name[len] == 2;
}

/* GAS will call this to generate a reloc, passing the resulting reloc
   to `bfd_install_relocation'.  This currently works poorly, as
   `bfd_install_relocation' often does the wrong thing, and instances of
   `tc_gen_reloc' have been written to work around the problems, which
   in turns makes it difficult to fix `bfd_install_relocation'.  */

/* If while processing a fixup, a reloc really needs to be created
   then it is done here.  */

arelent **
tc_gen_reloc (asection * seg ATTRIBUTE_UNUSED, fixS * fixp)
{
  static arelent * no_relocs = NULL;
  static arelent * relocs[MAX_RELOC_EXPANSION + 1];
  arelent *reloc;

  reloc = XNEW (arelent);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);

  if (reloc->howto == (reloc_howto_type *) NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("reloc %d not supported by object file format"),
		    (int) fixp->fx_r_type);
      free (reloc);
      return & no_relocs;
    }

  relocs[0] = reloc;
  relocs[1] = NULL;

  if (fixp->fx_subsy
      && S_GET_SEGMENT (fixp->fx_subsy) == absolute_section)
    {
      fixp->fx_offset -= S_GET_VALUE (fixp->fx_subsy);
      fixp->fx_subsy = NULL;
    }

  if (fixp->fx_addsy && fixp->fx_subsy)
    {
      asection *asec, *ssec;

      asec = S_GET_SEGMENT (fixp->fx_addsy);
      ssec = S_GET_SEGMENT (fixp->fx_subsy);

      /* If we have a difference between two different, non-absolute symbols
	 we must generate two relocs (one for each symbol) and allow the
	 linker to resolve them - relaxation may change the distances between
	 symbols, even local symbols defined in the same section.

	 Unfortunately we cannot do this with assembler generated local labels
	 because there can be multiple incarnations of the same label, with
	 exactly the same name, in any given section and the linker will have
	 no way to identify the correct one.  Instead we just have to hope
	 that no relaxation will occur between the local label and the other
	 symbol in the expression.

	 Similarly we have to compute differences between symbols in the .eh_frame
	 section as the linker is not smart enough to apply relocations there
	 before attempting to process it.  */
      if ((ssec != absolute_section || asec != absolute_section)
	  && (fixp->fx_addsy != fixp->fx_subsy)
	  && strcmp (ssec->name, ".eh_frame") != 0
	  && ! S_IS_GAS_LOCAL (fixp->fx_addsy)
	  && ! S_IS_GAS_LOCAL (fixp->fx_subsy))
	{
	  arelent * reloc2 = XNEW (arelent);

	  relocs[0] = reloc2;
	  relocs[1] = reloc;

	  reloc2->address = reloc->address;
	  reloc2->howto = bfd_reloc_type_lookup (stdoutput,
						 BFD_RELOC_MSP430_SYM_DIFF);
	  reloc2->addend = - S_GET_VALUE (fixp->fx_subsy);

	  if (ssec == absolute_section)
	    reloc2->sym_ptr_ptr = bfd_abs_section_ptr->symbol_ptr_ptr;
	  else
	    {
	      reloc2->sym_ptr_ptr = XNEW (asymbol *);
	      *reloc2->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_subsy);
	    }

	  reloc->addend = fixp->fx_offset;
	  if (asec == absolute_section)
	    {
	      reloc->addend += S_GET_VALUE (fixp->fx_addsy);
	      reloc->sym_ptr_ptr = bfd_abs_section_ptr->symbol_ptr_ptr;
	    }
	  else
	    {
	      reloc->sym_ptr_ptr = XNEW (asymbol *);
	      *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
	    }

	  fixp->fx_pcrel = 0;
	  fixp->fx_done = 1;
	  return relocs;
	}
      else
	{
	  char *fixpos = fixp->fx_where + fixp->fx_frag->fr_literal;

	  reloc->addend = (S_GET_VALUE (fixp->fx_addsy)
			   - S_GET_VALUE (fixp->fx_subsy) + fixp->fx_offset);

	  switch (fixp->fx_r_type)
	    {
	    case BFD_RELOC_8:
	      md_number_to_chars (fixpos, reloc->addend, 1);
	      break;

	    case BFD_RELOC_16:
	      md_number_to_chars (fixpos, reloc->addend, 2);
	      break;

	    case BFD_RELOC_24:
	      md_number_to_chars (fixpos, reloc->addend, 3);
	      break;

	    case BFD_RELOC_32:
	      md_number_to_chars (fixpos, reloc->addend, 4);
	      break;

	    default:
	      reloc->sym_ptr_ptr
		= (asymbol **) bfd_abs_section_ptr->symbol_ptr_ptr;
	      return relocs;
	    }

	  free (reloc);
	  return & no_relocs;
	}
    }
  else
    {
#if 0
      if (fixp->fx_r_type == BFD_RELOC_MSP430X_ABS16
	  && S_GET_SEGMENT (fixp->fx_addsy) == absolute_section)
	{
	  bfd_vma amount = S_GET_VALUE (fixp->fx_addsy);
	  char *fixpos = fixp->fx_where + fixp->fx_frag->fr_literal;

	  md_number_to_chars (fixpos, amount, 2);
	  free (reloc);
	  return & no_relocs;
	}
#endif
      reloc->sym_ptr_ptr = XNEW (asymbol *);
      *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
      reloc->addend = fixp->fx_offset;

      if (fixp->fx_r_type == BFD_RELOC_VTABLE_INHERIT
	  || fixp->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
	reloc->address = fixp->fx_offset;
    }

  return relocs;
}

int
md_estimate_size_before_relax (fragS * fragP ATTRIBUTE_UNUSED,
			       asection * segment_type ATTRIBUTE_UNUSED)
{
  if (fragP->fr_symbol && S_GET_SEGMENT (fragP->fr_symbol) == segment_type)
    {
      /* This is a jump -> pcrel mode. Nothing to do much here.
         Return value == 2.  */
      fragP->fr_subtype =
	  ENCODE_RELAX (RELAX_LEN (fragP->fr_subtype), STATE_BITS10);
    }
  else if (fragP->fr_symbol)
    {
      /* It's got a segment, but it's not ours.   Even if fr_symbol is in
	 an absolute segment, we don't know a displacement until we link
	 object files. So it will always be long. This also applies to
	 labels in a subsegment of current. Liker may relax it to short
	 jump later. Return value == 8.  */
      fragP->fr_subtype =
	  ENCODE_RELAX (RELAX_LEN (fragP->fr_subtype), STATE_WORD);
    }
  else
    {
      /* We know the abs value. may be it is a jump to fixed address.
         Impossible in our case, cause all constants already handled. */
      fragP->fr_subtype =
	  ENCODE_RELAX (RELAX_LEN (fragP->fr_subtype), STATE_UNDEF);
    }

  return md_relax_table[fragP->fr_subtype].rlx_length;
}

void
md_convert_frag (bfd * abfd ATTRIBUTE_UNUSED,
		 asection * sec ATTRIBUTE_UNUSED,
		 fragS * fragP)
{
  char * where = 0;
  int rela = -1;
  int i;
  struct rcodes_s * cc = NULL;
  struct hcodes_s * hc = NULL;

  switch (fragP->fr_subtype)
    {
    case ENCODE_RELAX (STATE_UNCOND_BRANCH, STATE_BITS10):
    case ENCODE_RELAX (STATE_SIMPLE_BRANCH, STATE_BITS10):
    case ENCODE_RELAX (STATE_NOOV_BRANCH, STATE_BITS10):
      /* We do not have to convert anything here.
         Just apply a fix.  */
      rela = BFD_RELOC_MSP430_10_PCREL;
      break;

    case ENCODE_RELAX (STATE_UNCOND_BRANCH, STATE_WORD):
    case ENCODE_RELAX (STATE_UNCOND_BRANCH, STATE_UNDEF):
      /* Convert uncond branch jmp lab -> br lab.  */
      if (target_is_430x ())
	cc = msp430x_rcodes + 7;
      else
	cc = msp430_rcodes + 7;
      where = fragP->fr_literal + fragP->fr_fix;
      bfd_putl16 (cc->lop0, where);
      rela = BFD_RELOC_MSP430_RL_PCREL;
      fragP->fr_fix += 2;
      break;

    case ENCODE_RELAX (STATE_SIMPLE_BRANCH, STATE_WORD):
    case ENCODE_RELAX (STATE_SIMPLE_BRANCH, STATE_UNDEF):
      {
	/* Other simple branches.  */
	int insn = bfd_getl16 (fragP->fr_opcode);

	insn &= 0xffff;
	/* Find actual instruction.  */
	if (target_is_430x ())
	  {
	    for (i = 0; i < 7 && !cc; i++)
	      if (msp430x_rcodes[i].sop == insn)
		cc = msp430x_rcodes + i;
	  }
	else
	  {
	    for (i = 0; i < 7 && !cc; i++)
	      if (msp430_rcodes[i].sop == insn)
		cc = & msp430_rcodes[i];
	  }

	if (!cc || !cc->name)
	  as_fatal (_("internal inconsistency problem in %s: insn %04lx"),
		    __func__, (long) insn);
	where = fragP->fr_literal + fragP->fr_fix;
	bfd_putl16 (cc->lop0, where);
	bfd_putl16 (cc->lop1, where + 2);
	rela = BFD_RELOC_MSP430_RL_PCREL;
	fragP->fr_fix += 4;
      }
      break;

    case ENCODE_RELAX (STATE_NOOV_BRANCH, STATE_WORD):
    case ENCODE_RELAX (STATE_NOOV_BRANCH, STATE_UNDEF):
      if (target_is_430x ())
	cc = msp430x_rcodes + 6;
      else
	cc = msp430_rcodes + 6;
      where = fragP->fr_literal + fragP->fr_fix;
      bfd_putl16 (cc->lop0, where);
      bfd_putl16 (cc->lop1, where + 2);
      bfd_putl16 (cc->lop2, where + 4);
      rela = BFD_RELOC_MSP430_RL_PCREL;
      fragP->fr_fix += 6;
      break;

    case ENCODE_RELAX (STATE_EMUL_BRANCH, STATE_BITS10):
      {
	int insn = bfd_getl16 (fragP->fr_opcode + 2);

	insn &= 0xffff;
	if (target_is_430x ())
	  {
	    for (i = 0; i < 4 && !hc; i++)
	      if (msp430x_hcodes[i].op1 == insn)
		hc = msp430x_hcodes + i;
	  }
	else
	  {
	    for (i = 0; i < 4 && !hc; i++)
	      if (msp430_hcodes[i].op1 == insn)
		hc = &msp430_hcodes[i];
	  }
	if (!hc || !hc->name)
	  as_fatal (_("internal inconsistency problem in %s: ext. insn %04lx"),
	      __func__, (long) insn);
	rela = BFD_RELOC_MSP430_10_PCREL;
	/* Apply a fix for a first label if necessary.
	   another fix will be applied to the next word of insn anyway.  */
	if (hc->tlab == 2)
	  fix_new (fragP, fragP->fr_fix, 2, fragP->fr_symbol,
		   fragP->fr_offset, true, rela);
	fragP->fr_fix += 2;
      }

      break;

    case ENCODE_RELAX (STATE_EMUL_BRANCH, STATE_WORD):
    case ENCODE_RELAX (STATE_EMUL_BRANCH, STATE_UNDEF):
      {
	int insn = bfd_getl16 (fragP->fr_opcode + 2);

	insn &= 0xffff;
	if (target_is_430x ())
	  {
	    for (i = 0; i < 4 && !hc; i++)
	      if (msp430x_hcodes[i].op1 == insn)
		hc = msp430x_hcodes + i;
	  }
	else
	  {
	    for (i = 0; i < 4 && !hc; i++)
	      if (msp430_hcodes[i].op1 == insn)
		hc = & msp430_hcodes[i];
	  }
	if (!hc || !hc->name)
	  as_fatal (_("internal inconsistency problem in %s: ext. insn %04lx"),
	      __func__, (long) insn);
	rela = BFD_RELOC_MSP430_RL_PCREL;
	where = fragP->fr_literal + fragP->fr_fix;
	bfd_putl16 (hc->lop0, where);
	bfd_putl16 (hc->lop1, where + 2);
	bfd_putl16 (hc->lop2, where + 4);
	fragP->fr_fix += 6;
      }
      break;

    default:
      as_fatal (_("internal inconsistency problem in %s: %lx"),
		__func__, (long) fragP->fr_subtype);
      break;
    }

  /* Now apply fix.  */
  fix_new (fragP, fragP->fr_fix, 2, fragP->fr_symbol,
	   fragP->fr_offset, true, rela);
  /* Just fixed 2 bytes.  */
  fragP->fr_fix += 2;
}

/* Relax fragment. Mostly stolen from hc11 and mcore
   which arches I think I know.  */

long
msp430_relax_frag (segT seg ATTRIBUTE_UNUSED, fragS * fragP,
		   long stretch ATTRIBUTE_UNUSED)
{
  long growth;
  offsetT aim = 0;
  symbolS *symbolP;
  const relax_typeS *this_type;
  const relax_typeS *start_type;
  relax_substateT next_state;
  relax_substateT this_state;
  const relax_typeS *table = md_relax_table;

  /* Nothing to be done if the frag has already max size.  */
  if (RELAX_STATE (fragP->fr_subtype) == STATE_UNDEF
      || RELAX_STATE (fragP->fr_subtype) == STATE_WORD)
    return 0;

  if (RELAX_STATE (fragP->fr_subtype) == STATE_BITS10)
    {
      symbolP = fragP->fr_symbol;
      if (symbol_resolved_p (symbolP))
	as_fatal (_("internal inconsistency problem in %s: resolved symbol"),
		  __func__);
      /* We know the offset. calculate a distance.  */
      aim = S_GET_VALUE (symbolP) - fragP->fr_address - fragP->fr_fix;
    }

  if (!msp430_enable_relax)
    {
      /* Relaxation is not enabled. So, make all jump as long ones
         by setting 'aim' to quite high value.  */
      aim = 0x7fff;
    }

  this_state = fragP->fr_subtype;
  start_type = this_type = table + this_state;

  if (aim < 0)
    {
      /* Look backwards.  */
      for (next_state = this_type->rlx_more; next_state;)
	if (aim >= this_type->rlx_backward || !this_type->rlx_backward)
	  next_state = 0;
	else
	  {
	    /* Grow to next state.  */
	    this_state = next_state;
	    this_type = table + this_state;
	    next_state = this_type->rlx_more;
	  }
    }
  else
    {
      /* Look forwards.  */
      for (next_state = this_type->rlx_more; next_state;)
	if (aim <= this_type->rlx_forward || !this_type->rlx_forward)
	  next_state = 0;
	else
	  {
	    /* Grow to next state.  */
	    this_state = next_state;
	    this_type = table + this_state;
	    next_state = this_type->rlx_more;
	  }
    }

  growth = this_type->rlx_length - start_type->rlx_length;
  if (growth != 0)
    fragP->fr_subtype = this_state;
  return growth;
}

/* Return FALSE if the fixup in fixp should be left alone and not
   adjusted.   We return FALSE here so that linker relaxation will
   work.  */

bool
msp430_fix_adjustable (struct fix *fixp ATTRIBUTE_UNUSED)
{
  /* If the symbol is in a non-code section then it should be OK.  */
  if (fixp->fx_addsy
      && ((S_GET_SEGMENT (fixp->fx_addsy)->flags & SEC_CODE) == 0))
    return true;

  return false;
}

/* Scan uleb128 subtraction expressions and insert fixups for them.
   e.g., .uleb128 .L1 - .L0
   Because relaxation may change the value of the subtraction, we
   must resolve them at link-time.  */

static void
msp430_insert_uleb128_fixes (bfd *abfd ATTRIBUTE_UNUSED,
			    asection *sec, void *xxx ATTRIBUTE_UNUSED)
{
  segment_info_type *seginfo = seg_info (sec);
  struct frag *fragP;

  subseg_set (sec, 0);

  for (fragP = seginfo->frchainP->frch_root;
       fragP; fragP = fragP->fr_next)
    {
      expressionS *exp, *exp_dup;

      if (fragP->fr_type != rs_leb128  || fragP->fr_symbol == NULL)
	continue;

      exp = symbol_get_value_expression (fragP->fr_symbol);

      if (exp->X_op != O_subtract)
	continue;

      /* FIXME: Skip for .sleb128.  */
      if (fragP->fr_subtype != 0)
	continue;

      exp_dup = xmemdup (exp, sizeof (*exp), sizeof (*exp));
      exp_dup->X_op = O_symbol;
      exp_dup->X_op_symbol = NULL;

      /* Emit the SUB relocation first, since the SET relocation will write out
	 the final value.  */
      exp_dup->X_add_symbol = exp->X_op_symbol;
      fix_new_exp (fragP, fragP->fr_fix, 0,
		   exp_dup, 0, BFD_RELOC_MSP430_SUB_ULEB128);

      exp_dup->X_add_symbol = exp->X_add_symbol;
      /* Insert relocations to resolve the subtraction at link-time.  */
      fix_new_exp (fragP, fragP->fr_fix, 0,
		   exp_dup, 0, BFD_RELOC_MSP430_SET_ULEB128);

    }
}

/* Called after all assembly has been done.  */
void
msp430_md_finish (void)
{
  if (check_for_nop)
    {
      if (gen_interrupt_nops)
	{
	  gen_nop ();
	  if (warn_interrupt_nops)
	    as_warn (INSERT_NOP_AT_EOF);
	}
      else if (warn_interrupt_nops)
	as_warn (_(WARN_NOP_AT_EOF));
    }

  /* Insert relocations for uleb128 directives, so the values can be recomputed
     at link time.  */
  bfd_map_over_sections (stdoutput, msp430_insert_uleb128_fixes, NULL);

  /* We have already emitted an error if any of the following attributes
     disagree with the attributes in the input assembly file.  See
     msp430_object_attribute.  */
  bfd_elf_add_proc_attr_int (stdoutput, OFBA_MSPABI_Tag_ISA,
			     target_is_430x () ? OFBA_MSPABI_Val_ISA_MSP430X
			     : OFBA_MSPABI_Val_ISA_MSP430);

  bfd_elf_add_proc_attr_int (stdoutput, OFBA_MSPABI_Tag_Code_Model,
			     large_model ? OFBA_MSPABI_Val_Code_Model_LARGE
			     : OFBA_MSPABI_Val_Code_Model_SMALL);

  bfd_elf_add_proc_attr_int (stdoutput, OFBA_MSPABI_Tag_Data_Model,
			     large_model ? OFBA_MSPABI_Val_Code_Model_LARGE
			     : OFBA_MSPABI_Val_Code_Model_SMALL);

  /* The data region GNU attribute is ignored for the small memory model.  */
  if (large_model)
    bfd_elf_add_obj_attr_int (stdoutput, OBJ_ATTR_GNU,
			      Tag_GNU_MSP430_Data_Region, lower_data_region_only
			      ? Val_GNU_MSP430_Data_Region_Lower
			      : Val_GNU_MSP430_Data_Region_Any);
}

/* Returns FALSE if there is a msp430 specific reason why the
   subtraction of two same-section symbols cannot be computed by
   the assembler.  */

bool
msp430_allow_local_subtract (expressionS * left,
			     expressionS * right,
			     segT section)
{
  /* If the symbols are not in a code section then they are OK.  */
  if ((section->flags & SEC_CODE) == 0)
    return true;

  if (S_IS_GAS_LOCAL (left->X_add_symbol) || S_IS_GAS_LOCAL (right->X_add_symbol))
    return true;

  if (left->X_add_symbol == right->X_add_symbol)
    return true;

  /* We have to assume that there may be instructions between the
     two symbols and that relaxation may increase the distance between
     them.  */
  return false;
}
