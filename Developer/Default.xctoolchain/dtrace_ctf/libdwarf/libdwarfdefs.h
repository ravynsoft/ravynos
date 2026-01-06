/*

  Copyright (C) 2000,2004 Silicon Graphics, Inc.  All Rights Reserved.

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


/* libdwarfdefs.h
*/

#ifndef LIBDWARFDEFS_H
#define LIBDWARFDEFS_H

/* We want __uint32_t and __uint64_t and __int32_t __int64_t
   properly defined but not duplicated, since duplicate typedefs
   are not legal C.
*/
/*
 HAVE___UINT32_T
 HAVE___UINT64_T will be set by configure if
 our 4 types are predefined in compiler
*/


#if (!defined(HAVE___UINT32_T)) && defined(HAVE___UINT32_T_IN_SGIDEFS_H)
#include <sgidefs.h>		/* sgidefs.h defines them */
#define HAVE___UINT32_T 1
#endif

#if (!defined(HAVE___UINT64_T)) && defined(HAVE___UINT64_T_IN_SGIDEFS_H)
#include <sgidefs.h>		/* sgidefs.h defines them */
#define HAVE___UINT64_T 1
#endif


#if (!defined(HAVE___UINT32_T)) &&   \
	defined(HAVE_SYS_TYPES_H) &&   \
	defined(HAVE___UINT32_T_IN_SYS_TYPES_H)
#  include <sys/types.h>
#define HAVE___UINT32_T 1
#endif

#if (!defined(HAVE___UINT64_T)) &&   \
	defined(HAVE_SYS_TYPES_H) &&   \
	defined(HAVE___UINT64_T_IN_SYS_TYPES_H)
#  include <sys/types.h>
#define HAVE___UINT64_T 1
#endif

#ifndef HAVE___UINT32_T
typedef int __int32_t;
typedef unsigned __uint32_t;
#define HAVE___UINT32_T 1
#endif

#ifndef HAVE___UINT64_T
typedef long long __int64_t;
typedef unsigned long long __uint64_t;
#define HAVE___UINT64_T 1
#endif

#endif /* LIBDWARFDEFS_H */
