/* Disassembler code for CR16.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.
   Contributed by M R Swami Reddy (MR.Swami.Reddy@nsc.com).

   This file is part of GAS, GDB and the GNU binutils.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#include "sysdep.h"
#include "disassemble.h"
#include "opcode/cr16.h"
#include "libiberty.h"

/* String to print when opcode was not matched.  */
#define ILLEGAL  "illegal"
  /* Escape to 16-bit immediate.  */
#define ESCAPE_16_BIT  0xB

/* Extract 'n_bits' from 'a' starting from offset 'offs'.  */
#define EXTRACT(a, offs, n_bits) \
  (((a) >> (offs)) & ((1ul << ((n_bits) - 1) << 1) - 1))

/* Set Bit Mask - a mask to set all bits in a 32-bit word starting
   from offset 'offs'.  */
#define SBM(offs)  ((1ul << 31 << 1) - (1ul << (offs)))

/* Structure to map valid 'cinv' instruction options.  */

typedef struct
  {
    /* Cinv printed string.  */
    char *istr;
    /* Value corresponding to the string.  */
    char *ostr;
  }
cinv_entry;

/* CR16 'cinv' options mapping.  */
static const cinv_entry cr16_cinvs[] =
{
  {"cinv[i]",     "cinv    [i]"},
  {"cinv[i,u]",   "cinv    [i,u]"},
  {"cinv[d]",     "cinv    [d]"},
  {"cinv[d,u]",   "cinv    [d,u]"},
  {"cinv[d,i]",   "cinv    [d,i]"},
  {"cinv[d,i,u]", "cinv    [d,i,u]"}
};

/* Number of valid 'cinv' instruction options.  */
static int NUMCINVS = ARRAY_SIZE (cr16_cinvs);

/* Enum to distinguish different registers argument types.  */
typedef enum REG_ARG_TYPE
  {
    /* General purpose register (r<N>).  */
    REG_ARG = 0,
    /*Processor register   */
    P_ARG,
  }
REG_ARG_TYPE;

/* Current opcode table entry we're disassembling.  */
static const inst *instruction;
/* Current instruction we're disassembling.  */
static ins cr16_currInsn;
/* The current instruction is read into 3 consecutive words.  */
static wordU cr16_words[3];
/* Contains all words in appropriate order.  */
static ULONGLONG cr16_allWords;
/* Holds the current processed argument number.  */
static int processing_argument_number;
/* Nonzero means a IMM4 instruction.  */
static int imm4flag;
/* Nonzero means the instruction's original size is
   incremented (escape sequence is used).  */
static int size_changed;


/* Print the constant expression length.  */

static char *
print_exp_len (int size)
{
  switch (size)
    {
    case 4:
    case 5:
    case 6:
    case 8:
    case 14:
    case 16:
      return ":s";
    case 20:
    case 24:
    case 32:
      return ":m";
    case 48:
      return ":l";
    default:
      return "";
    }
}


/* Retrieve the number of operands for the current assembled instruction.  */

static int
get_number_of_operands (void)
{
  int i;

  for (i = 0; instruction->operands[i].op_type && i < MAX_OPERANDS; i++)
    ;

  return i;
}

/* Return the bit size for a given operand.  */

static int
getbits (operand_type op)
{
  if (op < MAX_OPRD)
    return cr16_optab[op].bit_size;

  return 0;
}

/* Return the argument type of a given operand.  */

static argtype
getargtype (operand_type op)
{
  if (op < MAX_OPRD)
    return cr16_optab[op].arg_type;

  return nullargs;
}

/* Given a 'CC' instruction constant operand, return its corresponding
   string. This routine is used when disassembling the 'CC' instruction.  */

static char *
getccstring (unsigned cc_insn)
{
  return (char *) cr16_b_cond_tab[cc_insn];
}


/* Given a 'cinv' instruction constant operand, return its corresponding
   string. This routine is used when disassembling the 'cinv' instruction. */

