/* Concrete NSNetServiceBrowser subclass using the avahi API.
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

#import "GSNetServices.h"
#import "GSAvahiClient.h"
#import "Foundation/NSNetServices.h"
#import "GNUstepBase/NSNetServices+GNUstepBase.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSDebug.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSRunLoop.h"
#import "Foundation/NSString.h"
#import "GNUstepBase/GSObjCRuntime.h"
#include <avahi-client/lookup.h>

#define SERVICE_KEY(name, type, domain, iface, proto) [NSString stringWithFormat: @"%@.%@.%@%d:%d", name, type, domain, iface, proto]


// Private Methods:
@interface GSAvahiNetServiceBrowser (GSAvahiNetServiceBrowserPrivate)
- (void) stopWithError: (BOOL)yesno;
- (BOOL) hasFirstEvent;
- (void) setHasFirstEvent: (BOOL)yesno;
- (void) setService: (GSAvahiNetService*)service
             forKey: (NSString*)key;
- (void) removeServiceForKey: (NSString*)key;
- (GSAvahiNetService*) serviceForKey: (NSString*)key;

// GSAvahiClient behaviour methods:

- (id) avahiClientInitWithRunLoop: (NSRunLoop*)loop
                          forMode: (NSString*)mode;

- (void) avahiClientHandleError: (int)error;

- (void) avahiClientDealloc;

- (NSDictionary*) errorDictWithErrorCode: (int)error;

- (void) handleError: (int)error;

@end

NSString*
GSNetServiceDotTerminatedNSStringFromString(const char *domain)
{
  if (domain != NULL)
    {
      NSString *theDomain = [NSString stringWithUTF8String: domain];
      //Add a trailing dot if necessary:
      if ((unichar)'.'
	!= [theDomain characterAtIndex: ([theDomain length] - 1)])
        {
          theDomain = [theDomain stringByAppendingString: @"."];
        }
      return theDomain;
    }
  return nil;
}

static void
GSAvahiDomainBrowserEvent(AvahiDomainBrowser *db,
  AvahiIfIndex ifIndex,
  AvahiProtocol protocol,
  AvahiBrowserEvent event,
  const char *domain,
  AvahiLookupResultFlags flags,
  void *userInfo)
{
  GSAvahiNetServiceBrowser *browser = nil;
  NSString *theDomain = nil;

  if (NULL == db)
    {
      NSDebugLog(@"NULL pointer to AvahiDomainBrowser.");
      return;
    }
  if (NULL == userInfo)
    {
      NSDebugLog(@"NULL pointer to NSNetServiceBrowser.");
      return;
    }
  browser = (GSAvahiNetServiceBrowser*)userInfo;
  theDomain = GSNetServiceDotTerminatedNSStringFromString(domain);
  switch (event)
    {
      case AVAHI_BROWSER_NEW:
        if (![browser hasFirstEvent])
          {
            [browser netServiceBrowserWillSearch:
	      (NSNetServiceBrowser*)browser];
            [browser setHasFirstEvent: YES];
          }
        [browser netServiceBrowser: (NSNetServiceBrowser*)browser
                     didFindDomain: theDomain
                        moreComing: NO];
        break;

      case AVAHI_BROWSER_REMOVE:
        [browser netServiceBrowser: (NSNetServiceBrowser*)browser
                   didRemoveDomain: theDomain
                        moreComing: NO];
        break;

      case AVAHI_BROWSER_FAILURE:
        [browser handleError:
	  avahi_client_errno(avahi_domain_browser_get_client(db))];
        break;

      case AVAHI_BROWSER_CACHE_EXHAUSTED: // Not interesting
      case AVAHI_BROWSER_ALL_FOR_NOW:
	// This is useless, it always happens one second after firing the query.
        break;
    }
}

static void
GSAvahiServiceBrowserEvent(
  AvahiServiceBrowser *sb,
  AvahiIfIndex ifIndex,
  AvahiProtocol proto,
  AvahiBrowserEvent event,
  const char *name,
  const char *type,
  const char *domain,
  AvahiLookupResultFlags flags,
  void *userInfo)
{
  GSAvahiNetServiceBrowser* browser = (GSAvahiNetServiceBrowser*)userInfo;
  NSString *theName = NSStringIfNotNull(name);
  NSString *theType = NSStringIfNotNull(type);
  NSString *theDomain = GSNetServiceDotTerminatedNSStringFromString(domain);
  NSString *serviceKey;

  /* Some services might appear on different interface/protocol combinations
   * and we will get back one event for each of them, so we need a more
   * specific key to store the service:
   */
  serviceKey = SERVICE_KEY(theName, theType,theDomain, ifIndex, proto);

  if (NULL == sb)
    {
      NSDebugLog(@"NULL pointer to AvahiServiceBrowser.");
      return;
    }
  if (NULL == userInfo)
    {
      NSDebugLog(@"NULL pointer to NSNetServiceBrowser.");
      return;
    }

  if (event != AVAHI_BROWSER_FAILURE)
    {
      if (![browser hasFirstEvent])
        {
          [browser netServiceBrowserWillSearch: (NSNetServiceBrowser*)browser];
          [browser setHasFirstEvent: YES];
        }
     }

  switch (event)
    {
      case AVAHI_BROWSER_NEW:
        if ((theName && theDomain) && theType)
          {
            NSNetService *service = [[[GSAvahiNetService alloc]
	      initWithDomain: theDomain
	      type: theType
	      name: theName
	      avahiIfIndex: ifIndex
	      avahiProtocol: proto] autorelease];
            [browser setService: (GSAvahiNetService*)service
                         forKey: serviceKey];
            [browser netServiceBrowser: (NSNetServiceBrowser*)browser
                        didFindService: service
                            moreComing: NO];
          }
        break;

      case AVAHI_BROWSER_REMOVE:
        if ((theName && theDomain) && theType)
          {
            NSNetService *service;

            service = (NSNetService*)[browser serviceForKey: serviceKey];
            [browser netServiceBrowser: (NSNetServiceBrowser*)browser
                      didRemoveService: service
                            moreComing: NO];
            [browser removeServiceForKey: serviceKey];
          }
        break;

      case AVAHI_BROWSER_FAILURE:
        [browser handleError:
	  avahi_client_errno(avahi_service_browser_get_client(sb))];
        break;

      case AVAHI_BROWSER_CACHE_EXHAUSTED: // Not interesting
      case AVAHI_BROWSER_ALL_FOR_NOW:
	// This is useless, it always happens one second after firing the query.
        break;
    }
}

