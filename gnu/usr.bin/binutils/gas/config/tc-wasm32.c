/* tc-wasm32.c -- Assembler code for the wasm32 target.

   Copyright (C) 2017-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "as.h"
#include "safe-ctype.h"
#include "subsegs.h"
#include "dwarf2dbg.h"
#include "dw2gencfi.h"
#include "elf/wasm32.h"
#include <float.h>

enum wasm_class
{
  wasm_typed,			/* a typed opcode: block, loop, or if */
  wasm_special,			/* a special opcode: unreachable, nop, else,
				   or end */
  wasm_break,			/* "br" */
  wasm_break_if,		/* "br_if" opcode */
  wasm_break_table,		/* "br_table" opcode */
  wasm_return,			/* "return" opcode */
  wasm_call,			/* "call" opcode */
  wasm_call_indirect,		/* "call_indirect" opcode */
  wasm_get_local,		/* "get_local" and "get_global" */
  wasm_set_local,		/* "set_local" and "set_global" */
  wasm_tee_local,		/* "tee_local" */
  wasm_drop,			/* "drop" */
  wasm_constant_i32,		/* "i32.const" */
  wasm_constant_i64,		/* "i64.const" */
  wasm_constant_f32,		/* "f32.const" */
  wasm_constant_f64,		/* "f64.const" */
  wasm_unary,			/* unary operators */
  wasm_binary,			/* binary operators */
  wasm_conv,			/* conversion operators */
  wasm_load,			/* load operators */
  wasm_store,			/* store operators */
  wasm_select,			/* "select" */
  wasm_relational,		/* comparison operators, except for "eqz" */
  wasm_eqz,			/* "eqz" */
  wasm_current_memory,		/* "current_memory" */
  wasm_grow_memory,		/* "grow_memory" */
  wasm_signature		/* "signature", which isn't an opcode */
};

#define WASM_OPCODE(opcode, name, intype, outtype, class, signedness)   \
  { name, wasm_ ## class, opcode },

struct wasm32_opcode_s
{
  const char *name;
  enum wasm_class clas;
  unsigned char opcode;
} wasm32_opcodes[] =
{
#include "opcode/wasm.h"
  {
  NULL, 0, 0}
};

const char comment_chars[] = ";#";
const char line_comment_chars[] = ";#";
const char line_separator_chars[] = "";

const char *md_shortopts = "m:";

const char EXP_CHARS[] = "eE";
const char FLT_CHARS[] = "dD";

/* The target specific pseudo-ops which we support.  */

const pseudo_typeS md_pseudo_table[] =
{
  {NULL, NULL, 0}
};

/* Opcode hash table.  */

static htab_t wasm32_hash;

struct option md_longopts[] =
{
  {NULL, no_argument, NULL, 0}
};

size_t md_longopts_size = sizeof (md_longopts);

/* No relaxation/no machine-dependent frags.  */

int
md_estimate_size_before_relax (fragS * fragp ATTRIBUTE_UNUSED,
			       asection * seg ATTRIBUTE_UNUSED)
{
  abort ();
  return 0;
}

void
md_show_usage (FILE * stream)
{
  fprintf (stream, _("wasm32 assembler options:\n"));
}

/* No machine-dependent options.  */

int
md_parse_option (int c ATTRIBUTE_UNUSED, const char *arg ATTRIBUTE_UNUSED)
{
  return 0;
}

/* No machine-dependent symbols.  */

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return NULL;
}

/* IEEE little-endian floats.  */

const char *
md_atof (int type, char *litP, int *sizeP)
{
  return ieee_md_atof (type, litP, sizeP, false);
}

/* No machine-dependent frags.  */

void
md_convert_frag (bfd * abfd ATTRIBUTE_UNUSED,
		 asection * sec ATTRIBUTE_UNUSED,
		 fragS * fragP ATTRIBUTE_UNUSED)
{
  abort ();
}

/* Build opcode hash table, set some flags.  */

void
md_begin (void)
{
  struct wasm32_opcode_s *opcode;

  wasm32_hash = str_htab_create ();

  /* Insert unique names into hash table.  This hash table then
     provides a quick index to the first opcode with a particular name
     in the opcode table.  */
  for (opcode = wasm32_opcodes; opcode->name; opcode++)
    str_hash_insert (wasm32_hash, opcode->name, opcode, 0);

  linkrelax = 0;
  flag_sectname_subst = 1;
  flag_no_comments = 0;
  flag_keep_locals = 1;
}

