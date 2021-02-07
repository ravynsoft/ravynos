/*
   Cocoa.h

   Cocoa compatible declarations. Not to be used in normal GNUstep code.

   Copyright (C) 2004 Free Software Foundation, Inc.

   Author:  Fred Kiefer <fredkiefer@gmx.de>
   Date: 2004

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

#ifndef _GNUstep_H_COCOA
#define _GNUstep_H_COCOA

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433
#endif

#if (!defined(__cplusplus) && !defined(__USE_ISOC99) && !defined(__bool_true_false_are_defined))
typedef BOOL bool;
#define false NO
#define true  YES
#endif 

#endif /* _GNUstep_H_COCOA */