@implementation GSAvahiNetServiceBrowser

+ (void) initialize
{
  if (self == [GSAvahiNetServiceBrowser class])
    {
      GSObjCAddClassBehavior(self, [GSAvahiClient class]);
    }
}


- (id) initWithRunLoop: (NSRunLoop*)rl
               forMode: (NSString*)mode
{
  if (nil == (self = [self avahiClientInitWithRunLoop: rl
                                              forMode: mode]))
    {
      return nil;
    }
  _services = [[NSMutableDictionary alloc] init];
  return self;
}

- (id) init
{
  return [self initWithRunLoop: [NSRunLoop currentRunLoop]
                       forMode: NSDefaultRunLoopMode];
}

- (void) setService: (GSAvahiNetService*)service
             forKey: (NSString*)key
{
  [_services setObject: service
                forKey: key];
}

- (void) removeServiceForKey: (NSString*)key
{
  [_services removeObjectForKey: key];
}

- (GSAvahiNetService*) serviceForKey: (NSString*)key
{
  return [_services objectForKey: key];
}

- (void) handleError: (int)err
{
  [self netServiceBrowser: self
             didNotSearch: [self errorDictWithErrorCode: err]];
  [self stopWithError: YES];
  [self avahiClientHandleError: err];
}

- (BOOL) canSearch
{
  int err = 0;

  if ([self delegate] == nil)
    {
      err = NSNetServicesInvalidError;
    }
  else if (_browser != NULL)
    {
      err = NSNetServicesActivityInProgress;
    }

  if (!err)
    {
      /* Check whether the avahi-client is in a working state (the caller
       * might be trying to search again after a failure event).
       */
      switch (avahi_client_get_state((AvahiClient*)_client))
	{
	  case AVAHI_CLIENT_FAILURE:
	    err = avahi_client_errno((AvahiClient*)_client);
	    break;

	  case AVAHI_CLIENT_CONNECTING:
	  case AVAHI_CLIENT_S_REGISTERING:
	    err = NSNetServicesActivityInProgress;
	    break;

	  case AVAHI_CLIENT_S_COLLISION:
	    err = NSNetServicesCollisionError;
	    break;

	  default:
	    break;
	}
    }

  if (err)
    {
      [self netServiceBrowser: (NSNetServiceBrowser*)self
                 didNotSearch: [self errorDictWithErrorCode: err]];
      return NO;
    }

  {
    NSRunLoop *rl = [ctx runLoop];
    NSString *mode = [ctx mode];
    BOOL schedule = ((rl == nil) || (mode == nil));

    if (schedule)
      {
        if (rl == nil)
          {
            rl = [NSRunLoop currentRunLoop];
          }
        if (mode == nil)
          {
            mode = NSDefaultRunLoopMode;
          }
        [self scheduleInRunLoop: rl
                        forMode: mode];
      }
  }
  return YES;
}

