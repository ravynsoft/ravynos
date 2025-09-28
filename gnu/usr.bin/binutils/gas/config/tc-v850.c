/* tc-v850.c -- Assembler code for the NEC V850
   Copyright (C) 1996-2023 Free Software Foundation, Inc.

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
#include "safe-ctype.h"
#include "subsegs.h"
#include "opcode/v850.h"
#include "dwarf2dbg.h"

/* Sign-extend a 16-bit number.  */
#define SEXT16(x)	((((x) & 0xffff) ^ (~0x7fff)) + 0x8000)

/* Set to TRUE if we want to be pedantic about signed overflows.  */
static bool warn_signed_overflows   = false;
static bool warn_unsigned_overflows = false;

/* Non-zero if floating point insns are not being used.  */
static signed int soft_float = -1;

/* Indicates the target BFD machine number.  */
static int machine = -1;


/* Indicates the target BFD architecture.  */
enum bfd_architecture v850_target_arch = bfd_arch_v850_rh850;
const char * v850_target_format = "elf32-v850-rh850";
static flagword v850_e_flags = 0;

/* Indicates the target processor(s) for the assemble.  */
static int processor_mask = 0;

/* Structure to hold information about predefined registers.  */
struct reg_name
{
  const char *name;
  int value;
  unsigned int processors;
};

/* Generic assembler global variables which must be defined by all
   targets.  */

/* Characters which always start a comment.  */
const char comment_chars[] = "#";

/* Characters which start a comment at the beginning of a line.  */
const char line_comment_chars[] = ";#";

/* Characters which may be used to separate multiple commands on a
   single line.  */
const char line_separator_chars[] = ";";

/* Characters which are used to indicate an exponent in a floating
   point number.  */
const char EXP_CHARS[] = "eE";

/* Characters which mean that a number is a floating point constant,
   as in 0d1.0.  */
const char FLT_CHARS[] = "dD";

const relax_typeS md_relax_table[] =
{
  /* Conditional branches.(V850/V850E, max 22bit)  */
#define SUBYPTE_COND_9_22	0
  {0xfe,	 -0x100,        2, SUBYPTE_COND_9_22 + 1},
  {0x1ffffe + 2, -0x200000 + 2, 6, 0},
  /* Conditional branches.(V850/V850E, max 22bit)  */
#define SUBYPTE_SA_9_22	2
  {0xfe,         -0x100,      2, SUBYPTE_SA_9_22 + 1},
  {0x1ffffe + 4, -0x200000 + 4, 8, 0},
  /* Unconditional branches.(V850/V850E, max 22bit)  */
#define SUBYPTE_UNCOND_9_22	4
  {0xfe,     -0x100,    2, SUBYPTE_UNCOND_9_22 + 1},
  {0x1ffffe, -0x200000, 4, 0},
  /* Conditional branches.(V850E2, max 32bit)  */
#define SUBYPTE_COND_9_22_32	6
  {0xfe,     -0x100,    2, SUBYPTE_COND_9_22_32 + 1},
  {0x1fffff + 2, -0x200000 + 2, 6, SUBYPTE_COND_9_22_32 + 2},
  {0x7ffffffe, -0x80000000, 8, 0},
  /* Conditional branches.(V850E2, max 32bit)  */
#define SUBYPTE_SA_9_22_32	9
  {0xfe,     -0x100,    2, SUBYPTE_SA_9_22_32 + 1},
  {0x1ffffe + 4, -0x200000 + 4, 8, SUBYPTE_SA_9_22_32 + 2},
  {0x7ffffffe, -0x80000000, 10, 0},
  /* Unconditional branches.(V850E2, max 32bit)  */
#define SUBYPTE_UNCOND_9_22_32	12
  {0xfe,     -0x100,    2, SUBYPTE_UNCOND_9_22_32 + 1},
  {0x1ffffe, -0x200000, 4, SUBYPTE_UNCOND_9_22_32 + 2},
  {0x7ffffffe, -0x80000000, 6, 0},
  /* Conditional branches.(V850E2R max 22bit)  */
#define SUBYPTE_COND_9_17_22	15
  {0xfe,     -0x100,    2, SUBYPTE_COND_9_17_22 + 1},
  {0xfffe, -0x10000,	4, SUBYPTE_COND_9_17_22 + 2},
  {0x1ffffe + 2, -0x200000 + 2, 6, 0},
  /* Conditional branches.(V850E2R max 22bit)  */
#define SUBYPTE_SA_9_17_22	18
  {0xfe,     -0x100,    2, SUBYPTE_SA_9_17_22 + 1},
  {0xfffe, -0x10000,	4, SUBYPTE_SA_9_17_22 + 2},
  {0x1ffffe + 4, -0x200000 + 4, 8, 0},
  /* Conditional branches.(V850E2R max 32bit)  */
#define SUBYPTE_COND_9_17_22_32	21
  {0xfe,     -0x100,    2, SUBYPTE_COND_9_17_22_32 + 1},
  {0xfffe, -0x10000,	4, SUBYPTE_COND_9_17_22_32 + 2},
  {0x1ffffe + 2, -0x200000 + 2, 6, SUBYPTE_COND_9_17_22_32 + 3},
  {0x7ffffffe, -0x80000000, 8, 0},
  /* Conditional branches.(V850E2R max 32bit)  */
#define SUBYPTE_SA_9_17_22_32	25
  {0xfe,     -0x100,    2, SUBYPTE_SA_9_17_22_32 + 1},
  {0xfffe, -0x10000,	4, SUBYPTE_SA_9_17_22_32 + 2},
  {0x1ffffe + 4, -0x200000 + 4, 8, SUBYPTE_SA_9_17_22_32 + 3},
  {0x7ffffffe, -0x80000000, 10, 0},
  /* Loop.  (V850E2V4_UP, max 22-bit).  */
#define SUBYPTE_LOOP_16_22	29
  {0x0, -0x0fffe, 4, SUBYPTE_LOOP_16_22 + 1},
  {0x1ffffe + 2, -0x200000 + 2, 6, 0},
};

static int v850_relax = 0;

/* Default branch disp size 22 or 32.  */
static int default_disp_size = 22;

/* Default no using bcond17.  */
static int no_bcond17 = 0;

/* Default no using ld/st 23bit offset.  */
static int no_stld23 = 0;

/* Fixups.  */
#define MAX_INSN_FIXUPS   5

struct v850_fixup
{
  expressionS exp;
  int opindex;
  bfd_reloc_code_real_type reloc;
};

struct v850_fixup fixups[MAX_INSN_FIXUPS];
static int fc;

struct v850_seg_entry
{
  segT s;
  const char *name;
  flagword flags;
};

struct v850_seg_entry v850_seg_table[] =
{
  { NULL, ".sdata",
    SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_DATA | SEC_HAS_CONTENTS
    | SEC_SMALL_DATA },
  { NULL, ".tdata",
    SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_DATA | SEC_HAS_CONTENTS },
  { NULL, ".zdata",
    SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_DATA | SEC_HAS_CONTENTS },
  { NULL, ".sbss",
    SEC_ALLOC | SEC_SMALL_DATA },
  { NULL, ".tbss",
    SEC_ALLOC },
  { NULL, ".zbss",
    SEC_ALLOC},
  { NULL, ".rosdata",
    SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_READONLY | SEC_DATA
    | SEC_HAS_CONTENTS | SEC_SMALL_DATA },
  { NULL, ".rozdata",
    SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_READONLY | SEC_DATA
    | SEC_HAS_CONTENTS },
  { NULL, ".scommon",
    SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_DATA | SEC_HAS_CONTENTS
    | SEC_SMALL_DATA | SEC_IS_COMMON },
  { NULL, ".tcommon",
    SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_DATA | SEC_HAS_CONTENTS
    | SEC_IS_COMMON },
  { NULL, ".zcommon",
    SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_DATA | SEC_HAS_CONTENTS
    | SEC_IS_COMMON },
  { NULL, ".call_table_data",
    SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_DATA | SEC_HAS_CONTENTS },
  { NULL, ".call_table_text",
    SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_READONLY | SEC_CODE
    | SEC_HAS_CONTENTS},
  { NULL, ".bss",
    SEC_ALLOC }
};

#define SDATA_SECTION		0
#define TDATA_SECTION		1
#define ZDATA_SECTION		2
#define SBSS_SECTION		3
#define TBSS_SECTION		4
#define ZBSS_SECTION		5
#define ROSDATA_SECTION		6
#define ROZDATA_SECTION		7
#define SCOMMON_SECTION		8
#define TCOMMON_SECTION		9
#define ZCOMMON_SECTION		10
#define CALL_TABLE_DATA_SECTION	11
#define CALL_TABLE_TEXT_SECTION	12
#define BSS_SECTION		13

static void
do_v850_seg (int i, subsegT sub)
{
  struct v850_seg_entry *seg = v850_seg_table + i;

  obj_elf_section_change_hook ();

  if (seg->s != NULL)
    subseg_set (seg->s, sub);
  else
    {
      seg->s = subseg_new (seg->name, sub);
      bfd_set_section_flags (seg->s, seg->flags);
      if ((seg->flags & SEC_LOAD) == 0)
	seg_info (seg->s)->bss = 1;
    }
}

static void
v850_seg (int i)
{
  subsegT sub = get_absolute_expression ();

  do_v850_seg (i, sub);
  demand_empty_rest_of_line ();
}

static void
v850_offset (int ignore ATTRIBUTE_UNUSED)
{
  char *pfrag;
  int temp = get_absolute_expression ();

  pfrag = frag_var (rs_org, 1, 1, (relax_substateT)0, (symbolS *)0,
		    (offsetT) temp, (char *) 0);
  *pfrag = 0;

  demand_empty_rest_of_line ();
}

/* Copied from obj_elf_common() in gas/config/obj-elf.c.  */

static void
v850_comm (int area)
{
  char *name;
  char c;
  char *p;
  int temp;
  unsigned int size;
  symbolS *symbolP;
  int have_align;

  c = get_symbol_name (&name);

  /* Just after name is now '\0'.  */
  p = input_line_pointer;
  *p = c;

  SKIP_WHITESPACE ();

  if (*input_line_pointer != ',')
    {
      as_bad (_("Expected comma after symbol-name"));
      ignore_rest_of_line ();
      return;
    }

  /* Skip ','.  */
  input_line_pointer++;

  if ((temp = get_absolute_expression ()) < 0)
    {
      /* xgettext:c-format  */
      as_bad (_(".COMMon length (%d.) < 0! Ignored."), temp);
      ignore_rest_of_line ();
      return;
    }

  size = temp;
  *p = 0;
  symbolP = symbol_find_or_make (name);
  *p = c;

  if (S_IS_DEFINED (symbolP) && ! S_IS_COMMON (symbolP))
    {
      as_bad (_("Ignoring attempt to re-define symbol"));
      ignore_rest_of_line ();
      return;
    }

  if (S_GET_VALUE (symbolP) != 0)
    {
      if (S_GET_VALUE (symbolP) != size)
	/* xgettext:c-format  */
	as_warn (_("Length of .comm \"%s\" is already %ld. Not changed to %d."),
		 S_GET_NAME (symbolP), (long) S_GET_VALUE (symbolP), size);
    }

  know (symbol_get_frag (symbolP) == &zero_address_frag);

  if (*input_line_pointer != ',')
    have_align = 0;
  else
    {
      have_align = 1;
      input_line_pointer++;
      SKIP_WHITESPACE ();
    }

  if (! have_align || *input_line_pointer != '"')
    {
      if (! have_align)
	temp = 0;
      else
	{
	  temp = get_absolute_expression ();

	  if (temp < 0)
	    {
	      temp = 0;
	      as_warn (_("Common alignment negative; 0 assumed"));
	    }
	}

      if (symbol_get_obj (symbolP)->local)
	{
	  segT old_sec;
	  int old_subsec;
	  char *pfrag;
	  int align;
	  flagword applicable;

	  old_sec = now_seg;
	  old_subsec = now_subseg;

	  applicable = bfd_applicable_section_flags (stdoutput);

	  applicable &= SEC_ALLOC;

	  switch (area)
	    {
	    case SCOMMON_SECTION:
	      do_v850_seg (SBSS_SECTION, 0);
	      break;

	    case ZCOMMON_SECTION:
	      do_v850_seg (ZBSS_SECTION, 0);
	      break;

	    case TCOMMON_SECTION:
	      do_v850_seg (TBSS_SECTION, 0);
	      break;
	    }

	  if (temp)
	    {
	      /* Convert to a power of 2 alignment.  */
	      for (align = 0; (temp & 1) == 0; temp >>= 1, ++align)
		;

	      if (temp != 1)
		{
		  as_bad (_("Common alignment not a power of 2"));
		  ignore_rest_of_line ();
		  return;
		}
	    }
	  else
	    align = 0;

	  record_alignment (now_seg, align);

	  if (align)
	    frag_align (align, 0, 0);

	  switch (area)
	    {
	    case SCOMMON_SECTION:
	      if (S_GET_SEGMENT (symbolP) == v850_seg_table[SBSS_SECTION].s)
		symbol_get_frag (symbolP)->fr_symbol = 0;
	      break;

	    case ZCOMMON_SECTION:
	      if (S_GET_SEGMENT (symbolP) == v850_seg_table[ZBSS_SECTION].s)
		symbol_get_frag (symbolP)->fr_symbol = 0;
	      break;

	    case TCOMMON_SECTION:
	      if (S_GET_SEGMENT (symbolP) == v850_seg_table[TBSS_SECTION].s)
		symbol_get_frag (symbolP)->fr_symbol = 0;
	      break;

	    default:
	      abort ();
	    }

	  symbol_set_frag (symbolP, frag_now);
	  pfrag = frag_var (rs_org, 1, 1, (relax_substateT) 0, symbolP,
			    (offsetT) size, (char *) 0);
	  *pfrag = 0;
	  S_SET_SIZE (symbolP, size);

	  switch (area)
	    {
	    case SCOMMON_SECTION:
	      S_SET_SEGMENT (symbolP, v850_seg_table[SBSS_SECTION].s);
	      break;

	    case ZCOMMON_SECTION:
	      S_SET_SEGMENT (symbolP, v850_seg_table[ZBSS_SECTION].s);
	      break;

	    case TCOMMON_SECTION:
	      S_SET_SEGMENT (symbolP, v850_seg_table[TBSS_SECTION].s);
	      break;

	    default:
	      abort ();
	    }

	  S_CLEAR_EXTERNAL (symbolP);
	  obj_elf_section_change_hook ();
	  subseg_set (old_sec, old_subsec);
	}
      else
	{
	  segT   old_sec;
	  int    old_subsec;

	allocate_common:
	  old_sec = now_seg;
	  old_subsec = now_subseg;

	  S_SET_VALUE (symbolP, (valueT) size);
	  S_SET_ALIGN (symbolP, temp);
	  S_SET_EXTERNAL (symbolP);

	  switch (area)
	    {
	    case SCOMMON_SECTION:
	    case ZCOMMON_SECTION:
	    case TCOMMON_SECTION:
	      do_v850_seg (area, 0);
	      S_SET_SEGMENT (symbolP, v850_seg_table[area].s);
	      break;

	    default:
	      abort ();
	    }

	  obj_elf_section_change_hook ();
	  subseg_set (old_sec, old_subsec);
	}
    }
  else
    {
      input_line_pointer++;

      /* @@ Some use the dot, some don't.  Can we get some consistency??  */
      if (*input_line_pointer == '.')
	input_line_pointer++;

      /* @@ Some say data, some say bss.  */
      if (!startswith (input_line_pointer, "bss\"")
	  && !startswith (input_line_pointer, "data\""))
	{
	  while (*--input_line_pointer != '"')
	    ;
	  input_line_pointer--;
	  goto bad_common_segment;
	}

      while (*input_line_pointer++ != '"')
	;

      goto allocate_common;
    }

  symbol_get_bfdsym (symbolP)->flags |= BSF_OBJECT;

  demand_empty_rest_of_line ();
  return;

  {
  bad_common_segment:
    p = input_line_pointer;
    while (*p && *p != '\n')
      p++;
    c = *p;
    *p = '\0';
    as_bad (_("bad .common segment %s"), input_line_pointer + 1);
    *p = c;
    input_line_pointer = p;
    ignore_rest_of_line ();
    return;
  }
}

