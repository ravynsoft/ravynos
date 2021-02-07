/*
   NSNibDeclarations.h

   Declarations for types used by Interface Builder when reading header
   files.

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author:  Michael Giddings <giddings@genetics.utah.edu>
   Date: Feb. 1999

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

#ifndef _NSNibDeclarations_H_
#define _NSNibDeclarations_H_
#import <GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/* IBOutlet and IBAction are now built-in macros in recent Clang */
#if !defined(IBOutlet)
#define IBOutlet
#endif
#if !defined(IBAction)
#define IBAction void
#endif
#endif

#endif
