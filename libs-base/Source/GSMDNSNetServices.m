/* Implementation for NSNetServices for GNUstep
   Copyright (C) 2006 Free Software Foundation, Inc.

   Written by:  Chris B. Vetter
   Date: 2006
   
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

#import "common.h"
#import "GSNetServices.h"
#import "Foundation/NSNetServices.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSData.h"
#import "Foundation/NSNull.h"
#import "Foundation/NSRunLoop.h"
#import "Foundation/NSTimer.h"
#import "Foundation/NSValue.h"
#if defined(_REENTRANT)
#import "GNUstepBase/GSLock.h"
#endif

#import <dns_sd.h>		// Apple's DNS Service Discovery

#import "GSNetwork.h"

//
// Define
//

#if ! defined(INET6_ADDRSTRLEN)
#  define INET6_ADDRSTRLEN	46
#endif

// trigger runloop timer every INTERVAL seconds
#define INTERVAL		0.3
#define SHORTTIMEOUT		0.25

// debugging stuff and laziness on my part
#if defined(VERBOSE)
#  define INTERNALTRACE	NSDebugLLog(@"Trace", @"%s", __PRETTY_FUNCTION__)
#  define LOG(f, args...)	NSDebugLLog(@"NSNetServices", f, ##args)
#else
#  define INTERNALTRACE
#  define LOG(f, args...)
#endif /* VERBOSE */

#if ! defined(VERSION)
#  define VERSION (((GNUSTEP_BASE_MAJOR_VERSION * 100)			\
		  + GNUSTEP_BASE_MINOR_VERSION) * 100)			\
		  + GNUSTEP_BASE_SUBMINOR_VERSION
#endif

#define SETVERSION(aClass)						\
        do {								\
          if (self == [aClass class]) { [self setVersion: VERSION]; }	\
          else { [self doesNotRecognizeSelector: _cmd]; }		\
        } while (0);

#if defined(_REENTRANT)
#  define THE_LOCK		NSRecursiveLock	*lock
#  define CREATELOCK(x)		x->lock = [NSRecursiveLock new]
#  define LOCK(x)		[x->lock lock]
#  define UNLOCK(x)		[x->lock unlock]
#  define DESTROYLOCK(x)	DESTROY(x->lock)
#else
#  define THE_LOCK		/* nothing */
#  define CREATELOCK(x)		/* nothing */
#  define LOCK(x)		/* nothing */
#  define UNLOCK(x)		/* nothing */
#  define DESTROYLOCK(x)	/* nothing */
#endif

//
// Typedef
//

typedef struct _Browser		// The actual NSNetServiceBrowser
{
  THE_LOCK;
  
  NSRunLoop		*runloop;
  NSString		*runloopmode;
  NSTimer		*timer;			// to control the runloop
  
  NSMutableDictionary	*services;
    // List of found services.
    // Key is <_name_type_domain> and value is an initialized NSNetService.
  
  int			 interfaceIndex;
} Browser;

typedef struct _Service		// The actual NSNetService
{
  THE_LOCK;
  
  NSRunLoop		*runloop;
  NSString		*runloopmode;
  NSTimer		*timer,			// to control the runloop
                        *timeout;		// to time-out the resolve
  
  NSMutableDictionary	*info;
    // The service's information, keys are
    // - Domain (string)
    // - Name (string)
    // - Type (string)
    // - Host (string)
    // - Addresses (mutable array)
    // - TXT (data)
  
  NSMutableArray	*foundAddresses;	// array of char*
  
  int			 interfaceIndex,	// should also be in 'info'
                         port;			// (in network byte-order) ditto
  
  id			 monitor;		// NSNetServiceMonitor
  
  BOOL			 isPublishing,		// true if publishing service
                         isMonitoring;		// true if monitoring
} Service;

typedef struct _Monitor		// The actual NSNetServiceMonitor
{
  THE_LOCK;
  
  NSRunLoop		*runloop;
  NSString		*runloopmode;
  NSTimer		*timer;			// to control the runloop
} Monitor;


//
// Private
//

//
// Private Interface
//

@interface GSMDNSNetServiceMonitor : NSObject
{
  @private
  void		* _netServiceMonitor;
  id		  _delegate;
  void		* _reserved;
}

- (id) initWithDelegate: (id) delegate;

- (void) removeFromRunLoop: (NSRunLoop *) aRunLoop
                   forMode: (NSString *) mode;
- (void) scheduleInRunLoop: (NSRunLoop *) aRunLoop
                   forMode: (NSString *) mode;

- (void) start;
- (void) stop;

@end

//
// Prototype
//

static NSDictionary *CreateError(id sender, int errorCode);

static int ConvertError(int errorCode);

static void DNSSD_API
  // used by NSNetServiceBrowser
  EnumerationCallback(DNSServiceRef		 sdRef,
                      DNSServiceFlags		 flags,
                      uint32_t			 interfaceIndex,
                      DNSServiceErrorType	 errorCode,
                      const char		*replyDomain,
                      void			*context);

static void DNSSD_API
  BrowserCallback(DNSServiceRef			 sdRef,
                  DNSServiceFlags		 flags,
                  uint32_t			 interfaceIndex,
                  DNSServiceErrorType		 errorCode,
                  const char			*replyName,
                  const char			*replyType,
                  const char			*replyDomain,
                  void				*context);

static void DNSSD_API
  // used by NSNetService
  ResolverCallback(DNSServiceRef		 sdRef,
                   DNSServiceFlags		 flags,
                   uint32_t			 interfaceIndex,
                   DNSServiceErrorType		 errorCode,
                   const char			*fullname,
                   const char			*hosttarget,
                   uint16_t			 port,
                   uint16_t			 txtLen,
                   const unsigned char		*txtRecord,
                   void				*context);

static void DNSSD_API
  RegistrationCallback(DNSServiceRef		 sdRef,
                       DNSServiceFlags		 flags,
                       DNSServiceErrorType	 errorCode,
                       const char		*name,
                       const char		*regtype,
                       const char		*domain,
                       void			*context);

static void DNSSD_API
  // used by NSNetService and NSNetServiceMonitor
  QueryCallback(DNSServiceRef			 sdRef,
                DNSServiceFlags			 flags,
                uint32_t			 interfaceIndex,
                DNSServiceErrorType		 errorCode,
                const char			*fullname,
                uint16_t			 rrtype,
                uint16_t			 rrclass,
                uint16_t			 rdlen,
                const void			*rdata,
                uint32_t			 ttl,
                void				*context);

/***************************************************************************
**
** Implementation
**
*/

@implementation GSMDNSNetServiceBrowser

/**
 * <em>Description forthcoming</em>
 *
 *
 */

+ (void) initialize
{
  INTERNALTRACE;
  
  SETVERSION(GSMDNSNetServiceBrowser);
  {
#ifndef _REENTRANT
    LOG(@"%@ may NOT be thread-safe!", [self class]);
#endif
  }
}

