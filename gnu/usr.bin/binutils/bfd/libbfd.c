/* Assorted BFD support routines, only used internally.
   Copyright (C) 1990-2023 Free Software Foundation, Inc.
   Written by Cygnus Support.

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
#include "bfd.h"
#include "libbfd.h"
#include "objalloc.h"

#ifndef HAVE_GETPAGESIZE
#define getpagesize() 2048
#endif

/*
SECTION
	Implementation details

SUBSECTION
	Internal functions

DESCRIPTION
	These routines are used within BFD.
	They are not intended for export, but are documented here for
	completeness.
*/

bool
_bfd_bool_bfd_false (bfd *abfd ATTRIBUTE_UNUSED)
{
  return false;
}

bool
_bfd_bool_bfd_asymbol_false (bfd *abfd ATTRIBUTE_UNUSED,
			     asymbol *sym ATTRIBUTE_UNUSED)
{
  return false;
}

/* A routine which is used in target vectors for unsupported
   operations.  */

bool
_bfd_bool_bfd_false_error (bfd *ignore ATTRIBUTE_UNUSED)
{
  bfd_set_error (bfd_error_invalid_operation);
  return false;
}

bool
_bfd_bool_bfd_link_false_error (bfd *abfd,
				struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (abfd);
}

/* A routine which is used in target vectors for supported operations
   which do not actually do anything.  */

bool
_bfd_bool_bfd_true (bfd *ignore ATTRIBUTE_UNUSED)
{
  return true;
}

bool
_bfd_bool_bfd_link_true (bfd *abfd ATTRIBUTE_UNUSED,
			 struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return true;
}

bool
_bfd_bool_bfd_bfd_true (bfd *ibfd ATTRIBUTE_UNUSED,
			bfd *obfd ATTRIBUTE_UNUSED)
{
  return true;
}

bool
_bfd_bool_bfd_uint_true (bfd *abfd ATTRIBUTE_UNUSED,
			 unsigned int flags ATTRIBUTE_UNUSED)
{
  return true;
}

bool
_bfd_bool_bfd_asection_bfd_asection_true (bfd *ibfd ATTRIBUTE_UNUSED,
					  asection *isec ATTRIBUTE_UNUSED,
					  bfd *obfd ATTRIBUTE_UNUSED,
					  asection *osec ATTRIBUTE_UNUSED)
{
  return true;
}

bool
_bfd_bool_bfd_asymbol_bfd_asymbol_true (bfd *ibfd ATTRIBUTE_UNUSED,
					asymbol *isym ATTRIBUTE_UNUSED,
					bfd *obfd ATTRIBUTE_UNUSED,
					asymbol *osym ATTRIBUTE_UNUSED)
{
  return true;
}

bool
_bfd_bool_bfd_ptr_true (bfd *abfd ATTRIBUTE_UNUSED,
			void *ptr ATTRIBUTE_UNUSED)
{
  return true;
}

/* A routine which is used in target vectors for unsupported
   operations which return a pointer value.  */

void *
_bfd_ptr_bfd_null_error (bfd *ignore ATTRIBUTE_UNUSED)
{
  bfd_set_error (bfd_error_invalid_operation);
  return NULL;
}

int
_bfd_int_bfd_0 (bfd *ignore ATTRIBUTE_UNUSED)
{
  return 0;
}

unsigned int
_bfd_uint_bfd_0 (bfd *ignore ATTRIBUTE_UNUSED)
{
   return 0;
}

long
_bfd_long_bfd_0 (bfd *ignore ATTRIBUTE_UNUSED)
{
  return 0;
}

/* A routine which is used in target vectors for unsupported
   operations which return -1 on error.  */

long
_bfd_long_bfd_n1_error (bfd *ignore_abfd ATTRIBUTE_UNUSED)
{
  bfd_set_error (bfd_error_invalid_operation);
  return -1;
}

void
_bfd_void_bfd (bfd *ignore ATTRIBUTE_UNUSED)
{
}

void
_bfd_void_bfd_link (bfd *abfd ATTRIBUTE_UNUSED,
		    struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
}

void
_bfd_void_bfd_asection (bfd *abfd ATTRIBUTE_UNUSED,
			asection *sec ATTRIBUTE_UNUSED)
{
}

long
_bfd_norelocs_get_reloc_upper_bound (bfd *abfd ATTRIBUTE_UNUSED,
				     asection *sec ATTRIBUTE_UNUSED)
{
  return sizeof (arelent *);
}

long
_bfd_norelocs_canonicalize_reloc (bfd *abfd ATTRIBUTE_UNUSED,
				  asection *sec ATTRIBUTE_UNUSED,
				  arelent **relptr,
				  asymbol **symbols ATTRIBUTE_UNUSED)
{
  *relptr = NULL;
  return 0;
}

void
_bfd_norelocs_set_reloc (bfd *abfd ATTRIBUTE_UNUSED,
			 asection *sec ATTRIBUTE_UNUSED,
			 arelent **relptr ATTRIBUTE_UNUSED,
			 unsigned int count ATTRIBUTE_UNUSED)
{
  /* Do nothing.  */
}

