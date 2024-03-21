/* TI PRU assembler.
   Copyright (C) 2014-2023 Free Software Foundation, Inc.
   Contributed by Dimitar Dimitrov <dimitar@dinux.eu>
   Based on tc-nios2.c

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
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "as.h"
#include <stdint.h>
#include "opcode/pru.h"
#include "elf/pru.h"
#include "tc-pru.h"
#include "bfd.h"
#include "dwarf2dbg.h"
#include "subsegs.h"
#include "safe-ctype.h"
#include "dw2gencfi.h"

#ifndef OBJ_ELF
/* We are not supporting any other target so we throw a compile time error.  */
  #error "OBJ_ELF not defined"
#endif

/* This array holds the chars that always start a comment.  If the
   pre-processor is disabled, these aren't very useful.  */
const char comment_chars[] = "#;";

/* This array holds the chars that only start a comment at the beginning of
   a line.  If the line seems to have the form '# 123 filename'
   .line and .file directives will appear in the pre-processed output.  */
/* Note that input_file.c hand checks for '#' at the beginning of the
   first line of the input file.  This is because the compiler outputs
   #NO_APP at the beginning of its output.  */
/* Also note that C style comments are always supported.  */
const char line_comment_chars[] = "#;*";

/* This array holds machine specific line separator characters.  */
const char line_separator_chars[] = "";

/* Chars that can be used to separate mant from exp in floating point nums.  */
const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant.
   As in 0f12.456
   or	 0d1.2345e12  */
const char FLT_CHARS[] = "rRsSfFdDxXpP";

/* Machine-dependent command-line options.  */

struct pru_opt_s
{
  /* -mno-link-relax / -mlink-relax: generate (or not)
     relocations for linker relaxation.  */
  bool link_relax;

  /* -mno-warn-regname-label: do not output a warning that a label name
     matches a register name.  */
  bool warn_regname_label;
};

static struct pru_opt_s pru_opt = { true, true };

const char *md_shortopts = "r";

enum options
{
  OPTION_LINK_RELAX = OPTION_MD_BASE + 1,
  OPTION_NO_LINK_RELAX,
  OPTION_NO_WARN_REGNAME_LABEL,
};

struct option md_longopts[] = {
  { "mlink-relax",  no_argument, NULL, OPTION_LINK_RELAX  },
  { "mno-link-relax",  no_argument, NULL, OPTION_NO_LINK_RELAX  },
  { "mno-warn-regname-label",  no_argument, NULL,
    OPTION_NO_WARN_REGNAME_LABEL  },
  { NULL, no_argument, NULL, 0 }
};

size_t md_longopts_size = sizeof (md_longopts);

typedef struct pru_insn_reloc
{
  /* Any expression in the instruction is parsed into this field,
     which is passed to fix_new_exp () to generate a fixup.  */
  expressionS reloc_expression;

  /* The type of the relocation to be applied.  */
  bfd_reloc_code_real_type reloc_type;

  /* PC-relative.  */
  unsigned int reloc_pcrel;

  /* The next relocation to be applied to the instruction.  */
  struct pru_insn_reloc *reloc_next;
} pru_insn_relocS;

/* This struct is used to hold state when assembling instructions.  */
typedef struct pru_insn_info
{
  /* Assembled instruction.  */
  unsigned long insn_code;
  /* Used for assembling LDI32.  */
  unsigned long ldi32_imm32;

  /* Pointer to the relevant bit of the opcode table.  */
  const struct pru_opcode *insn_pru_opcode;
  /* After parsing ptrs to the tokens in the instruction fill this array
     it is terminated with a null pointer (hence the first +1).
     The second +1 is because in some parts of the code the opcode
     is not counted as a token, but still placed in this array.  */
  const char *insn_tokens[PRU_MAX_INSN_TOKENS + 1 + 1];

  /* This holds information used to generate fixups
     and eventually relocations if it is not null.  */
  pru_insn_relocS *insn_reloc;
} pru_insn_infoS;

/* Opcode hash table.  */
static htab_t pru_opcode_hash = NULL;
#define pru_opcode_lookup(NAME) \
  ((struct pru_opcode *) str_hash_find (pru_opcode_hash, (NAME)))

/* Register hash table.  */
static htab_t pru_reg_hash = NULL;
#define pru_reg_lookup(NAME) \
  ((struct pru_reg *) str_hash_find (pru_reg_hash, (NAME)))

/* The known current alignment of the current section.  */
static int pru_current_align;
static segT pru_current_align_seg;

static int pru_auto_align_on = 1;

/* The last seen label in the current section.  This is used to auto-align
   labels preceding instructions.  */
static symbolS *pru_last_label;


/** Utility routines.  */
/* Function md_chars_to_number takes the sequence of
   bytes in buf and returns the corresponding value
   in an int.  n must be 1, 2, 4 or 8.  */
static uint64_t
md_chars_to_number (char *buf, int n)
{
  int i;
  uint64_t val;

  gas_assert (n == 1 || n == 2 || n == 4 || n == 8);

  val = 0;
  for (i = 0; i < n; ++i)
    val = val | ((buf[i] & 0xff) << 8 * i);
  return val;
}


/* This function turns a C long int, short int or char
   into the series of bytes that represent the number
   on the target machine.  */
void
md_number_to_chars (char *buf, valueT val, int n)
{
  gas_assert (n == 1 || n == 2 || n == 4 || n == 8);
  number_to_chars_littleendian (buf, val, n);
}

/* Turn a string in input_line_pointer into a floating point constant
   of type TYPE, and store the appropriate bytes in *LITP.  The number
   of LITTLENUMS emitted is stored in *SIZEP.  An error message is
   returned, or NULL on OK.  */
const char *
md_atof (int type, char *litP, int *sizeP)
{
  return ieee_md_atof (type, litP, sizeP, false);
}

/* Return true if STR starts with PREFIX, which should be a string literal.  */
#define strprefix(STR, PREFIX) \
  (strncmp ((STR), PREFIX, strlen (PREFIX)) == 0)

/* nop fill pattern for text section.  */
static char const nop[4] = { 0xe0, 0xe0, 0xe0, 0x12 };

/* Handles all machine-dependent alignment needs.  */
static void
pru_align (int log_size, const char *pfill, symbolS *label)
{
  int align;
  long max_alignment = 15;

  /* The front end is prone to changing segments out from under us
     temporarily when -g is in effect.  */
  int switched_seg_p = (pru_current_align_seg != now_seg);

  align = log_size;
  if (align > max_alignment)
    {
      align = max_alignment;
      as_bad (_("Alignment too large: %d assumed"), align);
    }
  else if (align < 0)
    {
      as_warn (_("Alignment negative: 0 assumed"));
      align = 0;
    }

  if (align != 0)
    {
      if (subseg_text_p (now_seg) && align >= 2)
	{
	  /* First, make sure we're on a four-byte boundary, in case
	     someone has been putting .byte values the text section.  */
	  if (pru_current_align < 2 || switched_seg_p)
	    frag_align (2, 0, 0);

	  /* Now fill in the alignment pattern.  */
	  if (pfill != NULL)
	    frag_align_pattern (align, pfill, sizeof nop, 0);
	  else
	    frag_align (align, 0, 0);
	}
      else
	frag_align (align, 0, 0);

      if (!switched_seg_p)
	pru_current_align = align;

      /* If the last label was in a different section we can't align it.  */
      if (label != NULL && !switched_seg_p)
	{
	  symbolS *sym;
	  int label_seen = false;
	  struct frag *old_frag;
	  valueT old_value;
	  valueT new_value;

	  gas_assert (S_GET_SEGMENT (label) == now_seg);

	  old_frag = symbol_get_frag (label);
	  old_value = S_GET_VALUE (label);
	  new_value = (valueT) frag_now_fix ();

	  /* It is possible to have more than one label at a particular
	     address, especially if debugging is enabled, so we must
	     take care to adjust all the labels at this address in this
	     fragment.  To save time we search from the end of the symbol
	     list, backwards, since the symbols we are interested in are
	     almost certainly the ones that were most recently added.
	     Also to save time we stop searching once we have seen at least
	     one matching label, and we encounter a label that is no longer
	     in the target fragment.  Note, this search is guaranteed to
	     find at least one match when sym == label, so no special case
	     code is necessary.  */
	  for (sym = symbol_lastP; sym != NULL; sym = symbol_previous (sym))
	    if (symbol_get_frag (sym) == old_frag
		&& S_GET_VALUE (sym) == old_value)
	      {
		label_seen = true;
		symbol_set_frag (sym, frag_now);
		S_SET_VALUE (sym, new_value);
	      }
	    else if (label_seen && symbol_get_frag (sym) != old_frag)
	      break;
	}
      record_alignment (now_seg, align);
    }
}