static void
set_machine (int number)
{
  machine = number;
  bfd_set_arch_mach (stdoutput, v850_target_arch, machine);

  switch (machine)
    {
    case 0:                SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850);    break;
    case bfd_mach_v850:    SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850);    break;
    case bfd_mach_v850e:   SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850E);   break;
    case bfd_mach_v850e1:  SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850E);   break;
    case bfd_mach_v850e2:  SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850E2);  break;
    case bfd_mach_v850e2v3:SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850E2V3); break;
    case bfd_mach_v850e3v5: SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850E3V5); break;
    }
}

static void
v850_longcode (int type)
{
  expressionS ex;

  if (! v850_relax)
    {
      if (type == 1)
	as_warn (_(".longcall pseudo-op seen when not relaxing"));
      else
	as_warn (_(".longjump pseudo-op seen when not relaxing"));
    }

  expression (&ex);

  if (ex.X_op != O_symbol || ex.X_add_number != 0)
    {
      as_bad (_("bad .longcall format"));
      ignore_rest_of_line ();

      return;
    }

  if (type == 1)
    fix_new_exp (frag_now, frag_now_fix (), 4, & ex, 1,
		 BFD_RELOC_V850_LONGCALL);
  else
    fix_new_exp (frag_now, frag_now_fix (), 4, & ex, 1,
		 BFD_RELOC_V850_LONGJUMP);

  demand_empty_rest_of_line ();
}

/* The target specific pseudo-ops which we support.  */
const pseudo_typeS md_pseudo_table[] =
{
  { "sdata",		v850_seg,		SDATA_SECTION		},
  { "tdata",		v850_seg,		TDATA_SECTION		},
  { "zdata",		v850_seg,		ZDATA_SECTION		},
  { "sbss",		v850_seg,		SBSS_SECTION		},
  { "tbss",		v850_seg,		TBSS_SECTION		},
  { "zbss",		v850_seg,		ZBSS_SECTION		},
  { "rosdata",		v850_seg,		ROSDATA_SECTION 	},
  { "rozdata",		v850_seg,		ROZDATA_SECTION 	},
  { "bss",		v850_seg,		BSS_SECTION		},
  { "offset",		v850_offset,		0			},
  { "word",		cons,			4			},
  { "zcomm",		v850_comm,		ZCOMMON_SECTION 	},
  { "scomm",		v850_comm,		SCOMMON_SECTION 	},
  { "tcomm",		v850_comm,		TCOMMON_SECTION 	},
  { "v850",		set_machine,		0			},
  { "call_table_data",	v850_seg,		CALL_TABLE_DATA_SECTION	},
  { "call_table_text",	v850_seg,		CALL_TABLE_TEXT_SECTION	},
  { "v850e",		set_machine,		bfd_mach_v850e		},
  { "v850e1",		set_machine,		bfd_mach_v850e1         },
  { "v850e2",		set_machine,		bfd_mach_v850e2 	},
  { "v850e2v3",		set_machine,		bfd_mach_v850e2v3 	},
  { "v850e2v4",		set_machine,		bfd_mach_v850e3v5 	},
  { "v850e3v5",		set_machine,		bfd_mach_v850e3v5 	},
  { "longcall",		v850_longcode,		1			},
  { "longjump",		v850_longcode,		2			},
  { NULL,		NULL,			0			}
};

/* Opcode hash table.  */
static htab_t v850_hash;

/* This table is sorted.  Suitable for searching by a binary search.  */
static const struct reg_name pre_defined_registers[] =
{
  { "ep",  30, PROCESSOR_ALL },		/* ep - element ptr.  */
  { "gp",   4, PROCESSOR_ALL },		/* gp - global ptr.  */
  { "hp",   2, PROCESSOR_ALL },		/* hp - handler stack ptr.  */
  { "lp",  31, PROCESSOR_ALL },		/* lp - link ptr.  */
  { "r0",   0, PROCESSOR_ALL },
  { "r1",   1, PROCESSOR_ALL },
  { "r10", 10, PROCESSOR_ALL },
  { "r11", 11, PROCESSOR_ALL },
  { "r12", 12, PROCESSOR_ALL },
  { "r13", 13, PROCESSOR_ALL },
  { "r14", 14, PROCESSOR_ALL },
  { "r15", 15, PROCESSOR_ALL },
  { "r16", 16, PROCESSOR_ALL },
  { "r17", 17, PROCESSOR_ALL },
  { "r18", 18, PROCESSOR_ALL },
  { "r19", 19, PROCESSOR_ALL },
  { "r2",   2, PROCESSOR_ALL },
  { "r20", 20, PROCESSOR_ALL },
  { "r21", 21, PROCESSOR_ALL },
  { "r22", 22, PROCESSOR_ALL },
  { "r23", 23, PROCESSOR_ALL },
  { "r24", 24, PROCESSOR_ALL },
  { "r25", 25, PROCESSOR_ALL },
  { "r26", 26, PROCESSOR_ALL },
  { "r27", 27, PROCESSOR_ALL },
  { "r28", 28, PROCESSOR_ALL },
  { "r29", 29, PROCESSOR_ALL },
  { "r3",   3, PROCESSOR_ALL },
  { "r30", 30, PROCESSOR_ALL },
  { "r31", 31, PROCESSOR_ALL },
  { "r4",   4, PROCESSOR_ALL },
  { "r5",   5, PROCESSOR_ALL },
  { "r6",   6, PROCESSOR_ALL },
  { "r7",   7, PROCESSOR_ALL },
  { "r8",   8, PROCESSOR_ALL },
  { "r9",   9, PROCESSOR_ALL },
  { "sp",   3, PROCESSOR_ALL },		/* sp - stack ptr.  */
  { "tp",   5, PROCESSOR_ALL },		/* tp - text ptr.  */
  { "zero", 0, PROCESSOR_ALL },
};

#define REG_NAME_CNT						\
  (sizeof (pre_defined_registers) / sizeof (struct reg_name))

static const struct reg_name system_registers[] =
{
  { "asid",        23, PROCESSOR_NOT_V850 },
  { "bpam",        25, PROCESSOR_NOT_V850 },
  { "bpav",        24, PROCESSOR_NOT_V850 },
  { "bpc",         22, PROCESSOR_NOT_V850 },
  { "bpdm",        27, PROCESSOR_NOT_V850 },
  { "bpdv",        26, PROCESSOR_NOT_V850 },
  { "bsel",        31, PROCESSOR_V850E2_UP },
  { "cfg",          7, PROCESSOR_V850E2V3_UP },
  { "ctbp",        20, PROCESSOR_NOT_V850 },
  { "ctpc",        16, PROCESSOR_NOT_V850 },
  { "ctpsw",       17, PROCESSOR_NOT_V850 },
  { "dbic",        15, PROCESSOR_V850E2_UP },
  { "dbpc",        18, PROCESSOR_NOT_V850 },
  { "dbpsw",       19, PROCESSOR_NOT_V850 },
  { "dbwr",        30, PROCESSOR_V850E2_UP },
  { "dir",         21, PROCESSOR_NOT_V850 },
  { "dpa0l",       16, PROCESSOR_V850E2V3_UP },
  { "dpa0u",       17, PROCESSOR_V850E2V3_UP },
  { "dpa1l",       18, PROCESSOR_V850E2V3_UP },
  { "dpa1u",       19, PROCESSOR_V850E2V3_UP },
  { "dpa2l",       20, PROCESSOR_V850E2V3_UP },
  { "dpa2u",       21, PROCESSOR_V850E2V3_UP },
  { "dpa3l",       22, PROCESSOR_V850E2V3_UP },
  { "dpa3u",       23, PROCESSOR_V850E2V3_UP },
  { "dpa4l",       24, PROCESSOR_V850E2V3_UP },
  { "dpa4u",       25, PROCESSOR_V850E2V3_UP },
  { "dpa5l",       26, PROCESSOR_V850E2V3_UP },
  { "dpa5u",       27, PROCESSOR_V850E2V3_UP },
  { "ecr",          4, PROCESSOR_ALL },
  { "eh_base",      3, PROCESSOR_V850E2V3_UP },
  { "eh_cfg",       1, PROCESSOR_V850E2V3_UP },
  { "eh_reset",     2, PROCESSOR_V850E2V3_UP },
  { "eiic",        13, PROCESSOR_V850E2_UP },
  { "eipc",         0, PROCESSOR_ALL },
  { "eipsw",        1, PROCESSOR_ALL },
  { "eiwr",        28, PROCESSOR_V850E2_UP },
  { "feic",        14, PROCESSOR_V850E2_UP },
  { "fepc",         2, PROCESSOR_ALL },
  { "fepsw",        3, PROCESSOR_ALL },
  { "fewr",        29, PROCESSOR_V850E2_UP },
  { "fpcc",         9, PROCESSOR_V850E2V3_UP },
  { "fpcfg",       10, PROCESSOR_V850E2V3_UP },
  { "fpec",        11, PROCESSOR_V850E2V3_UP },
  { "fpepc",        7, PROCESSOR_V850E2V3_UP },
  { "fpspc",       27, PROCESSOR_V850E2V3_UP },
  { "fpsr",         6, PROCESSOR_V850E2V3_UP },
  { "fpst",         8, PROCESSOR_V850E2V3_UP },
  { "ipa0l",        6, PROCESSOR_V850E2V3_UP },
  { "ipa0u",        7, PROCESSOR_V850E2V3_UP },
  { "ipa1l",        8, PROCESSOR_V850E2V3_UP },
  { "ipa1u",        9, PROCESSOR_V850E2V3_UP },
  { "ipa2l",       10, PROCESSOR_V850E2V3_UP },
  { "ipa2u",       11, PROCESSOR_V850E2V3_UP },
  { "ipa3l",       12, PROCESSOR_V850E2V3_UP },
  { "ipa3u",       13, PROCESSOR_V850E2V3_UP },
  { "ipa4l",       14, PROCESSOR_V850E2V3_UP },
  { "ipa4u",       15, PROCESSOR_V850E2V3_UP },
  { "mca",         24, PROCESSOR_V850E2V3_UP },
  { "mcc",         26, PROCESSOR_V850E2V3_UP },
  { "mcr",         27, PROCESSOR_V850E2V3_UP },
  { "mcs",         25, PROCESSOR_V850E2V3_UP },
  { "mpc",          1, PROCESSOR_V850E2V3_UP },
  { "mpm",          0, PROCESSOR_V850E2V3_UP },
  { "mpu10_dpa0l", 16, PROCESSOR_V850E2V3_UP },
  { "mpu10_dpa0u", 17, PROCESSOR_V850E2V3_UP },
  { "mpu10_dpa1l", 18, PROCESSOR_V850E2V3_UP },
  { "mpu10_dpa1u", 19, PROCESSOR_V850E2V3_UP },
  { "mpu10_dpa2l", 20, PROCESSOR_V850E2V3_UP },
  { "mpu10_dpa2u", 21, PROCESSOR_V850E2V3_UP },
  { "mpu10_dpa3l", 22, PROCESSOR_V850E2V3_UP },
  { "mpu10_dpa3u", 23, PROCESSOR_V850E2V3_UP },
  { "mpu10_dpa4l", 24, PROCESSOR_V850E2V3_UP },
  { "mpu10_dpa4u", 25, PROCESSOR_V850E2V3_UP },
  { "mpu10_dpa5l", 26, PROCESSOR_V850E2V3_UP },
  { "mpu10_dpa5u", 27, PROCESSOR_V850E2V3_UP },
  { "mpu10_ipa0l",  6, PROCESSOR_V850E2V3_UP },
  { "mpu10_ipa0u",  7, PROCESSOR_V850E2V3_UP },
  { "mpu10_ipa1l",  8, PROCESSOR_V850E2V3_UP },
  { "mpu10_ipa1u",  9, PROCESSOR_V850E2V3_UP },
  { "mpu10_ipa2l", 10, PROCESSOR_V850E2V3_UP },
  { "mpu10_ipa2u", 11, PROCESSOR_V850E2V3_UP },
  { "mpu10_ipa3l", 12, PROCESSOR_V850E2V3_UP },
  { "mpu10_ipa3u", 13, PROCESSOR_V850E2V3_UP },
  { "mpu10_ipa4l", 14, PROCESSOR_V850E2V3_UP },
  { "mpu10_ipa4u", 15, PROCESSOR_V850E2V3_UP },
  { "mpu10_mpc",    1, PROCESSOR_V850E2V3_UP },
  { "mpu10_mpm",    0, PROCESSOR_V850E2V3_UP },
  { "mpu10_tid",    2, PROCESSOR_V850E2V3_UP },
  { "mpu10_vmadr",  5, PROCESSOR_V850E2V3_UP },
  { "mpu10_vmecr",  3, PROCESSOR_V850E2V3_UP },
  { "mpu10_vmtid",  4, PROCESSOR_V850E2V3_UP },
  { "pid",          6, PROCESSOR_V850E2V3_UP },
  { "pmcr0",        4, PROCESSOR_V850E2V3_UP },
  { "pmis2",       14, PROCESSOR_V850E2V3_UP },
  { "psw",          5, PROCESSOR_ALL },
  { "scbp",        12, PROCESSOR_V850E2V3_UP },
  { "sccfg",       11, PROCESSOR_V850E2V3_UP },
  { "sr0",          0, PROCESSOR_ALL },
  { "sr1",          1, PROCESSOR_ALL },
  { "sr10",        10, PROCESSOR_ALL },
  { "sr11",        11, PROCESSOR_ALL },
  { "sr12",        12, PROCESSOR_ALL },
  { "sr13",        13, PROCESSOR_ALL },
  { "sr14",        14, PROCESSOR_ALL },
  { "sr15",        15, PROCESSOR_ALL },
  { "sr16",        16, PROCESSOR_ALL },
  { "sr17",        17, PROCESSOR_ALL },
  { "sr18",        18, PROCESSOR_ALL },
  { "sr19",        19, PROCESSOR_ALL },
  { "sr2",          2, PROCESSOR_ALL },
  { "sr20",        20, PROCESSOR_ALL },
  { "sr21",        21, PROCESSOR_ALL },
  { "sr22",        22, PROCESSOR_ALL },
  { "sr23",        23, PROCESSOR_ALL },
  { "sr24",        24, PROCESSOR_ALL },
  { "sr25",        25, PROCESSOR_ALL },
  { "sr26",        26, PROCESSOR_ALL },
  { "sr27",        27, PROCESSOR_ALL },
  { "sr28",        28, PROCESSOR_ALL },
  { "sr29",        29, PROCESSOR_ALL },
  { "sr3",          3, PROCESSOR_ALL },
  { "sr30",        30, PROCESSOR_ALL },
  { "sr31",        31, PROCESSOR_ALL },
  { "sr4",          4, PROCESSOR_ALL },
  { "sr5",          5, PROCESSOR_ALL },
  { "sr6",          6, PROCESSOR_ALL },
  { "sr7",          7, PROCESSOR_ALL },
  { "sr8",          8, PROCESSOR_ALL },
  { "sr9",          9, PROCESSOR_ALL },
  { "sw_base",      3, PROCESSOR_V850E2V3_UP },
  { "sw_cfg",       1, PROCESSOR_V850E2V3_UP },
  { "sw_ctl",       0, PROCESSOR_V850E2V3_UP },
  { "tid",          2, PROCESSOR_V850E2V3_UP },
  { "vmadr",        6, PROCESSOR_V850E2V3_UP },
  { "vmecr",        4, PROCESSOR_V850E2V3_UP },
  { "vmtid",        5, PROCESSOR_V850E2V3_UP },
  { "vsadr",        2, PROCESSOR_V850E2V3_UP },
  { "vsecr",        0, PROCESSOR_V850E2V3_UP },
  { "vstid",        1, PROCESSOR_V850E2V3_UP },
};

#define SYSREG_NAME_CNT						\
  (sizeof (system_registers) / sizeof (struct reg_name))