/***************************************************************************
**
** Private Methods
**
*/

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) cleanup
{
  Browser	*browser;
  
  INTERNALTRACE;
  
  browser = (Browser *) _reserved;
  
  LOCK(browser);
  {
    if (browser->runloop)
      {
	[self removeFromRunLoop: browser->runloop
			forMode: browser->runloopmode];
      }
    
    if (browser->timer)
      {
	[browser->timer invalidate];
	DESTROY(browser->timer);
      }
    
    if (_netServiceBrowser)
      {
	DNSServiceRefDeallocate(_netServiceBrowser);
	_netServiceBrowser = NULL;
      }
    
    [browser->services removeAllObjects];
  }
  UNLOCK(browser);
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) executeWithError: (DNSServiceErrorType) err
{
  Browser	*browser;
  
  INTERNALTRACE;
  
  browser = (Browser *) _reserved;
  
  LOCK(browser);
  {
    if (kDNSServiceErr_NoError == err)
      {
	[self netServiceBrowserWillSearch: self];
	
	if (! browser->runloop)
	  {
	    [self scheduleInRunLoop: [NSRunLoop currentRunLoop]
			    forMode: NSDefaultRunLoopMode];
	  }
	
	[browser->runloop addTimer: browser->timer
			   forMode: browser->runloopmode];
	
	[browser->timer fire];
      }
    else // notify the delegate of the error
      {
	[self netServiceBrowser: self
		   didNotSearch: CreateError(self, err)];
      }
  }
  UNLOCK(browser);
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) searchForDomain: (NSInteger) aFlag
{
  DNSServiceErrorType	err = kDNSServiceErr_NoError;
  Browser		*browser;
  
  INTERNALTRACE;
  
  browser = (Browser *) _reserved;
  
  LOCK(browser);
  {
    do
      {
	if (! [self delegate])
	  {
	    err = NSNetServicesInvalidError;
	    break;
	  }
	
	if (browser->timer)
	  {
	    err = NSNetServicesActivityInProgress;
	    break;
	  }
	
	err = DNSServiceEnumerateDomains((DNSServiceRef *)&_netServiceBrowser,
	  aFlag,
	  browser->interfaceIndex,
	  EnumerationCallback,
	  self);
      }
    while (0);
  }
  UNLOCK(browser);
  
  [self executeWithError: err];
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) enumCallback: (DNSServiceRef) sdRef
                flags: (DNSServiceFlags) flags
            interface: (uint32_t) interfaceIndex
                error: (DNSServiceErrorType) errorCode
               domain: (const char *) replyDomain
{
  Browser	*browser;
  
  INTERNALTRACE;
  
  browser = (Browser *) _reserved;
  
  LOCK(browser);
  
  if (_netServiceBrowser)
    {
      if (errorCode)
	{
	  [self cleanup];
	  
	  [self netServiceBrowser: self
		     didNotSearch: CreateError(self, errorCode)];
	}
      else
	{  
	  BOOL	more = NO;
	  
	  if (replyDomain)
	    {
	      NSString	*domain;

	      more = flags & kDNSServiceFlagsMoreComing;
	      
	      browser->interfaceIndex = interfaceIndex;
	      
	      domain = [NSString stringWithUTF8String: replyDomain];

	      if (flags & kDNSServiceFlagsAdd)
		{
		  LOG(@"Found domain <%s>", replyDomain);
		  
		  [self netServiceBrowser: self
			    didFindDomain: domain
			       moreComing: more];
		}
	      else // kDNSServiceFlagsRemove
		{
		  LOG(@"Removed domain <%s>", replyDomain);
		  
		  [self netServiceBrowser: self
			  didRemoveDomain: domain
			       moreComing: more];
		}
	    }
	}
    }
  UNLOCK(browser);
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) browseCallback: (DNSServiceRef) sdRef
                  flags: (DNSServiceFlags) flags
              interface: (uint32_t) interfaceIndex
                  error: (DNSServiceErrorType) errorCode
                   name: (const char *) replyName
                   type: (const char *) replyType
                 domain: (const char *) replyDomain
{
  Browser	*browser;
  
  INTERNALTRACE;
  
  browser = (Browser *) _reserved;
  
  LOCK(browser);
  
  if (_netServiceBrowser)
    {
      if (errorCode)
	{
	  [self cleanup];
		
	  [self netServiceBrowser: self
		     didNotSearch: CreateError(self, errorCode)];
	}
      else
	{
	  NSNetService	*service = nil;
	  NSString	*domain = nil;
	  NSString	*type = nil;
	  NSString	*name = nil;
	  NSString	*key = nil;
	  BOOL		more = (flags & kDNSServiceFlagsMoreComing);
	  
	  browser->interfaceIndex = interfaceIndex;
	  
	  if (nil == browser->services)
	    {
	      browser->services
		= [[NSMutableDictionary alloc] initWithCapacity: 1];
	    }
	
	  domain = [NSString stringWithUTF8String: replyDomain];
	  type = [NSString stringWithUTF8String: replyType];
	  name = [NSString stringWithUTF8String: replyName];
	  
	  key = [NSString stringWithFormat: @"%@%@%@", name, type, domain];
	  
	  if (flags & kDNSServiceFlagsAdd)
	    {
	      service = AUTORELEASE([[GSMDNSNetService alloc]
                initWithDomain: domain type: type name: name]);
	      
	      if (service)
		{
		  LOG(@"Found service <%s>", replyName);
		  
		  [self netServiceBrowser: self
			   didFindService: service
			       moreComing: more];
		  
		  [browser->services setObject: service
					forKey: key];
		}
	      else
		{
		  LOG(@"WARNING: Could not create an NSNetService for <%s>",
		    replyName);
		}
	    }
	  else // kDNSServiceFlagsRemove
	    {
	      service = [browser->services objectForKey: key];
	      
	      if (service)
		{
		  LOG(@"Removed service <%@>", [service name]);
		  
		  [self netServiceBrowser: self
			 didRemoveService: service
			       moreComing: more];
		}
	      else
		{
		  LOG(@"WARNING: Could not find <%@> in list", key);
		}
	    }
	}
    }
  UNLOCK(browser);
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) loop: (id) sender
{
  int			sock = 0;
  struct timeval	tout = { 0 };
  fd_set		set;
  DNSServiceErrorType	err = kDNSServiceErr_NoError;
  
  sock = DNSServiceRefSockFD(_netServiceBrowser);
  
  if (-1 != sock)
    {
      FD_ZERO(&set);
      FD_SET(sock, &set);
      
      if (1 == select(sock + 1, &set, (fd_set *) NULL, (fd_set *) NULL, &tout))
	{
	  err = DNSServiceProcessResult(_netServiceBrowser);
	}
    }
  
  if (kDNSServiceErr_NoError != err)
    {
      [self netServiceBrowser: self
		 didNotSearch: CreateError(self, err)];
    }
}

/**
 * Removes the receiver from the specified runloop.
 *
 *
 */
- (void) removeFromRunLoop: (NSRunLoop *) aRunLoop
                   forMode: (NSString *) mode
{
  Browser	*browser;
  
  INTERNALTRACE;
  
  browser = (Browser *) _reserved;
  
  LOCK(browser);
  {
    if (browser->timer)
      {
	[browser->timer setFireDate: [NSDate date]];
	[browser->timer invalidate];
	browser->timer = nil;
      }
    
    // Do not release the runloop!
    browser->runloop = nil;
    
    DESTROY(browser->runloopmode);
  }
  UNLOCK(browser);
}

/**
 * Adds the receiver to the specified runloop.
 *
 *
 */

- (void) scheduleInRunLoop: (NSRunLoop *) aRunLoop
                   forMode: (NSString *) mode
{
  Browser	*browser;
  
  INTERNALTRACE;
  
  browser = (Browser *) _reserved;
  
  LOCK(browser);
  {
    if (browser->timer)
      {
	[browser->timer setFireDate: [NSDate date]];
	[browser->timer invalidate];
	browser->timer = nil;
      }
    
    browser->timer = RETAIN([NSTimer timerWithTimeInterval: INTERVAL
                                                    target: self
                                                  selector: @selector(loop:)
                                                  userInfo: nil
                                                   repeats: YES]);
    
    browser->runloop = aRunLoop;
    browser->runloopmode = mode;
  }
  UNLOCK(browser);
}

/**
 * Search for all visible domains. This method is deprecated.
 *
 *
 */

- (void) searchForAllDomains
{
  DNSServiceFlags	flags = 0;
  
  INTERNALTRACE;
  
  flags = kDNSServiceFlagsBrowseDomains|kDNSServiceFlagsRegistrationDomains;
  [self searchForDomain: flags];
}

/**
 * Search for all browsable domains.
 *
 *
 */

- (void) searchForBrowsableDomains
{
  INTERNALTRACE;
  
  [self searchForDomain: kDNSServiceFlagsBrowseDomains];
}

/**
 * Search for all registration domains. These domains can be used to register
 * a service.
 *
 */

