/* tc-microblaze.c -- Assemble code for Xilinx MicroBlaze

   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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
#include <stdio.h>
#include "bfd.h"
#include "subsegs.h"
#define DEFINE_TABLE
#include "../opcodes/microblaze-opc.h"
#include "../opcodes/microblaze-opcm.h"
#include "safe-ctype.h"
#include <string.h>
#include <dwarf2dbg.h>
#include "aout/stab_gnu.h"

#ifndef streq
#define streq(a,b) (strcmp (a, b) == 0)
#endif

#define OPTION_EB (OPTION_MD_BASE + 0)
#define OPTION_EL (OPTION_MD_BASE + 1)

void microblaze_generate_symbol (char *sym);
static bool check_spl_reg (unsigned *);

/* Several places in this file insert raw instructions into the
   object. They should generate the instruction
   and then use these four macros to crack the instruction value into
   the appropriate byte values.  */
#define	INST_BYTE0(x)  (target_big_endian ? (((x) >> 24) & 0xFF) : ((x) & 0xFF))
#define	INST_BYTE1(x)  (target_big_endian ? (((x) >> 16) & 0xFF) : (((x) >> 8) & 0xFF))
#define	INST_BYTE2(x)  (target_big_endian ? (((x) >> 8) & 0xFF) : (((x) >> 16) & 0xFF))
#define	INST_BYTE3(x)  (target_big_endian ? ((x) & 0xFF) : (((x) >> 24) & 0xFF))

/* This array holds the chars that always start a comment.  If the
   pre-processor is disabled, these aren't very useful.  */
const char comment_chars[] = "#";

const char line_separator_chars[] = ";";

/* This array holds the chars that only start a comment at the beginning of
   a line.  */
const char line_comment_chars[] = "#";

const int md_reloc_size = 8; /* Size of relocation record.  */

/* Chars that can be used to separate mant
   from exp in floating point numbers.  */
const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant
   As in 0f12.456
   or    0d1.2345e12.  */
const char FLT_CHARS[] = "rRsSfFdDxXpP";

/* INST_PC_OFFSET and INST_NO_OFFSET are 0 and 1.  */
#define UNDEFINED_PC_OFFSET  2
#define DEFINED_ABS_SEGMENT  3
#define DEFINED_PC_OFFSET    4
#define DEFINED_RO_SEGMENT   5
#define DEFINED_RW_SEGMENT   6
#define LARGE_DEFINED_PC_OFFSET 7
#define GOT_OFFSET           8
#define PLT_OFFSET           9
#define GOTOFF_OFFSET        10
#define TLSGD_OFFSET         11
#define TLSLD_OFFSET         12
#define TLSDTPMOD_OFFSET     13
#define TLSDTPREL_OFFSET     14
#define TLSGOTTPREL_OFFSET   15
#define TLSTPREL_OFFSET      16
#define TEXT_OFFSET	     17
#define TEXT_PC_OFFSET       18

/* Initialize the relax table.  */
const relax_typeS md_relax_table[] =
{
  {          1,          1,                0, 0 },  /*  0: Unused.  */
  {          1,          1,                0, 0 },  /*  1: Unused.  */
  {          1,          1,                0, 0 },  /*  2: Unused.  */
  {          1,          1,                0, 0 },  /*  3: Unused.  */
  {      32767,   -32768, INST_WORD_SIZE, LARGE_DEFINED_PC_OFFSET }, /* 4: DEFINED_PC_OFFSET.  */
  {    1,     1,       0, 0 },                      /*  5: Unused.  */
  {    1,     1,       0, 0 },                      /*  6: Unused.  */
  { 0x7fffffff, 0x80000000, INST_WORD_SIZE*2, 0 },  /*  7: LARGE_DEFINED_PC_OFFSET.  */
  { 0x7fffffff, 0x80000000, INST_WORD_SIZE*2, 0 },  /*  8: GOT_OFFSET.  */
  { 0x7fffffff, 0x80000000, INST_WORD_SIZE*2, 0 },  /*  9: PLT_OFFSET.  */
  { 0x7fffffff, 0x80000000, INST_WORD_SIZE*2, 0 },  /* 10: GOTOFF_OFFSET.  */
  { 0x7fffffff, 0x80000000, INST_WORD_SIZE*2, 0 },  /* 11: TLSGD_OFFSET.  */
  { 0x7fffffff, 0x80000000, INST_WORD_SIZE*2, 0 },  /* 12: TLSLD_OFFSET.  */
  { 0x7fffffff, 0x80000000, INST_WORD_SIZE*1, 0 },  /* 13: TLSDTPMOD_OFFSET.  */
  { 0x7fffffff, 0x80000000, INST_WORD_SIZE*2, 0 },  /* 14: TLSDTPREL_OFFSET.  */
  { 0x7fffffff, 0x80000000, INST_WORD_SIZE*2, 0 },  /* 15: TLSGOTTPREL_OFFSET.  */
  { 0x7fffffff, 0x80000000, INST_WORD_SIZE*2, 0 },  /* 16: TLSTPREL_OFFSET.  */
  { 0x7fffffff, 0x80000000, INST_WORD_SIZE*2, 0 },  /* 17: TEXT_OFFSET.  */
  { 0x7fffffff, 0x80000000, INST_WORD_SIZE*2, 0 }   /* 18: TEXT_PC_OFFSET.  */
};

static htab_t  opcode_hash_control;	/* Opcode mnemonics.  */

static segT sbss_segment = 0; 	/* Small bss section.  */
static segT sbss2_segment = 0; 	/* Section not used.  */
static segT sdata_segment = 0; 	/* Small data section.  */
static segT sdata2_segment = 0; /* Small read-only section.  */
static segT rodata_segment = 0; /* read-only section.  */

/* Generate a symbol for stabs information.  */

void
microblaze_generate_symbol (char *sym)
{
#define MICROBLAZE_FAKE_LABEL_NAME "XL0\001"
  static int microblaze_label_count;
  sprintf (sym, "%sL%d", MICROBLAZE_FAKE_LABEL_NAME, microblaze_label_count);
  ++microblaze_label_count;
}

/* Handle the section changing pseudo-ops. */

static void
microblaze_s_text (int ignore ATTRIBUTE_UNUSED)
{
#ifdef OBJ_ELF
  obj_elf_text (ignore);
#else
  s_text (ignore);
#endif
}

static void
microblaze_s_data (int ignore ATTRIBUTE_UNUSED)
{
#ifdef OBJ_ELF
  obj_elf_change_section (".data", SHT_PROGBITS, SHF_ALLOC+SHF_WRITE,
			  0, 0, 0, 0);
#else
  s_data (ignore);
#endif
}

/* Things in the .sdata segment are always considered to be in the small data section.  */

static void
microblaze_s_sdata (int ignore ATTRIBUTE_UNUSED)
{
#ifdef OBJ_ELF
  obj_elf_change_section (".sdata", SHT_PROGBITS, SHF_ALLOC+SHF_WRITE,
			  0, 0, 0, 0);
#else
  s_data (ignore);
#endif
}

/* Pseudo op to make file scope bss items.  */

static void
microblaze_s_lcomm (int xxx ATTRIBUTE_UNUSED)
{
  char *name;
  char c;
  char *p;
  offsetT size;
  symbolS *symbolP;
  offsetT align;
  char *pfrag;
  int align2;
  segT current_seg = now_seg;
  subsegT current_subseg = now_subseg;

  c = get_symbol_name (&name);

  /* Just after name is now '\0'.  */
  p = input_line_pointer;
  (void) restore_line_pointer (c);
  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
      as_bad (_("Expected comma after symbol-name: rest of line ignored."));
      ignore_rest_of_line ();
      return;
    }

  input_line_pointer++;		/* skip ',' */
  if ((size = get_absolute_expression ()) < 0)
    {
      as_warn (_(".COMMon length (%ld.) <0! Ignored."), (long) size);
      ignore_rest_of_line ();
      return;
    }

  /* The third argument to .lcomm is the alignment.  */
  if (*input_line_pointer != ',')
    align = 8;
  else
    {
      ++input_line_pointer;
      align = get_absolute_expression ();
      if (align <= 0)
	{
	  as_warn (_("ignoring bad alignment"));
	  align = 8;
	}
    }

  *p = 0;
  symbolP = symbol_find_or_make (name);
  *p = c;

  if (S_IS_DEFINED (symbolP) && ! S_IS_COMMON (symbolP))
    {
      as_bad (_("Ignoring attempt to re-define symbol `%s'."),
	      S_GET_NAME (symbolP));
      ignore_rest_of_line ();
      return;
    }

  if (S_GET_VALUE (symbolP) && S_GET_VALUE (symbolP) != (valueT) size)
    {
      as_bad (_("Length of .lcomm \"%s\" is already %ld. Not changed to %ld."),
	      S_GET_NAME (symbolP),
	      (long) S_GET_VALUE (symbolP),
	      (long) size);

      ignore_rest_of_line ();
      return;
    }

  /* Allocate_bss.  */
  if (align)
    {
      /* Convert to a power of 2 alignment.  */
      for (align2 = 0; (align & 1) == 0; align >>= 1, ++align2);
      if (align != 1)
	{
	  as_bad (_("Common alignment not a power of 2"));
	  ignore_rest_of_line ();
	  return;
	}
    }
  else
    align2 = 0;

  record_alignment (current_seg, align2);
  subseg_set (current_seg, current_subseg);
  if (align2)
    frag_align (align2, 0, 0);
  if (S_GET_SEGMENT (symbolP) == current_seg)
    symbol_get_frag (symbolP)->fr_symbol = 0;
  symbol_set_frag (symbolP, frag_now);
  pfrag = frag_var (rs_org, 1, 1, (relax_substateT) 0, symbolP, size,
		    (char *) 0);
  *pfrag = 0;
  S_SET_SIZE (symbolP, size);
  S_SET_SEGMENT (symbolP, current_seg);
  subseg_set (current_seg, current_subseg);
  demand_empty_rest_of_line ();
}

static void
microblaze_s_rdata (int localvar)
{
#ifdef OBJ_ELF
  if (localvar == 0)
    {
      /* rodata.  */
      obj_elf_change_section (".rodata", SHT_PROGBITS, SHF_ALLOC,
			      0, 0, 0, 0);
      if (rodata_segment == 0)
	rodata_segment = subseg_new (".rodata", 0);
    }
  else
    {
      /* 1 .sdata2.  */
      obj_elf_change_section (".sdata2", SHT_PROGBITS, SHF_ALLOC,
			      0, 0, 0, 0);
    }
#else
  s_data (ignore);
#endif
}

