#ifndef __GSRunLoopWatcher_h_GNUSTEP_BASE_INCLUDE
#define __GSRunLoopWatcher_h_GNUSTEP_BASE_INCLUDE
/** 
   Copyright (C) 2008-2009 Free Software Foundation, Inc.

   By: Richard Frith-Macdonald <richard@brainstorm.co.uk>

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

   $Date$ $Revision$
*/

/*
 *	The 'GSRunLoopWatcher' class was written to permit the (relatively)
 *	easy addition of new events to be watched for in the runloop.
 *
 *	To add a new type of event, the 'RunLoopEventType' enumeration must be
 *	extended, and the methods must be modified to handle the new type.
 *
 *	The internal variables if the GSRunLoopWatcher are used as follows -
 *
 *	If '_invalidated' is set, the watcher should be disabled and should
 *	be removed from the runloop when next encountered.
 *
 *	If 'checkBlocking' is set, the run loop should ask the watcher
 *	whether it should block and/or trigger each loop iteration.
 *
 *	The 'data' variable is used to identify the  resource/event that the
 *	watcher is interested in.  Its meaning is system dependent.
 *
 *	The 'receiver' is the object which should be told when the event
 *	occurs.  This object is retained so that we know it will continue
 *	to exist and can handle a callback.
 *
 *	The 'type' variable indentifies the type of event watched for.
 *	NSRunLoops [-acceptInputForMode: beforeDate: ] method MUST contain
 *	code to watch for events of each type.
 *
 *	NB.  This class is private to NSRunLoop and must not be subclassed.
 */

#import "common.h"
#import "Foundation/NSRunLoop.h"

@class NSDate;

@interface GSRunLoopWatcher: NSObject
{
@public
  BOOL			_invalidated;
  BOOL			checkBlocking;
  void			*data;
  id			receiver;
  RunLoopEventType	type;
  unsigned 		count;
}
- (id) initWithType: (RunLoopEventType)type
	   receiver: (id)anObj
	       data: (void*)data;
/**
 * Returns a boolean indicating whether the receiver needs the loop to
 * block to wait for input, or whether the loop can run through at once.
 * It also sets *trigger to say whether the receiver should be triggered
 * once the input test has been done or not.
 */
- (BOOL) runLoopShouldBlock: (BOOL*)trigger;
@end

#endif /* __GSRunLoopWatcher_h_GNUSTEP_BASE_INCLUDE */