- (void) searchForRegistrationDomains
{
  INTERNALTRACE;
  
  [self searchForDomain: kDNSServiceFlagsRegistrationDomains];
}

/**
 * Search for a particular service within a given domain.
 *
 *
 */

- (void) searchForServicesOfType: (NSString *) serviceType
                        inDomain: (NSString *) domainName
{
  Browser		*browser;
  DNSServiceErrorType	err = kDNSServiceErr_NoError;
  DNSServiceFlags	flags = 0;
  
  INTERNALTRACE;
  
  browser = (Browser *) _reserved;
  
  LOCK(browser);
  {
    do
      {
	if (! [self delegate])
	  {
	    err = NSNetServicesInvalidError;
	    break;
	  }
	
	if (browser->timer)
	  {
	    err = NSNetServicesActivityInProgress;
	    break;
	  }
	
	err = DNSServiceBrowse((DNSServiceRef *) &_netServiceBrowser,
	  flags,
	  browser->interfaceIndex,
	  [serviceType UTF8String],
	  [domainName UTF8String],
	  BrowserCallback,
	  self);
      }
    while (0);
  }
  UNLOCK(browser);
  
  [self executeWithError: err];
}

/**
 * Halts all currently running searches.
 *
 *
 */

- (void) stop
{
  Browser	*browser;
  
  INTERNALTRACE;
  
  browser = (Browser *) _reserved;
  
  LOCK(browser);
  {
    [self cleanup];
    
    [self netServiceBrowserDidStopSearch: self];
  }
  UNLOCK(browser);
}


/** <init />
 * Initializes the receiver.
 *
 *
 */

- (id) init
{
  INTERNALTRACE;
  
  if ((self = [super init]))
    {
      Browser	*browser;
      
      browser = malloc(sizeof (struct _Browser));
      memset(browser, 0, sizeof &browser);
      
      CREATELOCK(browser);
      
      browser->runloop = nil;
      browser->runloopmode = nil;
      browser->timer = nil;
      
      browser->services = [[NSMutableDictionary alloc] initWithCapacity: 1];
      
      browser->interfaceIndex = 0;
      
      _netServiceBrowser = NULL;
      [self setDelegate: nil];
      _reserved = browser;
    }
  return self;
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) dealloc
{
  Browser	*browser;
  
  INTERNALTRACE;
  
  browser = (Browser *) _reserved;
  {
    LOCK(browser);
    {
      [self cleanup];
      
      DESTROY(browser->services);
      
      [self setDelegate: nil];
    }
    UNLOCK(browser);
    
    DESTROYLOCK(browser);
    
    free(browser);
  }
  [super dealloc];
}

@end

@implementation GSMDNSNetService

/**
 * <em>Description forthcoming</em>
 *
 *
 */

+ (void) initialize
{
  INTERNALTRACE;
  
  SETVERSION(GSMDNSNetService);
  {
#ifndef _REENTRANT
    LOG(@"%@ may NOT be thread-safe!", [self class]);
#endif
  }
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) executeWithError: (DNSServiceErrorType) err
{
  Service	*service;
  
  INTERNALTRACE;
  
  service = (Service *) _reserved;
  
  LOCK(service);
  {
    if (kDNSServiceErr_NoError == err)
      {
	if (YES == service->isPublishing)
	  {
	    [self netServiceWillPublish: self];
	  }
	else
	  {
	    [self netServiceWillResolve: self];
	  }
	
	if (! service->runloop)
	  {
	    [self scheduleInRunLoop: [NSRunLoop currentRunLoop]
			    forMode: NSDefaultRunLoopMode];
	  }
	
	[service->runloop addTimer: service->timer
			   forMode: service->runloopmode];
	
	[service->timer fire];
      }
    else // notify the delegate of the error
      {
	if (YES == service->isPublishing)
	  {
	    [self netService: self
	       didNotPublish: CreateError(self, err)];
	  }
	else
	  {
	    [self netService: self
	       didNotResolve: CreateError(self, err)];
	  }
      }
  }
  UNLOCK(service);
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) cleanup
{
  Service	*service;
  
  INTERNALTRACE;
  
  service = (Service *) _reserved;
  
  LOCK(service);
  {
    if (service->runloop)
      {
	[self removeFromRunLoop: service->runloop
			forMode: service->runloopmode];
      }
    
    if (service->timer)
      {
	[service->timer invalidate];
	DESTROY(service->timer);
      }
    
    if (_netService)
      {
	DNSServiceRefDeallocate(_netService);
	_netService = NULL;
      }
    
    [service->info removeAllObjects];
    [service->foundAddresses removeAllObjects];
  }
  UNLOCK(service);
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) stopResolving: (id) sender
{
  Service	*service;
  
  INTERNALTRACE;
  
  service = (Service *) _reserved;
  
  LOCK(service);
  {
    [service->timeout invalidate];
    [service->timer invalidate];
    
    [self netService: self
       didNotResolve: CreateError(self, NSNetServicesTimeoutError)];
  }
  UNLOCK(service);
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) resolverCallback: (DNSServiceRef) sdRef
                    flags: (DNSServiceFlags) flags
                interface: (uint32_t) interfaceIndex
                    error: (DNSServiceErrorType) errorCode
                 fullname: (const char *) fullname
                   target: (const char *) hosttarget
                     port: (uint16_t) port
                   length: (uint16_t) txtLen
                   record: (const unsigned char *) txtRecord
{
  Service	*service;
  
  INTERNALTRACE;
  
  service = (Service *) _reserved;
  
  LOCK(service);
  
  if (_netService)
    {
      if (errorCode)
	{
	  [self cleanup];
	  
	  [self netService: self
	     didNotResolve: CreateError(self, errorCode)];
	}
      else
	{
	  NSData	*txt = nil;
	  NSString	*target = nil;
	  
	  // Add the TXT record
	  txt = txtRecord
	    ? [NSData dataWithBytes: txtRecord length: txtLen]
	    : nil;
	  
	  // Get the host
	  target = hosttarget
	    ? [NSString stringWithUTF8String: hosttarget]
	    : nil;
	  
	  // Add the port
	  service->port = port;
	  
	  // Remove the old TXT entry
	  [service->info removeObjectForKey: @"TXT"];
	  
	  if (txt)
	    {
	      [service->info setObject: txt forKey: @"TXT"];
	    }
	  
	  // Remove the old host entry
	  [service->info removeObjectForKey: @"Host"];
	  
	  // Add the host if there is one
	  if (target)
	    {
	      [service->info setObject: target forKey: @"Host"];
	    }
	  
	  /* Add the interface so all subsequent
	   * queries are on the same interface
	   */
	  service->interfaceIndex = interfaceIndex;
	  
	  service->timer = nil;
	  
	  // Prepare query for A and/or AAAA record
	  errorCode = DNSServiceQueryRecord((DNSServiceRef *) &_netService,
	    flags,
	    interfaceIndex,
	    hosttarget,
	    kDNSServiceType_ANY,
	    kDNSServiceClass_IN,
	    QueryCallback,
	    self);
	  
	  // No error? Then create a new timer
	  if (kDNSServiceErr_NoError == errorCode)
	    {
	      service->timer = [NSTimer timerWithTimeInterval: INTERVAL
						       target: self
						     selector: @selector(loop:)
						     userInfo: nil
						      repeats: YES];
	      [service->timer fire];
	    }
	}
    }
  UNLOCK(service);
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (BOOL) addAddress: (char *) addressString
{
  Service	*service;
  NSString	*string;
  
  INTERNALTRACE;
  
  service = (Service *) _reserved;
  
  if (nil == service->foundAddresses)
    {
      service->foundAddresses = [[NSMutableArray alloc] init];
    }
  
  string = [NSString stringWithCString: addressString];
  if ([service->foundAddresses containsObject: string])
    {
      // duplicate, didn't add it
      return NO;
    }
  
  [service->foundAddresses addObject: string];

  return YES;
}


/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) addAddress: (const void *) rdata
             length: (uint16_t) rdlen
               type: (uint16_t) rrtype
          interface: (uint32_t) interfaceIndex
{
  Service	*service;
  
