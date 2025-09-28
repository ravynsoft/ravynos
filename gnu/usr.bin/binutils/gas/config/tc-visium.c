/* This is the machine dependent code of the Visium Assembler.

   Copyright (C) 2005-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "as.h"
#include "safe-ctype.h"
#include "subsegs.h"
#include "obstack.h"

#include "opcode/visium.h"
#include "elf/visium.h"
#include "dwarf2dbg.h"
#include "dw2gencfi.h"

/* Relocations and fixups:

   There are two different cases where an instruction or data
   directive operand requires relocation, or fixup.

   1. Relative branch instructions, take an 16-bit signed word
   offset. The formula for computing the offset is this:

    offset = (destination - pc) / 4

   Branch instructions never branch to a label not declared
   locally, so the actual offset can always be computed by the assembler.
   However, we provide a relocation type to support this.

   2. Load literal instructions, such as MOVIU, which take a 16-bit
   literal operand. The literal may be the top or bottom half of
   a 32-bit value computed by the assembler, or by the linker. We provide
   two relocation types here.

   3. Data items (long, word and byte) preset with a value computed by
   the linker.  */


/* This string holds the chars that always start a comment. If the
   pre-processor is disabled, these aren't very useful. The macro
   tc_comment_chars points to this.  */
const char *visium_comment_chars = "!;";

/* This array holds the chars that only start a comment at the beginning
   of a line.  If the line seems to have the form '# 123 filename' .line
   and .file directives will appear in the pre-processed output. Note that
   input_file.c hand checks for '#' at the beginning of the first line of
   the input file. This is because the compiler outputs #NO_APP at the
   beginning of its output. Also note that comments like this one will
   always work.  */
const char line_comment_chars[] = "#!;";
const char line_separator_chars[] = "";

/* Chars that can be used to separate mantissa from exponent in floating point
   numbers.  */
const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant, as in
   "0f12.456" or "0d1.2345e12".

   ...Also be aware that MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT may have to be
   changed in read.c. Ideally it shouldn't have to know about it at all,
   but nothing is ideal around here.  */
const char FLT_CHARS[] = "rRsSfFdDxXeE";

/* The size of a relocation record.  */
const int md_reloc_size = 8;

/* The architecture for which we are assembling.  */
enum visium_arch_val
{
  VISIUM_ARCH_DEF,
  VISIUM_ARCH_MCM24,
  VISIUM_ARCH_MCM,
  VISIUM_ARCH_GR6
};

static enum visium_arch_val visium_arch = VISIUM_ARCH_DEF;

/* The opcode architecture for which we are assembling.  In contrast to the
   previous one, this only determines which instructions are supported.  */
static enum visium_opcode_arch_val visium_opcode_arch = VISIUM_OPCODE_ARCH_DEF;

/* Flags to set in the ELF header e_flags field.  */
static flagword visium_flags = 0;

/* More than this number of nops in an alignment op gets a branch instead.  */
static unsigned int nop_limit = 5;


/* Translate internal representation of relocation info to BFD target
   format.  */
arelent *
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED, fixS *fixp)
{
  arelent *reloc;
  bfd_reloc_code_real_type code;

  reloc = XNEW (arelent);

  reloc->sym_ptr_ptr = XNEW (asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;

  switch (fixp->fx_r_type)
    {
    case BFD_RELOC_8:
    case BFD_RELOC_16:
    case BFD_RELOC_32:
    case BFD_RELOC_8_PCREL:
    case BFD_RELOC_16_PCREL:
    case BFD_RELOC_32_PCREL:
    case BFD_RELOC_VISIUM_HI16:
    case BFD_RELOC_VISIUM_LO16:
    case BFD_RELOC_VISIUM_IM16:
    case BFD_RELOC_VISIUM_REL16:
    case BFD_RELOC_VISIUM_HI16_PCREL:
    case BFD_RELOC_VISIUM_LO16_PCREL:
    case BFD_RELOC_VISIUM_IM16_PCREL:
    case BFD_RELOC_VTABLE_INHERIT:
    case BFD_RELOC_VTABLE_ENTRY:
      code = fixp->fx_r_type;
      break;
    default:
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    "internal error: unknown relocation type %d (`%s')",
		    fixp->fx_r_type,
		    bfd_get_reloc_code_name (fixp->fx_r_type));
      return 0;
    }

  reloc->howto = bfd_reloc_type_lookup (stdoutput, code);
  if (reloc->howto == 0)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    "internal error: can't export reloc type %d (`%s')",
		    fixp->fx_r_type, bfd_get_reloc_code_name (code));
      return 0;
    }

  /* Write the addend.  */
  if (reloc->howto->pc_relative == 0)
    reloc->addend = fixp->fx_addnumber;
  else
    reloc->addend = fixp->fx_offset;

  return reloc;
}

extern char *input_line_pointer;


static void s_bss (int);
static void visium_rdata (int);

static void visium_update_parity_bit (char *);
static char *parse_exp (char *, expressionS *);

/* These are the back-ends for the various machine dependent pseudo-ops.  */
void demand_empty_rest_of_line (void);


static void
s_bss (int ignore ATTRIBUTE_UNUSED)
{
  /* We don't support putting frags in the BSS segment, we fake it
     by marking in_bss, then looking at s_skip for clues.  */

  subseg_set (bss_section, 0);
  demand_empty_rest_of_line ();
}


/* This table describes all the machine specific pseudo-ops the assembler
   has to support. The fields are:

   1: Pseudo-op name without dot.
   2: Function to call to execute this pseudo-op.
   3: Integer arg to pass to the function.  */
const pseudo_typeS md_pseudo_table[] =
{
  {"bss", s_bss, 0},
  {"skip", s_space, 0},
  {"align", s_align_bytes, 0},
  {"noopt", s_ignore, 0},
  {"optim", s_ignore, 0},
  {"rdata", visium_rdata, 0},
  {"rodata", visium_rdata, 0},
  {0, 0, 0}
};


static void
visium_rdata (int xxx)
{
  char *save_line = input_line_pointer;
  static char section[] = ".rodata\n";

  /* Just pretend this is .section .rodata */
  input_line_pointer = section;
  obj_elf_section (xxx);
  input_line_pointer = save_line;
}

/* Align a section.  */
valueT
md_section_align (asection *seg, valueT addr)
{
  int align = bfd_section_alignment (seg);

  return ((addr + (1 << align) - 1) & -(1 << align));
}

void
md_number_to_chars (char *buf, valueT val, int n)
{
  number_to_chars_bigendian (buf, val, n);
}

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return 0;
}

/* The parse options.  */
const char *md_shortopts = "m:";

struct option md_longopts[] =
{
  {NULL, no_argument, NULL, 0}
};

size_t md_longopts_size = sizeof (md_longopts);

struct visium_option_table
{
  char *option;			/* Option name to match.  */
  char *help;			/* Help information.  */
  int *var;			/* Variable to change.  */
  int value;			/* To what to change it.  */
  char *deprecated;		/* If non-null, print this message. */
};

static struct visium_option_table visium_opts[] =
{
  {NULL, NULL, NULL, 0, NULL}
};

struct visium_arch_option_table
{
  const char *name;
  enum visium_arch_val value;
};

static struct visium_arch_option_table visium_archs[] =
{
  {"mcm24", VISIUM_ARCH_MCM24},
  {"mcm",   VISIUM_ARCH_MCM},
  {"gr5",   VISIUM_ARCH_MCM},
  {"gr6",   VISIUM_ARCH_GR6},
};

