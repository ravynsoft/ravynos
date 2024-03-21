/* xtensa-dis.c.  Disassembly functions for Xtensa.
   Copyright (C) 2003-2023 Free Software Foundation, Inc.
   Contributed by Bob Wilson at Tensilica, Inc. (bwilson@tensilica.com)

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include "xtensa-isa.h"
#include "ansidecl.h"
#include "libiberty.h"
#include "bfd.h"
#include "elf/xtensa.h"
#include "disassemble.h"

#include <setjmp.h>

extern xtensa_isa xtensa_default_isa;

#ifndef MAX
#define MAX(a,b) (a > b ? a : b)
#endif

int show_raw_fields;

struct dis_private
{
  bfd_byte *byte_buf;
  OPCODES_SIGJMP_BUF bailout;
  /* Persistent fields, valid for last_section only.  */
  asection *last_section;
  property_table_entry *insn_table_entries;
  int insn_table_entry_count;
  /* Cached property table search position.  */
  bfd_vma insn_table_cur_addr;
  int insn_table_cur_idx;
};

static void
xtensa_coalesce_insn_tables (struct dis_private *priv)
{
  const int mask = ~(XTENSA_PROP_DATA | XTENSA_PROP_NO_TRANSFORM);
  int count = priv->insn_table_entry_count;
  int i, j;

  /* Loop over all entries, combining adjacent ones that differ only in
     the flag bits XTENSA_PROP_DATA and XTENSA_PROP_NO_TRANSFORM.  */

  for (i = j = 0; j < count; ++i)
    {
      property_table_entry *entry = priv->insn_table_entries + i;

      *entry = priv->insn_table_entries[j];

      for (++j; j < count; ++j)
	{
	  property_table_entry *next = priv->insn_table_entries + j;
	  int fill = xtensa_compute_fill_extra_space (entry);
	  int size = entry->size + fill;

	  if (entry->address + size == next->address)
	    {
	      int entry_flags = entry->flags & mask;
	      int next_flags = next->flags & mask;

	      if (next_flags == entry_flags)
		entry->size = next->address - entry->address + next->size;
	      else
		break;
	    }
	  else
	    {
	      break;
	    }
	}
    }
  priv->insn_table_entry_count = i;
}

static property_table_entry *
xtensa_find_table_entry (bfd_vma memaddr, struct disassemble_info *info)
{
  struct dis_private *priv = (struct dis_private *) info->private_data;
  int i;

  if (priv->insn_table_entries == NULL
      || priv->insn_table_entry_count < 0)
    return NULL;

  if (memaddr < priv->insn_table_cur_addr)
    priv->insn_table_cur_idx = 0;

  for (i = priv->insn_table_cur_idx; i < priv->insn_table_entry_count; ++i)
    {
      property_table_entry *block = priv->insn_table_entries + i;

      if (block->size != 0)
	{
	  if ((memaddr >= block->address
	       && memaddr < block->address + block->size)
	      || memaddr < block->address)
	    {
	      priv->insn_table_cur_addr = memaddr;
	      priv->insn_table_cur_idx = i;
	      return block;
	    }
	}
    }
  return NULL;
}

/* Check whether an instruction crosses an instruction block boundary
   (according to property tables).
   If it does, return 0 (doesn't fit), else return 1.  */

static int
xtensa_instruction_fits (bfd_vma memaddr, int size,
			 property_table_entry *insn_block)
{
  unsigned max_size;

  /* If no property table info, assume it fits.  */
  if (insn_block == NULL || size <= 0)
    return 1;

  /* If too high, limit nextstop by the next insn address.  */
  if (insn_block->address > memaddr)
    {
      /* memaddr is not in an instruction block, but is followed by one.  */
      max_size = insn_block->address - memaddr;
    }
  else
    {
      /* memaddr is in an instruction block, go no further than the end.  */
      max_size = insn_block->address + insn_block->size - memaddr;
    }

  /* Crossing a boundary, doesn't "fit".  */
  if ((unsigned)size > max_size)
    return 0;
  return 1;
}

