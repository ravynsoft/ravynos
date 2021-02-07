/* Interface of NSDistributedNotificationCenter class
   Copyright (C) 1998 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: October 1998

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

#ifndef __NSDistributedNotificationCenter_h_GNUSTEP_BASE_INCLUDE
#define __NSDistributedNotificationCenter_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)

#import	<Foundation/NSObject.h>
#import	<Foundation/NSLock.h>
#import	<Foundation/NSNotification.h>

#if	defined(__cplusplus)
extern "C" {
#endif


/**
 *  Enumeration of possible values for specifying how
 *  [NSDistributedNotificationCenter] deals with notifications when the
 *  process to which the notification should be delivered is suspended:
 <example>
 {
  NSNotificationSuspensionBehaviorDrop,       // drop the notification
  NSNotificationSuspensionBehaviorCoalesce,   // drop all for this process but the latest-sent notification
  NSNotificationSuspensionBehaviorHold,       // queue all notifications for this process until it is resumed
  NSNotificationSuspensionBehaviorDeliverImmediately  // resume the process and deliver
}
 </example>
 */
enum {
  NSNotificationSuspensionBehaviorDrop = 1,
  NSNotificationSuspensionBehaviorCoalesce = 2,
  NSNotificationSuspensionBehaviorHold = 3,
  NSNotificationSuspensionBehaviorDeliverImmediately = 4
};
typedef NSUInteger NSNotificationSuspensionBehavior;

/**
 *  Type for [NSDistributedNotificationCenter+notificationCenterForType:] -
 *  localhost current user broadcast only.  This is the only type on OS X.
 */
GS_EXPORT NSString* const NSLocalNotificationCenterType;
#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)

/**
 *  Type of [NSDistributedNotificationCenter+notificationCenterForType:] -
 *  all users on the local host.  This type is available only on GNUstep.
 */
GS_EXPORT NSString* const GSPublicNotificationCenterType;

/**
 *  Type of [NSDistributedNotificationCenter+notificationCenterForType:] -
 *  localhost and LAN broadcast.  This type is available only on GNUstep.
 */
GS_EXPORT NSString* const GSNetworkNotificationCenterType;
#endif

@interface	NSDistributedNotificationCenter : NSNotificationCenter
{
#if	GS_EXPOSE(NSDistributedNotificationCenter)
  NSRecursiveLock *_centerLock;	/* For thread safety.		*/
  NSString	*_type;		/* Type of notification center.	*/
  id		_remote;	/* Proxy for center.		*/
  BOOL		_suspended;	/* Is delivery suspended?	*/
#endif
#if     GS_NONFRAGILE
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}
+ (id) defaultCenter;
+ (NSDistributedNotificationCenter*) notificationCenterForType: (NSString*)type;

- (void) addObserver: (id)anObserver
	    selector: (SEL)aSelector
		name: (NSString*)notificationName
	      object: (NSString*)anObject;
- (void) addObserver: (id)anObserver
	    selector: (SEL)aSelector
		name: (NSString*)notificationName
	      object: (NSString*)anObject
  suspensionBehavior: (NSNotificationSuspensionBehavior)suspensionBehavior;
- (void) postNotification: (NSNotification*)notification;
- (void) postNotificationName: (NSString*)notificationName
		       object: (NSString*)anObject;
- (void) postNotificationName: (NSString*)notificationName
		       object: (NSString*)anObject
		     userInfo: (NSDictionary*)userInfo;
- (void) postNotificationName: (NSString*)notificationName
		       object: (NSString*)anObject
		     userInfo: (NSDictionary*)userInfo
	   deliverImmediately: (BOOL)deliverImmediately;
- (void) removeObserver: (id)anObserver
		   name: (NSString*)notificationName
		 object: (NSString*)anObject;
- (void) setSuspended: (BOOL)flag;
- (BOOL) suspended;

@end

#if	defined(__cplusplus)
}
#endif

#endif
#endif

