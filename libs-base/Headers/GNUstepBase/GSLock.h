/** Some extra locking classes

   Copyright (C) 2003 Free Software Foundation, Inc.

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

   AutogsdocSource: Additions/GSLock.m

*/

#ifndef	INCLUDED_GS_LOCK_H
#define	INCLUDED_GS_LOCK_H

#ifndef NeXT_Foundation_LIBRARY
#import	<Foundation/NSLock.h>
#else
#import <Foundation/Foundation.h>
#endif

#import "GNUstepBase/GSObjCRuntime.h"

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSNotification;

@interface	GSLazyLock : NSLock
{
  int	locked;
}
- (void) _becomeThreaded: (NSNotification*)n;
@end

@interface	GSLazyRecursiveLock : NSRecursiveLock
{
  int	counter;
}
- (void) _becomeThreaded: (NSNotification*)n;
@end

/** Global lock to be used by classes when operating on any global
    data that invoke other methods which also access global; thus,
    creating the potential for deadlock. */
GS_EXPORT NSRecursiveLock *gnustep_global_lock;

#if	defined(__cplusplus)
}
#endif

#endif	/* INCLUDED_GS_LOCK_H */