static void
microblaze_s_bss (int localvar)
{
#ifdef OBJ_ELF
  if (localvar == 0) /* bss.  */
    obj_elf_change_section (".bss", SHT_NOBITS, SHF_ALLOC+SHF_WRITE,
			    0, 0, 0, 0);
  else if (localvar == 1)
    {
      /* sbss.  */
      obj_elf_change_section (".sbss", SHT_NOBITS, SHF_ALLOC+SHF_WRITE,
			      0, 0, 0, 0);
      if (sbss_segment == 0)
	sbss_segment = subseg_new (".sbss", 0);
    }
#else
  s_data (ignore);
#endif
}

/* endp_p is always 1 as this func is called only for .end <funcname>
   This func consumes the <funcname> and calls regular processing
   s_func(1) with arg 1 (1 for end). */

static void
microblaze_s_func (int end_p ATTRIBUTE_UNUSED)
{
  char *name;
  restore_line_pointer (get_symbol_name (&name));
  s_func (1);
}

/* Handle the .weakext pseudo-op as defined in Kane and Heinrich.  */

static void
microblaze_s_weakext (int ignore ATTRIBUTE_UNUSED)
{
  char *name;
  int c;
  symbolS *symbolP;
  expressionS exp;

  c = get_symbol_name (&name);
  symbolP = symbol_find_or_make (name);
  S_SET_WEAK (symbolP);
  (void) restore_line_pointer (c);

  SKIP_WHITESPACE ();

  if (!is_end_of_line[(unsigned char) *input_line_pointer])
    {
      if (S_IS_DEFINED (symbolP))
	{
	  as_bad ("Ignoring attempt to redefine symbol `%s'.",
		  S_GET_NAME (symbolP));
	  ignore_rest_of_line ();
	  return;
	}

      if (*input_line_pointer == ',')
	{
	  ++input_line_pointer;
	  SKIP_WHITESPACE ();
	}

      expression (&exp);
      if (exp.X_op != O_symbol)
	{
	  as_bad ("bad .weakext directive");
	  ignore_rest_of_line ();
	  return;
	}
      symbol_set_value_expression (symbolP, &exp);
    }

  demand_empty_rest_of_line ();
}

/* This table describes all the machine specific pseudo-ops the assembler
   has to support.  The fields are:
   Pseudo-op name without dot
   Function to call to execute this pseudo-op
   Integer arg to pass to the function.  */
/* If the pseudo-op is not found in this table, it searches in the obj-elf.c,
   and then in the read.c table.  */
const pseudo_typeS md_pseudo_table[] =
{
  {"lcomm", microblaze_s_lcomm, 1},
  {"data", microblaze_s_data, 0},
  {"data8", cons, 1},      /* Same as byte.  */
  {"data16", cons, 2},     /* Same as hword.  */
  {"data32", cons, 4},     /* Same as word.  */
  {"ent", s_func, 0}, /* Treat ent as function entry point.  */
  {"end", microblaze_s_func, 1}, /* Treat end as function end point.  */
  {"gpword", s_rva, 4}, /* gpword label => store resolved label address in data section.  */
  {"weakext", microblaze_s_weakext, 0},
  {"rodata", microblaze_s_rdata, 0},
  {"sdata2", microblaze_s_rdata, 1},
  {"sdata", microblaze_s_sdata, 0},
  {"bss", microblaze_s_bss, 0},
  {"sbss", microblaze_s_bss, 1},
  {"text", microblaze_s_text, 0},
  {"word", cons, 4},
  {"frame", s_ignore, 0},
  {"mask", s_ignore, 0}, /* Emitted by gcc.  */
  {NULL, NULL, 0}
};

/* This function is called once, at assembler startup time.  This should
   set up all the tables, etc that the MD part of the assembler needs.  */

void
md_begin (void)
{
  const struct op_code_struct * opcode;

  opcode_hash_control = str_htab_create ();

  /* Insert unique names into hash table.  */
  for (opcode = microblaze_opcodes; opcode->name; opcode ++)
    str_hash_insert (opcode_hash_control, opcode->name, opcode, 0);
}

/* Try to parse a reg name.  */

static char *
parse_reg (char * s, unsigned * reg)
{
  unsigned tmpreg = 0;

  /* Strip leading whitespace.  */
  while (ISSPACE (* s))
    ++ s;

  if (strncasecmp (s, "rpc", 3) == 0)
    {
      *reg = REG_PC;
      return s + 3;
    }
  else if (strncasecmp (s, "rmsr", 4) == 0)
    {
      *reg = REG_MSR;
      return s + 4;
    }
  else if (strncasecmp (s, "rear", 4) == 0)
    {
      *reg = REG_EAR;
      return s + 4;
    }
  else if (strncasecmp (s, "resr", 4) == 0)
    {
      *reg = REG_ESR;
      return s + 4;
    }
  else if (strncasecmp (s, "rfsr", 4) == 0)
    {
      *reg = REG_FSR;
      return s + 4;
    }
  else if (strncasecmp (s, "rbtr", 4) == 0)
    {
      *reg = REG_BTR;
      return s + 4;
    }
  else if (strncasecmp (s, "redr", 4) == 0)
    {
      *reg = REG_EDR;
      return s + 4;
    }
  /* MMU registers start.  */
  else if (strncasecmp (s, "rpid", 4) == 0)
    {
      *reg = REG_PID;
      return s + 4;
    }
  else if (strncasecmp (s, "rzpr", 4) == 0)
    {
      *reg = REG_ZPR;
      return s + 4;
    }
  else if (strncasecmp (s, "rtlbx", 5) == 0)
    {
      *reg = REG_TLBX;
      return s + 5;
    }
  else if (strncasecmp (s, "rtlblo", 6) == 0)
    {
      *reg = REG_TLBLO;
      return s + 6;
    }
  else if (strncasecmp (s, "rtlbhi", 6) == 0)
    {
      *reg = REG_TLBHI;
      return s + 6;
    }
  else if (strncasecmp (s, "rtlbsx", 6) == 0)
    {
      *reg = REG_TLBSX;
      return s + 6;
    }
  /* MMU registers end.  */
  else if (strncasecmp (s, "rpvr", 4) == 0)
    {
      if (ISDIGIT (s[4]) && ISDIGIT (s[5]))
        {
          tmpreg = (s[4]-'0')*10 + s[5] - '0';
          s += 6;
        }

      else if (ISDIGIT (s[4]))
        {
          tmpreg = s[4] - '0';
          s += 5;
        }
      else
        as_bad (_("register expected, but saw '%.6s'"), s);
      if ((int) tmpreg >= MIN_PVR_REGNUM && tmpreg <= MAX_PVR_REGNUM)
        *reg = REG_PVR + tmpreg;
      else
        {
          as_bad (_("Invalid register number at '%.6s'"), s);
          *reg = REG_PVR;
        }
      return s;
    }
  else if (strncasecmp (s, "rsp", 3) == 0)
    {
      *reg = REG_SP;
      return s + 3;
    }
  else if (strncasecmp (s, "rfsl", 4) == 0)
    {
      if (ISDIGIT (s[4]) && ISDIGIT (s[5]))
        {
          tmpreg = (s[4] - '0') * 10 + s[5] - '0';
          s += 6;
        }
      else if (ISDIGIT (s[4]))
        {
          tmpreg = s[4] - '0';
          s += 5;
        }
      else
	as_bad (_("register expected, but saw '%.6s'"), s);

      if ((int) tmpreg >= MIN_REGNUM && tmpreg <= MAX_REGNUM)
        *reg = tmpreg;
      else
	{
          as_bad (_("Invalid register number at '%.6s'"), s);
          *reg = 0;
	}
      return s;
    }
  /* Stack protection registers.  */
  else if (strncasecmp (s, "rshr", 4) == 0)
    {
      *reg = REG_SHR;
      return s + 4;
    }
  else if (strncasecmp (s, "rslr", 4) == 0)
    {
      *reg = REG_SLR;
      return s + 4;
    }
  else
    {
      if (TOLOWER (s[0]) == 'r')
        {
          if (ISDIGIT (s[1]) && ISDIGIT (s[2]))
            {
              tmpreg = (s[1] - '0') * 10 + s[2] - '0';
              s += 3;
            }
          else if (ISDIGIT (s[1]))
            {
              tmpreg = s[1] - '0';
              s += 2;
            }
          else
            as_bad (_("register expected, but saw '%.6s'"), s);

          if ((int)tmpreg >= MIN_REGNUM && tmpreg <= MAX_REGNUM)
            *reg = tmpreg;
          else
	    {
              as_bad (_("Invalid register number at '%.6s'"), s);
              *reg = 0;
	    }
          return s;
        }
    }
  as_bad (_("register expected, but saw '%.6s'"), s);
  *reg = 0;
  return s;
}

static char *
parse_exp (char *s, expressionS *e)
{
  char *save;
  char *new_pointer;

  /* Skip whitespace.  */
  while (ISSPACE (* s))
    ++ s;

  save = input_line_pointer;
  input_line_pointer = s;

  expression (e);

  if (e->X_op == O_absent)
    as_fatal (_("missing operand"));

  new_pointer = input_line_pointer;
  input_line_pointer = save;

  return new_pointer;
}

/* Symbol modifiers (@GOT, @PLT, @GOTOFF).  */
#define IMM_NONE   0
#define IMM_GOT    1
#define IMM_PLT    2
#define IMM_GOTOFF 3
#define IMM_TLSGD  4
#define IMM_TLSLD  5
#define IMM_TLSDTPMOD 6
#define IMM_TLSDTPREL 7
#define IMM_TLSTPREL  8
#define IMM_TXTREL    9
#define IMM_TXTPCREL  10
#define IMM_MAX    11

struct imm_type {
	const char *isuffix;	 /* Suffix String */
	int itype;       /* Suffix Type */
	int otype;       /* Offset Type */
};

/* These are NOT in ascending order of type, GOTOFF is ahead to make
   sure @GOTOFF does not get matched with @GOT  */
