/* Opcode printing code for the WebAssembly target
   Copyright (C) 2017-2023 Free Software Foundation, Inc.

   This file is part of libopcodes.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "disassemble.h"
#include "opintl.h"
#include "safe-ctype.h"
#include "floatformat.h"
#include "libiberty.h"
#include "elf-bfd.h"
#include "elf/internal.h"
#include "elf/wasm32.h"
#include <stdint.h>

#include <limits.h>
#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

/* Type names for blocks and signatures.  */
#define BLOCK_TYPE_NONE              0x40
#define BLOCK_TYPE_I32               0x7f
#define BLOCK_TYPE_I64               0x7e
#define BLOCK_TYPE_F32               0x7d
#define BLOCK_TYPE_F64               0x7c

enum wasm_class
{
  wasm_typed,
  wasm_special,
  wasm_break,
  wasm_break_if,
  wasm_break_table,
  wasm_return,
  wasm_call,
  wasm_call_import,
  wasm_call_indirect,
  wasm_get_local,
  wasm_set_local,
  wasm_tee_local,
  wasm_drop,
  wasm_constant_i32,
  wasm_constant_i64,
  wasm_constant_f32,
  wasm_constant_f64,
  wasm_unary,
  wasm_binary,
  wasm_conv,
  wasm_load,
  wasm_store,
  wasm_select,
  wasm_relational,
  wasm_eqz,
  wasm_current_memory,
  wasm_grow_memory,
  wasm_signature
};

struct wasm32_private_data
{
  bool print_registers;
  bool print_well_known_globals;

  /* Limit valid symbols to those with a given prefix.  */
  const char *section_prefix;
};

typedef struct
{
  const char *name;
  const char *description;
} wasm32_options_t;

static const wasm32_options_t options[] =
{
  { "registers", N_("Disassemble \"register\" names") },
  { "globals",   N_("Name well-known globals") },
};

