/* Classes for avahi-client behaviour used by NSNetServices.
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

#import "GSAvahiClient.h"
#import "Foundation/NSNetServices.h"
#import "Foundation/NSDebug.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSValue.h"

#include <avahi-client/client.h>
#include <avahi-common/error.h>

static void
GSAvahiClientState(AvahiClient *client, AvahiClientState state, void *userInfo)
{
  [(GSAvahiClient*)userInfo setState: state
			   forClient: client];
}

@implementation GSAvahiClient

/**
 *
 *
 */
+ (int)netServicesErrorForAvahiError: (int)errorCode
{
  switch(errorCode)
    {
      case AVAHI_ERR_INVALID_HOST_NAME:
      case AVAHI_ERR_INVALID_DOMAIN_NAME:
      case AVAHI_ERR_INVALID_TTL:
      case AVAHI_ERR_IS_PATTERN:
      case AVAHI_ERR_INVALID_RECORD:
      case AVAHI_ERR_INVALID_SERVICE_NAME:
      case AVAHI_ERR_INVALID_SERVICE_TYPE:
      case AVAHI_ERR_INVALID_PORT:
      case AVAHI_ERR_INVALID_KEY:
      case AVAHI_ERR_INVALID_ADDRESS:
      case AVAHI_ERR_INVALID_SERVICE_SUBTYPE:
      case AVAHI_ERR_BAD_STATE:
      case AVAHI_ERR_INVALID_FLAGS:
	return NSNetServicesBadArgumentError;
      case AVAHI_ERR_TIMEOUT:
	return NSNetServicesTimeoutError;
      case AVAHI_ERR_NO_NETWORK:
      case AVAHI_ERR_OS:
      case AVAHI_ERR_INVALID_CONFIG:
      case AVAHI_ERR_DBUS_ERROR:
      case AVAHI_ERR_DISCONNECTED:
      case AVAHI_ERR_NO_DAEMON:
      case AVAHI_ERR_NO_MEMORY:
      case AVAHI_ERR_INVALID_INTERFACE:
      case AVAHI_ERR_INVALID_PROTOCOL:
      case AVAHI_ERR_TOO_MANY_CLIENTS:
      case AVAHI_ERR_TOO_MANY_OBJECTS:
      case AVAHI_ERR_TOO_MANY_ENTRIES:
      case AVAHI_ERR_ACCESS_DENIED:
	return NSNetServicesUnknownError;
      case AVAHI_ERR_NOT_FOUND:
	return NSNetServicesNotFoundError;
      case AVAHI_ERR_INVALID_OPERATION:
      case AVAHI_ERR_INVALID_OBJECT:
      case AVAHI_ERR_VERSION_MISMATCH:
	return NSNetServicesInvalidError;
      case AVAHI_ERR_COLLISION:
	return NSNetServicesCollisionError;
    }
  return NSNetServicesUnknownError;
}


- (id) avahiClientInitWithRunLoop: (NSRunLoop*)rl
                          forMode: (NSString*)mode
{
  int avahiError = 0;

  if ( nil == (self = [super init]))
    {
      return nil;
    }
  _lock = [[NSRecursiveLock alloc] init];
  ctx = [[GSAvahiRunLoopContext alloc] initWithRunLoop: rl
					       forMode: mode];

  // NOTE: Not setting the AVAHI_CLIENT_NO_FAIL flag mimicks the behaviour of
  // the DNS-SD compatability code. We will set it subsequently if the avahi
  // server goes away while we are running.
  [self setupClientWithFlags: 0
	      andReportError: &avahiError];
  if (avahiError)
    {
      [self handleError: avahiError];
    }
  return self;
}

- (id) initWithRunLoop: (NSRunLoop*)rl
               forMode: (NSString*)mode
{
  self = [self avahiClientInitWithRunLoop: rl
				  forMode: mode];
  return self;
}

- (id) avahiClientInit
{
  return [self avahiClientInitWithRunLoop: [NSRunLoop currentRunLoop]
				  forMode: NSDefaultRunLoopMode];
}