/** Support for self-check mode.  */

/* Mode of the assembler.  */
typedef enum
{
  PRU_MODE_ASSEMBLE,		/* Ordinary operation.  */
  PRU_MODE_TEST		/* Hidden mode used for self testing.  */
} PRU_MODE;

static PRU_MODE pru_mode = PRU_MODE_ASSEMBLE;

/* This function is used to in self-checking mode
   to check the assembled instruction.
   OPCODE should be the assembled opcode, and exp_opcode
   the parsed string representing the expected opcode.  */

static void
pru_check_assembly (unsigned int opcode, const char *exp_opcode)
{
  if (pru_mode == PRU_MODE_TEST)
    {
      if (exp_opcode == NULL)
	as_bad (_("expecting opcode string in self test mode"));
      else if (opcode != strtoul (exp_opcode, NULL, 16))
	as_bad (_("assembly 0x%08x, expected %s"), opcode, exp_opcode);
    }
}


/** Support for machine-dependent assembler directives.  */
/* Handle the .align pseudo-op.  This aligns to a power of two.  It
   also adjusts any current instruction label.  We treat this the same
   way the MIPS port does: .align 0 turns off auto alignment.  */
static void
s_pru_align (int ignore ATTRIBUTE_UNUSED)
{
  int align;
  char fill;
  const char *pfill = NULL;
  long max_alignment = 15;

  align = get_absolute_expression ();
  if (align > max_alignment)
    {
      align = max_alignment;
      as_bad (_("Alignment too large: %d assumed"), align);
    }
  else if (align < 0)
    {
      as_warn (_("Alignment negative: 0 assumed"));
      align = 0;
    }

  if (*input_line_pointer == ',')
    {
      input_line_pointer++;
      fill = get_absolute_expression ();
      pfill = (const char *) &fill;
    }
  else if (subseg_text_p (now_seg))
    pfill = (const char *) &nop;
  else
    {
      pfill = NULL;
      pru_last_label = NULL;
    }

  if (align != 0)
    {
      pru_auto_align_on = 1;
      pru_align (align, pfill, pru_last_label);
      pru_last_label = NULL;
    }
  else
    pru_auto_align_on = 0;

  demand_empty_rest_of_line ();
}

/* Handle the .text pseudo-op.  This is like the usual one, but it
   clears the saved last label and resets known alignment.  */
static void
s_pru_text (int i)
{
  s_text (i);
  pru_last_label = NULL;
  pru_current_align = 0;
  pru_current_align_seg = now_seg;
}

/* Handle the .data pseudo-op.  This is like the usual one, but it
   clears the saved last label and resets known alignment.  */
static void
s_pru_data (int i)
{
  s_data (i);
  pru_last_label = NULL;
  pru_current_align = 0;
  pru_current_align_seg = now_seg;
}

/* Handle the .section pseudo-op.  This is like the usual one, but it
   clears the saved last label and resets known alignment.  */
static void
s_pru_section (int ignore)
{
  obj_elf_section (ignore);
  pru_last_label = NULL;
  pru_current_align = 0;
  pru_current_align_seg = now_seg;
}

/* Explicitly unaligned cons.  */
static void
s_pru_ucons (int nbytes)
{
  int hold;
  hold = pru_auto_align_on;
  pru_auto_align_on = 0;
  cons (nbytes);
  pru_auto_align_on = hold;
}

/* .set sets assembler options.  */
static void
s_pru_set (int equiv)
{
  char *save = input_line_pointer;
  char *directive;
  char delim = get_symbol_name (&directive);
  char *endline = input_line_pointer;

  (void) restore_line_pointer (delim);

  /* We only want to handle ".set XXX" if the
     user has tried ".set XXX, YYY" they are not
     trying a directive.  This prevents
     us from polluting the name space.  */
  SKIP_WHITESPACE ();
  if (is_end_of_line[(unsigned char) *input_line_pointer])
    {
      bool done = true;
      *endline = 0;

      if (!strcmp (directive, "no_warn_regname_label"))
	  pru_opt.warn_regname_label = false;
      else
	done = false;

      if (done)
	{
	  *endline = delim;
	  demand_empty_rest_of_line ();
	  return;
	}
    }

  /* If we fall through to here, either we have ".set XXX, YYY"
     or we have ".set XXX" where XXX is unknown or we have
     a syntax error.  */
  input_line_pointer = save;
  s_set (equiv);
}

/* Machine-dependent assembler directives.
   Format of each entry is:
   { "directive", handler_func, param }	 */
const pseudo_typeS md_pseudo_table[] = {
  {"align", s_pru_align, 0},
  {"text", s_pru_text, 0},
  {"data", s_pru_data, 0},
  {"section", s_pru_section, 0},
  {"section.s", s_pru_section, 0},
  {"sect", s_pru_section, 0},
  {"sect.s", s_pru_section, 0},
  /* .dword and .half are included for compatibility with MIPS.  */
  {"dword", cons, 8},
  {"half", cons, 2},
  /* PRU native word size is 4 bytes, so we override
     the GAS default of 2.  */
  {"word", cons, 4},
  /* Explicitly unaligned directives.  */
  {"2byte", s_pru_ucons, 2},
  {"4byte", s_pru_ucons, 4},
  {"8byte", s_pru_ucons, 8},
  {"16byte", s_pru_ucons, 16},
  {"set", s_pru_set, 0},
  {NULL, NULL, 0}
};


int
md_estimate_size_before_relax (fragS *fragp ATTRIBUTE_UNUSED,
			       asection *seg ATTRIBUTE_UNUSED)
{
  abort ();
  return 0;
}

void
md_convert_frag (bfd *headers ATTRIBUTE_UNUSED, segT segment ATTRIBUTE_UNUSED,
		 fragS *fragp ATTRIBUTE_UNUSED)
{
  abort ();
}


static bool
relaxable_section (asection *sec)
{
  return ((sec->flags & SEC_DEBUGGING) == 0
	  && (sec->flags & SEC_CODE) != 0
	  && (sec->flags & SEC_ALLOC) != 0);
}

/* Does whatever the xtensa port does.  */
int
pru_validate_fix_sub (fixS *fix)
{
  segT add_symbol_segment, sub_symbol_segment;

  /* The difference of two symbols should be resolved by the assembler when
     linkrelax is not set.  If the linker may relax the section containing
     the symbols, then an Xtensa DIFF relocation must be generated so that
     the linker knows to adjust the difference value.  */
  if (!linkrelax || fix->fx_addsy == NULL)
    return 0;

  /* Make sure both symbols are in the same segment, and that segment is
     "normal" and relaxable.  If the segment is not "normal", then the
     fix is not valid.  If the segment is not "relaxable", then the fix
     should have been handled earlier.  */
  add_symbol_segment = S_GET_SEGMENT (fix->fx_addsy);
  if (! SEG_NORMAL (add_symbol_segment)
      || ! relaxable_section (add_symbol_segment))
    return 0;

  sub_symbol_segment = S_GET_SEGMENT (fix->fx_subsy);
  return (sub_symbol_segment == add_symbol_segment);
}