static struct imm_type imm_types[] = {
	{ "NONE", IMM_NONE , 0 },
	{ "GOTOFF", IMM_GOTOFF , GOTOFF_OFFSET },
	{ "GOT", IMM_GOT , GOT_OFFSET },
	{ "PLT", IMM_PLT , PLT_OFFSET },
	{ "TLSGD", IMM_TLSGD , TLSGD_OFFSET },
	{ "TLSLDM", IMM_TLSLD, TLSLD_OFFSET },
	{ "TLSDTPMOD", IMM_TLSDTPMOD, TLSDTPMOD_OFFSET },
	{ "TLSDTPREL", IMM_TLSDTPREL, TLSDTPREL_OFFSET },
	{ "TLSTPREL", IMM_TLSTPREL, TLSTPREL_OFFSET },
	{ "TXTREL", IMM_TXTREL, TEXT_OFFSET },
	{ "TXTPCREL", IMM_TXTPCREL, TEXT_PC_OFFSET }
};

static int
match_imm (const char *s, int *ilen)
{
  int i;
  int slen;

  /* Check for matching suffix */
  for (i = 1; i < IMM_MAX; i++)
    {
      slen = strlen (imm_types[i].isuffix);

      if (strncmp (imm_types[i].isuffix, s, slen) == 0)
        {
          *ilen = slen;
          return imm_types[i].itype;
        }
    } /* for */
  *ilen = 0;
  return 0;
}

static int
get_imm_otype (int itype)
{
  int i, otype;

  otype = 0;
  /* Check for matching itype */
  for (i = 1; i < IMM_MAX; i++)
    {
      if (imm_types[i].itype == itype)
        {
          otype = imm_types[i].otype;
          break;
        }
    }
  return otype;
}

static symbolS * GOT_symbol;

#define GOT_SYMBOL_NAME "_GLOBAL_OFFSET_TABLE_"

static char *
parse_imm (char * s, expressionS * e, offsetT min, offsetT max)
{
  char *new_pointer;
  char *atp;
  int itype, ilen;

  ilen = 0;

  /* Find the start of "@GOT" or "@PLT" suffix (if any) */
  for (atp = s; *atp != '@'; atp++)
    if (is_end_of_line[(unsigned char) *atp])
      break;

  if (*atp == '@')
    {
      itype = match_imm (atp + 1, &ilen);
      if (itype != 0)
        {
          *atp = 0;
          e->X_md = itype;
        }
      else
        {
          atp = NULL;
          e->X_md = 0;
          ilen = 0;
        }
      *atp = 0;
    }
  else
    {
      atp = NULL;
      e->X_md = 0;
    }

  if (atp && !GOT_symbol)
    {
      GOT_symbol = symbol_find_or_make (GOT_SYMBOL_NAME);
    }

  new_pointer = parse_exp (s, e);

  if (!GOT_symbol && startswith (s, GOT_SYMBOL_NAME))
    {
      GOT_symbol = symbol_find_or_make (GOT_SYMBOL_NAME);
    }

  if (e->X_op == O_absent)
    ; /* An error message has already been emitted.  */
  else if ((e->X_op != O_constant && e->X_op != O_symbol) )
    as_fatal (_("operand must be a constant or a label"));
  else if (e->X_op == O_constant)
    {
      /* Special case: sign extend negative 32-bit values to offsetT size.  */
      if ((e->X_add_number >> 31) == 1)
	e->X_add_number |= -((addressT) (1U << 31));

      if (e->X_add_number < min || e->X_add_number > max)
	{
	  as_fatal (_("operand must be absolute in range %lx..%lx, not %lx"),
		    (long) min, (long) max, (long) e->X_add_number);
	}
    }

  if (atp)
    {
      *atp = '@'; /* restore back (needed?)  */
      if (new_pointer >= atp)
        new_pointer += ilen + 1; /* sizeof (imm_suffix) + 1 for '@' */
    }
  return new_pointer;
}

static char *
check_got (int * got_type, int * got_len)
{
  char *new_pointer;
  char *atp;
  char *past_got;
  int first, second;
  char *tmpbuf;

  /* Find the start of "@GOT" or "@PLT" suffix (if any).  */
  for (atp = input_line_pointer; *atp != '@'; atp++)
    if (is_end_of_line[(unsigned char) *atp])
      return NULL;

  if (startswith (atp + 1, "GOTOFF"))
    {
      *got_len = 6;
      *got_type = IMM_GOTOFF;
    }
  else if (startswith (atp + 1, "GOT"))
    {
      *got_len = 3;
      *got_type = IMM_GOT;
    }
  else if (startswith (atp + 1, "PLT"))
    {
      *got_len = 3;
      *got_type = IMM_PLT;
    }
  else
    return NULL;

  if (!GOT_symbol)
    GOT_symbol = symbol_find_or_make (GOT_SYMBOL_NAME);

  first = atp - input_line_pointer;

  past_got = atp + *got_len + 1;
  for (new_pointer = past_got; !is_end_of_line[(unsigned char) *new_pointer++];)
    ;
  second = new_pointer - past_got;
  /* One extra byte for ' ' and one for NUL.  */
  tmpbuf = XNEWVEC (char, first + second + 2);
  memcpy (tmpbuf, input_line_pointer, first);
  tmpbuf[first] = ' '; /* @GOTOFF is replaced with a single space.  */
  memcpy (tmpbuf + first + 1, past_got, second);
  tmpbuf[first + second + 1] = '\0';

  return tmpbuf;
}

extern bfd_reloc_code_real_type
parse_cons_expression_microblaze (expressionS *exp, int size)
{
  if (size == 4)
    {
      /* Handle @GOTOFF et.al.  */
      char *save, *gotfree_copy;
      int got_len, got_type;

      save = input_line_pointer;
      gotfree_copy = check_got (& got_type, & got_len);
      if (gotfree_copy)
        input_line_pointer = gotfree_copy;

      expression (exp);

      if (gotfree_copy)
	{
          exp->X_md = got_type;
          input_line_pointer = save + (input_line_pointer - gotfree_copy)
	    + got_len;
          free (gotfree_copy);
        }
    }
  else
    expression (exp);
  return BFD_RELOC_NONE;
}

/* This is the guts of the machine-dependent assembler.  STR points to a
   machine dependent instruction.  This function is supposed to emit
   the frags/bytes it assembles to.  */

static const char * str_microblaze_ro_anchor = "RO";
static const char * str_microblaze_rw_anchor = "RW";

static bool
check_spl_reg (unsigned * reg)
{
  if ((*reg == REG_MSR)   || (*reg == REG_PC)
      || (*reg == REG_EAR)   || (*reg == REG_ESR)
      || (*reg == REG_FSR)   || (*reg == REG_BTR) || (*reg == REG_EDR)
      || (*reg == REG_PID)   || (*reg == REG_ZPR)
      || (*reg == REG_TLBX)  || (*reg == REG_TLBLO)
      || (*reg == REG_TLBHI) || (*reg == REG_TLBSX)
      || (*reg == REG_SHR)   || (*reg == REG_SLR)
      || (*reg >= REG_PVR+MIN_PVR_REGNUM && *reg <= REG_PVR+MAX_PVR_REGNUM))
    return true;

  return false;
}

/* Here we decide which fixups can be adjusted to make them relative to
   the beginning of the section instead of the symbol.  Basically we need
   to make sure that the dynamic relocations are done correctly, so in
   some cases we force the original symbol to be used.  */

int
tc_microblaze_fix_adjustable (struct fix *fixP)
{
  if (GOT_symbol && fixP->fx_subsy == GOT_symbol)
    return 0;

  if (fixP->fx_r_type == BFD_RELOC_MICROBLAZE_64_GOTOFF
      || fixP->fx_r_type == BFD_RELOC_MICROBLAZE_32_GOTOFF
      || fixP->fx_r_type == BFD_RELOC_MICROBLAZE_64_GOT
      || fixP->fx_r_type == BFD_RELOC_MICROBLAZE_64_PLT
      || fixP->fx_r_type == BFD_RELOC_MICROBLAZE_64_TLSGD
      || fixP->fx_r_type == BFD_RELOC_MICROBLAZE_64_TLSLD
      || fixP->fx_r_type == BFD_RELOC_MICROBLAZE_32_TLSDTPMOD
      || fixP->fx_r_type == BFD_RELOC_MICROBLAZE_32_TLSDTPREL
      || fixP->fx_r_type == BFD_RELOC_MICROBLAZE_64_TLSDTPREL
      || fixP->fx_r_type == BFD_RELOC_MICROBLAZE_64_TLSGOTTPREL
      || fixP->fx_r_type == BFD_RELOC_MICROBLAZE_64_TLSTPREL)
    return 0;

  return 1;
}