  INTERNALTRACE;
  
  service = (Service *) _reserved;
  
  LOCK(service);
  {
    NSData		*data = nil;
    NSMutableArray	*addresses = nil;
    struct sockaddr	*address = { 0 };
    size_t		length = 0;
    const unsigned char	*rd = rdata;
    char		rdb[INET6_ADDRSTRLEN];
    
    memset(rdb, 0, sizeof rdb);
    
    addresses = [service->info objectForKey: @"Addresses"];
    if (nil == addresses)
      {
	addresses = [NSMutableArray arrayWithCapacity: 1];
      }
    else
      {
        addresses = AUTORELEASE([addresses mutableCopy]);
      }
    
    switch(rrtype)
      {
	case kDNSServiceType_A:		// AF_INET
	  {
	    struct sockaddr_in	ip4;
	    
	    // oogly
	    snprintf(rdb, sizeof(rdb),
	      "%d.%d.%d.%d", rd[0], rd[1], rd[2], rd[3]);
	    LOG(@"Found IPv4 <%s> on port %d", rdb, ntohs(service->port));
	    
	    length = sizeof (struct sockaddr_in);
	    memset(&ip4, 0, length);
	    
	    inet_pton(AF_INET, rdb, &ip4.sin_addr);
	    ip4.sin_family = AF_INET;
	    ip4.sin_port = service->port;
	    
	    address = (struct sockaddr *) &ip4;
	  }
	  break;
	
  #if defined(AF_INET6)
	case kDNSServiceType_AAAA:	// AF_INET6
	case kDNSServiceType_A6:		// deprecates AAAA
	  {
	    struct sockaddr_in6	ip6;
	    
	    // Even more oogly
	    snprintf(rdb, sizeof(rdb),
	      "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x",
	      rd[0], rd[1], rd[2], rd[3],
	      rd[4], rd[5], rd[6], rd[7],
	      rd[8], rd[9], rd[10], rd[11],
	      rd[12], rd[13], rd[14], rd[15]);
	    LOG(@"Found IPv6 <%s> on port %d", rdb, ntohs(service->port));
	    
	    length = sizeof (struct sockaddr_in6);
	    memset(&ip6, 0, length);
	    
	    inet_pton(AF_INET6, rdb, &ip6.sin6_addr);
#if defined(HAVE_SA_LEN)
	    ip6.sin6_len = sizeof ip6;
#endif
	    ip6.sin6_family = AF_INET6;
	    ip6.sin6_port = service->port;
	    ip6.sin6_flowinfo = 0;
	    ip6.sin6_scope_id = interfaceIndex;
	    
	    address = (struct sockaddr *) &ip6;
	  }
	  break;
#endif /* AF_INET6 */      
	
	default:
	  LOG(@"Unkown type of length <%d>", rdlen);
	  break;
      }
    
    // check for duplicate entries
    if ([self addAddress: rdb])
      {
	// add it
	data = [NSData dataWithBytes: address
			      length: length];
	
	[addresses addObject: data];
	[service->info setObject: AUTORELEASE([addresses copy])
			  forKey: @"Addresses"];
	
	// notify the delegate
	[self netServiceDidResolveAddress: self];
	
	// got it, so invalidate the timeout
	[service->timeout invalidate];
	service->timeout = nil;
      }
  }
  UNLOCK(service);
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) queryCallback: (DNSServiceRef) sdRef
                 flags: (DNSServiceFlags) flags
             interface: (uint32_t) interfaceIndex
                 error: (DNSServiceErrorType) errorCode
              fullname: (const char *) fullname
                  type: (uint16_t) rrtype
                 class: (uint16_t) rrclass
                length: (uint16_t) rdlen
                  data: (const void *) rdata
                   ttl: (uint32_t) ttl
{
  Service	*service;
  
  INTERNALTRACE;
  
  service = (Service *) _reserved;
  
  LOCK(service);
  
  if (_netService)
    {
      if (errorCode)
	{
	  [self cleanup];
	  
	  [self netService: self
	     didNotResolve: CreateError(self, errorCode)];
	  
	  UNLOCK(service);
	  
	  return;
	}
      
      switch(rrtype)
	{
	  case kDNSServiceType_A:		// 1 -- AF_INET
	    [self addAddress: rdata
		      length: rdlen
			type: rrtype
		   interface: interfaceIndex];
	    break;
	  
	  case kDNSServiceType_NS:
	  case kDNSServiceType_MD:
	  case kDNSServiceType_MF:
	  case kDNSServiceType_CNAME:	// 5
	  case kDNSServiceType_SOA:
	  case kDNSServiceType_MB:
	  case kDNSServiceType_MG:
	  case kDNSServiceType_MR:
	  case kDNSServiceType_NULL:	// 10
	  case kDNSServiceType_WKS:
	  case kDNSServiceType_PTR:
	  case kDNSServiceType_HINFO:
	  case kDNSServiceType_MINFO:
	  case kDNSServiceType_MX:		// 15
	    // not handled (yet)
	    break;
	  
	  case kDNSServiceType_TXT:
	    {
	      NSData
		*data = nil;
	      
	      data = [NSData dataWithBytes: rdata
				    length: rdlen];
	      
	      [service->info removeObjectForKey: @"TXT"];
	      [service->info setObject: data
				forKey: @"TXT"];
	      
	      [self        netService: self
	       didUpdateTXTRecordData: data];
	    
	    }
	    break;
	  
	  case kDNSServiceType_RP:
	  case kDNSServiceType_AFSDB:
	  case kDNSServiceType_X25:
	  case kDNSServiceType_ISDN:	// 20
	  case kDNSServiceType_RT:
	  case kDNSServiceType_NSAP:
	  case kDNSServiceType_NSAP_PTR:
	  case kDNSServiceType_SIG:
	  case kDNSServiceType_KEY:		// 25
	  case kDNSServiceType_PX:
	  case kDNSServiceType_GPOS:
	    // not handled (yet)
	    break;
	  
	  case kDNSServiceType_AAAA:	// 28 -- AF_INET6
	    [self addAddress: rdata
		      length: rdlen
			type: rrtype
		   interface: interfaceIndex];
	    break;
	  
	  case kDNSServiceType_LOC:
	  case kDNSServiceType_NXT:		// 30
	  case kDNSServiceType_EID:
	  case kDNSServiceType_NIMLOC:
	  case kDNSServiceType_SRV:
	  case kDNSServiceType_ATMA:
	  case kDNSServiceType_NAPTR:	// 35
	  case kDNSServiceType_KX:
	  case kDNSServiceType_CERT:
	    // not handled (yet)
	    break;
	  
	  case kDNSServiceType_A6:	// 38 -- AF_INET6, deprecates AAAA
	    [self addAddress: rdata
		      length: rdlen
			type: rrtype
		   interface: interfaceIndex];
	    break;
	  
	  case kDNSServiceType_DNAME:
	  case kDNSServiceType_SINK:	// 40
	  case kDNSServiceType_OPT:
	    // not handled (yet)
	    break;
	  
	  case kDNSServiceType_TKEY:	// 249
	  case kDNSServiceType_TSIG:	// 250
	  case kDNSServiceType_IXFR:
	  case kDNSServiceType_AXFR:
	  case kDNSServiceType_MAILB:
	  case kDNSServiceType_MAILA:
	    // not handled (yet)
	    break;
	  
	  case kDNSServiceType_ANY:
	    LOG(@"Oops, got the wildcard match...");
	    break;
	  
	  default:
	    LOG(@"Don't know how to handle rrtype <%d>", rrtype);
	    break;
	}
    }
  UNLOCK(service);
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) registerCallback: (DNSServiceRef) sdRef
                    flags: (DNSServiceFlags) flags
                    error: (DNSServiceErrorType) errorCode
                     name: (const char *) name
                     type: (const char *) regtype
                   domain: (const char *) domain
{
  Service	*service;
  
  INTERNALTRACE;
  
  service = (Service *) _reserved;
  
  LOCK(service);
  
  if (_netService)
    {
      if (errorCode)
	{
	  [self cleanup];
	  
	  [self netService: self
	     didNotPublish: CreateError(self, errorCode)];
	}
      else
	{
	  [self netServiceDidPublish: self];
	}
    }
  UNLOCK(service);
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) loop: (id) sender
{
  int			sock = 0;
  struct timeval	tout = { 0 };
  fd_set		set;
  DNSServiceErrorType	err = kDNSServiceErr_NoError;
  
  sock = DNSServiceRefSockFD(_netService);
  
  if (-1 != sock)
    {
      FD_ZERO(&set);
      FD_SET(sock, &set);
      
      if (1 == select(sock + 1, &set, (fd_set *) NULL, (fd_set *) NULL, &tout))
	{
	  err = DNSServiceProcessResult(_netService);
	}
    }
  
  if (kDNSServiceErr_NoError != err)
    {
      Service	*service;
      
      service = (Service *) _reserved;
      
      if (YES == service->isPublishing)
	{
	  [self netService: self
	     didNotPublish: CreateError(self, err)];
	}
      else
	{
	  [self netService: self
	     didNotResolve: CreateError(self, err)];
	}
    }
}

