/* Compressed section support (intended for debug sections).
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

#include "sysdep.h"
#include <zlib.h>
#ifdef HAVE_ZSTD
#include <zstd.h>
#endif
#include "bfd.h"
#include "elf-bfd.h"
#include "libbfd.h"
#include "safe-ctype.h"
#include "libiberty.h"

#define MAX_COMPRESSION_HEADER_SIZE 24

/*
EXTERNAL
.{* Types of compressed DWARF debug sections.  *}
.enum compressed_debug_section_type
.{
.  COMPRESS_DEBUG_NONE = 0,
.  COMPRESS_DEBUG_GNU_ZLIB = 1 << 1,
.  COMPRESS_DEBUG_GABI_ZLIB = 1 << 2,
.  COMPRESS_DEBUG_ZSTD = 1 << 3,
.  COMPRESS_UNKNOWN = 1 << 4
.};
.
.{* Tuple for compressed_debug_section_type and their name.  *}
.struct compressed_type_tuple
.{
.  enum compressed_debug_section_type type;
.  const char *name;
.};
.
.{* Compression header ch_type values.  *}
.enum compression_type
.{
.  ch_none = 0,
.  ch_compress_zlib = 1	,	{* Compressed with zlib.  *}
.  ch_compress_zstd = 2		{* Compressed with zstd (www.zstandard.org).  *}
.};
.
.static inline char *
.bfd_debug_name_to_zdebug (bfd *abfd, const char *name)
.{
.  size_t len = strlen (name);
.  char *new_name = (char *) bfd_alloc (abfd, len + 2);
.  if (new_name == NULL)
.    return NULL;
.  new_name[0] = '.';
.  new_name[1] = 'z';
.  memcpy (new_name + 2, name + 1, len);
.  return new_name;
.}
.
.static inline char *
.bfd_zdebug_name_to_debug (bfd *abfd, const char *name)
.{
.  size_t len = strlen (name);
.  char *new_name = (char *) bfd_alloc (abfd, len);
.  if (new_name == NULL)
.    return NULL;
.  new_name[0] = '.';
.  memcpy (new_name + 1, name + 2, len - 1);
.  return new_name;
.}
.
*/

/* Display texts for type of compressed DWARF debug sections.  */
static const struct compressed_type_tuple compressed_debug_section_names[] =
{
  { COMPRESS_DEBUG_NONE, "none" },
  { COMPRESS_DEBUG_GABI_ZLIB, "zlib" },
  { COMPRESS_DEBUG_GNU_ZLIB, "zlib-gnu" },
  { COMPRESS_DEBUG_GABI_ZLIB, "zlib-gabi" },
  { COMPRESS_DEBUG_ZSTD, "zstd" },
};

/*
FUNCTION
	bfd_get_compression_algorithm
SYNOPSIS
	enum compressed_debug_section_type
	  bfd_get_compression_algorithm (const char *name);
DESCRIPTION
	Return compressed_debug_section_type from a string representation.
*/
enum compressed_debug_section_type
bfd_get_compression_algorithm (const char *name)
{
  for (unsigned i = 0; i < ARRAY_SIZE (compressed_debug_section_names); ++i)
    if (strcasecmp (compressed_debug_section_names[i].name, name) == 0)
      return compressed_debug_section_names[i].type;

  return COMPRESS_UNKNOWN;
}

/*
FUNCTION
	bfd_get_compression_algorithm_name
SYNOPSIS
	const char *bfd_get_compression_algorithm_name
	  (enum compressed_debug_section_type type);
DESCRIPTION
	Return compression algorithm name based on the type.
*/
const char *
bfd_get_compression_algorithm_name (enum compressed_debug_section_type type)
{
  for (unsigned i = 0; i < ARRAY_SIZE (compressed_debug_section_names); ++i)
    if (type == compressed_debug_section_names[i].type)
      return compressed_debug_section_names[i].name;

  return NULL;
}

/*
FUNCTION
	bfd_update_compression_header

SYNOPSIS
	void bfd_update_compression_header
	  (bfd *abfd, bfd_byte *contents, asection *sec);

DESCRIPTION
	Set the compression header at CONTENTS of SEC in ABFD and update
	elf_section_flags for compression.
*/

