/* Interface for NSProtocolChecker for GNUStep
   Copyright (C) 1995 Free Software Foundation, Inc.

   Written by:  Mike Kienenberger
   Date: Jun 1998
   
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

#ifndef __NSProtocolChecker_h_GNUSTEP_BASE_INCLUDE
#define __NSProtocolChecker_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSObject.h>
#import	<Foundation/NSProxy.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class Protocol;

GS_EXPORT_CLASS
@interface NSProtocolChecker : NSProxy
{
#if	GS_EXPOSE(NSProtocolChecker)
@private
  Protocol *_myProtocol;
  NSObject *_myTarget;
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

// Creating a checker

+ (id) protocolCheckerWithTarget: (NSObject*)anObject
			protocol: (Protocol*)aProtocol;

- (id) initWithTarget: (NSObject*)anObject
	     protocol: (Protocol*)aProtocol;

// Reimplemented NSObject methods
 
- (void) forwardInvocation: (NSInvocation*)anInvocation;
   
// Getting information
- (Protocol*) protocol;
- (NSObject*) target;

@end

#if	defined(__cplusplus)
}
#endif

#endif
