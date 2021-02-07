/*
   Copyright (C) 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

   Author:  Alexander Malmberg <alexander@malmberg.org>

   This file is part of GNUstep.

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

#include <Foundation/NSString.h>
#include "FTFaceInfo.h"

@implementation FTFaceInfo

- (NSString *) description
{
  return [NSString stringWithFormat: @"<FTFaceInfo %p: '%@' %@ %i %i>",
    self, displayName, files, weight, traits];
}

/* FTFaceInfo:s should never be deallocated */
- (void) dealloc
{
  NSLog(@"Warning: -dealloc called on %@",self);
  GSNOSUPERDEALLOC;
}

@end