void
md_assemble (char * str)
{
  char * op_start;
  char * op_end;
  struct op_code_struct * opcode, *opcode1;
  char * output = NULL;
  int nlen = 0;
  int i;
  unsigned long inst, inst1;
  unsigned reg1;
  unsigned reg2;
  unsigned reg3;
  unsigned isize;
  unsigned int immed = 0, temp;
  expressionS exp;
  char name[20];

  /* Drop leading whitespace.  */
  while (ISSPACE (* str))
    str ++;

  /* Find the op code end.  */
  for (op_start = op_end = str;
       *op_end && !is_end_of_line[(unsigned char) *op_end] && *op_end != ' ';
       op_end++)
    {
      name[nlen] = op_start[nlen];
      nlen++;
      if (nlen == sizeof (name) - 1)
	break;
    }

  name [nlen] = 0;

  if (nlen == 0)
    {
      as_bad (_("can't find opcode "));
      return;
    }

  opcode = (struct op_code_struct *) str_hash_find (opcode_hash_control, name);
  if (opcode == NULL)
    {
      as_bad (_("unknown opcode \"%s\""), name);
      return;
    }

  inst = opcode->bit_sequence;
  isize = 4;

  switch (opcode->inst_type)
    {
    case INST_TYPE_RD_R1_R2:
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg1);  /* Get rd.  */
      else
        {
          as_fatal (_("Error in statement syntax"));
          reg1 = 0;
        }
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg2);  /* Get r1.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg2 = 0;
        }
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg3);  /* Get r2.  */
      else
 	{
          as_fatal (_("Error in statement syntax"));
          reg3 = 0;
        }

      /* Check for spl registers.  */
      if (check_spl_reg (& reg1))
        as_fatal (_("Cannot use special register with this instruction"));
      if (check_spl_reg (& reg2))
        as_fatal (_("Cannot use special register with this instruction"));
      if (check_spl_reg (& reg3))
        as_fatal (_("Cannot use special register with this instruction"));

      if (streq (name, "sub"))
	{
          /* sub rd, r1, r2 becomes rsub rd, r2, r1.  */
          inst |= (reg1 << RD_LOW) & RD_MASK;
          inst |= (reg3 << RA_LOW) & RA_MASK;
          inst |= (reg2 << RB_LOW) & RB_MASK;
        }
      else
        {
          inst |= (reg1 << RD_LOW) & RD_MASK;
          inst |= (reg2 << RA_LOW) & RA_MASK;
          inst |= (reg3 << RB_LOW) & RB_MASK;
        }
      output = frag_more (isize);
      break;

    case INST_TYPE_RD_R1_IMM:
      if (strcmp (op_end, ""))
	op_end = parse_reg (op_end + 1, &reg1);  /* Get rd.  */
      else
 	{
          as_fatal (_("Error in statement syntax"));
          reg1 = 0;
        }
      if (strcmp (op_end, ""))
	op_end = parse_reg (op_end + 1, &reg2);  /* Get r1.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg2 = 0;
        }
      if (strcmp (op_end, ""))
	op_end = parse_imm (op_end + 1, & exp, MIN_IMM, MAX_IMM);
      else
	as_fatal (_("Error in statement syntax"));

      /* Check for spl registers.  */
      if (check_spl_reg (& reg1))
	as_fatal (_("Cannot use special register with this instruction"));
      if (check_spl_reg (& reg2))
	as_fatal (_("Cannot use special register with this instruction"));

      if (exp.X_op != O_constant || exp.X_md == IMM_TXTPCREL)
	{
          const char *opc;
	  relax_substateT subtype;

          if (streq (name, "lmi"))
	    as_fatal (_("lmi pseudo instruction should not use a label in imm field"));
	  else if (streq (name, "smi"))
	    as_fatal (_("smi pseudo instruction should not use a label in imm field"));

	  if (reg2 == REG_ROSDP)
	    opc = str_microblaze_ro_anchor;
	  else if (reg2 == REG_RWSDP)
	    opc = str_microblaze_rw_anchor;
	  else
	    opc = NULL;
	  if (exp.X_md != 0)
	    subtype = get_imm_otype(exp.X_md);
	  else
	    subtype = opcode->inst_offset_type;

	  output = frag_var (rs_machine_dependent,
			     isize * 2, /* maxm of 2 words.  */
			     isize,     /* minm of 1 word.  */
			     subtype,   /* PC-relative or not.  */
			     exp.X_add_symbol,
			     exp.X_add_number,
			     (char *) opc);
	  immed = 0;
        }
      else
	{
          output = frag_more (isize);
          immed = exp.X_add_number;
        }

      if (streq (name, "lmi") || streq (name, "smi"))
	{
          /* Load/store 32-d consecutive registers.  Used on exit/entry
             to subroutines to save and restore registers to stack.
             Generate 32-d insts.  */
          int count;

          count = 32 - reg1;
          if (streq (name, "lmi"))
	    opcode
	      = (struct op_code_struct *) str_hash_find (opcode_hash_control,
							 "lwi");
          else
	    opcode
	      = (struct op_code_struct *) str_hash_find (opcode_hash_control,
							 "swi");
          if (opcode == NULL)
            {
              as_bad (_("unknown opcode \"%s\""), "lwi");
              return;
            }
          inst  = opcode->bit_sequence;
          inst |= (reg1 << RD_LOW) & RD_MASK;
          inst |= (reg2 << RA_LOW) & RA_MASK;
          inst |= (immed << IMM_LOW) & IMM_MASK;

          for (i = 0; i < count - 1; i++)
	    {
              output[0] = INST_BYTE0 (inst);
              output[1] = INST_BYTE1 (inst);
              output[2] = INST_BYTE2 (inst);
              output[3] = INST_BYTE3 (inst);
              output = frag_more (isize);
              immed = immed + 4;
              reg1++;
              inst = opcode->bit_sequence;
              inst |= (reg1 << RD_LOW) & RD_MASK;
              inst |= (reg2 << RA_LOW) & RA_MASK;
              inst |= (immed << IMM_LOW) & IMM_MASK;
            }
	}
      else
	{
          temp = immed & 0xFFFF8000;
          if ((temp != 0) && (temp != 0xFFFF8000))
	    {
              /* Needs an immediate inst.  */
	      opcode1
		= (struct op_code_struct *) str_hash_find (opcode_hash_control,
							   "imm");
              if (opcode1 == NULL)
                {
                  as_bad (_("unknown opcode \"%s\""), "imm");
                  return;
                }

              inst1 = opcode1->bit_sequence;
              inst1 |= ((immed & 0xFFFF0000) >> 16) & IMM_MASK;
              output[0] = INST_BYTE0 (inst1);
              output[1] = INST_BYTE1 (inst1);
              output[2] = INST_BYTE2 (inst1);
              output[3] = INST_BYTE3 (inst1);
              output = frag_more (isize);
	    }
	  inst |= (reg1 << RD_LOW) & RD_MASK;
	  inst |= (reg2 << RA_LOW) & RA_MASK;
	  inst |= (immed << IMM_LOW) & IMM_MASK;
	}
      break;

    case INST_TYPE_RD_R1_IMM5:
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg1);  /* Get rd.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg1 = 0;
        }
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg2);  /* Get r1.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg2 = 0;
        }
      if (strcmp (op_end, ""))
        op_end = parse_imm (op_end + 1, & exp, MIN_IMM, MAX_IMM);
      else
        as_fatal (_("Error in statement syntax"));

      /* Check for spl registers.  */
      if (check_spl_reg (&reg1))
        as_fatal (_("Cannot use special register with this instruction"));
      if (check_spl_reg (&reg2))
        as_fatal (_("Cannot use special register with this instruction"));

      if (exp.X_op != O_constant)
        as_warn (_("Symbol used as immediate for shift instruction"));
      else
	{
          output = frag_more (isize);
          immed = exp.X_add_number;
        }

      if (immed != (immed % 32))
	{
          as_warn (_("Shift value > 32. using <value %% 32>"));
          immed = immed % 32;
        }
      inst |= (reg1 << RD_LOW) & RD_MASK;
      inst |= (reg2 << RA_LOW) & RA_MASK;
      inst |= (immed << IMM_LOW) & IMM5_MASK;
      break;

    case INST_TYPE_R1_R2:
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg1);  /* Get r1.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg1 = 0;
        }
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg2);  /* Get r2.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg2 = 0;
        }

      /* Check for spl registers.  */
      if (check_spl_reg (& reg1))
        as_fatal (_("Cannot use special register with this instruction"));
      if (check_spl_reg (& reg2))
        as_fatal (_("Cannot use special register with this instruction"));

      inst |= (reg1 << RA_LOW) & RA_MASK;
      inst |= (reg2 << RB_LOW) & RB_MASK;
      output = frag_more (isize);
      break;

    case INST_TYPE_RD_R1:
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg1);  /* Get rd.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg1 = 0;
        }
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg2);  /* Get r1.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg2 =0;
        }

      /* Check for spl registers.  */
      if (check_spl_reg (&reg1))
        as_fatal (_("Cannot use special register with this instruction"));
      if (check_spl_reg (&reg2))
        as_fatal (_("Cannot use special register with this instruction"));

      inst |= (reg1 << RD_LOW) & RD_MASK;
      inst |= (reg2 << RA_LOW) & RA_MASK;
      output = frag_more (isize);
      break;

    case INST_TYPE_RD_RFSL:
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg1);  /* Get rd.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg1 = 0;
        }
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &immed);  /* Get rfslN.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          immed = 0;
        }

      /* Check for spl registers.  */
      if (check_spl_reg (&reg1))
        as_fatal (_("Cannot use special register with this instruction"));

      inst |= (reg1 << RD_LOW) & RD_MASK;
      inst |= (immed << IMM_LOW) & RFSL_MASK;
      output = frag_more (isize);
      break;

    case INST_TYPE_RD_IMM15:
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg1);  /* Get rd.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg1 = 0;
        }

      if (strcmp (op_end, ""))
        op_end = parse_imm (op_end + 1, & exp, MIN_IMM15, MAX_IMM15);
      else
        as_fatal (_("Error in statement syntax"));

      /* Check for spl registers. */
      if (check_spl_reg (&reg1))
        as_fatal (_("Cannot use special register with this instruction"));

      if (exp.X_op != O_constant)
        as_fatal (_("Symbol used as immediate value for msrset/msrclr instructions"));
      else
	{
          output = frag_more (isize);
          immed = exp.X_add_number;
        }
      inst |= (reg1 << RD_LOW) & RD_MASK;
      inst |= (immed << IMM_LOW) & IMM15_MASK;
      break;

    case INST_TYPE_R1_RFSL:
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg1);  /* Get r1.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg1 = 0;
        }
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &immed);  /* Get rfslN.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          immed = 0;
        }

      /* Check for spl registers.  */
      if (check_spl_reg (&reg1))
        as_fatal (_("Cannot use special register with this instruction"));

      inst |= (reg1 << RA_LOW) & RA_MASK;
      inst |= (immed << IMM_LOW) & RFSL_MASK;
      output = frag_more (isize);
      break;

    case INST_TYPE_RFSL:
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &immed);  /* Get rfslN.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          immed = 0;
        }
      inst |= (immed << IMM_LOW) & RFSL_MASK;
      output = frag_more (isize);
      break;

    case INST_TYPE_R1:
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg1);  /* Get r1.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg1 = 0;
        }

      /* Check for spl registers.  */
      if (check_spl_reg (&reg1))
        as_fatal (_("Cannot use special register with this instruction"));

      inst |= (reg1 << RA_LOW) & RA_MASK;
      output = frag_more (isize);
      break;

      /* For tuqula insn...:) */
    case INST_TYPE_RD:
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg1);  /* Get rd.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg1 = 0;
        }

      /* Check for spl registers.  */
      if (check_spl_reg (&reg1))
        as_fatal (_("Cannot use special register with this instruction"));

      inst |= (reg1 << RD_LOW) & RD_MASK;
      output = frag_more (isize);
      break;

    case INST_TYPE_RD_SPECIAL:
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg1);  /* Get rd.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg1 = 0;
        }
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg2);  /* Get r1.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg2 = 0;
        }

      if (reg2 == REG_MSR)
        immed = opcode->immval_mask | REG_MSR_MASK;
      else if (reg2 == REG_PC)
        immed = opcode->immval_mask | REG_PC_MASK;
      else if (reg2 == REG_EAR)
        immed = opcode->immval_mask | REG_EAR_MASK;
      else if (reg2 == REG_ESR)
        immed = opcode->immval_mask | REG_ESR_MASK;
      else if (reg2 == REG_FSR)
        immed = opcode->immval_mask | REG_FSR_MASK;
      else if (reg2 == REG_BTR)
        immed = opcode->immval_mask | REG_BTR_MASK;
      else if (reg2 == REG_EDR)
        immed = opcode->immval_mask | REG_EDR_MASK;
      else if (reg2 == REG_PID)
        immed = opcode->immval_mask | REG_PID_MASK;
      else if (reg2 == REG_ZPR)
        immed = opcode->immval_mask | REG_ZPR_MASK;
      else if (reg2 == REG_TLBX)
        immed = opcode->immval_mask | REG_TLBX_MASK;
      else if (reg2 == REG_TLBLO)
        immed = opcode->immval_mask | REG_TLBLO_MASK;
      else if (reg2 == REG_TLBHI)
        immed = opcode->immval_mask | REG_TLBHI_MASK;
      else if (reg2 == REG_SHR)
        immed = opcode->immval_mask | REG_SHR_MASK;
      else if (reg2 == REG_SLR)
        immed = opcode->immval_mask | REG_SLR_MASK;
      else if (reg2 >= (REG_PVR+MIN_PVR_REGNUM) && reg2 <= (REG_PVR+MAX_PVR_REGNUM))
	immed = opcode->immval_mask | REG_PVR_MASK | reg2;
      else
        as_fatal (_("invalid value for special purpose register"));
      inst |= (reg1 << RD_LOW) & RD_MASK;
      inst |= (immed << IMM_LOW) & IMM_MASK;
      output = frag_more (isize);
      break;

    case INST_TYPE_SPECIAL_R1:
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg1);  /* Get rd.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg1 = 0;
        }
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg2);  /* Get r1.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg2 = 0;
        }

      if (reg1 == REG_MSR)
        immed = opcode->immval_mask | REG_MSR_MASK;
      else if (reg1 == REG_PC)
        immed = opcode->immval_mask | REG_PC_MASK;
      else if (reg1 == REG_EAR)
        immed = opcode->immval_mask | REG_EAR_MASK;
      else if (reg1 == REG_ESR)
        immed = opcode->immval_mask | REG_ESR_MASK;
      else if (reg1 == REG_FSR)
        immed = opcode->immval_mask | REG_FSR_MASK;
      else if (reg1 == REG_BTR)
        immed = opcode->immval_mask | REG_BTR_MASK;
      else if (reg1 == REG_EDR)
        immed = opcode->immval_mask | REG_EDR_MASK;
      else if (reg1 == REG_PID)
        immed = opcode->immval_mask | REG_PID_MASK;
      else if (reg1 == REG_ZPR)
        immed = opcode->immval_mask | REG_ZPR_MASK;
      else if (reg1 == REG_TLBX)
        immed = opcode->immval_mask | REG_TLBX_MASK;
      else if (reg1 == REG_TLBLO)
        immed = opcode->immval_mask | REG_TLBLO_MASK;
      else if (reg1 == REG_TLBHI)
        immed = opcode->immval_mask | REG_TLBHI_MASK;
      else if (reg1 == REG_TLBSX)
        immed = opcode->immval_mask | REG_TLBSX_MASK;
      else if (reg1 == REG_SHR)
        immed = opcode->immval_mask | REG_SHR_MASK;
      else if (reg1 == REG_SLR)
        immed = opcode->immval_mask | REG_SLR_MASK;
      else
        as_fatal (_("invalid value for special purpose register"));
      inst |= (reg2 << RA_LOW) & RA_MASK;
      inst |= (immed << IMM_LOW) & IMM_MASK;
      output = frag_more (isize);
      break;

    case INST_TYPE_R1_R2_SPECIAL:
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg1);  /* Get r1.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg1 = 0;
        }
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg2);  /* Get r2.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg2 =0;
        }

      /* Check for spl registers.  */
      if (check_spl_reg (&reg1))
        as_fatal (_("Cannot use special register with this instruction"));
      if (check_spl_reg (&reg2))
        as_fatal (_("Cannot use special register with this instruction"));

      /* insn wic ra, rb => wic ra, ra, rb.  */
      inst |= (reg1 << RA_LOW) & RA_MASK;
      inst |= (reg2 << RB_LOW) & RB_MASK;

      output = frag_more (isize);
      break;

    case INST_TYPE_RD_R2:
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg1);  /* Get rd.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg1 = 0;
        }
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg2);  /* Get r2.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg2 = 0;
        }

      /* Check for spl registers.  */
      if (check_spl_reg (&reg1))
        as_fatal (_("Cannot use special register with this instruction"));
      if (check_spl_reg (&reg2))
        as_fatal (_("Cannot use special register with this instruction"));

      inst |= (reg1 << RD_LOW) & RD_MASK;
      inst |= (reg2 << RB_LOW) & RB_MASK;
      output = frag_more (isize);
      break;

    case INST_TYPE_R1_IMM:
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg1);  /* Get r1.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg1 = 0;
        }
      if (strcmp (op_end, ""))
        op_end = parse_imm (op_end + 1, & exp, MIN_IMM, MAX_IMM);
      else
        as_fatal (_("Error in statement syntax"));

      /* Check for spl registers.  */
      if (check_spl_reg (&reg1))
        as_fatal (_("Cannot use special register with this instruction"));

      if (exp.X_op != O_constant)
	{
          char *opc = NULL;
          relax_substateT subtype;

	  if (exp.X_md != 0)
	    subtype = get_imm_otype(exp.X_md);
	  else
	    subtype = opcode->inst_offset_type;

	  output = frag_var (rs_machine_dependent,
			     isize * 2, /* maxm of 2 words.  */
			     isize,     /* minm of 1 word.  */
			     subtype,   /* PC-relative or not.  */
			     exp.X_add_symbol,
			     exp.X_add_number,
			     opc);
	  immed = 0;
	}
      else
	{
          output = frag_more (isize);
          immed = exp.X_add_number;
        }

      temp = immed & 0xFFFF8000;
      if ((temp != 0) && (temp != 0xFFFF8000))
	{
          /* Needs an immediate inst.  */
	  opcode1
	    = (struct op_code_struct *) str_hash_find (opcode_hash_control,
						       "imm");
          if (opcode1 == NULL)
            {
              as_bad (_("unknown opcode \"%s\""), "imm");
	      return;
            }

          inst1 = opcode1->bit_sequence;
          inst1 |= ((immed & 0xFFFF0000) >> 16) & IMM_MASK;
          output[0] = INST_BYTE0 (inst1);
          output[1] = INST_BYTE1 (inst1);
          output[2] = INST_BYTE2 (inst1);
          output[3] = INST_BYTE3 (inst1);
          output = frag_more (isize);
        }

      inst |= (reg1 << RA_LOW) & RA_MASK;
      inst |= (immed << IMM_LOW) & IMM_MASK;
      break;

    case INST_TYPE_RD_IMM:
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg1);  /* Get rd.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg1 = 0;
        }
      if (strcmp (op_end, ""))
        op_end = parse_imm (op_end + 1, & exp, MIN_IMM, MAX_IMM);
      else
        as_fatal (_("Error in statement syntax"));

      /* Check for spl registers.  */
      if (check_spl_reg (&reg1))
        as_fatal (_("Cannot use special register with this instruction"));

      if (exp.X_op != O_constant)
	{
          char *opc = NULL;
          relax_substateT subtype;

	  if (exp.X_md != 0)
	    subtype = get_imm_otype(exp.X_md);
	  else
	    subtype = opcode->inst_offset_type;

          output = frag_var (rs_machine_dependent,
			     isize * 2, /* maxm of 2 words.  */
			     isize,     /* minm of 1 word.  */
			     subtype,   /* PC-relative or not.  */
			     exp.X_add_symbol,
			     exp.X_add_number,
			     opc);
          immed = 0;
	}
      else
	{
          output = frag_more (isize);
          immed = exp.X_add_number;
        }

      temp = immed & 0xFFFF8000;
      if ((temp != 0) && (temp != 0xFFFF8000))
	{
          /* Needs an immediate inst.  */
          opcode1
	    = (struct op_code_struct *) str_hash_find (opcode_hash_control,
						       "imm");
          if (opcode1 == NULL)
            {
              as_bad (_("unknown opcode \"%s\""), "imm");
              return;
            }

          inst1 = opcode1->bit_sequence;
          inst1 |= ((immed & 0xFFFF0000) >> 16) & IMM_MASK;
          output[0] = INST_BYTE0 (inst1);
          output[1] = INST_BYTE1 (inst1);
          output[2] = INST_BYTE2 (inst1);
          output[3] = INST_BYTE3 (inst1);
          output = frag_more (isize);
        }

      inst |= (reg1 << RD_LOW) & RD_MASK;
      inst |= (immed << IMM_LOW) & IMM_MASK;
      break;

    case INST_TYPE_R2:
      if (strcmp (op_end, ""))
        op_end = parse_reg (op_end + 1, &reg2);  /* Get r2.  */
      else
	{
          as_fatal (_("Error in statement syntax"));
          reg2 = 0;
        }

      /* Check for spl registers.  */
      if (check_spl_reg (&reg2))
        as_fatal (_("Cannot use special register with this instruction"));

      inst |= (reg2 << RB_LOW) & RB_MASK;
      output = frag_more (isize);
      break;

    case INST_TYPE_IMM:
      if (streq (name, "imm"))
        as_fatal (_("An IMM instruction should not be present in the .s file"));

      op_end = parse_imm (op_end + 1, & exp, MIN_IMM, MAX_IMM);

      if (exp.X_op != O_constant)
	{
          char *opc = NULL;
          relax_substateT subtype;

	  if (exp.X_md != 0)
	    subtype = get_imm_otype(exp.X_md);
	  else
	    subtype = opcode->inst_offset_type;

          output = frag_var (rs_machine_dependent,
			     isize * 2, /* maxm of 2 words.  */
			     isize,     /* minm of 1 word.  */
			     subtype,   /* PC-relative or not.  */
			     exp.X_add_symbol,
			     exp.X_add_number,
			     opc);
          immed = 0;
        }
      else
	{
          output = frag_more (isize);
          immed = exp.X_add_number;
        }


      temp = immed & 0xFFFF8000;
      if ((temp != 0) && (temp != 0xFFFF8000))
	{
          /* Needs an immediate inst.  */
          opcode1
	    = (struct op_code_struct *) str_hash_find (opcode_hash_control,
						       "imm");
          if (opcode1 == NULL)
            {
              as_bad (_("unknown opcode \"%s\""), "imm");
              return;
            }

          inst1 = opcode1->bit_sequence;
          inst1 |= ((immed & 0xFFFF0000) >> 16) & IMM_MASK;
          output[0] = INST_BYTE0 (inst1);
          output[1] = INST_BYTE1 (inst1);
          output[2] = INST_BYTE2 (inst1);
          output[3] = INST_BYTE3 (inst1);
          output = frag_more (isize);
        }
      inst |= (immed << IMM_LOW) & IMM_MASK;
      break;

    case INST_TYPE_NONE:
      output = frag_more (isize);
      break;

    case INST_TYPE_IMM5:
      if (strcmp(op_end, ""))
        op_end = parse_imm (op_end + 1, & exp, MIN_IMM5, MAX_IMM5);
      else
        as_fatal(_("Error in statement syntax"));
      if (exp.X_op != O_constant) {
        as_warn(_("Symbol used as immediate for mbar instruction"));
      } else {
        output = frag_more (isize);
        immed = exp.X_add_number;
      }
      if (immed != (immed % 32)) {
        as_warn(_("Immediate value for mbar > 32. using <value %% 32>"));
        immed = immed % 32;
      }
      inst |= (immed << IMM_MBAR);
      break;

    default:
      as_fatal (_("unimplemented opcode \"%s\""), name);
    }

  /* Drop whitespace after all the operands have been parsed.  */
  while (ISSPACE (* op_end))
    op_end ++;

  /* Give warning message if the insn has more operands than required.  */
  if (strcmp (op_end, opcode->name) && strcmp (op_end, ""))
    as_warn (_("ignoring operands: %s "), op_end);

  output[0] = INST_BYTE0 (inst);
  output[1] = INST_BYTE1 (inst);
  output[2] = INST_BYTE2 (inst);
  output[3] = INST_BYTE3 (inst);