static int
fetch_data (struct disassemble_info *info, bfd_vma memaddr)
{
  int length, status = 0;
  struct dis_private *priv = (struct dis_private *) info->private_data;
  int insn_size = xtensa_isa_maxlength (xtensa_default_isa);

  insn_size = MAX (insn_size, 4);

  /* Read the maximum instruction size, padding with zeros if we go past
     the end of the text section.  This code will automatically adjust
     length when we hit the end of the buffer.  */

  memset (priv->byte_buf, 0, insn_size);
  for (length = insn_size; length > 0; length--)
    {
      status = (*info->read_memory_func) (memaddr, priv->byte_buf, length,
					  info);
      if (status == 0)
	return length;
    }
  (*info->memory_error_func) (status, memaddr, info);
  OPCODES_SIGLONGJMP (priv->bailout, 1);
  /*NOTREACHED*/
}


static void
print_xtensa_operand (bfd_vma memaddr,
		      struct disassemble_info *info,
		      xtensa_opcode opc,
		      int opnd,
		      unsigned operand_val)
{
  xtensa_isa isa = xtensa_default_isa;
  int signed_operand_val, status;
  bfd_byte litbuf[4];

  if (show_raw_fields)
    {
      if (operand_val < 0xa)
	(*info->fprintf_func) (info->stream, "%u", operand_val);
      else
	(*info->fprintf_func) (info->stream, "0x%x", operand_val);
      return;
    }

  (void) xtensa_operand_decode (isa, opc, opnd, &operand_val);
  signed_operand_val = (int) operand_val;

  if (xtensa_operand_is_register (isa, opc, opnd) == 0)
    {
      if (xtensa_operand_is_PCrelative (isa, opc, opnd) == 1)
	{
	  (void) xtensa_operand_undo_reloc (isa, opc, opnd,
					    &operand_val, memaddr);
	  info->target = operand_val;
	  (*info->print_address_func) (info->target, info);
	  /*  Also display value loaded by L32R (but not if reloc exists,
	      those tend to be wrong):  */
	  if ((info->flags & INSN_HAS_RELOC) == 0
	      && !strcmp ("l32r", xtensa_opcode_name (isa, opc)))
	    status = (*info->read_memory_func) (operand_val, litbuf, 4, info);
	  else
	    status = -1;

	  if (status == 0)
	    {
	      unsigned literal = bfd_get_bits (litbuf, 32,
					       info->endian == BFD_ENDIAN_BIG);

	      (*info->fprintf_func) (info->stream, " (");
	      (*info->print_address_func) (literal, info);
	      (*info->fprintf_func) (info->stream, ")");
	    }
	}
      else
	{
	  if ((signed_operand_val > -256) && (signed_operand_val < 256))
	    (*info->fprintf_styled_func) (info->stream, dis_style_immediate,
					  "%d", signed_operand_val);
	  else
	    (*info->fprintf_styled_func) (info->stream, dis_style_immediate,
					  "0x%x", signed_operand_val);
	}
    }
  else
    {
      int i = 1;
      xtensa_regfile opnd_rf = xtensa_operand_regfile (isa, opc, opnd);
      (*info->fprintf_styled_func) (info->stream, dis_style_register,
				    "%s%u",
				    xtensa_regfile_shortname (isa, opnd_rf),
				    operand_val);
      while (i < xtensa_operand_num_regs (isa, opc, opnd))
	{
	  operand_val++;
	  (*info->fprintf_styled_func) (info->stream, dis_style_register,
					":%s%u",
					xtensa_regfile_shortname (isa, opnd_rf),
					operand_val);
	  i++;
	}
    }
}


/* Print the Xtensa instruction at address MEMADDR on info->stream.
   Returns length of the instruction in bytes.  */

