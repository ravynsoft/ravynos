/* Classes for integration of avahi-client into NSRunLoop.
   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by:  Niels Grewe <niels.grewe@halbordnung.de>
   Date: March 2010
   
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

#import "GSAvahiRunLoopIntegration.h"

#define CTX(x) GSAvahiRunLoopContext *ctx = (GSAvahiRunLoopContext*)x
@interface GSAvahiWatcher: NSObject <RunLoopEvents>
{
  //The callback to call for avahi.
  AvahiWatchCallback callback;
  BOOL callbackInProgress;
  AvahiWatchEvent oldEvents;
  AvahiWatchEvent lastEvent;
  int fileDesc;
  GSAvahiRunLoopContext *ctx;
  void *userData;
}
- (void)listenForEvents: (AvahiWatchEvent)events;
- (AvahiWatchEvent)getEvents;
- (void)removeFromContext;
- (void)setContext: (GSAvahiRunLoopContext*)aCtxt;
- (void)unschedule;
- (void)reschedule;
@end

@implementation GSAvahiWatcher
- (void) listenForEvents: (AvahiWatchEvent)events
               saveState: (BOOL)saveState
{
  /* FIXME: NSRunLoop doesn't expose equivalents for POLLERR and POLLHUP but
   * Avahi doesn't seem to strictly require them (their Qt API doesn't handle
   * them either). Still, it would be nice to handle AVAHI_WATCH_(ERR|HUP)
   * here.
   */

  // Remove old events:
  if (!(events & AVAHI_WATCH_IN)
    && (oldEvents & AVAHI_WATCH_IN)) 
    {
      [[ctx runLoop] removeEvent: (void*)(intptr_t)fileDesc
                            type: ET_RDESC
                         forMode: [ctx mode]
                             all: NO];
    }
  if (!(events & AVAHI_WATCH_OUT)
    && (oldEvents & AVAHI_WATCH_OUT)) 
    { 
      [[ctx runLoop] removeEvent: (void*)(intptr_t)fileDesc
                            type: ET_WDESC
                         forMode: [ctx mode]
                             all: NO];
    }
  
  // Remember event state:
  if (saveState)
    {
      oldEvents = events;
    }

  // Dispatch new events to the runLoop:
  if (events & AVAHI_WATCH_IN)
    {
      [[ctx runLoop] addEvent: (void*)(intptr_t)fileDesc
                         type: ET_RDESC
                      watcher: self
                      forMode: [ctx mode]];
    }
  if (events & AVAHI_WATCH_OUT)
    {
      [[ctx runLoop] addEvent: (void*)(intptr_t)fileDesc
                         type: ET_WDESC
                      watcher: self
                      forMode: [ctx mode]];
    }
}

- (void) listenForEvents: (AvahiWatchEvent)events
{
  [self listenForEvents: events
              saveState: YES];
}

- (void) unschedule
{
  /* Don't save the new event state (i.e. no events) if we are unscheduling
   * the watcher. We might want to reschedule it with the prior state.
   */
  [self listenForEvents: (AvahiWatchEvent)0
              saveState: NO];
}

- (void) reschedule
{
  [self listenForEvents: oldEvents
              saveState: NO];
}

- (id) initWithCallback: (AvahiWatchCallback)cback
             andContext: (GSAvahiRunLoopContext*)aCtx
                onEvent: (AvahiWatchEvent)someEvents
                  forFd: (int)fd
               userData: (void*)ud
{
  if (nil == (self = [super init]))
    {
      return nil;
    }
  fileDesc = fd;
  // The context retains its watchers and timers:
  ctx = aCtx;
  callback = cback;
  userData = ud;
  [self listenForEvents: someEvents];
  return self;
}

- (AvahiWatchEvent) getEvents
{
  if (callbackInProgress)
    {
      return (AvahiWatchEvent)0;
    }
  return lastEvent;
}

- (void) removeFromContext
{
  [self unschedule];
  [ctx removeWatcher: self];
  // Don't reference the context anymore, since it won't have any chance of
  // notifying us if it goes away.
  ctx = nil;
}

- (void) receivedEvent: (void*)data
                  type: (RunLoopEventType)type
                 extra: (void*)extra
               forMode: (NSString*)mode
{
  int fd = (int)(intptr_t)data;

  if (fileDesc != fd)
    {
      //Not good
      return;
    }
  
  /* FIXME ... in the following switch, as well as setting lastEvent the
   * code was clearing the corresponding bit in the oldEvents bitmask.
   * This was causng a crash becasue it meant that we didn't unregister
   * the event watcher from the run loop before deallocating it and a
   * new incoming event was sent to the deallocated instance.
   * I therefore removed that code, but can't see what it was intended
   * to do.
   */
  switch (type)
    {
      case ET_RDESC:
        lastEvent = AVAHI_WATCH_IN;
        break;
      case ET_WDESC:
        lastEvent = AVAHI_WATCH_OUT;
        break;
      default:
        return;
    }

  /* FIXME: NSRunLoop doesn't expose equivalents for POLLERR and POLLHUP but
   * Avahi doesn't seem to strictly require them (their Qt API doesn't handle
   * them either).
   */
  callbackInProgress = YES;
  callback((AvahiWatch*)self, fd, lastEvent, userData);
  callbackInProgress = NO;
}

