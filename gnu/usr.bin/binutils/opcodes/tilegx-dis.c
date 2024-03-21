/* tilegx-dis.c.  Disassembly routines for the TILE-Gx architecture.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

   This file is part of the GNU opcodes library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include <stddef.h>
#include <assert.h>
#include "bfd.h"
#include "elf/tilegx.h"
#include "elf-bfd.h"
#include "disassemble.h"
#include "opcode/tilegx.h"


int
print_insn_tilegx (bfd_vma memaddr, disassemble_info *info)
{
  struct tilegx_decoded_instruction
    decoded[TILEGX_MAX_INSTRUCTIONS_PER_BUNDLE];
  bfd_byte opbuf[TILEGX_BUNDLE_SIZE_IN_BYTES];
  int status, i, num_instructions, num_printed;
  tilegx_mnemonic padding_mnemonic;

  status = (*info->read_memory_func) (memaddr, opbuf,
                                      TILEGX_BUNDLE_SIZE_IN_BYTES, info);
  if (status != 0)
    {
      (*info->memory_error_func) (status, memaddr, info);
      return -1;
    }

  info->bytes_per_line = TILEGX_BUNDLE_SIZE_IN_BYTES;
  info->bytes_per_chunk = TILEGX_BUNDLE_SIZE_IN_BYTES;
  info->octets_per_byte = 1;
  info->display_endian = BFD_ENDIAN_LITTLE;

  /* Parse the instructions in the bundle.  */
  num_instructions =
    parse_insn_tilegx (bfd_getl64 (opbuf), memaddr, decoded);

  /* Print the instructions in the bundle.  */
  info->fprintf_func (info->stream, "{ ");
  num_printed = 0;

  /* Determine which nop opcode is used for padding and should be skipped.  */
  padding_mnemonic = TILEGX_OPC_FNOP;
  for (i = 0; i < num_instructions; i++)
    {
      if (!decoded[i].opcode->can_bundle)
	{
	  /* Instructions that cannot be bundled are padded out with nops,
	     rather than fnops. Displaying them is always clutter. */
	  padding_mnemonic = TILEGX_OPC_NOP;
	  break;
	}
    }

  for (i = 0; i < num_instructions; i++)
    {
      const struct tilegx_opcode *opcode = decoded[i].opcode;
      const char *name;
      int j;

      /* Do not print out fnops, unless everything is an fnop, in
	 which case we will print out just the last one. */
      if (opcode->mnemonic == padding_mnemonic
	  && (num_printed > 0 || i + 1 < num_instructions))
	continue;

      if (num_printed > 0)
	info->fprintf_func (info->stream, " ; ");
      ++num_printed;

      name = opcode->name;
      if (name == NULL)
	name = "<invalid>";
      info->fprintf_func (info->stream, "%s", name);

      for (j = 0; j < opcode->num_operands; j++)
	{
	  bfd_vma num;
	  const struct tilegx_operand *op;
	  const char *spr_name;

	  if (j > 0)
	    info->fprintf_func (info->stream, ",");
	  info->fprintf_func (info->stream, " ");

	  num = decoded[i].operand_values[j];

	  op = decoded[i].operands[j];
	  switch (op->type)
	    {
	    case TILEGX_OP_TYPE_REGISTER:
	      info->fprintf_func (info->stream, "%s",
				  tilegx_register_names[(int) num]);
	      break;
	    case TILEGX_OP_TYPE_SPR:
	      spr_name = get_tilegx_spr_name (num);
	      if (spr_name != NULL)
		info->fprintf_func (info->stream, "%s", spr_name);
	      else
		info->fprintf_func (info->stream, "%d", (int)num);
	      break;
	    case TILEGX_OP_TYPE_IMMEDIATE:
	      info->fprintf_func (info->stream, "%d", (int)num);
	      break;
	    case TILEGX_OP_TYPE_ADDRESS:
	      info->print_address_func (num, info);
	      break;
	    default:
	      abort ();
	    }
	}
    }
  info->fprintf_func (info->stream, " }");

  return TILEGX_BUNDLE_SIZE_IN_BYTES;
}
