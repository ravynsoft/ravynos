/** Implementation for GSLock

   Copyright (C) 2003 Free Software Foundation, Inc.

   Written by: Richard Frith-Macdonald <rfm@gnu.org>
   Date: October 2003

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

   $Date$ $Revision$
*/

#import "common.h"
#define	EXPOSE_GSLock_IVARS	1
#import "Foundation/NSException.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSNotification.h"
#import "Foundation/NSThread.h"
#import "GNUstepBase/GSLock.h"

/**
 * This implements a class which, when used in single-threaded mode,
 * acts like a lock while avoiding the overheads of actually using
 * a real lock.  However, when the programm in which the class is
 * used becomes multi-threaded, all instances of this class transform
 * themselves into real locks in the correct state (locked/unlocked)
 * corresponding to whether the lazy lock was locked or not at the
 * point where the program became multi threadeed.<br />
 * Use of this class allows you to write thread-safe code which avoids
 * locking inefficiencies when used in a single threaded application,
 * without having to worry about dealing with the issue yourself.
 */
@implementation	GSLazyLock

/**
 * Do not use this method ... it is used internally to handle the transition
 * from a single threaded system to a multi threaded one.
 */
- (void) _becomeThreaded: (NSNotification*)n
{
  [[NSNotificationCenter defaultCenter] removeObserver: self];
  object_setClass(self, [NSLock class]);
  if (locked == YES)
    {
      if ([self tryLock] == NO)
	{
	  [NSException raise: NSInternalInconsistencyException
		      format: @"Failed to lock mutex"];
	}
    }
  /*
   * While we have changed 'isa', it's possible someone might have
   * cached our old method implementations, so we set the 'locked'
   * ivar to a value to tell the old method implementations to use
   * the superclass implementatins.
   */
  locked = -1;
}

- (void) dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver: self];
  [super dealloc];
}

- (id) init
{
  self = [super init];

  if ([NSThread isMultiThreaded] == YES)
    {
      DESTROY(self);
      return (GSLazyLock*)[NSLock new];
    }
  else if (self != nil)
    {
      locked = NO;
      [[NSNotificationCenter defaultCenter]
	addObserver: self
	selector: @selector(_becomeThreaded:)
	name: NSWillBecomeMultiThreadedNotification
	object: nil];
    }
  return self;
}

- (void) lock
{
  if (locked == NO)
    {
      locked = YES;
    }
  else if (locked == YES)
    {
      [NSException raise: NSGenericException
		  format: @"lock: when already locked"];
    }
  else
    {
      [super lock];
    }
}

- (BOOL) lockBeforeDate: (NSDate*)limit
{
  BOOL	result;

  if (locked == NO)
    {
      result = YES;
    }
  else if (locked == YES)
    {
      result = NO;
      [NSException raise: NSGenericException
		  format: @"lock: when already locked"];
    }
  else
    {
      result = [super lockBeforeDate: limit];
    }
  return result;
}

- (BOOL) tryLock
{
  if (locked == NO)
    {
      locked = YES;
      return YES;
    }
  else if (locked == YES)
    {
      return NO;
    }
  else
    {
      return [super tryLock];
    }
}

- (void) unlock
{
  if (locked == YES)
    {
      locked = NO;
    }
  else if (locked == NO)
    {
      [NSException raise: NSGenericException
		  format: @"unlock: when already unlocked"];
    }
  else
    {
      [super unlock];
    }
}

@end



/**
 * This implements a class which, when used in single-threaded mode,
 * acts like a recursive lock while avoiding the overheads of using
 * a real lock.  However, when the programm in which the class is
 * used becomes multi-threaded, all instances of this class transform
 * themselves into real locks in the correct state (locked/unlocked)
 * corresponding to whether the lazy recursive lock was locked or not
 * at the point where the program became multi threadeed.<br />
 * Use of this class allows you to write thread-safe code which avoids
 * locking inefficiencies when used in a single threaded application,
 * without having to worry about dealing with the issue yourself.
 */
@implementation	GSLazyRecursiveLock

/**
 * Do not use this method ... it is used internally to handle the transition
 * from a single threaded system to a multi threaded one.
 */
- (void) _becomeThreaded: (NSNotification*)n
{
  [[NSNotificationCenter defaultCenter] removeObserver: self];
  object_setClass(self, [NSRecursiveLock class]);
  while (counter-- > 0)
    {
      if ([self tryLock] == NO)
	{
	  [NSException raise: NSInternalInconsistencyException
		      format: @"Failed to lock mutex"];
	}
    }
  /*
   * While we have changed 'isa', it's possible someone might have
   * cached our old method implementations, so we set the 'locked'
   * ivar to a value to tell the old method implementations to use
   * the superclass implementatins.
   */
  counter = -1;
}

- (void) dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver: self];
  [super dealloc];
}

- (id) init
{
  self = [super init];

  if ([NSThread isMultiThreaded] == YES)
    {
      DESTROY(self);
      return (GSLazyRecursiveLock*)[NSRecursiveLock new];
    }
  else
    {
      if (self != nil)
	{
	  [[NSNotificationCenter defaultCenter]
	    addObserver: self
	    selector: @selector(_becomeThreaded:)
	    name: NSWillBecomeMultiThreadedNotification
	    object: nil];
	}
    }
  return self;
}

- (void) lock
{
  if (counter >= 0)
    {
      counter++;
    }
  else
    {
      [super lock];
    }
}

- (BOOL) lockBeforeDate: (NSDate*)limit
{
  if (counter >= 0)
    {
      counter++;
      return YES;
    }
  else
    {
      return [super lockBeforeDate: limit];
    }
}

- (BOOL) tryLock
{
  if (counter >= 0)
    {
      counter++;
      return YES;
    }
  else
    {
      return [super tryLock];
    }
}

- (void) unlock
{
  if (counter > 0)
    {
      counter--;
    }
  else if (counter == 0)
    {
      [NSException raise: NSGenericException
		  format: @"unlock: failed to unlock mutex"];
    }
  else
    {
      [super unlock];
    }
}

@end

/* Global lock to be used by classes when operating on any global
   data that invoke other methods which also access global; thus,
   creating the potential for deadlock. */
NSRecursiveLock *gnustep_global_lock = nil;