/**
 * Converts txtDictionary into a TXT data.
 *
 *
 */

+ (NSData *) dataFromTXTRecordDictionary: (NSDictionary *) txtDictionary
{
  NSMutableData	*result = nil;
  NSArray	*keys = nil;
  NSArray	*values = nil;
  int		count = 0;
  
  INTERNALTRACE;
  
  count = [txtDictionary count];
  
  if (count)
    {
      keys = [txtDictionary allKeys];
      values = [txtDictionary allValues];
      
      if (keys && values)
	{
	  TXTRecordRef	txt;
	  int		i = 0;
	  char		key[256];
	  
	  TXTRecordCreate(&txt, 0, NULL);
	  
	  for (; i < count; i++)
	    {
	      int			length = 0;
	      int			used = 0;
	      DNSServiceErrorType err = kDNSServiceErr_Unknown;
	      
	      if (! [[keys objectAtIndex: i] isKindOfClass: [NSString class]])
		{
		  LOG(@"%@ is not a string", [keys objectAtIndex: i]);
		  break;
		}
	      
	      length = [[keys objectAtIndex: i] length];
	      [[keys objectAtIndex: i] getCString: key
					maxLength: sizeof key];
	      used = strlen(key);
	      
	      if (! length || (used >= sizeof key))
		{
		  LOG(@"incorrect length %d - %d - %d",
		    length, used, sizeof key);
		  break;
		}
	      
	      if ([[values objectAtIndex: i] isKindOfClass: [NSString class]])
		{
		  char	value[256];
		  
		  length = [[values objectAtIndex: i] length];
		  [[values objectAtIndex: i] getCString: value
					      maxLength: sizeof value];
		  used = strlen(value);
		  
		  if (used >= sizeof value)
		    {
		      LOG(@"incorrect length %d - %d - %d",
			length, used, sizeof value);
		      break;
		    }
		  
		  err = TXTRecordSetValue(&txt,
		    (const char *) key,
		    used,
		    value);
		}
	      else if ([[values objectAtIndex: i] isKindOfClass: [NSData class]]
		&& [[values objectAtIndex: i] length] < 256
		&& [[values objectAtIndex: i] length] > 0)
		{
		  err = TXTRecordSetValue(&txt,
		    (const char *) key,
		    [[values objectAtIndex: i] length],
		    [[values objectAtIndex: i] bytes]);
		}
	      else if ([values objectAtIndex: i] == [NSNull null])
		{
		  err = TXTRecordSetValue(&txt,
		    (const char *) key,
		    0,
		    NULL);
		}
	      else
		{
		  LOG(@"unknown value type");
		  break;
		}
	      
	      if (err != kDNSServiceErr_NoError)
		{
		  LOG(@"error creating data type");
		  break;
		}
	    }
	  
	  if (i == count)
	    {
	      result = [NSData dataWithBytes: TXTRecordGetBytesPtr(&txt)
				      length: TXTRecordGetLength(&txt)];
	    }
	  
	  TXTRecordDeallocate(&txt);
	}
      else
	{
	  LOG(@"No keys or values");
	}
      
      // both are autorelease'd
      keys = nil;
      values = nil;
    }
  else
    {
      LOG(@"Dictionary seems empty");
    }
  return result;
}

/**
 * Converts the TXT data txtData into a dictionary.
 *
 *
 */

+ (NSDictionary *) dictionaryFromTXTRecordData: (NSData *) txtData
{
  NSMutableDictionary	*result = nil;
  int			len = 0;
  const void		*txt = 0;
  
  INTERNALTRACE;
  
  len = [txtData length];
  txt = [txtData bytes];
  
  //
  // A TXT record cannot exceed 65535 bytes, see Chapter 6.1 of
  // http://files.dns-sd.org/draft-cheshire-dnsext-dns-sd.txt
  //
  if ((len > 0) && (len < 65536))
    {
      uint16_t	i = 0;
      uint16_t	count = 0;
      
      // get number of keys
      count = TXTRecordGetCount(len, txt);
      result = [NSMutableDictionary dictionaryWithCapacity: 1];
      
      if (result)
	{
	  // go through all keys
	  for (; i < count; i++)
	    {
	      char			key[256];
	      uint8_t			valLen = 0;
	      const void		*value = NULL;
	      DNSServiceErrorType	err = kDNSServiceErr_NoError;
	      
	      err = TXTRecordGetItemAtIndex(len, txt, i,
		sizeof key, key,
		&valLen, &value);
	      
	      // only if we can get the key and value...
	      if (kDNSServiceErr_NoError == err)
		{
		  NSData	*data = nil;
		  NSString	*str = nil;
		  
		  str = [NSString stringWithUTF8String: key];
		  
		  if (value)
		    {
		      data = [NSData dataWithBytes: value
					    length: valLen];
		    }
		  
		  if (data && str && [str length]
		    && ! [result objectForKey: str])
		    {
		      /* only add if key and value were created
		       * and key doesn't exist yet
		       */
		      [result setValue: data
				forKey: str];
		    }
		  else
		    {
		      /* I'm not exactly sure what to do if there
		       * is a key WITHOUT a value
		       * Theoretically '<6>foobar' should be identical
		       * to '<7>foobar=' i.e. the value would be [NSNull null]
		       */
		      [result setValue: [NSNull null]
				forKey: str];
		    }
		  
		  // both are autorelease'd
		  data = nil;
		  str = nil;
		}
	      else
		{
		  LOG(@"Couldn't get TXTRecord item");
		}
	    }
	}
      else
	{
	  LOG(@"Couldn't create dictionary");
	}
    }
  else
    {
      LOG(@"TXT record has incorrect length: <%d>", len);
    }
  return result;
}

/**
 * Initializes the receiver for service resolution. Use this method to create
 * an object if you intend to -resolve a service.
 *
 */

- (id) initWithDomain: (NSString *) domain
                 type: (NSString *) type
                 name: (NSString *) name
{
  INTERNALTRACE;
  
  return [self initWithDomain: domain
                         type: type
                         name: name
                         port: -1]; // -1 to indicate resolution, not publish
}

/** <init />
 * Initializes the receiver for service publication. Use this method to create
 * an object if you intend to -publish a service.
 *
 */