/* Do the normal thing for md_section_align.  */

valueT
md_section_align (asection * seg, valueT addr)
{
  int align = bfd_section_alignment (seg);
  return ((addr + (1 << align) - 1) & -(1 << align));
}

/* Apply a fixup, return TRUE if done (and no relocation is
   needed).  */

static bool
apply_full_field_fix (fixS * fixP, char *buf, bfd_vma val, int size)
{
  if (fixP->fx_addsy != NULL || fixP->fx_pcrel)
    {
      fixP->fx_addnumber = val;
      return false;
    }

  number_to_chars_littleendian (buf, val, size);
  return true;
}

/* Apply a fixup (potentially PC-relative), set the fx_done flag if
   done.  */

void
md_apply_fix (fixS * fixP, valueT * valP, segT seg ATTRIBUTE_UNUSED)
{
  char *buf = fixP->fx_where + fixP->fx_frag->fr_literal;
  long val = (long) *valP;

  if (fixP->fx_pcrel)
    {
      switch (fixP->fx_r_type)
	{
	default:
	  bfd_set_error (bfd_error_bad_value);
	  return;

	case BFD_RELOC_32:
	  fixP->fx_r_type = BFD_RELOC_32_PCREL;
	  return;
	}
    }

  if (apply_full_field_fix (fixP, buf, val, fixP->fx_size))
    fixP->fx_done = 1;
}

/* Skip whitespace.  */

static inline char *
skip_space (char *s)
{
  while (*s == ' ' || *s == '\t')
    ++s;
  return s;
}

/* Allow '/' in opcodes.  */

static inline bool
is_part_of_opcode (char c)
{
  return is_part_of_name (c) || (c == '/');
}

/* Extract an opcode.  */

static char *
extract_opcode (char *from, char *to, int limit)
{
  char *op_end;
  int size = 0;

  /* Drop leading whitespace.  */
  from = skip_space (from);
  *to = 0;

  /* Find the op code end.  */
  for (op_end = from; *op_end != 0 && is_part_of_opcode (*op_end);)
    {
      to[size++] = *op_end++;
      if (size + 1 >= limit)
	break;
    }

  to[size] = 0;
  return op_end;
}

/* Produce an unsigned LEB128 integer padded to the right number of
   bytes to store BITS bits, of value VALUE.  Uses FRAG_APPEND_1_CHAR
   to write.  */

static void
wasm32_put_long_uleb128 (int bits, unsigned long value)
{
  unsigned char c;
  int i = 0;

  do
    {
      c = value & 0x7f;
      value >>= 7;
      if (i < (bits - 1) / 7)
	c |= 0x80;
      FRAG_APPEND_1_CHAR (c);
    }
  while (++i < (bits + 6) / 7);
}

/* Produce a signed LEB128 integer, using FRAG_APPEND_1_CHAR to
   write.  */

static void
wasm32_put_sleb128 (long value)
{
  unsigned char c;
  int more;

  do
    {
      c = (value & 0x7f);
      value >>= 7;
      more = !((((value == 0) && ((c & 0x40) == 0))
		|| ((value == -1) && ((c & 0x40) != 0))));
      if (more)
	c |= 0x80;
      FRAG_APPEND_1_CHAR (c);
    }
  while (more);
}

/* Produce an unsigned LEB128 integer, using FRAG_APPEND_1_CHAR to
   write.  */

static void
wasm32_put_uleb128 (unsigned long value)
{
  unsigned char c;

  do
    {
      c = value & 0x7f;
      value >>= 7;
      if (value)
	c |= 0x80;
      FRAG_APPEND_1_CHAR (c);
    }
  while (value);
}

/* Read an integer expression.  Produce an LEB128-encoded integer if
   it's a constant, a padded LEB128 plus a relocation if it's a
   symbol, or a special relocation for <expr>@got, <expr>@gotcode, and
   <expr>@plt{__sigchar_<signature>}.  */

