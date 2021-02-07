/* GSPortPrivate
   Copyright (C) 2005 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   
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
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
   MA 02111 USA.
*/ 

#ifndef __GSPortPrivate_h_
#define __GSPortPrivate_h_

/*
 * Nameserver deregistration methods
 */
@interface      NSPortNameServer (GNUstep)
- (NSArray*) namesForPort: (NSPort*)port;       /* return all names for port */
- (BOOL) removePort: (NSPort*)port;             /* remove all names for port */
- (BOOL) removePort: (NSPort*)port forName: (NSString*)name;
@end

#if	defined(_WIN32)
@interface NSMessagePort(Private)
+ (id) newWithName: (NSString*)name;
- (id) initWithName: (NSString*)name;
- (NSString*) name;
- (void) receivedEventRead;
- (void) receivedEventWrite;
@end
#else
@class	GSMessageHandle;

@interface NSMessagePort(Private)
- (int) _listener;
- (const unsigned char *) _name;
+ (NSMessagePort*) _portWithName: (const unsigned char *)socketName
			listener: (BOOL)shouldListen;
- (void) addHandle: (GSMessageHandle*)handle forSend: (BOOL)send;
- (void) removeHandle: (GSMessageHandle*)handle;
@end
#endif	/* _WIN32 */

@class	GSTcpHandle;

@interface	NSSocketPort (Private)
- (void) addHandle: (GSTcpHandle*)handle forSend: (BOOL)send;
- (GSTcpHandle*) handleForPort: (NSSocketPort*)recvPort
                    beforeDate: (NSDate*)when;
- (void) removeHandle: (GSTcpHandle*)handle;
@end

#endif