struct visium_long_option_table
{
  const char *option;			/* Substring to match.  */
  const char *help;			/* Help information.  */
  int (*func) (const char *subopt);	/* Function to decode sub-option.  */
  const char *deprecated;		/* If non-null, print this message.  */
};

static int
visium_parse_arch (const char *str)
{
  unsigned int i;

  if (strlen (str) == 0)
    {
      as_bad ("missing architecture name `%s'", str);
      return 0;
    }

  for (i = 0; i < ARRAY_SIZE (visium_archs); i++)
    if (strcmp (visium_archs[i].name, str) == 0)
      {
	visium_arch = visium_archs[i].value;
	return 1;
      }

  as_bad ("unknown architecture `%s'\n", str);
  return 0;
}

static struct visium_long_option_table visium_long_opts[] =
{
  {"mtune=", "<arch_name>\t assemble for architecture <arch name>",
   visium_parse_arch, NULL},
  {NULL, NULL, NULL, NULL}
};

int
md_parse_option (int c, const char *arg)
{
  struct visium_option_table *opt;
  struct visium_long_option_table *lopt;

  switch (c)
    {
    case 'a':
      /* Listing option.  Just ignore these, we don't support additional
         ones.  */
      return 0;

    default:
      for (opt = visium_opts; opt->option != NULL; opt++)
	{
	  if (c == opt->option[0]
	      && ((arg == NULL && opt->option[1] == 0)
		  || strcmp (arg, opt->option + 1) == 0))
	    {
	      /* If the option is deprecated, tell the user.  */
	      if (opt->deprecated != NULL)
		as_tsktsk ("option `-%c%s' is deprecated: %s", c,
			   arg ? arg : "", opt->deprecated);

	      if (opt->var != NULL)
		*opt->var = opt->value;

	      return 1;
	    }
	}

      for (lopt = visium_long_opts; lopt->option != NULL; lopt++)
	{
	  /* These options are expected to have an argument.  */
	  if (c == lopt->option[0]
	      && arg != NULL
	      && strncmp (arg, lopt->option + 1,
			  strlen (lopt->option + 1)) == 0)
	    {
	      /* If the option is deprecated, tell the user.  */
	      if (lopt->deprecated != NULL)
		as_tsktsk ("option `-%c%s' is deprecated: %s", c, arg,
			   lopt->deprecated);

	      /* Call the sup-option parser.  */
	      return lopt->func (arg + strlen (lopt->option) - 1);
	    }
	}

      return 0;
    }

  return 1;
}

void
md_show_usage (FILE * fp)
{
  struct visium_option_table *opt;
  struct visium_long_option_table *lopt;

  fprintf (fp, " Visium-specific assembler options:\n");

  for (opt = visium_opts; opt->option != NULL; opt++)
    if (opt->help != NULL)
      fprintf (fp, "  -%-23s%s\n", opt->option, opt->help);

  for (lopt = visium_long_opts; lopt->option != NULL; lopt++)
    if (lopt->help != NULL)
      fprintf (fp, "  -%s%s\n", lopt->option, lopt->help);

}

/* Interface to relax_segment.  */

/* Return the estimate of the size of a machine dependent frag
   before any relaxing is done.  It may also create any necessary
   relocations.  */
int
md_estimate_size_before_relax (fragS * fragP,
			       segT segment ATTRIBUTE_UNUSED)
{
  fragP->fr_var = 4;
  return 4;
}

/* Get the address of a symbol during relaxation.  From tc-arm.c.  */
static addressT
relaxed_symbol_addr (fragS *fragp, long stretch)
{
  fragS *sym_frag;
  addressT addr;
  symbolS *sym;

  sym = fragp->fr_symbol;
  sym_frag = symbol_get_frag (sym);
  know (S_GET_SEGMENT (sym) != absolute_section
	|| sym_frag == &zero_address_frag);
  addr = S_GET_VALUE (sym) + fragp->fr_offset;

  /* If frag has yet to be reached on this pass, assume it will
     move by STRETCH just as we did.  If this is not so, it will
     be because some frag between grows, and that will force
     another pass.  */
  if (stretch != 0
      && sym_frag->relax_marker != fragp->relax_marker)
    {
      fragS *f;

      /* Adjust stretch for any alignment frag.  Note that if have
	 been expanding the earlier code, the symbol may be
	 defined in what appears to be an earlier frag.  FIXME:
	 This doesn't handle the fr_subtype field, which specifies
	 a maximum number of bytes to skip when doing an
	 alignment.  */
      for (f = fragp; f != NULL && f != sym_frag; f = f->fr_next)
	{
	  if (f->fr_type == rs_align || f->fr_type == rs_align_code)
	    {
	      if (stretch < 0)
		stretch = - ((- stretch)
			     & ~ ((1 << (int) f->fr_offset) - 1));
	      else
		stretch &= ~ ((1 << (int) f->fr_offset) - 1);
	      if (stretch == 0)
		break;
	    }
	}
      if (f != NULL)
	addr += stretch;
    }

  return addr;
}

/* Relax a machine dependent frag.  This returns the amount by which
   the current size of the frag should change.  */
int
visium_relax_frag (asection *sec, fragS *fragP, long stretch)
{
  int old_size, new_size;
  addressT addr;

  /* We only handle relaxation for the BRR instruction.  */
  gas_assert (fragP->fr_subtype == mode_ci);

  if (!S_IS_DEFINED (fragP->fr_symbol)
      || sec != S_GET_SEGMENT (fragP->fr_symbol)
      || S_IS_WEAK (fragP->fr_symbol))
    return 0;

  old_size = fragP->fr_var;
  addr = relaxed_symbol_addr (fragP, stretch);

  /* If the target is the address of the instruction, we'll insert a NOP.  */
  if (addr == fragP->fr_address + fragP->fr_fix)
    new_size = 8;
  else
    new_size = 4;

  fragP->fr_var = new_size;
  return new_size - old_size;
}

/* Convert a machine dependent frag.  */
void
md_convert_frag (bfd * abfd ATTRIBUTE_UNUSED, segT sec ATTRIBUTE_UNUSED,
		 fragS * fragP)
{
  char *buf = &fragP->fr_literal[0] + fragP->fr_fix;
  expressionS exp;
  fixS *fixP;

  /* We only handle relaxation for the BRR instruction.  */
  gas_assert (fragP->fr_subtype == mode_ci);

  /* Insert the NOP if requested.  */
  if (fragP->fr_var == 8)
    {
      memcpy (buf + 4, buf, 4);
      memset (buf, 0, 4);
      fragP->fr_fix += 4;
    }

  exp.X_op = O_symbol;
  exp.X_add_symbol = fragP->fr_symbol;
  exp.X_add_number = fragP->fr_offset;

  /* Now we can create the relocation at the correct offset.  */
  fixP = fix_new_exp (fragP, fragP->fr_fix, 4, &exp, 1, BFD_RELOC_VISIUM_REL16);
  fixP->fx_file = fragP->fr_file;
  fixP->fx_line = fragP->fr_line;
  fragP->fr_fix += 4;
  fragP->fr_var = 0;
}

/* The location from which a PC relative jump should be calculated,
   given a PC relative jump reloc.  */