/* TC_FORCE_RELOCATION hook.  */

/* If linkrelax is turned on, and the symbol to relocate
   against is in a relaxable segment, don't compute the value -
   generate a relocation instead.  */
int
pru_force_relocation (fixS *fix)
{
  if (linkrelax && fix->fx_addsy
      && relaxable_section (S_GET_SEGMENT (fix->fx_addsy)))
    return 1;

  return generic_force_reloc (fix);
}



/** Fixups and overflow checking.  */

/* Check a fixup for overflow.  */
static bfd_reloc_status_type
pru_check_overflow (valueT fixup, reloc_howto_type *howto)
{
  bfd_reloc_status_type ret;

  ret = bfd_check_overflow (howto->complain_on_overflow,
			    howto->bitsize,
			    howto->rightshift,
			    bfd_get_reloc_size (howto) * 8,
			    fixup);

  return ret;
}

/* Emit diagnostic for fixup overflow.  */
static void
pru_diagnose_overflow (valueT fixup, reloc_howto_type *howto,
			 fixS *fixP, valueT value)
{
  if (fixP->fx_r_type == BFD_RELOC_8
      || fixP->fx_r_type == BFD_RELOC_16
      || fixP->fx_r_type == BFD_RELOC_32)
    /* These relocs are against data, not instructions.  */
    as_bad_where (fixP->fx_file, fixP->fx_line,
		  _("immediate value 0x%x truncated to 0x%x"),
		  (unsigned int) fixup,
		  (unsigned int) (~(~(valueT) 0 << howto->bitsize) & fixup));
  else
    {
      /* What opcode is the instruction?  This will determine
	 whether we check for overflow in immediate values
	 and what error message we get.  */
      const struct pru_opcode *opcode;
      enum overflow_type overflow_msg_type;
      unsigned int range_min;
      unsigned int range_max;
      unsigned int address;
      gas_assert (fixP->fx_size == 4);
      opcode = pru_find_opcode (value);
      gas_assert (opcode);
      overflow_msg_type = opcode->overflow_msg;
      switch (overflow_msg_type)
	{
	case call_target_overflow:
	  range_min
	    = ((fixP->fx_frag->fr_address + fixP->fx_where) & 0xf0000000);
	  range_max = range_min + 0x0fffffff;
	  address = fixup | range_min;

	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("call target address 0x%08x out of range 0x%08x to 0x%08x"),
			address, range_min, range_max);
	  break;
	case qbranch_target_overflow:
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("quick branch offset %d out of range %d to %d"),
			(int)fixup, -((1<<9) * 4), (1 << 9) * 4);
	  break;
	case address_offset_overflow:
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("%s offset %d out of range %d to %d"),
			opcode->name, (int)fixup, -32768, 32767);
	  break;
	case signed_immed16_overflow:
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("immediate value %d out of range %d to %d"),
			(int)fixup, -32768, 32767);
	  break;
	case unsigned_immed32_overflow:
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("immediate value %llu out of range %u to %lu"),
			(unsigned long long)fixup, 0, 0xfffffffflu);
	  break;
	case unsigned_immed16_overflow:
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("immediate value %u out of range %u to %u"),
			(unsigned int)fixup, 0, 65535);
	  break;
	case unsigned_immed5_overflow:
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("immediate value %u out of range %u to %u"),
			(unsigned int)fixup, 0, 31);
	  break;
	default:
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("overflow in immediate argument"));
	  break;
	}
    }
}