static const struct reg_name cc_names[] =
{
  { "c",  0x1, PROCESSOR_ALL },
  { "e",  0x2, PROCESSOR_ALL },
  { "ge", 0xe, PROCESSOR_ALL },
  { "gt", 0xf, PROCESSOR_ALL },
  { "h",  0xb, PROCESSOR_ALL },
  { "l",  0x1, PROCESSOR_ALL },
  { "le", 0x7, PROCESSOR_ALL },
  { "lt", 0x6, PROCESSOR_ALL },
  { "n",  0x4, PROCESSOR_ALL },
  { "nc", 0x9, PROCESSOR_ALL },
  { "ne", 0xa, PROCESSOR_ALL },
  { "nh", 0x3, PROCESSOR_ALL },
  { "nl", 0x9, PROCESSOR_ALL },
  { "ns", 0xc, PROCESSOR_ALL },
  { "nv", 0x8, PROCESSOR_ALL },
  { "nz", 0xa, PROCESSOR_ALL },
  { "p",  0xc, PROCESSOR_ALL },
  { "s",  0x4, PROCESSOR_ALL },
#define COND_SA_NUM 0xd
  { "sa", COND_SA_NUM, PROCESSOR_ALL },
  { "t",  0x5, PROCESSOR_ALL },
  { "v",  0x0, PROCESSOR_ALL },
  { "z",  0x2, PROCESSOR_ALL },
};

#define CC_NAME_CNT					\
  (sizeof (cc_names) / sizeof (struct reg_name))

static const struct reg_name float_cc_names[] =
{
  { "eq",  0x2, PROCESSOR_V850E2V3_UP },	/* true.  */
  { "f",   0x0, PROCESSOR_V850E2V3_UP },	/* true.  */
  { "ge",  0xd, PROCESSOR_V850E2V3_UP },	/* false.  */
  { "gl",  0xb, PROCESSOR_V850E2V3_UP },	/* false.  */
  { "gle", 0x9, PROCESSOR_V850E2V3_UP },	/* false.  */
  { "gt",  0xf, PROCESSOR_V850E2V3_UP },	/* false.  */
  { "le",  0xe, PROCESSOR_V850E2V3_UP },	/* true.  */
  { "lt",  0xc, PROCESSOR_V850E2V3_UP },	/* true.  */
  { "neq", 0x2, PROCESSOR_V850E2V3_UP },	/* false.  */
  { "nge", 0xd, PROCESSOR_V850E2V3_UP },	/* true.  */
  { "ngl", 0xb, PROCESSOR_V850E2V3_UP },	/* true.  */
  { "ngle",0x9, PROCESSOR_V850E2V3_UP },	/* true.  */
  { "ngt", 0xf, PROCESSOR_V850E2V3_UP },	/* true.  */
  { "nle", 0xe, PROCESSOR_V850E2V3_UP },	/* false.  */
  { "nlt", 0xc, PROCESSOR_V850E2V3_UP },	/* false.  */
  { "oge", 0x5, PROCESSOR_V850E2V3_UP },	/* false.  */
  { "ogl", 0x3, PROCESSOR_V850E2V3_UP },	/* false.  */
  { "ogt", 0x7, PROCESSOR_V850E2V3_UP },	/* false.  */
  { "ole", 0x6, PROCESSOR_V850E2V3_UP },	/* true.  */
  { "olt", 0x4, PROCESSOR_V850E2V3_UP },	/* true.  */
  { "or",  0x1, PROCESSOR_V850E2V3_UP },	/* false.  */
  { "seq", 0xa, PROCESSOR_V850E2V3_UP },	/* true.  */
  { "sf",  0x8, PROCESSOR_V850E2V3_UP },	/* true.  */
  { "sne", 0xa, PROCESSOR_V850E2V3_UP },	/* false.  */
  { "st",  0x8, PROCESSOR_V850E2V3_UP },	/* false.  */
  { "t",   0x0, PROCESSOR_V850E2V3_UP },	/* false.  */
  { "ueq", 0x3, PROCESSOR_V850E2V3_UP },	/* true.  */
  { "uge", 0x4, PROCESSOR_V850E2V3_UP },	/* false.  */
  { "ugt", 0x6, PROCESSOR_V850E2V3_UP },	/* false.  */
  { "ule", 0x7, PROCESSOR_V850E2V3_UP },	/* true.  */
  { "ult", 0x5, PROCESSOR_V850E2V3_UP },	/* true.  */
  { "un",  0x1, PROCESSOR_V850E2V3_UP },	/* true.  */
};

#define FLOAT_CC_NAME_CNT					\
  (sizeof (float_cc_names) / sizeof (struct reg_name))


static const struct reg_name cacheop_names[] =
{
  { "cfald",   0x44, PROCESSOR_V850E3V5_UP },
  { "cfali",   0x40, PROCESSOR_V850E3V5_UP },
  { "chbid",   0x04, PROCESSOR_V850E3V5_UP },
  { "chbii",   0x00, PROCESSOR_V850E3V5_UP },
  { "chbiwbd", 0x06, PROCESSOR_V850E3V5_UP },
  { "chbwbd",  0x07, PROCESSOR_V850E3V5_UP },
  { "cibid",   0x24, PROCESSOR_V850E3V5_UP },
  { "cibii",   0x20, PROCESSOR_V850E3V5_UP },
  { "cibiwbd", 0x26, PROCESSOR_V850E3V5_UP },
  { "cibwbd",  0x27, PROCESSOR_V850E3V5_UP },
  { "cildd",   0x65, PROCESSOR_V850E3V5_UP },
  { "cildi",   0x61, PROCESSOR_V850E3V5_UP },
  { "cistd",   0x64, PROCESSOR_V850E3V5_UP },
  { "cisti",   0x60, PROCESSOR_V850E3V5_UP },
};

#define CACHEOP_NAME_CNT					\
  (sizeof (cacheop_names) / sizeof (struct reg_name))

static const struct reg_name prefop_names[] =
{
  { "prefd",   0x04, PROCESSOR_V850E3V5_UP },
  { "prefi",   0x00, PROCESSOR_V850E3V5_UP },
};

#define PREFOP_NAME_CNT					\
  (sizeof (prefop_names) / sizeof (struct reg_name))

static const struct reg_name vector_registers[] =
{
  { "vr0",   0, PROCESSOR_V850E3V5_UP },
  { "vr1",   1, PROCESSOR_V850E3V5_UP },
  { "vr10", 10, PROCESSOR_V850E3V5_UP },
  { "vr11", 11, PROCESSOR_V850E3V5_UP },
  { "vr12", 12, PROCESSOR_V850E3V5_UP },
  { "vr13", 13, PROCESSOR_V850E3V5_UP },
  { "vr14", 14, PROCESSOR_V850E3V5_UP },
  { "vr15", 15, PROCESSOR_V850E3V5_UP },
  { "vr16", 16, PROCESSOR_V850E3V5_UP },
  { "vr17", 17, PROCESSOR_V850E3V5_UP },
  { "vr18", 18, PROCESSOR_V850E3V5_UP },
  { "vr19", 19, PROCESSOR_V850E3V5_UP },
  { "vr2",   2, PROCESSOR_V850E3V5_UP },
  { "vr20", 20, PROCESSOR_V850E3V5_UP },
  { "vr21", 21, PROCESSOR_V850E3V5_UP },
  { "vr22", 22, PROCESSOR_V850E3V5_UP },
  { "vr23", 23, PROCESSOR_V850E3V5_UP },
  { "vr24", 24, PROCESSOR_V850E3V5_UP },
  { "vr25", 25, PROCESSOR_V850E3V5_UP },
  { "vr26", 26, PROCESSOR_V850E3V5_UP },
  { "vr27", 27, PROCESSOR_V850E3V5_UP },
  { "vr28", 28, PROCESSOR_V850E3V5_UP },
  { "vr29", 29, PROCESSOR_V850E3V5_UP },
  { "vr3",   3, PROCESSOR_V850E3V5_UP },
  { "vr30", 30, PROCESSOR_V850E3V5_UP },
  { "vr31", 31, PROCESSOR_V850E3V5_UP },
  { "vr4",   4, PROCESSOR_V850E3V5_UP },
  { "vr5",   5, PROCESSOR_V850E3V5_UP },
  { "vr6",   6, PROCESSOR_V850E3V5_UP },
  { "vr7",   7, PROCESSOR_V850E3V5_UP },
  { "vr8",   8, PROCESSOR_V850E3V5_UP },
  { "vr9",   9, PROCESSOR_V850E3V5_UP },
};

#define VREG_NAME_CNT						\
  (sizeof (vector_registers) / sizeof (struct reg_name))

/* Do a binary search of the given register table to see if NAME is a
   valid register name.  Return the register number from the array on
   success, or -1 on failure.  */

static int
reg_name_search (const struct reg_name *regs,
		 int regcount,
		 const char *name,
		 bool accept_numbers)
{
  int middle, low, high;
  int cmp;
  symbolS *symbolP;

  /* If the register name is a symbol, then evaluate it.  */
  if ((symbolP = symbol_find (name)) != NULL)
    {
      /* If the symbol is an alias for another name then use that.
	 If the symbol is an alias for a number, then return the number.  */
      if (symbol_equated_p (symbolP))
	name
	  = S_GET_NAME (symbol_get_value_expression (symbolP)->X_add_symbol);
      else if (accept_numbers)
	{
	  int reg = S_GET_VALUE (symbolP);
	  return reg;
	}

      /* Otherwise drop through and try parsing name normally.  */
    }

  low = 0;
  high = regcount - 1;

  do
    {
      middle = (low + high) / 2;
      cmp = strcasecmp (name, regs[middle].name);
      if (cmp < 0)
	high = middle - 1;
      else if (cmp > 0)
	low = middle + 1;
      else
	return ((regs[middle].processors & processor_mask)
		? regs[middle].value
		: -1);
    }
  while (low <= high);
  return -1;
}

/* Summary of register_name().

   in: Input_line_pointer points to 1st char of operand.

   out: An expressionS.
  	The operand may have been a register: in this case, X_op == O_register,
  	X_add_number is set to the register number, and truth is returned.
  	Input_line_pointer->(next non-blank) char after operand, or is in
  	its original state.  */

static bool
register_name (expressionS *expressionP)
{
  int reg_number;
  char *name;
  char *start;
  char c;

  /* Find the spelling of the operand.  */
  start = input_line_pointer;
  c = get_symbol_name (&name);

  reg_number = reg_name_search (pre_defined_registers, REG_NAME_CNT,
				name, false);

  /* Put back the delimiting char.  */
  (void) restore_line_pointer (c);

  expressionP->X_add_symbol = NULL;
  expressionP->X_op_symbol  = NULL;

  /* Look to see if it's in the register table.  */
  if (reg_number >= 0)
    {
      expressionP->X_op		= O_register;
      expressionP->X_add_number = reg_number;

      return true;
    }

  /* Reset the line as if we had not done anything.  */
  input_line_pointer = start;

  expressionP->X_op = O_illegal;

  return false;
}

/* Summary of system_register_name().

   in:  INPUT_LINE_POINTER points to 1st char of operand.
	EXPRESSIONP points to an expression structure to be filled in.
	ACCEPT_NUMBERS is true iff numerical register names may be used.

   out: An expressionS structure in expressionP.
  	The operand may have been a register: in this case, X_op == O_register,
  	X_add_number is set to the register number, and truth is returned.
  	Input_line_pointer->(next non-blank) char after operand, or is in
  	its original state.  */

static bool
system_register_name (expressionS *expressionP,
		      bool accept_numbers)
{
  int reg_number;
  char *name;
  char *start;
  char c;

  /* Find the spelling of the operand.  */
  start = input_line_pointer;
  c = get_symbol_name (&name);
  reg_number = reg_name_search (system_registers, SYSREG_NAME_CNT, name,
				accept_numbers);

  /* Put back the delimiting char.  */
  (void) restore_line_pointer (c);

  if (reg_number < 0
      && accept_numbers)
    {
      /* Reset input_line pointer.  */
      input_line_pointer = start;

      if (ISDIGIT (*input_line_pointer))
	{
	  reg_number = strtol (input_line_pointer, &input_line_pointer, 0);
	}
    }

  expressionP->X_add_symbol = NULL;
  expressionP->X_op_symbol  = NULL;

  /* Look to see if it's in the register table.  */
  if (reg_number >= 0)
    {
      expressionP->X_op		= O_register;
      expressionP->X_add_number = reg_number;

      return true;
    }

  /* Reset the line as if we had not done anything.  */
  input_line_pointer = start;

  expressionP->X_op = O_illegal;

  return false;
}

/* Summary of cc_name().

   in: INPUT_LINE_POINTER points to 1st char of operand.

   out: An expressionS.
  	The operand may have been a register: in this case, X_op == O_register,
  	X_add_number is set to the register number, and truth is returned.
  	Input_line_pointer->(next non-blank) char after operand, or is in
  	its original state.  */

static bool
cc_name (expressionS *expressionP,
	 bool accept_numbers)
{
  int reg_number;
  char *name;
  char *start;
  char c;

  /* Find the spelling of the operand.  */
  start = input_line_pointer;
  c = get_symbol_name (&name);
  reg_number = reg_name_search (cc_names, CC_NAME_CNT, name, accept_numbers);

  /* Put back the delimiting char.  */
  (void) restore_line_pointer (c);

  if (reg_number < 0
      && accept_numbers)
    {
      /* Reset input_line pointer.  */
      input_line_pointer = start;

      if (ISDIGIT (*input_line_pointer))
	{
	  reg_number = strtol (input_line_pointer, &input_line_pointer, 0);
	}
    }

  expressionP->X_add_symbol = NULL;
  expressionP->X_op_symbol  = NULL;

  /* Look to see if it's in the register table.  */
  if (reg_number >= 0)
    {
      expressionP->X_op		= O_constant;
      expressionP->X_add_number = reg_number;

      return true;
    }

  /* Reset the line as if we had not done anything.  */
  input_line_pointer = start;

  expressionP->X_op = O_illegal;
  expressionP->X_add_number = 0;

  return false;
}

static bool
float_cc_name (expressionS *expressionP,
	       bool accept_numbers)
{
  int reg_number;
  char *name;
  char *start;
  char c;

  /* Find the spelling of the operand.  */
  start = input_line_pointer;
  c = get_symbol_name (&name);
  reg_number = reg_name_search (float_cc_names, FLOAT_CC_NAME_CNT, name, accept_numbers);

  /* Put back the delimiting char.  */
  (void) restore_line_pointer (c);

  if (reg_number < 0
      && accept_numbers)
    {
      /* Reset input_line pointer.  */
      input_line_pointer = start;

      if (ISDIGIT (*input_line_pointer))
	{
	  reg_number = strtol (input_line_pointer, &input_line_pointer, 0);
	}
    }

  expressionP->X_add_symbol = NULL;
  expressionP->X_op_symbol  = NULL;

  /* Look to see if it's in the register table.  */
  if (reg_number >= 0)
    {
      expressionP->X_op		= O_constant;
      expressionP->X_add_number = reg_number;

      return true;
    }

  /* Reset the line as if we had not done anything.  */
  input_line_pointer = start;

  expressionP->X_op = O_illegal;
  expressionP->X_add_number = 0;

  return false;
}

static bool
cacheop_name (expressionS * expressionP,
	      bool accept_numbers)
{
  int reg_number;
  char *name;
  char *start;
  char c;

  /* Find the spelling of the operand.  */
  start = input_line_pointer;
  c = get_symbol_name (&name);
  reg_number = reg_name_search (cacheop_names, CACHEOP_NAME_CNT, name, accept_numbers);

  /* Put back the delimiting char.  */
  (void) restore_line_pointer (c);

  if (reg_number < 0
      && accept_numbers)
    {
      /* Reset input_line pointer.  */
      input_line_pointer = start;

      if (ISDIGIT (*input_line_pointer))
	reg_number = strtol (input_line_pointer, &input_line_pointer, 0);
    }

  expressionP->X_add_symbol = NULL;
  expressionP->X_op_symbol  = NULL;

  /* Look to see if it's in the register table.  */
  if (reg_number >= 0)
    {
      expressionP->X_op		= O_constant;
      expressionP->X_add_number = reg_number;

      return true;
    }

  /* Reset the line as if we had not done anything.  */
  input_line_pointer = start;

  expressionP->X_op = O_illegal;
  expressionP->X_add_number = 0;

  return false;
}

static bool
prefop_name (expressionS * expressionP,
	     bool accept_numbers)
{
  int reg_number;
  char *name;
  char *start;
  char c;

  /* Find the spelling of the operand.  */
  start = input_line_pointer;
  c = get_symbol_name (&name);
  reg_number = reg_name_search (prefop_names, PREFOP_NAME_CNT, name, accept_numbers);

  /* Put back the delimiting char.  */
  (void) restore_line_pointer (c);

  if (reg_number < 0
      && accept_numbers)
    {
      /* Reset input_line pointer.  */
      input_line_pointer = start;

      if (ISDIGIT (*input_line_pointer))
	reg_number = strtol (input_line_pointer, &input_line_pointer, 0);
    }

  expressionP->X_add_symbol = NULL;
  expressionP->X_op_symbol  = NULL;

  /* Look to see if it's in the register table.  */
  if (reg_number >= 0)
    {
      expressionP->X_op		= O_constant;
      expressionP->X_add_number = reg_number;

      return true;
    }

  /* Reset the line as if we had not done anything.  */
  input_line_pointer = start;

  expressionP->X_op = O_illegal;
  expressionP->X_add_number = 0;

  return false;
}