long
visium_pcrel_from_section (fixS *fixP, segT sec)
{
  if (fixP->fx_addsy != (symbolS *) NULL
      && (!S_IS_DEFINED (fixP->fx_addsy)
	  || S_GET_SEGMENT (fixP->fx_addsy) != sec))
    {
      /* The symbol is undefined (or is defined but not in this section).
         Let the linker figure it out.  */
      return 0;
    }

  /* Return the address of the instruction.  */
  return fixP->fx_where + fixP->fx_frag->fr_address;
}

/* Indicate whether a fixup against a locally defined
   symbol should be adjusted to be against the section
   symbol.  */
bool
visium_fix_adjustable (fixS *fix)
{
  /* We need the symbol name for the VTABLE entries.  */
  return (fix->fx_r_type != BFD_RELOC_VTABLE_INHERIT
	  && fix->fx_r_type != BFD_RELOC_VTABLE_ENTRY);
}

/* Update the parity bit of the 4-byte instruction in BUF.  */
static void
visium_update_parity_bit (char *buf)
{
  int p1 = (buf[0] & 0x7f) ^ buf[1] ^ buf[2] ^ buf[3];
  int p2 = 0;
  int i;

  for (i = 1; i <= 8; i++)
    {
      p2 ^= (p1 & 1);
      p1 >>= 1;
    }

  buf[0] = (buf[0] & 0x7f) | ((p2 << 7) & 0x80);
}

/* This is called from HANDLE_ALIGN in write.c.  Fill in the contents
   of an rs_align_code fragment.  */
void
visium_handle_align (fragS *fragP)
{
  valueT count
    = fragP->fr_next->fr_address - (fragP->fr_address + fragP->fr_fix);
  valueT fix = count & 3;
  char *p = fragP->fr_literal + fragP->fr_fix;

  if (fix)
    {
      memset (p, 0, fix);
      p += fix;
      count -= fix;
      fragP->fr_fix += fix;
    }

  if (count == 0)
    return;

  fragP->fr_var = 4;

  if (count > 4 * nop_limit && count <= 131068)
    {
      struct frag *rest;

      /* Make a branch, then follow with nops.  Insert another
         frag to handle the nops.  */
      md_number_to_chars (p, 0x78000000 + (count >> 2), 4);
      visium_update_parity_bit (p);

      rest = xmalloc (SIZEOF_STRUCT_FRAG + 4);
      memcpy (rest, fragP, SIZEOF_STRUCT_FRAG);
      fragP->fr_next = rest;
      rest->fr_address += rest->fr_fix + 4;
      rest->fr_fix = 0;
      /* If we leave the next frag as rs_align_code we'll come here
	 again, resulting in a bunch of branches rather than a
	 branch followed by nops.  */
      rest->fr_type = rs_align;
      p = rest->fr_literal;
    }

  memset (p, 0, 4);
}

/* Apply a fixS to the frags, now that we know the value it ought to
   hold.  */
void
md_apply_fix (fixS * fixP, valueT * value, segT segment)
{
  char *buf = fixP->fx_where + fixP->fx_frag->fr_literal;
  offsetT val;
  long insn;

  val = *value;

  gas_assert (fixP->fx_r_type < BFD_RELOC_UNUSED);

  /* Remember value for tc_gen_reloc.  */
  fixP->fx_addnumber = val;

  /* Since DIFF_EXPR_OK is defined, .-foo gets turned into PC
     relative relocs. If this has happened, a non-PC relative
     reloc must be reinstalled with its PC relative version here.  */
  if (fixP->fx_pcrel)
    {
      switch (fixP->fx_r_type)
	{
	case BFD_RELOC_8:
	  fixP->fx_r_type = BFD_RELOC_8_PCREL;
	  break;
	case BFD_RELOC_16:
	  fixP->fx_r_type = BFD_RELOC_16_PCREL;
	  break;
	case BFD_RELOC_32:
	  fixP->fx_r_type = BFD_RELOC_32_PCREL;
	  break;
	case BFD_RELOC_VISIUM_HI16:
	  fixP->fx_r_type = BFD_RELOC_VISIUM_HI16_PCREL;
	  break;
	case BFD_RELOC_VISIUM_LO16:
	  fixP->fx_r_type = BFD_RELOC_VISIUM_LO16_PCREL;
	  break;
	case BFD_RELOC_VISIUM_IM16:
	  fixP->fx_r_type = BFD_RELOC_VISIUM_IM16_PCREL;
	  break;
	default:
	  break;
	}
    }

  /* If this is a data relocation, just output VAL.  */
  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_8:
    case BFD_RELOC_8_PCREL:
      md_number_to_chars (buf, val, 1);
      break;
    case BFD_RELOC_16:
    case BFD_RELOC_16_PCREL:
      md_number_to_chars (buf, val, 2);
      break;
    case BFD_RELOC_32:
    case BFD_RELOC_32_PCREL:
      md_number_to_chars (buf, val, 4);
      break;
    case BFD_RELOC_VTABLE_INHERIT:
    case BFD_RELOC_VTABLE_ENTRY:
      fixP->fx_done = 0;
      break;
    default:
      /* It's a relocation against an instruction.  */
      insn = bfd_getb32 ((unsigned char *) buf);

      switch (fixP->fx_r_type)
	{
	case BFD_RELOC_VISIUM_REL16:
	  if (fixP->fx_addsy == NULL
	      || (S_IS_DEFINED (fixP->fx_addsy)
		  && S_GET_SEGMENT (fixP->fx_addsy) == segment))
	    {
	      if (val > 0x1fffc || val < -0x20000)
		as_bad_where
		 (fixP->fx_file, fixP->fx_line,
		  "16-bit word displacement out of range: value = %d",
		  (int) val);
	      val = (val >> 2);

	      insn = (insn & 0xffff0000) | (val & 0x0000ffff);
	    }
	  break;

	case BFD_RELOC_VISIUM_HI16:
	case BFD_RELOC_VISIUM_HI16_PCREL:
	  if (fixP->fx_addsy == NULL)
	    insn = (insn & 0xffff0000) | ((val >> 16) & 0x0000ffff);
	  break;

	case BFD_RELOC_VISIUM_LO16:
	case BFD_RELOC_VISIUM_LO16_PCREL:
	  if (fixP->fx_addsy == NULL)
	    insn = (insn & 0xffff0000) | (val & 0x0000ffff);
	  break;

	case BFD_RELOC_VISIUM_IM16:
	case BFD_RELOC_VISIUM_IM16_PCREL:
	  if (fixP->fx_addsy == NULL)
	    {
	      if ((val & 0xffff0000) != 0)
		as_bad_where (fixP->fx_file, fixP->fx_line,
			      "16-bit immediate out of range: value = %d",
			      (int) val);

	      insn = (insn & 0xffff0000) | val;
	    }
	  break;

	case BFD_RELOC_NONE:
	default:
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			"bad or unhandled relocation type: 0x%02x",
			fixP->fx_r_type);
	  break;
	}

      bfd_putb32 (insn, (unsigned char *) buf);
      visium_update_parity_bit (buf);
      break;
    }

  /* Are we finished with this relocation now?  */
  if (fixP->fx_addsy == NULL)
    fixP->fx_done = 1;
}

char *
parse_exp (char *s, expressionS * op)
{
  char *save = input_line_pointer;
  char *new;

  if (!s)
    {
      return s;
    }

  input_line_pointer = s;
  expression (op);
  new = input_line_pointer;
  input_line_pointer = save;
  return new;
}