bool
_bfd_nocore_core_file_matches_executable_p
  (bfd *ignore_core_bfd ATTRIBUTE_UNUSED,
   bfd *ignore_exec_bfd ATTRIBUTE_UNUSED)
{
  bfd_set_error (bfd_error_invalid_operation);
  return false;
}

/* Routine to handle core_file_failing_command entry point for targets
   without core file support.  */

char *
_bfd_nocore_core_file_failing_command (bfd *ignore_abfd ATTRIBUTE_UNUSED)
{
  bfd_set_error (bfd_error_invalid_operation);
  return NULL;
}

/* Routine to handle core_file_failing_signal entry point for targets
   without core file support.  */

int
_bfd_nocore_core_file_failing_signal (bfd *ignore_abfd ATTRIBUTE_UNUSED)
{
  bfd_set_error (bfd_error_invalid_operation);
  return 0;
}

/* Routine to handle the core_file_pid entry point for targets without
   core file support.  */

int
_bfd_nocore_core_file_pid (bfd *ignore_abfd ATTRIBUTE_UNUSED)
{
  bfd_set_error (bfd_error_invalid_operation);
  return 0;
}

bfd_cleanup
_bfd_dummy_target (bfd *ignore_abfd ATTRIBUTE_UNUSED)
{
  bfd_set_error (bfd_error_wrong_format);
  return 0;
}

/* Allocate memory using malloc.  */

#ifndef SSIZE_MAX
#define SSIZE_MAX ((size_t) -1 >> 1)
#endif

/*
INTERNAL_FUNCTION
	bfd_malloc

SYNOPSIS
	void *bfd_malloc (bfd_size_type {*size*});

DESCRIPTION
	Returns a pointer to an allocated block of memory that is at least
	SIZE bytes long.  If SIZE is 0 then it will be treated as if it were
	1.  If SIZE is too big then NULL will be returned.
	
	Returns NULL upon error and sets bfd_error.
*/
void *
bfd_malloc (bfd_size_type size)
{
  void *ptr;
  size_t sz = (size_t) size;

  if (size != sz
      /* This is to pacify memory checkers like valgrind.  */
      || sz > SSIZE_MAX)
    {
      bfd_set_error (bfd_error_no_memory);
      return NULL;
    }

  ptr = malloc (sz ? sz : 1);
  if (ptr == NULL)
    bfd_set_error (bfd_error_no_memory);

  return ptr;
}

/*
INTERNAL_FUNCTION
	bfd_realloc

SYNOPSIS
	void *bfd_realloc (void *{*mem*}, bfd_size_type {*size*});

DESCRIPTION
	Returns a pointer to an allocated block of memory that is at least
	SIZE bytes long.  If SIZE is 0 then it will be treated as if it were
	1.  If SIZE is too big then NULL will be returned.
	
	If MEM is not NULL then it must point to an allocated block of memory.
	If this block is large enough then MEM may be used as the return
	value for this function, but this is not guaranteed.

	If MEM is not returned then the first N bytes in the returned block
	will be identical to the first N bytes in region pointed to by MEM,
	where N is the lessor of SIZE and the length of the region of memory
	currently addressed by MEM.

	Returns NULL upon error and sets bfd_error.
*/
void *
bfd_realloc (void *ptr, bfd_size_type size)
{
  void *ret;
  size_t sz = (size_t) size;

  if (ptr == NULL)
    return bfd_malloc (size);

  if (size != sz
      /* This is to pacify memory checkers like valgrind.  */
      || sz > SSIZE_MAX)
    {
      bfd_set_error (bfd_error_no_memory);
      return NULL;
    }

  /* The behaviour of realloc(0) is implementation defined,
     but for this function we always allocate memory.  */
  ret = realloc (ptr, sz ? sz : 1);

  if (ret == NULL)
    bfd_set_error (bfd_error_no_memory);

  return ret;
}

/*
INTERNAL_FUNCTION
	bfd_realloc_or_free

SYNOPSIS
	void *bfd_realloc_or_free (void *{*mem*}, bfd_size_type {*size*});

DESCRIPTION
	Returns a pointer to an allocated block of memory that is at least
	SIZE bytes long.  If SIZE is 0 then no memory will be allocated,
	MEM will be freed, and NULL will be returned.  This will not cause
	bfd_error to be set.

	If SIZE is too big then NULL will be returned and bfd_error will be
	set. 
	
	If MEM is not NULL then it must point to an allocated block of memory.
	If this block is large enough then MEM may be used as the return
	value for this function, but this is not guaranteed.

	If MEM is not returned then the first N bytes in the returned block
	will be identical to the first N bytes in region pointed to by MEM,
	where N is the lessor of SIZE and the length of the region of memory
	currently addressed by MEM.
*/
void *
bfd_realloc_or_free (void *ptr, bfd_size_type size)
{
  void *ret;

  /* The behaviour of realloc(0) is implementation defined, but
     for this function we treat it is always freeing the memory.  */
  if (size == 0)
    {
      free (ptr);
      return NULL;
    }
      
  ret = bfd_realloc (ptr, size);
  if (ret == NULL)
    free (ptr);

  return ret;
}

