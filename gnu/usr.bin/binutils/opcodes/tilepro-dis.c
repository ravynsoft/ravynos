/* tilepro-dis.c.  Disassembly routines for the TILEPro architecture.
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
#include "elf/tilepro.h"
#include "elf-bfd.h"
#include "disassemble.h"
#include "opcode/tilepro.h"


#define TREG_ZERO 63

static int
contains_insn (tilepro_mnemonic expected_mnemonic,
	       int expected_first_operand,
	       int expected_second_operand,
	       bfd_vma memaddr,
	       int *last_operand_ret,
	       disassemble_info *info)
{
  struct tilepro_decoded_instruction
    decoded[TILEPRO_MAX_INSTRUCTIONS_PER_BUNDLE];
  bfd_byte opbuf[TILEPRO_BUNDLE_SIZE_IN_BYTES];
  int i, num_instructions;

  if ((*info->read_memory_func) (memaddr, opbuf,
				 TILEPRO_BUNDLE_SIZE_IN_BYTES, info) != 0)
    /* If we cannot even read the memory, it obviously does not have the
       instruction for which we are looking. */
    return 0;

  /* Parse the instructions in the bundle. */
  num_instructions = parse_insn_tilepro (bfd_getl64 (opbuf), memaddr, decoded);

  for (i = 0; i < num_instructions; i++)
    {
      const struct tilepro_opcode *opcode = decoded[i].opcode;

      if (opcode->mnemonic != expected_mnemonic)
	continue;

      if (expected_first_operand != -1
	  && decoded[i].operand_values[0] != expected_first_operand)
	continue;

      if (expected_second_operand != -1
	  && decoded[i].operand_values[1] != expected_second_operand)
	continue;

      *last_operand_ret = decoded[i].operand_values[opcode->num_operands - 1];
      return 1;
    }

  /* No match. */
  return 0;
}


int
print_insn_tilepro (bfd_vma memaddr, disassemble_info *info)
{
  struct tilepro_decoded_instruction
    decoded[TILEPRO_MAX_INSTRUCTIONS_PER_BUNDLE];
  bfd_byte opbuf[TILEPRO_BUNDLE_SIZE_IN_BYTES];
  int status, i, num_instructions, num_printed;
  tilepro_mnemonic padding_mnemonic;

  status = (*info->read_memory_func) (memaddr, opbuf,
                                      TILEPRO_BUNDLE_SIZE_IN_BYTES, info);
  if (status != 0)
    {
      (*info->memory_error_func) (status, memaddr, info);
      return -1;
    }

  info->bytes_per_line = TILEPRO_BUNDLE_SIZE_IN_BYTES;
  info->bytes_per_chunk = TILEPRO_BUNDLE_SIZE_IN_BYTES;
  info->octets_per_byte = 1;
  info->display_endian = BFD_ENDIAN_LITTLE;

  /* Parse the instructions in the bundle.  */
  num_instructions = parse_insn_tilepro (bfd_getl64 (opbuf), memaddr, decoded);

  /* Print the instructions in the bundle.  */
  info->fprintf_func (info->stream, "{ ");
  num_printed = 0;

  /* Determine which nop opcode is used for padding and should be skipped.  */
  padding_mnemonic = TILEPRO_OPC_FNOP;
  for (i = 0; i < num_instructions; i++)
    {
      if (!decoded[i].opcode->can_bundle)
	{
	  /* Instructions that cannot be bundled are padded out with nops,
	     rather than fnops. Displaying them is always clutter.  */
	  padding_mnemonic = TILEPRO_OPC_NOP;
	  break;
	}
    }

  for (i = 0; i < num_instructions; i++)
    {
      const struct tilepro_opcode *opcode = decoded[i].opcode;
      const char *name;
      int j;

      /* Do not print out fnops, unless everything is an fnop, in
	 which case we will print out just the last one.  */
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
	  int num;
	  const struct tilepro_operand *op;
	  const char *spr_name;

	  if (j > 0)
	    info->fprintf_func (info->stream, ",");
	  info->fprintf_func (info->stream, " ");

	  num = decoded[i].operand_values[j];

	  op = decoded[i].operands[j];
	  switch (op->type)
	    {
	    case TILEPRO_OP_TYPE_REGISTER:
	      info->fprintf_func (info->stream, "%s",
				  tilepro_register_names[num]);
	      break;

	    case TILEPRO_OP_TYPE_SPR:
	      spr_name = get_tilepro_spr_name(num);
	      if (spr_name != NULL)
		info->fprintf_func (info->stream, "%s", spr_name);
	      else
		info->fprintf_func (info->stream, "%d", num);
	      break;

	    case TILEPRO_OP_TYPE_IMMEDIATE:
	      {
		bfd_vma addr = 0;
		int found_addr = 0;
		int addr_piece;

		switch (opcode->mnemonic)
		  {
		  case TILEPRO_OPC_ADDLI:
		    if (contains_insn (TILEPRO_OPC_AULI,
				       decoded[i].operand_values[1],
				       TREG_ZERO,
				       memaddr - TILEPRO_BUNDLE_SIZE_IN_BYTES,
				       &addr_piece,
				       info))
		      {
			addr = num + (addr_piece << 16);
			found_addr = 1;
		      }
		    break;

		  case TILEPRO_OPC_AULI:
		    if (contains_insn (TILEPRO_OPC_MOVELI,
				       decoded[i].operand_values[1],
				       -1,
				       memaddr - TILEPRO_BUNDLE_SIZE_IN_BYTES,
				       &addr_piece,
				       info))
		      {
			addr = (num << 16) + addr_piece;
			found_addr = 1;
		      }
		    break;

		  default:
		    /* Operand does not look like a constructed address.  */
		    break;
		  }

		info->fprintf_func (info->stream, "%d", num);

		if (found_addr)
		  {
		    info->fprintf_func (info->stream, " /* ");
		    info->print_address_func (addr, info);
		    info->fprintf_func (info->stream, " */");
		  }
	      }
	      break;

	    case TILEPRO_OP_TYPE_ADDRESS:
	      info->print_address_func ((bfd_vma)(unsigned int) num, info);
	      break;

	    default:
	      abort ();
	    }
	}
    }
  info->fprintf_func (info->stream, " }");

  return TILEPRO_BUNDLE_SIZE_IN_BYTES;
}
