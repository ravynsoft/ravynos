/* Interface for NSNetServices for GNUstep
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

#ifndef __NSNetServices_h_GNUSTEP_BASE_INCLUDE
#define __NSNetServices_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSObject.h>
#import	<Foundation/NSRange.h>

#if	defined(__cplusplus)
extern "C" {
#endif


enum
{
  NSNetServicesUnknownError		= -72000L,
  NSNetServicesCollisionError		= -72001L,
  NSNetServicesNotFoundError		= -72002L,
  NSNetServicesActivityInProgress	= -72003L,
  NSNetServicesBadArgumentError		= -72004L,
  NSNetServicesCancelledError		= -72005L,
  NSNetServicesInvalidError		= -72006L,
  NSNetServicesTimeoutError		= -72007L
};
  /**
   * <list>
   *   <item>
   *     <strong>NSNetServicesUnknownError</strong><br />
   *     An unknown error occurred.
   *     <br /><br />
   *   </item>
   *   <item>
   *     <strong>NSNetServicesCollisionError</strong><br />
   *     The given registration has had a name collision. Registration should
   *     be cancelled and tried again with a different name.
   *     <br /><br />
   *   </item>
   *   <item>
   *     <strong>NSNetServicesNotFoundError</strong><br />
   *     The service could not be found.
   *     <br /><br />
   *   </item>
   *   <item>
   *     <strong>NSNetServicesActivityInProgress</strong><br />
   *     A request is already in progress.
   *     <br /><br />
   *   </item>
   *   <item>
   *     <strong>NSNetServicesBadArgumentError</strong><br />
   *     An invalid argument was used to create the object.
   *     <br /><br />
   *   </item>
   *   <item>
   *     <strong>NSNetServicesCancelledError</strong><br />
   *     The request has been cancelled.
   *     <br /><br />
   *   </item>
   *   <item>
   *     <strong>NSNetServicesInvalidError</strong><br />
   *     The service was improperly configured.
   *     <br /><br />
   *   </item>
   *   <item>
   *     <strong>NSNetServicesTimeoutError</strong><br />
   *     The request has timed out before a successful resolution.
   *     <br /><br />
   *   </item>
   * </list>
   */
typedef NSUInteger NSNetServicesError;

enum {
  NSNetServiceNoAutoRename          = 1 << 0
#if OS_API_VERSION(MAC_OS_X_VERSION_10_9,GS_API_LATEST)
  ,NSNetServiceListenForConnections = 1 << 1
#endif
};
typedef NSUInteger NSNetServiceOptions;


GS_EXPORT NSString * const NSNetServicesErrorCode;
GS_EXPORT NSString * const NSNetServicesErrorDomain;


@class	NSInputStream;
@class  NSOutputStream;
@class  NSRunLoop;

@class NSNetService;
@class NSNetServiceBrowser;


@protocol  NSNetServiceDelegate
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST) && GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#else
@end
@interface NSObject (NSNetServiceDelegateMethods)
#endif

/**
 * Notifies the delegate that the network is ready to publish the service.
 *
 * <p><strong>See also:</strong><br />
 *   [NSNetService-publish]<br />
 * </p>
 */

- (void) netServiceWillPublish: (NSNetService *) sender;

/**
 * Notifies the delegate that the service was successfully published.
 *
 * <p><strong>See also:</strong><br />
 *   [NSNetService-publish]<br />
 * </p>
 */

- (void) netServiceDidPublish: (NSNetService *) sender;

/**
 * Notifies the delegate that the service could not get published.
 *
 * <p><strong>See also:</strong><br />
 *   [NSNetService-publish]<br />
 * </p>
 */

- (void) netService: (NSNetService *) sender
      didNotPublish: (NSDictionary *) errorDict;

/**
 * Notifies the delegate that the network is ready to resolve the service.
 *
 * <p><strong>See also:</strong><br />
 *   [NSNetService-resolveWithTimeout:]<br />
 * </p>
 */

- (void) netServiceWillResolve: (NSNetService *) sender;

/**
 * Notifies the delegate that the service was resolved.
 *
 * <p><strong>See also:</strong><br />
 *   [NSNetService-resolveWithTimeout:]<br />
 * </p>
 */

- (void) netServiceDidResolveAddress: (NSNetService *) sender;

/**
 * Notifies the delegate that the service could not get resolved.
 *
 * <p><strong>See also:</strong><br />
 *   [NSNetService-resolveWithTimeout:]<br />
 * </p>
 */

- (void) netService: (NSNetService *) sender
      didNotResolve: (NSDictionary *) errorDict;