#ifdef OBJ_ELF
  dwarf2_emit_insn (4);
#endif
}

symbolS *
md_undefined_symbol (char * name ATTRIBUTE_UNUSED)
{
  return NULL;
}

/* Turn a string in input_line_pointer into a floating point constant of type
   type, and store the appropriate bytes in *litP.  The number of LITTLENUMS
   emitted is stored in *sizeP.  An error message is returned, or NULL on OK.*/

const char *
md_atof (int type, char * litP, int * sizeP)
{
  int prec;
  LITTLENUM_TYPE words[MAX_LITTLENUMS];
  int    i;
  char * t;

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
      return _("Bad call to MD_NTOF()");
    }

  t = atof_ieee (input_line_pointer, type, words);

  if (t)
    input_line_pointer = t;

  *sizeP = prec * sizeof (LITTLENUM_TYPE);

  if (! target_big_endian)
    {
      for (i = prec - 1; i >= 0; i--)
        {
          md_number_to_chars (litP, (valueT) words[i],
                              sizeof (LITTLENUM_TYPE));
          litP += sizeof (LITTLENUM_TYPE);
        }
    }
  else
    for (i = 0; i < prec; i++)
      {
        md_number_to_chars (litP, (valueT) words[i],
                            sizeof (LITTLENUM_TYPE));
        litP += sizeof (LITTLENUM_TYPE);
      }

  return NULL;
}