void
bfd_update_compression_header (bfd *abfd, bfd_byte *contents,
			       asection *sec)
{
  if ((abfd->flags & BFD_COMPRESS) == 0)
    abort ();

  switch (bfd_get_flavour (abfd))
    {
    case bfd_target_elf_flavour:
      if ((abfd->flags & BFD_COMPRESS_GABI) != 0)
	{
	  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
	  struct bfd_elf_section_data * esd = elf_section_data (sec);
	  enum compression_type ch_type = (abfd->flags & BFD_COMPRESS_ZSTD
					   ? ch_compress_zstd
					   : ch_compress_zlib);

	  /* Set the SHF_COMPRESSED bit.  */
	  elf_section_flags (sec) |= SHF_COMPRESSED;

	  if (bed->s->elfclass == ELFCLASS32)
	    {
	      Elf32_External_Chdr *echdr = (Elf32_External_Chdr *) contents;
	      bfd_put_32 (abfd, ch_type, &echdr->ch_type);
	      bfd_put_32 (abfd, sec->size, &echdr->ch_size);
	      bfd_put_32 (abfd, 1u << sec->alignment_power,
			  &echdr->ch_addralign);
	      /* bfd_log2 (alignof (Elf32_Chdr)) */
	      bfd_set_section_alignment (sec, 2);
	      esd->this_hdr.sh_addralign = 4;
	    }
	  else
	    {
	      Elf64_External_Chdr *echdr = (Elf64_External_Chdr *) contents;
	      bfd_put_32 (abfd, ch_type, &echdr->ch_type);
	      bfd_put_32 (abfd, 0, &echdr->ch_reserved);
	      bfd_put_64 (abfd, sec->size, &echdr->ch_size);
	      bfd_put_64 (abfd, UINT64_C (1) << sec->alignment_power,
			  &echdr->ch_addralign);
	      /* bfd_log2 (alignof (Elf64_Chdr)) */
	      bfd_set_section_alignment (sec, 3);
	      esd->this_hdr.sh_addralign = 8;
	    }
	  break;
	}

      /* Clear the SHF_COMPRESSED bit.  */
      elf_section_flags (sec) &= ~SHF_COMPRESSED;
      /* Fall through.  */

    default:
      /* Write the zlib header.  It should be "ZLIB" followed by
	 the uncompressed section size, 8 bytes in big-endian
	 order.  */
      memcpy (contents, "ZLIB", 4);
      bfd_putb64 (sec->size, contents + 4);
      /* No way to keep the original alignment, just use 1 always. */
      bfd_set_section_alignment (sec, 0);
      break;
    }
}

/* Check the compression header at CONTENTS of SEC in ABFD and store the
   ch_type in CH_TYPE, uncompressed size in UNCOMPRESSED_SIZE, and the
   uncompressed data alignment in UNCOMPRESSED_ALIGNMENT_POWER if the
   compression header is valid.  */

static bool
bfd_check_compression_header (bfd *abfd, bfd_byte *contents,
			      asection *sec,
			      enum compression_type *ch_type,
			      bfd_size_type *uncompressed_size,
			      unsigned int *uncompressed_alignment_power)
{
  if (bfd_get_flavour (abfd) == bfd_target_elf_flavour
      && (elf_section_flags (sec) & SHF_COMPRESSED) != 0)
    {
      Elf_Internal_Chdr chdr;
      const struct elf_backend_data *bed = get_elf_backend_data (abfd);
      if (bed->s->elfclass == ELFCLASS32)
	{
	  Elf32_External_Chdr *echdr = (Elf32_External_Chdr *) contents;
	  chdr.ch_type = bfd_get_32 (abfd, &echdr->ch_type);
	  chdr.ch_size = bfd_get_32 (abfd, &echdr->ch_size);
	  chdr.ch_addralign = bfd_get_32 (abfd, &echdr->ch_addralign);
	}
      else
	{
	  Elf64_External_Chdr *echdr = (Elf64_External_Chdr *) contents;
	  chdr.ch_type = bfd_get_32 (abfd, &echdr->ch_type);
	  chdr.ch_size = bfd_get_64 (abfd, &echdr->ch_size);
	  chdr.ch_addralign = bfd_get_64 (abfd, &echdr->ch_addralign);
	}
      *ch_type = chdr.ch_type;
      if ((chdr.ch_type == ch_compress_zlib
	   || chdr.ch_type == ch_compress_zstd)
	  && chdr.ch_addralign == (chdr.ch_addralign & -chdr.ch_addralign))
	{
	  *uncompressed_size = chdr.ch_size;
	  *uncompressed_alignment_power = bfd_log2 (chdr.ch_addralign);
	  return true;
	}
    }

  return false;
}

/*
FUNCTION
	bfd_get_compression_header_size

SYNOPSIS
	int bfd_get_compression_header_size (bfd *abfd, asection *sec);

DESCRIPTION
	Return the size of the compression header of SEC in ABFD.
*/

int
bfd_get_compression_header_size (bfd *abfd, asection *sec)
{
  if (bfd_get_flavour (abfd) == bfd_target_elf_flavour)
    {
      if (sec == NULL)
	{
	  if (!(abfd->flags & BFD_COMPRESS_GABI))
	    return 0;
	}
      else if (!(elf_section_flags (sec) & SHF_COMPRESSED))
	return 0;

      if (get_elf_backend_data (abfd)->s->elfclass == ELFCLASS32)
	return sizeof (Elf32_External_Chdr);
      else
	return sizeof (Elf64_External_Chdr);
    }

  return 0;
}