- (id) init
{
  self = [self avahiClientInit];
  return self;
}

- (void*) client
{
  return _client;
}

- (void) freeClient
{
  if (_client != NULL)
    {
      [_lock lock];
      if (_client != NULL)
	{
	  avahi_client_free((AvahiClient*)_client);
	  _client = NULL;
	}
      [_lock unlock];
    }
}

- (void) setupClientWithFlags: (int)flags
               andReportError: (int*)errNo
{
  if (errNo != NULL)
    {
      *errNo = 0;
    }
  if (_client == NULL)
    {
      [_lock lock];
      if (_client == NULL)
	{
	  _client = (void*)avahi_client_new([ctx avahiPoll],
	    (AvahiClientFlags)flags,
	    GSAvahiClientState,
	    (void*)self,
	    errNo);
	}
      [_lock unlock];
    }
}

- (id) delegate
{
  return _delegate;
}

- (void) setState: (int)theState
        forClient: (void*)cl
{
  if (cl == _client)
    {		
      NSLog(@"Client entered state: %d",theState);
      if (theState == AVAHI_CLIENT_FAILURE)
	{
	  [self handleError: avahi_client_errno((AvahiClient*)cl)];
	}
    }
}

- (void) scheduleInRunLoop: (NSRunLoop*)rl
                   forMode: (NSString*)mode
{
  [ctx scheduleInRunLoop: rl forMode: mode];
}

- (void) removeFromRunLoop: (NSRunLoop*)rl
                   forMode: (NSString*)mode
{
  [ctx removeFromRunLoop: rl forMode: mode];
}

- (void) setDelegate: (id)aDelegate
{
  _delegate = aDelegate;
}


- (void) avahiClientHandleError: (int)errNo
{
  AvahiClientState state = 0;
  NSDebugLog(@"Error: %s", avahi_strerror(errNo));
  // Try to reconnect (in case the server did simply restart.)
  if (_client != NULL)
    {
      state = avahi_client_get_state((AvahiClient*)_client);
    }
  else
    {
      return;
    }

  if (state == AVAHI_CLIENT_FAILURE)
    {
      [_lock lock];
      if (_client != NULL)
	{
	  state = avahi_client_get_state((AvahiClient*)_client);
	}
      else
	{
	  [_lock unlock];
	  return;
	}
      if (state == AVAHI_CLIENT_FAILURE)
	{
	  /* If the daemon failed, we need to remove timers and watchers
	   * from the runloop, because the dbus internals underlying the
	   * avahi-client api will have changed for the new connection to
	   * the avahi-daemon:
	   */
	  NSRunLoop *rl = [ctx runLoop];
	  NSString *mode = [[ctx mode] retain];

	  [ctx removeFromRunLoop: rl
			 forMode: mode];
	  [self freeClient];
	  [ctx scheduleInRunLoop: rl
			 forMode: mode];
	  [mode release];
	  [self setupClientWithFlags: AVAHI_CLIENT_NO_FAIL
		      andReportError: NULL];
	}
      [_lock unlock];
    }
}

- (void) handleError: (int)errNo
{
  [self avahiClientHandleError: errNo];
}

- (NSDictionary*) errorDictWithErrorCode: (int)errorCode
{
  NSMutableDictionary	*dictionary = nil;
  int			error = 0;
   
  dictionary = [NSMutableDictionary dictionary];
  error = [[self class] netServicesErrorForAvahiError: errorCode];
  
  [dictionary setObject: [NSNumber numberWithInt: error]
                 forKey: NSNetServicesErrorCode];
  [dictionary setObject: self
                 forKey: NSNetServicesErrorDomain];
  return dictionary; // autorelease'd
}

- (void) avahiClientDealloc
{
  [ctx removeFromRunLoop: [ctx runLoop] forMode: [ctx mode]];
  [self freeClient];
  [ctx release];
  [_lock release];
}

- (void) dealloc
{
  [self avahiClientDealloc];
  [super dealloc];
}

@end