static bool
wasm32_leb128 (char **line, int bits, int sign)
{
  char *t = input_line_pointer;
  char *str = *line;
  char *str0 = str;
  struct reloc_list *reloc;
  expressionS ex;
  int gotrel = 0;
  int pltrel = 0;
  int code = 0;
  const char *relname;

  input_line_pointer = str;
  expression (&ex);

  if (ex.X_op == O_constant && *input_line_pointer != '@')
    {
      long value = ex.X_add_number;

      str = input_line_pointer;
      str = skip_space (str);
      *line = str;
      if (sign)
	wasm32_put_sleb128 (value);
      else
	{
	  if (value < 0)
	    as_bad (_("unexpected negative constant"));
	  wasm32_put_uleb128 (value);
	}
      input_line_pointer = t;
      return str != str0;
    }

  reloc = XNEW (struct reloc_list);
  reloc->u.a.offset_sym = expr_build_dot ();
  if (ex.X_op == O_symbol)
    {
      reloc->u.a.sym = ex.X_add_symbol;
      reloc->u.a.addend = ex.X_add_number;
    }
  else
    {
      reloc->u.a.sym = make_expr_symbol (&ex);
      reloc->u.a.addend = 0;
    }
  /* i32.const fpointer@gotcode */
  if (startswith (input_line_pointer, "@gotcode"))
    {
      gotrel = 1;
      code = 1;
      input_line_pointer += 8;
    }
  /* i32.const data@got */
  else if (startswith (input_line_pointer, "@got"))
    {
      gotrel = 1;
      input_line_pointer += 4;
    }
  /* call f@plt{__sigchar_FiiiiE} */
  else if (startswith (input_line_pointer, "@plt"))
    {
      char *end_of_sig;

      pltrel = 1;
      code = 1;
      input_line_pointer += 4;

      if (startswith (input_line_pointer, "{")
          && (end_of_sig = strchr (input_line_pointer, '}')))
	{
	  char *signature;
	  struct reloc_list *reloc2;
	  size_t siglength = end_of_sig - (input_line_pointer + 1);

	  signature = strndup (input_line_pointer + 1, siglength);

	  reloc2 = XNEW (struct reloc_list);
	  reloc2->u.a.offset_sym = expr_build_dot ();
	  reloc2->u.a.sym = symbol_find_or_make (signature);
	  reloc2->u.a.addend = 0;
	  reloc2->u.a.howto = bfd_reloc_name_lookup
	    (stdoutput, "R_WASM32_PLT_SIG");
	  reloc2->next = reloc_list;
	  reloc_list = reloc2;
	  input_line_pointer = end_of_sig + 1;
	}
      else
	{
	  as_bad (_("no function type on PLT reloc"));
	}
    }

  if (gotrel && code)
    relname = "R_WASM32_LEB128_GOT_CODE";
  else if (gotrel)
    relname = "R_WASM32_LEB128_GOT";
  else if (pltrel)
    relname = "R_WASM32_LEB128_PLT";
  else
    relname = "R_WASM32_LEB128";

  reloc->u.a.howto = bfd_reloc_name_lookup (stdoutput, relname);
  if (!reloc->u.a.howto)
    as_bad (_("couldn't find relocation to use"));
  reloc->file = as_where (&reloc->line);
  reloc->next = reloc_list;
  reloc_list = reloc;

  str = input_line_pointer;
  str = skip_space (str);
  *line = str;
  wasm32_put_long_uleb128 (bits, 0);
  input_line_pointer = t;

  return str != str0;
}

/* Read an integer expression and produce an unsigned LEB128 integer,
   or a relocation for it.  */

static bool
wasm32_uleb128 (char **line, int bits)
{
  return wasm32_leb128 (line, bits, 0);
}

/* Read an integer expression and produce a signed LEB128 integer, or
   a relocation for it.  */

static bool
wasm32_sleb128 (char **line, int bits)
{
  return wasm32_leb128 (line, bits, 1);
}

/* Read an f32.  (Like float_cons ('f')).  */

static void
wasm32_f32 (char **line)
{
  char *t = input_line_pointer;

  input_line_pointer = *line;
  float_cons ('f');
  *line = input_line_pointer;
  input_line_pointer = t;
}

/* Read an f64.  (Like float_cons ('d')).  */

static void
wasm32_f64 (char **line)
{
  char *t = input_line_pointer;

  input_line_pointer = *line;
  float_cons ('d');
  *line = input_line_pointer;
  input_line_pointer = t;
}