#define WASM_OPCODE(opcode, name, intype, outtype, clas, signedness)     \
  { name, wasm_ ## clas, opcode },

struct wasm32_opcode_s
{
  const char *name;
  enum wasm_class clas;
  unsigned char opcode;
} wasm32_opcodes[] =
{
#include "opcode/wasm.h"
  { NULL, 0, 0 }
};

/* Parse the disassembler options in OPTS and initialize INFO.  */

static void
parse_wasm32_disassembler_options (struct disassemble_info *info,
                                   const char *opts)
{
  struct wasm32_private_data *private = info->private_data;

  while (opts != NULL)
    {
      if (startswith (opts, "registers"))
        private->print_registers = true;
      else if (startswith (opts, "globals"))
        private->print_well_known_globals = true;

      opts = strchr (opts, ',');
      if (opts)
        opts++;
    }
}

/* Check whether SYM is valid.  Special-case absolute symbols, which
   are unhelpful to print, and arguments to a "call" insn, which we
   want to be in a section matching a given prefix.  */

static bool
wasm32_symbol_is_valid (asymbol *sym,
                        struct disassemble_info *info)
{
  struct wasm32_private_data *private_data = info->private_data;

  if (sym == NULL)
    return false;

  if (strcmp(sym->section->name, "*ABS*") == 0)
    return false;

  if (private_data && private_data->section_prefix != NULL
      && strncmp (sym->section->name, private_data->section_prefix,
                  strlen (private_data->section_prefix)))
    return false;

  return true;
}

/* Initialize the disassembler structures for INFO.  */

void
disassemble_init_wasm32 (struct disassemble_info *info)
{
  if (info->private_data == NULL)
    {
      static struct wasm32_private_data private;

      private.print_registers = false;
      private.print_well_known_globals = false;
      private.section_prefix = NULL;

      info->private_data = &private;
    }

  if (info->disassembler_options)
    {
      parse_wasm32_disassembler_options (info, info->disassembler_options);

      info->disassembler_options = NULL;
    }

  info->symbol_is_valid = wasm32_symbol_is_valid;
}

/* Read an LEB128-encoded integer from INFO at address PC, reading one
   byte at a time.  Set ERROR_RETURN if no complete integer could be
   read, LENGTH_RETURN to the number oof bytes read (including bytes
   in incomplete numbers).  SIGN means interpret the number as
   SLEB128.  Unfortunately, this is a duplicate of wasm-module.c's
   wasm_read_leb128 ().  */

static uint64_t
wasm_read_leb128 (bfd_vma pc,
                  struct disassemble_info *info,
                  bool *error_return,
                  unsigned int *length_return,
                  bool sign)
{
  uint64_t result = 0;
  unsigned int num_read = 0;
  unsigned int shift = 0;
  unsigned char byte = 0;
  unsigned char lost, mask;
  int status = 1;

  while (info->read_memory_func (pc + num_read, &byte, 1, info) == 0)
    {
      num_read++;

      if (shift < CHAR_BIT * sizeof (result))
	{
	  result |= ((uint64_t) (byte & 0x7f)) << shift;
	  /* These bits overflowed.  */
	  lost = byte ^ (result >> shift);
	  /* And this is the mask of possible overflow bits.  */
	  mask = 0x7f ^ ((uint64_t) 0x7f << shift >> shift);
	  shift += 7;
	}
      else
	{
	  lost = byte;
	  mask = 0x7f;
	}
      if ((lost & mask) != (sign && (int64_t) result < 0 ? mask : 0))
	status |= 2;

      if ((byte & 0x80) == 0)
	{
	  status &= ~1;
	  if (sign && shift < CHAR_BIT * sizeof (result) && (byte & 0x40))
	    result |= -((uint64_t) 1 << shift);
	  break;
	}
    }

  if (length_return != NULL)
    *length_return = num_read;
  if (error_return != NULL)
    *error_return = status != 0;

  return result;
}

/* Read a 32-bit IEEE float from PC using INFO, convert it to a host
   double, and store it at VALUE.  */

static int
read_f32 (double *value, bfd_vma pc, struct disassemble_info *info)
{
  bfd_byte buf[4];

  if (info->read_memory_func (pc, buf, sizeof (buf), info))
    return -1;

  floatformat_to_double (&floatformat_ieee_single_little, buf,
                         value);

  return sizeof (buf);
}

/* Read a 64-bit IEEE float from PC using INFO, convert it to a host
   double, and store it at VALUE.  */

static int
read_f64 (double *value, bfd_vma pc, struct disassemble_info *info)
{
  bfd_byte buf[8];

  if (info->read_memory_func (pc, buf, sizeof (buf), info))
    return -1;

  floatformat_to_double (&floatformat_ieee_double_little, buf,
                         value);

  return sizeof (buf);
}

/* Main disassembly routine.  Disassemble insn at PC using INFO.  */

int
print_insn_wasm32 (bfd_vma pc, struct disassemble_info *info)
{
  unsigned char opcode;
  struct wasm32_opcode_s *op;
  bfd_byte buffer[16];
  void *stream = info->stream;
  fprintf_ftype prin = info->fprintf_func;
  struct wasm32_private_data *private_data = info->private_data;
  uint64_t val;
  int len;
  unsigned int bytes_read;
  bool error;

  if (info->read_memory_func (pc, buffer, 1, info))
    return -1;

  opcode = buffer[0];

  for (op = wasm32_opcodes; op->name; op++)
    if (op->opcode == opcode)
      break;

  if (!op->name)
    {
      prin (stream, "\t.byte 0x%02x\n", buffer[0]);
      return 1;
    }

  len = 1;

  prin (stream, "\t");
  prin (stream, "%s", op->name);

  if (op->clas == wasm_typed)
    {
      val = wasm_read_leb128 (pc + len, info, &error, &bytes_read, false);
      if (error)
	return -1;
      len += bytes_read;
      switch (val)
	{
	case BLOCK_TYPE_NONE:
	  prin (stream, "[]");
	  break;
	case BLOCK_TYPE_I32:
	  prin (stream, "[i]");
	  break;
	case BLOCK_TYPE_I64:
	  prin (stream, "[l]");
	  break;
	case BLOCK_TYPE_F32:
	  prin (stream, "[f]");
	  break;
	case BLOCK_TYPE_F64:
	  prin (stream, "[d]");
	  break;
	default:
	  return -1;
	}
    }

  switch (op->clas)
    {
    case wasm_special:
    case wasm_eqz:
    case wasm_binary:
    case wasm_unary:
    case wasm_conv:
    case wasm_relational:
    case wasm_drop:
    case wasm_signature:
    case wasm_call_import:
    case wasm_typed:
    case wasm_select:
      break;

    case wasm_break_table:
      {
	uint32_t target_count, i;
	val = wasm_read_leb128 (pc + len, info, &error, &bytes_read,
				false);
	target_count = val;
	if (error || target_count != val || target_count == (uint32_t) -1)
	  return -1;
	len += bytes_read;
	prin (stream, " %u", target_count);
	for (i = 0; i < target_count + 1; i++)
	  {
	    uint32_t target;
	    val = wasm_read_leb128 (pc + len, info, &error, &bytes_read,
				    false);
	    target = val;
	    if (error || target != val)
	      return -1;
	    len += bytes_read;
	    prin (stream, " %u", target);
	  }
      }
      break;

    case wasm_break:
    case wasm_break_if:
      {
	uint32_t depth;
	val = wasm_read_leb128 (pc + len, info, &error, &bytes_read,
				false);
	depth = val;
	if (error || depth != val)
	  return -1;
	len += bytes_read;
	prin (stream, " %u", depth);
      }
      break;

    case wasm_return:
      break;

    case wasm_constant_i32:
    case wasm_constant_i64:
      val = wasm_read_leb128 (pc + len, info, &error, &bytes_read, true);
      if (error)
	return -1;
      len += bytes_read;
      prin (stream, " %" PRId64, val);
      break;

    case wasm_constant_f32:
      {
	double fconstant;
	int ret;
	/* This appears to be the best we can do, even though we're
	   using host doubles for WebAssembly floats.  */
	ret = read_f32 (&fconstant, pc + len, info);
	if (ret < 0)
	  return -1;
	len += ret;
	prin (stream, " %.9g", fconstant);
      }
      break;

    case wasm_constant_f64:
      {
	double fconstant;
	int ret;
	ret = read_f64 (&fconstant, pc + len, info);
	if (ret < 0)
	  return -1;
	len += ret;
	prin (stream, " %.17g", fconstant);
      }
      break;

    case wasm_call:
      {
	uint32_t function_index;
	val = wasm_read_leb128 (pc + len, info, &error, &bytes_read,
				false);
	function_index = val;
	if (error || function_index != val)
	  return -1;
	len += bytes_read;
	prin (stream, " ");
	private_data->section_prefix = ".space.function_index";
	(*info->print_address_func) ((bfd_vma) function_index, info);
	private_data->section_prefix = NULL;
      }
      break;

    case wasm_call_indirect:
      {
	uint32_t type_index, xtra_index;
	val = wasm_read_leb128 (pc + len, info, &error, &bytes_read,
				false);
	type_index = val;
	if (error || type_index != val)
	  return -1;
	len += bytes_read;
	prin (stream, " %u", type_index);
	val = wasm_read_leb128 (pc + len, info, &error, &bytes_read,
				false);
	xtra_index = val;
	if (error || xtra_index != val)
	  return -1;
	len += bytes_read;
	prin (stream, " %u", xtra_index);
      }
      break;

    case wasm_get_local:
    case wasm_set_local:
    case wasm_tee_local:
      {
	uint32_t local_index;
	val = wasm_read_leb128 (pc + len, info, &error, &bytes_read,
				false);
	local_index = val;
	if (error || local_index != val)
	  return -1;
	len += bytes_read;
	prin (stream, " %u", local_index);
	if (strcmp (op->name + 4, "local") == 0)
	  {
	    static const char *locals[] =
	      {
		"$dpc", "$sp1", "$r0", "$r1", "$rpc", "$pc0",
		"$rp", "$fp", "$sp",
		"$r2", "$r3", "$r4", "$r5", "$r6", "$r7",
		"$i0", "$i1", "$i2", "$i3", "$i4", "$i5", "$i6", "$i7",
		"$f0", "$f1", "$f2", "$f3", "$f4", "$f5", "$f6", "$f7",
	      };
	    if (private_data->print_registers
		&& local_index < ARRAY_SIZE (locals))
	      prin (stream, " <%s>", locals[local_index]);
	  }
	else
	  {
	    static const char *globals[] =
	      {
		"$got", "$plt", "$gpo"
	      };
	    if (private_data->print_well_known_globals
		&& local_index < ARRAY_SIZE (globals))
	      prin (stream, " <%s>", globals[local_index]);
	  }
      }
      break;

    case wasm_grow_memory:
    case wasm_current_memory:
      {
	uint32_t reserved_size;
	val = wasm_read_leb128 (pc + len, info, &error, &bytes_read,
				false);
	reserved_size = val;
	if (error || reserved_size != val)
	  return -1;
	len += bytes_read;
	prin (stream, " %u", reserved_size);
      }
      break;

    case wasm_load:
    case wasm_store:
      {
	uint32_t flags, offset;
	val = wasm_read_leb128 (pc + len, info, &error, &bytes_read,
				false);
	flags = val;
	if (error || flags != val)
	  return -1;
	len += bytes_read;
	val = wasm_read_leb128 (pc + len, info, &error, &bytes_read,
				false);
	offset = val;
	if (error || offset != val)
	  return -1;
	len += bytes_read;
	prin (stream, " a=%u %u", flags, offset);
      }
      break;
    }
  return len;
}

/* Print valid disassembler options to STREAM.  */

void
print_wasm32_disassembler_options (FILE *stream)
{
  unsigned int i, max_len = 0;

  fprintf (stream, _("\
The following WebAssembly-specific disassembler options are supported for use\n\
with the -M switch:\n"));

  for (i = 0; i < ARRAY_SIZE (options); i++)
    {
      unsigned int len = strlen (options[i].name);

      if (max_len < len)
	max_len = len;
    }

  for (i = 0, max_len++; i < ARRAY_SIZE (options); i++)
    fprintf (stream, "  %s%*c %s\n",
	     options[i].name,
	     (int)(max_len - strlen (options[i].name)), ' ',
	     _(options[i].description));
}