static bool
vector_register_name (expressionS *expressionP)
{
  int reg_number;
  char *name;
  char *start;
  char c;

  /* Find the spelling of the operand.  */
  start = input_line_pointer;
  c = get_symbol_name (&name);

  reg_number = reg_name_search (vector_registers, VREG_NAME_CNT,
				name, false);

  /* Put back the delimiting char.  */
  (void) restore_line_pointer (c);

  expressionP->X_add_symbol = NULL;
  expressionP->X_op_symbol  = NULL;

  /* Look to see if it's in the register table.  */
  if (reg_number >= 0)
    {
      expressionP->X_op		= O_register;
      expressionP->X_add_number = reg_number;

      return true;
    }

  /* Reset the line as if we had not done anything.  */
  input_line_pointer = start;

  expressionP->X_op = O_illegal;

  return false;
}

static void
skip_white_space (void)
{
  while (*input_line_pointer == ' '
	 || *input_line_pointer == '\t')
    ++input_line_pointer;
}

/* Summary of parse_register_list ().

   in: INPUT_LINE_POINTER  points to 1st char of a list of registers.
       INSN		   is the partially constructed instruction.
       OPERAND		   is the operand being inserted.

   out: NULL if the parse completed successfully, otherwise a
	pointer to an error message is returned.  If the parse
	completes the correct bit fields in the instruction
	will be filled in.

   Parses register lists with the syntax:

     { rX }
     { rX, rY }
     { rX - rY }
     { rX - rY, rZ }
     etc

   and also parses constant expressions whose bits indicate the
   registers in the lists.  The LSB in the expression refers to
   the lowest numbered permissible register in the register list,
   and so on upwards.  System registers are considered to be very
   high numbers.  */

static const char *
parse_register_list (unsigned long *insn,
		     const struct v850_operand *operand)
{
  static int type1_regs[32] =
  {
    30,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0, 31, 29, 28, 23, 22, 21, 20, 27, 26, 25, 24
  };

  int *regs;
  expressionS exp;

  /* Select a register array to parse.  */
  switch (operand->shift)
    {
    case 0xffe00001: regs = type1_regs; break;
    default:
      as_bad (_("unknown operand shift: %x\n"), operand->shift);
      return _("internal failure in parse_register_list");
    }

  skip_white_space ();

  /* If the expression starts with a curly brace it is a register list.
     Otherwise it is a constant expression, whose bits indicate which
     registers are to be included in the list.  */
  if (*input_line_pointer != '{')
    {
      int reg;
      int i;

      expression (&exp);

      if (exp.X_op != O_constant)
	return _("constant expression or register list expected");

      if (regs == type1_regs)
	{
	  if (exp.X_add_number & 0xFFFFF000)
	    return _("high bits set in register list expression");

	  for (reg = 20; reg < 32; reg++)
	    if (exp.X_add_number & (1 << (reg - 20)))
	      {
		for (i = 0; i < 32; i++)
		  if (regs[i] == reg)
		    *insn |= (1 << i);
	      }
	}

      return NULL;
    }

  input_line_pointer++;

  /* Parse the register list until a terminator (closing curly brace or
     new-line) is found.  */
  for (;;)
    {
      skip_white_space ();

      if (register_name (&exp))
	{
	  int i;

	  /* Locate the given register in the list, and if it is there,
	     insert the corresponding bit into the instruction.  */
	  for (i = 0; i < 32; i++)
	    {
	      if (regs[i] == exp.X_add_number)
		{
		  *insn |= 1u << i;
		  break;
		}
	    }

	  if (i == 32)
	    return _("illegal register included in list");
	}
      else if (system_register_name (&exp, true))
	{
	  if (regs == type1_regs)
	    {
	      return _("system registers cannot be included in list");
	    }
	}

      if (*input_line_pointer == '}')
	{
	  input_line_pointer++;
	  break;
	}
      else if (*input_line_pointer == ',')
	{
	  input_line_pointer++;
	  continue;
	}
      else if (*input_line_pointer == '-')
	{
	  /* We have encountered a range of registers: rX - rY.  */
	  int j;
	  expressionS exp2;

	  /* Skip the dash.  */
	  ++input_line_pointer;

	  /* Get the second register in the range.  */
	  if (! register_name (&exp2))
	    {
	      return _("second register should follow dash in register list");
	    }

	  if (exp.X_add_number > exp2.X_add_number)
	    {
	      return _("second register should be greater than first register");
	    }

	  /* Add the rest of the registers in the range.  */
	  for (j = exp.X_add_number + 1; j <= exp2.X_add_number; j++)
	    {
	      int i;

	      /* Locate the given register in the list, and if it is there,
		 insert the corresponding bit into the instruction.  */
	      for (i = 0; i < 32; i++)
		{
		  if (regs[i] == j)
		    {
		      *insn |= (1 << i);
		      break;
		    }
		}

	      if (i == 32)
		return _("illegal register included in list");
	    }

	  exp = exp2;
	}
      else
	break;
    }

  return NULL;
}

const char *md_shortopts = "m:";

struct option md_longopts[] =
{
#define OPTION_DISP_SIZE_DEFAULT_22 (OPTION_MD_BASE)
  {"disp-size-default-22", no_argument, NULL, OPTION_DISP_SIZE_DEFAULT_22},
#define OPTION_DISP_SIZE_DEFAULT_32 (OPTION_MD_BASE + 1)
  {"disp-size-default-32", no_argument, NULL, OPTION_DISP_SIZE_DEFAULT_32},
  {NULL, no_argument, NULL, 0}
};

size_t md_longopts_size = sizeof (md_longopts);

static bool v850_data_8 = false;

void
md_show_usage (FILE *stream)
{
  fprintf (stream, _(" V850 options:\n"));
  fprintf (stream, _("  -mwarn-signed-overflow    Warn if signed immediate values overflow\n"));
  fprintf (stream, _("  -mwarn-unsigned-overflow  Warn if unsigned immediate values overflow\n"));
  fprintf (stream, _("  -mv850                    The code is targeted at the v850\n"));
  fprintf (stream, _("  -mv850e                   The code is targeted at the v850e\n"));
  fprintf (stream, _("  -mv850e1                  The code is targeted at the v850e1\n"));
  fprintf (stream, _("  -mv850e2                  The code is targeted at the v850e2\n"));
  fprintf (stream, _("  -mv850e2v3                The code is targeted at the v850e2v3\n"));
  fprintf (stream, _("  -mv850e2v4                Alias for -mv850e3v5\n"));
  fprintf (stream, _("  -mv850e3v5                The code is targeted at the v850e3v5\n"));
  fprintf (stream, _("  -mrelax                   Enable relaxation\n"));
  fprintf (stream, _("  --disp-size-default-22    branch displacement with unknown size is 22 bits (default)\n"));
  fprintf (stream, _("  --disp-size-default-32    branch displacement with unknown size is 32 bits\n"));
  fprintf (stream, _("  -mextension               enable extension opcode support\n"));
  fprintf (stream, _("  -mno-bcond17		  disable b<cond> disp17 instruction\n"));
  fprintf (stream, _("  -mno-stld23		  disable st/ld offset23 instruction\n"));
  fprintf (stream, _("  -mgcc-abi                 Mark the binary as using the old GCC ABI\n"));
  fprintf (stream, _("  -mrh850-abi               Mark the binary as using the RH850 ABI (default)\n"));
  fprintf (stream, _("  -m8byte-align             Mark the binary as using 64-bit alignment\n"));
  fprintf (stream, _("  -m4byte-align             Mark the binary as using 32-bit alignment (default)\n"));
  fprintf (stream, _("  -msoft-float              Mark the binary as not using FP insns (default for pre e2v3)\n"));
  fprintf (stream, _("  -mhard-float              Mark the binary as using FP insns (default for e2v3 and up)\n"));
}

int
md_parse_option (int c, const char *arg)
{
  if (c != 'm')
    {
      switch (c)
        {
        case OPTION_DISP_SIZE_DEFAULT_22:
          default_disp_size = 22;
          return 1;

        case OPTION_DISP_SIZE_DEFAULT_32:
          default_disp_size = 32;
          return 1;
        }
      return 0;
    }

  if (strcmp (arg, "warn-signed-overflow") == 0)
    warn_signed_overflows = true;

  else if (strcmp (arg, "warn-unsigned-overflow") == 0)
    warn_unsigned_overflows = true;

  else if (strcmp (arg, "v850") == 0)
    {
      machine = 0;
      SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850);
    }
  else if (strcmp (arg, "v850e") == 0)
    {
      machine = bfd_mach_v850e;
      SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850E);
    }
  else if (strcmp (arg, "v850e1") == 0)
    {
      machine = bfd_mach_v850e1;
      SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850E1);
    }
  else if (strcmp (arg, "v850e2") == 0)
    {
      machine = bfd_mach_v850e2;
      SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850E2);
    }
  else if (strcmp (arg, "v850e2v3") == 0)
    {
      machine = bfd_mach_v850e2v3;
      SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850E2V3);
    }
  else if (strcmp (arg, "v850e2v4") == 0)
    {
      machine = bfd_mach_v850e3v5;
      SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850E3V5);
    }
  else if (strcmp (arg, "v850e3v5") == 0)
    {
      machine = bfd_mach_v850e3v5;
      SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850E3V5);
    }
  else if (strcmp (arg, "extension") == 0)
    {
      processor_mask |= PROCESSOR_OPTION_EXTENSION | PROCESSOR_OPTION_ALIAS;
    }
  else if (strcmp (arg, "no-bcond17") == 0)
    {
      no_bcond17 = 1;
    }
  else if (strcmp (arg, "no-stld23") == 0)
    {
      no_stld23 = 1;
    }
  else if (strcmp (arg, "relax") == 0)
    v850_relax = 1;
  else if (strcmp (arg, "gcc-abi") == 0)
    {
      v850_target_arch = bfd_arch_v850;
      v850_target_format = "elf32-v850";
    }
  else if (strcmp (arg, "rh850-abi") == 0)
    {
      v850_target_arch = bfd_arch_v850_rh850;
      v850_target_format = "elf32-v850-rh850";
    }
  else if (strcmp (arg, "8byte-align") == 0)
    {
      v850_data_8 = true;
      v850_e_flags |= EF_RH850_DATA_ALIGN8;
    }
  else if (strcmp (arg, "4byte-align") == 0)
    {
      v850_data_8 = false;
      v850_e_flags &= ~ EF_RH850_DATA_ALIGN8;
    }
  else if (strcmp (arg, "soft-float") == 0)
    soft_float = 1;
  else if (strcmp (arg, "hard-float") == 0)
    soft_float = 0;
  else
    return 0;

  return 1;
}

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return 0;
}

const char *
md_atof (int type, char *litp, int *sizep)
{
  return ieee_md_atof (type, litp, sizep, false);
}

/* Very gross.  */

void
md_convert_frag (bfd *abfd ATTRIBUTE_UNUSED,
		 asection *sec,
		 fragS *fragP)
{
  union u
  {
    bfd_reloc_code_real_type fx_r_type;
    char * fr_opcode;
  }
  opcode_converter;
  subseg_change (sec, 0);

  opcode_converter.fr_opcode = fragP->fr_opcode;

  subseg_change (sec, 0);

  if (fragP->fr_subtype == SUBYPTE_LOOP_16_22)
    {
      fix_new (fragP, fragP->fr_fix, 4, fragP->fr_symbol,
	       fragP->fr_offset, 1,
	       BFD_RELOC_UNUSED + opcode_converter.fx_r_type);
      fragP->fr_fix += 4;
    }
  else if (fragP->fr_subtype == SUBYPTE_LOOP_16_22 + 1)
    {
      unsigned char * buffer =
	(unsigned char *) (fragP->fr_fix + &fragP->fr_literal[0]);
      int loop_reg = (buffer[0] & 0x1f);

      /* Add -1.reg.  */
      md_number_to_chars ((char *) buffer, 0x025f | (loop_reg << 11), 2);
      /* Now create the conditional branch + fixup to the final target.  */
      /* 0x000107ea = bne LBL(disp17).  */
      md_number_to_chars ((char *) buffer + 2, 0x000107ea, 4);
      fix_new (fragP, fragP->fr_fix + 2, 4, fragP->fr_symbol,
	       fragP->fr_offset, 1,
	       BFD_RELOC_V850_17_PCREL);
      fragP->fr_fix += 6;
    }
  /* In range conditional or unconditional branch.  */
  else if (fragP->fr_subtype == SUBYPTE_COND_9_22
      || fragP->fr_subtype == SUBYPTE_UNCOND_9_22
      || fragP->fr_subtype == SUBYPTE_COND_9_22_32
      || fragP->fr_subtype == SUBYPTE_UNCOND_9_22_32
      || fragP->fr_subtype == SUBYPTE_COND_9_17_22
      || fragP->fr_subtype == SUBYPTE_COND_9_17_22_32
      || fragP->fr_subtype == SUBYPTE_SA_9_22
      || fragP->fr_subtype == SUBYPTE_SA_9_22_32
      || fragP->fr_subtype == SUBYPTE_SA_9_17_22
      || fragP->fr_subtype == SUBYPTE_SA_9_17_22_32)

    {
      fix_new (fragP, fragP->fr_fix, 2, fragP->fr_symbol,
	       fragP->fr_offset, 1,
	       BFD_RELOC_UNUSED + opcode_converter.fx_r_type);
      fragP->fr_fix += 2;
    }
  /* V850e2r-v3 17bit conditional branch.  */
  else if (fragP->fr_subtype == SUBYPTE_COND_9_17_22 + 1
	   || fragP->fr_subtype == SUBYPTE_COND_9_17_22_32 + 1
	   || fragP->fr_subtype == SUBYPTE_SA_9_17_22 + 1
	   || fragP->fr_subtype == SUBYPTE_SA_9_17_22_32 + 1)
    {
      unsigned char *buffer =
	(unsigned char *) (fragP->fr_fix + &fragP->fr_literal[0]);

      buffer[0] &= 0x0f;	/* Use condition.  */
      buffer[0] |= 0xe0;
      buffer[1] = 0x07;

      /* Now create the unconditional branch + fixup to the final
	 target.  */
      md_number_to_chars ((char *) buffer + 2, 0x0001, 2);
      fix_new (fragP, fragP->fr_fix, 4, fragP->fr_symbol,
	       fragP->fr_offset, 1, BFD_RELOC_V850_17_PCREL);
      fragP->fr_fix += 4;
    }
  /* Out of range conditional branch.  Emit a branch around a 22bit jump.  */
  else if (fragP->fr_subtype == SUBYPTE_COND_9_22 + 1
	   || fragP->fr_subtype == SUBYPTE_COND_9_22_32 + 1
	   || fragP->fr_subtype == SUBYPTE_COND_9_17_22 + 2
	   || fragP->fr_subtype == SUBYPTE_COND_9_17_22_32 + 2)
    {
      unsigned char *buffer =
	(unsigned char *) (fragP->fr_fix + fragP->fr_literal);

      /* Reverse the condition of the first branch.  */
      buffer[0] ^= 0x08;
      /* Mask off all the displacement bits.  */
      buffer[0] &= 0x8f;
      buffer[1] &= 0x07;
      /* Now set the displacement bits so that we branch
	 around the unconditional branch.  */
      buffer[0] |= 0x30;

      /* Now create the unconditional branch + fixup to the final
	 target.  */
      md_number_to_chars ((char *) buffer + 2, 0x00000780, 4);
      fix_new (fragP, fragP->fr_fix + 2, 4, fragP->fr_symbol,
	       fragP->fr_offset, 1, BFD_RELOC_V850_22_PCREL);
      fragP->fr_fix += 6;
    }
  /* Out of range conditional branch.  Emit a branch around a 32bit jump.  */
  else if (fragP->fr_subtype == SUBYPTE_COND_9_22_32 + 2
	   || fragP->fr_subtype == SUBYPTE_COND_9_17_22_32 + 3)
    {
      unsigned char *buffer =
	(unsigned char *) (fragP->fr_fix + fragP->fr_literal);

      /* Reverse the condition of the first branch.  */
      buffer[0] ^= 0x08;
      /* Mask off all the displacement bits.  */
      buffer[0] &= 0x8f;
      buffer[1] &= 0x07;
      /* Now set the displacement bits so that we branch
	 around the unconditional branch.  */
      buffer[0] |= 0x40;

      /* Now create the unconditional branch + fixup to the final
	 target.  */
      md_number_to_chars ((char *) buffer + 2, 0x02e0, 2);
      fix_new (fragP, fragP->fr_fix + 4, 4, fragP->fr_symbol,
	       fragP->fr_offset + 2, 1, BFD_RELOC_V850_32_PCREL);
      fragP->fr_fix += 8;
    }
  /* Out of range unconditional branch.  Emit a 22bit jump.  */
  else if (fragP->fr_subtype == SUBYPTE_UNCOND_9_22 + 1
	   || fragP->fr_subtype == SUBYPTE_UNCOND_9_22_32 + 1)
    {
      md_number_to_chars (fragP->fr_fix + fragP->fr_literal, 0x00000780, 4);
      fix_new (fragP, fragP->fr_fix, 4, fragP->fr_symbol,
	       fragP->fr_offset, 1, BFD_RELOC_V850_22_PCREL);
      fragP->fr_fix += 4;
    }
  /* Out of range unconditional branch.  Emit a 32bit jump.  */
  else if (fragP->fr_subtype == SUBYPTE_UNCOND_9_22_32 + 2)
    {
      md_number_to_chars (fragP->fr_fix + fragP->fr_literal, 0x02e0, 2);
      fix_new (fragP, fragP->fr_fix + 4, 4, fragP->fr_symbol,
	       fragP->fr_offset + 2, 1, BFD_RELOC_V850_32_PCREL);
      fragP->fr_fix += 6;
    }
  /* Out of range SA conditional branch.  Emit a branch to a 22bit jump.  */
  else if (fragP->fr_subtype == SUBYPTE_SA_9_22 + 1
	   || fragP->fr_subtype == SUBYPTE_SA_9_22_32 + 1
	   || fragP->fr_subtype == SUBYPTE_SA_9_17_22 + 2
	   || fragP->fr_subtype == SUBYPTE_SA_9_17_22_32 + 2)
    {
      unsigned char *buffer =
	(unsigned char *) (fragP->fr_fix + fragP->fr_literal);

      /* bsa .+4 */
      buffer[0] &= 0x8f;
      buffer[0] |= 0x20;
      buffer[1] &= 0x07;

      /* br .+6 */
      md_number_to_chars ((char *) buffer + 2, 0x05b5, 2);

      /* Now create the unconditional branch + fixup to the final
	 target.  */
      /* jr SYM */
      md_number_to_chars ((char *) buffer + 4, 0x00000780, 4);
      fix_new (fragP, fragP->fr_fix + 4, 4, fragP->fr_symbol,
	       fragP->fr_offset, 1,
	       BFD_RELOC_V850_22_PCREL);
      fragP->fr_fix += 8;
    }
  /* Out of range SA conditional branch.  Emit a branch around a 32bit jump.  */
  else if (fragP->fr_subtype == SUBYPTE_SA_9_22_32 + 2
	   || fragP->fr_subtype == SUBYPTE_SA_9_17_22_32 + 3)
    {
      unsigned char *buffer =
	(unsigned char *) (fragP->fr_fix + fragP->fr_literal);

      /* bsa .+2 */
      buffer[0] &= 0x8f;
      buffer[0] |= 0x20;
      buffer[1] &= 0x07;

      /* br .+8 */
      md_number_to_chars ((char *) buffer + 2, 0x05c5, 2);

      /* Now create the unconditional branch + fixup to the final
	 target.  */
      /* jr SYM */
      md_number_to_chars ((char *) buffer + 4, 0x02e0, 2);
      fix_new (fragP, fragP->fr_fix + 6, 4, fragP->fr_symbol,
	       fragP->fr_offset + 2, 1, BFD_RELOC_V850_32_PCREL);

      fragP->fr_fix += 10;
    }
  else
    abort ();
}