static char *
getcinvstring (const char *str)
{
  const cinv_entry *cinv;

  for (cinv = cr16_cinvs; cinv < (cr16_cinvs + NUMCINVS); cinv++)
    if (strcmp (cinv->istr, str) == 0)
      return cinv->ostr;

  return ILLEGAL;
}

/* Given the trap index in dispatch table, return its name.
   This routine is used when disassembling the 'excp' instruction.  */

static char *
gettrapstring (unsigned int trap_index)
{
  const trap_entry *trap;

  for (trap = cr16_traps; trap < cr16_traps + NUMTRAPS; trap++)
    if (trap->entry == trap_index)
      return trap->name;

  return ILLEGAL;
}

/* Given a register enum value, retrieve its name.  */

static char *
getregname (reg r)
{
  const reg_entry * regentry = cr16_regtab + r;

  if (regentry->type != CR16_R_REGTYPE)
    return ILLEGAL;

  return regentry->name;
}

/* Given a register pair enum value, retrieve its name.  */

static char *
getregpname (reg r)
{
  const reg_entry * regentry = cr16_regptab + r;

  if (regentry->type != CR16_RP_REGTYPE)
    return ILLEGAL;

  return regentry->name;
}

/* Given a index register pair enum value, retrieve its name.  */

static char *
getidxregpname (reg r)
{
  const reg_entry * regentry;

  switch (r)
   {
   case 0: r = 0; break;
   case 1: r = 2; break;
   case 2: r = 4; break;
   case 3: r = 6; break;
   case 4: r = 8; break;
   case 5: r = 10; break;
   case 6: r = 3; break;
   case 7: r = 5; break;
   default:
     break;
   }

  regentry = cr16_regptab + r;

  if (regentry->type != CR16_RP_REGTYPE)
    return ILLEGAL;

  return regentry->name;
}

/* Getting a processor register name.  */

static char *
getprocregname (int reg_index)
{
  const reg_entry *r;

  for (r = cr16_pregtab; r < cr16_pregtab + NUMPREGS; r++)
    if (r->image == reg_index)
      return r->name;

  return "ILLEGAL REGISTER";
}

/* Getting a processor register name - 32 bit size.  */

static char *
getprocpregname (int reg_index)
{
  const reg_entry *r;

  for (r = cr16_pregptab; r < cr16_pregptab + NUMPREGPS; r++)
    if (r->image == reg_index)
      return r->name;

  return "ILLEGAL REGISTER";
}

/* START and END are relating 'cr16_allWords' struct, which is 48 bits size.

                          START|--------|END
             +---------+---------+---------+---------+
             |         |   V    |     A    |   L     |
             +---------+---------+---------+---------+
                       0         16        32        48
    words                  [0]       [1]       [2]      */

static inline dwordU
makelongparameter (ULONGLONG val, int start, int end)
{
  return EXTRACT (val, 48 - end, end - start);
}

/* Build a mask of the instruction's 'constant' opcode,
   based on the instruction's printing flags.  */

static unsigned long
build_mask (void)
{
  unsigned long mask = SBM (instruction->match_bits);

  /* Adjust mask for bcond with 32-bit size instruction.  */
  if ((IS_INSN_MNEMONIC("b") && instruction->size == 2))
    mask = 0xff0f0000;

  return mask;
}

/* Search for a matching opcode. Return 1 for success, 0 for failure.  */

int
cr16_match_opcode (void)
{
  unsigned long mask;
  /* The instruction 'constant' opcode doesn't exceed 32 bits.  */
  unsigned long doubleWord = cr16_words[1] + ((unsigned) cr16_words[0] << 16);

  /* Start searching from end of instruction table.  */
  instruction = &cr16_instruction[NUMOPCODES - 2];

  /* Loop over instruction table until a full match is found.  */
  while (instruction >= cr16_instruction)
    {
      mask = build_mask ();

      if ((doubleWord & mask) == BIN (instruction->match,
				      instruction->match_bits))
	return 1;
      else
	instruction--;
    }
  return 0;
}

