/** Implementation of GSBlocks for GNUStep
   Copyright (C) 2011 Free Software Foundation, Inc.

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.

   */ 

#import "Foundation/NSObject.h"

@interface GSBlock : NSObject
@end

@implementation GSBlock
+ (void) load
{
  unsigned int	methodCount;
  Method	*m = NULL;
  Method	*methods = class_copyMethodList(self, &methodCount);
  id		blockClass = objc_lookUpClass("_NSBlock");
  Protocol	*nscopying = NULL;

  /* If we don't have an _NSBlock class, we don't have blocks support in the
   * runtime, so give up.
   */
  if (nil == blockClass)
    {
      return;
    }

  /* Copy all of the methods in this class onto the block-runtime-provided
   * _NSBlock
   */
  for (m = methods; NULL != *m; m++)
    {
      class_addMethod(blockClass, method_getName(*m),
	method_getImplementation(*m), method_getTypeEncoding(*m));
    }
  nscopying = objc_getProtocol("NSCopying");
  class_addProtocol(blockClass, nscopying);
  free(methods);
}

- (id) copyWithZone: (NSZone*)aZone
{
  return _Block_copy(self);
}

- (id) copy
{
  return _Block_copy(self);
}

- (id) retain
{
  return _Block_copy(self);
}

- (oneway void) release
{
  _Block_release(self);
}
@end