/*
INTERNAL_FUNCTION
	bfd_zmalloc

SYNOPSIS
	void *bfd_zmalloc (bfd_size_type {*size*});

DESCRIPTION
	Returns a pointer to an allocated block of memory that is at least
	SIZE bytes long.  If SIZE is 0 then it will be treated as if it were
	1.  If SIZE is too big then NULL will be returned.
	
	Returns NULL upon error and sets bfd_error.

	If NULL is not returned then the allocated block of memory will
	have been cleared.
*/
void *
bfd_zmalloc (bfd_size_type size)
{
  void *ptr = bfd_malloc (size);

  if (ptr != NULL)
    memset (ptr, 0, size ? (size_t) size : 1);

  return ptr;
}

/*
FUNCTION
	bfd_alloc

SYNOPSIS
	void *bfd_alloc (bfd *abfd, bfd_size_type wanted);

DESCRIPTION
	Allocate a block of @var{wanted} bytes of memory attached to
	<<abfd>> and return a pointer to it.
*/

void *
bfd_alloc (bfd *abfd, bfd_size_type size)
{
  void *ret;
  unsigned long ul_size = (unsigned long) size;

  if (size != ul_size
      /* Note - although objalloc_alloc takes an unsigned long as its
	 argument, internally the size is treated as a signed long.  This can
	 lead to problems where, for example, a request to allocate -1 bytes
	 can result in just 1 byte being allocated, rather than
	 ((unsigned long) -1) bytes.  Also memory checkers will often
	 complain about attempts to allocate a negative amount of memory.
	 So to stop these problems we fail if the size is negative.  */
      || ((signed long) ul_size) < 0)
    {
      bfd_set_error (bfd_error_no_memory);
      return NULL;
    }

  ret = objalloc_alloc ((struct objalloc *) abfd->memory, ul_size);
  if (ret == NULL)
    bfd_set_error (bfd_error_no_memory);
  else
    abfd->alloc_size += size;
  return ret;
}

/*
FUNCTION
	bfd_zalloc

SYNOPSIS
	void *bfd_zalloc (bfd *abfd, bfd_size_type wanted);

DESCRIPTION
	Allocate a block of @var{wanted} bytes of zeroed memory
	attached to <<abfd>> and return a pointer to it.
*/

void *
bfd_zalloc (bfd *abfd, bfd_size_type size)
{
  void *res;

  res = bfd_alloc (abfd, size);
  if (res)
    memset (res, 0, (size_t) size);
  return res;
}

/*
FUNCTION
	bfd_release

SYNOPSIS
	void bfd_release (bfd *, void *);

DESCRIPTION
	Free a block allocated for a BFD.
	Note: Also frees all more recently allocated blocks!
*/

void
bfd_release (bfd *abfd, void *block)
{
  objalloc_free_block ((struct objalloc *) abfd->memory, block);
}

/*
INTERNAL_FUNCTION
	bfd_write_bigendian_4byte_int

SYNOPSIS
	bool bfd_write_bigendian_4byte_int (bfd *, unsigned int);

DESCRIPTION
	Write a 4 byte integer @var{i} to the output BFD @var{abfd}, in big
	endian order regardless of what else is going on.  This is useful in
	archives.

*/
bool
bfd_write_bigendian_4byte_int (bfd *abfd, unsigned int i)
{
  bfd_byte buffer[4];
  bfd_putb32 ((bfd_vma) i, buffer);
  return bfd_bwrite (buffer, (bfd_size_type) 4, abfd) == 4;
}


/** The do-it-yourself (byte) sex-change kit */

/* The middle letter e.g. get<b>short indicates Big or Little endian
   target machine.  It doesn't matter what the byte order of the host
   machine is; these routines work for either.  */

/* FIXME: Should these take a count argument?
   Answer (gnu@cygnus.com):  No, but perhaps they should be inline
			     functions in swap.h #ifdef __GNUC__.
			     Gprof them later and find out.  */

