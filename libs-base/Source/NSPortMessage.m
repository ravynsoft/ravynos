/** Implementation of NSPortMessage for GNUstep
   Copyright (C) 1998,2000 Free Software Foundation, Inc.

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

   <title>NSPortMessage class reference</title>
   $Date$ $Revision$
   */

#import "common.h"
#define	EXPOSE_NSPortMessage_IVARS	1
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSData.h"
#import "Foundation/NSException.h"
#import "Foundation/NSPortMessage.h"

@implementation	NSPortMessage

- (void) dealloc
{
  RELEASE(_recv);
  RELEASE(_send);
  RELEASE(_components);
  [super dealloc];
}

- (NSString*) description
{
  return [NSString stringWithFormat: @"NSPortMessage 0x%"PRIxPTR
    @" (Id %u)\n  Send: %@\n  Recv: %@\n  Components -\n%@",
    (NSUInteger)self, _msgid, _send, _recv, _components];
}

/*	PortMessages MUST be initialised with ports and data.	*/
- (id) init
{
  [self shouldNotImplement: _cmd];
  return nil;
}

/*	PortMessages MUST be initialised with ports and data.	*/
- (id) initWithMachMessage: (void*)buffer
{
  [self shouldNotImplement: _cmd];
  return nil;
}

/*	This is the designated initialiser.	*/
- (id) initWithSendPort: (NSPort*)aPort
	    receivePort: (NSPort*)anotherPort
	     components: (NSArray*)items
{
  self = [super init];
  if (self != nil)
    {
      _msgid = 0;
      _send = RETAIN(aPort);
      _recv = RETAIN(anotherPort);
      _components = [[NSMutableArray allocWithZone: [self zone]]
				     initWithArray: items];
    }
  return self;
}

- (NSArray*) components
{
  return AUTORELEASE([_components copy]);
}

- (unsigned) msgid
{
  return _msgid;
}

- (NSPort*) receivePort
{
  return _recv;
}

- (BOOL) sendBeforeDate: (NSDate*)when
{
  return [_send sendBeforeDate: when
			 msgid: _msgid
		    components: _components
			  from: _recv
		      reserved: 0];
}

- (NSPort*) sendPort
{
  return _send;
}

- (void) setMsgid: (unsigned)anId
{
  _msgid = anId;
}
@end

@implementation	NSPortMessage (Private)
- (NSMutableArray*) _components
{
  return _components;
}
@end