/* Apply a fixup to the object file.  */
void
md_apply_fix (fixS *fixP, valueT *valP, segT seg ATTRIBUTE_UNUSED)
{
  unsigned char *where;
  valueT value = *valP;

  /* Assert that the fixup is one we can handle.  */
  gas_assert (fixP != NULL && valP != NULL
	      && (fixP->fx_r_type == BFD_RELOC_8
		  || fixP->fx_r_type == BFD_RELOC_16
		  || fixP->fx_r_type == BFD_RELOC_32
		  || fixP->fx_r_type == BFD_RELOC_64
		  || fixP->fx_r_type == BFD_RELOC_PRU_LDI32
		  || fixP->fx_r_type == BFD_RELOC_PRU_U16
		  || fixP->fx_r_type == BFD_RELOC_PRU_U16_PMEMIMM
		  || fixP->fx_r_type == BFD_RELOC_PRU_S10_PCREL
		  || fixP->fx_r_type == BFD_RELOC_PRU_U8_PCREL
		  || fixP->fx_r_type == BFD_RELOC_PRU_32_PMEM
		  || fixP->fx_r_type == BFD_RELOC_PRU_16_PMEM
		  /* Add other relocs here as we generate them.  */
	      ));

  if (fixP->fx_r_type == BFD_RELOC_64)
    {
      /* We may reach here due to .8byte directives, but we never output
	 BFD_RELOC_64; it must be resolved.  */
      if (fixP->fx_addsy != NULL)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("cannot create 64-bit relocation"));
      else
	{
	  md_number_to_chars (fixP->fx_frag->fr_literal + fixP->fx_where,
			      *valP, 8);
	  fixP->fx_done = 1;
	}
      return;
    }

  /* gas_assert (had_errors () || !fixP->fx_subsy); */

  /* In general, fix instructions with immediate
     constants.  But leave LDI32 for the linker,
     which is prepared to shorten insns.  */
  if (fixP->fx_addsy == (symbolS *) NULL
      && fixP->fx_r_type != BFD_RELOC_PRU_LDI32)
    fixP->fx_done = 1;

  else if (fixP->fx_pcrel)
    {
      segT s = S_GET_SEGMENT (fixP->fx_addsy);

      if (s == seg || s == absolute_section)
	{
	  /* Blindly copied from AVR, but I don't understand why
	     this is needed in the first place.  Fail hard to catch
	     when this curious code snippet is utilized.  */
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("unexpected PC relative expression"));
	  value += S_GET_VALUE (fixP->fx_addsy);
	  fixP->fx_done = 1;
	}
    }
  else if (linkrelax && fixP->fx_subsy)
    {
      /* For a subtraction relocation expression, generate one
	 of the DIFF relocs, with the value being the difference.
	 Note that a sym1 - sym2 expression is adjusted into a
	 section_start_sym + sym4_offset_from_section_start - sym1
	 expression.  fixP->fx_addsy holds the section start symbol,
	 fixP->fx_offset holds sym2's offset, and fixP->fx_subsy
	 holds sym1.  Calculate the current difference and write value,
	 but leave fx_offset as is - during relaxation,
	 fx_offset - value gives sym1's value.  */

      offsetT diffval;	/* valueT is unsigned, so use offsetT.  */

      diffval = S_GET_VALUE (fixP->fx_addsy)
		+ fixP->fx_offset - S_GET_VALUE (fixP->fx_subsy);

      switch (fixP->fx_r_type)
	{
	case BFD_RELOC_8:
	  fixP->fx_r_type = BFD_RELOC_PRU_GNU_DIFF8;
	  break;
	case BFD_RELOC_16:
	  fixP->fx_r_type = BFD_RELOC_PRU_GNU_DIFF16;
	  break;
	case BFD_RELOC_32:
	  fixP->fx_r_type = BFD_RELOC_PRU_GNU_DIFF32;
	  break;
	case BFD_RELOC_PRU_16_PMEM:
	  fixP->fx_r_type = BFD_RELOC_PRU_GNU_DIFF16_PMEM;
	  if (diffval % 4)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("residual low bits in pmem diff relocation"));
	  diffval /= 4;
	  break;
	case BFD_RELOC_PRU_32_PMEM:
	  fixP->fx_r_type = BFD_RELOC_PRU_GNU_DIFF32_PMEM;
	  if (diffval % 4)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("residual low bits in pmem diff relocation"));
	  diffval /= 4;
	  break;
	default:
	  as_bad_subtract (fixP);
	  break;
	}

      value = *valP = diffval;

      fixP->fx_subsy = NULL;
  }
  /* We don't actually support subtracting a symbol.  */
  if (fixP->fx_subsy != (symbolS *) NULL)
    as_bad_subtract (fixP);

  /* For the DIFF relocs, write the value into the object file while still
     keeping fx_done FALSE, as both the difference (recorded in the object file)
     and the sym offset (part of fixP) are needed at link relax time.  */
  where = (unsigned char *) fixP->fx_frag->fr_literal + fixP->fx_where;
  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_PRU_GNU_DIFF8:
      *where = value;
      break;
    case BFD_RELOC_PRU_GNU_DIFF16:
    case BFD_RELOC_PRU_GNU_DIFF16_PMEM:
      bfd_putl16 ((bfd_vma) value, where);
      break;
    case BFD_RELOC_PRU_GNU_DIFF32:
    case BFD_RELOC_PRU_GNU_DIFF32_PMEM:
      bfd_putl32 ((bfd_vma) value, where);
      break;
    default:
      break;
    }

  if (fixP->fx_done)
    /* Fully resolved fixup.  */
    {
      reloc_howto_type *howto
	= bfd_reloc_type_lookup (stdoutput, fixP->fx_r_type);

      if (howto == NULL)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("relocation is not supported"));
      else
	{
	  valueT fixup = value;
	  uint64_t insn;
	  char *buf;

	  /* Get the instruction or data to be fixed up.  */
	  buf = fixP->fx_frag->fr_literal + fixP->fx_where;
	  insn = md_chars_to_number (buf, fixP->fx_size);

	  /* Check for overflow, emitting a diagnostic if necessary.  */
	  if (pru_check_overflow (fixup, howto) != bfd_reloc_ok)
	    pru_diagnose_overflow (fixup, howto, fixP, insn);

	  /* Apply the right shift.  */
	  fixup = (offsetT) fixup >> howto->rightshift;

	  /* Truncate the fixup to right size.  */
	  if (howto->bitsize == 0)
	    fixup = 0;
	  else
	    fixup &= ((valueT) 2 << (howto->bitsize - 1)) - 1;

	  /* Fix up the instruction.  Non-contiguous bitfields need
	     special handling.  */
	  if (fixP->fx_r_type == BFD_RELOC_PRU_LDI32)
	    {
	      /* As the only 64-bit "insn", LDI32 needs special handling. */
	      uint32_t insn1 = insn & 0xffffffff;
	      uint32_t insn2 = insn >> 32;
	      SET_INSN_FIELD (IMM16, insn1, fixup >> 16);
	      SET_INSN_FIELD (IMM16, insn2, fixup & 0xffff);

	      SET_INSN_FIELD (RDSEL, insn1, RSEL_31_16);
	      SET_INSN_FIELD (RDSEL, insn2, RSEL_15_0);

	      md_number_to_chars (buf, insn1, 4);
	      md_number_to_chars (buf + 4, insn2, 4);
	    }
	  else
	    {
	      if (fixP->fx_r_type == BFD_RELOC_PRU_S10_PCREL)
		SET_BROFF_URAW (insn, fixup);
	      else
		insn = (insn & ~howto->dst_mask) | (fixup << howto->bitpos);
	      md_number_to_chars (buf, insn, fixP->fx_size);
	    }
	}

      fixP->fx_done = 1;
    }

  if (fixP->fx_r_type == BFD_RELOC_VTABLE_INHERIT)
    {
      fixP->fx_done = 0;
      if (fixP->fx_addsy
	  && !S_IS_DEFINED (fixP->fx_addsy) && !S_IS_WEAK (fixP->fx_addsy))
	S_SET_WEAK (fixP->fx_addsy);
    }
  else if (fixP->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    fixP->fx_done = 0;
}



/** Instruction parsing support.  */

/* Creates a new pru_insn_relocS and returns a pointer to it.  */
static pru_insn_relocS *
pru_insn_reloc_new (bfd_reloc_code_real_type reloc_type, unsigned int pcrel)
{
  pru_insn_relocS *retval;
  retval = XNEW (pru_insn_relocS);
  if (retval == NULL)
    {
      as_bad (_("can't create relocation"));
      abort ();
    }

  /* Fill out the fields with default values.  */
  retval->reloc_next = NULL;
  retval->reloc_type = reloc_type;
  retval->reloc_pcrel = pcrel;
  return retval;
}

/* Frees up memory previously allocated by pru_insn_reloc_new ().  */
static void
pru_insn_reloc_destroy (pru_insn_relocS *reloc)
{
  pru_insn_relocS *next;

  while (reloc)
    {
      next = reloc->reloc_next;
      free (reloc);
      reloc = next;
    }
}

/* The various pru_assemble_* functions call this
   function to generate an expression from a string representing an expression.
   It then tries to evaluate the expression, and if it can, returns its value.
   If not, it creates a new pru_insn_relocS and stores the expression and
   reloc_type for future use.  */
static unsigned long
pru_assemble_expression (const char *exprstr,
			   pru_insn_infoS *insn,
			   pru_insn_relocS *prev_reloc,
			   bfd_reloc_code_real_type reloc_type,
			   unsigned int pcrel)
{
  expressionS *ep;
  pru_insn_relocS *reloc;
  char *saved_line_ptr;
  unsigned short value;

  gas_assert (exprstr != NULL);
  gas_assert (insn != NULL);

  /* We use this blank keyword to distinguish register from
     label operands.  */
  if (strstr (exprstr, "%label") != NULL)
    {
      exprstr += strlen ("%label") + 1;
    }

  /* Check for pmem relocation operator.
     Change the relocation type and advance the ptr to the start of
     the expression proper.  */
  if (strstr (exprstr, "%pmem") != NULL)
    {
      reloc_type = BFD_RELOC_PRU_U16_PMEMIMM;
      exprstr += strlen ("%pmem") + 1;
    }

  /* We potentially have a relocation.  */
  reloc = pru_insn_reloc_new (reloc_type, pcrel);
  if (prev_reloc != NULL)
    prev_reloc->reloc_next = reloc;
  else
    insn->insn_reloc = reloc;

  /* Parse the expression string.  */
  ep = &reloc->reloc_expression;
  saved_line_ptr = input_line_pointer;
  input_line_pointer = (char *) exprstr;
  SKIP_WHITESPACE ();
  expression (ep);
  SKIP_WHITESPACE ();
  if (*input_line_pointer)
    as_bad (_("trailing garbage after expression: %s"), input_line_pointer);
  input_line_pointer = saved_line_ptr;


  if (ep->X_op == O_illegal || ep->X_op == O_absent)
    as_bad (_("expected expression, got %s"), exprstr);

  /* This is redundant as the fixup will put this into
     the instruction, but it is included here so that
     self-test mode (-r) works.  */
  value = 0;
  if (pru_mode == PRU_MODE_TEST && ep->X_op == O_constant)
    value = ep->X_add_number;

  return (unsigned long) value;
}

/* Try to parse a non-relocatable expression.  */
static unsigned long
pru_assemble_noreloc_expression (const char *exprstr)
{
  expressionS exp;
  char *saved_line_ptr;
  unsigned long val;

  gas_assert (exprstr != NULL);

  saved_line_ptr = input_line_pointer;
  input_line_pointer = (char *) exprstr;
  SKIP_WHITESPACE ();
  expression (&exp);
  SKIP_WHITESPACE ();
  if (*input_line_pointer)
    as_bad (_("trailing garbage after expression: %s"), input_line_pointer);
  input_line_pointer = saved_line_ptr;

  val = 0;
  if (exp.X_op != O_constant)
    as_bad (_("expected constant expression, got %s"), exprstr);
  else
    val = exp.X_add_number;

  return val;
}

