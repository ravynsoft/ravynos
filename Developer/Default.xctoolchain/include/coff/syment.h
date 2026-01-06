#ifndef _COFF_SYMENT_H
#define _COFF_SYMENT_H
/*
 * These data structures are discribed in the pecoff_v8.doc in section
 * "5.4. COFF Symbol Table"
 */
#include <stdint.h>

/*
 * Since definitions for this header were based from the GNU binutils
 * coff/external.h header file that copyright info is below.
 */

/* external.h  -- External COFF structures
   
   Copyright 2001 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#define E_SYMNMLEN	8	/* # characters in a symbol name	*/

struct syment 
{
  union
  {
    char e_name[E_SYMNMLEN];

    struct
    {
      uint32_t e_zeroes;
      uint32_t e_offset;
    } e;
  } e;

  uint32_t e_value;
  uint16_t e_scnum;
  uint16_t e_type;
  char e_sclass;
  char e_numaux;
}
/*
 * The symbol table is an array of this struct which must be 18 bytes in size.
 * Which is why the packed and and alignment of 2 bytes is done.
 */
__attribute((packed,aligned(2))) ;

/* constants used in the e_sclass (Storage Class) field */
#define IMAGE_SYM_CLASS_EXTERNAL 2
#endif /* _COFF_SYMENT_H */