/* If the given string is a Visium opcode mnemonic return the code
   otherwise return -1. Use binary chop to find matching entry.  */
static int
get_opcode (int *code, enum addressing_mode *mode, char *flags, char *mnem)
{
  int l = 0;
  int r = sizeof (opcode_table) / sizeof (struct opcode_entry) - 1;

  do
    {
      int mid = (l + r) / 2;
      int ans = strcmp (mnem, opcode_table[mid].mnem);

      if (ans < 0)
	r = mid - 1;
      else if (ans > 0)
	l = mid + 1;
      else
	{
	  *code = opcode_table[mid].code;
	  *mode = opcode_table[mid].mode;
	  *flags = opcode_table[mid].flags;

	  return 0;
	}
    }
  while (l <= r);

  return -1;
}

/* This function is called when the assembler starts up. It is called
   after the options have been parsed and the output file has been
   opened.  */
void
md_begin (void)
{
  switch (visium_arch)
    {
    case VISIUM_ARCH_DEF:
      break;
    case VISIUM_ARCH_MCM24:
      visium_opcode_arch = VISIUM_OPCODE_ARCH_GR5;
      visium_flags |= EF_VISIUM_ARCH_MCM24;
      break;
    case VISIUM_ARCH_MCM:
      visium_opcode_arch = VISIUM_OPCODE_ARCH_GR5;
      visium_flags |= EF_VISIUM_ARCH_MCM;
      break;
    case VISIUM_ARCH_GR6:
      visium_opcode_arch = VISIUM_OPCODE_ARCH_GR6;
      visium_flags |= EF_VISIUM_ARCH_MCM | EF_VISIUM_ARCH_GR6;
      nop_limit = 2;
      break;
    default:
      gas_assert (0);
    }

  bfd_set_private_flags (stdoutput, visium_flags);
}

/* This is identical to the md_atof in m68k.c.  I think this is right,
   but I'm not sure.

   Turn a string in input_line_pointer into a floating point constant of type
   type, and store the appropriate bytes in *litP.  The number of LITTLENUMS
   emitted is stored in *sizeP .  An error message is returned,
   or NULL on OK.  */

const char *
md_atof (int type, char *litP, int *sizeP)
{
  int i, prec;
  LITTLENUM_TYPE words[MAX_LITTLENUMS];
  char *t;

  switch (type)
    {
    case 'f':
    case 'F':
    case 's':
    case 'S':
      prec = 2;
      break;

    case 'd':
    case 'D':
    case 'r':
    case 'R':
      prec = 4;
      break;

    case 'x':
    case 'X':
      prec = 6;
      break;

    case 'p':
    case 'P':
      prec = 6;
      break;

    default:
      *sizeP = 0;
      return _("Bad call to MD_ATOF()");
    }

  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;
  *sizeP = prec * sizeof (LITTLENUM_TYPE);

  if (target_big_endian)
    {
      for (i = 0; i < prec; i++)
	{
	  md_number_to_chars (litP, (valueT) words[i],
			      sizeof (LITTLENUM_TYPE));
	  litP += sizeof (LITTLENUM_TYPE);
	}
    }
  else
    {
      for (i = prec - 1; i >= 0; i--)
	{
	  md_number_to_chars (litP, (valueT) words[i],
			      sizeof (LITTLENUM_TYPE));
	  litP += sizeof (LITTLENUM_TYPE);
	}
    }

  return 0;
}

static inline char *
skip_space (char *s)
{
  while (*s == ' ' || *s == '\t')
    ++s;

  return s;
}

static int
parse_gen_reg (char **sptr, int *rptr)
{
  char *s = skip_space (*sptr);
  char buf[10];
  int cnt;
  int l, r;

  cnt = 0;
  memset (buf, '\0', 10);
  while ((ISALNUM (*s)) && cnt < 10)
    buf[cnt++] = TOLOWER (*s++);

  l = 0;
  r = sizeof (gen_reg_table) / sizeof (struct reg_entry) - 1;

  do
    {
      int mid = (l + r) / 2;
      int ans = strcmp (buf, gen_reg_table[mid].name);

      if (ans < 0)
	r = mid - 1;
      else if (ans > 0)
	l = mid + 1;
      else
	{
	  *rptr = gen_reg_table[mid].code;
	  *sptr = s;
	  return 0;
	}
    }
  while (l <= r);

  return -1;
}

static int
parse_fp_reg (char **sptr, int *rptr)
{
  char *s = skip_space (*sptr);
  char buf[10];
  int cnt;
  int l, r;

  cnt = 0;
  memset (buf, '\0', 10);
  while ((ISALNUM (*s)) && cnt < 10)
    buf[cnt++] = TOLOWER (*s++);

  l = 0;
  r = sizeof (fp_reg_table) / sizeof (struct reg_entry) - 1;

  do
    {
      int mid = (l + r) / 2;
      int ans = strcmp (buf, fp_reg_table[mid].name);

      if (ans < 0)
	r = mid - 1;
      else if (ans > 0)
	l = mid + 1;
      else
	{
	  *rptr = fp_reg_table[mid].code;
	  *sptr = s;
	  return 0;
	}
    }
  while (l <= r);

  return -1;
}

static int
parse_cc (char **sptr, int *rptr)
{
  char *s = skip_space (*sptr);
  char buf[10];
  int cnt;
  int l, r;

  cnt = 0;
  memset (buf, '\0', 10);
  while ((ISALNUM (*s)) && cnt < 10)
    buf[cnt++] = TOLOWER (*s++);

  l = 0;
  r = sizeof (cc_table) / sizeof (struct cc_entry) - 1;

  do
    {
      int mid = (l + r) / 2;
      int ans = strcmp (buf, cc_table[mid].name);

      if (ans < 0)
	r = mid - 1;
      else if (ans > 0)
	l = mid + 1;
      else
	{
	  *rptr = cc_table[mid].code;
	  *sptr = s;
	  return 0;
	}
    }
  while (l <= r);

  return -1;
}

/* Previous dest is the destination register number of the instruction
   before the current one.  */
static int previous_dest = 0;
static int previous_mode = 0;
static int condition_code = 0;
static int this_dest = 0;
static int this_mode = 0;


/* This is the main function in this file. It takes a line of assembly language
   source code and assembles it. Note, labels and pseudo ops have already
   been removed, so too has leading white space. */