/*
FUNCTION
	bfd_convert_section_setup

SYNOPSIS
	bool bfd_convert_section_setup
	  (bfd *ibfd, asection *isec, bfd *obfd,
	   const char **new_name, bfd_size_type *new_size);

DESCRIPTION
	Do early setup for objcopy, when copying @var{isec} in input
	BFD @var{ibfd} to output BFD @var{obfd}.  Returns the name and
	size of the output section.
*/

bool
bfd_convert_section_setup (bfd *ibfd, asection *isec, bfd *obfd,
			   const char **new_name, bfd_size_type *new_size)
{
  bfd_size_type hdr_size;

  if ((isec->flags & SEC_DEBUGGING) != 0
      && (isec->flags & SEC_HAS_CONTENTS) != 0)
    {
      const char *name = *new_name;

      if ((obfd->flags & (BFD_DECOMPRESS | BFD_COMPRESS_GABI)) != 0)
	{
	  /* When we decompress or compress with SHF_COMPRESSED,
	     convert section name from .zdebug_* to .debug_*.  */
	  if (startswith (name, ".zdebug_"))
	    {
	      name = bfd_zdebug_name_to_debug (obfd, name);
	      if (name == NULL)
		return false;
	    }
	}

      /* PR binutils/18087: Compression does not always make a
	 section smaller.  So only rename the section when
	 compression has actually taken place.  If input section
	 name is .zdebug_*, we should never compress it again.  */
      else if (isec->compress_status == COMPRESS_SECTION_DONE
	       && startswith (name, ".debug_"))
	{
	  name = bfd_debug_name_to_zdebug (obfd, name);
	  if (name == NULL)
	    return false;
	}
      *new_name = name;
    }
  *new_size = bfd_section_size (isec);

  /* Do nothing if either input or output aren't ELF.  */
  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  /* Do nothing if ELF classes of input and output are the same. */
  if (get_elf_backend_data (ibfd)->s->elfclass
      == get_elf_backend_data (obfd)->s->elfclass)
    return true;

  /* Convert GNU property size.  */
  if (startswith (isec->name, NOTE_GNU_PROPERTY_SECTION_NAME))
    {
      *new_size = _bfd_elf_convert_gnu_property_size (ibfd, obfd);
      return true;
    }

  /* Do nothing if input file will be decompressed.  */
  if ((ibfd->flags & BFD_DECOMPRESS))
    return true;

  /* Do nothing if the input section isn't a SHF_COMPRESSED section. */
  hdr_size = bfd_get_compression_header_size (ibfd, isec);
  if (hdr_size == 0)
    return true;

  /* Adjust the size of the output SHF_COMPRESSED section.  */
  if (hdr_size == sizeof (Elf32_External_Chdr))
    *new_size += sizeof (Elf64_External_Chdr) - sizeof (Elf32_External_Chdr);
  else
    *new_size -= sizeof (Elf64_External_Chdr) - sizeof (Elf32_External_Chdr);
  return true;
}

/*
FUNCTION
	bfd_convert_section_contents

SYNOPSIS
	bool bfd_convert_section_contents
	  (bfd *ibfd, asection *isec, bfd *obfd,
	   bfd_byte **ptr, bfd_size_type *ptr_size);

DESCRIPTION
	Convert the contents, stored in @var{*ptr}, of the section
	@var{isec} in input BFD @var{ibfd} to output BFD @var{obfd}
	if needed.  The original buffer pointed to by @var{*ptr} may
	be freed and @var{*ptr} is returned with memory malloc'd by this
	function, and the new size written to @var{ptr_size}.
*/

