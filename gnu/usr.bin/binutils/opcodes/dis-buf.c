/* Disassemble from a buffer, for GNU.
   Copyright (C) 1993-2023 Free Software Foundation, Inc.

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
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "dis-asm.h"
#include <errno.h>
#include "opintl.h"

/* Get LENGTH bytes from info's buffer, at target address memaddr.
   Transfer them to myaddr.  */
int
buffer_read_memory (bfd_vma memaddr,
		    bfd_byte *myaddr,
		    unsigned int length,
		    struct disassemble_info *info)
{
  unsigned int opb = info->octets_per_byte;
  size_t end_addr_offset = length / opb;
  size_t max_addr_offset = info->buffer_length / opb;
  size_t octets = (memaddr - info->buffer_vma) * opb;

  if (memaddr < info->buffer_vma
      || memaddr - info->buffer_vma > max_addr_offset
      || memaddr - info->buffer_vma + end_addr_offset > max_addr_offset
      || (info->stop_vma && (memaddr >= info->stop_vma
			     || memaddr + end_addr_offset > info->stop_vma)))
    /* Out of bounds.  Use EIO because GDB uses it.  */
    return EIO;
  memcpy (myaddr, info->buffer + octets, length);

  return 0;
}

/* Print an error message.  We can assume that this is in response to
   an error return from buffer_read_memory.  */

void
perror_memory (int status,
	       bfd_vma memaddr,
	       struct disassemble_info *info)
{
  if (status != EIO)
    /* Can't happen.  */
    info->fprintf_func (info->stream, _("Unknown error %d\n"), status);
  else
    {
      /* Actually, address between memaddr and memaddr + len was
	 out of bounds.  */
      info->fprintf_func (info->stream,
			  _("Address 0x%" PRIx64 " is out of bounds.\n"),
			  (uint64_t) memaddr);
    }
}

/* This could be in a separate file, to save miniscule amounts of space
   in statically linked executables.  */

/* Just print the address is hex.  This is included for completeness even
   though both GDB and objdump provide their own (to print symbolic
   addresses).  */

void
generic_print_address (bfd_vma addr, struct disassemble_info *info)
{
  (*info->fprintf_func) (info->stream, "0x%08" PRIx64, (uint64_t) addr);
}

/* Just return NULL.  */

asymbol *
generic_symbol_at_address (bfd_vma addr ATTRIBUTE_UNUSED,
			   struct disassemble_info *info ATTRIBUTE_UNUSED)
{
  return NULL;
}

/* Just return TRUE.  */

bool
generic_symbol_is_valid (asymbol * sym ATTRIBUTE_UNUSED,
			 struct disassemble_info *info ATTRIBUTE_UNUSED)
{
  return true;
}
