/* Implementation of extension methods to base additions

   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>

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
#import "Foundation/NSException.h"
#import "GNUstepBase/NSLock+GNUstepBase.h"
#import "GNUstepBase/GSLock.h"

/**
 * GNUstep specific (non-standard) additions to the NSLock class.
 */

static GSLazyRecursiveLock *local_lock = nil;

/* This class only exists to provide a thread safe mechanism to
   initialize local_lock as +initialize is called under a lock in ObjC
   runtimes.  User code should resort to GS_INITIALIZED_LOCK(), which
   uses the +newLockAt: extension.  */

@interface _GSLockInitializer : NSObject
@end
@implementation _GSLockInitializer
+ (void) initialize
{
  if (local_lock == nil)
    {
      /* As we do not know whether creating custom locks may
	 implicitly create other locks, we use a recursive lock.  */
      local_lock = [GSLazyRecursiveLock new];
    }
}

@end

static inline id
newLockAt(Class self, SEL _cmd, id *location)
{
  if (location == 0)
    {
      [NSException raise: NSInvalidArgumentException
                   format: @"'%@' called with nil location",
		   NSStringFromSelector(_cmd)];
    }

  if (*location == nil)
    {
      if (local_lock == nil)
	{
	  [_GSLockInitializer class];
	}

      [local_lock lock];

      if (*location == nil)
	{
	  *location = [[(id)self alloc] init];
	}

      [local_lock unlock];
    }

  return *location;
}


@implementation NSLock (GNUstepBase)
+ (id) newLockAt: (id *)location
{
  return newLockAt(self, _cmd, location);
}
@end

@implementation NSRecursiveLock (GNUstepBase)
+ (id) newLockAt: (id *)location
{
  return newLockAt(self, _cmd, location);
}
@end

