/** Control of executable units within a shared virtual memory space
   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by: Richard Frith-Macdonald <richard@brainstorm.co.uk>

   This file is part of the GNUstep Objective-C Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.

   $Date: 2010-02-17 11:47:06 +0000 (Wed, 17 Feb 2010) $ $Revision: 29657 $
*/

#import "common.h"
#import "Foundation/NSThread.h"

#if	defined(NeXT_Foundation_LIBRARY)

/* These functions are in NSThread.m in the base library.
 */
NSThread*
GSCurrentThread(void)
{
  return [NSThread currentThread];
}

NSMutableDictionary*
GSCurrentThreadDictionary(void)
{
  return [GSCurrentThread() threadDictionary];
}

#endif