/* Assemble a signature from LINE, replacing it with the new input
   pointer.  Signatures are simple expressions matching the regexp
   F[ilfd]*v?E, and interpreted as though they were C++-mangled
   function types on a 64-bit machine. */

static void
wasm32_signature (char **line)
{
  unsigned long count = 0;
  char *str = *line;
  char *ostr;
  char *result;

  if (*str++ != 'F')
    as_bad (_("Not a function type"));
  result = str;
  ostr = str + 1;
  str++;

  while (*str != 'E')
    {
      switch (*str++)
	{
	case 'i':
	case 'l':
	case 'f':
	case 'd':
	  count++;
	  break;
	default:
	  as_bad (_("Unknown type %c\n"), str[-1]);
	}
    }
  wasm32_put_uleb128 (count);
  str = ostr;
  while (*str != 'E')
    {
      switch (*str++)
	{
	case 'i':
	  FRAG_APPEND_1_CHAR (BLOCK_TYPE_I32);
	  break;
	case 'l':
	  FRAG_APPEND_1_CHAR (BLOCK_TYPE_I64);
	  break;
	case 'f':
	  FRAG_APPEND_1_CHAR (BLOCK_TYPE_F32);
	  break;
	case 'd':
	  FRAG_APPEND_1_CHAR (BLOCK_TYPE_F64);
	  break;
	default:
	  as_bad (_("Unknown type"));
	}
    }
  str++;
  switch (*result)
    {
    case 'v':
      FRAG_APPEND_1_CHAR (0x00);	/* no return value */
      break;
    case 'i':
      FRAG_APPEND_1_CHAR (0x01);	/* one return value */
      FRAG_APPEND_1_CHAR (BLOCK_TYPE_I32);
      break;
    case 'l':
      FRAG_APPEND_1_CHAR (0x01);	/* one return value */
      FRAG_APPEND_1_CHAR (BLOCK_TYPE_I64);
      break;
    case 'f':
      FRAG_APPEND_1_CHAR (0x01);	/* one return value */
      FRAG_APPEND_1_CHAR (BLOCK_TYPE_F32);
      break;
    case 'd':
      FRAG_APPEND_1_CHAR (0x01);	/* one return value */
      FRAG_APPEND_1_CHAR (BLOCK_TYPE_F64);
      break;
    default:
      as_bad (_("Unknown type"));
    }
  *line = str;
}

/* Main operands function.  Read the operands for OPCODE from LINE,
   replacing it with the new input pointer.  */