bool
bfd_convert_section_contents (bfd *ibfd, sec_ptr isec, bfd *obfd,
			      bfd_byte **ptr, bfd_size_type *ptr_size)
{
  bfd_byte *contents;
  bfd_size_type ihdr_size, ohdr_size, size;
  Elf_Internal_Chdr chdr;
  bool use_memmove;

  /* Do nothing if either input or output aren't ELF.  */
  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  /* Do nothing if ELF classes of input and output are the same.  */
  if (get_elf_backend_data (ibfd)->s->elfclass
      == get_elf_backend_data (obfd)->s->elfclass)
    return true;

  /* Convert GNU properties.  */
  if (startswith (isec->name, NOTE_GNU_PROPERTY_SECTION_NAME))
    return _bfd_elf_convert_gnu_properties (ibfd, isec, obfd, ptr,
					    ptr_size);

  /* Do nothing if input file will be decompressed.  */
  if ((ibfd->flags & BFD_DECOMPRESS))
    return true;

  /* Do nothing if the input section isn't a SHF_COMPRESSED section.  */
  ihdr_size = bfd_get_compression_header_size (ibfd, isec);
  if (ihdr_size == 0)
    return true;

  /* PR 25221.  Check for corrupt input sections.  */
  if (ihdr_size > bfd_get_section_limit (ibfd, isec))
    /* FIXME: Issue a warning about a corrupt
       compression header size field ?  */
    return false;

  contents = *ptr;

  /* Convert the contents of the input SHF_COMPRESSED section to
     output.  Get the input compression header and the size of the
     output compression header.  */
  if (ihdr_size == sizeof (Elf32_External_Chdr))
    {
      Elf32_External_Chdr *echdr = (Elf32_External_Chdr *) contents;
      chdr.ch_type = bfd_get_32 (ibfd, &echdr->ch_type);
      chdr.ch_size = bfd_get_32 (ibfd, &echdr->ch_size);
      chdr.ch_addralign = bfd_get_32 (ibfd, &echdr->ch_addralign);

      ohdr_size = sizeof (Elf64_External_Chdr);

      use_memmove = false;
    }
  else if (ihdr_size != sizeof (Elf64_External_Chdr))
    {
      /* FIXME: Issue a warning about a corrupt
	 compression header size field ?  */
      return false;
    }
  else
    {
      Elf64_External_Chdr *echdr = (Elf64_External_Chdr *) contents;
      chdr.ch_type = bfd_get_32 (ibfd, &echdr->ch_type);
      chdr.ch_size = bfd_get_64 (ibfd, &echdr->ch_size);
      chdr.ch_addralign = bfd_get_64 (ibfd, &echdr->ch_addralign);

      ohdr_size = sizeof (Elf32_External_Chdr);
      use_memmove = true;
    }

  size = bfd_section_size (isec) - ihdr_size + ohdr_size;
  if (!use_memmove)
    {
      contents = (bfd_byte *) bfd_malloc (size);
      if (contents == NULL)
	return false;
    }

  /* Write out the output compression header.  */
  if (ohdr_size == sizeof (Elf32_External_Chdr))
    {
      Elf32_External_Chdr *echdr = (Elf32_External_Chdr *) contents;
      bfd_put_32 (obfd, chdr.ch_type, &echdr->ch_type);
      bfd_put_32 (obfd, chdr.ch_size, &echdr->ch_size);
      bfd_put_32 (obfd, chdr.ch_addralign, &echdr->ch_addralign);
    }
  else
    {
      Elf64_External_Chdr *echdr = (Elf64_External_Chdr *) contents;
      bfd_put_32 (obfd, chdr.ch_type, &echdr->ch_type);
      bfd_put_32 (obfd, 0, &echdr->ch_reserved);
      bfd_put_64 (obfd, chdr.ch_size, &echdr->ch_size);
      bfd_put_64 (obfd, chdr.ch_addralign, &echdr->ch_addralign);
    }

  /* Copy the compressed contents.  */
  if (use_memmove)
    memmove (contents + ohdr_size, *ptr + ihdr_size, size - ohdr_size);
  else
    {
      memcpy (contents + ohdr_size, *ptr + ihdr_size, size - ohdr_size);
      free (*ptr);
      *ptr = contents;
    }

  *ptr_size = size;
  return true;
}

static bool
decompress_contents (bool is_zstd, bfd_byte *compressed_buffer,
		     bfd_size_type compressed_size,
		     bfd_byte *uncompressed_buffer,
		     bfd_size_type uncompressed_size)
{
  if (is_zstd)
    {
#ifdef HAVE_ZSTD
      size_t ret = ZSTD_decompress (uncompressed_buffer, uncompressed_size,
				    compressed_buffer, compressed_size);
      return !ZSTD_isError (ret);
#endif
    }

  z_stream strm;
  int rc;

  /* It is possible the section consists of several compressed
     buffers concatenated together, so we uncompress in a loop.  */
  /* PR 18313: The state field in the z_stream structure is supposed
     to be invisible to the user (ie us), but some compilers will
     still complain about it being used without initialisation.  So
     we first zero the entire z_stream structure and then set the fields
     that we need.  */
  memset (& strm, 0, sizeof strm);
  strm.avail_in = compressed_size;
  strm.next_in = (Bytef*) compressed_buffer;
  strm.avail_out = uncompressed_size;
  /* FIXME: strm.avail_in and strm.avail_out are typically unsigned
     int.  Supporting sizes that don't fit in an unsigned int is
     possible but will require some rewriting of this function.  */
  if (strm.avail_in != compressed_size || strm.avail_out != uncompressed_size)
    return false;

  BFD_ASSERT (Z_OK == 0);
  rc = inflateInit (&strm);
  while (strm.avail_in > 0 && strm.avail_out > 0)
    {
      if (rc != Z_OK)
	break;
      strm.next_out = ((Bytef*) uncompressed_buffer
		       + (uncompressed_size - strm.avail_out));
      rc = inflate (&strm, Z_FINISH);
      if (rc != Z_STREAM_END)
	break;
      rc = inflateReset (&strm);
    }
  return inflateEnd (&strm) == Z_OK && rc == Z_OK && strm.avail_out == 0;
}