/*
FUNCTION
	bfd_put_size
FUNCTION
	bfd_get_size

DESCRIPTION
	These macros as used for reading and writing raw data in
	sections; each access (except for bytes) is vectored through
	the target format of the BFD and mangled accordingly. The
	mangling performs any necessary endian translations and
	removes alignment restrictions.  Note that types accepted and
	returned by these macros are identical so they can be swapped
	around in macros---for example, @file{libaout.h} defines <<GET_WORD>>
	to either <<bfd_get_32>> or <<bfd_get_64>>.

	In the put routines, @var{val} must be a <<bfd_vma>>.  If we are on a
	system without prototypes, the caller is responsible for making
	sure that is true, with a cast if necessary.  We don't cast
	them in the macro definitions because that would prevent <<lint>>
	or <<gcc -Wall>> from detecting sins such as passing a pointer.
	To detect calling these with less than a <<bfd_vma>>, use
	<<gcc -Wconversion>> on a host with 64 bit <<bfd_vma>>'s.

.
.{* Byte swapping macros for user section data.  *}
.
.#define bfd_put_8(abfd, val, ptr) \
.  ((void) (*((bfd_byte *) (ptr)) = (val) & 0xff))
.#define bfd_put_signed_8 \
.  bfd_put_8
.#define bfd_get_8(abfd, ptr) \
.  ((bfd_vma) *(const bfd_byte *) (ptr) & 0xff)
.#define bfd_get_signed_8(abfd, ptr) \
.  ((((bfd_signed_vma) *(const bfd_byte *) (ptr) & 0xff) ^ 0x80) - 0x80)
.
.#define bfd_put_16(abfd, val, ptr) \
.  BFD_SEND (abfd, bfd_putx16, ((val),(ptr)))
.#define bfd_put_signed_16 \
.  bfd_put_16
.#define bfd_get_16(abfd, ptr) \
.  BFD_SEND (abfd, bfd_getx16, (ptr))
.#define bfd_get_signed_16(abfd, ptr) \
.  BFD_SEND (abfd, bfd_getx_signed_16, (ptr))
.
.#define bfd_put_24(abfd, val, ptr) \
.  do					\
.    if (bfd_big_endian (abfd))		\
.      bfd_putb24 ((val), (ptr));	\
.    else				\
.      bfd_putl24 ((val), (ptr));	\
.  while (0)
.
.bfd_vma bfd_getb24 (const void *p);
.bfd_vma bfd_getl24 (const void *p);
.
.#define bfd_get_24(abfd, ptr) \
.  (bfd_big_endian (abfd) ? bfd_getb24 (ptr) : bfd_getl24 (ptr))
.
.#define bfd_put_32(abfd, val, ptr) \
.  BFD_SEND (abfd, bfd_putx32, ((val),(ptr)))
.#define bfd_put_signed_32 \
.  bfd_put_32
.#define bfd_get_32(abfd, ptr) \
.  BFD_SEND (abfd, bfd_getx32, (ptr))
.#define bfd_get_signed_32(abfd, ptr) \
.  BFD_SEND (abfd, bfd_getx_signed_32, (ptr))
.
.#define bfd_put_64(abfd, val, ptr) \
.  BFD_SEND (abfd, bfd_putx64, ((val), (ptr)))
.#define bfd_put_signed_64 \
.  bfd_put_64
.#define bfd_get_64(abfd, ptr) \
.  BFD_SEND (abfd, bfd_getx64, (ptr))
.#define bfd_get_signed_64(abfd, ptr) \
.  BFD_SEND (abfd, bfd_getx_signed_64, (ptr))
.
.#define bfd_get(bits, abfd, ptr)			\
.  ((bits) == 8 ? bfd_get_8 (abfd, ptr)			\
.   : (bits) == 16 ? bfd_get_16 (abfd, ptr)		\
.   : (bits) == 32 ? bfd_get_32 (abfd, ptr)		\
.   : (bits) == 64 ? bfd_get_64 (abfd, ptr)		\
.   : (abort (), (bfd_vma) - 1))
.
.#define bfd_put(bits, abfd, val, ptr)			\
.  ((bits) == 8 ? bfd_put_8  (abfd, val, ptr)		\
.   : (bits) == 16 ? bfd_put_16 (abfd, val, ptr)	\
.   : (bits) == 32 ? bfd_put_32 (abfd, val, ptr)	\
.   : (bits) == 64 ? bfd_put_64 (abfd, val, ptr)	\
.   : (abort (), (void) 0))
.
*/