static void
wasm32_operands (struct wasm32_opcode_s *opcode, char **line)
{
  char *str = *line;
  unsigned long block_type = 0;

  FRAG_APPEND_1_CHAR (opcode->opcode);
  str = skip_space (str);
  if (str[0] == '[')
    {
      if (opcode->clas == wasm_typed)
	{
	  str++;
	  block_type = BLOCK_TYPE_NONE;
	  if (str[0] != ']')
	    {
	      str = skip_space (str);
	      switch (str[0])
		{
		case 'i':
		  block_type = BLOCK_TYPE_I32;
		  str++;
		  break;
		case 'l':
		  block_type = BLOCK_TYPE_I64;
		  str++;
		  break;
		case 'f':
		  block_type = BLOCK_TYPE_F32;
		  str++;
		  break;
		case 'd':
		  block_type = BLOCK_TYPE_F64;
		  str++;
		  break;
		}
	      str = skip_space (str);
	      if (str[0] == ']')
		str++;
	      else
		as_bad (_("only single block types allowed"));
	      str = skip_space (str);
	    }
	  else
	    {
	      str++;
	      str = skip_space (str);
	    }
	}
      else
	as_bad (_("instruction does not take a block type"));
    }

  switch (opcode->clas)
    {
    case wasm_drop:
    case wasm_special:
    case wasm_binary:
    case wasm_unary:
    case wasm_relational:
    case wasm_select:
    case wasm_eqz:
    case wasm_conv:
    case wasm_return:
      break;
    case wasm_typed:
      if (block_type == 0)
	as_bad (_("missing block type"));
      FRAG_APPEND_1_CHAR (block_type);
      break;
    case wasm_store:
    case wasm_load:
      if (str[0] == 'a' && str[1] == '=')
	{
	  str += 2;
	  if (!wasm32_uleb128 (&str, 32))
	    as_bad (_("missing alignment hint"));
	}
      else
	{
	  as_bad (_("missing alignment hint"));
	}
      str = skip_space (str);
      if (!wasm32_uleb128 (&str, 32))
	as_bad (_("missing offset"));
      break;
    case wasm_set_local:
    case wasm_get_local:
    case wasm_tee_local:
      if (!wasm32_uleb128 (&str, 32))
	as_bad (_("missing local index"));
      break;
    case wasm_break:
    case wasm_break_if:
      if (!wasm32_uleb128 (&str, 32))
	as_bad (_("missing break count"));
      break;
    case wasm_current_memory:
    case wasm_grow_memory:
      if (!wasm32_uleb128 (&str, 32))
	as_bad (_("missing reserved current_memory/grow_memory argument"));
      break;
    case wasm_call:
      if (!wasm32_uleb128 (&str, 32))
	as_bad (_("missing call argument"));
      break;
    case wasm_call_indirect:
      if (!wasm32_uleb128 (&str, 32))
	as_bad (_("missing call signature"));
      if (!wasm32_uleb128 (&str, 32))
	as_bad (_("missing table index"));
      break;
    case wasm_constant_i32:
      wasm32_sleb128 (&str, 32);
      break;
    case wasm_constant_i64:
      wasm32_sleb128 (&str, 64);
      break;
    case wasm_constant_f32:
      wasm32_f32 (&str);
      return;
    case wasm_constant_f64:
      wasm32_f64 (&str);
      return;
    case wasm_break_table:
      {
	do
	  {
	    wasm32_uleb128 (&str, 32);
	    str = skip_space (str);
	  }
	while (str[0]);

	break;
      }
    case wasm_signature:
      wasm32_signature (&str);
    }
  str = skip_space (str);

  if (*str)
    as_bad (_("junk at end of line, first unrecognized character is `%c'"),
	    *str);

  *line = str;

  return;
}

/* Main assembly function.  Find the opcode and call
   wasm32_operands().  */

void
md_assemble (char *str)
{
  char op[32];
  char *t;
  struct wasm32_opcode_s *opcode;

  str = skip_space (extract_opcode (str, op, sizeof (op)));

  if (!op[0])
    as_bad (_("can't find opcode "));

  opcode = (struct wasm32_opcode_s *) str_hash_find (wasm32_hash, op);

  if (opcode == NULL)
    {
      as_bad (_("unknown opcode `%s'"), op);
      return;
    }

  dwarf2_emit_insn (0);

  t = input_line_pointer;
  wasm32_operands (opcode, &str);
  input_line_pointer = t;
}

/* Don't replace PLT/GOT relocations with section symbols, so they
   don't get an addend.  */

int
wasm32_force_relocation (fixS * f)
{
  if (f->fx_r_type == BFD_RELOC_WASM32_LEB128_PLT
      || f->fx_r_type == BFD_RELOC_WASM32_LEB128_GOT)
    return 1;

  return 0;
}

/* Don't replace PLT/GOT relocations with section symbols, so they
   don't get an addend.  */

bool
wasm32_fix_adjustable (fixS * fixP)
{
  if (fixP->fx_addsy == NULL)
    return true;

  if (fixP->fx_r_type == BFD_RELOC_WASM32_LEB128_PLT
      || fixP->fx_r_type == BFD_RELOC_WASM32_LEB128_GOT)
    return false;

  return true;
}

/* Generate a reloc for FIXP.  */

arelent *
tc_gen_reloc (asection * sec ATTRIBUTE_UNUSED, fixS * fixp)
{
  arelent *reloc;

  reloc = (arelent *) xmalloc (sizeof (*reloc));
  reloc->sym_ptr_ptr = (asymbol **) xmalloc (sizeof (asymbol *));
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;

  /* Make sure none of our internal relocations make it this far.
     They'd better have been fully resolved by this point.  */
  gas_assert ((int) fixp->fx_r_type > 0);

  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
  if (reloc->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("cannot represent `%s' relocation in object file"),
		    bfd_get_reloc_code_name (fixp->fx_r_type));
      return NULL;
    }

  reloc->addend = fixp->fx_offset;

  return reloc;
}
