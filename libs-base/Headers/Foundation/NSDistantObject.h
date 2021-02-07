/* Interface for GNU Objective-C version of NSDistantObject
   Copyright (C) 1997 Free Software Foundation, Inc.
   
   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Based on code by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
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

#ifndef __NSDistantObject_h_GNUSTEP_BASE_INCLUDE
#define __NSDistantObject_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSProxy.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class	NSConnection;

GS_EXPORT_CLASS
@interface NSDistantObject : NSProxy <NSCoding>
{
#if	GS_EXPOSE(NSDistantObject)
@public
  NSConnection	*_connection;
  id		_object;
  unsigned	_handle;
  Protocol	*_protocol;
  unsigned	_counter;
  void		*_sigs;
#endif
}

+ (NSDistantObject*) proxyWithLocal: (id)anObject
			 connection: (NSConnection*)aConnection;
/*
 *	NB. Departure from the OpenStep/MacOS spec - the type of a target
 *	is an integer, not an id, since we can't safely pass id's
 *	between address spaces on machines with different pointer sizes.
 */
+ (NSDistantObject*) proxyWithTarget: (unsigned)anObject
			  connection: (NSConnection*)aConnection;

- (NSConnection*) connectionForProxy;
- (id) initWithLocal: (id)anObject
	  connection: (NSConnection*)aConnection;
- (id) initWithTarget: (unsigned)target
	   connection: (NSConnection*)aConnection;
- (NSMethodSignature*) methodSignatureForSelector: (SEL)aSelector;
- (void) setProtocolForProxy: (Protocol*)aProtocol;

@end

#if	defined(__cplusplus)
}
#endif

#endif /* __NSDistantObject_h_GNUSTEP_BASE_INCLUDE */