/* Set the proper parameter value for different type of arguments.  */

static void
make_argument (argument * a, int start_bits)
{
  int inst_bit_size;
  dwordU p;

  if ((instruction->size == 3) && a->size >= 16)
    inst_bit_size = 48;
  else
    inst_bit_size = 32;

  switch (a->type)
    {
    case arg_r:
      p = makelongparameter (cr16_allWords,
			     inst_bit_size - (start_bits + a->size),
			     inst_bit_size - start_bits);
      a->r = p;
      break;

    case arg_rp:
      p = makelongparameter (cr16_allWords,
			     inst_bit_size - (start_bits + a->size),
			     inst_bit_size - start_bits);
      a->rp = p;
      break;

    case arg_pr:
      p = makelongparameter (cr16_allWords,
			     inst_bit_size - (start_bits + a->size),
			     inst_bit_size - start_bits);
      a->pr = p;
      break;

    case arg_prp:
      p = makelongparameter (cr16_allWords,
			     inst_bit_size - (start_bits + a->size),
			     inst_bit_size - start_bits);
      a->prp = p;
      break;

    case arg_ic:
      p = makelongparameter (cr16_allWords,
			     inst_bit_size - (start_bits + a->size),
			     inst_bit_size - start_bits);
      a->constant = p;
      break;

    case arg_cc:
      p = makelongparameter (cr16_allWords,
			     inst_bit_size - (start_bits + a->size),
			     inst_bit_size - start_bits);
      a->cc = p;
      break;

    case arg_idxr:
      if (IS_INSN_TYPE (CSTBIT_INS) && instruction->mnemonic[4] == 'b')
	p = makelongparameter (cr16_allWords, 8, 9);
      else
	p = makelongparameter (cr16_allWords, 9, 10);
      a->i_r = p;
      p = makelongparameter (cr16_allWords,
			     inst_bit_size - a->size, inst_bit_size);
      a->constant = p;
      break;

    case arg_idxrp:
      p = makelongparameter (cr16_allWords, start_bits + 12, start_bits + 13);
      a->i_r = p;
      p = makelongparameter (cr16_allWords, start_bits + 13, start_bits + 16);
      a->rp = p;
      if (inst_bit_size > 32)
	{
	  p = makelongparameter (cr16_allWords, inst_bit_size - start_bits - 12,
				 inst_bit_size);
	  a->constant = (p & 0xffff) | (p >> 8 & 0xf0000);
	}
      else if (instruction->size == 2)
	{
	  p = makelongparameter (cr16_allWords, inst_bit_size - 22,
				 inst_bit_size);
	  a->constant = ((p & 0xf) | (((p >> 20) & 0x3) << 4)
			 | ((p >> 14 & 0x3) << 6) | (((p >>7) & 0x1f) << 7));
	}
      else if (instruction->size == 1 && a->size == 0)
	a->constant = 0;

      break;

    case arg_rbase:
      p = makelongparameter (cr16_allWords, inst_bit_size, inst_bit_size);
      a->constant = p;
      p = makelongparameter (cr16_allWords, inst_bit_size - (start_bits + 4),
			     inst_bit_size - start_bits);
      a->r = p;
      break;

    case arg_cr:
      p = makelongparameter (cr16_allWords, start_bits + 12, start_bits + 16);
      a->r = p;
      p = makelongparameter (cr16_allWords, inst_bit_size - 28, inst_bit_size);
      a->constant = ((p >> 8) & 0xf0000) | (p & 0xffff);
      break;

    case arg_crp:
      if (instruction->size == 1)
	p = makelongparameter (cr16_allWords, 12, 16);
      else
	p = makelongparameter (cr16_allWords, start_bits + 12, start_bits + 16);
      a->rp = p;

      if (inst_bit_size > 32)
	{
	  p = makelongparameter (cr16_allWords, inst_bit_size - start_bits - 12,
				 inst_bit_size);
	  a->constant = ((p & 0xffff) | (p >> 8 & 0xf0000));
	}
      else if (instruction->size == 2)
	{
	  p = makelongparameter (cr16_allWords, inst_bit_size - 16,
				 inst_bit_size);
	  a->constant = p;
	}
      else if (instruction->size == 1 && a->size != 0)
	{
	  p = makelongparameter (cr16_allWords, 4, 8);
	  if (IS_INSN_MNEMONIC ("loadw")
	      || IS_INSN_MNEMONIC ("loadd")
	      || IS_INSN_MNEMONIC ("storw")
	      || IS_INSN_MNEMONIC ("stord"))
	    a->constant = p * 2;
	  else
	    a->constant = p;
	}
      else /* below case for 0x0(reg pair) */
	a->constant = 0;

      break;

    case arg_c:

      if ((IS_INSN_TYPE (BRANCH_INS))
	  || (IS_INSN_MNEMONIC ("bal"))
	  || (IS_INSN_TYPE (CSTBIT_INS))
	  || (IS_INSN_TYPE (LD_STOR_INS)))
	{
	  switch (a->size)
	    {
	    case 8 :
	      p = makelongparameter (cr16_allWords, 0, start_bits);
	      a->constant = ((p & 0xf00) >> 4) | (p & 0xf);
	      break;

	    case 24:
	      if (instruction->size == 3)
		{
		  p = makelongparameter (cr16_allWords, 16, inst_bit_size);
		  a->constant = ((((p >> 16) & 0xf) << 20)
				 | (((p >> 24) & 0xf) << 16)
				 | (p & 0xffff));
		}
	      else if (instruction->size == 2)
		{
		  p = makelongparameter (cr16_allWords, 8, inst_bit_size);
		  a->constant = p;
		}
	      break;

	    default:
	      p = makelongparameter (cr16_allWords,
				     inst_bit_size - (start_bits + a->size),
				     inst_bit_size - start_bits);
	      a->constant = p;
	      break;
	    }
	}
      else
	{
	  p = makelongparameter (cr16_allWords,
				 inst_bit_size - (start_bits + a->size),
				 inst_bit_size - start_bits);
	  a->constant = p;
	}
      break;

    default:
      break;
    }
}