valueT
md_section_align (asection *seg, valueT addr)
{
  int align = bfd_section_alignment (seg);
  return ((addr + (1 << align) - 1) & -(1 << align));
}

void
md_begin (void)
{
  const char *prev_name = "";
  const struct v850_opcode *op;

  if (startswith (TARGET_CPU, "v850e3v5"))
    {
      if (machine == -1)
	machine = bfd_mach_v850e3v5;

      if (!processor_mask)
	SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850E3V5);
    }
  else if (startswith (TARGET_CPU, "v850e2v4"))
    {
      if (machine == -1)
	machine = bfd_mach_v850e3v5;

      if (!processor_mask)
	SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850E3V5);
    }
  else if (startswith (TARGET_CPU, "v850e2v3"))
    {
      if (machine == -1)
        machine = bfd_mach_v850e2v3;

      if (!processor_mask)
        SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850E2V3);
    }
  else if (startswith (TARGET_CPU, "v850e2"))
    {
      if (machine == -1)
	machine = bfd_mach_v850e2;

      if (!processor_mask)
	SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850E2);
    }
  else if (startswith (TARGET_CPU, "v850e1"))
    {
      if (machine == -1)
        machine = bfd_mach_v850e1;

      if (!processor_mask)
        SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850E1);
    }
  else if (startswith (TARGET_CPU, "v850e"))
    {
      if (machine == -1)
	machine = bfd_mach_v850e;

      if (!processor_mask)
	SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850E);
    }
  else if (startswith (TARGET_CPU, "v850"))
    {
      if (machine == -1)
	machine = 0;

      if (!processor_mask)
	SET_PROCESSOR_MASK (processor_mask, PROCESSOR_V850);
    }
  else
    /* xgettext:c-format  */
    as_bad (_("Unable to determine default target processor from string: %s"),
	    TARGET_CPU);

  if (soft_float == -1)
    soft_float = machine < bfd_mach_v850e2v3;

  v850_hash = str_htab_create ();

  /* Insert unique names into hash table.  The V850 instruction set
     has many identical opcode names that have different opcodes based
     on the operands.  This hash table then provides a quick index to
     the first opcode with a particular name in the opcode table.  */
  op = v850_opcodes;
  while (op->name)
    {
      if (strcmp (prev_name, op->name))
	{
	  prev_name = (char *) op->name;
	  str_hash_insert (v850_hash, op->name, op, 0);
	}
      op++;
    }

  v850_seg_table[BSS_SECTION].s = bss_section;
  bfd_set_arch_mach (stdoutput, v850_target_arch, machine);
  bfd_set_private_flags (stdoutput, v850_e_flags);
}


static bfd_reloc_code_real_type
handle_hi016 (const struct v850_operand *operand, const char **errmsg)
{
  if (operand == NULL)
    return BFD_RELOC_HI16;

  if (operand->default_reloc == BFD_RELOC_HI16)
    return BFD_RELOC_HI16;

  if (operand->default_reloc == BFD_RELOC_HI16_S)
    return BFD_RELOC_HI16;

  if (operand->default_reloc == BFD_RELOC_16)
    return BFD_RELOC_HI16;

  *errmsg = _("hi0() relocation used on an instruction which does "
	      "not support it");
  return BFD_RELOC_64;  /* Used to indicate an error condition.  */
}

static bfd_reloc_code_real_type
handle_hi16 (const struct v850_operand *operand, const char **errmsg)
{
  if (operand == NULL)
    return BFD_RELOC_HI16_S;

  if (operand->default_reloc == BFD_RELOC_HI16_S)
    return BFD_RELOC_HI16_S;

  if (operand->default_reloc == BFD_RELOC_HI16)
    return BFD_RELOC_HI16_S;

  if (operand->default_reloc == BFD_RELOC_16)
    return BFD_RELOC_HI16_S;

  *errmsg = _("hi() relocation used on an instruction which does "
	      "not support it");
  return BFD_RELOC_64;  /* Used to indicate an error condition.  */
}

static bfd_reloc_code_real_type
handle_lo16 (const struct v850_operand *operand, const char **errmsg)
{
  if (operand == NULL)
    return BFD_RELOC_LO16;
  
  switch (operand->default_reloc)
    {
    case BFD_RELOC_LO16: return BFD_RELOC_LO16;
    case BFD_RELOC_V850_LO16_SPLIT_OFFSET: return BFD_RELOC_V850_LO16_SPLIT_OFFSET;
    case BFD_RELOC_V850_16_SPLIT_OFFSET: return BFD_RELOC_V850_LO16_SPLIT_OFFSET;
    case BFD_RELOC_V850_16_S1: return BFD_RELOC_V850_LO16_S1;
    case BFD_RELOC_16: return BFD_RELOC_LO16;
    default:
      *errmsg = _("lo() relocation used on an instruction which does "
		  "not support it");
      return BFD_RELOC_64;  /* Used to indicate an error condition.  */
    }
}

static bfd_reloc_code_real_type
handle_ctoff (const struct v850_operand *operand, const char **errmsg)
{
  if (v850_target_arch == bfd_arch_v850_rh850)
    {
      *errmsg = _("ctoff() is not supported by the rh850 ABI. Use -mgcc-abi instead");
      return BFD_RELOC_64;  /* Used to indicate an error condition.  */
    }

  if (operand == NULL)
    return BFD_RELOC_V850_CALLT_16_16_OFFSET;

  if (operand->default_reloc == BFD_RELOC_V850_CALLT_6_7_OFFSET)
    return operand->default_reloc;

  if (operand->default_reloc == BFD_RELOC_V850_16_S1)
    return BFD_RELOC_V850_CALLT_15_16_OFFSET;

  if (operand->default_reloc == BFD_RELOC_16)
    return BFD_RELOC_V850_CALLT_16_16_OFFSET;

  *errmsg = _("ctoff() relocation used on an instruction which does not support it");
  return BFD_RELOC_64;  /* Used to indicate an error condition.  */
}

static bfd_reloc_code_real_type
handle_sdaoff (const struct v850_operand *operand, const char **errmsg)
{
  if (operand == NULL)
    return BFD_RELOC_V850_SDA_16_16_OFFSET;

  if (operand->default_reloc == BFD_RELOC_V850_16_SPLIT_OFFSET)
    return BFD_RELOC_V850_SDA_16_16_SPLIT_OFFSET;

  if (operand->default_reloc == BFD_RELOC_16)
    return BFD_RELOC_V850_SDA_16_16_OFFSET;

  if (operand->default_reloc == BFD_RELOC_V850_16_S1)
    return BFD_RELOC_V850_SDA_15_16_OFFSET;

  *errmsg = _("sdaoff() relocation used on an instruction which does not support it");
  return BFD_RELOC_64;  /* Used to indicate an error condition.  */
}

static bfd_reloc_code_real_type
handle_zdaoff (const struct v850_operand *operand, const char **errmsg)
{
  if (operand == NULL)
    return BFD_RELOC_V850_ZDA_16_16_OFFSET;

  if (operand->default_reloc == BFD_RELOC_V850_16_SPLIT_OFFSET)
    return BFD_RELOC_V850_ZDA_16_16_SPLIT_OFFSET;

  if (operand->default_reloc == BFD_RELOC_16)
    return BFD_RELOC_V850_ZDA_16_16_OFFSET;

  if (operand->default_reloc == BFD_RELOC_V850_16_S1)
    return BFD_RELOC_V850_ZDA_15_16_OFFSET;

  *errmsg = _("zdaoff() relocation used on an instruction which does not support it");
  return BFD_RELOC_64;  /* Used to indicate an error condition.  */
}

static bfd_reloc_code_real_type
handle_tdaoff (const struct v850_operand *operand, const char **errmsg)
{
  if (operand == NULL)
    /* Data item, not an instruction.  */
    return BFD_RELOC_V850_TDA_16_16_OFFSET;

  switch (operand->default_reloc)
    {
      /* sld.hu, operand: D5-4.  */
    case BFD_RELOC_V850_TDA_4_5_OFFSET:
      /* sld.bu, operand: D4.  */
    case BFD_RELOC_V850_TDA_4_4_OFFSET:
    /* sld.w/sst.w, operand: D8_6.  */
    case BFD_RELOC_V850_TDA_6_8_OFFSET:
    /* sld.h/sst.h, operand: D8_7.  */
    case BFD_RELOC_V850_TDA_7_8_OFFSET:
      /* sld.b/sst.b, operand: D7.  */
    case BFD_RELOC_V850_TDA_7_7_OFFSET:
      return operand->default_reloc;
    default:
      break;
    }

  if (operand->default_reloc == BFD_RELOC_16 && operand->shift == 16)
    /* set1 & chums, operands: D16.  */
    return BFD_RELOC_V850_TDA_16_16_OFFSET;

  *errmsg = _("tdaoff() relocation used on an instruction which does not support it");
  /* Used to indicate an error condition.  */
  return BFD_RELOC_64;
}

/* Warning: The code in this function relies upon the definitions
   in the v850_operands[] array (defined in opcodes/v850-opc.c)
   matching the hard coded values contained herein.  */

static bfd_reloc_code_real_type
v850_reloc_prefix (const struct v850_operand *operand, const char **errmsg)
{
  bool paren_skipped = false;

  /* Skip leading opening parenthesis.  */
  if (*input_line_pointer == '(')
    {
      ++input_line_pointer;
      paren_skipped = true;
    }

#define CHECK_(name, reloc) 						\
  if (strncmp (input_line_pointer, name "(", strlen (name) + 1) == 0)	\
    {									\
      input_line_pointer += strlen (name);				\
      return reloc;							\
    }

  CHECK_ ("hi0",    handle_hi016 (operand, errmsg));
  CHECK_ ("hi",	    handle_hi16 (operand, errmsg));
  CHECK_ ("lo",	    handle_lo16 (operand, errmsg));
  CHECK_ ("sdaoff", handle_sdaoff (operand, errmsg));
  CHECK_ ("zdaoff", handle_zdaoff (operand, errmsg));
  CHECK_ ("tdaoff", handle_tdaoff (operand, errmsg));
  CHECK_ ("hilo",   BFD_RELOC_32);
  CHECK_ ("lo23",   BFD_RELOC_V850_23);
  CHECK_ ("ctoff",  handle_ctoff (operand, errmsg));

  /* Restore skipped parenthesis.  */
  if (paren_skipped)
    --input_line_pointer;

  return BFD_RELOC_NONE;
}

/* Insert an operand value into an instruction.  */

static unsigned long
v850_insert_operand (unsigned long insn,
		     const struct v850_operand *operand,
		     offsetT val,
		     const char **errmsg)
{
  if (operand->insert)
    {
      const char *message = NULL;

      insn = operand->insert (insn, val, &message);
      if (message != NULL)
	{
	  if ((operand->flags & V850_OPERAND_SIGNED)
	      && ! warn_signed_overflows
              && v850_msg_is_out_of_range (message))
	    {
	      /* Skip warning...  */
	    }
	  else if ((operand->flags & V850_OPERAND_SIGNED) == 0
		   && ! warn_unsigned_overflows
                  && v850_msg_is_out_of_range (message))
	    {
	      /* Skip warning...  */
	    }
	  else
	    {
             if (errmsg != NULL)
               *errmsg = message;
	    }
	}
    }
  else if (operand->bits == -1
          || operand->flags & V850E_IMMEDIATE16
          || operand->flags & V850E_IMMEDIATE23
          || operand->flags & V850E_IMMEDIATE32)
    {
      abort ();
    }
  else
    {
      if (operand->bits < 32)
	{
	  long min, max;

	  if ((operand->flags & V850_OPERAND_SIGNED) != 0)
	    {
	      if (! warn_signed_overflows)
		max = (1 << operand->bits) - 1;
	      else
		max = (1 << (operand->bits - 1)) - 1;

	      min = -(1 << (operand->bits - 1));
	    }
	  else
	    {
	      max = (1 << operand->bits) - 1;

	      if (! warn_unsigned_overflows)
		min = -(1 << (operand->bits - 1));
	      else
		min = 0;
	    }

	  /* Some people write constants with the sign extension done by
	     hand but only up to 32 bits.  This shouldn't really be valid,
	     but, to permit this code to assemble on a 64-bit host, we
	     sign extend the 32-bit value to 64 bits if so doing makes the
	     value valid.  */
	  if (val > max
	      && (offsetT) (val - 0x80000000 - 0x80000000) >= min
	      && (offsetT) (val - 0x80000000 - 0x80000000) <= max)
	    val = val - 0x80000000 - 0x80000000;

	  /* Similarly, people write expressions like ~(1<<15), and expect
	     this to be OK for a 32-bit unsigned value.  */
	  else if (val < min
		   && (offsetT) (val + 0x80000000 + 0x80000000) >= min
		   && (offsetT) (val + 0x80000000 + 0x80000000) <= max)
	    val = val + 0x80000000 + 0x80000000;

	  else if (val < (offsetT) min || val > (offsetT) max)
	    {
	      static char buf [128];

	      /* Restore min and mix to expected values for decimal ranges.  */
	      if ((operand->flags & V850_OPERAND_SIGNED)
		  && ! warn_signed_overflows)
		max = (1 << (operand->bits - 1)) - 1;

	      if (! (operand->flags & V850_OPERAND_SIGNED)
		  && ! warn_unsigned_overflows)
		min = 0;

	      sprintf (buf, _("operand out of range (%d is not between %d and %d)"),
		       (int) val, (int) min, (int) max);
	      *errmsg = buf;
	    }

	  insn |= (((long) val & ((1 << operand->bits) - 1)) << operand->shift);
	}
      else
	{
	  insn |= (((long) val) << operand->shift);
	}
    }

  return insn;
}