/* Compress section contents using zlib/zstd and store
   as the contents field.  This function assumes the contents
   field was allocated using bfd_malloc() or equivalent.

   Return the uncompressed size if the full section contents is
   compressed successfully.  Otherwise return 0.  */

static bfd_size_type
bfd_compress_section_contents (bfd *abfd, sec_ptr sec)
{
  bfd_byte *input_buffer;
  uLong compressed_size;
  bfd_byte *buffer;
  bfd_size_type buffer_size;
  int zlib_size = 0;
  int orig_header_size;
  bfd_size_type uncompressed_size;
  unsigned int uncompressed_alignment_pow;
  enum compression_type ch_type = ch_none;
  int new_header_size = bfd_get_compression_header_size (abfd, NULL);
  bool compressed
    = bfd_is_section_compressed_info (abfd, sec,
				      &orig_header_size,
				      &uncompressed_size,
				      &uncompressed_alignment_pow,
				      &ch_type);
  bool update = false;

  /* We shouldn't be trying to decompress unsupported compressed sections.  */
  if (compressed && orig_header_size < 0)
    abort ();

  /* Either ELF compression header or the 12-byte, "ZLIB" + 8-byte size,
     overhead in .zdebug* section.  */
  if (!new_header_size)
    new_header_size = 12;
  if (ch_type == ch_none)
    orig_header_size = 12;

  input_buffer = sec->contents;
  if (compressed)
    {
      zlib_size = sec->size - orig_header_size;
      compressed_size = zlib_size + new_header_size;

      /* If we are converting between zlib-gnu and zlib-gabi then the
	 compressed contents just need to be moved.  */
      update = (ch_type < ch_compress_zstd
		&& (abfd->flags & BFD_COMPRESS_ZSTD) == 0);

      /* Uncompress when not just moving contents or when compressed
	 is not smaller than uncompressed.  */
      if (!update || compressed_size >= uncompressed_size)
	{
	  buffer_size = uncompressed_size;
	  buffer = bfd_malloc (buffer_size);
	  if (buffer == NULL)
	    return 0;

	  if (!decompress_contents (ch_type == ch_compress_zstd,
				    input_buffer + orig_header_size,
				    zlib_size, buffer, buffer_size))
	    {
	      bfd_set_error (bfd_error_bad_value);
	      free (buffer);
	      return 0;
	    }
	  free (input_buffer);
	  bfd_set_section_alignment (sec, uncompressed_alignment_pow);
	  sec->contents = buffer;
	  sec->flags |= SEC_IN_MEMORY;
	  sec->compress_status = COMPRESS_SECTION_NONE;
	  sec->size = uncompressed_size;
	  input_buffer = buffer;
	}
    }

  if (!update)
    compressed_size = compressBound (uncompressed_size) + new_header_size;

  buffer_size = compressed_size;
  buffer = bfd_alloc (abfd, buffer_size);
  if (buffer == NULL)
    return 0;

  if (update)
    {
      if (compressed_size < uncompressed_size)
	memcpy (buffer + new_header_size,
		input_buffer + orig_header_size,
		zlib_size);
    }
  else
    {
      if (abfd->flags & BFD_COMPRESS_ZSTD)
	{
#if HAVE_ZSTD
	  compressed_size = ZSTD_compress (buffer + new_header_size,
					   compressed_size,
					   input_buffer,
					   uncompressed_size,
					   ZSTD_CLEVEL_DEFAULT);
	  if (ZSTD_isError (compressed_size))
	    {
	      bfd_release (abfd, buffer);
	      bfd_set_error (bfd_error_bad_value);
	      return 0;
	    }
#endif
	}
      else if (compress ((Bytef *) buffer + new_header_size, &compressed_size,
			 (const Bytef *) input_buffer, uncompressed_size)
	       != Z_OK)
	{
	  bfd_release (abfd, buffer);
	  bfd_set_error (bfd_error_bad_value);
	  return 0;
	}

      compressed_size += new_header_size;
    }

  /* If compression didn't make the section smaller, keep it uncompressed.  */
  if (compressed_size >= uncompressed_size)
    {
      memcpy (buffer, input_buffer, uncompressed_size);
      if (bfd_get_flavour (abfd) == bfd_target_elf_flavour)
	elf_section_flags (sec) &= ~SHF_COMPRESSED;
      sec->compress_status = COMPRESS_SECTION_NONE;
    }
  else
    {
      sec->size = uncompressed_size;
      bfd_update_compression_header (abfd, buffer, sec);
      sec->size = compressed_size;
      sec->compress_status = COMPRESS_SECTION_DONE;
    }
  sec->contents = buffer;
  sec->flags |= SEC_IN_MEMORY;
  free (input_buffer);
  return uncompressed_size;
}