/*  Print a single argument.  */

static void
print_arg (argument *a, bfd_vma memaddr, struct disassemble_info *info)
{
  LONGLONG longdisp, mask;
  int sign_flag = 0;
  int relative = 0;
  bfd_vma number;
  void *stream = info->stream;
  fprintf_ftype func = info->fprintf_func;

  switch (a->type)
    {
    case arg_r:
      func (stream, "%s", getregname (a->r));
      break;

    case arg_rp:
      func (stream, "%s", getregpname (a->rp));
      break;

    case arg_pr:
      func (stream, "%s", getprocregname (a->pr));
      break;

    case arg_prp:
      func (stream, "%s", getprocpregname (a->prp));
      break;

    case arg_cc:
      func (stream, "%s", getccstring (a->cc));
      func (stream, "%s", "\t");
      break;

    case arg_ic:
      if (IS_INSN_MNEMONIC ("excp"))
	{
	  func (stream, "%s", gettrapstring (a->constant));
	  break;
	}
      else if ((IS_INSN_TYPE (ARITH_INS) || IS_INSN_TYPE (ARITH_BYTE_INS))
	       && ((instruction->size == 1) && (a->constant == 9)))
	func (stream, "$%d", -1);
      else if (INST_HAS_REG_LIST)
	func (stream, "$0x%lx", a->constant +1);
      else if (IS_INSN_TYPE (SHIFT_INS))
	{
	  longdisp = a->constant;
	  mask = ((LONGLONG)1 << a->size) - 1;
	  if (longdisp & ((LONGLONG)1 << (a->size -1)))
	    {
	      sign_flag = 1;
	      longdisp = ~(longdisp) + 1;
	    }
	  a->constant = (unsigned long int) (longdisp & mask);
	  func (stream, "$%d", ((int)(sign_flag ? -a->constant :
				      a->constant)));
	}
      else
	func (stream, "$0x%lx", a->constant);
      switch (a->size)
	{
	case 4  : case 5  : case 6  : case 8  :
	  func (stream, "%s", ":s"); break;
	case 16 : case 20 : func (stream, "%s", ":m"); break;
	case 24 : case 32 : func (stream, "%s", ":l"); break;
	default: break;
	}
      break;

    case arg_idxr:
      if (a->i_r == 0) func (stream, "[r12]");
      if (a->i_r == 1) func (stream, "[r13]");
      func (stream, "0x%lx", a->constant);
      func (stream, "%s", print_exp_len (instruction->size * 16));
      break;

    case arg_idxrp:
      if (a->i_r == 0) func (stream, "[r12]");
      if (a->i_r == 1) func (stream, "[r13]");
      func (stream, "0x%lx", a->constant);
      func (stream, "%s", print_exp_len (instruction->size * 16));
      func (stream, "%s", getidxregpname (a->rp));
      break;

    case arg_rbase:
      func (stream, "(%s)", getregname (a->r));
      break;

    case arg_cr:
      func (stream, "0x%lx", a->constant);
      func (stream, "%s", print_exp_len (instruction->size * 16));
      func (stream, "(%s)", getregname (a->r));
      break;

    case arg_crp:
      func (stream, "0x%lx", a->constant);
      func (stream, "%s", print_exp_len (instruction->size * 16));
      func (stream, "%s", getregpname (a->rp));
      break;

    case arg_c:
      /*Removed the *2 part as because implicit zeros are no more required.
	Have to fix this as this needs a bit of extension in terms of branch
	instructions. */
      if (IS_INSN_TYPE (BRANCH_INS) || IS_INSN_MNEMONIC ("bal"))
	{
	  relative = 1;
	  longdisp = a->constant;
	  /* REVISIT: To sync with WinIDEA and CR16 4.1tools, the below
	     line commented */
	  /* longdisp <<= 1; */
	  mask = ((LONGLONG)1 << a->size) - 1;
	  switch (a->size)
	    {
	    case 8  :
	      {
		longdisp <<= 1;
		if (longdisp & ((LONGLONG)1 << a->size))
		  {
		    sign_flag = 1;
		    longdisp = ~(longdisp) + 1;
		  }
		break;
	      }
	    case 16 :
	    case 24 :
	      {
		if (longdisp & 1)
		  {
		    sign_flag = 1;
		    longdisp = ~(longdisp) + 1;
		  }
		break;
	      }
	    default:
	      func (stream, "Wrong offset used in branch/bal instruction");
	      break;
	    }
	  a->constant = (unsigned long int) (longdisp & mask);
	}
      /* For branch Neq instruction it is 2*offset + 2.  */
      else if (IS_INSN_TYPE (BRANCH_NEQ_INS))
	a->constant = 2 * a->constant + 2;

      if ((!IS_INSN_TYPE (CSTBIT_INS)) && (!IS_INSN_TYPE (LD_STOR_INS)))
	(sign_flag) ? func (stream, "%s", "*-"): func (stream, "%s","*+");

      /* PR 10173: Avoid printing the 0x prefix twice.  */
      if (info->symtab_size > 0)
	func (stream, "%s", "0x");
      number = ((relative ? memaddr : 0) +
		(sign_flag ? ((- a->constant) & 0xffffffe) : a->constant));

      (*info->print_address_func) ((number & ((1 << 24) - 1)), info);

      func (stream, "%s", print_exp_len (instruction->size * 16));
      break;

    default:
      break;
    }
}