/*
FUNCTION
	bfd_h_put_size
	bfd_h_get_size

DESCRIPTION
	These macros have the same function as their <<bfd_get_x>>
	brethren, except that they are used for removing information
	for the header records of object files. Believe it or not,
	some object files keep their header records in big endian
	order and their data in little endian order.
.
.{* Byte swapping macros for file header data.  *}
.
.#define bfd_h_put_8(abfd, val, ptr) \
.  bfd_put_8 (abfd, val, ptr)
.#define bfd_h_put_signed_8(abfd, val, ptr) \
.  bfd_put_8 (abfd, val, ptr)
.#define bfd_h_get_8(abfd, ptr) \
.  bfd_get_8 (abfd, ptr)
.#define bfd_h_get_signed_8(abfd, ptr) \
.  bfd_get_signed_8 (abfd, ptr)
.
.#define bfd_h_put_16(abfd, val, ptr) \
.  BFD_SEND (abfd, bfd_h_putx16, (val, ptr))
.#define bfd_h_put_signed_16 \
.  bfd_h_put_16
.#define bfd_h_get_16(abfd, ptr) \
.  BFD_SEND (abfd, bfd_h_getx16, (ptr))
.#define bfd_h_get_signed_16(abfd, ptr) \
.  BFD_SEND (abfd, bfd_h_getx_signed_16, (ptr))
.
.#define bfd_h_put_32(abfd, val, ptr) \
.  BFD_SEND (abfd, bfd_h_putx32, (val, ptr))
.#define bfd_h_put_signed_32 \
.  bfd_h_put_32
.#define bfd_h_get_32(abfd, ptr) \
.  BFD_SEND (abfd, bfd_h_getx32, (ptr))
.#define bfd_h_get_signed_32(abfd, ptr) \
.  BFD_SEND (abfd, bfd_h_getx_signed_32, (ptr))
.
.#define bfd_h_put_64(abfd, val, ptr) \
.  BFD_SEND (abfd, bfd_h_putx64, (val, ptr))
.#define bfd_h_put_signed_64 \
.  bfd_h_put_64
.#define bfd_h_get_64(abfd, ptr) \
.  BFD_SEND (abfd, bfd_h_getx64, (ptr))
.#define bfd_h_get_signed_64(abfd, ptr) \
.  BFD_SEND (abfd, bfd_h_getx_signed_64, (ptr))
.
.{* Aliases for the above, which should eventually go away.  *}
.
.#define H_PUT_64  bfd_h_put_64
.#define H_PUT_32  bfd_h_put_32
.#define H_PUT_16  bfd_h_put_16
.#define H_PUT_8   bfd_h_put_8
.#define H_PUT_S64 bfd_h_put_signed_64
.#define H_PUT_S32 bfd_h_put_signed_32
.#define H_PUT_S16 bfd_h_put_signed_16
.#define H_PUT_S8  bfd_h_put_signed_8
.#define H_GET_64  bfd_h_get_64
.#define H_GET_32  bfd_h_get_32
.#define H_GET_16  bfd_h_get_16
.#define H_GET_8   bfd_h_get_8
.#define H_GET_S64 bfd_h_get_signed_64
.#define H_GET_S32 bfd_h_get_signed_32
.#define H_GET_S16 bfd_h_get_signed_16
.#define H_GET_S8  bfd_h_get_signed_8
.
.*/

/* Sign extension to bfd_signed_vma.  */
#define COERCE16(x) (((bfd_vma) (x) ^ 0x8000) - 0x8000)
#define COERCE32(x) (((bfd_vma) (x) ^ 0x80000000) - 0x80000000)
#define COERCE64(x) \
  (((uint64_t) (x) ^ ((uint64_t) 1 << 63)) - ((uint64_t) 1 << 63))

/*
FUNCTION
	Byte swapping routines.

SYNOPSIS
	uint64_t bfd_getb64 (const void *);
	uint64_t bfd_getl64 (const void *);
	int64_t bfd_getb_signed_64 (const void *);
	int64_t bfd_getl_signed_64 (const void *);
	bfd_vma bfd_getb32 (const void *);
	bfd_vma bfd_getl32 (const void *);
	bfd_signed_vma bfd_getb_signed_32 (const void *);
	bfd_signed_vma bfd_getl_signed_32 (const void *);
	bfd_vma bfd_getb16 (const void *);
	bfd_vma bfd_getl16 (const void *);
	bfd_signed_vma bfd_getb_signed_16 (const void *);
	bfd_signed_vma bfd_getl_signed_16 (const void *);
	void bfd_putb64 (uint64_t, void *);
	void bfd_putl64 (uint64_t, void *);
	void bfd_putb32 (bfd_vma, void *);
	void bfd_putl32 (bfd_vma, void *);
	void bfd_putb24 (bfd_vma, void *);
	void bfd_putl24 (bfd_vma, void *);
	void bfd_putb16 (bfd_vma, void *);
	void bfd_putl16 (bfd_vma, void *);
	uint64_t bfd_get_bits (const void *, int, bool);
	void bfd_put_bits (uint64_t, void *, int, bool);
*/

bfd_vma
bfd_getb16 (const void *p)
{
  const bfd_byte *addr = (const bfd_byte *) p;
  return (addr[0] << 8) | addr[1];
}

bfd_vma
bfd_getl16 (const void *p)
{
  const bfd_byte *addr = (const bfd_byte *) p;
  return (addr[1] << 8) | addr[0];
}

bfd_signed_vma
bfd_getb_signed_16 (const void *p)
{
  const bfd_byte *addr = (const bfd_byte *) p;
  return COERCE16 ((addr[0] << 8) | addr[1]);
}

bfd_signed_vma
bfd_getl_signed_16 (const void *p)
{
  const bfd_byte *addr = (const bfd_byte *) p;
  return COERCE16 ((addr[1] << 8) | addr[0]);
}

void
bfd_putb16 (bfd_vma data, void *p)
{
  bfd_byte *addr = (bfd_byte *) p;
  addr[0] = (data >> 8) & 0xff;
  addr[1] = data & 0xff;
}

void
bfd_putl16 (bfd_vma data, void *p)
{
  bfd_byte *addr = (bfd_byte *) p;
  addr[0] = data & 0xff;
  addr[1] = (data >> 8) & 0xff;
}

void
bfd_putb24 (bfd_vma data, void *p)
{
  bfd_byte *addr = (bfd_byte *) p;
  addr[0] = (data >> 16) & 0xff;
  addr[1] = (data >> 8) & 0xff;
  addr[2] = data & 0xff;
}

