/* NSPortMessage interface for GNUstep
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

#ifndef __NSPortMessage_h_GNUSTEP_BASE_INCLUDE
#define __NSPortMessage_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSArray.h>
#import	<Foundation/NSPort.h>

#if	defined(__cplusplus)
extern "C" {
#endif

/**
 * <p>The data transported for distributed objects communications is sent over
 * the network encapsulated by NSPortMessage objects, which consist of two
 * [NSPort]s (sender and receiver, not sent over the network) and a body
 * consisting of one or more [NSData] or [NSPort] objects. (Data in the
 * [NSData] must be in network byte order.)</p>
 *
 * <p>See the [NSConnection] and [NSPortCoder] classes.</p>
 */
@interface	NSPortMessage : NSObject
{
#if	GS_EXPOSE(NSPortMessage)
@private
  unsigned		_msgid;
  NSPort		*_recv;
  NSPort		*_send;
  NSMutableArray	*_components;
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
/**
 * OpenStep compatibility.
 */
- (id) initWithMachMessage: (void*)buffer;

/** <init/> Initializes to send message described by items (which should
 * contain only [NSPort] and/or [NSData] objects, with contents in network
 * byte order) over aPort.  If/when a reply to the message is sent, it will
 * arrive on anotherPort.
 */
- (id) initWithSendPort: (NSPort*)aPort
	    receivePort: (NSPort*)anotherPort
	     components: (NSArray*)items;

/**
 * Request that the message be sent before when.  Will block until either
 * sends it (returns YES) or when expires (returns NO).  The latter may occur
 * if many messages are queued up (by multiple threads) faster than they can
 * be sent over the network.
 */
- (BOOL) sendBeforeDate: (NSDate*)when;

/**
 * Returns the message components originally used to constitute this message.
 */
- (NSArray*) components;

/**
 * For an outgoing message, returns the port the receiver will send itself
 * through.  For an incoming message, returns the port replies to the receiver
 * should be sent through.
 */
- (NSPort*) sendPort;

/**
 * For an outgoing message, returns the port on which a reply to this message
 * will arrive.  For an incoming message, returns the port this message
 * arrived on.
 */
- (NSPort*) receivePort;

/**
 * Sets ID for message.  This is not used by the distributed objects system,
 * but may be used in custom ways by cooperating applications to sort or
 * otherwise organize messages.
 */
- (void) setMsgid: (unsigned)anId;

/**
 * Returns ID for message.  This is not used by the distributed objects
 * system, but may be used in custom ways by cooperating applications to sort
 * or otherwise organize messages.  Set to 0 initially.
 */
- (unsigned) msgid;
@end

#if	defined(__cplusplus)
}
#endif

#endif

