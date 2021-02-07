/* Interface for NSNetServices for GNUstep
   Copyright (C) 2006, 2010 Free Software Foundation, Inc.

   Originally written by:  Chris B. Vetter
   Date: 2006
   Modified by: Niels Grewe <niels.grewe@halbordnung.de>
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
#import "GSNetServices.h"
#import "GSFastEnumeration.h"
#import "GNUstepBase/NSNetServices+GNUstepBase.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSData.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSHost.h"
#import "Foundation/NSStream.h"
#import "Foundation/NSString.h"
/**
 * This key identifies the most recent error.
 */
NSString * const NSNetServicesErrorCode = @"NSNetServicesErrorCode";

/**
 * This key identifies the originator of the error.
 */
NSString * const NSNetServicesErrorDomain = @"NSNetServicesErrorDomain";

static Class abstractServiceClass;
static Class concreteServiceClass;
static Class abstractBrowserClass;
static Class concreteBrowserClass;

@implementation NSNetService
+ (void)initialize
{
  if (self == [NSNetService class])
    {
      abstractServiceClass = self;
#     if GS_USE_AVAHI==1 
        concreteServiceClass = [GSAvahiNetService class];
#     else
        concreteServiceClass = [GSMDNSNetService class];
#     endif
  }
}

+ (id) allocWithZone: (NSZone*)zone
{
  if (self == abstractServiceClass)
    {
      return [concreteServiceClass allocWithZone: zone];
    }
  return [super allocWithZone: zone];
}

+ (NSData *) dataFromTXTRecordDictionary: (NSDictionary *) txtDictionary
{
  return [concreteServiceClass dataFromTXTRecordDictionary: txtDictionary];
}

+ (NSDictionary *) dictionaryFromTXTRecordData: (NSData *) txtData
{
  return [concreteServiceClass dictionaryFromTXTRecordData: txtData];
}

- (id) initWithDomain: (NSString *) domain
                 type: (NSString *) type
                 name: (NSString *) name
{
  return [self subclassResponsibility: _cmd];
}

- (id) initWithDomain: (NSString *) domain
                 type: (NSString *) type
                 name: (NSString *) name
                 port: (NSInteger) port
{
  return [self subclassResponsibility: _cmd];
}

- (void) removeFromRunLoop: (NSRunLoop *) aRunLoop
                   forMode: (NSString *) mode
{
  [self subclassResponsibility: _cmd];
}

- (void) scheduleInRunLoop: (NSRunLoop *) aRunLoop
                   forMode: (NSString *) mode
{
  [self subclassResponsibility: _cmd];
}


#if OS_API_VERSION(100500,GS_API_LATEST) 
/** Not implemented */
- (NSInteger)port
{
  [self subclassResponsibility: _cmd];
  return 0;
}

/** Not implemented */
- (void) publishWithOptions: (NSNetServiceOptions)options
{
  [self subclassResponsibility: _cmd];
}

#endif

- (void) publish
{
  [self subclassResponsibility: _cmd];
}

- (void) resolve
{
  [self subclassResponsibility: _cmd];
}

- (void) resolveWithTimeout: (NSTimeInterval) timeout
{
  [self subclassResponsibility: _cmd];
}

- (void) stop
{
  [self subclassResponsibility: _cmd];
}


- (void) startMonitoring
{
  [self subclassResponsibility: _cmd];
}

- (void) stopMonitoring
{
  [self subclassResponsibility: _cmd];
}


- (id<NSNetServiceDelegate>) delegate
{
  return _delegate;
}

- (void) setDelegate: (id<NSNetServiceDelegate>) delegate
{
  _delegate = delegate;
}


- (NSArray *) addresses
{
  return [self subclassResponsibility: _cmd];
}

- (NSString *) domain
{
  return [self subclassResponsibility: _cmd];
}

- (NSString *) hostName
{
  return [self subclassResponsibility: _cmd];
}

- (NSString *) name
{
  return [self subclassResponsibility: _cmd];
}

- (NSString *) type
{
  return [self subclassResponsibility: _cmd];
}

- (NSData *) TXTRecordData
{
  return [self subclassResponsibility: _cmd];
}

- (BOOL) setTXTRecordData: (NSData *) recordData
{
  [self subclassResponsibility: _cmd];
  return NO;
}