/*
FUNCTION
	bfd_get_full_section_contents

SYNOPSIS
	bool bfd_get_full_section_contents
	  (bfd *abfd, asection *section, bfd_byte **ptr);

DESCRIPTION
	Read all data from @var{section} in BFD @var{abfd}, decompress
	if needed, and store in @var{*ptr}.  If @var{*ptr} is NULL,
	return @var{*ptr} with memory malloc'd by this function.

	Return @code{TRUE} if the full section contents is retrieved
	successfully.  If the section has no contents then this function
	returns @code{TRUE} but @var{*ptr} is set to NULL.
*/

bool
bfd_get_full_section_contents (bfd *abfd, sec_ptr sec, bfd_byte **ptr)
{
  bfd_size_type readsz = bfd_get_section_limit_octets (abfd, sec);
  bfd_size_type allocsz = bfd_get_section_alloc_size (abfd, sec);
  bfd_byte *p = *ptr;
  bool ret;
  bfd_size_type save_size;
  bfd_size_type save_rawsize;
  bfd_byte *compressed_buffer;
  unsigned int compression_header_size;
  const unsigned int compress_status = sec->compress_status;

  if (allocsz == 0)
    {
      *ptr = NULL;
      return true;
    }

  if (p == NULL
      && compress_status != COMPRESS_SECTION_DONE
      && _bfd_section_size_insane (abfd, sec))
    {
      /* PR 24708: Avoid attempts to allocate a ridiculous amount
	 of memory.  */
      _bfd_error_handler
	/* xgettext:c-format */
	(_("error: %pB(%pA) is too large (%#" PRIx64 " bytes)"),
	 abfd, sec, (uint64_t) readsz);
      return false;
    }

  switch (compress_status)
    {
    case COMPRESS_SECTION_NONE:
      if (p == NULL)
	{
	  p = (bfd_byte *) bfd_malloc (allocsz);
	  if (p == NULL)
	    {
	      /* PR 20801: Provide a more helpful error message.  */
	      if (bfd_get_error () == bfd_error_no_memory)
		_bfd_error_handler
		  /* xgettext:c-format */
		  (_("error: %pB(%pA) is too large (%#" PRIx64 " bytes)"),
		  abfd, sec, (uint64_t) allocsz);
	      return false;
	    }
	}

      if (!bfd_get_section_contents (abfd, sec, p, 0, readsz))
	{
	  if (*ptr != p)
	    free (p);
	  return false;
	}
      *ptr = p;
      return true;

    case DECOMPRESS_SECTION_ZLIB:
    case DECOMPRESS_SECTION_ZSTD:
      /* Read in the full compressed section contents.  */
      compressed_buffer = (bfd_byte *) bfd_malloc (sec->compressed_size);
      if (compressed_buffer == NULL)
	return false;
      save_rawsize = sec->rawsize;
      save_size = sec->size;
      /* Clear rawsize, set size to compressed size and set compress_status
	 to COMPRESS_SECTION_NONE.  If the compressed size is bigger than
	 the uncompressed size, bfd_get_section_contents will fail.  */
      sec->rawsize = 0;
      sec->size = sec->compressed_size;
      sec->compress_status = COMPRESS_SECTION_NONE;
      ret = bfd_get_section_contents (abfd, sec, compressed_buffer,
				      0, sec->compressed_size);
      /* Restore rawsize and size.  */
      sec->rawsize = save_rawsize;
      sec->size = save_size;
      sec->compress_status = compress_status;
      if (!ret)
	goto fail_compressed;

      if (p == NULL)
	p = (bfd_byte *) bfd_malloc (allocsz);
      if (p == NULL)
	goto fail_compressed;

      compression_header_size = bfd_get_compression_header_size (abfd, sec);
      if (compression_header_size == 0)
	/* Set header size to the zlib header size if it is a
	   SHF_COMPRESSED section.  */
	compression_header_size = 12;
      bool is_zstd = compress_status == DECOMPRESS_SECTION_ZSTD;
      if (!decompress_contents (
	      is_zstd, compressed_buffer + compression_header_size,
	      sec->compressed_size - compression_header_size, p, readsz))
	{
	  bfd_set_error (bfd_error_bad_value);
	  if (p != *ptr)
	    free (p);
	fail_compressed:
	  free (compressed_buffer);
	  return false;
	}

      free (compressed_buffer);
      *ptr = p;
      return true;

    case COMPRESS_SECTION_DONE:
      if (sec->contents == NULL)
	return false;
      if (p == NULL)
	{
	  p = (bfd_byte *) bfd_malloc (allocsz);
	  if (p == NULL)
	    return false;
	  *ptr = p;
	}
      /* PR 17512; file: 5bc29788.  */
      if (p != sec->contents)
	memcpy (p, sec->contents, readsz);
      return true;

    default:
      abort ();
    }
}

