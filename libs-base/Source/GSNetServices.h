/* Interface for concrete subclasses of NSNetServices on GNUstep
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

#import "common.h"
#define	EXPOSE_NSNetService_IVARS	1
#define	EXPOSE_NSNetServiceBrowser_IVARS	1
#import	"Foundation/NSNetServices.h"
#import "GNUstepBase/NSNetServices+GNUstepBase.h"

#if GS_USE_AVAHI==1

// Subclasses using Avahi:

/** Convenience macro to create NSStrings if possible or return nil otherwise */
#define NSStringIfNotNull(x) x ? [NSString stringWithUTF8String: x] : nil

/**
 * Possible types of Avahi browsers.
 */
typedef enum
{
  GSAvahiUnknownBrowser,
  GSAvahiDomainBrowser,
  GSAvahiServiceBrowser,
  GSAvahiBrowserMax
} GSAvahiBrowserType;

/**
 * States of an Avahi service.
 */
typedef enum
{
  GSNetServiceIdle,
  GSNetServiceResolving,
  GSNetServiceResolved,
  GSNetServiceRecordBrowsing,
  GSNetServicePublishing,
  GSNetServicePublished,
  GSNetServiceStateMax
} GSNetServiceState;

/**
 * Turns a string into an NSString, adding a trailing "." if neccessary.
 */
NSString* GSNetServiceDotTerminatedNSStringFromString(const char* string);

@class GSAvahiRunLoopContext;
@class NSTimer;
@class NSRecursiveLock;
@class NSMutableDictionary;
@class NSMapTable;

/**
 * NSNetService using the avahi-client API.
 */
@interface GSAvahiNetService : NSNetService <NSNetServiceDelegate>
{
  // GSAvahiClient behaviour ivars:
  // From superclass: id _delegate;
  GSAvahiRunLoopContext *ctx;
  void *_client;
  NSRecursiveLock *_lock;
  // Ivars for this class:
  NSMutableDictionary *_info;
  NSRecursiveLock *_infoLock;
  NSUInteger _infoSeq;
  GSNetServiceState _serviceState;
  int _ifIndex;
  int _protocol;
  void *_entryGroup;
  void *_resolver;
  NSMapTable *_browsers;
  NSMapTable *_browserTimeouts;
  NSTimer *_timer;
}

/**
 * Intializer that passes interface index and protocol information
 * alongside the usual information for a mDNS service. This is used by
 * GSNetServiceBrowser which already knows about these.
 */
- (id) initWithDomain: (NSString*)domain
                 type: (NSString*)type
                 name: (NSString*)name
         avahiIfIndex: (int)anIfIndex
        avahiProtocol: (int)aProtocol;

#if GS_USE_AVAHI==1
- (id<NSObject,GSNetServiceDelegate>)delegate;
#else
- (id<NSObject>)delegate;
#endif
@end

/**
 * NSNetServiceBrowser using the avahi-client API.
 */
@interface GSAvahiNetServiceBrowser
  : NSNetServiceBrowser <NSNetServiceBrowserDelegate>
{
  // GSAvahiClient behaviour ivars:
  // from superclass: id _delegate;
  GSAvahiRunLoopContext *ctx;
  void *_client;
  NSRecursiveLock *_lock;
  // Ivars for this class:
  void* _browser;
  GSAvahiBrowserType _type;
  BOOL _hasFirstEvent;
  NSMutableDictionary *_services;
}
@end

#else // GS_USE_MDNS

// Subclasses using mDNSResponder:

/**
 * NSNetService using the mDNSResponder API.
 */
@interface GSMDNSNetService : NSNetService <NSNetServiceDelegate>
@end

/**
 * NSNetServiceBrowser using the mDNSResponder API.
 */
@interface GSMDNSNetServiceBrowser : NSNetServiceBrowser <NSNetServiceBrowserDelegate>
@end

#endif // GS_USE_AVAHI
