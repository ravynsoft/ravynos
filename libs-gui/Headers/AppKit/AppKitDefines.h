/* Plateform specific definitions for externs
   Copyright (C) 2001 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@gnu.org>
   Date: Jul, 2001
   
   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/ 

#ifndef __AppKitDefines_INCLUDE
#define __AppKitDefines_INCLUDE
#import <GNUstepBase/GSVersionMacros.h>

#ifdef __cplusplus
#define APPKIT_EXTERN extern "C"
#else
#define APPKIT_EXTERN extern
#endif

#ifdef GNUSTEP_WITH_DLL 

#if BUILD_libgnustep_gui_DLL
#
# if defined(__MINGW32__)
  /* On Mingw, the compiler will export all symbols automatically, so
   * __declspec(dllexport) is not needed.
   */
#  define APPKIT_EXPORT  APPKIT_EXTERN
#  define APPKIT_DECLARE 
# else
#  define APPKIT_EXPORT  __declspec(dllexport) APPKIT_EXTERN
#  define APPKIT_DECLARE __declspec(dllexport)
# endif
#else
#  define APPKIT_EXPORT  APPKIT_EXTERN __declspec(dllimport)
#  define APPKIT_DECLARE __declspec(dllimport)
#endif

#else /* GNUSTEP_WITH[OUT]_DLL */

#  define APPKIT_EXPORT APPKIT_EXTERN 
#  define APPKIT_DECLARE

#endif

#ifdef __clang__
#  define PACKAGE_SCOPE @package
#else
#  define PACKAGE_SCOPE @public
#endif

#endif /* __AppKitDefines_INCLUDE */
