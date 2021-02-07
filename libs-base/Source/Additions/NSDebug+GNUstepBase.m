/** Debugging utilities for GNUStep and OpenStep
   Copyright (C) 1997,1999,2000,2001 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: August 1997
   Extended by: Nicola Pero <n.pero@mi.flashnet.it>
   Date: December 2000, April 2001

   This file is part of the GNUstep Base Library.

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
#import "GNUstepBase/NSDebug+GNUstepBase.h"

NSString*
GSDebugFunctionMsg(const char *func, const char *file, int line, NSString *fmt)
{
  NSString *message;

  message = [NSString stringWithFormat: @"File %s: %d. In %s %@",
	file, line, func, fmt];
  return message;
}

NSString*
GSDebugMethodMsg(id obj, SEL sel, const char *file, int line, NSString *fmt)
{
  NSString	*message;
  Class		cls = [obj class];
  char		c = '-';

  if (class_isMetaClass(cls))
    {
      cls = (Class)obj;
      c = '+';
    }
  message = [NSString stringWithFormat: @"File %s: %d. In [%@ %c%@] %@",
	file, line, NSStringFromClass(cls), c, NSStringFromSelector(sel), fmt];
  return message;
}

