/* BFD back-end for verilog hex memory dump files.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Written by Anthony Green <green@moxielogic.com>

   This file is part of BFD, the Binary File Descriptor library.

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


/* SUBSECTION
	Verilog hex memory file handling

   DESCRIPTION

	Verilog hex memory files cannot hold anything but addresses
	and data, so that's all that we implement.

	The syntax of the text file is described in the IEEE standard
	for Verilog.  Briefly, the file contains two types of tokens:
	data and optional addresses.  The tokens are separated by
	whitespace and comments.  Comments may be single line or
	multiline, using syntax similar to C++.  Addresses are
	specified by a leading "at" character (@) and are always
	hexadecimal strings.  Data and addresses may contain
	underscore (_) characters.

	If no address is specified, the data is assumed to start at
	address 0.  Similarly, if data exists before the first
	specified address, then that data is assumed to start at
	address 0.


   EXAMPLE
	@1000
	01 ae 3f 45 12

   DESCRIPTION
	@1000 specifies the starting address for the memory data.
	The following characters describe the 5 bytes at 0x1000.  */


#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "libiberty.h"
#include "safe-ctype.h"

/* Modified by obcopy.c
   Data width in bytes.  */
unsigned int VerilogDataWidth = 1;

/* Modified by obcopy.c
   Data endianness.  */
enum bfd_endian VerilogDataEndianness = BFD_ENDIAN_UNKNOWN;

/* Macros for converting between hex and binary.  */

static const char digs[] = "0123456789ABCDEF";

#define NIBBLE(x)    hex_value (x)
#define HEX(buffer) ((NIBBLE ((buffer)[0]) << 4) + NIBBLE ((buffer)[1]))
#define TOHEX(d, x) \
	d[1] = digs[(x) & 0xf]; \
	d[0] = digs[((x) >> 4) & 0xf];

/* When writing a verilog memory dump file, we write them in the order
   in which they appear in memory. This structure is used to hold them
   in memory.  */

struct verilog_data_list_struct
{
  struct verilog_data_list_struct *next;
  bfd_byte * data;
  bfd_vma where;
  bfd_size_type size;
};

typedef struct verilog_data_list_struct verilog_data_list_type;

/* The verilog tdata information.  */

typedef struct verilog_data_struct
{
  verilog_data_list_type *head;
  verilog_data_list_type *tail;
}
tdata_type;

static bool
verilog_set_arch_mach (bfd *abfd, enum bfd_architecture arch, unsigned long mach)
{
  if (arch != bfd_arch_unknown)
    return bfd_default_set_arch_mach (abfd, arch, mach);

  abfd->arch_info = & bfd_default_arch_struct;
  return true;
}

/* We have to save up all the output for a splurge before output.  */

static bool
verilog_set_section_contents (bfd *abfd,
			      sec_ptr section,
			      const void * location,
			      file_ptr offset,
			      bfd_size_type bytes_to_do)
{
  tdata_type *tdata = abfd->tdata.verilog_data;
  verilog_data_list_type *entry;

  entry = (verilog_data_list_type *) bfd_alloc (abfd, sizeof (* entry));
  if (entry == NULL)
    return false;

  if (bytes_to_do
      && (section->flags & SEC_ALLOC)
      && (section->flags & SEC_LOAD))
    {
      bfd_byte *data;

      data = (bfd_byte *) bfd_alloc (abfd, bytes_to_do);
      if (data == NULL)
	return false;
      memcpy ((void *) data, location, (size_t) bytes_to_do);

      entry->data = data;
      entry->where = section->lma + offset;
      entry->size = bytes_to_do;

      /* Sort the records by address.  Optimize for the common case of
	 adding a record to the end of the list.  */
      if (tdata->tail != NULL
	  && entry->where >= tdata->tail->where)
	{
	  tdata->tail->next = entry;
	  entry->next = NULL;
	  tdata->tail = entry;
	}
      else
	{
	  verilog_data_list_type **look;

	  for (look = &tdata->head;
	       *look != NULL && (*look)->where < entry->where;
	       look = &(*look)->next)
	    ;
	  entry->next = *look;
	  *look = entry;
	  if (entry->next == NULL)
	    tdata->tail = entry;
	}
    }
  return true;
}

static bool
verilog_write_address (bfd *abfd, bfd_vma address)
{
  char buffer[20];
  char *dst = buffer;
  bfd_size_type wrlen;

  /* Write the address.  */
  *dst++ = '@';
#ifdef BFD64
  if (address >= (bfd_vma)1 << 32)
    {
      TOHEX (dst, (address >> 56));
      dst += 2;
      TOHEX (dst, (address >> 48));
      dst += 2;
      TOHEX (dst, (address >> 40));
      dst += 2;
      TOHEX (dst, (address >> 32));
      dst += 2;
    }
#endif
  TOHEX (dst, (address >> 24));
  dst += 2;
  TOHEX (dst, (address >> 16));
  dst += 2;
  TOHEX (dst, (address >> 8));
  dst += 2;
  TOHEX (dst, (address));
  dst += 2;
  *dst++ = '\r';
  *dst++ = '\n';
  wrlen = dst - buffer;

  return bfd_bwrite ((void *) buffer, wrlen, abfd) == wrlen;
}