/*
FUNCTION
	bfd_is_section_compressed_info

SYNOPSIS
	bool bfd_is_section_compressed_info
	  (bfd *abfd, asection *section,
	   int *compression_header_size_p,
	   bfd_size_type *uncompressed_size_p,
	   unsigned int *uncompressed_alignment_power_p,
	   enum compression_type *ch_type);

DESCRIPTION
	Return @code{TRUE} if @var{section} is compressed.  Compression
	header size is returned in @var{compression_header_size_p},
	uncompressed size is returned in @var{uncompressed_size_p}
	and the uncompressed data alignement power is returned in
	@var{uncompressed_align_pow_p}.  If compression is
	unsupported, compression header size is returned with -1
	and uncompressed size is returned with 0.
*/

bool
bfd_is_section_compressed_info (bfd *abfd, sec_ptr sec,
				int *compression_header_size_p,
				bfd_size_type *uncompressed_size_p,
				unsigned int *uncompressed_align_pow_p,
				enum compression_type *ch_type)
{
  bfd_byte header[MAX_COMPRESSION_HEADER_SIZE];
  int compression_header_size;
  int header_size;
  unsigned int saved = sec->compress_status;
  bool compressed;

  *uncompressed_align_pow_p = 0;

  compression_header_size = bfd_get_compression_header_size (abfd, sec);
  if (compression_header_size > MAX_COMPRESSION_HEADER_SIZE)
    abort ();
  header_size = compression_header_size ? compression_header_size : 12;

  /* Don't decompress the section.  */
  sec->compress_status = COMPRESS_SECTION_NONE;

  /* Read the header.  */
  if (bfd_get_section_contents (abfd, sec, header, 0, header_size))
    {
      if (compression_header_size == 0)
	/* In this case, it should be "ZLIB" followed by the uncompressed
	   section size, 8 bytes in big-endian order.  */
	compressed = startswith ((char*) header , "ZLIB");
      else
	compressed = true;
    }
  else
    compressed = false;

  *uncompressed_size_p = sec->size;
  if (compressed)
    {
      if (compression_header_size != 0)
	{
	  if (!bfd_check_compression_header (abfd, header, sec, ch_type,
					     uncompressed_size_p,
					     uncompressed_align_pow_p))
	    compression_header_size = -1;
	}
      /* Check for the pathalogical case of a debug string section that
	 contains the string ZLIB.... as the first entry.  We assume that
	 no uncompressed .debug_str section would ever be big enough to
	 have the first byte of its (big-endian) size be non-zero.  */
      else if (strcmp (sec->name, ".debug_str") == 0
	       && ISPRINT (header[4]))
	compressed = false;
      else
	*uncompressed_size_p = bfd_getb64 (header + 4);
    }

  /* Restore compress_status.  */
  sec->compress_status = saved;
  *compression_header_size_p = compression_header_size;
  return compressed;
}

/*
FUNCTION
	bfd_is_section_compressed

SYNOPSIS
	bool bfd_is_section_compressed
	  (bfd *abfd, asection *section);

DESCRIPTION
	Return @code{TRUE} if @var{section} is compressed.
*/

bool
bfd_is_section_compressed (bfd *abfd, sec_ptr sec)
{
  int compression_header_size;
  bfd_size_type uncompressed_size;
  unsigned int uncompressed_align_power;
  enum compression_type ch_type;
  return (bfd_is_section_compressed_info (abfd, sec,
					  &compression_header_size,
					  &uncompressed_size,
					  &uncompressed_align_power,
					  &ch_type)
	  && compression_header_size >= 0
	  && uncompressed_size > 0);
}

/*
FUNCTION
	bfd_init_section_decompress_status

SYNOPSIS
	bool bfd_init_section_decompress_status
	  (bfd *abfd, asection *section);

DESCRIPTION
	Record compressed section size, update section size with
	decompressed size and set compress_status to
	DECOMPRESS_SECTION_{ZLIB,ZSTD}.

	Return @code{FALSE} if the section is not a valid compressed
	section.  Otherwise, return @code{TRUE}.
*/