static char copy_of_instruction[128];

void
md_assemble (char *str)
{
  char *s;
  char *start_of_operands;
  struct v850_opcode *opcode;
  struct v850_opcode *next_opcode;
  const unsigned char *opindex_ptr;
  int next_opindex;
  int relaxable = 0;
  unsigned long insn = 0;
  unsigned long insn_size;
  char *f = NULL;
  int i;
  int match;
  bool extra_data_after_insn = false;
  unsigned extra_data_len = 0;
  unsigned long extra_data = 0;
  char *saved_input_line_pointer;
  char most_match_errmsg[1024];
  int most_match_count = -1;

  strncpy (copy_of_instruction, str, sizeof (copy_of_instruction) - 1);
  most_match_errmsg[0] = 0;

  /* Get the opcode.  */
  for (s = str; *s != '\0' && ! ISSPACE (*s); s++)
    continue;

  if (*s != '\0')
    *s++ = '\0';

  /* Find the first opcode with the proper name.  */
  opcode = (struct v850_opcode *) str_hash_find (v850_hash, str);
  if (opcode == NULL)
    {
      /* xgettext:c-format  */
      as_bad (_("Unrecognized opcode: `%s'"), str);
      ignore_rest_of_line ();
      return;
    }

  str = s;
  while (ISSPACE (*str))
    ++str;

  start_of_operands = str;

  saved_input_line_pointer = input_line_pointer;

  for (;;)
    {
      const char *errmsg = NULL;
      const char *warningmsg = NULL;

      match = 0;
      opindex_ptr = opcode->operands;

      if (no_stld23)
	{
	  if ((startswith (opcode->name, "st.")
	       && v850_operands[opcode->operands[1]].bits == 23)
	      || (startswith (opcode->name, "ld.")
		  && v850_operands[opcode->operands[0]].bits == 23))
	    {
	      errmsg = _("st/ld offset 23 instruction was disabled .");
	      goto error;
	    }
	}

      if ((opcode->processors & processor_mask & PROCESSOR_MASK) == 0
	  || (((opcode->processors & ~PROCESSOR_MASK) != 0)
	      && ((opcode->processors & processor_mask & ~PROCESSOR_MASK) == 0)))
	{
	  errmsg = _("Target processor does not support this instruction.");
	  goto error;
	}

      relaxable = 0;
      fc = 0;
      next_opindex = 0;
      insn = opcode->opcode;
      extra_data_len = 0;
      extra_data_after_insn = false;

      input_line_pointer = str = start_of_operands;

      for (opindex_ptr = opcode->operands; *opindex_ptr != 0; opindex_ptr++)
	{
	  const struct v850_operand *operand;
	  char *hold;
	  expressionS ex;
	  bfd_reloc_code_real_type reloc;

	  if (next_opindex == 0)
	    operand = &v850_operands[*opindex_ptr];
	  else
	    {
	      operand = &v850_operands[next_opindex];
	      next_opindex = 0;
	    }

	  errmsg = NULL;

	  while (*str == ' ')
	    ++str;

	  if (operand->flags & V850_OPERAND_BANG
	      && *str == '!')
	    ++str;
	  else if (operand->flags & V850_OPERAND_PERCENT
		   && *str == '%')
	    ++str;

	  if (*str == ',' || *str == '[' || *str == ']')
	    ++str;

	  while (*str == ' ')
	    ++str;

	  if (   (strcmp (opcode->name, "pushsp") == 0
	       || strcmp (opcode->name, "popsp") == 0
	       || strcmp (opcode->name, "dbpush") == 0)
	      && (*str == '-'))
	    ++str;

	  if (operand->flags & V850_OPERAND_RELAX)
	    relaxable = 1;

	  /* Gather the operand.  */
	  hold = input_line_pointer;
	  input_line_pointer = str;

	  /* lo(), hi(), hi0(), etc...  */
	  if ((reloc = v850_reloc_prefix (operand, &errmsg)) != BFD_RELOC_NONE)
	    {
	      /* This is a fake reloc, used to indicate an error condition.  */
	      if (reloc == BFD_RELOC_64)
		{
		  /* match = 1;  */
		  goto error;
		}

	      expression (&ex);

	      if (ex.X_op == O_constant)
		{
		  switch (reloc)
		    {
		    case BFD_RELOC_V850_ZDA_16_16_OFFSET:
		    case BFD_RELOC_V850_ZDA_16_16_SPLIT_OFFSET:
		    case BFD_RELOC_V850_ZDA_15_16_OFFSET:
		      /* To cope with "not1 7, zdaoff(0xfffff006)[r0]"
			 and the like.  */
		      /* Fall through.  */

		    case BFD_RELOC_LO16:
		    case BFD_RELOC_V850_LO16_S1:
		    case BFD_RELOC_V850_LO16_SPLIT_OFFSET:
		      {
			/* Truncate, then sign extend the value.  */
			ex.X_add_number = SEXT16 (ex.X_add_number);
			break;
		      }

		    case BFD_RELOC_HI16:
		      {
			/* Truncate, then sign extend the value.  */
			ex.X_add_number = SEXT16 (ex.X_add_number >> 16);
			break;
		      }

		    case BFD_RELOC_HI16_S:
		      {
			/* Truncate, then sign extend the value.  */
			int temp = (ex.X_add_number >> 16) & 0xffff;

			temp += (ex.X_add_number >> 15) & 1;

			ex.X_add_number = SEXT16 (temp);
			break;
		      }

		    case BFD_RELOC_V850_23:
		      if ((operand->flags & V850E_IMMEDIATE23) == 0)
			{
			  errmsg = _("immediate operand is too large");
			  goto error;
			}
		      break;

		    case BFD_RELOC_32:
		    case BFD_RELOC_V850_32_ABS:
		    case BFD_RELOC_V850_32_PCREL:
		      if ((operand->flags & V850E_IMMEDIATE32) == 0)
			{
			  errmsg = _("immediate operand is too large");
			  goto error;
			}

		      break;

		    default:
		      as_bad (_("AAARG -> unhandled constant reloc: %d"), reloc);
		      break;
		    }

		  if (operand->flags & V850E_IMMEDIATE32)
		    {
		      extra_data_after_insn = true;
		      extra_data_len	    = 4;
		      extra_data	    = 0;
		    }
		  else if (operand->flags & V850E_IMMEDIATE23)
		    {
		      if (reloc != BFD_RELOC_V850_23)
			{
			  errmsg = _("immediate operand is too large");
			  goto error;
			}
		      extra_data_after_insn = true;
		      extra_data_len	    = 2;
		      extra_data	    = 0;
		    }
		  else if ((operand->flags & V850E_IMMEDIATE16)
			   || (operand->flags & V850E_IMMEDIATE16HI))
		    {
		      if (operand->flags & V850E_IMMEDIATE16HI
			  && reloc != BFD_RELOC_HI16
			  && reloc != BFD_RELOC_HI16_S)
			{
			  errmsg = _("immediate operand is too large");
			  goto error;
			}
		      else if (operand->flags & V850E_IMMEDIATE16
			       && reloc != BFD_RELOC_LO16)
			{
			  errmsg = _("immediate operand is too large");
			  goto error;
			}

		      extra_data_after_insn = true;
		      extra_data_len	    = 2;
		      extra_data	    = 0;
		    }

		  if (fc > MAX_INSN_FIXUPS)
		    as_fatal (_("too many fixups"));

		  fixups[fc].exp     = ex;
		  fixups[fc].opindex = *opindex_ptr;
		  fixups[fc].reloc   = reloc;
		  fc++;
		}
	      else	/* ex.X_op != O_constant.  */
		{
		  if ((reloc == BFD_RELOC_32
		       || reloc == BFD_RELOC_V850_32_ABS
		       || reloc == BFD_RELOC_V850_32_PCREL)
		      && operand->bits < 32)
		    {
		      errmsg = _("immediate operand is too large");
		      goto error;
		    }
		  else if (reloc == BFD_RELOC_V850_23
			   && (operand->flags & V850E_IMMEDIATE23) == 0)
		    {
		      errmsg = _("immediate operand is too large");
		      goto error;
		    }
		  else if ((reloc == BFD_RELOC_HI16
			    || reloc == BFD_RELOC_HI16_S)
			   && operand->bits < 16)
		    {
		      errmsg = _("immediate operand is too large");
		      goto error;
		    }

		  if (operand->flags & V850E_IMMEDIATE32)
		    {
		      extra_data_after_insn = true;
		      extra_data_len	    = 4;
		      extra_data	    = 0;
		    }
		  else if (operand->flags & V850E_IMMEDIATE23)
		    {
		      if (reloc != BFD_RELOC_V850_23)
			{
			  errmsg = _("immediate operand is too large");
			  goto error;
			}
		      extra_data_after_insn = true;
		      extra_data_len	    = 2;
		      extra_data	    = 0;
		    }
		  else if ((operand->flags & V850E_IMMEDIATE16)
			   || (operand->flags & V850E_IMMEDIATE16HI))
		    {
		      if (operand->flags & V850E_IMMEDIATE16HI
			  && reloc != BFD_RELOC_HI16
			  && reloc != BFD_RELOC_HI16_S)
			{
			  errmsg = _("immediate operand is too large");
			  goto error;
			}
		      else if (operand->flags & V850E_IMMEDIATE16
			       && reloc != BFD_RELOC_LO16)
			{
			  errmsg = _("immediate operand is too large");
			  goto error;
			}

		      extra_data_after_insn = true;
		      extra_data_len	    = 2;
		      extra_data	    = 0;
		    }

		  if (fc > MAX_INSN_FIXUPS)
		    as_fatal (_("too many fixups"));

		  fixups[fc].exp     = ex;
		  fixups[fc].opindex = *opindex_ptr;
		  fixups[fc].reloc   = reloc;
		  fc++;
		}
	    }
	  else if (operand->flags & V850E_IMMEDIATE16
		   || operand->flags & V850E_IMMEDIATE16HI)
	    {
	      expression (&ex);

	      switch (ex.X_op)
		{
		case O_constant:
		  if (operand->flags & V850E_IMMEDIATE16HI)
		    {
		      if (ex.X_add_number & 0xffff)
			{
			  errmsg = _("constant too big to fit into instruction");
			  goto error;
			}

		      ex.X_add_number >>= 16;
		    }
		  if (operand->flags & V850E_IMMEDIATE16)
		    {
		      if ((ex.X_add_number & 0xffff8000)
			  && ((ex.X_add_number & 0xffff8000) != 0xffff8000))
			{
			  errmsg = _("constant too big to fit into instruction");
			  goto error;
			}
		    }
		  break;

		case O_illegal:
		  errmsg = _("illegal operand");
		  goto error;

		case O_absent:
		  errmsg = _("missing operand");
		  goto error;

		default:
		  if (fc >= MAX_INSN_FIXUPS)
		    as_fatal (_("too many fixups"));

		  fixups[fc].exp     = ex;
		  fixups[fc].opindex = *opindex_ptr;
		  fixups[fc].reloc   = operand->default_reloc;
		  ++fc;

		  ex.X_add_number = 0;
		  break;
		}

	      extra_data_after_insn = true;
	      extra_data_len        = 2;
	      extra_data            = ex.X_add_number;
	    }
	  else if (operand->flags & V850E_IMMEDIATE23)
	    {
	      expression (&ex);

	      switch (ex.X_op)
		{
		case O_constant:
		  break;

		case O_illegal:
		  errmsg = _("illegal operand");
		  goto error;

		case O_absent:
		  errmsg = _("missing operand");
		  goto error;

		default:
		  break;
		}

	      if (fc >= MAX_INSN_FIXUPS)
		as_fatal (_("too many fixups"));

	      fixups[fc].exp     = ex;
	      fixups[fc].opindex = *opindex_ptr;
	      fixups[fc].reloc   = operand->default_reloc;
	      ++fc;

	      extra_data_after_insn = true;
	      extra_data_len        = 2;
	      extra_data            = 0;
	    }
	  else if (operand->flags & V850E_IMMEDIATE32)
	    {
	      expression (&ex);

	      switch (ex.X_op)
		{
		case O_constant:
		  if ((operand->default_reloc == BFD_RELOC_V850_32_ABS
		       || operand->default_reloc == BFD_RELOC_V850_32_PCREL)
		      && (ex.X_add_number & 1))
		    {
		      errmsg = _("odd number cannot be used here");
		      goto error;
		    }
		  break;

		case O_illegal:
		  errmsg = _("illegal operand");
		  goto error;

		case O_absent:
		  errmsg = _("missing operand");
		  goto error;

		default:
		  if (fc >= MAX_INSN_FIXUPS)
		    as_fatal (_("too many fixups"));

		  fixups[fc].exp     = ex;
		  fixups[fc].opindex = *opindex_ptr;
		  fixups[fc].reloc   = operand->default_reloc;
		  ++fc;

		  ex.X_add_number = 0;
		  break;
		}

	      extra_data_after_insn = true;
	      extra_data_len        = 4;
	      extra_data            = ex.X_add_number;
	    }
	  else if (operand->flags & V850E_OPERAND_REG_LIST)
	    {
	      errmsg = parse_register_list (&insn, operand);

	      if (errmsg)
		goto error;
	    }
	  else
	    {
	      errmsg = NULL;

	      if ((operand->flags & V850_OPERAND_REG) != 0)
		{
		  if (!register_name (&ex))
		    {
		      errmsg = _("invalid register name");
		    }

		  if ((operand->flags & V850_NOT_R0)
			   && ex.X_add_number == 0)
		    {
		      errmsg = _("register r0 cannot be used here");
		    }

		  if (operand->flags & V850_REG_EVEN)
		    {
		      if (ex.X_add_number % 2)
			errmsg = _("odd register cannot be used here");
		      ex.X_add_number = ex.X_add_number / 2;
		    }

		}
	      else if ((operand->flags & V850_OPERAND_SRG) != 0)
		{
		  if (!system_register_name (&ex, true))
		    {
		      errmsg = _("invalid system register name");
		    }
		}
	      else if ((operand->flags & V850_OPERAND_EP) != 0)
		{
		  char *start = input_line_pointer;
		  char *name;
		  char c = get_symbol_name (&name);

		  if (strcmp (name, "ep") != 0 && strcmp (name, "r30") != 0)
		    {
		      /* Put things back the way we found them.  */
		      (void) restore_line_pointer (c);
		      input_line_pointer = start;
		      errmsg = _("expected EP register");
		      goto error;
		    }

		  (void) restore_line_pointer (c);
		  str = input_line_pointer;
		  input_line_pointer = hold;

		  while (*str == ' ' || *str == ','
			 || *str == '[' || *str == ']')
		    ++str;
		  continue;
		}
	      else if ((operand->flags & V850_OPERAND_CC) != 0)
		{
		  if (!cc_name (&ex, true))
		    {
		      errmsg = _("invalid condition code name");
		    }

		  if ((operand->flags & V850_NOT_SA)
		      && ex.X_add_number == COND_SA_NUM)
		    {
		      errmsg = _("condition sa cannot be used here");
		    }
		}
	      else if ((operand->flags & V850_OPERAND_FLOAT_CC) != 0)
		{
		  if (!float_cc_name (&ex, true))
		    {
		      errmsg = _("invalid condition code name");
		    }
		}
	      else if ((operand->flags & V850_OPERAND_CACHEOP) != 0)
		{
		  if (!cacheop_name (&ex, true))
		    errmsg = _("invalid cache operation name");
		}
	      else if ((operand->flags & V850_OPERAND_PREFOP) != 0)
		{
		  if (!prefop_name (&ex, true))
		    errmsg = _("invalid pref operation name");
		}
	      else if ((operand->flags & V850_OPERAND_VREG) != 0)
		{
		  if (!vector_register_name (&ex))
		    errmsg = _("invalid vector register name");
		}
	      else if ((register_name (&ex)
			&& (operand->flags & V850_OPERAND_REG) == 0))
		{
		  char *name;
		  char c;
		  int exists = 0;

		  /* It is possible that an alias has been defined that
		     matches a register name.  For example the code may
		     include a ".set ZERO, 0" directive, which matches
		     the register name "zero".  Attempt to reparse the
		     field as an expression, and only complain if we
		     cannot generate a constant.  */

		  input_line_pointer = str;

		  c = get_symbol_name (&name);

		  if (symbol_find (name) != NULL)
		    exists = 1;

		  (void) restore_line_pointer (c);
		  input_line_pointer = str;

		  expression (&ex);

		  if (ex.X_op != O_constant)
		    {
		      /* If this register is actually occurring too early on
			 the parsing of the instruction, (because another
			 field is missing) then report this.  */
		      if (opindex_ptr[1] != 0
			  && ((v850_operands[opindex_ptr[1]].flags
			       & V850_OPERAND_REG)
			      ||(v850_operands[opindex_ptr[1]].flags
				 & V850_OPERAND_VREG)))
			errmsg = _("syntax error: value is missing before the register name");
		      else
			errmsg = _("syntax error: register not expected");

		      /* If we created a symbol in the process of this
			 test then delete it now, so that it will not
			 be output with the real symbols...  */
		      if (exists == 0
			  && ex.X_op == O_symbol)
			symbol_remove (ex.X_add_symbol,
				       &symbol_rootP, &symbol_lastP);
		    }
		}
	      else if (system_register_name (&ex, false)
		       && (operand->flags & V850_OPERAND_SRG) == 0)
		{
		  errmsg = _("syntax error: system register not expected");
		}
	      else if (cc_name (&ex, false)
		       && (operand->flags & V850_OPERAND_CC) == 0)
		{
		  errmsg = _("syntax error: condition code not expected");
		}
	      else if (float_cc_name (&ex, false)
		       && (operand->flags & V850_OPERAND_FLOAT_CC) == 0)
		{
		  errmsg = _("syntax error: condition code not expected");
		}
	      else if (vector_register_name (&ex)
		       && (operand->flags & V850_OPERAND_VREG) == 0)
		{
		  errmsg = _("syntax error: vector register not expected");
		}
	      else
		{
		  expression (&ex);
		  resolve_register (&ex);

		  if ((operand->flags & V850_NOT_IMM0)
		      && ex.X_op == O_constant
		      && ex.X_add_number == 0)
		    {
		      errmsg = _("immediate 0 cannot be used here");
		    }

		  /* Special case:
		     If we are assembling a MOV/JARL/JR instruction and the immediate
		     value does not fit into the bits available then create a
		     fake error so that the next MOV/JARL/JR instruction will be
		     selected.  This one has a 32 bit immediate field.  */

		  if ((strcmp (opcode->name, "mov") == 0
		       || strcmp (opcode->name, "jarl") == 0
		       || strcmp (opcode->name, "jr") == 0)
		      && ex.X_op == O_constant
		      && (ex.X_add_number < (-(1 << (operand->bits - 1)))
			  || ex.X_add_number > ((1 << (operand->bits - 1)) - 1)))
		    {
		      errmsg = _("immediate operand is too large");
		    }

		  if ((strcmp (opcode->name, "jarl") == 0
		       || strcmp (opcode->name, "jr") == 0)
		      && ex.X_op != O_constant
		      && operand->bits != default_disp_size)
		    {
		      errmsg = _("immediate operand is not match");
		    }

                  /* Special case2 :
                     If we are assembling a ld/st instruction and the immediate
                     value does not fit into the bits available then create a
                     fake error so that the next ld/st instruction will be
                     selected.  */
                  if ( (  (startswith (opcode->name, "st."))
		       || (startswith (opcode->name, "ld.")))
                      && ex.X_op == O_constant
                      && (ex.X_add_number < (-(1 << (operand->bits - 1)))
			  || ex.X_add_number > ((1 << (operand->bits - 1)) - 1)))
		    errmsg = _("displacement is too large");
		}

	      if (errmsg)
		goto error;

	      switch (ex.X_op)
		{
		case O_illegal:
		  errmsg = _("illegal operand");
		  goto error;
		case O_absent:
		  errmsg = _("missing operand");
		  goto error;
		case O_register:
		  if ((operand->flags
		       & (V850_OPERAND_REG | V850_OPERAND_SRG | V850_OPERAND_VREG)) == 0)
		    {
		      errmsg = _("invalid operand");
		      goto error;
		    }

		  insn = v850_insert_operand (insn, operand,
					      ex.X_add_number,
					      &warningmsg);

		  break;

		case O_constant:
		  insn = v850_insert_operand (insn, operand, ex.X_add_number,
					      &warningmsg);
		  break;

		default:
		  /* We need to generate a fixup for this expression.  */
		  if (fc >= MAX_INSN_FIXUPS)
		    as_fatal (_("too many fixups"));

		  fixups[fc].exp     = ex;
		  fixups[fc].opindex = *opindex_ptr;
		  fixups[fc].reloc   = BFD_RELOC_NONE;
		  ++fc;
		  break;
		}
	    }

	  str = input_line_pointer;
	  input_line_pointer = hold;

	  while (*str == ' ' || *str == ',' || *str == '[' || *str == ']'
		 || *str == ')')
	    ++str;
	}

      while (ISSPACE (*str))
	++str;

      if (*str == '\0')
	match = 1;

    error:
      if (match == 0)
	{
	  if ((opindex_ptr - opcode->operands) >= most_match_count)
	    {
	      most_match_count = opindex_ptr - opcode->operands;
	      if (errmsg != NULL)
		strncpy (most_match_errmsg, errmsg, sizeof (most_match_errmsg)-1);
	    }

	  next_opcode = opcode + 1;
	  if (next_opcode->name != NULL
	      && strcmp (next_opcode->name, opcode->name) == 0)
	    {
	      opcode = next_opcode;

	      /* Skip versions that are not supported by the target
		 processor.  */
	      if ((opcode->processors & processor_mask) == 0)
		goto error;

	      continue;
	    }

	  if (most_match_errmsg[0] == 0)
	    /* xgettext:c-format.  */
	    as_bad (_("junk at end of line: `%s'"), str);
	  else
	    as_bad ("%s: %s", copy_of_instruction, most_match_errmsg);

	  if (*input_line_pointer == ']')
	    ++input_line_pointer;

	  ignore_rest_of_line ();
	  input_line_pointer = saved_input_line_pointer;
	  return;
	}

      if (warningmsg != NULL)
	as_warn ("%s", warningmsg);
      break;
    }

  input_line_pointer = str;

  /* Tie dwarf2 debug info to the address at the start of the insn.
     We can't do this after the insn has been output as the current
     frag may have been closed off.  eg. by frag_var.  */
  dwarf2_emit_insn (0);

  /* Write out the instruction.  */
  if (relaxable && fc > 0)
    {
      insn_size = 2;
      fc = 0;

      if (strcmp (opcode->name, "loop") == 0)
	{
	  if (((processor_mask & PROCESSOR_V850E3V5_UP) == 0) || default_disp_size == 22)
	    {
	      insn_size = 4;
	      f = frag_var (rs_machine_dependent, 6, 2, SUBYPTE_LOOP_16_22,
			    fixups[0].exp.X_add_symbol,
			    fixups[0].exp.X_add_number,
			    (char *)(size_t) fixups[0].opindex);
	      md_number_to_chars (f, insn, insn_size);
	      md_number_to_chars (f+4, 0, 4);
	    }
	  else
	    {
	      as_bad (_("loop: 32-bit displacement not supported"));
	    }
	}
      else if (strcmp (opcode->name, "br") == 0
	       || strcmp (opcode->name, "jbr") == 0)
	{
	  if ((processor_mask & PROCESSOR_V850E2_UP) == 0 || default_disp_size == 22)
	    {
	      f = frag_var (rs_machine_dependent, 4, 2, SUBYPTE_UNCOND_9_22,
			    fixups[0].exp.X_add_symbol,
			    fixups[0].exp.X_add_number,
			    (char *)(size_t) fixups[0].opindex);
	      md_number_to_chars (f, insn, insn_size);
	      md_number_to_chars (f + 2, 0, 2);
	    }
	  else
	    {
	      f = frag_var (rs_machine_dependent, 6, 4, SUBYPTE_UNCOND_9_22_32,
			    fixups[0].exp.X_add_symbol,
			    fixups[0].exp.X_add_number,
			    (char *)(size_t) fixups[0].opindex);
	      md_number_to_chars (f, insn, insn_size);
	      md_number_to_chars (f + 2, 0, 4);
	    }
	}
      else /* b<cond>, j<cond>.  */
	{
	  if (default_disp_size == 22
	      || (processor_mask & PROCESSOR_V850E2_UP) == 0)
	    {
	      if (processor_mask & PROCESSOR_V850E2V3_UP && !no_bcond17)
		{
		  if (strcmp (opcode->name, "bsa") == 0)
		    {
		      f = frag_var (rs_machine_dependent, 8, 6, SUBYPTE_SA_9_17_22,
				    fixups[0].exp.X_add_symbol,
				    fixups[0].exp.X_add_number,
				    (char *)(size_t) fixups[0].opindex);
		      md_number_to_chars (f, insn, insn_size);
		      md_number_to_chars (f + 2, 0, 6);
		    }
		  else
		    {
		      f = frag_var (rs_machine_dependent, 6, 4, SUBYPTE_COND_9_17_22,
				    fixups[0].exp.X_add_symbol,
				    fixups[0].exp.X_add_number,
				    (char *)(size_t) fixups[0].opindex);
		      md_number_to_chars (f, insn, insn_size);
		      md_number_to_chars (f + 2, 0, 4);
		    }
		}
	      else
		{
		  if (strcmp (opcode->name, "bsa") == 0)
		    {
		      f = frag_var (rs_machine_dependent, 8, 6, SUBYPTE_SA_9_22,
				    fixups[0].exp.X_add_symbol,
				    fixups[0].exp.X_add_number,
				    (char *)(size_t) fixups[0].opindex);
		      md_number_to_chars (f, insn, insn_size);
		      md_number_to_chars (f + 2, 0, 6);
		    }
		  else
		    {
		      f = frag_var (rs_machine_dependent, 6, 4, SUBYPTE_COND_9_22,
				    fixups[0].exp.X_add_symbol,
				    fixups[0].exp.X_add_number,
				    (char *)(size_t) fixups[0].opindex);
		      md_number_to_chars (f, insn, insn_size);
		      md_number_to_chars (f + 2, 0, 4);
		    }
		}
	    }
	  else
	    {
	      if (processor_mask & PROCESSOR_V850E2V3_UP && !no_bcond17)
		{
		  if (strcmp (opcode->name, "bsa") == 0)
		    {
		      f = frag_var (rs_machine_dependent, 10, 8, SUBYPTE_SA_9_17_22_32,
				    fixups[0].exp.X_add_symbol,
				    fixups[0].exp.X_add_number,
				    (char *)(size_t) fixups[0].opindex);
		      md_number_to_chars (f, insn, insn_size);
		      md_number_to_chars (f + 2, 0, 8);
		    }
		  else
		    {
		      f = frag_var (rs_machine_dependent, 8, 6, SUBYPTE_COND_9_17_22_32,
				    fixups[0].exp.X_add_symbol,
				    fixups[0].exp.X_add_number,
				    (char *)(size_t) fixups[0].opindex);
		      md_number_to_chars (f, insn, insn_size);
		      md_number_to_chars (f + 2, 0, 6);
		    }
		}
	      else
		{
		  if (strcmp (opcode->name, "bsa") == 0)
		    {
		      f = frag_var (rs_machine_dependent, 10, 8, SUBYPTE_SA_9_22_32,
				    fixups[0].exp.X_add_symbol,
				    fixups[0].exp.X_add_number,
				    (char *)(size_t) fixups[0].opindex);
		      md_number_to_chars (f, insn, insn_size);
		      md_number_to_chars (f + 2, 0, 8);
		    }
		  else
		    {
		      f = frag_var (rs_machine_dependent, 8, 6, SUBYPTE_COND_9_22_32,
				    fixups[0].exp.X_add_symbol,
				    fixups[0].exp.X_add_number,
				    (char *)(size_t) fixups[0].opindex);
		      md_number_to_chars (f, insn, insn_size);
		      md_number_to_chars (f + 2, 0, 6);
		    }
		}
	    }
	}
    }
  else
    {
      /* Four byte insns have an opcode with the two high bits on.  */
      if ((insn & 0x0600) == 0x0600)
	insn_size = 4;
      else
	insn_size = 2;

      /* Special case: 32 bit MOV.  */
      if ((insn & 0xffe0) == 0x0620)
	insn_size = 2;

      /* Special case: 32 bit JARL,JMP,JR.  */
      if ((insn & 0x1ffe0) == 0x2e0	/* JARL.  */
	  || (insn & 0x1ffe0) == 0x6e0	/* JMP.  */
	  || (insn & 0x1ffff) == 0x2e0)	/* JR.  */
	insn_size = 2;

      if (obstack_room (& frchain_now->frch_obstack) < (insn_size + extra_data_len))
	{
          frag_wane (frag_now);
          frag_new (0);
	}

      f = frag_more (insn_size);
      md_number_to_chars (f, insn, insn_size);

      if (extra_data_after_insn)
	{
	  f = frag_more (extra_data_len);
	  md_number_to_chars (f, extra_data, extra_data_len);

	  extra_data_after_insn = false;
	}
    }

  /* Create any fixups.  At this point we do not use a
     bfd_reloc_code_real_type, but instead just use the
     BFD_RELOC_UNUSED plus the operand index.  This lets us easily
     handle fixups for any operand type, although that is admittedly
     not a very exciting feature.  We pick a BFD reloc type in
     md_apply_fix.  */
  for (i = 0; i < fc; i++)
    {
      const struct v850_operand *operand;
      bfd_reloc_code_real_type reloc;

      operand = &v850_operands[fixups[i].opindex];

      reloc = fixups[i].reloc;

      if (reloc != BFD_RELOC_NONE)
	{
	  reloc_howto_type *reloc_howto =
	    bfd_reloc_type_lookup (stdoutput, reloc);
	  int size;
	  int address;
	  fixS *fixP;

	  if (!reloc_howto)
	    abort ();

	  size = bfd_get_reloc_size (reloc_howto);

	  /* XXX This will abort on an R_V850_8 reloc -
	     is this reloc actually used?  */
	  if (size != 2 && size != 4)
	    abort ();

	  if (extra_data_len == 0)
	    {
	      address = (f - frag_now->fr_literal) + insn_size - size;
	    }
	  else
	    {
	      address = (f - frag_now->fr_literal) + extra_data_len - size;
	    }

	  if ((operand->flags & V850E_IMMEDIATE32) && (operand->flags & V850_PCREL))
	    {
	      fixups[i].exp.X_add_number += 2;
	    }
	  else if (operand->default_reloc ==  BFD_RELOC_V850_16_PCREL)
	    {
	      fixups[i].exp.X_add_number += 2;
	      address += 2;
	    }

	  /* fprintf (stderr, "0x%x %d %ld\n", address, size, fixups[i].exp.X_add_number);  */
	  fixP = fix_new_exp (frag_now, address, size,
			      &fixups[i].exp,
			      reloc_howto->pc_relative,
			      reloc);

	  fixP->tc_fix_data = (void *) operand;

	  switch (reloc)
	    {
	    case BFD_RELOC_LO16:
	    case BFD_RELOC_V850_LO16_S1:
	    case BFD_RELOC_V850_LO16_SPLIT_OFFSET:
	    case BFD_RELOC_HI16:
	    case BFD_RELOC_HI16_S:
	      fixP->fx_no_overflow = 1;
	      break;
	    default:
	      break;
	    }
	}
      else
	{
	  gas_assert (f != NULL);
	  fix_new_exp (frag_now,
		       f - frag_now->fr_literal, 4,
		       & fixups[i].exp,
		       (operand->flags & V850_PCREL) != 0,
		       (bfd_reloc_code_real_type) (fixups[i].opindex
						   + (int) BFD_RELOC_UNUSED));
	}
    }

  input_line_pointer = saved_input_line_pointer;
}