/* Argument assemble functions.
   All take an instruction argument string, and a pointer
   to an instruction opcode.  Upon return the insn_opcode
   has the relevant fields filled in to represent the arg
   string.  The return value is NULL if successful, or
   an error message if an error was detected.  */

static void
pru_assemble_arg_d (pru_insn_infoS *insn_info, const char *argstr)
{
  struct pru_reg *dst = pru_reg_lookup (argstr);

  if (dst == NULL)
    as_bad (_("unknown register %s"), argstr);
  else
    {
      SET_INSN_FIELD (RD, insn_info->insn_code, dst->index);
      SET_INSN_FIELD (RDSEL, insn_info->insn_code, dst->regsel);
    }
}

static void
pru_assemble_arg_D (pru_insn_infoS *insn_info, const char *argstr)
{
  struct pru_reg *dst;

  /* The leading & before an address register is optional.  */
  if (*argstr == '&')
    argstr++;

  dst = pru_reg_lookup (argstr);

  if (dst == NULL)
    as_bad (_("unknown register %s"), argstr);
  else
    {
      unsigned long rxb = 0;

      switch (dst->regsel)
	{
	case RSEL_31_0: rxb = 0; break;	/* whole register defaults to .b0  */
	case RSEL_7_0: rxb = 0; break;
	case RSEL_15_8: rxb = 1; break;
	case RSEL_23_16: rxb = 2; break;
	case RSEL_31_24: rxb = 3; break;
	default:
	  as_bad (_("data transfer register cannot be halfword"));
	}

      SET_INSN_FIELD (RD, insn_info->insn_code, dst->index);
      SET_INSN_FIELD (RDB, insn_info->insn_code, rxb);
    }
}

static void
pru_assemble_arg_R (pru_insn_infoS *insn_info, const char *argstr)
{
  struct pru_reg *dst = pru_reg_lookup (argstr);

  if (dst == NULL)
    as_bad (_("unknown register %s"), argstr);
  else
    {
      if (dst->regsel != RSEL_31_0)
	{
	  as_bad (_("destination register must be full-word"));
	}

      SET_INSN_FIELD (RD, insn_info->insn_code, dst->index);
      SET_INSN_FIELD (RDSEL, insn_info->insn_code, dst->regsel);
    }
}

static void
pru_assemble_arg_s (pru_insn_infoS *insn_info, const char *argstr)
{
  struct pru_reg *src1 = pru_reg_lookup (argstr);

  if (src1 == NULL)
    as_bad (_("unknown register %s"), argstr);
  else
    {
      SET_INSN_FIELD (RS1, insn_info->insn_code, src1->index);
      SET_INSN_FIELD (RS1SEL, insn_info->insn_code, src1->regsel);
    }
}

static void
pru_assemble_arg_S (pru_insn_infoS *insn_info, const char *argstr)
{
  struct pru_reg *src1 = pru_reg_lookup (argstr);

  if (src1 == NULL)
    as_bad (_("unknown register %s"), argstr);
  else
    {
      if (src1->regsel != RSEL_31_0)
	as_bad (_("cannot use partial register %s for addressing"), argstr);
      SET_INSN_FIELD (RS1, insn_info->insn_code, src1->index);
    }
}

static void
pru_assemble_arg_b (pru_insn_infoS *insn_info, const char *argstr)
{
  struct pru_reg *src2 = pru_reg_lookup (argstr);
  if (src2 == NULL)
    {
      unsigned long imm8 = pru_assemble_noreloc_expression (argstr);
      if (imm8 >= 0x100)
	as_bad (_("value %lu is too large for a byte operand"), imm8);
      SET_INSN_FIELD (IMM8, insn_info->insn_code, imm8);
      SET_INSN_FIELD (IO, insn_info->insn_code, 1);
    }
  else
    {
      SET_INSN_FIELD (IO, insn_info->insn_code, 0);
      SET_INSN_FIELD (RS2, insn_info->insn_code, src2->index);
      SET_INSN_FIELD (RS2SEL, insn_info->insn_code, src2->regsel);
    }

}

static void
pru_assemble_arg_B (pru_insn_infoS *insn_info, const char *argstr)
{
  struct pru_reg *src2 = pru_reg_lookup (argstr);
  if (src2 == NULL)
    {
      unsigned long imm8;
      imm8 = pru_assemble_noreloc_expression (argstr);
      if (!imm8 || imm8 > 0xff)
	as_bad (_("loop count constant %ld is out of range [1..%d]"),
		imm8, 0xff);
      /* Note: HW expects the immediate loop count field
	 to be one less than the actual loop count.  */
      SET_INSN_FIELD (IMM8, insn_info->insn_code, imm8 - 1);
      SET_INSN_FIELD (IO, insn_info->insn_code, 1);
    }
  else
    {
      SET_INSN_FIELD (IO, insn_info->insn_code, 0);
      SET_INSN_FIELD (RS2, insn_info->insn_code, src2->index);
      SET_INSN_FIELD (RS2SEL, insn_info->insn_code, src2->regsel);
    }
}

static void
pru_assemble_arg_i (pru_insn_infoS *insn_info, const char *argstr)
{
  unsigned long imm32;

  /* We must not generate PRU_LDI32 relocation if relaxation is disabled in
     GAS. Consider the following scenario: GAS relaxation is disabled, so
     DIFF* expressions are fixed and not emitted as relocations. Then if LD
     has relaxation enabled, it may shorten LDI32 but will not update
     accordingly the DIFF expressions.  */
  if (pru_opt.link_relax)
    imm32 = pru_assemble_expression (argstr, insn_info,
				     insn_info->insn_reloc,
				     BFD_RELOC_PRU_LDI32, 0);
  else
    imm32 = pru_assemble_noreloc_expression (argstr);

  /* QUIRK: LDI must clear IO bit high, even though it has immediate arg. */
  SET_INSN_FIELD (IO, insn_info->insn_code, 0);
  SET_INSN_FIELD (RDSEL, insn_info->insn_code, RSEL_31_16);
  SET_INSN_FIELD (IMM16, insn_info->insn_code, imm32 >> 16);
  insn_info->ldi32_imm32 = imm32;
}

static void
pru_assemble_arg_j (pru_insn_infoS *insn_info, const char *argstr)
{
  struct pru_reg *src2 = pru_reg_lookup (argstr);

  if (src2 == NULL)
    {
      unsigned long imm16 = pru_assemble_expression (argstr, insn_info,
						     insn_info->insn_reloc,
						     BFD_RELOC_PRU_U16_PMEMIMM,
						     0);
      SET_INSN_FIELD (IMM16, insn_info->insn_code, imm16);
      SET_INSN_FIELD (IO, insn_info->insn_code, 1);
    }
  else
    {
      SET_INSN_FIELD (IO, insn_info->insn_code, 0);
      SET_INSN_FIELD (RS2, insn_info->insn_code, src2->index);
      SET_INSN_FIELD (RS2SEL, insn_info->insn_code, src2->regsel);
    }
}

static void
pru_assemble_arg_W (pru_insn_infoS *insn_info, const char *argstr)
{
  unsigned long imm16 = pru_assemble_expression (argstr, insn_info,
						 insn_info->insn_reloc,
						 BFD_RELOC_PRU_U16, 0);
  /* QUIRK: LDI must clear IO bit high, even though it has immediate arg.  */
  SET_INSN_FIELD (IO, insn_info->insn_code, 0);
  SET_INSN_FIELD (IMM16, insn_info->insn_code, imm16);
}