- (void) setContext: (GSAvahiRunLoopContext*)aCtxt
{
  ctx = aCtxt;
}

- (void) dealloc
{
  // Remove all leftover event-handlers from the runLoop:
  [self listenForEvents: (AvahiWatchEvent)0];
  [super dealloc];
}
@end


@interface GSAvahiTimer: NSObject
{
  GSAvahiRunLoopContext *ctx;
  AvahiTimeoutCallback callback;
  NSTimer *timer;
  NSDate *fireDate;
  void *userData;
}
@end

@implementation GSAvahiTimer
- (void) didTimeout: (NSTimer*)timer
{
  callback((AvahiTimeout*)self, userData);
}

- (void) setTimerToInterval: (NSTimeInterval)interval
{
  // Invalidate the old timer;
  if (timer != nil)
    {
      [timer invalidate];
      timer = nil;
    }
 
  // NOTE: the timer ivar is a weak reference; runloops retain their
  // timers.
  timer = [NSTimer timerWithTimeInterval: interval
                                  target: self
                                selector: @selector(didTimeout:)
                                userInfo: nil
                                 repeats: NO];
  [[ctx runLoop] addTimer: timer
                  forMode: [ctx mode]];
}

- (void) setTimerToTimeval: (const struct timeval*)tv
{
  // Invalidate the old timer
  if (timer != nil)
    {
      [timer invalidate];
      timer = nil;
    }
 
  if (NULL != tv)
    {
      // Construct a NSTimeInterval for the timer:
      NSTimeInterval interval = (NSTimeInterval)tv->tv_sec;
      interval += (NSTimeInterval)(tv->tv_usec / 1000000.0);
      [self setTimerToInterval: interval];
    }
}
- (id) initWithCallback: (AvahiTimeoutCallback)aCallback
             andContext: (GSAvahiRunLoopContext*)aCtx
             forTimeval: (const struct timeval*)tv
               userData: (void*)ud
{
  if (nil == (self = [super init]))
    {
      return nil;
    }
  // The context retains its watchers and timeouts:
  ctx = aCtx;
  callback = aCallback;
  userData = ud;
  [self setTimerToTimeval: tv];
  return self;
}

- (void) unschedule
{
  if ([timer isValid])
    {
      fireDate = [[timer fireDate] retain];
      [timer invalidate];
      timer = nil;
    }
}

- (void) removeFromContext
{
  [self unschedule];
  [ctx removeTimeout: self];
  ctx = nil;
}

- (void) setContext: (GSAvahiRunLoopContext*)aCtxt
{
  ctx = aCtxt;
}

- (void) reschedule
{
  // Only reschedule if fireDate has been set, otherwise the Avahi layer will
  // schedule a new timer.
  if (nil != fireDate)
    {
      NSTimeInterval interval = [fireDate timeIntervalSinceNow];
      [self setTimerToInterval: MAX(0.1,interval)];
      [fireDate release];
      fireDate = nil;
    }
}

- (void) dealloc
{
  if (nil != timer)
    {
      [timer invalidate];
    }
  [super dealloc];
}
@end

static AvahiWatch*
GSAvahiWatchNew(const AvahiPoll *api, int fd, AvahiWatchEvent
  event, AvahiWatchCallback callback, void *userData)
{
  // NOTE: strangly enough, the userData parameter is not the userdata we
  // passed to the poll structure (it is somehow related to the dbus
  // internals).
  CTX(api->userdata);
  GSAvahiWatcher *w = [ctx avahiWatcherWithCallback: callback
                                            onEvent: event
                                  forFileDescriptor: fd
                                           userData: userData];
  // NOTE: avahi defines AvahiWatch as a struct, since we only pass around
  // pointers to those, we can just cast the pointer to our watcher object to
  // AvahiWatch*.
  return (AvahiWatch*)w;
}

static void
GSAvahiWatchUpdate(AvahiWatch *watch, AvahiWatchEvent event)
{
  [(GSAvahiWatcher*)watch listenForEvents: event];
}

static AvahiWatchEvent
GSAvahiWatchGetEvents(AvahiWatch *watch)
{
  return [(GSAvahiWatcher*)watch getEvents];
}

static void
GSAvahiWatchFree(AvahiWatch *watch)
{
  [(GSAvahiWatcher*)watch removeFromContext];
}

static AvahiTimeout*
GSAvahiTimeoutNew(const AvahiPoll *api,
  const struct timeval *tv, AvahiTimeoutCallback callback, void *userData)
{
  // NOTE: strangly enough, the userData parameter is not the userdata we
  // passed to the poll structure (it is somehow related to the dbus
  // internals.)
  CTX(api->userdata);
  GSAvahiTimer *t = [ctx avahiTimerWithCallback: callback
                                    withTimeval: tv
                                       userData: userData];
  // NOTE: Cf. GSAvahiWatchNew().
  return (AvahiTimeout*)t;
}