/* If while processing a fixup, a reloc really needs to be created
   then it is done here.  */

arelent *
tc_gen_reloc (asection *seg ATTRIBUTE_UNUSED, fixS *fixp)
{
  arelent *reloc;

  reloc		      = XNEW (arelent);
  reloc->sym_ptr_ptr  = XNEW (asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address      = fixp->fx_frag->fr_address + fixp->fx_where;

  if (   fixp->fx_r_type == BFD_RELOC_VTABLE_ENTRY
      || fixp->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fixp->fx_r_type == BFD_RELOC_V850_LONGCALL
      || fixp->fx_r_type == BFD_RELOC_V850_LONGJUMP
      || fixp->fx_r_type == BFD_RELOC_V850_ALIGN)
    reloc->addend = fixp->fx_offset;
  else
    {
#if 0
      if (fixp->fx_r_type == BFD_RELOC_32
	  && fixp->fx_pcrel)
	fixp->fx_r_type = BFD_RELOC_32_PCREL;
#endif

      reloc->addend = fixp->fx_addnumber;
    }

  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);

  if (reloc->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    /* xgettext:c-format  */
		    _("reloc %d not supported by object file format"),
		    (int) fixp->fx_r_type);

      xfree (reloc);

      return NULL;
    }

  return reloc;
}

