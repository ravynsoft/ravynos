/** Interface of NSPortNameServer class for Distributed Objects
   Copyright (C) 1998,1999,2003 Free Software Foundation, Inc.

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

   <title>NSPortNameServer class reference</title>

   AutogsdocSource: NSPortNameServer.m
   AutogsdocSource: NSSocketPortNameServer.m
   AutogsdocSource: NSMessagePortNameServer.m

   */

#ifndef __NSPortNameServer_h_GNUSTEP_BASE_INCLUDE
#define __NSPortNameServer_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSObject.h>
#import	<Foundation/NSMapTable.h>

#if	OS_API_VERSION(GS_API_MACOSX,HS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class	NSPort, NSString, NSMutableArray;

GS_EXPORT_CLASS
@interface	NSPortNameServer : NSObject
+ (id) systemDefaultPortNameServer;
- (NSPort*) portForName: (NSString*)name;
- (NSPort*) portForName: (NSString*)name
		 onHost: (NSString*)host;
- (BOOL) registerPort: (NSPort*)port
	      forName: (NSString*)name;
- (BOOL) removePortForName: (NSString*)name;
@end

GS_EXPORT_CLASS
@interface NSSocketPortNameServer : NSPortNameServer
{
#if	GS_EXPOSE(NSSocketPortNameServer)
  NSMapTable	*_portMap;	/* Registered ports information.	*/
  NSMapTable	*_nameMap;	/* Registered names information.	*/
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
+ (id) sharedInstance;
- (NSPort*) portForName: (NSString*)name
		 onHost: (NSString*)host;
- (BOOL) registerPort: (NSPort*)port
	      forName: (NSString*)name;
- (BOOL) removePortForName: (NSString*)name;
@end


GS_EXPORT_CLASS
@interface NSMessagePortNameServer : NSPortNameServer
+ (id) sharedInstance;

/** Returns the [NSMessagePort] instance registered for the specified name
 * if it exists on the local host.
 */
- (NSPort*) portForName: (NSString*)name;

/** Returns the port registered for the specified name (if it exists).<br />
 * The host must be an empty string or nil, since [NSMessagePort] instances
 * on other hosts are inaccessible from the current host.
 */
- (NSPort*) portForName: (NSString*)name
		 onHost: (NSString*)host;
@end

#if	defined(__cplusplus)
}
#endif

#endif

#endif