- (NSString*)protocolSpecificInformation
{
  /* 
   * Note: This implementation follows the one that was in GSMDNSNetService,
   * which had the following comment:
   *
   * I must admit, the following may not be entirely correct...
   *
   * It uses [self class] to obtain the concrete subclass that implements
   * +dictionaryFromTXTRecordData:.
   */
  NSDictionary *dict;
  NSMutableArray *array = nil;
  NSString *retVal = nil;

  dict = [[self class] dictionaryFromTXTRecordData: [self TXTRecordData]];
  if (dict == nil)
    {
      return nil;
    }
  array = [[NSMutableArray alloc] initWithCapacity: [dict count]];
  FOR_IN(NSString*, key, dict)
    {
      NSData *value = [dict objectForKey: key];

      if ([value length] > 0)
        {
          NSString *valueString;

          valueString = [[NSString alloc] initWithBytes: [value bytes]
						 length: [value length]
					       encoding: NSUTF8StringEncoding];
          [array addObject:
	    [NSString stringWithFormat: @"%@=%@", key, valueString]];
          DESTROY(valueString);
        }
      else if ([key length] > 0)
        {
          [array addObject: [[key copy] autorelease]];
        }
    }
  END_FOR_IN(dict)
  
  if ([array count] > 0)
    {
      retVal = [array componentsJoinedByString: @"\001"];
    }
  [array release];
  return retVal;
}

- (void) setProtocolSpecificInformation: (NSString *) specificInformation
{
  NSArray *array = [specificInformation componentsSeparatedByString: @"\001"];
    
  if (array != nil)
    {
      NSMutableDictionary *dictionary;

      dictionary
	= [[NSMutableDictionary alloc] initWithCapacity: [array count]];
      FOR_IN(NSString*, item, array)
        {
          NSArray	*parts;
          NSData	*value;

          parts = [item componentsSeparatedByString: @"="];
          value = [[parts objectAtIndex: 1]
	    dataUsingEncoding: NSUTF8StringEncoding];
          [dictionary setObject: value
                         forKey: [parts objectAtIndex: 0]];
        }
      END_FOR_IN(array)
      [self setTXTRecordData:
        [[self class] dataFromTXTRecordDictionary: dictionary]];
    }
}

- (BOOL) getInputStream: (NSInputStream **) inputStream
           outputStream: (NSOutputStream **) outputStream
{
  [NSStream getStreamsToHost: [NSHost hostWithName: [self hostName]]
                        port: [self port]
                 inputStream: inputStream
                outputStream: outputStream];
  
  return inputStream || outputStream;
}

/*
 * NSNetService delegate methods:
 */
- (void) netServiceWillResolve: (NSNetService*)service
{
  if ([_delegate respondsToSelector: @selector(netServiceWillResolve:)])
    {
      [_delegate netServiceWillResolve: service];
    }
}

- (void) netService: (NSNetService*)service
      didNotResolve: (NSDictionary*)errorDict
{
  if ([_delegate respondsToSelector: @selector(netService:didNotResolve:)])
    {
      [_delegate netService: service
              didNotResolve: errorDict];
    }
}

- (void) netServiceDidResolveAddress: service
{
  if ([_delegate respondsToSelector: @selector(netServiceDidResolveAddress:)])
    {
      [_delegate netServiceDidResolveAddress: service];
    }
}

- (void) netServiceDidStop: (NSNetService*)service
{
  if ([_delegate respondsToSelector: @selector(netServiceDidStop:)])
    {
      [_delegate netServiceDidStop: service];
    }
}

- (void) netServiceWillPublish: (NSNetService*)service
{
  if ([_delegate respondsToSelector: @selector(netServiceWillPublish:)])
    {
      [_delegate netServiceWillPublish: service];
    }
}

- (void) netService: (NSNetService*)service
      didNotPublish: (NSDictionary*)errorDict
{
  if ([_delegate respondsToSelector: @selector(netService:didNotPublish:)])
    {
      [_delegate netService: service
              didNotPublish: errorDict];
    }
}

- (void) netServiceDidPublish: (NSNetService*)service
{
  if ([_delegate respondsToSelector: @selector(netServiceDidPublish:)])
    {
      [_delegate netServiceDidPublish: service];
    }
}

/*
 * Define extensions for the Avahi API subclass.
 */
#if GS_USE_AVAHI==1
- (void) startMonitoringForRecordType: (NSString*)recordType
{
  [self subclassResponsibility: _cmd];
}

- (void) stopMonitoringForRecordType: (NSString*)recordType
{
  [self subclassResponsibility: _cmd];
}

- (BOOL) addServiceRecord
{
  [self subclassResponsibility: _cmd];
  return 0;
}

- (BOOL) addRecordData: (NSData*)data
{
  [self subclassResponsibility: _cmd];
  return 0;
}

- (id) recordDataForRecordType: (NSString*)type
{
  return [self subclassResponsibility: _cmd];
}