void
bfd_putl24 (bfd_vma data, void *p)
{
  bfd_byte *addr = (bfd_byte *) p;
  addr[0] = data & 0xff;
  addr[1] = (data >> 8) & 0xff;
  addr[2] = (data >> 16) & 0xff;
}

bfd_vma
bfd_getb24 (const void *p)
{
  const bfd_byte *addr = (const bfd_byte *) p;
  uint32_t v;

  v =  (uint32_t) addr[0] << 16;
  v |= (uint32_t) addr[1] << 8;
  v |= (uint32_t) addr[2];
  return v;
}

bfd_vma
bfd_getl24 (const void *p)
{
  const bfd_byte *addr = (const bfd_byte *) p;
  uint32_t v;

  v = (uint32_t) addr[0];
  v |= (uint32_t) addr[1] << 8;
  v |= (uint32_t) addr[2] << 16;
  return v;
}

bfd_vma
bfd_getb32 (const void *p)
{
  const bfd_byte *addr = (const bfd_byte *) p;
  uint32_t v;

  v = (uint32_t) addr[0] << 24;
  v |= (uint32_t) addr[1] << 16;
  v |= (uint32_t) addr[2] << 8;
  v |= (uint32_t) addr[3];
  return v;
}

bfd_vma
bfd_getl32 (const void *p)
{
  const bfd_byte *addr = (const bfd_byte *) p;
  uint32_t v;

  v = (uint32_t) addr[0];
  v |= (uint32_t) addr[1] << 8;
  v |= (uint32_t) addr[2] << 16;
  v |= (uint32_t) addr[3] << 24;
  return v;
}

bfd_signed_vma
bfd_getb_signed_32 (const void *p)
{
  const bfd_byte *addr = (const bfd_byte *) p;
  uint32_t v;

  v = (uint32_t) addr[0] << 24;
  v |= (uint32_t) addr[1] << 16;
  v |= (uint32_t) addr[2] << 8;
  v |= (uint32_t) addr[3];
  return COERCE32 (v);
}

bfd_signed_vma
bfd_getl_signed_32 (const void *p)
{
  const bfd_byte *addr = (const bfd_byte *) p;
  uint32_t v;

  v = (uint32_t) addr[0];
  v |= (uint32_t) addr[1] << 8;
  v |= (uint32_t) addr[2] << 16;
  v |= (uint32_t) addr[3] << 24;
  return COERCE32 (v);
}

uint64_t
bfd_getb64 (const void *p)
{
  const bfd_byte *addr = (const bfd_byte *) p;
  uint64_t v;

  v  = addr[0]; v <<= 8;
  v |= addr[1]; v <<= 8;
  v |= addr[2]; v <<= 8;
  v |= addr[3]; v <<= 8;
  v |= addr[4]; v <<= 8;
  v |= addr[5]; v <<= 8;
  v |= addr[6]; v <<= 8;
  v |= addr[7];

  return v;
}

uint64_t
bfd_getl64 (const void *p)
{
  const bfd_byte *addr = (const bfd_byte *) p;
  uint64_t v;

  v  = addr[7]; v <<= 8;
  v |= addr[6]; v <<= 8;
  v |= addr[5]; v <<= 8;
  v |= addr[4]; v <<= 8;
  v |= addr[3]; v <<= 8;
  v |= addr[2]; v <<= 8;
  v |= addr[1]; v <<= 8;
  v |= addr[0];

  return v;
}

int64_t
bfd_getb_signed_64 (const void *p)
{
  const bfd_byte *addr = (const bfd_byte *) p;
  uint64_t v;

  v  = addr[0]; v <<= 8;
  v |= addr[1]; v <<= 8;
  v |= addr[2]; v <<= 8;
  v |= addr[3]; v <<= 8;
  v |= addr[4]; v <<= 8;
  v |= addr[5]; v <<= 8;
  v |= addr[6]; v <<= 8;
  v |= addr[7];

  return COERCE64 (v);
}

int64_t
bfd_getl_signed_64 (const void *p)
{
  const bfd_byte *addr = (const bfd_byte *) p;
  uint64_t v;

  v  = addr[7]; v <<= 8;
  v |= addr[6]; v <<= 8;
  v |= addr[5]; v <<= 8;
  v |= addr[4]; v <<= 8;
  v |= addr[3]; v <<= 8;
  v |= addr[2]; v <<= 8;
  v |= addr[1]; v <<= 8;
  v |= addr[0];

  return COERCE64 (v);
}

void
bfd_putb32 (bfd_vma data, void *p)
{
  bfd_byte *addr = (bfd_byte *) p;
  addr[0] = (data >> 24) & 0xff;
  addr[1] = (data >> 16) & 0xff;
  addr[2] = (data >>  8) & 0xff;
  addr[3] = data & 0xff;
}

void
bfd_putl32 (bfd_vma data, void *p)
{
  bfd_byte *addr = (bfd_byte *) p;
  addr[0] = data & 0xff;
  addr[1] = (data >>  8) & 0xff;
  addr[2] = (data >> 16) & 0xff;
  addr[3] = (data >> 24) & 0xff;
}