void
v850_handle_align (fragS * frag)
{
  if (v850_relax
      && frag->fr_type == rs_align
      && frag->fr_address + frag->fr_fix > 0
      && frag->fr_offset > 1
      && now_seg != bss_section
      && now_seg != v850_seg_table[SBSS_SECTION].s
      && now_seg != v850_seg_table[TBSS_SECTION].s
      && now_seg != v850_seg_table[ZBSS_SECTION].s)
    fix_new (frag, frag->fr_fix, 2, & abs_symbol, frag->fr_offset, 0,
	     BFD_RELOC_V850_ALIGN);
}

/* Return current size of variable part of frag.  */

int
md_estimate_size_before_relax (fragS *fragp, asection *seg ATTRIBUTE_UNUSED)
{
  if (fragp->fr_subtype >= sizeof (md_relax_table) / sizeof (md_relax_table[0]))
    abort ();

  return md_relax_table[fragp->fr_subtype].rlx_length;
}

long
v850_pcrel_from_section (fixS *fixp, segT section)
{
  /* If the symbol is undefined, or in a section other than our own,
     or it is weak (in which case it may well be in another section,
     then let the linker figure it out.  */
  if (fixp->fx_addsy != (symbolS *) NULL
      && (! S_IS_DEFINED (fixp->fx_addsy)
	  || S_IS_WEAK (fixp->fx_addsy)
	  || (S_GET_SEGMENT (fixp->fx_addsy) != section)))
    return 0;

  return fixp->fx_frag->fr_address + fixp->fx_where;
}

void
md_apply_fix (fixS *fixP, valueT *valueP, segT seg ATTRIBUTE_UNUSED)
{
  valueT value = * valueP;
  char *where;

  if (fixP->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fixP->fx_r_type == BFD_RELOC_V850_LONGCALL
      || fixP->fx_r_type == BFD_RELOC_V850_LONGJUMP
      || fixP->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    {
      fixP->fx_done = 0;
      return;
    }

  if (fixP->fx_addsy == (symbolS *) NULL)
    fixP->fx_addnumber = value,
    fixP->fx_done = 1;

  else if (fixP->fx_pcrel)
    fixP->fx_addnumber = fixP->fx_offset;

  else
    {
      value = fixP->fx_offset;
      if (fixP->fx_subsy != (symbolS *) NULL)
	{
	  if (S_GET_SEGMENT (fixP->fx_subsy) == absolute_section)
	    value -= S_GET_VALUE (fixP->fx_subsy);
	  else
	    /* We don't actually support subtracting a symbol.  */
	    as_bad_subtract (fixP);
	}
      fixP->fx_addnumber = value;
    }

  if ((int) fixP->fx_r_type >= (int) BFD_RELOC_UNUSED)
    {
      int opindex;
      const struct v850_operand *operand;
      unsigned long insn;
      const char *errmsg = NULL;

      opindex = (int) fixP->fx_r_type - (int) BFD_RELOC_UNUSED;
      operand = &v850_operands[opindex];

      /* Fetch the instruction, insert the fully resolved operand
	 value, and stuff the instruction back again.

	 Note the instruction has been stored in little endian
	 format!  */
      where = fixP->fx_frag->fr_literal + fixP->fx_where;

      if (fixP->fx_size > 2)
	insn = bfd_getl32 ((unsigned char *) where);
      else
	insn = bfd_getl16 ((unsigned char *) where);

      /* When inserting loop offsets a backwards displacement
	 is encoded as a positive value.  */
      if (operand->flags & V850_INVERSE_PCREL)
	value = - value;

      insn = v850_insert_operand (insn, operand, (offsetT) value,
				  &errmsg);
      if (errmsg)
	as_warn_where (fixP->fx_file, fixP->fx_line, "%s", errmsg);

      if (fixP->fx_size > 2)
	bfd_putl32 ((bfd_vma) insn, (unsigned char *) where);
      else
	bfd_putl16 ((bfd_vma) insn, (unsigned char *) where);

      if (fixP->fx_done)
	/* Nothing else to do here.  */
	return;

      /* Determine a BFD reloc value based on the operand information.
	 We are only prepared to turn a few of the operands into relocs.  */

      if (operand->default_reloc == BFD_RELOC_NONE)
	{
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("unresolved expression that must be resolved"));
	  fixP->fx_done = 1;
	  return;
	}

      {
	fixP->fx_r_type = operand->default_reloc;
	if (operand->default_reloc ==  BFD_RELOC_V850_16_PCREL)
	  {
	    fixP->fx_where += 2;
	    fixP->fx_size = 2;
	    fixP->fx_addnumber += 2;
	  }
      }
    }
  else if (fixP->fx_done)
    {
      /* We still have to insert the value into memory!  */
      where = fixP->fx_frag->fr_literal + fixP->fx_where;

      if (fixP->tc_fix_data != NULL
          && ((struct v850_operand *) fixP->tc_fix_data)->insert != NULL)
        {
          const char * message = NULL;
          struct v850_operand * operand = (struct v850_operand *) fixP->tc_fix_data;
          unsigned long insn;

          /* The variable "where" currently points at the exact point inside
             the insn where we need to insert the value.  But we need to
             extract the entire insn so we probably need to move "where"
             back a few bytes.  */

          if (fixP->fx_size == 2)
            where -= 2;
          else if (fixP->fx_size == 1)
            where -= 3;

          insn = bfd_getl32 ((unsigned char *) where);

          /* Use the operand's insertion procedure, if present, in order to
             make sure that the value is correctly stored in the insn.  */
          insn = operand->insert (insn, (offsetT) value, & message);
          /* Ignore message even if it is set.  */

          bfd_putl32 ((bfd_vma) insn, (unsigned char *) where);
        }
      else
        {
	  switch (fixP->fx_r_type)
	    {
	    case BFD_RELOC_V850_32_ABS:
	    case BFD_RELOC_V850_32_PCREL:
	      bfd_putl32 (value & 0xfffffffe, (unsigned char *) where);
	      break;

	    case BFD_RELOC_32:
	      bfd_putl32 (value, (unsigned char *) where);
	      break;

	    case BFD_RELOC_V850_23:
	      bfd_putl32 (((value & 0x7f) << 4) | ((value & 0x7fff80) << (16-7))
			  | (bfd_getl32 (where) & ~((0x7f << 4) | (0xffff << 16))),
			  (unsigned char *) where);
	    break;

	    case BFD_RELOC_16:
	    case BFD_RELOC_HI16:
	    case BFD_RELOC_HI16_S:
	    case BFD_RELOC_LO16:
	    case BFD_RELOC_V850_ZDA_16_16_OFFSET:
	    case BFD_RELOC_V850_SDA_16_16_OFFSET:
	    case BFD_RELOC_V850_TDA_16_16_OFFSET:
	    case BFD_RELOC_V850_CALLT_16_16_OFFSET:
	      bfd_putl16 (value & 0xffff, (unsigned char *) where);
	      break;

	    case BFD_RELOC_8:
	      *where = value & 0xff;
	      break;

	    case BFD_RELOC_V850_9_PCREL:
	      bfd_putl16 (((value & 0x1f0) << 7) | ((value & 0x0e) << 3)
			  | (bfd_getl16 (where) & ~((0x1f0 << 7) | (0x0e << 3))), where);
	      break;

	    case BFD_RELOC_V850_17_PCREL:
	      bfd_putl32 (((value & 0x10000) >> (16 - 4)) | ((value & 0xfffe) << 16)
			  | (bfd_getl32 (where) & ~((0x10000 >> (16 - 4)) | (0xfffe << 16))), where);
	      break;

	    case BFD_RELOC_V850_16_PCREL:
	      bfd_putl16 ((-value & 0xfffe) | (bfd_getl16 (where + 2) & 0x0001),
			  (unsigned char *) (where + 2));
	      break;

	    case BFD_RELOC_V850_22_PCREL:
	      bfd_putl32 (((value & 0xfffe) << 16) | ((value & 0x3f0000) >> 16)
			  | (bfd_getl32 (where) & ~((0xfffe << 16) | (0x3f0000 >> 16))), where);
	      break;

	    case BFD_RELOC_V850_16_S1:
	    case BFD_RELOC_V850_LO16_S1:
	    case BFD_RELOC_V850_ZDA_15_16_OFFSET:
	    case BFD_RELOC_V850_SDA_15_16_OFFSET:
	      bfd_putl16 (value & 0xfffe, (unsigned char *) where);
	      break;

	    case BFD_RELOC_V850_16_SPLIT_OFFSET:
	    case BFD_RELOC_V850_LO16_SPLIT_OFFSET:
	    case BFD_RELOC_V850_ZDA_16_16_SPLIT_OFFSET:
	    case BFD_RELOC_V850_SDA_16_16_SPLIT_OFFSET:
	      bfd_putl32 (((value << 16) & 0xfffe0000)
			  | ((value << 5) & 0x20)
			  | (bfd_getl32 (where) & ~0xfffe0020), where);
	      break;

	    case BFD_RELOC_V850_TDA_6_8_OFFSET:
	      *where = (*where & ~0x7e) | ((value >> 1) & 0x7e);
	      break;

	    case BFD_RELOC_V850_TDA_7_8_OFFSET:
	      *where = (*where & ~0x7f) | ((value >> 1) & 0x7f);
	      break;

	    case BFD_RELOC_V850_TDA_7_7_OFFSET:
	      *where = (*where & ~0x7f) | (value & 0x7f);
	      break;

	    case BFD_RELOC_V850_TDA_4_5_OFFSET:
	      *where = (*where & ~0xf) | ((value >> 1) & 0xf);
	      break;

	    case BFD_RELOC_V850_TDA_4_4_OFFSET:
	      *where = (*where & ~0xf) | (value & 0xf);
	      break;

	    case BFD_RELOC_V850_CALLT_6_7_OFFSET:
	      *where = (*where & ~0x3f) | (value & 0x3f);
	      break;

	    default:
	      abort ();
	    }
        }
    }
}

/* Parse a cons expression.  We have to handle hi(), lo(), etc
   on the v850.  */

bfd_reloc_code_real_type
parse_cons_expression_v850 (expressionS *exp)
{
  const char *errmsg;
  bfd_reloc_code_real_type r;

  /* See if there's a reloc prefix like hi() we have to handle.  */
  r = v850_reloc_prefix (NULL, &errmsg);

  /* Do normal expression parsing.  */
  expression (exp);
  return r;
}

/* Create a fixup for a cons expression.  If parse_cons_expression_v850
   found a reloc prefix, then we use that reloc, else we choose an
   appropriate one based on the size of the expression.  */

void
cons_fix_new_v850 (fragS *frag,
		   int where,
		   int size,
		   expressionS *exp,
		   bfd_reloc_code_real_type r)
{
  if (r == BFD_RELOC_NONE)
    {
      if (size == 4)
	r = BFD_RELOC_32;
      if (size == 2)
	r = BFD_RELOC_16;
      if (size == 1)
	r = BFD_RELOC_8;
    }

  if (exp != NULL)
    fix_new_exp (frag, where, size, exp, 0, r);
  else
    fix_new (frag, where, size, NULL, 0, 0, r);
}

bool
v850_fix_adjustable (fixS *fixP)
{
  if (fixP->fx_addsy == NULL)
    return 1;

  /* Don't adjust function names.  */
  if (S_IS_FUNCTION (fixP->fx_addsy))
    return 0;

  /* We need the symbol name for the VTABLE entries.  */
  if (fixP->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fixP->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    return 0;

  return 1;
}

int
v850_force_relocation (struct fix *fixP)
{
  if (fixP->fx_r_type == BFD_RELOC_V850_LONGCALL
      || fixP->fx_r_type == BFD_RELOC_V850_LONGJUMP)
    return 1;

  if (v850_relax
      && (fixP->fx_pcrel
	  || fixP->fx_r_type == BFD_RELOC_V850_ALIGN
	  || fixP->fx_r_type == BFD_RELOC_V850_9_PCREL
	  || fixP->fx_r_type == BFD_RELOC_V850_16_PCREL
	  || fixP->fx_r_type == BFD_RELOC_V850_17_PCREL
	  || fixP->fx_r_type == BFD_RELOC_V850_22_PCREL
	  || fixP->fx_r_type == BFD_RELOC_V850_32_PCREL
	  || fixP->fx_r_type >= BFD_RELOC_UNUSED))
    return 1;

  return generic_force_reloc (fixP);
}

/* Create a v850 note section.  */
void
v850_md_finish (void)
{
  segT note_sec;
  segT orig_seg = now_seg;
  subsegT orig_subseg = now_subseg;
  enum v850_notes id;

  note_sec = subseg_new (V850_NOTE_SECNAME, 0);
  bfd_set_section_flags (note_sec, SEC_HAS_CONTENTS | SEC_READONLY | SEC_MERGE);
  bfd_set_section_alignment (note_sec, 2);

  /* Provide default values for all of the notes.  */
  for (id = V850_NOTE_ALIGNMENT; id <= NUM_V850_NOTES; id++)
    {
      int val = 0;
      char * p;

      /* Follow the standard note section layout:
	 First write the length of the name string.  */
      p = frag_more (4);
      md_number_to_chars (p, 4, 4);

      /* Next comes the length of the "descriptor", i.e., the actual data.  */
      p = frag_more (4);
      md_number_to_chars (p, 4, 4);

      /* Write the note type.  */
      p = frag_more (4);
      md_number_to_chars (p, (valueT) id, 4);

      /* Write the name field.  */
      p = frag_more (4);
      memcpy (p, V850_NOTE_NAME, 4);

      /* Finally, write the descriptor.  */
      p = frag_more (4);
      switch (id)
	{
	case V850_NOTE_ALIGNMENT:
	  val = v850_data_8 ? EF_RH850_DATA_ALIGN8 : EF_RH850_DATA_ALIGN4;
	  break;

	case V850_NOTE_DATA_SIZE:
	  /* GCC does not currently support an option
	     for 32-bit doubles with the V850 backend.  */
	  val = EF_RH850_DOUBLE64;
	  break;

	case V850_NOTE_FPU_INFO:
	  if (! soft_float)
	    switch (machine)
	      {
	      case bfd_mach_v850e3v5: val = EF_RH850_FPU30; break;
	      case bfd_mach_v850e2v3: val = EF_RH850_FPU20; break;
	      default: break;
	      }
	  break;

	default:
	  break;
	}
      md_number_to_chars (p, val, 4);
    }

  /* Paranoia - we probably do not need this.  */
  subseg_set (orig_seg, orig_subseg);
}
