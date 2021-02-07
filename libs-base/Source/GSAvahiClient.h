/* Interface for avahi-client behaviour used by NSNetServices.
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

@interface GSAvahiClient: NSObject
{
  id _delegate;
  void *_unused;
  void *_reserved;
  GSAvahiRunLoopContext *ctx;
  void *_client;
  NSRecursiveLock *_lock;
}


+ (int) netServicesErrorForAvahiError: (int)errNo;


- (void*) client;

- (id) initWithRunLoop: (NSRunLoop*)rl
               forMode: (NSString*)mode;

- (void) setupClientWithFlags: (int)flags
               andReportError: (int*)error;

- (void) setState: (int)theState
        forClient: (void*)cl;

- (void) scheduleInRunLoop: (NSRunLoop*)rl
                   forMode: (NSString*)mode;

- (void) removeFromRunLoop: (NSRunLoop*)rl
                   forMode: (NSString*)mode;

- (void) handleError: (int)errNo;

- (void) setDelegate: (id)delegate;

- (id) delegate;

- (NSDictionary*) errorDictWithErrorCode: (int) error;
@end