/**
 * Notifies the delegate that the request was stopped.
 *
 * <p><strong>See also:</strong><br />
 *   [NSNetService-stop]<br />
 * </p>
 */

- (void) netServiceDidStop: (NSNetService *) sender;

/**
 * Notifies the delegate that the TXT record has been updated.
 *
 * <p><strong>See also:</strong><br />
 *   [NSNetService-startMonitoring]<br />
 *   [NSNetService-stopMonitoring]
 * </p>
 */

- (void)      netService: (NSNetService *) sender
  didUpdateTXTRecordData: (NSData *) data;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_9,GS_API_LATEST)

/**
 * Notifies the delegate that the service, which must have been published with
 * option NSNetServiceListenForConnections, received a new connection.
 * In order to communicate with the connecting client, you must -open
 * the streams and schedule them with a runloop.
 * To reject a connection, just -open and immediately -close both streams.
 */

- (void)                  netService: (NSNetService *) sender
  didAcceptConnectionWithInputStream: (NSInputStream *) inputStream
						outputStream: (NSOutputStream *)outputStream;

#endif

@end

/**
 * <unit>
 *   <heading>
 *     NSNetServiceBrowserDelegate protocol description
 *   </heading>
 *   <p>
 *     <!-- Foreword -->
 *   </p>
 *   <unit />
 *   <p>
 *     <!-- Afterword -->
 *   </p>
 * </unit>
 * <p>
 *  This protocol must be adopted by any class wishing to implement
 *  an [NSNetServiceBrowser] delegate.
 * </p>
 */

@protocol NSNetServiceBrowserDelegate
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST) && GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#else
@end
@interface NSObject (NSNetServiceBrowserDelegateMethods)
#endif

/**
 * Notifies the delegate that the search is about to begin.
 *
 * <p><strong>See also:</strong><br />
 *   [NSNetServiceBrowser-netServiceBrowser:didNotSearch:]<br />
 * </p>
 */

- (void) netServiceBrowserWillSearch: (NSNetServiceBrowser *)aNetServiceBrowser;

/**
 * Notifies the delegate that the search was unsuccessful.
 *
 * <p><strong>See also:</strong><br />
 *   [NSNetServiceBrowser-netServiceBrowserWillSearch:]<br />
 * </p>
 */

- (void) netServiceBrowser: (NSNetServiceBrowser *) aNetServiceBrowser
              didNotSearch: (NSDictionary *) errorDict;

/**
 * Notifies the delegate that the search was stopped.
 *
 * <p><strong>See also:</strong><br />
 *   [NSNetServiceBrowser-stop]<br />
 * </p>
 */

- (void) netServiceBrowserDidStopSearch:
  (NSNetServiceBrowser *)aNetServiceBrowser;

/**
 * Notifies the delegate that a domain was found.
 *
 * <p><strong>See also:</strong><br />
 *   [NSNetServiceBrowser-searchForBrowsableDomains]<br />
 *   [NSNetServiceBrowser-searchForRegistrationDomains]<br />
 * </p>
 */

- (void) netServiceBrowser: (NSNetServiceBrowser *) aNetServiceBrowser
             didFindDomain: (NSString *) domainString
                moreComing: (BOOL) moreComing;

/**
 * Notifies the delegate that a domain has become unavailable.
 *
 * <p><strong>See also:</strong><br />
 *   <br />
 * </p>
 */

- (void) netServiceBrowser: (NSNetServiceBrowser *) aNetServiceBrowser
           didRemoveDomain: (NSString *) domainString
                moreComing: (BOOL) moreComing;

/**
 * Notifies the delegate that a service was found.
 *
 * <p><strong>See also:</strong><br />
 *   [NSNetServiceBrowser-searchForServicesOfType:inDomain:]<br />
 * </p>
 */

- (void) netServiceBrowser: (NSNetServiceBrowser *) aNetServiceBrowser
            didFindService: (NSNetService *) aNetService
                moreComing: (BOOL) moreComing;

/**
 * Notifies the delegate that a service has become unavailable.
 *
 * <p><strong>See also:</strong><br />
 *   <br />
 * </p>
 */

- (void) netServiceBrowser: (NSNetServiceBrowser *) aNetServiceBrowser
          didRemoveService: (NSNetService *) aNetService
                moreComing: (BOOL) moreComing;

@end



/**
 * <unit>
 *   <heading>
 *     NSNetService class description
 *   </heading>
 *   <p>
 *     <!-- Foreword -->
 *   </p>
 *   <unit />
 *   <p>
 *     <!-- Afterword -->
 *   </p>
 * </unit>
 * <p>
 *   [NSNetService] lets you publish a network service in a domain using
 *   multicast DNS. Additionally, it lets you resolve a network service that
 *   was discovered by [NSNetServiceBrowser]. This class is an abstract
 *   superclass for concrete implementations of its functionality.
 * </p>
 */