static void
GSAvahiTimeoutUpdate(AvahiTimeout* timeout,
  const struct timeval *tv)
{
  [(GSAvahiTimer*)timeout setTimerToTimeval: tv];
}

static void
GSAvahiTimeoutFree(AvahiTimeout* timeout)
{
  [(GSAvahiTimer*)timeout removeFromContext];
}

@implementation GSAvahiRunLoopContext
- (id) initWithRunLoop: (NSRunLoop*)rl
               forMode: (NSString*)aMode
{
  if (nil == (self = [super init]))
    {
      return nil;
    }
  lock = [[NSLock alloc] init];
  [lock setName: @"GSAvahiRunLoopContextLock"];
  poll = malloc(sizeof(AvahiPoll));
  NSAssert(poll, @"Could not allocate avahi polling structure.");
  poll->userdata = (void*)self; //userInfo
  poll->watch_new = GSAvahiWatchNew; //create a new GSAvahiWatcher
  poll->watch_update = GSAvahiWatchUpdate; //update the watcher
  poll->watch_get_events = GSAvahiWatchGetEvents; //retrieve events
  poll->watch_free = GSAvahiWatchFree; //remove watcher from context
  poll->timeout_new = GSAvahiTimeoutNew; //create a new GSAvahiTimer
  poll->timeout_update = GSAvahiTimeoutUpdate; //update the timer
  poll->timeout_free = GSAvahiTimeoutFree; //remove the timer from context
  //Runloops don't need to be retained;
  runLoop = rl;
  ASSIGNCOPY(mode,aMode);
  children = [[NSMutableArray alloc] init];
  return self;
}

- (NSRunLoop*) runLoop
{
  // NOTE: We don't protect this with the lock because it will only ever be
  // changed by -removeFromRunLoop:forMode: or -scheduleInRunLoop:forMode:,
  // which is where we do the locking.
  return runLoop;
}

- (NSString*) mode
{
  /* NOTE: We don't protect this with the lock because it will only ever be
   * changed by -removeFromRunLoop:forMode: or -scheduleInRunLoop:forMode:,
   * which is where we do the locking.
   */
  return mode;
}

- (const AvahiPoll*) avahiPoll
{
  return (const AvahiPoll*)poll;
}

- (GSAvahiTimer*) avahiTimerWithCallback: (AvahiTimeoutCallback)callback
                             withTimeval: (const struct timeval*)tv
                                userData: (void*)ud
{
  GSAvahiTimer *timer = nil;
  [lock lock];
  timer = [[[GSAvahiTimer alloc] initWithCallback: callback
                                       andContext: self
                                       forTimeval: tv
                                         userData: ud] autorelease];
  if (nil != timer)
    {
      [children addObject: timer];
    }
  [lock unlock];
  return timer;
}

- (GSAvahiWatcher*) avahiWatcherWithCallback: (AvahiWatchCallback)callback
                                     onEvent: (AvahiWatchEvent)someEvents
                           forFileDescriptor: (NSInteger)fd
                                    userData: (void*)ud
{
  GSAvahiWatcher *w = nil;

  [lock lock];
  w = [[[GSAvahiWatcher alloc] initWithCallback: callback
                                     andContext: self
                                        onEvent: someEvents
                                          forFd: fd
                                       userData: ud] autorelease];
  
  if (nil != w)
    {
      [children addObject: w];
    }
  [lock unlock];
  return w;
}

- (void) removeChild: (id)c
{
  if (nil != c)
    {
      [lock lock];
      [children removeObject: c];
      [lock unlock];
    }
}

- (void) removeWatcher: (GSAvahiWatcher*)w
{
  [self removeChild: w];
}

- (void) removeTimeout: (GSAvahiTimer*)at
{
  [self removeChild: at];
}

- (void) removeFromRunLoop: (NSRunLoop*)rl
                   forMode: (NSString*)m
{
  [lock lock];
  if ((rl == runLoop) && [mode isEqualToString: m])
    {
      FOR_IN(GSAvahiWatcher*, child, children)
        {
          [child unschedule];
        }
      END_FOR_IN(children)
      runLoop = nil;
      [mode release];
      mode = nil;
    }
  [lock unlock];
}

- (void) scheduleInRunLoop: (NSRunLoop*)rl
                   forMode: (NSString*)m
{
  [lock lock];
  if ((runLoop == nil) && (mode == nil)
    && ((rl != nil) && (m != nil)))
    {
      runLoop = rl;
      ASSIGNCOPY(mode,m);
      FOR_IN(GSAvahiWatcher*, child, children)
        {
          [child reschedule];
        }
      END_FOR_IN(children)
    }
  [lock unlock];
}

- (void) dealloc
{
  /* Some avahi internals might still reference the poll structure and could
   * try to create additional watchers and timers on the runloop, so we should
   * clean it up properly:
   */
  poll->userdata = (void*)NULL;
  [self removeFromRunLoop: runLoop
                  forMode: mode];
  FOR_IN(GSAvahiWatcher*, child, children)
    {
      [child setContext: nil];
    }
  END_FOR_IN(children)
  free(poll);
  poll = NULL;
  [children release];
  [mode release];
  [lock release];
  [super dealloc];
}
@end