- (NSString*) fullServiceName
{
  return [self subclassResponsibility: _cmd];
}
#endif // GS_USE_AVAHI
@end

@implementation NSNetServiceBrowser
+ (void) initialize
{
  if (self == [NSNetServiceBrowser class])
    {
      abstractBrowserClass = self;
#     if GS_USE_AVAHI==1 
        concreteBrowserClass = [GSAvahiNetServiceBrowser class];
#     else // Not Avahi (=GS_USE_MDNS)
        concreteBrowserClass = [GSMDNSNetServiceBrowser class];
#     endif // GS_USE_AVAHI
    }
}

+ (id) allocWithZone: (NSZone*)zone
{
  if (self == abstractBrowserClass)
    {
      return [concreteBrowserClass allocWithZone: zone];
    }
  return [super allocWithZone: zone];
}

- (id) init
{
  return [super init];
}


- (void) removeFromRunLoop: (NSRunLoop *) aRunLoop
                   forMode: (NSString *) mode
{
  [self subclassResponsibility: _cmd];
}

- (void) scheduleInRunLoop: (NSRunLoop *) aRunLoop
                   forMode: (NSString *) mode
{
  [self subclassResponsibility: _cmd];
}


- (void) searchForAllDomains
{
  [self subclassResponsibility: _cmd];
}

- (void) searchForBrowsableDomains
{
  [self subclassResponsibility: _cmd];
}

- (void) searchForRegistrationDomains
{
  [self subclassResponsibility: _cmd];
}


- (void) searchForServicesOfType: (NSString *) serviceType
                        inDomain: (NSString *) domainName
{
  [self subclassResponsibility: _cmd];
}


- (void) stop
{
  [self subclassResponsibility: _cmd];
}


- (id<NSNetServiceBrowserDelegate>) delegate
{
  return _delegate;
}

- (void) setDelegate: (id<NSNetServiceBrowserDelegate>) delegate
{
  _delegate = delegate;
}

- (void) netServiceBrowserWillSearch: (NSNetServiceBrowser*)aBrowser
{
  if ([_delegate respondsToSelector: @selector(netServiceBrowserWillSearch:)])
    {
      [_delegate netServiceBrowserWillSearch: aBrowser];
    }
}

- (void) netServiceBrowserDidStopSearch: (NSNetServiceBrowser*)aBrowser
{
  if ([_delegate respondsToSelector:
    @selector(netServiceBrowserDidStopSearch:)])
    {
      [_delegate netServiceBrowserDidStopSearch: aBrowser];
    }
}

- (void) netServiceBrowser: (NSNetServiceBrowser*)aBrowser
              didNotSearch: (NSDictionary*)errorDict
{
  if ([_delegate respondsToSelector:
    @selector(netServiceBrowser:didNotSearch:)])
    {
      [_delegate netServiceBrowser: aBrowser
                      didNotSearch: errorDict];
    }
}

- (void) netServiceBrowser: (NSNetServiceBrowser*)aBrowser
             didFindDomain: (NSString*)theDomain
                moreComing: (BOOL)moreComing
{
  if ([_delegate respondsToSelector:
    @selector(netServiceBrowser:didFindDomain:moreComing:)])
    {
      [_delegate netServiceBrowser: aBrowser
                     didFindDomain: theDomain
                        moreComing: moreComing];
    }
}

- (void) netServiceBrowser: (NSNetServiceBrowser*)aBrowser
           didRemoveDomain: (NSString*)theDomain
                moreComing: (BOOL)moreComing
{
  if ([_delegate respondsToSelector:
    @selector(netServiceBrowser:didRemoveDomain:moreComing:)])
    {
      [_delegate netServiceBrowser: aBrowser
                  didRemoveDomain: theDomain
                       moreComing: moreComing];
    }
}

- (void) netServiceBrowser: (NSNetServiceBrowser*)aBrowser
            didFindService: (NSNetService*)theService
                moreComing: (BOOL)moreComing
{
  if ([_delegate respondsToSelector:
    @selector(netServiceBrowser:didFindService:moreComing:)])
    {
      [_delegate netServiceBrowser: aBrowser
                    didFindService: theService
                        moreComing: moreComing];
    }
}

- (void) netServiceBrowser: (NSNetServiceBrowser*)aBrowser
          didRemoveService: (NSNetService*)theService
                moreComing: (BOOL)moreComing
{
  if ([_delegate respondsToSelector:
    @selector(netServiceBrowser:didRemoveService:moreComing:)])
    {
      [_delegate netServiceBrowser: aBrowser
                  didRemoveService: theService
                        moreComing: moreComing];
    }
}
@end