static void
pru_assemble_arg_o (pru_insn_infoS *insn_info, const char *argstr)
{
  unsigned long imm10 = pru_assemble_expression (argstr, insn_info,
						 insn_info->insn_reloc,
						 BFD_RELOC_PRU_S10_PCREL, 1);
  SET_BROFF_URAW (insn_info->insn_code, imm10);
}

static void
pru_assemble_arg_O (pru_insn_infoS *insn_info, const char *argstr)
{
  unsigned long imm8 = pru_assemble_expression (argstr, insn_info,
						insn_info->insn_reloc,
						BFD_RELOC_PRU_U8_PCREL, 1);
  SET_INSN_FIELD (LOOP_JMPOFFS, insn_info->insn_code, imm8);
}

static void
pru_assemble_arg_l (pru_insn_infoS *insn_info, const char *argstr)
{
  unsigned long burstlen = 0;
  struct pru_reg *blreg = pru_reg_lookup (argstr);

  if (blreg == NULL)
    {
      burstlen = pru_assemble_noreloc_expression (argstr);
      if (!burstlen || burstlen > LSSBBO_BYTECOUNT_R0_BITS7_0)
	as_bad (_("byte count constant %ld is out of range [1..%d]"),
		burstlen, LSSBBO_BYTECOUNT_R0_BITS7_0);
      burstlen--;
    }
  else
    {
      if (blreg->index != 0)
	as_bad (_("only r0 can be used as byte count register"));
      else if (blreg->regsel > RSEL_31_24)
	as_bad (_("only r0.bX byte fields of r0 can be used as byte count"));
      else
	burstlen = LSSBBO_BYTECOUNT_R0_BITS7_0 + blreg->regsel;
    }
    SET_BURSTLEN (insn_info->insn_code, burstlen);
}

static void
pru_assemble_arg_n (pru_insn_infoS *insn_info, const char *argstr)
{
  unsigned long burstlen = 0;
  struct pru_reg *blreg = pru_reg_lookup (argstr);

  if (blreg == NULL)
    {
      burstlen = pru_assemble_noreloc_expression (argstr);
      if (!burstlen || burstlen > LSSBBO_BYTECOUNT_R0_BITS7_0)
	as_bad (_("byte count constant %ld is out of range [1..%d]"),
		burstlen, LSSBBO_BYTECOUNT_R0_BITS7_0);
      burstlen--;
    }
  else
    {
      if (blreg->index != 0)
	as_bad (_("only r0 can be used as byte count register"));
      else if (blreg->regsel > RSEL_31_24)
	as_bad (_("only r0.bX byte fields of r0 can be used as byte count"));
      else
	burstlen = LSSBBO_BYTECOUNT_R0_BITS7_0 + blreg->regsel;
    }
    SET_INSN_FIELD (XFR_LENGTH, insn_info->insn_code, burstlen);
}

static void
pru_assemble_arg_c (pru_insn_infoS *insn_info, const char *argstr)
{
  unsigned long cb = pru_assemble_noreloc_expression (argstr);

  if (cb > 31)
    as_bad (_("invalid constant table offset %ld"), cb);
  else
    SET_INSN_FIELD (CB, insn_info->insn_code, cb);
}

static void
pru_assemble_arg_w (pru_insn_infoS *insn_info, const char *argstr)
{
  unsigned long wk = pru_assemble_noreloc_expression (argstr);

  if (wk != 0 && wk != 1)
    as_bad (_("invalid WakeOnStatus %ld"), wk);
  else
    SET_INSN_FIELD (WAKEONSTATUS, insn_info->insn_code, wk);
}

static void
pru_assemble_arg_x (pru_insn_infoS *insn_info, const char *argstr)
{
  unsigned long wba = pru_assemble_noreloc_expression (argstr);

  if (wba > 255)
    as_bad (_("invalid XFR WideBus Address %ld"), wba);
  else
    SET_INSN_FIELD (XFR_WBA, insn_info->insn_code, wba);
}

/* The function consume_arg takes a pointer into a string
   of instruction tokens (args) and a pointer into a string
   representing the expected sequence of tokens and separators.
   It checks whether the first argument in argstr is of the
   expected type, throwing an error if it is not, and returns
   the pointer argstr.  */
static char *
pru_consume_arg (char *argstr, const char *parsestr)
{
  char *temp;

  switch (*parsestr)
    {
    case 'W':
      if (*argstr == '%')
	{
	  if (strprefix (argstr, "%pmem") || strprefix (argstr, "%label"))
	    {
	      /* We zap the parentheses because we don't want them confused
		 with separators.  */
	      temp = strchr (argstr, '(');
	      if (temp != NULL)
		*temp = ' ';
	      temp = strchr (argstr, ')');
	      if (temp != NULL)
		*temp = ' ';
	    }
	  else
	    as_bad (_("badly formed expression near %s"), argstr);
	}
      break;

    case 'j':
    case 'o':
    case 'O':
      if (*argstr == '%')
	{
	  /* Only 'j' really requires %label for distinguishing registers
	     from labels, but we include 'o' and 'O' here to avoid
	     confusing assembler programmers. Thus for completeness all
	     jump operands can be prefixed with %label.  */
	  if (strprefix (argstr, "%label"))
	    {
	      /* We zap the parentheses because we don't want them confused
		 with separators.  */
	      temp = strchr (argstr, '(');
	      if (temp != NULL)
		*temp = ' ';
	      temp = strchr (argstr, ')');
	      if (temp != NULL)
		*temp = ' ';
	    }
	  else
	    as_bad (_("badly formed expression near %s"), argstr);
	}
      break;

    case 'b':
    case 'B':
    case 'c':
    case 'd':
    case 'D':
    case 'E':
    case 'i':
    case 's':
    case 'S':
    case 'l':
    case 'n':
    case 'R':
    case 'w':
    case 'x':
      /* We can't have %pmem here.  */
      if (*argstr == '%')
	as_bad (_("badly formed expression near %s"), argstr);
      break;
    default:
      BAD_CASE (*parsestr);
      break;
    }

  return argstr;
}

/* The function consume_separator takes a pointer into a string
   of instruction tokens (args) and a pointer into a string representing
   the expected sequence of tokens and separators.  It finds the first
   instance of the character pointed to by separator in argstr, and
   returns a pointer to the next element of argstr, which is the
   following token in the sequence.  */
static char *
pru_consume_separator (char *argstr, const char *separator)
{
  char *p;

  p = strchr (argstr, *separator);

  if (p != NULL)
    *p++ = 0;
  else
    as_bad (_("expecting %c near %s"), *separator, argstr);
  return p;
}


/* The principal argument parsing function which takes a string argstr
   representing the instruction arguments for insn, and extracts the argument
   tokens matching parsestr into parsed_args.  */
static void
pru_parse_args (pru_insn_infoS *insn ATTRIBUTE_UNUSED, char *argstr,
		  const char *parsestr, char **parsed_args)
{
  char *p;
  char *end = NULL;
  int i;
  p = argstr;
  i = 0;
  bool terminate = false;

  /* This rest of this function is it too fragile and it mostly works,
     therefore special case this one.  */
  if (*parsestr == 0 && argstr != 0)
    {
      as_bad (_("too many arguments"));
      parsed_args[0] = NULL;
      return;
    }

  while (p != NULL && !terminate && i < PRU_MAX_INSN_TOKENS)
    {
      parsed_args[i] = pru_consume_arg (p, parsestr);
      ++parsestr;
      if (*parsestr != '\0')
	{
	  p = pru_consume_separator (p, parsestr);
	  ++parsestr;
	}
      else
	{
	  /* Check that the argument string has no trailing arguments.  */
	  /* If we've got a %pmem relocation, we've zapped the parens with
	     spaces.  */
	  if (strprefix (p, "%pmem") || strprefix (p, "%label"))
	    end = strpbrk (p, ",");
	  else
	    end = strpbrk (p, " ,");

	  if (end != NULL)
	    as_bad (_("too many arguments"));
	}

      if (*parsestr == '\0' || (p != NULL && *p == '\0'))
	terminate = true;
      ++i;
    }

  parsed_args[i] = NULL;

  /* There are no instructions with optional arguments; complain.  */
  if (*parsestr != '\0')
    as_bad (_("missing argument"));
}


