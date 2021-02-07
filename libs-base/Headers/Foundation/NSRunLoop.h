/* Interface for NSRunLoop for GNUStep
   Copyright (C) 1996 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Created: March 1996

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

#ifndef __NSRunLoop_h_GNUSTEP_BASE_INCLUDE
#define __NSRunLoop_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSMapTable.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSTimer, NSDate, NSPort;

typedef NSString* NSRunLoopMode;
  
/**
 * Run loop mode used to deal with input sources other than NSConnections or
 * dialog windows.  Most commonly used. Defined in
 * <code>Foundation/NSRunLoop.h</code>.
 */
GS_EXPORT NSRunLoopMode const NSDefaultRunLoopMode;
GS_EXPORT NSRunLoopMode const NSRunLoopCommonModes;

GS_EXPORT_CLASS
@interface NSRunLoop : NSObject
{
#if	GS_EXPOSE(NSRunLoop)
  @private
  NSString		*_currentMode;
  NSMapTable		*_contextMap;
  NSMutableArray	*_contextStack;
  NSMutableArray	*_timedPerformers;
  void			*_extra;
#endif
}

/**
 * Returns the run loop instance for the current thread.
 */
+ (NSRunLoop*) currentRunLoop;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5,GS_API_LATEST)
/**
 * Returns the run loop instance of the main thread.
 */
+ (NSRunLoop*) mainRunLoop;
#endif

- (void) acceptInputForMode: (NSString*)mode
                 beforeDate: (NSDate*)limit_date;

- (void) addTimer: (NSTimer*)timer
	  forMode: (NSString*)mode;

- (NSString*) currentMode;

- (NSDate*) limitDateForMode: (NSString*)mode;

- (void) run;

/**
 * Calls -limitDateForMode: to determine if a timeout occurs before the
 * specified date, then calls -acceptInputForMode:beforeDate: to run the
 * loop once.<br />
 * The specified date may be nil ... in which case the loop runs
 * until the limit date of the first input or timeout.<br />
 * If the specified date is in the past, this runs the loop once only,
 * to handle any events already available.<br />
 * If there are no input sources or timers in mode, this method
 * returns NO without running the loop (irrespective of the supplied
 * date argument), otherwise returns YES.
 */
- (BOOL) runMode: (NSString*)mode
      beforeDate: (NSDate*)date;

- (void) runUntilDate: (NSDate*)date;

@end

@interface NSRunLoop(OPENSTEP)

- (void) addPort: (NSPort*)port
         forMode: (NSString*)mode;

- (void) cancelPerformSelectorsWithTarget: (id)target;

- (void) cancelPerformSelector: (SEL)aSelector
			target: (id)target
		      argument: (id)argument;

- (void) configureAsServer;

- (void) performSelector: (SEL)aSelector
		  target: (id)target
		argument: (id)argument
		   order: (NSUInteger)order
		   modes: (NSArray*)modes;

- (void) removePort: (NSPort*)port
            forMode: (NSString*)mode;

@end

/** This type specifies the kinds of event which may be 'watched' in a
 * run loop.
 */
typedef	enum {
#ifdef _WIN32
    ET_HANDLE,	/* Watch for an I/O event on a handle.		*/
    ET_RPORT,	/* Watch for message arriving on port.		*/
    ET_WINMSG,	/* Watch for a message on a window handle.	*/
    ET_TRIGGER	/* Trigger immediately when the loop runs.	*/
#else
    ET_RDESC,	/* Watch for descriptor becoming readable.	*/
    ET_WDESC,	/* Watch for descriptor becoming writeable.	*/
    ET_RPORT,	/* Watch for message arriving on port.		*/
    ET_EDESC,	/* Watch for descriptor with out-of-band data.	*/
    ET_TRIGGER	/* Trigger immediately when the loop runs.	*/
#endif
} RunLoopEventType;

/** This protocol defines the mandatory interface a run loop watcher must
 * provide in order for it to be notified of events occurring in the loop
 * it is watching.<br />
 * Optional methods are documented in the NSObject(RunLoopEvents)
 * category.
 */
@protocol RunLoopEvents
/** This is the message sent back to a watcher when an event is observed
 * by the run loop.<br />
 * The 'data', 'type' and 'mode' arguments are the same as the arguments
 * passed to the -addEvent:type:watcher:forMode: method.<br />
 * The 'extra' argument varies.  For an ET_TRIGGER event, it is the same
 * as the 'data' argument.  For other events on unix it is the file
 * descriptor associated with the event (which may be the same as the
 * 'data' argument, but is not in the case of ET_RPORT).<br />
 * For windows it will be the handle or the windows message assciated
 * with the event.
 */ 
- (void) receivedEvent: (void*)data
		  type: (RunLoopEventType)type
		 extra: (void*)extra
	       forMode: (NSString*)mode;
@end

/** This informal protocol defiens optional methods of the run loop watcher.
 */
@interface NSObject (RunLoopEvents)
/** Called by the run loop to find out whether it needs to block to wait
 * for events for this watcher.  The shouldTrigger flag is used to inform
 * the run loop if tit should immediately trigger a received event for the
 * watcher.
 */
- (BOOL) runLoopShouldBlock: (BOOL*)shouldTrigger;
@end

/**
 * The run loop watcher API was originally intended to perform two
 * tasks ...
 * 1. provide the most efficient API reasonably possible to integrate
 * unix networking code into the runloop.
 * 2. provide a standard mechanism to allow people to contribute
 * code to add new I/O mechanisms to GNUstep (OpenStep didn't allow this).
 * It succeeded in 1, and partially succeeded in 2 (adding support
 * for the win32 API).
 */
@interface NSRunLoop(GNUstepExtensions)
/** Adds a watcher to the receiver ... the watcher is used to monitor events
 * of the specified type which are associted with the event handle data and
 * it operates in the specified run loop modes.<br />
 * The watcher remains in place until a corresponding call to
 * -removeEvent:type:forMode:all: is made.
 */
- (void) addEvent: (void*)data
	     type: (RunLoopEventType)type
	  watcher: (id<RunLoopEvents>)watcher
	  forMode: (NSString*)mode;
/** Removes a watcher from the receiver ... the watcher must have been 
 * previously added using -addEvent:type:watcher:forMode:<br />
 * This method mirrors exactly one addition of a watcher unless removeAll
 * is YES, in which case it removes all additions of watchers matching the
 * other paramters.
 */
- (void) removeEvent: (void*)data
	        type: (RunLoopEventType)type
	     forMode: (NSString*)mode
		 all: (BOOL)removeAll;
@end

#if	defined(__cplusplus)
}
#endif

#endif /*__NSRunLoop_h_GNUSTEP_BASE_INCLUDE */
