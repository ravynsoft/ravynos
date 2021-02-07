#ifndef	INCLUDED_GSSOCKETSTREAM_H
#define	INCLUDED_GSSOCKETSTREAM_H

/** Implementation for GSSocketStream for GNUStep
   Copyright (C) 2006-2008 Free Software Foundation, Inc.

   Written by:  Derek Zhou <derekzhou@gmail.com>
   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Date: 2006

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

/* You should have included GSStream.h before this */

#import "GSNetwork.h"

typedef	union {
  struct sockaddr	s;
  struct sockaddr_in	i4;
#ifdef	AF_INET6
  struct sockaddr_in	i6;
#endif
#ifndef	_WIN32
  struct sockaddr_un	u;
#endif
} sockaddr_any;

#define	SOCKIVARS \
{ \
  id            _sibling;       /* For bidirectional traffic.  	*/\
  BOOL          _passive;       /* YES means already connected. */\
  BOOL		_closing;	/* Must close on next failure.	*/\
  SOCKET        _sock;          /* Needed for ms-windows.       */\
  id            _handler;       /* TLS/SOCKS handler.           */\
  sockaddr_any	_address;	/* Socket address info.		*/\
}

/* The semi-abstract GSSocketStream class is not intended to be subclassed
 * but is used to add behaviors to other socket based classes.
 */
@interface GSSocketStream : GSStream
SOCKIVARS

/**
 * get the sockaddr
 */
- (struct sockaddr*) _address;

/**
 * set the sockaddr
 */
- (void) _setAddress: (struct sockaddr*)address;

/**
 * setter for closing flag ... the remote end has stopped either sending
 * or receiving, so any I/O operation which would block means that the
 * connection is no longer operable in that direction.
 */
- (void) _setClosing: (BOOL)passive;

/*
 * Set the handler for this stream.
 */
- (void) _setHandler: (id)h;

/**
 * setter for passive (the underlying socket connection is already open and
 * doesw not need to be re-opened).
 */
- (void) _setPassive: (BOOL)passive;

/**
 * setter for sibling
 */
- (void) _setSibling: (GSSocketStream*)sibling;

/*
 * Set the socket used for this stream.
 */
- (void) _setSock: (SOCKET)sock;

/*
 * Set the socket address from string information.
 */
- (BOOL) _setSocketAddress: (NSString*)address
                      port: (NSInteger)port
                    family: (NSInteger)family;

/* Return the socket
 */
- (SOCKET) _sock;

@end

/**
 * The abstract subclass of NSInputStream that reads from a socket.
 * It inherits from GSInputStream and adds behaviors from GSSocketStream
 * so it must have the same instance variable layout as GSSocketStream.
 */
@interface GSSocketInputStream : GSInputStream
SOCKIVARS
@end
@interface GSSocketInputStream (AddedBehaviors)
- (struct sockaddr*) _address;
- (void) _setAddress: (struct sockaddr*)address;
- (NSInteger) _read: (uint8_t *)buffer maxLength: (NSUInteger)len;
- (void) _setClosing: (BOOL)passive;
- (void) _setHandler: (id)h;
- (void) _setPassive: (BOOL)passive;
- (void) _setSibling: (GSSocketStream*)sibling;
- (void) _setSock: (SOCKET)sock;
- (BOOL) _setSocketAddress: (NSString*)address
                      port: (NSInteger)port
                    family: (NSInteger)family;
- (SOCKET) _sock;
@end

@interface GSInetInputStream : GSSocketInputStream

/**
 * the designated initializer
 */
- (id) initToAddr: (NSString*)addr port: (NSInteger)port;

@end


@interface GSInet6InputStream : GSSocketInputStream

/**
 * the designated initializer
 */
- (id) initToAddr: (NSString*)addr port: (NSInteger)port;

@end

/**
 * The abstract subclass of NSOutputStream that writes to a socket.
 * It inherits from GSOutputStream and adds behaviors from GSSocketStream
 * so it must have the same instance variable layout as GSSocketStream.
 */
@interface GSSocketOutputStream : GSOutputStream
SOCKIVARS
@end
@interface GSSocketOutputStream (AddedBehaviors)
- (struct sockaddr*) _address;
- (void) _setAddress: (struct sockaddr*)address;
- (void) _setClosing: (BOOL)passive;
- (void) _setHandler: (id)h;
- (void) _setPassive: (BOOL)passive;
- (void) _setSibling: (GSSocketStream*)sibling;
- (void) _setSock: (SOCKET)sock;
- (BOOL) _setSocketAddress: (NSString*)address
                      port: (NSInteger)port
                    family: (NSInteger)family;
- (SOCKET) _sock;
- (NSInteger) _write: (const uint8_t *)buffer maxLength: (NSUInteger)len;
@end

@interface GSInetOutputStream : GSSocketOutputStream

/**
 * the designated initializer
 */
- (id) initToAddr: (NSString*)addr port: (NSInteger)port;

@end

@interface GSInet6OutputStream : GSSocketOutputStream

/**
 * the designated initializer
 */
- (id) initToAddr: (NSString*)addr port: (NSInteger)port;

@end


/**
 * The subclass of NSStream that accepts connections from a socket.
 * It inherits from GSAbstractServerStream and adds behaviors from
 * GSSocketStream so it must have the same instance variable layout
 * as GSSocketStream.
 */
@interface GSSocketServerStream : GSAbstractServerStream
SOCKIVARS

/**
 * Return the class of the inputStream associated with this
 * type of serverStream.
 */
- (Class) _inputStreamClass;

/**
 * Return the class of the outputStream associated with this
 * type of serverStream.
 */
- (Class) _outputStreamClass;

@end
@interface GSSocketServerStream (AddedBehaviors)
- (struct sockaddr*) _address;
- (void) _setAddress: (struct sockaddr*)address;
- (void) _setClosing: (BOOL)passive;
- (void) _setHandler: (id)h;
- (void) _setPassive: (BOOL)passive;
- (void) _setSibling: (GSSocketStream*)sibling;
- (void) _setSock: (SOCKET)sock;
- (BOOL) _setSocketAddress: (NSString*)address
                      port: (NSInteger)port
                    family: (NSInteger)family;
- (SOCKET) _sock;
@end

@interface GSInetServerStream : GSSocketServerStream
@end

@interface GSInet6ServerStream : GSSocketServerStream
@end

#endif