- (id) initWithDomain: (NSString *) domain
                 type: (NSString *) type
                 name: (NSString *) name
                 port: (NSInteger) port
{
  INTERNALTRACE;
  
  if ((self = [super init]))
    {
      Service	*service;
      
      service = malloc(sizeof (struct _Service));
      memset(service, 0, sizeof &service);
      
      CREATELOCK(service);
      
      service->runloop = nil;
      service->runloopmode = nil;
      service->timer = nil;
      service->timeout = nil;
      
      service->info = [[NSMutableDictionary alloc] initWithCapacity: 3];
      [service->info setObject: AUTORELEASE([domain copy])
			forKey: @"Domain"];
      [service->info setObject: AUTORELEASE([name copy])
			forKey: @"Name"];
      [service->info setObject: AUTORELEASE([type copy])
			forKey: @"Type"];
      
      service->foundAddresses = nil;
      
      service->interfaceIndex = 0;
      service->port = htons(port);
      
      service->monitor = nil;
      
      service->isPublishing = (-1 == port) ? NO : YES;
      service->isMonitoring = NO;
      
      _netService = NULL;
      [self setDelegate: nil];
      _reserved = service;
      
      return self;
    }
  
  return nil;
}

/**
 * Removes the service from the specified run loop.
 *
 *
 */

- (void) removeFromRunLoop: (NSRunLoop *) aRunLoop
                   forMode: (NSString *) mode
{
  Service	*service;
  
  INTERNALTRACE;
  
  service = (Service *) _reserved;
  
  LOCK(service);
  {
    if (service->timer)
      {
	[service->timer setFireDate: [NSDate date]];
	[service->timer invalidate];
	
	// Do not release the timer!
	service->timer = nil;
      }
    
    // Do not release the runloop!
    service->runloop = nil;
    
    DESTROY(service->runloopmode);
  }
  UNLOCK(service);
}

/**
 * Adds the service to the specified run loop.
 *
 *
 */

- (void) scheduleInRunLoop: (NSRunLoop *) aRunLoop
                   forMode: (NSString *) mode
{
  Service	*service;
  
  INTERNALTRACE;
  
  service = (Service *) _reserved;
  
  LOCK(service);
  {
    if (service->timer)
      {
	[service->timer setFireDate: [NSDate date]];
	[service->timer invalidate];
	service->timer = nil;
      }
    
    service->timer = RETAIN([NSTimer timerWithTimeInterval: INTERVAL
                                                    target: self
                                                  selector: @selector(loop:)
                                                  userInfo: nil
                                                   repeats: YES]);
    
    service->runloop = aRunLoop;
    service->runloopmode = mode;
  }
  UNLOCK(service);
}

/**
 * Attempts to publish a service on the network.
 *
 *
 */

- (void) publishWithFlags: (DNSServiceFlags)flags
{
  Service		*service;
  DNSServiceErrorType	err = kDNSServiceErr_NoError;
  
  INTERNALTRACE;
  
  service = (Service *) _reserved;
  
  LOCK(service);
  {
    do
      {
	// cannot -publish on a service that's init'd for resolving
	if (NO == service->isPublishing)
	  {
	    err = NSNetServicesBadArgumentError;
	    break;
	  }
	
	if (service->timer)
	  {
	    err = NSNetServicesActivityInProgress;
	    break;
	  }
	
	if (service->timeout)
	  {
	    [service->timeout setFireDate: [NSDate date]];
	    [service->timeout invalidate];
	    service->timeout = nil;
	  }
	
	err = DNSServiceRegister((DNSServiceRef *) &_netService,
	  flags, service->interfaceIndex,
	  [[service->info objectForKey: @"Name"] UTF8String],
	  [[service->info objectForKey: @"Type"] UTF8String],
	  [[service->info objectForKey: @"Domain"] UTF8String],
	  NULL, service->port, 0, NULL,
	  RegistrationCallback, self);
      }
    while (0);
  }
  UNLOCK(service);
  
  [self executeWithError: err];
}

- (void) publishWithOptions: (NSNetServiceOptions)options
{
  DNSServiceFlags flags = 0;
  if (options & NSNetServiceNoAutoRename)
  {
    flags = flags | kDNSServiceFlagsNoAutoRename;
  }
  [self publishWithFlags: flags];
}

- (void) publish
{
  [self publishWithFlags: 0];
}

/**
 * This method is deprecated. Use -resolveWithTimeout: instead.
 *
 *
 */

- (void) resolve
{
  INTERNALTRACE;
  
  [self resolveWithTimeout: 5];
}

/**
 * Starts a service resolution for a limited duration.
 *
 *
 */

- (void) resolveWithTimeout: (NSTimeInterval) timeout
{
  Service		*service;
  DNSServiceErrorType	err = kDNSServiceErr_NoError;
  DNSServiceFlags	flags = 0;
  
  INTERNALTRACE;
  
  service = (Service *) _reserved;
  
  LOCK(service);
  {
    do
      {
	// cannot -resolve on a service that's init'd for publishing
	if (YES == service->isPublishing)
	  {
	    err = NSNetServicesBadArgumentError;
	    break;
	  }
	
	if (! [self delegate])
	  {
	    err = NSNetServicesInvalidError;
	    break;
	  }
	
	if (service->timer)
	  {
	    err = NSNetServicesActivityInProgress;
	    break;
	  }
	
	if (service->timeout)
	  {
	    [service->timeout setFireDate: [NSDate date]];
	    [service->timeout invalidate];
	    service->timeout = nil;
	  }
	
	service->timeout = [NSTimer alloc];
	{
	  NSDate	*date = nil;
	  
	  date = [NSDate dateWithTimeIntervalSinceNow: timeout + SHORTTIMEOUT];
	  
	  [service->timeout initWithFireDate: date
				    interval: INTERVAL
				      target: self
				    selector: @selector(stopResolving:)
				    userInfo: nil
				     repeats: NO];
	}
	
	err = DNSServiceResolve((DNSServiceRef *) &_netService,
	  flags,
	  service->interfaceIndex,
	  [[service->info objectForKey: @"Name"] UTF8String],
	  [[service->info objectForKey: @"Type"] UTF8String],
	  [[service->info objectForKey: @"Domain"] UTF8String],
	  ResolverCallback,
	  self);
      }
    while (0);
  }
  UNLOCK(service);
  
  [self executeWithError: err];
}

/**
 * Stops the current attempt to publish or resolve a service.
 *
 *
 */

- (void) stop
{
  Service	*service;
  
  INTERNALTRACE;
  
  service = (Service *) _reserved;
  
  LOCK(service);
  {
    [self cleanup];
    
    [self netServiceDidStop: self];
  }
  UNLOCK(service);
}

/**
 * Starts monitoring of TXT record updates.
 *
 *
 */

- (void) startMonitoring
{
  Service	*service;
  
  INTERNALTRACE;
  
  service = (Service *) _reserved;
  
  LOCK(service);
  {
    // Obviously this will only work on a resolver
    if (! service->isPublishing)
      {
	if (! service->isMonitoring)
	  {
	    service->monitor
	      = [[GSMDNSNetServiceMonitor alloc] initWithDelegate: self];
	    
	    [service->monitor scheduleInRunLoop: service->runloop
					forMode: service->runloopmode];
	    [service->monitor start];
	    
	    service->isMonitoring = YES;
	  }
      }
  }
  UNLOCK(service);
}

/**
 * Stops monitoring of TXT record updates.
 *
 *
 */

- (void) stopMonitoring
{
  Service	*service;
  
  INTERNALTRACE;
  
  service = (Service *) _reserved;
  
  LOCK(service);
  {
    if (! service->isPublishing)
      {
	if (service->isMonitoring)
	  {
	    [service->monitor stop];
	    
	    // Probably don't need it anymore, so release it
	    DESTROY(service->monitor);
	    service->isMonitoring = NO;
	  }
      }
  }
  UNLOCK(service);
}

/**
 * Returns an array of NSData objects that each contain the socket address of
 * the service.
 *
 */

- (NSArray *) addresses
{
  INTERNALTRACE;
  
  return [((Service*)_reserved)->info objectForKey: @"Addresses"];
}

/**
 * Returns the domain name of the service.
 *
 *
 */