GS_EXPORT_CLASS
@interface NSNetService : NSObject
{
#if	GS_EXPOSE(NSNetService)
  id		_delegate;
  void		*_netService;
  void		*_reserved;
#endif
}

+ (NSData *) dataFromTXTRecordDictionary: (NSDictionary *) txtDictionary;
+ (NSDictionary *) dictionaryFromTXTRecordData: (NSData *) txtData;

- (id) initWithDomain: (NSString *) domain
                 type: (NSString *) type
                 name: (NSString *) name;
- (id) initWithDomain: (NSString *) domain
                 type: (NSString *) type
                 name: (NSString *) name
                 port: (NSInteger) port;

- (void) removeFromRunLoop: (NSRunLoop *) aRunLoop
                   forMode: (NSString *) mode;
- (void) scheduleInRunLoop: (NSRunLoop *) aRunLoop
                   forMode: (NSString *) mode;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5,GS_API_LATEST) 
- (NSInteger) port;

- (void) publishWithOptions: (NSNetServiceOptions)options;
#endif

- (void) publish;
- (void) resolve;
- (void) resolveWithTimeout: (NSTimeInterval) timeout;
- (void) stop;

- (void) startMonitoring;
- (void) stopMonitoring;

- (id<NSNetServiceDelegate>) delegate;
- (void) setDelegate: (id<NSNetServiceDelegate>) delegate;

- (NSArray *) addresses;
- (NSString *) domain;
- (NSString *) hostName;
- (NSString *) name;
- (NSString *) type;

- (NSString *) protocolSpecificInformation;
- (void) setProtocolSpecificInformation: (NSString *) specificInformation;

- (NSData *) TXTRecordData;
- (BOOL) setTXTRecordData: (NSData *) recordData;

- (BOOL) getInputStream: (NSInputStream **) inputStream
           outputStream: (NSOutputStream **) outputStream;

@end

/**
 * <unit>
 *   <heading>
 *     NSNetServiceBrowser class description
 *   </heading>
 *   <p>
 *     <!-- Foreword -->
 *   </p>
 *   <unit />
 *   <p>
 *     <!-- Afterword -->
 *   </p>
 * </unit>
 * <p>
 *   [NSNetServiceBrowser] asynchronously lets you discover network domains
 *   and, additionally, search for a type of network service. It sends its
 *   delegate a message whenever it discovers a new network service, and
 *   whenever a network service goes away.
 * </p>
 * <p>
 *   Each [NSNetServiceBrowser] performs one search at a time. So in order
 *   to perform multiple searches simultaneously, create multiple instances.
 *   This class is an abstract superclass for concrete implementations of its
 *   functionality.
 * </p>
 */
GS_EXPORT_CLASS
@interface NSNetServiceBrowser : NSObject
{
#if	GS_EXPOSE(NSNetServiceBrowser)
  id		_delegate;
  void		*_netServiceBrowser;
  void		*_reserved;
#endif
}

- (id) init;

- (void) removeFromRunLoop: (NSRunLoop *) aRunLoop
                   forMode: (NSString *) mode;
- (void) scheduleInRunLoop: (NSRunLoop *) aRunLoop
                   forMode: (NSString *) mode;

- (void) searchForAllDomains;
- (void) searchForBrowsableDomains;
- (void) searchForRegistrationDomains;

- (void) searchForServicesOfType: (NSString *) serviceType
                        inDomain: (NSString *) domainName;

- (void) stop;

- (id<NSNetServiceBrowserDelegate>) delegate;
- (void) setDelegate: (id<NSNetServiceBrowserDelegate>) delegate;

@end

/**
 * <unit>
 *   <heading>
 *     NSNetServiceDelegate protocol description
 *   </heading>
 *   <p>
 *     <!-- Foreword -->
 *   </p>
 *   <unit />
 *   <p>
 *     <!-- Afterword -->
 *   </p>
 * </unit>
 * <p>
 *  This protocol must be adopted by any class wishing to implement
 *  an [NSNetService] delegate.
 * </p>
 */


#if	!NO_GNUSTEP && !defined(GNUSTEP_BASE_INTERNAL)
#import	<GNUstepBase/NSNetServices+GNUstepBase.h>
#endif
#if	defined(__cplusplus)
}
#endif

#endif	/* __NSNetServices_h_GNUSTEP_BASE_INCLUDE */