const char * md_shortopts = "";

struct option md_longopts[] =
{
  {"EB", no_argument, NULL, OPTION_EB},
  {"EL", no_argument, NULL, OPTION_EL},
  { NULL,          no_argument, NULL, 0}
};

size_t md_longopts_size = sizeof (md_longopts);

int md_short_jump_size;

void
md_create_short_jump (char * ptr ATTRIBUTE_UNUSED,
		      addressT from_Nddr ATTRIBUTE_UNUSED,
		      addressT to_Nddr ATTRIBUTE_UNUSED,
		      fragS * frag ATTRIBUTE_UNUSED,
		      symbolS * to_symbol ATTRIBUTE_UNUSED)
{
  as_fatal (_("failed sanity check: short_jump"));
}

void
md_create_long_jump (char * ptr ATTRIBUTE_UNUSED,
		     addressT from_Nddr ATTRIBUTE_UNUSED,
		     addressT to_Nddr ATTRIBUTE_UNUSED,
		     fragS * frag ATTRIBUTE_UNUSED,
		     symbolS * to_symbol ATTRIBUTE_UNUSED)
{
  as_fatal (_("failed sanity check: long_jump"));
}

/* Called after relaxing, change the frags so they know how big they are.  */

void
md_convert_frag (bfd * abfd ATTRIBUTE_UNUSED,
	         segT sec ATTRIBUTE_UNUSED,
		 fragS * fragP)
{
  fixS *fixP;

  switch (fragP->fr_subtype)
    {
    case UNDEFINED_PC_OFFSET:
      fix_new (fragP, fragP->fr_fix, INST_WORD_SIZE * 2, fragP->fr_symbol,
	       fragP->fr_offset, true, BFD_RELOC_64_PCREL);
      fragP->fr_fix += INST_WORD_SIZE * 2;
      fragP->fr_var = 0;
      break;
    case DEFINED_ABS_SEGMENT:
      if (fragP->fr_symbol == GOT_symbol)
        fix_new (fragP, fragP->fr_fix, INST_WORD_SIZE * 2, fragP->fr_symbol,
	         fragP->fr_offset, true, BFD_RELOC_MICROBLAZE_64_GOTPC);
      else
        fix_new (fragP, fragP->fr_fix, INST_WORD_SIZE * 2, fragP->fr_symbol,
	         fragP->fr_offset, false, BFD_RELOC_64);
      fragP->fr_fix += INST_WORD_SIZE * 2;
      fragP->fr_var = 0;
      break;
    case DEFINED_RO_SEGMENT:
      fix_new (fragP, fragP->fr_fix, INST_WORD_SIZE, fragP->fr_symbol,
	       fragP->fr_offset, false, BFD_RELOC_MICROBLAZE_32_ROSDA);
      fragP->fr_fix += INST_WORD_SIZE;
      fragP->fr_var = 0;
      break;
    case DEFINED_RW_SEGMENT:
      fix_new (fragP, fragP->fr_fix, INST_WORD_SIZE, fragP->fr_symbol,
	       fragP->fr_offset, false, BFD_RELOC_MICROBLAZE_32_RWSDA);
      fragP->fr_fix += INST_WORD_SIZE;
      fragP->fr_var = 0;
      break;
    case DEFINED_PC_OFFSET:
      fix_new (fragP, fragP->fr_fix, INST_WORD_SIZE, fragP->fr_symbol,
	       fragP->fr_offset, true, BFD_RELOC_MICROBLAZE_32_LO_PCREL);
      fragP->fr_fix += INST_WORD_SIZE;
      fragP->fr_var = 0;
      break;
    case LARGE_DEFINED_PC_OFFSET:
      fix_new (fragP, fragP->fr_fix, INST_WORD_SIZE * 2, fragP->fr_symbol,
	       fragP->fr_offset, true, BFD_RELOC_64_PCREL);
      fragP->fr_fix += INST_WORD_SIZE * 2;
      fragP->fr_var = 0;
      break;
    case GOT_OFFSET:
      fix_new (fragP, fragP->fr_fix, INST_WORD_SIZE * 2, fragP->fr_symbol,
	       fragP->fr_offset, false, BFD_RELOC_MICROBLAZE_64_GOT);
      fragP->fr_fix += INST_WORD_SIZE * 2;
      fragP->fr_var = 0;
      break;
    case TEXT_OFFSET:
      fix_new (fragP, fragP->fr_fix, INST_WORD_SIZE * 2, fragP->fr_symbol,
	       fragP->fr_offset, false, BFD_RELOC_MICROBLAZE_64_TEXTREL);
      fragP->fr_fix += INST_WORD_SIZE * 2;
      fragP->fr_var = 0;
      break;
    case TEXT_PC_OFFSET:
      fix_new (fragP, fragP->fr_fix, INST_WORD_SIZE * 2, fragP->fr_symbol,
	       fragP->fr_offset, false, BFD_RELOC_MICROBLAZE_64_TEXTPCREL);
      fragP->fr_fix += INST_WORD_SIZE * 2;
      fragP->fr_var = 0;
      break;
    case PLT_OFFSET:
      fixP = fix_new (fragP, fragP->fr_fix, INST_WORD_SIZE * 2, fragP->fr_symbol,
	              fragP->fr_offset, true, BFD_RELOC_MICROBLAZE_64_PLT);
      /* fixP->fx_plt = 1; */
      (void) fixP;
      fragP->fr_fix += INST_WORD_SIZE * 2;
      fragP->fr_var = 0;
      break;
    case GOTOFF_OFFSET:
      fix_new (fragP, fragP->fr_fix, INST_WORD_SIZE * 2, fragP->fr_symbol,
	       fragP->fr_offset, false, BFD_RELOC_MICROBLAZE_64_GOTOFF);
      fragP->fr_fix += INST_WORD_SIZE * 2;
      fragP->fr_var = 0;
      break;
    case TLSGD_OFFSET:
      fix_new (fragP, fragP->fr_fix, INST_WORD_SIZE * 2, fragP->fr_symbol,
	       fragP->fr_offset, false, BFD_RELOC_MICROBLAZE_64_TLSGD);
      fragP->fr_fix += INST_WORD_SIZE * 2;
      fragP->fr_var = 0;
      break;
    case TLSLD_OFFSET:
      fix_new (fragP, fragP->fr_fix, INST_WORD_SIZE * 2, fragP->fr_symbol,
	       fragP->fr_offset, false, BFD_RELOC_MICROBLAZE_64_TLSLD);
      fragP->fr_fix += INST_WORD_SIZE * 2;
      fragP->fr_var = 0;
      break;
    case TLSDTPREL_OFFSET:
      fix_new (fragP, fragP->fr_fix, INST_WORD_SIZE * 2, fragP->fr_symbol,
	       fragP->fr_offset, false, BFD_RELOC_MICROBLAZE_64_TLSDTPREL);
      fragP->fr_fix += INST_WORD_SIZE * 2;
      fragP->fr_var = 0;
      break;

    default:
      abort ();
    }
}

/* Applies the desired value to the specified location.
   Also sets up addends for 'rela' type relocations.  */