/* Write a record of type, of the supplied number of bytes. The
   supplied bytes and length don't have a checksum.  That's worked
   out here.  */

static bool
verilog_write_record (bfd *abfd,
		      const bfd_byte *data,
		      const bfd_byte *end)
{
  char buffer[52];
  const bfd_byte *src = data;
  char *dst = buffer;
  bfd_size_type wrlen;

  /* Paranoia - check that we will not overflow "buffer".  */
  if (((end - data) * 2) /* Number of hex characters we want to emit.  */
      + ((end - data) / VerilogDataWidth) /* Number of spaces we want to emit.  */
      + 2 /* The carriage return & line feed characters.  */
      > (long) sizeof (buffer))
    {
      /* FIXME: Should we generate an error message ?  */
      return false;
    }

  /* Write the data.
     FIXME: Under some circumstances we can emit a space at the end of
     the line.  This is not really necessary, but catching these cases
     would make the code more complicated.  */
  if (VerilogDataWidth == 1)
    {
      for (src = data; src < end;)
	{
	  TOHEX (dst, *src);
	  dst += 2;
	  src ++;
	  if (src < end)
	    *dst++ = ' ';
	}
    }
  else if ((VerilogDataEndianness == BFD_ENDIAN_UNKNOWN && bfd_little_endian (abfd)) /* FIXME: Can this happen ?  */
	   || (VerilogDataEndianness == BFD_ENDIAN_LITTLE))
    {
      /* If the input byte stream contains:
	   05 04 03 02 01 00
	 and VerilogDataWidth is 4 then we want to emit:
           02030405 0001  */
      int i;

      for (src = data; src < (end - VerilogDataWidth); src += VerilogDataWidth)
	{
	  for (i = VerilogDataWidth - 1; i >= 0; i--)
	    {
	      TOHEX (dst, src[i]);
	      dst += 2;
	    }
	  *dst++ = ' ';
	}

      /* Emit any remaining bytes.  Be careful not to read beyond "end".  */
      while (end > src)
	{
	  -- end;
	  TOHEX (dst, *end);
	  dst += 2;
	}

      /* FIXME: Should padding bytes be inserted here ?  */
    }
  else /* Big endian output.  */
    {
      for (src = data; src < end;)
	{
	  TOHEX (dst, *src);
	  dst += 2;
	  ++ src;
	  if ((src - data) % VerilogDataWidth == 0)
	    *dst++ = ' ';
	}
      /* FIXME: Should padding bytes be inserted here ?  */
    }

  *dst++ = '\r';
  *dst++ = '\n';
  wrlen = dst - buffer;

  return bfd_bwrite ((void *) buffer, wrlen, abfd) == wrlen;
}

static bool
verilog_write_section (bfd *abfd,
		       tdata_type *tdata ATTRIBUTE_UNUSED,
		       verilog_data_list_type *list)
{
  unsigned int octets_written = 0;
  bfd_byte *location = list->data;

  /* Insist that the starting address is a multiple of the data width.  */
  if (list->where % VerilogDataWidth)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  verilog_write_address (abfd, list->where / VerilogDataWidth);
  while (octets_written < list->size)
    {
      unsigned int octets_this_chunk = list->size - octets_written;

      if (octets_this_chunk > 16)
	octets_this_chunk = 16;

      if (! verilog_write_record (abfd,
				  location,
				  location + octets_this_chunk))
	return false;

      octets_written += octets_this_chunk;
      location += octets_this_chunk;
    }

  return true;
}

static bool
verilog_write_object_contents (bfd *abfd)
{
  tdata_type *tdata = abfd->tdata.verilog_data;
  verilog_data_list_type *list;

  /* Now wander though all the sections provided and output them.  */
  list = tdata->head;

  while (list != (verilog_data_list_type *) NULL)
    {
      if (! verilog_write_section (abfd, tdata, list))
	return false;
      list = list->next;
    }
  return true;
}

/* Initialize by filling in the hex conversion array.  */

static void
verilog_init (void)
{
  static bool inited = false;

  if (! inited)
    {
      inited = true;
      hex_init ();
    }
}

/* Set up the verilog tdata information.  */

static bool
verilog_mkobject (bfd *abfd)
{
  tdata_type *tdata;

  verilog_init ();

  tdata = (tdata_type *) bfd_alloc (abfd, sizeof (tdata_type));
  if (tdata == NULL)
    return false;

  abfd->tdata.verilog_data = tdata;
  tdata->head = NULL;
  tdata->tail = NULL;

  return true;
}

