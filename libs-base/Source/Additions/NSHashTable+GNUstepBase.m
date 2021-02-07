/* Implementation of extension methods to base additions

   Copyright (C) 2015 Free Software Foundation, Inc.

   Written by:  Niels Grewe <niels.grewe@halbordnung.de>

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

*/
#import "common.h"
#import "Foundation/NSArray.h"
#import "GNUstepBase/NSHashTable+GNUstepBase.h"
#import "GSPrivate.h"
#import "GSFastEnumeration.h"

@implementation NSHashTable (GNUstepBase)

- (void)addObjectsFromArray: (NSArray*)array
{
  FOR_IN(id, obj, array)
    NSHashInsert(self,obj);
  END_FOR_IN(array)
}
@end
