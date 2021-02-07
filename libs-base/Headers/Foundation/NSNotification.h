/** Interface for NSNotification and NSNotificationCenter for GNUstep
   Copyright (C) 1996,1999 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Rewrite by:  Richard Frith-Macdonald <rfm@gnu.org>
   Created: March 1996

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

  AutogsdocSource: NSNotification.m
  AutogsdocSource: NSNotificationCenter.m
*/

#ifndef __NSNotification_h_GNUSTEP_BASE_INCLUDE
#define __NSNotification_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSObject.h>
#import	<Foundation/NSMapTable.h>
#import <GNUstepBase/GSBlocks.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSString;
@class NSDictionary;
@class NSLock;
@class NSOperationQueue;

typedef NSString* NSNotificationName;

GS_EXPORT_CLASS
@interface NSNotification : NSObject <NSCopying, NSCoding>

/* Creating a Notification Object */
+ (NSNotification*) notificationWithName: (NSString*)name
				  object: (id)object;

+ (NSNotification*) notificationWithName: (NSString*)name
				  object: (id)object
			        userInfo: (NSDictionary*)info;

/* Querying a Notification Object */

- (NSString*) name;
- (id) object;
- (NSDictionary*) userInfo;

@end


#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
DEFINE_BLOCK_TYPE(GSNotificationBlock, void, NSNotification *);
#endif

GS_EXPORT_CLASS
@interface NSNotificationCenter : NSObject
{
#if	GS_EXPOSE(NSNotificationCenter)
@private
  void	*_table;
#endif
}

+ (NSNotificationCenter*) defaultCenter;

- (void) addObserver: (id)observer
            selector: (SEL)selector
                name: (NSString*)name
              object: (id)object;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
- (id) addObserverForName: (NSString *)name 
                   object: (id)object 
                    queue: (NSOperationQueue *)queue 
               usingBlock: (GSNotificationBlock)block;
#endif

- (void) removeObserver: (id)observer;
- (void) removeObserver: (id)observer
                   name: (NSString*)name
                 object: (id)object;

- (void) postNotification: (NSNotification*)notification;
- (void) postNotificationName: (NSString*)name
                       object: (id)object;
- (void) postNotificationName: (NSString*)name
                       object: (id)object
                     userInfo: (NSDictionary*)info;

@end

#if	defined(__cplusplus)
}
#endif

#endif /*__NSNotification_h_GNUSTEP_BASE_INCLUDE */
