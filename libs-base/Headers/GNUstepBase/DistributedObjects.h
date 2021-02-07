/* Interface internal use by Distributed Objects components
   Copyright (C) 1997 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: August 1997

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

#ifndef __DistributedObjects_h
#define __DistributedObjects_h

/*
 *	For <strong>INTERNAL</strong> use by the GNUstep base library.
 *	This file should not be installed.  The only reason why it is
 *	located here, is to allow target specific headers (like mframe.h), 
 *	which are located according to dis/enabled-flattened,
 *	may include this file via standard "GNUstepBase/DistributedObjects.h"
 *	and won't require an extra -I flag.
 *	
 *	Classes should implement [-classForPortCoder] to return the class
 *	that should be sent over the wire.
 *
 *	Classes should implement [-replacementObjectForPortCoder:] to encode
 *	objects.
 *	The default action is to send a proxy.
 */

#import <Foundation/NSConnection.h>
#import <Foundation/NSDistantObject.h>
#import <Foundation/NSPortCoder.h>
#import <Foundation/NSPort.h>

/*
 *	Distributed Objects identifiers
 *	These define the type of messages sent by the D.O. system.
 */
enum {
 METHOD_REQUEST = 0,
 METHOD_REPLY,
 ROOTPROXY_REQUEST,
 ROOTPROXY_REPLY,
 CONNECTION_SHUTDOWN,
 METHODTYPE_REQUEST,
 METHODTYPE_REPLY,
 PROXY_RELEASE,
 PROXY_RETAIN,
 RETAIN_REPLY
};


/*
 * Category containing the methods by which the public interface to
 * NSConnection must be extended in order to allow it's use by
 * by NSDistantObject et al for implementation of Distributed objects.
 */
@interface NSConnection (Internal)
- (NSDistantObject*) includesLocalTarget: (unsigned)target;
- (NSDistantObject*) localForObject: (id)object;
- (NSDistantObject*) locateLocalTarget: (unsigned)target;
- (NSDistantObject*) proxyForTarget: (unsigned)target;
- (void) retainTarget: (unsigned)target;

- (void) forwardInvocation: (NSInvocation *)inv 
		  forProxy: (NSDistantObject*)object;
- (const char *) typeForSelector: (SEL)sel remoteTarget: (unsigned)target;
@end

@interface NSPort (Internal)
- (id) conversation: (NSPort*)receivePort;
@end

#endif /* __DistributedObjects_h */