void
md_assemble (char *str0)
{
  char *str = str0;
  int cnt;
  char mnem[10];
  int opcode;
  enum addressing_mode amode;
  char arch_flags;
  int ans;

  char *output;
  int reloc = 0;
  relax_substateT relax = 0;
  expressionS e1;
  int r1, r2, r3;
  int cc;
  int indx;

  /* Initialize the expression.  */
  e1.X_op = O_absent;

  /* Initialize destination register.
     If the instruction we just looked at is in the delay slot of an
     unconditional branch, then there is no index hazard.  */
  if ((previous_mode == mode_cad || previous_mode == mode_ci)
      && condition_code == 15)
    this_dest = 0;

  previous_dest = this_dest;
  previous_mode = this_mode;
  this_dest = 0;

  /* Drop leading whitespace (probably not required).  */
  while (*str == ' ')
    str++;

  /* Get opcode mnemonic and make sure it's in lower case.  */
  cnt = 0;
  memset (mnem, '\0', 10);
  while ((ISALNUM (*str) || *str == '.' || *str == '_') && cnt < 10)
    mnem[cnt++] = TOLOWER (*str++);

  /* Look up mnemonic in opcode table, and get the code,
     the instruction format, and the flags that indicate
     which family members support this mnemonic.  */
  if (get_opcode (&opcode, &amode, &arch_flags, mnem) < 0)
    {
      as_bad ("Unknown instruction mnemonic `%s'", mnem);
      return;
    }

  if ((VISIUM_OPCODE_ARCH_MASK (visium_opcode_arch) & arch_flags) == 0)
    {
      as_bad ("Architecture mismatch on `%s'", mnem);
      return;
    }

  this_mode = amode;

  switch (amode)
    {
    case mode_d:
      /* register :=
         Example:
         readmda r1  */
      ans = parse_gen_reg (&str, &r1);
      if (ans < 0)
	{
	  as_bad ("Dest register required");
	  return;
	}
      opcode |= (r1 << 10);
      this_dest = r1;
      break;

    case mode_a:
      /* op= register
         Example: asld r1  */
      ans = parse_gen_reg (&str, &r1);
      if (ans < 0)
	{
	  as_bad ("SourceA register required");
	  return;
	}
      opcode |= (r1 << 16);
      break;

    case mode_ab:
      /* register * register
         Example:
         mults r1,r2  */
      ans = parse_gen_reg (&str, &r1);
      if (ans < 0)
	{
	  as_bad ("SourceA register required");
	  return;
	}
      str = skip_space (str);
      if (*str == ',')
	{
	  str++;
	  ans = parse_gen_reg (&str, &r2);
	  if (ans < 0)
	    {
	      as_bad ("SourceB register required");
	      return;
	    }
	  opcode |= (r1 << 16) | (r2 << 4);
	}
      else
	{
	  as_bad ("SourceB register required");
	  return;
	}
      break;

    case mode_da:
      /* register := register
         Example:
         extb.l  r1,r2  */
      ans = parse_gen_reg (&str, &r1);
      if (ans < 0)
	{
	  as_bad ("Dest register required");
	  return;
	}
      str = skip_space (str);
      if (*str == ',')
	{
	  str++;
	  ans = parse_gen_reg (&str, &r2);
	  if (ans < 0)
	    {
	      as_bad ("SourceA register required");
	      return;
	    }
	  opcode |= (r1 << 10) | (r2 << 16);
	}
      else
	{
	  as_bad ("SourceB register required");
	  return;
	}
      this_dest = r1;
      break;

    case mode_dab:
      /* register := register * register
         Example:
         add.l r1,r2,r3  */
      ans = parse_gen_reg (&str, &r1);
      if (ans < 0)
	{
	  as_bad ("Dest register required");
	  return;
	}
      str = skip_space (str);
      if (*str == ',')
	{
	  str++;
	  ans = parse_gen_reg (&str, &r2);
	  if (ans < 0)
	    {
	      as_bad ("SourceA register required");
	      return;
	    }
	  str = skip_space (str);
	  if (*str == ',')
	    {
	      str++;
	      ans = parse_gen_reg (&str, &r3);
	      if (ans < 0)
		{
		  as_bad ("SourceB register required");
		  return;
		}

	      /* Got three regs, assemble instruction.  */
	      opcode |= (r1 << 10) | (r2 << 16) | (r3 << 4);
	    }
	  else
	    {
	      as_bad ("SourceA register required");
	      return;
	    }
	}
      else
	{
	  as_bad ("Dest register required");
	  return;
	}
      this_dest = r1;
      break;

    case mode_iab:
      /* 5-bit immediate * register * register
         Example:
         eamwrite 3,r1,r2  */
      str = parse_exp (str, &e1);
      str = skip_space (str);
      if (e1.X_op != O_absent && *str == ',')
	{
	  int eam_op = e1.X_add_number;

	  str = skip_space (str + 1);
	  ans = parse_gen_reg (&str, &r2);
	  if (ans < 0)
	    {
	      as_bad ("SourceA register required");
	      return;
	    }
	  str = skip_space (str);
	  if (*str == ',')
	    {
	      str++;
	      ans = parse_gen_reg (&str, &r3);
	      if (ans < 0)
		{
		  as_bad ("SourceB register required");
		  return;
		}

	      /* Got three operands, assemble instruction.  */
	      if (eam_op < 0 || eam_op > 31)
		{
		  as_bad ("eam_op out of range");
		}
	      opcode |= ((eam_op & 0x1f) << 10) | (r2 << 16) | (r3 << 4);
	    }
	}
      else
	{
	  as_bad ("EAM_OP required");
	  return;
	}
      break;

    case mode_0ab:
      /* zero * register * register
         Example:
         cmp.l  r1,r2 */
      ans = parse_gen_reg (&str, &r1);
      if (ans < 0)
	{
	  as_bad ("SourceA register required");
	  return;
	}
      str = skip_space (str);
      if (*str == ',')
	{
	  str++;
	  ans = parse_gen_reg (&str, &r2);
	  if (ans < 0)
	    {
	      as_bad ("SourceB register required");
	      return;
	    }
	  opcode |= (r1 << 16) | (r2 << 4);
	}
      else
	{
	  as_bad ("SourceB register required");
	  return;
	}
      break;

    case mode_da0:
      /* register * register * zero
         Example:
         move.l  r1,r2  */
      ans = parse_gen_reg (&str, &r1);
      if (ans < 0)
	{
	  as_bad ("Dest register required");
	  return;
	}
      str = skip_space (str);
      if (*str == ',')
	{
	  str++;
	  ans = parse_gen_reg (&str, &r2);
	  if (ans < 0)
	    {
	      as_bad ("SourceA register required");
	      return;
	    }
	  opcode |= (r1 << 10) | (r2 << 16);
	}
      else
	{
	  as_bad ("SourceA register required");
	  return;
	}
      this_dest = r1;
      break;

    case mode_cad:
      /* condition * register * register
         Example:
         bra  tr,r1,r2  */
      ans = parse_cc (&str, &cc);
      if (ans < 0)
	{
	  as_bad ("condition code required");
	  return;
	}

      str = skip_space (str);
      if (*str == ',')
	{
	  str = skip_space (str + 1);
	  ans = parse_gen_reg (&str, &r2);
	  if (ans < 0)
	    {
	      as_bad ("SourceA register required");
	      return;
	    }
	  str = skip_space (str);
	  if (*str == ',')
	    {
	      str++;
	      ans = parse_gen_reg (&str, &r3);
	      if (ans < 0)
		{
		  as_bad ("Dest register required");
		  return;
		}

	      /* Got three operands, assemble instruction.  */
	      opcode |= (cc << 27) | (r2 << 16) | (r3 << 10);
	    }
	  else
	    {
	      as_bad ("Dest register required");
	      return;
	    }
	}
      else
	{
	  as_bad ("SourceA register required");
	  return;
	}

      if (previous_mode == mode_cad || previous_mode == mode_ci)
	as_bad ("branch instruction in delay slot");

      /* For the GR6, BRA insns must be aligned on 64-bit boundaries.  */
      if (visium_arch == VISIUM_ARCH_GR6)
	do_align (3, NULL, 0, 0);

      this_dest = r3;
      condition_code = cc;
      break;

    case mode_das:
      /* register := register * 5-bit immediate/register shift count
         Example:
         asl.l  r1,r2,4  */
      ans = parse_gen_reg (&str, &r1);
      if (ans < 0)
	{
	  as_bad ("Dest register required");
	  return;
	}
      str = skip_space (str);
      if (*str == ',')
	{
	  str++;
	  ans = parse_gen_reg (&str, &r2);
	  if (ans < 0)
	    {
	      as_bad ("SourceA register required");
	      return;
	    }
	  str = skip_space (str);
	  if (*str == ',')
	    {
	      str++;
	      ans = parse_gen_reg (&str, &r3);
	      if (ans == 0)
		{
		  opcode |= (r1 << 10) | (r2 << 16) | (r3 << 4);
		}
	      else
		{
		  str = parse_exp (str, &e1);
		  if (e1.X_op == O_constant)
		    {
		      int imm = e1.X_add_number;

		      if (imm < 0 || imm > 31)
			as_bad ("immediate value out of range");

		      opcode |=
			(r1 << 10) | (r2 << 16) | (1 << 9) | ((imm & 0x1f) <<
							      4);
		    }
		  else
		    {
		      as_bad ("immediate operand required");
		      return;
		    }
		}
	    }
	}
      else
	{
	  as_bad ("SourceA register required");
	  return;
	}
      this_dest = r1;
      break;

    case mode_di:
      /* register := 5-bit immediate
         Example:
         eamread r1,3  */
      ans = parse_gen_reg (&str, &r1);
      if (ans < 0)
	{
	  as_bad ("Dest register required");
	  return;
	}
      str = skip_space (str);
      if (*str == ',')
	{
	  str++;
	  str = parse_exp (str, &e1);
	  if (e1.X_op == O_constant)
	    {
	      int opnd2 = e1.X_add_number;

	      if (opnd2 < 0 || opnd2 > 31)
		{
		  as_bad ("immediate operand out of range");
		  return;
		}
	      opcode |= (r1 << 10) | ((opnd2 & 0x1f) << 4);
	    }
	  else
	    {
	      as_bad ("immediate operand required");
	      return;
	    }
	}
      else
	{
	  as_bad ("immediate operand required");
	  return;
	}
      this_dest = r1;
      break;

    case mode_ir:
      /* 5-bit immediate * register, e.g. trace 1,r1  */
      str = parse_exp (str, &e1);
      str = skip_space (str);
      if (e1.X_op == O_constant && *str == ',')
	{
	  int opnd1 = e1.X_add_number;

	  str = skip_space (str + 1);
	  ans = parse_gen_reg (&str, &r2);
	  if (ans < 0)
	    {
	      as_bad ("SourceA register required");
	      return;
	    }

	  /* Got two operands, assemble instruction.  */
	  if (opnd1 < 0 || opnd1 > 31)
	    {
	      as_bad ("1st operand out of range");
	    }
	  opcode |= ((opnd1 & 0x1f) << 10) | (r2 << 16);
	}
      else
	{
	  as_bad ("Immediate operand required");
	  return;
	}
      break;

    case mode_ai:
      /* register *= 16-bit unsigned immediate
         Example:
         addi  r1,123  */
      ans = parse_gen_reg (&str, &r1);
      if (ans < 0)
	{
	  as_bad ("Dest register required");
	  return;
	}
      opcode |= (r1 << 16);

      str = skip_space (str);
      if (*str != ',')
	{
	  as_bad ("immediate value missing");
	  return;
	}
      this_dest = r1;
      /* Fall through.  */

    case mode_i:
      /* MOVIL/WRTL traditionally get an implicit "%l" applied
	 to their immediate value.  For other opcodes, unless
	 the immediate value is decorated with "%u" or "%l"
	 it must be in the range 0 .. 65535.  */
      if ((opcode & 0x7fe00000) == 0x04800000
	  || (opcode & 0x7fe00000) == 0x05000000)
	reloc = BFD_RELOC_VISIUM_LO16;
      else
	reloc = BFD_RELOC_VISIUM_IM16;

      str = skip_space (str + 1);

      if (*str == '%')
	{
	  if (str[1] == 'u')
	    reloc = BFD_RELOC_VISIUM_HI16;
	  else if (str[1] == 'l')
	    reloc = BFD_RELOC_VISIUM_LO16;
	  else
	    {
	      as_bad ("bad char after %%");
	      return;
	    }

	  str += 2;
	}
      str = parse_exp (str, &e1);
      if (e1.X_op != O_absent)
	{
	  if (e1.X_op == O_constant)
	    {
	      int imm = e1.X_add_number;

	      if (reloc == BFD_RELOC_VISIUM_HI16)
		opcode |= ((imm >> 16) & 0xffff);
	      else if (reloc == BFD_RELOC_VISIUM_LO16)
		opcode |= (imm & 0xffff);
	      else
		{
		  if (imm < 0 || imm > 0xffff)
		    as_bad ("immediate value out of range");

		  opcode |= (imm & 0xffff);
		}
	      /* No relocation is needed.  */
	      reloc = 0;
	    }
	}
      else
	{
	  as_bad ("immediate value missing");
	  return;
	}
      break;

    case mode_bax:
      /* register * register * 5-bit immediate,
         SourceB * SourceA * Index
         Examples
         write.l (r1),r2
         write.l 3(r1),r2  */
      str = skip_space (str);

      indx = 0;
      if (*str != '(')
	{
	  str = parse_exp (str, &e1);
	  if (e1.X_op == O_constant)
	    {
	      indx = e1.X_add_number;

	      if (indx < 0 || indx > 31)
		{
		  as_bad ("Index out of range");
		  return;
		}
	    }
	  else
	    {
	      as_bad ("Index(SourceA) required");
	      return;
	    }
	}

      str = skip_space (str);

      if (*str != '(')
	{
	  as_bad ("Index(SourceA) required");
	  return;
	}

      str = skip_space (str + 1);

      ans = parse_gen_reg (&str, &r1);
      if (ans < 0)
	{
	  as_bad ("SourceA register required");
	  return;
	}
      str = skip_space (str);
      if (*str != ')')
	{
	  as_bad ("(SourceA) required");
	  return;
	}
      str = skip_space (str + 1);

      if (*str == ',')
	{
	  str = skip_space (str + 1);
	  ans = parse_gen_reg (&str, &r2);
	  if (ans < 0)
	    {
	      as_bad ("SourceB register required");
	      return;
	    }
	}
      else
	{
	  as_bad ("SourceB register required");
	  return;
	}

      opcode |= (r1 << 16) | (r2 << 4) | ((indx & 0x1f) << 10);

      if (indx != 0 && previous_mode == mode_cad)
	{
	  /* We're in a delay slot.
	     If the base reg is the destination of the branch, then issue
	     an error message.
	     Otherwise it is safe to use the base and index.  */
	  if (previous_dest != 0 && r1 == previous_dest)
	    {
	      as_bad ("base register not ready");
	      return;
	    }
	}
      else if (previous_dest != 0
	       && r1 == previous_dest
	       && (visium_arch == VISIUM_ARCH_MCM
		   || visium_arch == VISIUM_ARCH_MCM24
		   || (visium_arch == VISIUM_ARCH_DEF && indx != 0)))
	{
	  as_warn ("base register not ready, NOP inserted.");
	  /* Insert a NOP before the write instruction.  */
	  output = frag_more (4);
	  memset (output, 0, 4);
	}
      break;

    case mode_dax:
      /*  register := register * 5-bit immediate
         Examples:
         read.b  r1,(r2)
         read.w  r1,3(r2)  */
      ans = parse_gen_reg (&str, &r1);
      if (ans < 0)
	{
	  as_bad ("Dest register required");
	  return;
	}
      str = skip_space (str);
      if (*str != ',')
	{
	  as_bad ("SourceA required");
	  return;
	}
      str = skip_space (str + 1);

      indx = 0;
      if (*str != '(')
	{
	  str = parse_exp (str, &e1);
	  if (e1.X_op == O_constant)
	    {
	      indx = e1.X_add_number;

	      if (indx < 0 || indx > 31)
		{
		  as_bad ("Index out of range");
		  return;
		}
	    }
	  else
	    {
	      as_bad ("Immediate 0 to 31 required");
	      return;
	    }
	}
      if (*str != '(')
	{
	  as_bad ("(SourceA) required");
	  return;
	}
      str++;
      ans = parse_gen_reg (&str, &r2);
      if (ans < 0)
	{
	  as_bad ("SourceA register required");
	  return;
	}
      str = skip_space (str);
      if (*str != ')')
	{
	  as_bad ("(SourceA) required");
	  return;
	}
      str++;
      opcode |= (r1 << 10) | (r2 << 16) | ((indx & 0x1f) << 4);
      this_dest = r1;

      if (indx != 0 && previous_mode == mode_cad)
	{
	  /* We're in a delay slot.
	     If the base reg is the destination of the branch, then issue
	     an error message.
	     Otherwise it is safe to use the base and index.  */
	  if (previous_dest != 0 && r2 == previous_dest)
	    {
	      as_bad ("base register not ready");
	      return;
	    }
	}
      else if (previous_dest != 0
	       && r2 == previous_dest
	       && (visium_arch == VISIUM_ARCH_MCM
		   || visium_arch == VISIUM_ARCH_MCM24
		   || (visium_arch == VISIUM_ARCH_DEF && indx != 0)))
	{
	  as_warn ("base register not ready, NOP inserted.");
	  /* Insert a NOP before the read instruction.  */
	  output = frag_more (4);
	  memset (output, 0, 4);
	}
      break;

    case mode_s:
      /* special mode
         Example:
         nop  */
      str = skip_space (str);
      break;

    case mode_ci:
      /* condition * 16-bit signed word displacement
         Example:
         brr L1  */
      ans = parse_cc (&str, &cc);
      if (ans < 0)
	{
	  as_bad ("condition code required");
	  return;
	}
      opcode |= (cc << 27);

      str = skip_space (str);
      if (*str == ',')
	{
	  str = skip_space (str + 1);
	  str = parse_exp (str, &e1);
	  if (e1.X_op != O_absent)
	    {
	      if (e1.X_op == O_constant)
		{
		  int imm = e1.X_add_number;

		  if (imm < -32768 || imm > 32767)
		    as_bad ("immediate value out of range");

		  /* The GR6 doesn't correctly handle a 0 displacement
		     so we insert a NOP and change it to -1.  */
		  if (imm == 0 && cc != 0 && visium_arch == VISIUM_ARCH_GR6)
		    {
		      output = frag_more (4);
		      memset (output, 0, 4);
		      imm = -1;
		    }

		  opcode |= (imm & 0xffff);
		}
	      else if (e1.X_op == O_symbol)
		{
		  /* The GR6 doesn't correctly handle a 0 displacement
		     so the instruction requires relaxation.  */
		  if (cc != 0 && visium_arch == VISIUM_ARCH_GR6)
		    relax = amode;
		  else
		    reloc = BFD_RELOC_VISIUM_REL16;
		}
	      else
		{
		  as_bad ("immediate value missing");
		  return;
		}
	    }
	  else
	    {
	      as_bad ("immediate value missing");
	      return;
	    }
	}
      else
	{
	  as_bad ("immediate value missing");
	  return;
	}

      if (previous_mode == mode_cad || previous_mode == mode_ci)
	as_bad ("branch instruction in delay slot");

      condition_code = cc;
      break;

    case mode_fdab:
      /* float := float * float
         Example
         fadd    f4,f3,f2  */
      ans = parse_fp_reg (&str, &r1);
      if (ans < 0)
	{
	  as_bad ("floating point destination register required");
	  return;
	}
      str = skip_space (str);
      if (*str == ',')
	{
	  str++;
	  ans = parse_fp_reg (&str, &r2);
	  if (ans < 0)
	    {
	      as_bad ("floating point SourceA register required");
	      return;
	    }
	  str = skip_space (str);
	  if (*str == ',')
	    {
	      str++;
	      ans = parse_fp_reg (&str, &r3);
	      if (ans < 0)
		{
		  as_bad ("floating point SourceB register required");
		  return;
		}

	      /* Got 3 floating regs, assemble instruction.  */
	      opcode |= (r1 << 10) | (r2 << 16) | (r3 << 4);
	    }
	  else
	    {
	      as_bad ("floating point SourceB register required");
	      return;
	    }
	}
      else
	{
	  as_bad ("floating point SourceA register required");
	  return;
	}
      break;

    case mode_ifdab:
      /* 4-bit immediate * float * float * float
         Example
         fpinst   10,f1,f2,f3  */
      str = parse_exp (str, &e1);
      str = skip_space (str);
      if (e1.X_op != O_absent && *str == ',')
	{
	  int finst = e1.X_add_number;

	  str = skip_space (str + 1);
	  ans = parse_fp_reg (&str, &r1);
	  if (ans < 0)
	    {
	      as_bad ("floating point destination register required");
	      return;
	    }
	  str = skip_space (str);
	  if (*str == ',')
	    {
	      str++;
	      ans = parse_fp_reg (&str, &r2);
	      if (ans < 0)
		{
		  as_bad ("floating point SourceA register required");
		  return;
		}
	      str = skip_space (str);
	      if (*str == ',')
		{
		  str++;
		  ans = parse_fp_reg (&str, &r3);
		  if (ans < 0)
		    {
		      as_bad ("floating point SourceB register required");
		      return;
		    }

		  /* Got immediate and 3 floating regs,
		     assemble instruction.  */
		  if (finst < 0 || finst > 15)
		    as_bad ("finst out of range");

		  opcode |=
		    ((finst & 0xf) << 27) | (r1 << 10) | (r2 << 16) | (r3 <<
								       4);
		}
	      else
		{
		  as_bad ("floating point SourceB register required");
		  return;
		}
	    }
	  else
	    {
	      as_bad ("floating point SourceA register required");
	      return;
	    }
	}
      else
	{
	  as_bad ("finst missing");
	  return;
	}
      break;

    case mode_idfab:
      /* 4-bit immediate * register * float * float
         Example
         fpuread   4,r25,f2,f3  */
      str = parse_exp (str, &e1);
      str = skip_space (str);
      if (e1.X_op != O_absent && *str == ',')
	{
	  int finst = e1.X_add_number;

	  str = skip_space (str + 1);
	  ans = parse_gen_reg (&str, &r1);
	  if (ans < 0)
	    {
	      as_bad ("destination general register required");
	      return;
	    }
	  str = skip_space (str);
	  if (*str == ',')
	    {
	      str++;
	      ans = parse_fp_reg (&str, &r2);
	      if (ans < 0)
		{
		  as_bad ("floating point SourceA register required");
		  return;
		}
	      str = skip_space (str);
	      if (*str == ',')
		{
		  str++;
		  ans = parse_fp_reg (&str, &r3);
		  if (ans < 0)
		    {
		      as_bad ("floating point SourceB register required");
		      return;
		    }

		  /* Got immediate and 3 floating regs,
		     assemble instruction.  */
		  if (finst < 0 || finst > 15)
		    as_bad ("finst out of range");

		  opcode |=
		    ((finst & 0xf) << 27) | (r1 << 10) | (r2 << 16) | (r3 <<
								       4);
		}
	      else
		{
		  as_bad ("floating point SourceB register required");
		  return;
		}
	    }
	  else
	    {
	      as_bad ("floating point SourceA register required");
	      return;
	    }
	}
      else
	{
	  as_bad ("finst missing");
	  return;
	}
      break;

    case mode_fda:
      /* float := float
         Example
         fsqrt    f4,f3  */
      ans = parse_fp_reg (&str, &r1);
      if (ans < 0)
	{
	  as_bad ("floating point destination register required");
	  return;
	}
      str = skip_space (str);
      if (*str == ',')
	{
	  str++;
	  ans = parse_fp_reg (&str, &r2);
	  if (ans < 0)
	    {
	      as_bad ("floating point source register required");
	      return;
	    }

	  /* Got 2 floating regs, assemble instruction.  */
	  opcode |= (r1 << 10) | (r2 << 16);
	}
      else
	{
	  as_bad ("floating point source register required");
	  return;
	}
      break;

    case mode_fdra:
      /* float := register
         Example
         fload   f15,r6  */
      ans = parse_fp_reg (&str, &r1);
      if (ans < 0)
	{
	  as_bad ("floating point destination register required");
	  return;
	}
      str = skip_space (str);
      if (*str == ',')
	{
	  str++;
	  ans = parse_gen_reg (&str, &r2);
	  if (ans < 0)
	    {
	      as_bad ("SourceA general register required");
	      return;
	    }

	  /* Got 2 regs, assemble instruction.  */
	  opcode |= (r1 << 10) | (r2 << 16);
	}
      else
	{
	  as_bad ("SourceA general register required");
	  return;
	}
      break;

    case mode_rdfab:
      /* register := float * float
         Example
         fcmp    r0,f4,f8
         For the GR6, register must be r0 and can be omitted.  */
      ans = parse_gen_reg (&str, &r1);
      if (ans < 0)
	{
	  if (visium_opcode_arch == VISIUM_OPCODE_ARCH_GR5)
	    {
	      as_bad ("Dest general register required");
	      return;
	    }
	  r1 = 0;
	}
      else
	{
	  if (r1 != 0 && visium_opcode_arch != VISIUM_OPCODE_ARCH_GR5)
	    {
	      as_bad ("FCMP/FCMPE can only use r0 as Dest register");
	      return;
	     }

	  str = skip_space (str);
	  if (*str == ',')
	    str++;
	  else
	    {
	      as_bad ("floating point SourceA register required");
	      return;
	    }
	}

      ans = parse_fp_reg (&str, &r2);
      if (ans < 0)
	{
	  as_bad ("floating point SourceA register required");
	  return;
	}
      str = skip_space (str);
      if (*str == ',')
	{
	  str++;
	  ans = parse_fp_reg (&str, &r3);
	  if (ans < 0)
	    {
	      as_bad ("floating point SourceB register required");
	      return;
	    }

	  /* Got 3 regs, assemble instruction.  */
	  opcode |= (r1 << 10) | (r2 << 16) | (r3 << 4);
	}

      this_dest = r1;
      break;

    case mode_rdfa:
      /* register := float
         Example
         fstore r5,f12  */
      ans = parse_gen_reg (&str, &r1);
      if (ans < 0)
	{
	  as_bad ("Dest general register required");
	  return;
	}
      str = skip_space (str);
      if (*str == ',')
	{
	  str++;
	  ans = parse_fp_reg (&str, &r2);
	  if (ans < 0)
	    {
	      as_bad ("floating point source register required");
	      return;
	    }

	  /* Got 2 regs, assemble instruction.  */
	  opcode |= (r1 << 10) | (r2 << 16);
	}
      else
	{
	  as_bad ("floating point source register required");
	  return;
	}

      this_dest = r1;
      break;

    case mode_rrr:
      /* register register register, all sources and destinations
         Example:
         bmd   r1,r2,r3  */

      ans = parse_gen_reg (&str, &r1);
      if (ans < 0)
	{
	  as_bad ("destination address register required");
	  return;
	}
      str = skip_space (str);
      if (*str == ',')
	{
	  str++;
	  ans = parse_gen_reg (&str, &r2);
	  if (ans < 0)
	    {
	      as_bad ("source address register required");
	      return;
	    }
	  str = skip_space (str);
	  if (*str == ',')
	    {
	      str++;
	      ans = parse_gen_reg (&str, &r3);
	      if (ans < 0)
		{
		  as_bad ("count register required");
		  return;
		}

	      /* We insist on three registers but the opcode can only use
		 r1,r2,r3.  */
	      if (r1 != 1 || r2 != 2 || r3 != 3)
		{
		  as_bad ("BMI/BMD can only use format op r1,r2,r3");
		  return;
		}

	      /* Opcode is unmodified by what comes out of the table.  */
	    }
	  else
	    {
	      as_bad ("register required");
	      return;
	    }
	}
      else
	{
	  as_bad ("register required");
	  return;
	}

      this_dest = r1;
      break;

    default:
      break;
    }

  if (relax)
    output = frag_var (rs_machine_dependent, 8, 4, relax, e1.X_add_symbol,
		       e1.X_add_number, NULL);
  else
    output = frag_more (4);

  /* Build the 32-bit instruction in a host-endian-neutral fashion.  */
  output[0] = (opcode >> 24) & 0xff;
  output[1] = (opcode >> 16) & 0xff;
  output[2] = (opcode >> 8) & 0xff;
  output[3] = (opcode >> 0) & 0xff;

  if (relax)
    /* The size of the instruction is unknown, so tie the debug info to the
       start of the instruction.  */
    dwarf2_emit_insn (0);
  else
    {
      if (reloc)
	fix_new_exp (frag_now, output - frag_now->fr_literal, 4, &e1,
		     reloc == BFD_RELOC_VISIUM_REL16, reloc);
      else
	visium_update_parity_bit (output);

      dwarf2_emit_insn (4);
    }

  if (*str != '\0')
    as_bad ("junk after instruction");
}

void
visium_cfi_frame_initial_instructions (void)
{
  /* The CFA is in SP on function entry.  */
  cfi_add_CFA_def_cfa (23, 0);
}

int
visium_regname_to_dw2regnum (char *regname)
{
  if (!regname[0])
    return -1;

  if (regname[0] == 'f' && regname[1] == 'p' && !regname[2])
    return 22;

  if (regname[0] == 's' && regname[1] == 'p' && !regname[2])
    return 23;

  if (regname[0] == 'm' && regname[1] == 'd' && !regname[3])
    switch (regname[2])
      {
      case 'b': return 32;
      case 'a': return 33;
      case 'c': return 34;
      default : return -1;
      }

  if (regname[0] == 'f' || regname[0] == 'r')
    {
      char *p;
      unsigned int regnum = strtoul (regname + 1, &p, 10);
      if (*p)
	return -1;
      if (regnum >= (regname[0] == 'f' ? 16 : 32))
	return -1;
      if (regname[0] == 'f')
	regnum += 35;
      return regnum;
    }

  return -1;
}