bool
bfd_init_section_decompress_status (bfd *abfd, sec_ptr sec)
{
  bfd_byte header[MAX_COMPRESSION_HEADER_SIZE];
  int compression_header_size;
  int header_size;
  bfd_size_type uncompressed_size;
  unsigned int uncompressed_alignment_power = 0;
  enum compression_type ch_type;
  z_stream strm;

  compression_header_size = bfd_get_compression_header_size (abfd, sec);
  if (compression_header_size > MAX_COMPRESSION_HEADER_SIZE)
    abort ();
  header_size = compression_header_size ? compression_header_size : 12;

  /* Read the header.  */
  if (sec->rawsize != 0
      || sec->contents != NULL
      || sec->compress_status != COMPRESS_SECTION_NONE
      || !bfd_get_section_contents (abfd, sec, header, 0, header_size))
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  if (compression_header_size == 0)
    {
      /* In this case, it should be "ZLIB" followed by the uncompressed
	 section size, 8 bytes in big-endian order.  */
      if (! startswith ((char*) header, "ZLIB"))
	{
	  bfd_set_error (bfd_error_wrong_format);
	  return false;
	}
      uncompressed_size = bfd_getb64 (header + 4);
      ch_type = ch_none;
    }
  else if (!bfd_check_compression_header (abfd, header, sec,
					  &ch_type,
					  &uncompressed_size,
					  &uncompressed_alignment_power))
    {
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }

  /* PR28530, reject sizes unsupported by decompress_contents.  */
  strm.avail_in = sec->size;
  strm.avail_out = uncompressed_size;
  if (strm.avail_in != sec->size || strm.avail_out != uncompressed_size)
    {
      bfd_set_error (bfd_error_nonrepresentable_section);
      return false;
    }

  sec->compressed_size = sec->size;
  sec->size = uncompressed_size;
  bfd_set_section_alignment (sec, uncompressed_alignment_power);
  sec->compress_status = (ch_type == ch_compress_zstd
			  ? DECOMPRESS_SECTION_ZSTD : DECOMPRESS_SECTION_ZLIB);

  return true;
}

/*
FUNCTION
	bfd_init_section_compress_status

SYNOPSIS
	bool bfd_init_section_compress_status
	  (bfd *abfd, asection *section);

DESCRIPTION
	If open for read, compress section, update section size with
	compressed size and set compress_status to COMPRESS_SECTION_DONE.

	Return @code{FALSE} if the section is not a valid compressed
	section.  Otherwise, return @code{TRUE}.
*/

bool
bfd_init_section_compress_status (bfd *abfd, sec_ptr sec)
{
  bfd_size_type uncompressed_size;
  bfd_byte *uncompressed_buffer;

  /* Error if not opened for read.  */
  if (abfd->direction != read_direction
      || sec->size == 0
      || sec->rawsize != 0
      || sec->contents != NULL
      || sec->compress_status != COMPRESS_SECTION_NONE
      || _bfd_section_size_insane (abfd, sec))
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  /* Read in the full section contents and compress it.  */
  uncompressed_size = sec->size;
  uncompressed_buffer = (bfd_byte *) bfd_malloc (uncompressed_size);
  /* PR 21431 */
  if (uncompressed_buffer == NULL)
    return false;

  if (!bfd_get_section_contents (abfd, sec, uncompressed_buffer,
				 0, uncompressed_size))
    {
      free (uncompressed_buffer);
      return false;
    }

  sec->contents = uncompressed_buffer;
  if (bfd_compress_section_contents (abfd, sec) == 0)
    {
      free (sec->contents);
      sec->contents = NULL;
      return false;
    }
  return true;
}

/*
FUNCTION
	bfd_compress_section

SYNOPSIS
	bool bfd_compress_section
	  (bfd *abfd, asection *section, bfd_byte *uncompressed_buffer);

DESCRIPTION
	If open for write, compress section, update section size with
	compressed size and set compress_status to COMPRESS_SECTION_DONE.

	Return @code{FALSE} if compression fail.  Otherwise, return
	@code{TRUE}.  UNCOMPRESSED_BUFFER is freed in both cases.
*/

bool
bfd_compress_section (bfd *abfd, sec_ptr sec, bfd_byte *uncompressed_buffer)
{
  bfd_size_type uncompressed_size = sec->size;

  /* Error if not opened for write.  */
  if (abfd->direction != write_direction
      || uncompressed_size == 0
      || uncompressed_buffer == NULL
      || sec->contents != NULL
      || sec->compressed_size != 0
      || sec->compress_status != COMPRESS_SECTION_NONE)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  sec->contents = uncompressed_buffer;
  if (bfd_compress_section_contents (abfd, sec) == 0)
    {
      free (sec->contents);
      sec->contents = NULL;
      return false;
    }
  return true;
}