void
md_apply_fix (fixS *   fixP,
	      valueT * valp,
	      segT     segment)
{
  char *       buf  = fixP->fx_where + &fixP->fx_frag->fr_literal[0];
  const char *       file = fixP->fx_file ? fixP->fx_file : _("unknown");
  const char * symname;
  /* Note: use offsetT because it is signed, valueT is unsigned.  */
  offsetT      val  = (offsetT) * valp;
  int          i;
  struct op_code_struct * opcode1;
  unsigned long inst1;

  symname = fixP->fx_addsy ? S_GET_NAME (fixP->fx_addsy) : _("<unknown>");

  /* fixP->fx_offset is supposed to be set up correctly for all
     symbol relocations.  */
  if (fixP->fx_addsy == NULL)
    {
      if (!fixP->fx_pcrel)
        fixP->fx_offset = val; /* Absolute relocation.  */
      else
        fprintf (stderr, "NULL symbol PC-relative relocation? offset = %08x, val = %08x\n",
                 (unsigned int) fixP->fx_offset, (unsigned int) val);
    }

  /* If we aren't adjusting this fixup to be against the section
     symbol, we need to adjust the value.  */
  if (fixP->fx_addsy != NULL)
    {
      if (S_IS_WEAK (fixP->fx_addsy)
	  || (symbol_used_in_reloc_p (fixP->fx_addsy)
	      && (((bfd_section_flags (S_GET_SEGMENT (fixP->fx_addsy))
		    & SEC_LINK_ONCE) != 0)
		  || startswith (segment_name (S_GET_SEGMENT (fixP->fx_addsy)),
				 ".gnu.linkonce"))))
	{
	  val -= S_GET_VALUE (fixP->fx_addsy);
	  if (val != 0 && ! fixP->fx_pcrel)
            {
              /* In this case, the bfd_install_relocation routine will
                 incorrectly add the symbol value back in.  We just want
                 the addend to appear in the object file.
	         FIXME: If this makes VALUE zero, we're toast.  */
              val -= S_GET_VALUE (fixP->fx_addsy);
            }
	}
    }

  /* If the fix is relative to a symbol which is not defined, or not
     in the same segment as the fix, we cannot resolve it here.  */
  /* fixP->fx_addsy is NULL if valp contains the entire relocation.  */
  if (fixP->fx_addsy != NULL
      && (!S_IS_DEFINED (fixP->fx_addsy)
          || (S_GET_SEGMENT (fixP->fx_addsy) != segment)))
    {
      fixP->fx_done = 0;
#ifdef OBJ_ELF
      /* For ELF we can just return and let the reloc that will be generated
         take care of everything.  For COFF we still have to insert 'val'
         into the insn since the addend field will be ignored.  */
      /* return; */
#endif
    }
  /* All fixups in the text section must be handled in the linker.  */
  else if (segment->flags & SEC_CODE)
    fixP->fx_done = 0;
  else if (!fixP->fx_pcrel && fixP->fx_addsy != NULL)
    fixP->fx_done = 0;
  else
    fixP->fx_done = 1;

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_MICROBLAZE_32_LO:
    case BFD_RELOC_MICROBLAZE_32_LO_PCREL:
      if (target_big_endian)
	{
	  buf[2] |= ((val >> 8) & 0xff);
	  buf[3] |= (val & 0xff);
	}
      else
	{
	  buf[1] |= ((val >> 8) & 0xff);
	  buf[0] |= (val & 0xff);
	}
      break;
    case BFD_RELOC_MICROBLAZE_32_ROSDA:
    case BFD_RELOC_MICROBLAZE_32_RWSDA:
      /* Don't do anything if the symbol is not defined.  */
      if (fixP->fx_addsy == NULL || S_IS_DEFINED (fixP->fx_addsy))
	{
	  if (((val & 0xFFFF8000) != 0) && ((val & 0xFFFF8000) != 0xFFFF8000))
	    as_bad_where (file, fixP->fx_line,
			  _("pcrel for branch to %s too far (0x%x)"),
			  symname, (int) val);
	  if (target_big_endian)
	    {
	      buf[2] |= ((val >> 8) & 0xff);
	      buf[3] |= (val & 0xff);
	    }
	  else
	    {
	      buf[1] |= ((val >> 8) & 0xff);
	      buf[0] |= (val & 0xff);
	    }
	}
      break;
    case BFD_RELOC_32:
    case BFD_RELOC_RVA:
    case BFD_RELOC_32_PCREL:
    case BFD_RELOC_MICROBLAZE_32_SYM_OP_SYM:
      /* Don't do anything if the symbol is not defined.  */
      if (fixP->fx_addsy == NULL || S_IS_DEFINED (fixP->fx_addsy))
	{
	  if (target_big_endian)
	    {
	      buf[0] |= ((val >> 24) & 0xff);
	      buf[1] |= ((val >> 16) & 0xff);
	      buf[2] |= ((val >> 8) & 0xff);
	      buf[3] |= (val & 0xff);
	    }
	  else
	    {
	      buf[3] |= ((val >> 24) & 0xff);
	      buf[2] |= ((val >> 16) & 0xff);
	      buf[1] |= ((val >> 8) & 0xff);
	      buf[0] |= (val & 0xff);
	    }
	}
      break;
    case BFD_RELOC_64_PCREL:
    case BFD_RELOC_64:
    case BFD_RELOC_MICROBLAZE_64_TEXTREL:
      /* Add an imm instruction.  First save the current instruction.  */
      for (i = 0; i < INST_WORD_SIZE; i++)
	buf[i + INST_WORD_SIZE] = buf[i];

      /* Generate the imm instruction.  */
      opcode1
	= (struct op_code_struct *) str_hash_find (opcode_hash_control, "imm");
      if (opcode1 == NULL)
	{
	  as_bad (_("unknown opcode \"%s\""), "imm");
	  return;
	}

      inst1 = opcode1->bit_sequence;
      if (fixP->fx_addsy == NULL || S_IS_DEFINED (fixP->fx_addsy))
	inst1 |= ((val & 0xFFFF0000) >> 16) & IMM_MASK;

      buf[0] = INST_BYTE0 (inst1);
      buf[1] = INST_BYTE1 (inst1);
      buf[2] = INST_BYTE2 (inst1);
      buf[3] = INST_BYTE3 (inst1);

      /* Add the value only if the symbol is defined.  */
      if (fixP->fx_addsy == NULL || S_IS_DEFINED (fixP->fx_addsy))
	{
	  if (target_big_endian)
	    {
	      buf[6] |= ((val >> 8) & 0xff);
	      buf[7] |= (val & 0xff);
	    }
	  else
	    {
	      buf[5] |= ((val >> 8) & 0xff);
	      buf[4] |= (val & 0xff);
	    }
	}
      break;

    case BFD_RELOC_MICROBLAZE_64_TLSDTPREL:
    case BFD_RELOC_MICROBLAZE_64_TLSGD:
    case BFD_RELOC_MICROBLAZE_64_TLSLD:
      S_SET_THREAD_LOCAL (fixP->fx_addsy);
      /* Fall through.  */

    case BFD_RELOC_MICROBLAZE_64_GOTPC:
    case BFD_RELOC_MICROBLAZE_64_GOT:
    case BFD_RELOC_MICROBLAZE_64_PLT:
    case BFD_RELOC_MICROBLAZE_64_GOTOFF:
    case BFD_RELOC_MICROBLAZE_64_TEXTPCREL:
      /* Add an imm instruction.  First save the current instruction.  */
      for (i = 0; i < INST_WORD_SIZE; i++)
	buf[i + INST_WORD_SIZE] = buf[i];

      /* Generate the imm instruction.  */
      opcode1
	= (struct op_code_struct *) str_hash_find (opcode_hash_control, "imm");
      if (opcode1 == NULL)
	{
	  as_bad (_("unknown opcode \"%s\""), "imm");
	  return;
	}

      inst1 = opcode1->bit_sequence;

      /* We can fixup call to a defined non-global address
	 within the same section only.  */
      buf[0] = INST_BYTE0 (inst1);
      buf[1] = INST_BYTE1 (inst1);
      buf[2] = INST_BYTE2 (inst1);
      buf[3] = INST_BYTE3 (inst1);
      return;

    default:
      break;
    }

  if (fixP->fx_addsy == NULL)
    {
      /* This fixup has been resolved.  Create a reloc in case the linker
	 moves code around due to relaxing.  */
      if (fixP->fx_r_type == BFD_RELOC_64_PCREL)
	fixP->fx_r_type = BFD_RELOC_MICROBLAZE_64_NONE;
      else
	fixP->fx_r_type = BFD_RELOC_NONE;
      fixP->fx_addsy = section_symbol (absolute_section);
    }
  return;
}

void
md_operand (expressionS * expressionP)
{
  /* Ignore leading hash symbol, if present.  */
  if (*input_line_pointer == '#')
    {
      input_line_pointer ++;
      expression (expressionP);
    }
}

/* Called just before address relaxation, return the length
   by which a fragment must grow to reach it's destination.  */