- (NSString *) domain
{
  INTERNALTRACE;
  
  return [((Service*)_reserved)->info objectForKey: @"Domain"];
}

/**
 * Returns the host name of the computer publishing the service.
 *
 *
 */

- (NSString *) hostName
{
  INTERNALTRACE;
  
  return [((Service*)_reserved)->info objectForKey: @"Host"];
}

/**
 * Returns the name of the service.
 *
 *
 */

- (NSString *) name
{
  INTERNALTRACE;
  
  return [((Service*)_reserved)->info objectForKey: @"Name"];
}

- (NSInteger) port
{
	return ntohs(((Service*)_reserved)->port);
  return 0;
}


/**
 * Returns the type of the service.
 *
 *
 */

- (NSString *) type
{
  INTERNALTRACE;
  
  return [((Service*)_reserved)->info objectForKey: @"Type"];
}

/**
 * This method is deprecated. Use -TXTRecordData instead.
 *
 *
 */

- (NSString *) protocolSpecificInformation
{
  NSString	*retVal = nil;
  Service		*service;
  
  INTERNALTRACE;
  
  service = (Service *) _reserved;
  
  LOCK(service);
  {
	retVal = [super protocolSpecificInformation];
  }
  UNLOCK(service);
  
  return retVal;
}

/**
 * This method is deprecated. Use -setTXTRecordData: instead.
 *
 *
 */

- (void) setProtocolSpecificInformation: (NSString *) specificInformation
{
  Service	*service;
  
  INTERNALTRACE;
  
  service = (Service *) _reserved;
  
  //
  // Again, the following may not be entirely correct...
  //
  LOCK(service);
  {
	  [super setProtocolSpecificInformation: specificInformation];
  }
  UNLOCK(service);
}

/**
 * Returns the TXT record.
 *
 *
 */

- (NSData *) TXTRecordData
{
  INTERNALTRACE;
  
  return [((Service*)_reserved)->info objectForKey: @"TXT"];
}

/**
 * Sets the TXT record.
 *
 *
 */

- (BOOL) setTXTRecordData: (NSData *) recordData
{
  Service	*service;
  BOOL		result = NO;
  
  INTERNALTRACE;
  
  service = (Service *) _reserved;
  
  LOCK(service);
  {
    // Not allowed on a resolver...
    if (service->isPublishing)
      {
	DNSServiceErrorType
	  err = kDNSServiceErr_NoError;
	
	// Set the value, or remove it if empty
	if (recordData)
	  {
	    [service->info setObject: recordData
			      forKey: @"TXT"];
	  }
	else
	  {
	    [service->info removeObjectForKey: @"TXT"];
	  }
	
	// Assume it worked
	result = YES;
	
	// Now update the record so others can pick it up
	err = DNSServiceUpdateRecord(_netService,
	  NULL,
	  0,
	  recordData ? [recordData length] : 0,
	  recordData ? [recordData bytes] : NULL,
	  0);
	if (err)
	  {
	    result = NO;
	  }
      }
  }
  UNLOCK(service);
  
  return result;
}