void
bfd_putb64 (uint64_t data, void *p)
{
  bfd_byte *addr = (bfd_byte *) p;
  addr[0] = (data >> (7*8)) & 0xff;
  addr[1] = (data >> (6*8)) & 0xff;
  addr[2] = (data >> (5*8)) & 0xff;
  addr[3] = (data >> (4*8)) & 0xff;
  addr[4] = (data >> (3*8)) & 0xff;
  addr[5] = (data >> (2*8)) & 0xff;
  addr[6] = (data >> (1*8)) & 0xff;
  addr[7] = (data >> (0*8)) & 0xff;
}

void
bfd_putl64 (uint64_t data, void *p)
{
  bfd_byte *addr = (bfd_byte *) p;
  addr[7] = (data >> (7*8)) & 0xff;
  addr[6] = (data >> (6*8)) & 0xff;
  addr[5] = (data >> (5*8)) & 0xff;
  addr[4] = (data >> (4*8)) & 0xff;
  addr[3] = (data >> (3*8)) & 0xff;
  addr[2] = (data >> (2*8)) & 0xff;
  addr[1] = (data >> (1*8)) & 0xff;
  addr[0] = (data >> (0*8)) & 0xff;
}

void
bfd_put_bits (uint64_t data, void *p, int bits, bool big_p)
{
  bfd_byte *addr = (bfd_byte *) p;
  int i;
  int bytes;

  if (bits % 8 != 0)
    abort ();

  bytes = bits / 8;
  for (i = 0; i < bytes; i++)
    {
      int addr_index = big_p ? bytes - i - 1 : i;

      addr[addr_index] = data & 0xff;
      data >>= 8;
    }
}

uint64_t
bfd_get_bits (const void *p, int bits, bool big_p)
{
  const bfd_byte *addr = (const bfd_byte *) p;
  uint64_t data;
  int i;
  int bytes;

  if (bits % 8 != 0)
    abort ();

  data = 0;
  bytes = bits / 8;
  for (i = 0; i < bytes; i++)
    {
      int addr_index = big_p ? i : bytes - i - 1;

      data = (data << 8) | addr[addr_index];
    }

  return data;
}

/* Default implementation */

bool
_bfd_generic_get_section_contents (bfd *abfd,
				   sec_ptr section,
				   void *location,
				   file_ptr offset,
				   bfd_size_type count)
{
  bfd_size_type sz;
  if (count == 0)
    return true;

  if (section->compress_status != COMPRESS_SECTION_NONE)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: unable to get decompressed section %pA"),
	 abfd, section);
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  sz = bfd_get_section_limit_octets (abfd, section);
  if (offset + count < count
      || offset + count > sz
      || (abfd->my_archive != NULL
	  && !bfd_is_thin_archive (abfd->my_archive)
	  && ((ufile_ptr) section->filepos + offset + count
	      > arelt_size (abfd))))
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  if (bfd_seek (abfd, section->filepos + offset, SEEK_SET) != 0
      || bfd_bread (location, count, abfd) != count)
    return false;

  return true;
}

bool
_bfd_generic_get_section_contents_in_window
  (bfd *abfd ATTRIBUTE_UNUSED,
   sec_ptr section ATTRIBUTE_UNUSED,
   bfd_window *w ATTRIBUTE_UNUSED,
   file_ptr offset ATTRIBUTE_UNUSED,
   bfd_size_type count ATTRIBUTE_UNUSED)
{
#ifdef USE_MMAP
  bfd_size_type sz;

  if (count == 0)
    return true;
  if (abfd->xvec->_bfd_get_section_contents
      != _bfd_generic_get_section_contents)
    {
      /* We don't know what changes the bfd's get_section_contents
	 method may have to make.  So punt trying to map the file
	 window, and let get_section_contents do its thing.  */
      /* @@ FIXME : If the internal window has a refcount of 1 and was
	 allocated with malloc instead of mmap, just reuse it.  */
      bfd_free_window (w);
      w->i = bfd_zmalloc (sizeof (bfd_window_internal));
      if (w->i == NULL)
	return false;
      w->i->data = bfd_malloc (count);
      if (w->i->data == NULL)
	{
	  free (w->i);
	  w->i = NULL;
	  return false;
	}
      w->i->mapped = 0;
      w->i->refcount = 1;
      w->size = w->i->size = count;
      w->data = w->i->data;
      return bfd_get_section_contents (abfd, section, w->data, offset, count);
    }
  if (abfd->direction != write_direction && section->rawsize != 0)
    sz = section->rawsize;
  else
    sz = section->size;
  if (offset + count < count
      || offset + count > sz
      || (abfd->my_archive != NULL
	  && !bfd_is_thin_archive (abfd->my_archive)
	  && ((ufile_ptr) section->filepos + offset + count
	      > arelt_size (abfd)))
      || ! bfd_get_file_window (abfd, section->filepos + offset, count, w,
				true))
    return false;
  return true;
#else
  abort ();
#endif
}