/* Print all the arguments of CURRINSN instruction.  */

static void
print_arguments (ins *currentInsn, bfd_vma memaddr, struct disassemble_info *info)
{
  int i;

  /* For "pop/push/popret RA instruction only.  */
  if ((IS_INSN_MNEMONIC ("pop")
       || (IS_INSN_MNEMONIC ("popret")
	   || (IS_INSN_MNEMONIC ("push"))))
      && currentInsn->nargs == 1)
    {
      info->fprintf_func (info->stream, "RA");
      return;
    }

  for (i = 0; i < currentInsn->nargs; i++)
    {
      processing_argument_number = i;

      /* For "bal (ra), disp17" instruction only.  */
      if ((IS_INSN_MNEMONIC ("bal")) && (i == 0) && instruction->size == 2)
	{
	  info->fprintf_func (info->stream, "(ra),");
	  continue;
	}

      if ((INST_HAS_REG_LIST) && (i == 2))
	info->fprintf_func (info->stream, "RA");
      else
	print_arg (&currentInsn->arg[i], memaddr, info);

      if ((i != currentInsn->nargs - 1) && (!IS_INSN_MNEMONIC ("b")))
	info->fprintf_func (info->stream, ",");
    }
}

/* Build the instruction's arguments.  */

void
cr16_make_instruction (void)
{
  int i;
  unsigned int shift;

  for (i = 0; i < cr16_currInsn.nargs; i++)
    {
      argument a;

      memset (&a, 0, sizeof (a));
      a.type = getargtype (instruction->operands[i].op_type);
      a.size = getbits (instruction->operands[i].op_type);
      shift = instruction->operands[i].shift;

      make_argument (&a, shift);
      cr16_currInsn.arg[i] = a;
    }

  /* Calculate instruction size (in bytes).  */
  cr16_currInsn.size = instruction->size + (size_changed ? 1 : 0);
  /* Now in bits.  */
  cr16_currInsn.size *= 2;
}

