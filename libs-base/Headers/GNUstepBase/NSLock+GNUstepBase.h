/** Declaration of extension methods for base additions

   Copyright (C) 2003-2010 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   and:         Adam Fedor <fedor@gnu.org>

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

#ifndef	INCLUDED_NSLock_GNUstepBase_h
#define	INCLUDED_NSLock_GNUstepBase_h

#import <GNUstepBase/GSVersionMacros.h>
#import <Foundation/NSLock.h>

#if	defined(__cplusplus)
extern "C" {
#endif

#if	OS_API_VERSION(GS_API_NONE,GS_API_LATEST)

/**
 * Returns IDENT which will be initialized
 * to an instance of a CLASSNAME in a thread safe manner.  
 * If IDENT has been previously initialized 
 * this macro merely returns IDENT.
 * IDENT is considered uninitialized, if it contains nil.
 * CLASSNAME must be either NSLock, NSRecursiveLock or one
 * of their subclasses.
 * See [NSLock+newLockAt:] for details.
 * This macro is intended for code that cannot insure
 * that a lock can be initialized in thread safe manner otherwise.
 * <example>
 * NSLock *my_lock = nil;
 *
 * void function (void)
 * {
 *   [GS_INITIALIZED_LOCK(my_lock, NSLock) lock];
 *   do_work ();
 *   [my_lock unlock];
 * }
 *
 * </example>
 */
#define GS_INITIALIZED_LOCK(IDENT,CLASSNAME) \
           (IDENT != nil ? (id)IDENT : (id)[CLASSNAME newLockAt: &IDENT])

@interface NSLock (GNUstepBase)
/**
 * Initializes the id pointed to by location
 * with a new instance of the receiver's class
 * in a thread safe manner, unless
 * it has been previously initialized.
 * Returns the contents pointed to by location.  
 * The location is considered unintialized if it contains nil.
 * <br/>
 * This method is used in the GS_INITIALIZED_LOCK macro
 * to initialize lock variables when it cannot be insured
 * that they can be initialized in a thread safe environment.
 * <example>
 * NSLock *my_lock = nil;
 *
 * void function (void)
 * {
 *   [GS_INITIALIZED_LOCK(my_lock, NSLock) lock];
 *   do_work ();
 *   [my_lock unlock];
 * }
 * 
 * </example>
 */
+ (id) newLockAt: (id *)location;
@end

@interface NSRecursiveLock (GNUstepBase)
/**
 * Initializes the id pointed to by location
 * with a new instance of the receiver's class
 * in a thread safe manner, unless
 * it has been previously initialized.
 * Returns the contents pointed to by location.  
 * The location is considered unintialized if it contains nil.
 * <br/>
 * This method is used in the GS_INITIALIZED_LOCK macro
 * to initialize lock variables when it cannot be insured
 * that they can be initialized in a thread safe environment.
 * <example>
 * NSLock *my_lock = nil;
 *
 * void function (void)
 * {
 *   [GS_INITIALIZED_LOCK(my_lock, NSLock) lock];
 *   do_work ();
 *   [my_lock unlock];
 * }
 * 
 * </example>
 */
+ (id) newLockAt: (id *)location;
@end

#endif	/* OS_API_VERSION */

#if	defined(__cplusplus)
}
#endif

#endif	/* INCLUDED_NSLock_GNUstepBase_h */

