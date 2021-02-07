/* Interface for GNU Objective-C version of NSProxy
   Copyright (C) 1997 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: August 1997

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

#ifndef __NSProxy_h_GNUSTEP_BASE_INCLUDE
#define __NSProxy_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSObject.h>

#if	defined(__cplusplus)
extern "C" {
#endif

GS_EXPORT_CLASS GS_ROOT_CLASS
@interface NSProxy <NSObject>
{
@public
    Class	isa;
#if !(GS_NONFRAGILE == 1)
@private
	/**
	 * Legacy compatibility ivar.  Remove in the next ABI-breaking release.
	 */
    NSUInteger	_retain_count;
#endif
}

+ (id) alloc;
+ (id) allocWithZone: (NSZone*)z;
+ (id) autorelease;
+ (Class) class;
+ (NSString*) description;
+ (BOOL) isKindOfClass: (Class)aClass;
+ (BOOL) isMemberOfClass: (Class)aClass;
/** <override-dummy />
 */
+ (void) load;
/** <override-dummy />
 */
+ (oneway void) release;
+ (BOOL) respondsToSelector: (SEL)aSelector;
+ (id) retain;
+ (NSUInteger) retainCount;

- (id) autorelease;
- (Class) class;
- (BOOL) conformsToProtocol: (Protocol*)aProtocol;
- (void) dealloc;
- (NSString*) description;
- (void) forwardInvocation: (NSInvocation*)anInvocation;
- (NSUInteger) hash;
- (id) init;
- (BOOL) isEqual: (id)anObject;
- (BOOL) isKindOfClass: (Class)aClass;
- (BOOL) isMemberOfClass: (Class)aClass;
- (BOOL) isProxy;
- (NSMethodSignature*) methodSignatureForSelector: (SEL)aSelector;
- (oneway void) release;
- (BOOL) respondsToSelector: (SEL)aSelector;
- (id) retain;
- (NSUInteger) retainCount;
- (id) self;
- (Class) superclass;
- (NSZone*) zone;

@end

#if	defined(__cplusplus)
}
#endif

#endif /* __NSProxy_h_GNUSTEP_BASE_INCLUDE */