/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void)      netService: (NSNetService *) sender
  didUpdateTXTRecordData: (NSData *) data
{
  id delegate = [self delegate];
  INTERNALTRACE;
  
  if ([delegate respondsToSelector:
    @selector(netService:didUpdateTXTRecordData:)])
    {
      [delegate netService: sender didUpdateTXTRecordData: data];
    }
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) netService: (NSNetService *) sender
      didNotMonitor: (NSDictionary *) errorDict
{
  INTERNALTRACE;
  
  // This method is kind of a misnomer. It's called whenever NSNetMonitor
  // encounters an error while monitoring.
  // All we do is stop monitoring -- which we COULD do from NSNetMonitor
  // directly, but this seems to be much cleaner.
  
  [self stopMonitoring];
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (id) init
{
  DESTROY(self);
  return self;
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) dealloc
{
  Service	*service;
  
  INTERNALTRACE;
  [self setDelegate: nil];
  service = (Service *) _reserved;
  {
    LOCK(service);
    {
      [self stopMonitoring];
      [self cleanup];
      
      DESTROY(service->info);
      DESTROY(service->foundAddresses);
    }
    UNLOCK(service);
    
    DESTROYLOCK(service);
    
    free(service);
  }
  [super dealloc];
}

@end

@implementation GSMDNSNetServiceMonitor

/**
 * <em>Description forthcoming</em>
 *
 *
 */

+ (void) initialize
{
  INTERNALTRACE;
  
  SETVERSION(GSMDNSNetServiceMonitor);
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) loop: (id) sender
{
  int			sock = 0;
  struct timeval	tout = { 0 };
  fd_set		set;
  DNSServiceErrorType	err = kDNSServiceErr_NoError;
  
  sock = DNSServiceRefSockFD(_netServiceMonitor);
  
  if (-1 != sock)
    {
      FD_ZERO(&set);
      FD_SET(sock, &set);
      
      if (1 == select(sock + 1, &set, (fd_set *) NULL, (fd_set *) NULL, &tout))
	{
	  err = DNSServiceProcessResult(_netServiceMonitor);
	}
    }
  
  if (kDNSServiceErr_NoError != err)
    {
      LOG(@"Error <%d> while monitoring", err);
      
      [_delegate netService: _delegate
	      didNotMonitor: CreateError(self, err)];
    }
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) queryCallback: (DNSServiceRef) sdRef
                 flags: (DNSServiceFlags) flags
             interface: (uint32_t) interfaceIndex
                 error: (DNSServiceErrorType) errorCode
              fullname: (const char *) fullname
                  type: (uint16_t) rrtype
                 class: (uint16_t) rrclass
                length: (uint16_t) rdlen
                  data: (const void *) rdata
                   ttl: (uint32_t) ttl
{
  Monitor	*monitor;
  
  INTERNALTRACE;
  
  monitor = (Monitor *) _reserved;
  
  LOCK(monitor);
  
  if (_delegate)
    {
      // we are 'monitoring' kDNSServiceType_TXT
      // this is already handled by the delegate's method of the same name
      // so we simply pass this through
      [_delegate queryCallback: sdRef
			 flags: flags
		     interface: interfaceIndex
			 error: errorCode
		      fullname: fullname
			  type: rrtype
			 class: rrclass
			length: rdlen
			  data: rdata
			   ttl: ttl];
    }
  UNLOCK(monitor);
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (id) initWithDelegate: (id) delegate
{
  INTERNALTRACE;
  
  if ((self = [super init]) != nil)
    {
      Monitor	*monitor;
      
      monitor = malloc(sizeof (struct _Monitor));
      memset(monitor, 0, sizeof &monitor);
      
      CREATELOCK(monitor);
      
      monitor->runloop = nil;
      monitor->runloopmode = nil;
      monitor->timer = nil;
      
      _netServiceMonitor = NULL;
      ASSIGN(_delegate, delegate);
      _reserved = monitor;
    }
  return self;
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) removeFromRunLoop: (NSRunLoop *) aRunLoop
                   forMode: (NSString *) mode
{
  Monitor	*monitor;
  
  INTERNALTRACE;
  
  monitor = (Monitor *) _reserved;
  
  LOCK(monitor);
  {
    if (monitor->timer)
      {
	[monitor->timer setFireDate: [NSDate date]];
	[monitor->timer invalidate];
	monitor->timer = nil;
      }
    
    // Do not release the runloop!
    monitor->runloop = nil;
    
    // [monitor->runloopmode release];
    monitor->runloopmode = nil;
  }
  UNLOCK(monitor);
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) scheduleInRunLoop: (NSRunLoop *) aRunLoop
                   forMode: (NSString *) mode
{
  Monitor	*monitor;
  
  INTERNALTRACE;
  
  monitor = (Monitor *) _reserved;
  
  LOCK(monitor);
  {
    if (monitor->timer)
      {
	[monitor->timer setFireDate: [NSDate date]];
	[monitor->timer invalidate];
	monitor->timer = nil;
      }
    
    monitor->runloop = aRunLoop;
    monitor->runloopmode = mode;
  }
  UNLOCK(monitor);
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) start
{
  Monitor	*monitor;
  
  INTERNALTRACE;
  
  monitor = (Monitor *) _reserved;
  
  LOCK(monitor);
  {
    DNSServiceErrorType	err = kDNSServiceErr_NoError;
    DNSServiceFlags	flags = kDNSServiceFlagsLongLivedQuery;
    NSString		*fullname = nil;
    
    do
      {
	if (! _delegate)
	  {
	    err = NSNetServicesInvalidError;
	    break;
	  }
	
	if (monitor->timer)
	  {
	    err = NSNetServicesActivityInProgress;
	    break;
	  }
	
	fullname = [NSString stringWithFormat: @"%@.%@%@",
	  [_delegate name], [_delegate type], [_delegate domain]];
	
	err = DNSServiceQueryRecord((DNSServiceRef *) &_netServiceMonitor,
	  flags,
	  0,
	  [fullname UTF8String],
	  kDNSServiceType_TXT,
	  kDNSServiceClass_IN,
	  QueryCallback,
	  self);
	
	if (kDNSServiceErr_NoError == err)
	  {
	    monitor->timer = [NSTimer timerWithTimeInterval: INTERVAL
						     target: self
						   selector: @selector(loop:)
						   userInfo: nil
						    repeats: YES];

	    [monitor->runloop addTimer: monitor->timer
			       forMode: monitor->runloopmode];
	    
	    [monitor->timer fire];
	  }
      }
    while (0);
  }
  UNLOCK(monitor);
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) stop
{
  Monitor	*monitor;
  
  INTERNALTRACE;
  
  monitor = (Monitor *) _reserved;
  
  LOCK(monitor);
  {
    if (monitor->runloop)
      {
	[self removeFromRunLoop: monitor->runloop
			forMode: monitor->runloopmode];
      }
    
    if (monitor->timer)
      {
	[monitor->timer invalidate];
	monitor->timer = nil;
      }
    
    if (_netServiceMonitor)
      {
	DNSServiceRefDeallocate(_netServiceMonitor);
	_netServiceMonitor = NULL;
      }
  }
  UNLOCK(monitor);
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (id) init
{
  DESTROY(self);
  return self;
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

- (void) dealloc
{
  Monitor	*monitor;
  
  INTERNALTRACE;
  
  monitor = (Monitor *) _reserved;
  {
    LOCK(monitor);
    {
      [self stop];
      
      _delegate = nil;
    }
    UNLOCK(monitor);
    
    DESTROYLOCK(monitor);
    
    free(monitor);
  }
  [super dealloc];
}

@end

/**
 * <em>Description forthcoming</em>
 *
 *
 */

static NSDictionary *
CreateError(id sender, int errorCode)
{
  NSMutableDictionary	*dictionary = nil;
  int			error = 0;
  
  INTERNALTRACE;
  
  dictionary = [NSMutableDictionary dictionary];
  error = ConvertError(errorCode);
  
  LOG(@"%@ says error <%d> - <%d>", [sender description], errorCode, error);
  
  [dictionary setObject: [NSNumber numberWithInt: error]
                 forKey: NSNetServicesErrorCode];
  [dictionary setObject: sender
                 forKey: NSNetServicesErrorDomain];
  
  return dictionary; // autorelease'd
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

static int
ConvertError(int errorCode)
{
  INTERNALTRACE;
  
  switch(errorCode)
    {
      case kDNSServiceErr_Unknown:
	return NSNetServicesUnknownError;
      
      case kDNSServiceErr_NoSuchName:
	return NSNetServicesNotFoundError;
      
      case kDNSServiceErr_NoMemory:
	return NSNetServicesUnknownError;
      
      case kDNSServiceErr_BadParam:
      case kDNSServiceErr_BadReference:
      case kDNSServiceErr_BadState:
      case kDNSServiceErr_BadFlags:
	return NSNetServicesBadArgumentError;
      
      case kDNSServiceErr_Unsupported:
	return NSNetServicesUnknownError;
      
      case kDNSServiceErr_NotInitialized:
	return NSNetServicesInvalidError;
      
      case kDNSServiceErr_AlreadyRegistered:
      case kDNSServiceErr_NameConflict:
	return NSNetServicesCollisionError;
      
      case kDNSServiceErr_Invalid:
	return NSNetServicesInvalidError;
      
      case kDNSServiceErr_Firewall:
	return NSNetServicesUnknownError;
      
      case kDNSServiceErr_Incompatible:
	// The client library is incompatible with the daemon
	return NSNetServicesInvalidError;
      
      case kDNSServiceErr_BadInterfaceIndex:
      case kDNSServiceErr_Refused:
	return NSNetServicesUnknownError;
      
      case kDNSServiceErr_NoSuchRecord:
      case kDNSServiceErr_NoAuth:
      case kDNSServiceErr_NoSuchKey:
	return NSNetServicesNotFoundError;
      
      case kDNSServiceErr_NATTraversal:
      case kDNSServiceErr_DoubleNAT:
      case kDNSServiceErr_BadTime:
	return NSNetServicesUnknownError;
    }
  
  return errorCode;
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

static void DNSSD_API
EnumerationCallback(DNSServiceRef sdRef,
                      DNSServiceFlags flags,
                      uint32_t interfaceIndex,
                      DNSServiceErrorType  errorCode,
                      const char *replyDomain,
                      void *context)
{
  // NSNetServiceBrowser
  [(id) context enumCallback: sdRef
                       flags: flags
                   interface: interfaceIndex
                       error: errorCode
                      domain: replyDomain];
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

static void DNSSD_API
BrowserCallback(DNSServiceRef sdRef,
                  DNSServiceFlags flags,
                  uint32_t interfaceIndex,
                  DNSServiceErrorType errorCode,
                  const char *replyName,
                  const char *replyType,
                  const char *replyDomain,
                  void *context)
{
  // NSNetServiceBrowser
  [(id) context browseCallback: sdRef
                         flags: flags
                     interface: interfaceIndex
                         error: errorCode
                          name: replyName
                          type: replyType
                        domain: replyDomain];
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

static void DNSSD_API
ResolverCallback(DNSServiceRef sdRef,
                   DNSServiceFlags flags,
                   uint32_t interfaceIndex,
                   DNSServiceErrorType errorCode,
                   const char *fullname,
                   const char *hosttarget,
                   uint16_t port,
                   uint16_t txtLen,
                   const unsigned char *txtRecord,
                   void *context)
{
  // NSNetService
  [(id) context resolverCallback: sdRef
                           flags: flags
                       interface: interfaceIndex
                           error: errorCode
                        fullname: fullname
                          target: hosttarget
                            port: port
                          length: txtLen
                          record: txtRecord];
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

static void DNSSD_API
RegistrationCallback(DNSServiceRef sdRef,
                       DNSServiceFlags flags,
                       DNSServiceErrorType errorCode,
                       const char *name,
                       const char *regtype,
                       const char *domain,
                       void *context)
{
  // NSNetService
  [(id) context registerCallback: sdRef
                           flags: flags
                           error: errorCode
                            name: name
                            type: regtype
                          domain: domain];
}

/**
 * <em>Description forthcoming</em>
 *
 *
 */

static void DNSSD_API
QueryCallback(DNSServiceRef sdRef,
                DNSServiceFlags flags,
                uint32_t interfaceIndex,
                DNSServiceErrorType errorCode,
                const char *fullname,
                uint16_t rrtype,
                uint16_t rrclass,
                uint16_t rdlen,
                const void *rdata,
                uint32_t ttl,
                void *context)
{
  // NSNetService, NSNetServiceMonitor
  [(id) context queryCallback: sdRef
                        flags: flags
                    interface: interfaceIndex
                        error: errorCode
                     fullname: fullname
                         type: rrtype
                        class: rrclass
                       length: rdlen
                         data: rdata
                          ttl: ttl];
}