/** Assembler output support.  */

/* Output a normal instruction.  */
static void
output_insn (pru_insn_infoS *insn)
{
  char *f;
  pru_insn_relocS *reloc;

  f = frag_more (4);
  /* This allocates enough space for the instruction
     and puts it in the current frag.  */
  md_number_to_chars (f, insn->insn_code, 4);
  /* Emit debug info.  */
  dwarf2_emit_insn (4);
  /* Create any fixups to be acted on later.  */
  for (reloc = insn->insn_reloc; reloc != NULL; reloc = reloc->reloc_next)
    fix_new_exp (frag_now, f - frag_now->fr_literal, 4,
		 &reloc->reloc_expression, reloc->reloc_pcrel,
		 reloc->reloc_type);
}

/* Output two LDI instructions from LDI32 macro */
static void
output_insn_ldi32 (pru_insn_infoS *insn)
{
  char *f;
  pru_insn_relocS *reloc;
  unsigned long insn2;

  f = frag_more (8);
  SET_INSN_FIELD (IMM16, insn->insn_code, insn->ldi32_imm32 >> 16);
  SET_INSN_FIELD (RDSEL, insn->insn_code, RSEL_31_16);
  md_number_to_chars (f, insn->insn_code, 4);

  insn2 = insn->insn_code;
  SET_INSN_FIELD (IMM16, insn2, insn->ldi32_imm32 & 0xffff);
  SET_INSN_FIELD (RDSEL, insn2, RSEL_15_0);
  md_number_to_chars (f + 4, insn2, 4);

  /* Emit debug info.  */
  dwarf2_emit_insn (8);

  /* Create any fixups to be acted on later.  */
  for (reloc = insn->insn_reloc; reloc != NULL; reloc = reloc->reloc_next)
    fix_new_exp (frag_now, f - frag_now->fr_literal, 4,
		 &reloc->reloc_expression, reloc->reloc_pcrel,
		 reloc->reloc_type);
}


/** External interfaces.  */

/* The following functions are called by machine-independent parts of
   the assembler.  */
int
md_parse_option (int c, const char *arg ATTRIBUTE_UNUSED)
{
  switch (c)
    {
    case 'r':
      /* Hidden option for self-test mode.  */
      pru_mode = PRU_MODE_TEST;
      break;
    case OPTION_LINK_RELAX:
      pru_opt.link_relax = true;
      break;
    case OPTION_NO_LINK_RELAX:
      pru_opt.link_relax = false;
      break;
    case OPTION_NO_WARN_REGNAME_LABEL:
      pru_opt.warn_regname_label = false;
      break;
    default:
      return 0;
      break;
    }

  return 1;
}

const char *
pru_target_format (void)
{
  return "elf32-pru";
}

/* Machine-dependent usage message.  */
void
md_show_usage (FILE *stream)
{
  fprintf (stream,
    _("PRU options:\n"
      "  -mlink-relax     generate relocations for linker relaxation (default).\n"
      "  -mno-link-relax  don't generate relocations for linker relaxation.\n"
    ));

}

/* This function is called once, at assembler startup time.
   It should set up all the tables, etc.  that the MD part of the
   assembler will need.  */
void
md_begin (void)
{
  int i;

  /* Create and fill a hashtable for the PRU opcodes, registers and
     arguments.  */
  pru_opcode_hash = str_htab_create ();
  pru_reg_hash = str_htab_create ();

  for (i = 0; i < NUMOPCODES; ++i)
    if (str_hash_insert (pru_opcode_hash, pru_opcodes[i].name,
			 &pru_opcodes[i], 0) != NULL)
      as_fatal (_("duplicate %s"), pru_opcodes[i].name);

  for (i = 0; i < pru_num_regs; ++i)
    if (str_hash_insert (pru_reg_hash, pru_regs[i].name, &pru_regs[i], 0))
      as_fatal (_("duplicate %s"), pru_regs[i].name);

  linkrelax = pru_opt.link_relax;
  /* Initialize the alignment data.  */
  pru_current_align_seg = now_seg;
  pru_last_label = NULL;
  pru_current_align = 0;
}


/* Assembles a single line of PRU assembly language.  */
void
md_assemble (char *op_str)
{
  char *argstr;
  char *op_strdup = NULL;
  pru_insn_infoS thisinsn;
  pru_insn_infoS *insn = &thisinsn;

  /* Make sure we are aligned on a 4-byte boundary.  */
  if (pru_current_align < 2)
    pru_align (2, NULL, pru_last_label);
  else if (pru_current_align > 2)
    pru_current_align = 2;
  pru_last_label = NULL;

  /* We don't want to clobber to op_str
     because we want to be able to use it in messages.  */
  op_strdup = strdup (op_str);
  insn->insn_tokens[0] = strtok (op_strdup, " ");
  argstr = strtok (NULL, "");

  /* Assemble the opcode.  */
  insn->insn_pru_opcode = pru_opcode_lookup (insn->insn_tokens[0]);
  insn->insn_reloc = NULL;

  if (insn->insn_pru_opcode != NULL)
    {
      const char *argsfmt = insn->insn_pru_opcode->args;
      const char **argtk = &insn->insn_tokens[1];
      const char *argp;

      /* Set the opcode for the instruction.  */
      insn->insn_code = insn->insn_pru_opcode->match;

      if (pru_mode == PRU_MODE_TEST)
	{
	  /* Add the "expected" instruction parameter used for validation.  */
	  argsfmt = malloc (strlen (argsfmt) + 3);
	  sprintf ((char *)argsfmt, "%s,E", insn->insn_pru_opcode->args);
	}
      pru_parse_args (insn, argstr, argsfmt,
		      (char **) &insn->insn_tokens[1]);

      for (argp = argsfmt; !had_errors () && *argp && *argtk; ++argp)
	{
	  gas_assert (argtk <= &insn->insn_tokens[PRU_MAX_INSN_TOKENS]);

	  switch (*argp)
	    {
	    case ',':
	      continue;

	    case 'd':
	      pru_assemble_arg_d (insn, *argtk++);
	      continue;
	    case 'D':
	      pru_assemble_arg_D (insn, *argtk++);
	      continue;
	    case 'R':
	      pru_assemble_arg_R (insn, *argtk++);
	      continue;
	    case 's':
	      pru_assemble_arg_s (insn, *argtk++);
	      continue;
	    case 'S':
	      pru_assemble_arg_S (insn, *argtk++);
	      continue;
	    case 'b':
	      pru_assemble_arg_b (insn, *argtk++);
	      continue;
	    case 'B':
	      pru_assemble_arg_B (insn, *argtk++);
	      continue;
	    case 'i':
	      pru_assemble_arg_i (insn, *argtk++);
	      continue;
	    case 'j':
	      pru_assemble_arg_j (insn, *argtk++);
	      continue;
	    case 'W':
	      pru_assemble_arg_W (insn, *argtk++);
	      continue;
	    case 'o':
	      pru_assemble_arg_o (insn, *argtk++);
	      continue;
	    case 'O':
	      pru_assemble_arg_O (insn, *argtk++);
	      continue;
	    case 'l':
	      pru_assemble_arg_l (insn, *argtk++);
	      continue;
	    case 'n':
	      pru_assemble_arg_n (insn, *argtk++);
	      continue;
	    case 'c':
	      pru_assemble_arg_c (insn, *argtk++);
	      continue;
	    case 'w':
	      pru_assemble_arg_w (insn, *argtk++);
	      continue;
	    case 'x':
	      pru_assemble_arg_x (insn, *argtk++);
	      continue;

	    case 'E':
	      pru_check_assembly (insn->insn_code, *argtk++);
	      continue;

	    default:
	      BAD_CASE (*argp);
	    }
	}

      if (*argp && !had_errors ())
	as_bad (_("missing argument"));

      if (!had_errors ())
	{
	  if (insn->insn_pru_opcode->pinfo & PRU_INSN_LDI32)
	    {
	      output_insn_ldi32 (insn);
	    }
	  else
	    {
	      output_insn (insn);
	    }
	}

      if (pru_mode == PRU_MODE_TEST)
	free ((char *)argsfmt);
    }
  else
    /* Unrecognised instruction - error.  */
    as_bad (_("unrecognised instruction %s"), insn->insn_tokens[0]);

  /* Don't leak memory.  */
  pru_insn_reloc_destroy (insn->insn_reloc);
  free (op_strdup);
}

