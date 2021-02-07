/* Interface for integration of avahi-client into NSRunLoop.
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

#import "Foundation/NSObject.h"
#import "Foundation/NSRunLoop.h"
#import "Foundation/NSTimer.h"
#import "Foundation/NSString.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSException.h"
#import "Foundation/NSLock.h"
#import "GSFastEnumeration.h"
#include <avahi-common/watch.h>

@class GSAvahiWatcher,GSAvahiTimer;

/**
 * The GSAvahiRunLoopContext provides hooks for Avahi to hook into NSRunLoop.
 * This provides relatively hassle-free event handling of all GSAvahiClient
 * subclasses.
 */
@interface GSAvahiRunLoopContext: NSObject
{
  NSRunLoop *runLoop;
  NSString *mode;
  AvahiPoll *poll;
  NSMutableArray *children;
  NSLock *lock;
}
- (id)initWithRunLoop: (NSRunLoop*)runLoop
              forMode: (NSString*)runLoopMode;
- (NSRunLoop*)runLoop;
- (NSString*)mode;
- (GSAvahiWatcher*)avahiWatcherWithCallback: (AvahiWatchCallback)callback
                                    onEvent: (AvahiWatchEvent)someEvents
                          forFileDescriptor: (NSInteger)fd
                                   userData: (void*)userData;
- (GSAvahiTimer*)avahiTimerWithCallback: (AvahiTimeoutCallback)callback
                            withTimeval: (const struct timeval*)tv
                               userData: (void*)userData;
- (void)removeWatcher: (GSAvahiWatcher*)aw;
- (void)removeTimeout: (GSAvahiTimer*)at;
- (void)removeFromRunLoop: (NSRunLoop*)rl
                  forMode: (NSString*)mode;
- (void)scheduleInRunLoop: (NSRunLoop*)rl
                  forMode: (NSString*)mode;
- (const AvahiPoll*)avahiPoll;
@end
