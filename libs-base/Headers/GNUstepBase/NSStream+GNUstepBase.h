/** Declaration of extension methods for base additions

   Copyright (C) 2010 Free Software Foundation, Inc.

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

*/

#ifndef	INCLUDED_NSStream_GNUstepBase_h
#define	INCLUDED_NSStream_GNUstepBase_h

#import <GNUstepBase/GSVersionMacros.h>
#import <Foundation/NSStream.h>

#if	defined(__cplusplus)
extern "C" {
#endif

#if	OS_API_VERSION(GS_API_NONE,GS_API_LATEST)

/**
 * The additional interface defined for GNUstep.<br />
 * Currently NOT implemented when using the Apple Foundation!<br />
 * Please contribute an Apple implementation.
 */
@interface NSStream (GNUstepBase)

/**
 * Creates and returns by reference an NSInputStream object and
 * NSOutputStream object for a local socket or named pipe connection
 * with the specified path. To use them you need to open them and wait
 * on the NSStreamEventOpenCompleted event on one of them.
 */
+ (void) getLocalStreamsToPath: (NSString *)path 
		   inputStream: (NSInputStream **)inputStream 
		  outputStream: (NSOutputStream **)outputStream;
/**
 * Creates and returns by reference an NSInputStream object and NSOutputStream 
 * object for a anonymous local socket or pipe. Although you still need to
 * open them, the open will be instantaneous, and no NSStreamEventOpenCompleted
 * event will be delivered.
 */
+ (void) pipeWithInputStream: (NSInputStream **)inputStream 
                outputStream: (NSOutputStream **)outputStream;
@end

/**
 * GSServerStream is a subclass of NSStream that encapsulate a "server"
 * stream; that is a stream that binds to a socket and accepts incoming
 * connections.<br />
 * Currently NOT implemented when using the Apple Foundation!<br />
 * Please contribute an Apple implementation.
 */
GS_EXPORT_CLASS
@interface GSServerStream : NSStream

/**
 * Create an IP (ipv4 or ipv6) server stream.<br />
 * The addr argument must be a string object containing a standard text
 * representation of an internet address (either the dot separated ipv4
 * syntax or the colon separated ipv6 syntax).
 */
+ (id) serverStreamToAddr: (NSString*)addr port: (NSInteger)port;

/**
 * Create a local (unix domain or named pipe) server stream.
 */
+ (id) serverStreamToAddr: (NSString*)addr;

/**
 * This is the method that accepts a connection and generates two streams
 * as the server side inputStream and OutputStream.<br />
 * You call this when you receive an event on the server stream signalling
 * that it is 'readable' (bytes available).<br />
 * Although you still need to open them, the open will be instantaneous,
 * and no NSStreamEventOpenCompleted event will be delivered.
 */
- (void) acceptWithInputStream: (NSInputStream **)inputStream 
                  outputStream: (NSOutputStream **)outputStream;

/**
 * The designated initializer for an IP (ipv4 or ipv6) server stream.<br />
 * The addr argument must be a string object containing a standard text
 * representation of an internet address (either the dot separated ipv4
 * syntax or the colon separated ipv6 syntax).
 */
- (id) initToAddr: (NSString*)addr port: (NSInteger)port;

/**
 * The designated initializer for a local (unix domain or named pipe)
 * server stream.
 */
- (id) initToAddr: (NSString*)addr;

@end

/** May be used to read the local IP address of a tcp/ip network stream. */
GS_EXPORT NSString * const GSStreamLocalAddressKey;
/** May be used to read the local port of a tcp/ip network stream. */
GS_EXPORT NSString * const GSStreamLocalPortKey;
/** May be used to read the remote IP address of a tcp/ip network stream. */
GS_EXPORT NSString * const GSStreamRemoteAddressKey;
/** May be used to read the remote port of a tcp/ip network stream. */
GS_EXPORT NSString * const GSStreamRemotePortKey;

#endif	/* OS_API_VERSION */

#if	defined(__cplusplus)
}
#endif

#endif	/* INCLUDED_NSStream_GNUstepBase_h */

