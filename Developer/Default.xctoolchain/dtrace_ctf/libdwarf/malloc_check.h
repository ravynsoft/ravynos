/*

  Copyright (C) 2005 Silicon Graphics, Inc.  All Rights Reserved.
  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2.1 of the GNU Lesser General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement
  or the like.  Any license provided herein, whether implied or
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with
  other software, or any other product whatsoever.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, write the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307,
  USA.

  Contact information:  Silicon Graphics, Inc., 1500 Crittenden Lane,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/


/* malloc_check.h */

/* A simple libdwarf-aware malloc checker. 
   define WANT_LIBBDWARF_MALLOC_CHECK and rebuild libdwarf
   do make a checking-for-alloc-mistakes libdwarf.
   NOT  recommended for production use.

   When defined, also add malloc_check.c to the list of
   files in Makefile.
*/

#undef WANT_LIBBDWARF_MALLOC_CHECK 
/*#define WANT_LIBBDWARF_MALLOC_CHECK  1 */

#ifdef WANT_LIBBDWARF_MALLOC_CHECK

void dwarf_malloc_check_alloc_data(void * addr,unsigned char code);
void dwarf_malloc_check_dealloc_data(void * addr,unsigned char code);
void dwarf_malloc_check_complete(char *wheremsg); /* called at exit of app */

#else /* !WANT_LIBBDWARF_MALLOC_CHECK */

#define dwarf_malloc_check_alloc_data(a,b)  /* nothing */
#define dwarf_malloc_check_dealloc_data(a,b)  /* nothing */
#define dwarf_malloc_check_complete(a) /* nothing */

#endif /* WANT_LIBBDWARF_MALLOC_CHECK */