/* Round up section size.  */
valueT
md_section_align (asection *seg, valueT addr)
{
  int align = bfd_section_alignment (seg);
  return ((addr + (1 << align) - 1) & (-((valueT) 1 << align)));
}

/* Implement tc_fix_adjustable.  */
int
pru_fix_adjustable (fixS *fixp)
{
  if (fixp->fx_addsy == NULL)
    return 1;

  /* Prevent all adjustments to global symbols.  */
  if (OUTPUT_FLAVOR == bfd_target_elf_flavour
      && (S_IS_EXTERNAL (fixp->fx_addsy) || S_IS_WEAK (fixp->fx_addsy)))
    return 0;

  if (fixp->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fixp->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    return 0;

  /* Preserve relocations against symbols with function type.  */
  if (symbol_get_bfdsym (fixp->fx_addsy)->flags & BSF_FUNCTION)
    return 0;

  return 1;
}

/* The function tc_gen_reloc creates a relocation structure for the
   fixup fixp, and returns a pointer to it.  This structure is passed
   to bfd_install_relocation so that it can be written to the object
   file for linking.  */
arelent *
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED, fixS *fixp)
{
  arelent *reloc = XNEW (arelent);
  reloc->sym_ptr_ptr = XNEW (asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);

  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
  reloc->addend = fixp->fx_offset;  /* fixp->fx_addnumber; */

  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
  if (reloc->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("can't represent relocation type %s"),
		    bfd_get_reloc_code_name (fixp->fx_r_type));

      /* Set howto to a garbage value so that we can keep going.  */
      reloc->howto = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_32);
      gas_assert (reloc->howto != NULL);
    }
  return reloc;
}

long
md_pcrel_from (fixS *fixP ATTRIBUTE_UNUSED)
{
  return fixP->fx_where + fixP->fx_frag->fr_address;
}

/* Called just before the assembler exits.  */
void
pru_md_end (void)
{
  htab_delete (pru_opcode_hash);
  htab_delete (pru_reg_hash);
}

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return NULL;
}

/* Implement tc_frob_label.  */
void
pru_frob_label (symbolS *lab)
{
  /* Emit dwarf information.  */
  dwarf2_emit_label (lab);

  /* Update the label's address with the current output pointer.  */
  symbol_set_frag (lab, frag_now);
  S_SET_VALUE (lab, (valueT) frag_now_fix ());

  /* Record this label for future adjustment after we find out what
     kind of data it references, and the required alignment therewith.  */
  pru_last_label = lab;

  if (pru_opt.warn_regname_label && pru_reg_lookup (S_GET_NAME (lab)))
    as_warn (_("Label \"%s\" matches a CPU register name"), S_GET_NAME (lab));
}

static inline char *
skip_space (char *s)
{
  while (*s == ' ' || *s == '\t')
    ++s;
  return s;
}

/* Parse special CONS expression: pmem (expression).  Idea from AVR.

   Used to catch and mark code (program memory) in constant expression
   relocations.  Return non-zero for program memory.  */

int
pru_parse_cons_expression (expressionS *exp, int nbytes)
{
  int is_pmem = false;
  char *tmp;

  tmp = input_line_pointer = skip_space (input_line_pointer);

  if (nbytes == 4 || nbytes == 2)
    {
      const char *pmem_str = "%pmem";
      int len = strlen (pmem_str);

      if (strncasecmp (input_line_pointer, pmem_str, len) == 0)
	{
	  input_line_pointer = skip_space (input_line_pointer + len);

	  if (*input_line_pointer == '(')
	    {
	      input_line_pointer = skip_space (input_line_pointer + 1);
	      is_pmem = true;
	      expression (exp);

	      if (*input_line_pointer == ')')
		++input_line_pointer;
	      else
		{
		  as_bad (_("`)' required"));
		  is_pmem = false;
		}

	      return is_pmem;
	    }

	  input_line_pointer = tmp;
	}
    }

  expression (exp);

  return is_pmem;
}

/* Implement TC_CONS_FIX_NEW.  */
void
pru_cons_fix_new (fragS *frag, int where, unsigned int nbytes,
		    expressionS *exp, const int is_pmem)
{
  bfd_reloc_code_real_type r;

  switch (nbytes | (!!is_pmem << 8))
    {
    case 1 | (0 << 8): r = BFD_RELOC_8; break;
    case 2 | (0 << 8): r = BFD_RELOC_16; break;
    case 4 | (0 << 8): r = BFD_RELOC_32; break;
    case 8 | (0 << 8): r = BFD_RELOC_64; break;
    case 2 | (1 << 8): r = BFD_RELOC_PRU_16_PMEM; break;
    case 4 | (1 << 8): r = BFD_RELOC_PRU_32_PMEM; break;
    default:
      as_bad (_("illegal %s relocation size: %d"),
	      is_pmem ? "text" : "data", nbytes);
      return;
    }

  fix_new_exp (frag, where, (int) nbytes, exp, 0, r);
}

/* Implement tc_regname_to_dw2regnum, to convert REGNAME to a DWARF-2
   register number.  Return the starting HW byte-register number.  */

int
pru_regname_to_dw2regnum (char *regname)
{
  static const unsigned int regstart[RSEL_NUM_ITEMS] =
    {
     [RSEL_7_0]	  = 0,
     [RSEL_15_8]  = 1,
     [RSEL_23_16] = 2,
     [RSEL_31_24] = 3,
     [RSEL_15_0]  = 0,
     [RSEL_23_8]  = 1,
     [RSEL_31_16] = 2,
     [RSEL_31_0]  = 0,
    };

  struct pru_reg *r = pru_reg_lookup (regname);

  if (r == NULL || r->regsel >= RSEL_NUM_ITEMS)
    return -1;
  return r->index * 4 + regstart[r->regsel];
}

/* Implement tc_cfi_frame_initial_instructions, to initialize the DWARF-2
   unwind information for this procedure.  */
void
pru_frame_initial_instructions (void)
{
  const unsigned fp_regno = 4 * 4;
  cfi_add_CFA_def_cfa (fp_regno, 0);
}

bool
pru_allow_local_subtract (expressionS * left,
			     expressionS * right,
			     segT section)
{
  /* If we are not in relaxation mode, subtraction is OK.  */
  if (!linkrelax)
    return true;

  /* If the symbols are not in a code section then they are OK.  */
  if ((section->flags & SEC_CODE) == 0)
    return true;

  if (left->X_add_symbol == right->X_add_symbol)
    return true;

  /* We have to assume that there may be instructions between the
     two symbols and that relaxation may increase the distance between
     them.  */
  return false;
}