#define verilog_close_and_cleanup		     _bfd_generic_close_and_cleanup
#define verilog_bfd_free_cached_info		     _bfd_generic_bfd_free_cached_info
#define verilog_new_section_hook		     _bfd_generic_new_section_hook
#define verilog_bfd_is_target_special_symbol	     _bfd_bool_bfd_asymbol_false
#define verilog_bfd_is_local_label_name		     bfd_generic_is_local_label_name
#define verilog_get_lineno			     _bfd_nosymbols_get_lineno
#define verilog_find_nearest_line		     _bfd_nosymbols_find_nearest_line
#define verilog_find_nearest_line_with_alt	     _bfd_nosymbols_find_nearest_line_with_alt
#define verilog_find_inliner_info		     _bfd_nosymbols_find_inliner_info
#define verilog_make_empty_symbol		     _bfd_generic_make_empty_symbol
#define verilog_bfd_make_debug_symbol		     _bfd_nosymbols_bfd_make_debug_symbol
#define verilog_read_minisymbols		     _bfd_generic_read_minisymbols
#define verilog_minisymbol_to_symbol		     _bfd_generic_minisymbol_to_symbol
#define verilog_get_section_contents_in_window	     _bfd_generic_get_section_contents_in_window
#define verilog_bfd_get_relocated_section_contents   bfd_generic_get_relocated_section_contents
#define verilog_bfd_relax_section		     bfd_generic_relax_section
#define verilog_bfd_gc_sections			     bfd_generic_gc_sections
#define verilog_bfd_merge_sections		     bfd_generic_merge_sections
#define verilog_bfd_is_group_section		     bfd_generic_is_group_section
#define verilog_bfd_group_name			     bfd_generic_group_name
#define verilog_bfd_discard_group		     bfd_generic_discard_group
#define verilog_section_already_linked		     _bfd_generic_section_already_linked
#define verilog_bfd_link_hash_table_create	     _bfd_generic_link_hash_table_create
#define verilog_bfd_link_add_symbols		     _bfd_generic_link_add_symbols
#define verilog_bfd_link_just_syms		     _bfd_generic_link_just_syms
#define verilog_bfd_final_link			     _bfd_generic_final_link
#define verilog_bfd_link_split_section		     _bfd_generic_link_split_section

const bfd_target verilog_vec =
{
  "verilog",			/* Name.  */
  bfd_target_verilog_flavour,
  BFD_ENDIAN_UNKNOWN,		/* Target byte order.  */
  BFD_ENDIAN_UNKNOWN,		/* Target headers byte order.  */
  (HAS_RELOC | EXEC_P |		/* Object flags.  */
   HAS_LINENO | HAS_DEBUG |
   HAS_SYMS | HAS_LOCALS | WP_TEXT | D_PAGED),
  (SEC_CODE | SEC_DATA | SEC_ROM | SEC_HAS_CONTENTS
   | SEC_ALLOC | SEC_LOAD | SEC_RELOC),	/* Section flags.  */
  0,				/* Leading underscore.  */
  ' ',				/* AR_pad_char.  */
  16,				/* AR_max_namelen.  */
  0,				/* match priority.  */
  TARGET_KEEP_UNUSED_SECTION_SYMBOLS, /* keep unused section symbols.  */
  bfd_getb64, bfd_getb_signed_64, bfd_putb64,
  bfd_getb32, bfd_getb_signed_32, bfd_putb32,
  bfd_getb16, bfd_getb_signed_16, bfd_putb16,	/* Data.  */
  bfd_getb64, bfd_getb_signed_64, bfd_putb64,
  bfd_getb32, bfd_getb_signed_32, bfd_putb32,
  bfd_getb16, bfd_getb_signed_16, bfd_putb16,	/* Hdrs.  */

  {
    _bfd_dummy_target,
    _bfd_dummy_target,
    _bfd_dummy_target,
    _bfd_dummy_target,
  },
  {
    _bfd_bool_bfd_false_error,
    verilog_mkobject,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
  },
  {				/* bfd_write_contents.  */
    _bfd_bool_bfd_false_error,
    verilog_write_object_contents,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
  },

  BFD_JUMP_TABLE_GENERIC (_bfd_generic),
  BFD_JUMP_TABLE_COPY (_bfd_generic),
  BFD_JUMP_TABLE_CORE (_bfd_nocore),
  BFD_JUMP_TABLE_ARCHIVE (_bfd_noarchive),
  BFD_JUMP_TABLE_SYMBOLS (_bfd_nosymbols),
  BFD_JUMP_TABLE_RELOCS (_bfd_norelocs),
  BFD_JUMP_TABLE_WRITE (verilog),
  BFD_JUMP_TABLE_LINK (_bfd_nolink),
  BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),

  NULL,

  NULL
};