- (void) searchForDomains: (AvahiDomainBrowserType)domainType
{
  [_lock lock];
  if ([self canSearch])
    {
      _browser = (void*)avahi_domain_browser_new((AvahiClient*)_client,
        AVAHI_IF_UNSPEC,
        AVAHI_PROTO_UNSPEC,
        "local",
        domainType,
        0,
        GSAvahiDomainBrowserEvent,
        (void*)self);
      if (NULL == _browser)
        {
          [self handleError: avahi_client_errno((AvahiClient*)_client)];
        }
      else
        {
          _type = GSAvahiDomainBrowser;
        }
      /* NOTE: -netServiceBrowserWillSearch: will be called from
       * GSAvahiDomainBrowserEvent().
       */
    }
  [_lock unlock];
}

- (void) searchForDefaultRegistrationDomain
{
  [self searchForDomains: AVAHI_DOMAIN_BROWSER_REGISTER_DEFAULT];
}

- (void) searchForDefaultBrowseDomain
{
  [self searchForDomains: AVAHI_DOMAIN_BROWSER_BROWSE_DEFAULT];
}

- (void) searchForAllDomains
{
  /* The dns-sd compatibility layer of avahi assumes the browsable domains
   * to be a superset of the registration domains, so we take an easy
   * shortcut here (also, Apple has deprecated it in Mac OS 10.4
   */
  [self searchForBrowsableDomains];
}

- (void) searchForBrowsableDomains
{
  [self searchForDomains: AVAHI_DOMAIN_BROWSER_BROWSE];
}

- (void) searchForRegistrationDomains
{
  [self searchForDomains: AVAHI_DOMAIN_BROWSER_REGISTER];
}

- (void) searchForServicesOfType: (NSString *)serviceType
                        inDomain: (NSString *)domainName
{
  const char* domain;

  if (![domainName isEqualToString: @""])
    {
      domain = [domainName UTF8String];
    }
  else
    {
      domain = NULL;
    }
  [_lock lock];
  if ([self canSearch])
    {
      _browser = (void*)avahi_service_browser_new((AvahiClient*)_client,
        AVAHI_IF_UNSPEC,
        AVAHI_PROTO_UNSPEC,
        [serviceType UTF8String],
        domain,
        0,
        GSAvahiServiceBrowserEvent,
        (void*)self);
      if (NULL == _browser)
        {
          [self handleError: avahi_client_errno((AvahiClient*)_client)];
        }
      else
        {
          _type = GSAvahiServiceBrowser;
        }
      /* NOTE: -netServiceBrowserWillSearch: will be called from
       * GSAvahiServiceBrowserEvent().
       */
    }
  [_lock unlock];
}

- (BOOL) hasFirstEvent
{
  return _hasFirstEvent;
}

- (void) setHasFirstEvent: (BOOL)yesno
{
  _hasFirstEvent = yesno;
}

- (void) stopWithError: (BOOL)hadError
{
  [_lock lock];
  if (_browser != NULL)
    {
      /* We can't recover if the type is wrong and we don't want to
       * leak random memory.
       */
      NSAssert((_type < GSAvahiBrowserMax) && (_type != GSAvahiUnknownBrowser),
	NSInternalInconsistencyException);
      [self setHasFirstEvent: NO];
      switch (_type)
        {
          case GSAvahiDomainBrowser:
            avahi_domain_browser_free((AvahiDomainBrowser*)_browser);
            break;

          case GSAvahiServiceBrowser:
            avahi_service_browser_free((AvahiServiceBrowser*)_browser);
            break;

          case GSAvahiUnknownBrowser:
          case GSAvahiBrowserMax:
            break;
        }
      _browser = NULL;
  }
  _type = GSAvahiUnknownBrowser;
  [_services removeAllObjects];
  
  if (!hadError)
   {
     [self netServiceBrowserDidStopSearch: self];
   }
  [_lock unlock];
}

- (void) stop
{
  [self stopWithError: NO];
}

- (void)dealloc
{
  if (_browser != NULL)
    {
      [self stop];
    }
  [self setDelegate: nil];
  [_services release];
  [self avahiClientDealloc];
  [super dealloc];
}

@end