int
print_insn_xtensa (bfd_vma memaddr, struct disassemble_info *info)
{
  unsigned operand_val;
  int bytes_fetched, size, maxsize, i, n, noperands, nslots;
  xtensa_isa isa;
  xtensa_opcode opc;
  xtensa_format fmt;
  static struct dis_private priv;
  static bfd_byte *byte_buf = NULL;
  static xtensa_insnbuf insn_buffer = NULL;
  static xtensa_insnbuf slot_buffer = NULL;
  int first, first_slot, valid_insn;
  property_table_entry *insn_block;
  enum dis_insn_type insn_type;
  bfd_vma target;

  if (!xtensa_default_isa)
    xtensa_default_isa = xtensa_isa_init (0, 0);

  info->target = 0;
  maxsize = xtensa_isa_maxlength (xtensa_default_isa);

  /* Set bytes_per_line to control the amount of whitespace between the hex
     values and the opcode.  For Xtensa, we always print one "chunk" and we
     vary bytes_per_chunk to determine how many bytes to print.  (objdump
     would apparently prefer that we set bytes_per_chunk to 1 and vary
     bytes_per_line but that makes it hard to fit 64-bit instructions on
     an 80-column screen.)  The value of bytes_per_line here is not exactly
     right, because objdump adds an extra space for each chunk so that the
     amount of whitespace depends on the chunk size.  Oh well, it's good
     enough....  Note that we set the minimum size to 4 to accomodate
     literal pools.  */
  info->bytes_per_line = MAX (maxsize, 4);

  /* Allocate buffers the first time through.  */
  if (!insn_buffer)
    {
      insn_buffer = xtensa_insnbuf_alloc (xtensa_default_isa);
      slot_buffer = xtensa_insnbuf_alloc (xtensa_default_isa);
      byte_buf = (bfd_byte *) xmalloc (MAX (maxsize, 4));
    }

  priv.byte_buf = byte_buf;

  info->private_data = (void *) &priv;

  /* Prepare instruction tables.  */

  if (info->section != NULL)
    {
      asection *section = info->section;

      if (priv.last_section != section)
	{
	  bfd *abfd = section->owner;

	  if (priv.last_section != NULL)
	    {
	      /* Reset insn_table_entries.  */
	      priv.insn_table_entry_count = 0;
	      free (priv.insn_table_entries);
	      priv.insn_table_entries = NULL;
	    }
	  priv.last_section = section;

	  /* Read insn_table_entries.  */
	  priv.insn_table_entry_count =
	    xtensa_read_table_entries (abfd, section,
				       &priv.insn_table_entries,
				       XTENSA_PROP_SEC_NAME, false);
	  if (priv.insn_table_entry_count == 0)
	    {
	      free (priv.insn_table_entries);
	      priv.insn_table_entries = NULL;
	      /* Backwards compatibility support.  */
	      priv.insn_table_entry_count =
		xtensa_read_table_entries (abfd, section,
					   &priv.insn_table_entries,
					   XTENSA_INSN_SEC_NAME, false);
	    }
	  priv.insn_table_cur_idx = 0;
	  xtensa_coalesce_insn_tables (&priv);
	}
      /* Else nothing to do, same section as last time.  */
    }

  if (OPCODES_SIGSETJMP (priv.bailout) != 0)
      /* Error return.  */
      return -1;

  /* Fetch the maximum size instruction.  */
  bytes_fetched = fetch_data (info, memaddr);

  insn_block = xtensa_find_table_entry (memaddr, info);

  /* Don't set "isa" before the setjmp to keep the compiler from griping.  */
  isa = xtensa_default_isa;
  size = 0;
  nslots = 0;
  valid_insn = 0;
  fmt = 0;
  if (!insn_block || (insn_block->flags & XTENSA_PROP_INSN))
    {
      /* Copy the bytes into the decode buffer.  */
      memset (insn_buffer, 0, (xtensa_insnbuf_size (isa) *
			       sizeof (xtensa_insnbuf_word)));
      xtensa_insnbuf_from_chars (isa, insn_buffer, priv.byte_buf,
				 bytes_fetched);

      fmt = xtensa_format_decode (isa, insn_buffer);
      if (fmt != XTENSA_UNDEFINED
	  && ((size = xtensa_format_length (isa, fmt)) <= bytes_fetched)
	  && xtensa_instruction_fits (memaddr, size, insn_block))
	{
	  /* Make sure all the opcodes are valid.  */
	  valid_insn = 1;
	  nslots = xtensa_format_num_slots (isa, fmt);
	  for (n = 0; n < nslots; n++)
	    {
	      xtensa_format_get_slot (isa, fmt, n, insn_buffer, slot_buffer);
	      if (xtensa_opcode_decode (isa, fmt, n, slot_buffer)
		  == XTENSA_UNDEFINED)
		{
		  valid_insn = 0;
		  break;
		}
	    }
	}
    }

  if (!valid_insn)
    {
      if (insn_block && (insn_block->flags & XTENSA_PROP_LITERAL)
	  && (memaddr & 3) == 0 && bytes_fetched >= 4)
	{
	  info->bytes_per_chunk = 4;
	  return 4;
	}
      else
	{
	  (*info->fprintf_styled_func) (info->stream,
					dis_style_assembler_directive,
					".byte");
	  (*info->fprintf_func) (info->stream, "\t");
	  (*info->fprintf_styled_func) (info->stream,
					dis_style_immediate,
					"%#02x", priv.byte_buf[0]);
	  return 1;
	}
    }

  if (nslots > 1)
    (*info->fprintf_func) (info->stream, "{ ");

  insn_type = dis_nonbranch;
  target = 0;
  first_slot = 1;
  for (n = 0; n < nslots; n++)
    {
      int imm_pcrel = 0;

      if (first_slot)
	first_slot = 0;
      else
	(*info->fprintf_func) (info->stream, "; ");

      xtensa_format_get_slot (isa, fmt, n, insn_buffer, slot_buffer);
      opc = xtensa_opcode_decode (isa, fmt, n, slot_buffer);
      (*info->fprintf_styled_func) (info->stream,
				    dis_style_mnemonic, "%s",
				    xtensa_opcode_name (isa, opc));

      if (xtensa_opcode_is_branch (isa, opc))
	info->insn_type = dis_condbranch;
      else if (xtensa_opcode_is_jump (isa, opc))
	info->insn_type = dis_branch;
      else if (xtensa_opcode_is_call (isa, opc))
	info->insn_type = dis_jsr;
      else
	info->insn_type = dis_nonbranch;

      /* Print the operands (if any).  */
      noperands = xtensa_opcode_num_operands (isa, opc);
      first = 1;
      for (i = 0; i < noperands; i++)
	{
	  if (xtensa_operand_is_visible (isa, opc, i) == 0)
	    continue;
	  if (first)
	    {
	      (*info->fprintf_func) (info->stream, "\t");
	      first = 0;
	    }
	  else
	    (*info->fprintf_func) (info->stream, ", ");
	  (void) xtensa_operand_get_field (isa, opc, i, fmt, n,
					   slot_buffer, &operand_val);

	  print_xtensa_operand (memaddr, info, opc, i, operand_val);
	  if (xtensa_operand_is_PCrelative (isa, opc, i))
	    ++imm_pcrel;
	}
      if (!imm_pcrel)
	info->insn_type = dis_nonbranch;
      if (info->insn_type != dis_nonbranch)
	{
	  insn_type = info->insn_type;
	  target = info->target;
	}
    }
  info->insn_type = insn_type;
  info->target = target;
  info->insn_info_valid = 1;

  if (nslots > 1)
    (*info->fprintf_func) (info->stream, " }");

  info->bytes_per_chunk = size;
  info->display_endian = info->endian;

  return size;
}