/* Retrieve a single word from a given memory address.  */

static wordU
get_word_at_PC (bfd_vma memaddr, struct disassemble_info *info)
{
  bfd_byte buffer[4];
  int status;
  wordU insn = 0;

  status = info->read_memory_func (memaddr, buffer, 2, info);

  if (status == 0)
    insn = (wordU) bfd_getl16 (buffer);

  return insn;
}

/* Retrieve multiple words (3) from a given memory address.  */

static void
get_words_at_PC (bfd_vma memaddr, struct disassemble_info *info)
{
  int i;
  bfd_vma mem;

  for (i = 0, mem = memaddr; i < 3; i++, mem += 2)
    cr16_words[i] = get_word_at_PC (mem, info);

  cr16_allWords =  ((ULONGLONG) cr16_words[0] << 32)
		   + ((unsigned long) cr16_words[1] << 16) + cr16_words[2];
}

/* Prints the instruction by calling print_arguments after proper matching.  */

int
print_insn_cr16 (bfd_vma memaddr, struct disassemble_info *info)
{
  int is_decoded;     /* Nonzero means instruction has a match.  */

  /* Initialize global variables.  */
  imm4flag = 0;
  size_changed = 0;

  /* Retrieve the encoding from current memory location.  */
  get_words_at_PC (memaddr, info);
  /* Find a matching opcode in table.  */
  is_decoded = cr16_match_opcode ();
  /* If found, print the instruction's mnemonic and arguments.  */
  if (is_decoded > 0 && (cr16_words[0] != 0 || cr16_words[1] != 0))
    {
      if (startswith (instruction->mnemonic, "cinv"))
	info->fprintf_func (info->stream,"%s",
			    getcinvstring (instruction->mnemonic));
      else
	info->fprintf_func (info->stream, "%s", instruction->mnemonic);

      if (((cr16_currInsn.nargs = get_number_of_operands ()) != 0)
	  && ! (IS_INSN_MNEMONIC ("b")))
	info->fprintf_func (info->stream, "\t");
      cr16_make_instruction ();
      /* For push/pop/pushrtn with RA instructions.  */
      if ((INST_HAS_REG_LIST) && ((cr16_words[0] >> 7) & 0x1))
	cr16_currInsn.nargs +=1;
      print_arguments (&cr16_currInsn, memaddr, info);
      return cr16_currInsn.size;
    }

  /* No match found.  */
  info->fprintf_func (info->stream,"%s ",ILLEGAL);
  return 2;
}