int
md_estimate_size_before_relax (fragS * fragP,
			       segT segment_type)
{
  sbss_segment = bfd_get_section_by_name (stdoutput, ".sbss");
  sbss2_segment = bfd_get_section_by_name (stdoutput, ".sbss2");
  sdata_segment = bfd_get_section_by_name (stdoutput, ".sdata");
  sdata2_segment = bfd_get_section_by_name (stdoutput, ".sdata2");

  switch (fragP->fr_subtype)
    {
    case INST_PC_OFFSET:
      /* Used to be a PC-relative branch.  */
      if (!fragP->fr_symbol)
        {
          /* We know the abs value: Should never happen.  */
          as_bad (_("Absolute PC-relative value in relaxation code.  Assembler error....."));
          abort ();
        }
      else if (S_GET_SEGMENT (fragP->fr_symbol) == segment_type &&
               !S_IS_WEAK (fragP->fr_symbol))
        {
          fragP->fr_subtype = DEFINED_PC_OFFSET;
          /* Don't know now whether we need an imm instruction.  */
          fragP->fr_var = INST_WORD_SIZE;
        }
      else if (S_IS_DEFINED (fragP->fr_symbol)
	       && (((S_GET_SEGMENT (fragP->fr_symbol))->flags & SEC_CODE) == 0))
        {
          /* Cannot have a PC-relative branch to a diff segment.  */
          as_bad (_("PC relative branch to label %s which is not in the instruction space"),
		  S_GET_NAME (fragP->fr_symbol));
          fragP->fr_subtype = UNDEFINED_PC_OFFSET;
          fragP->fr_var = INST_WORD_SIZE*2;
        }
      else
	{
	  fragP->fr_subtype = UNDEFINED_PC_OFFSET;
	  fragP->fr_var = INST_WORD_SIZE*2;
	}
      break;

    case INST_NO_OFFSET:
    case TEXT_OFFSET:
      /* Used to be a reference to somewhere which was unknown.  */
      if (fragP->fr_symbol)
        {
	  if (fragP->fr_opcode == NULL)
	    {
	      /* Used as an absolute value.  */
	      if (fragP->fr_subtype == INST_NO_OFFSET)
	        fragP->fr_subtype = DEFINED_ABS_SEGMENT;
	      /* Variable part does not change.  */
	      fragP->fr_var = INST_WORD_SIZE*2;
	    }
	  else if (streq (fragP->fr_opcode, str_microblaze_ro_anchor))
	    {
              /* It is accessed using the small data read only anchor.  */
              if ((S_GET_SEGMENT (fragP->fr_symbol) == bfd_com_section_ptr)
		  || (S_GET_SEGMENT (fragP->fr_symbol) == sdata2_segment)
		  || (S_GET_SEGMENT (fragP->fr_symbol) == sbss2_segment)
		  || (! S_IS_DEFINED (fragP->fr_symbol)))
		{
                  fragP->fr_subtype = DEFINED_RO_SEGMENT;
                  fragP->fr_var = INST_WORD_SIZE;
                }
	      else
		{
                  /* Variable not in small data read only segment accessed
		     using small data read only anchor.  */
                  const char *file = fragP->fr_file ? fragP->fr_file : _("unknown");

                  as_bad_where (file, fragP->fr_line,
                                _("Variable is accessed using small data read "
				  "only anchor, but it is not in the small data "
			          "read only section"));
                  fragP->fr_subtype = DEFINED_RO_SEGMENT;
                  fragP->fr_var = INST_WORD_SIZE;
                }
            }
	  else if (streq (fragP->fr_opcode, str_microblaze_rw_anchor))
	    {
              if ((S_GET_SEGMENT (fragP->fr_symbol) == bfd_com_section_ptr)
		  || (S_GET_SEGMENT (fragP->fr_symbol) == sdata_segment)
		  || (S_GET_SEGMENT (fragP->fr_symbol) == sbss_segment)
		  || (!S_IS_DEFINED (fragP->fr_symbol)))
	        {
                  /* It is accessed using the small data read write anchor.  */
                  fragP->fr_subtype = DEFINED_RW_SEGMENT;
                  fragP->fr_var = INST_WORD_SIZE;
                }
	      else
		{
                  const char *file = fragP->fr_file ? fragP->fr_file : _("unknown");

                  as_bad_where (file, fragP->fr_line,
                                _("Variable is accessed using small data read "
				  "write anchor, but it is not in the small data "
				  "read write section"));
                  fragP->fr_subtype = DEFINED_RW_SEGMENT;
                  fragP->fr_var = INST_WORD_SIZE;
                }
            }
          else
	    {
              as_bad (_("Incorrect fr_opcode value in frag.  Internal error....."));
              abort ();
            }
	}
      else
	{
	  /* We know the abs value: Should never happen.  */
	  as_bad (_("Absolute value in relaxation code.  Assembler error....."));
	  abort ();
	}
      break;

    case UNDEFINED_PC_OFFSET:
    case LARGE_DEFINED_PC_OFFSET:
    case DEFINED_ABS_SEGMENT:
    case GOT_OFFSET:
    case PLT_OFFSET:
    case GOTOFF_OFFSET:
    case TEXT_PC_OFFSET:
    case TLSGD_OFFSET:
    case TLSLD_OFFSET:
    case TLSTPREL_OFFSET:
    case TLSDTPREL_OFFSET:
      fragP->fr_var = INST_WORD_SIZE*2;
      break;
    case DEFINED_RO_SEGMENT:
    case DEFINED_RW_SEGMENT:
    case DEFINED_PC_OFFSET:
    case TLSDTPMOD_OFFSET:
      fragP->fr_var = INST_WORD_SIZE;
      break;
    default:
      abort ();
    }

  return fragP->fr_var;
}

/* Put number into target byte order.  */

void
md_number_to_chars (char * ptr, valueT use, int nbytes)
{
  if (target_big_endian)
    number_to_chars_bigendian (ptr, use, nbytes);
  else
    number_to_chars_littleendian (ptr, use, nbytes);
}

/* Round up a section size to the appropriate boundary.  */

valueT
md_section_align (segT segment ATTRIBUTE_UNUSED, valueT size)
{
  return size;			/* Byte alignment is fine.  */
}


/* The location from which a PC relative jump should be calculated,
   given a PC relative reloc.  */

long
md_pcrel_from_section (fixS * fixp, segT sec ATTRIBUTE_UNUSED)
{
#ifdef OBJ_ELF
  /* If the symbol is undefined or defined in another section
     we leave the add number alone for the linker to fix it later.
     Only account for the PC pre-bump (No PC-pre-bump on the Microblaze). */

  if (fixp->fx_addsy != (symbolS *) NULL
      && (!S_IS_DEFINED (fixp->fx_addsy)
          || (S_GET_SEGMENT (fixp->fx_addsy) != sec)))
    return 0;
  else
    {
      /* The case where we are going to resolve things... */
      if (fixp->fx_r_type == BFD_RELOC_64_PCREL)
        return  fixp->fx_where + fixp->fx_frag->fr_address + INST_WORD_SIZE;
      else
        return  fixp->fx_where + fixp->fx_frag->fr_address;
    }
#endif
}


#define F(SZ,PCREL)		(((SZ) << 1) + (PCREL))
#define MAP(SZ,PCREL,TYPE)	case F (SZ, PCREL): code = (TYPE); break

arelent *
tc_gen_reloc (asection * section ATTRIBUTE_UNUSED, fixS * fixp)
{
  arelent * rel;
  bfd_reloc_code_real_type code;

  switch (fixp->fx_r_type)
    {
    case BFD_RELOC_NONE:
    case BFD_RELOC_MICROBLAZE_64_NONE:
    case BFD_RELOC_32:
    case BFD_RELOC_MICROBLAZE_32_LO:
    case BFD_RELOC_MICROBLAZE_32_LO_PCREL:
    case BFD_RELOC_RVA:
    case BFD_RELOC_64:
    case BFD_RELOC_64_PCREL:
    case BFD_RELOC_MICROBLAZE_32_ROSDA:
    case BFD_RELOC_MICROBLAZE_32_RWSDA:
    case BFD_RELOC_MICROBLAZE_32_SYM_OP_SYM:
    case BFD_RELOC_MICROBLAZE_64_GOTPC:
    case BFD_RELOC_MICROBLAZE_64_GOT:
    case BFD_RELOC_MICROBLAZE_64_PLT:
    case BFD_RELOC_MICROBLAZE_64_GOTOFF:
    case BFD_RELOC_MICROBLAZE_32_GOTOFF:
    case BFD_RELOC_MICROBLAZE_64_TLSGD:
    case BFD_RELOC_MICROBLAZE_64_TLSLD:
    case BFD_RELOC_MICROBLAZE_32_TLSDTPMOD:
    case BFD_RELOC_MICROBLAZE_32_TLSDTPREL:
    case BFD_RELOC_MICROBLAZE_64_TLSDTPREL:
    case BFD_RELOC_MICROBLAZE_64_TLSGOTTPREL:
    case BFD_RELOC_MICROBLAZE_64_TLSTPREL:
    case BFD_RELOC_MICROBLAZE_64_TEXTPCREL:
    case BFD_RELOC_MICROBLAZE_64_TEXTREL:
      code = fixp->fx_r_type;
      break;

    default:
      switch (F (fixp->fx_size, fixp->fx_pcrel))
        {
          MAP (1, 0, BFD_RELOC_8);
          MAP (2, 0, BFD_RELOC_16);
          MAP (4, 0, BFD_RELOC_32);
          MAP (1, 1, BFD_RELOC_8_PCREL);
          MAP (2, 1, BFD_RELOC_16_PCREL);
          MAP (4, 1, BFD_RELOC_32_PCREL);
        default:
          code = fixp->fx_r_type;
          as_bad (_("Can not do %d byte %srelocation"),
                  fixp->fx_size,
                  fixp->fx_pcrel ? _("pc-relative ") : "");
        }
      break;
    }

  rel = XNEW (arelent);
  rel->sym_ptr_ptr = XNEW (asymbol *);

  if (code == BFD_RELOC_MICROBLAZE_32_SYM_OP_SYM)
    *rel->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_subsy);
  else
    *rel->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);

  rel->address = fixp->fx_frag->fr_address + fixp->fx_where;
  /* Always pass the addend along!  */
  rel->addend = fixp->fx_offset;
  rel->howto = bfd_reloc_type_lookup (stdoutput, code);

  if (rel->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
                    _("Cannot represent relocation type %s"),
                    bfd_get_reloc_code_name (code));

      /* Set howto to a garbage value so that we can keep going.  */
      rel->howto = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_32);
      gas_assert (rel->howto != NULL);
    }
  return rel;
}

int
md_parse_option (int c, const char * arg ATTRIBUTE_UNUSED)
{
  switch (c)
    {
    case OPTION_EB:
      target_big_endian = 1;
      break;
    case OPTION_EL:
      target_big_endian = 0;
      break;
    default:
      return 0;
    }
  return 1;
}

void
md_show_usage (FILE * stream ATTRIBUTE_UNUSED)
{
  /*  fprintf(stream, _("\
      MicroBlaze options:\n\
      -noSmall         Data in the comm and data sections do not go into the small data section\n")); */
}


/* Create a fixup for a cons expression.  If parse_cons_expression_microblaze
   found a machine specific op in an expression,
   then we create relocs accordingly.  */

void
cons_fix_new_microblaze (fragS * frag,
			 int where,
			 int size,
			 expressionS *exp,
			 bfd_reloc_code_real_type r)
{
  if ((exp->X_op == O_subtract) && (exp->X_add_symbol) &&
      (exp->X_op_symbol) && (now_seg != absolute_section) && (size == 4)
      && (!S_IS_LOCAL (exp->X_op_symbol)))
    r = BFD_RELOC_MICROBLAZE_32_SYM_OP_SYM;
  else if (exp->X_md == IMM_GOTOFF && exp->X_op == O_symbol_rva)
    {
      exp->X_op = O_symbol;
      r = BFD_RELOC_MICROBLAZE_32_GOTOFF;
    }
  else
    {
      switch (size)
        {
        case 1:
          r = BFD_RELOC_8;
          break;
        case 2:
          r = BFD_RELOC_16;
          break;
        case 4:
          r = BFD_RELOC_32;
          break;
        case 8:
          r = BFD_RELOC_64;
          break;
        default:
          as_bad (_("unsupported BFD relocation size %u"), size);
          r = BFD_RELOC_32;
          break;
        }
    }
  fix_new_exp (frag, where, size, exp, 0, r);
}