/* This generic function can only be used in implementations where creating
   NEW sections is disallowed.  It is useful in patching existing sections
   in read-write files, though.  See other set_section_contents functions
   to see why it doesn't work for new sections.  */
bool
_bfd_generic_set_section_contents (bfd *abfd,
				   sec_ptr section,
				   const void *location,
				   file_ptr offset,
				   bfd_size_type count)
{
  if (count == 0)
    return true;

  if (bfd_seek (abfd, section->filepos + offset, SEEK_SET) != 0
      || bfd_bwrite (location, count, abfd) != count)
    return false;

  return true;
}

/*
INTERNAL_FUNCTION
	bfd_log2

SYNOPSIS
	unsigned int bfd_log2 (bfd_vma x);

DESCRIPTION
	Return the log base 2 of the value supplied, rounded up.  E.g., an
	@var{x} of 1025 returns 11.  A @var{x} of 0 returns 0.
*/

unsigned int
bfd_log2 (bfd_vma x)
{
  unsigned int result = 0;

  if (x <= 1)
    return result;
  --x;
  do
    ++result;
  while ((x >>= 1) != 0);
  return result;
}

bool
bfd_generic_is_local_label_name (bfd *abfd, const char *name)
{
  char locals_prefix = (bfd_get_symbol_leading_char (abfd) == '_') ? 'L' : '.';

  return name[0] == locals_prefix;
}

/* Helper function for reading uleb128 encoded data.  */

bfd_vma
_bfd_read_unsigned_leb128 (bfd *abfd ATTRIBUTE_UNUSED,
			   bfd_byte *buf,
			   unsigned int *bytes_read_ptr)
{
  bfd_vma result;
  unsigned int num_read;
  unsigned int shift;
  bfd_byte byte;

  result = 0;
  shift = 0;
  num_read = 0;
  do
    {
      byte = bfd_get_8 (abfd, buf);
      buf++;
      num_read++;
      if (shift < 8 * sizeof (result))
	{
	  result |= (((bfd_vma) byte & 0x7f) << shift);
	  shift += 7;
	}
    }
  while (byte & 0x80);
  *bytes_read_ptr = num_read;
  return result;
}

/* Read in a LEB128 encoded value from ABFD starting at *PTR.
   If SIGN is true, return a signed LEB128 value.
   *PTR is incremented by the number of bytes read.
   No bytes will be read at address END or beyond.  */

bfd_vma
_bfd_safe_read_leb128 (bfd *abfd ATTRIBUTE_UNUSED,
		       bfd_byte **ptr,
		       bool sign,
		       const bfd_byte * const end)
{
  bfd_vma result = 0;
  unsigned int shift = 0;
  bfd_byte byte = 0;
  bfd_byte *data = *ptr;

  while (data < end)
    {
      byte = bfd_get_8 (abfd, data);
      data++;
      if (shift < 8 * sizeof (result))
	{
	  result |= ((bfd_vma) (byte & 0x7f)) << shift;
	  shift += 7;
	}
      if ((byte & 0x80) == 0)
	break;
    }

  *ptr = data;

  if (sign && (shift < 8 * sizeof (result)) && (byte & 0x40))
    result |= -((bfd_vma) 1 << shift);

  return result;
}

/* Helper function for reading sleb128 encoded data.  */

bfd_signed_vma
_bfd_read_signed_leb128 (bfd *abfd ATTRIBUTE_UNUSED,
			 bfd_byte *buf,
			 unsigned int *bytes_read_ptr)
{
  bfd_vma result;
  unsigned int shift;
  unsigned int num_read;
  bfd_byte byte;

  result = 0;
  shift = 0;
  num_read = 0;
  do
    {
      byte = bfd_get_8 (abfd, buf);
      buf ++;
      num_read ++;
      if (shift < 8 * sizeof (result))
	{
	  result |= (((bfd_vma) byte & 0x7f) << shift);
	  shift += 7;
	}
    }
  while (byte & 0x80);
  if (shift < 8 * sizeof (result) && (byte & 0x40))
    result |= (((bfd_vma) -1) << shift);
  *bytes_read_ptr = num_read;
  return result;
}

/* Write VAL in uleb128 format to P.
   END indicates the last byte of allocated space for the uleb128 value to fit
   in.
   Return a pointer to the byte following the last byte that was written, or
   NULL if the uleb128 value does not fit in the allocated space between P and
   END.  */
bfd_byte *
_bfd_write_unsigned_leb128 (bfd_byte *p, bfd_byte *end, bfd_vma val)
{
  bfd_byte c;
  do
    {
      if (p > end)
	return NULL;
      c = val & 0x7f;
      val >>= 7;
      if (val)
	c |= 0x80;
      *(p++) = c;
    }
  while (val);
  return p;
}

bool
_bfd_generic_init_private_section_data (bfd *ibfd ATTRIBUTE_UNUSED,
					asection *isec ATTRIBUTE_UNUSED,
					bfd *obfd ATTRIBUTE_UNUSED,
					asection *osec ATTRIBUTE_UNUSED,
					struct bfd_link_info *link_info ATTRIBUTE_UNUSED)
{
  return true;
}
