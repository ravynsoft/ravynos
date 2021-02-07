/* NSURLHandle.h - Class NSURLHandle
   Copyright (C) 1999 Free Software Foundation, Inc.
   
   Written by: 	Manuel Guesdon <mguesdon@sbuilders.com>
   Date: 	Jan 1999
   
   This file is part of the GNUstep Library.
   
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

#ifndef __NSURLHandle_h_GNUSTEP_BASE_INCLUDE
#define __NSURLHandle_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if	OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)

#import	<Foundation/NSObject.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSData;
@class NSString;
@class NSMutableArray;
@class NSMutableData;
@class NSURLHandle;
@class NSURL;

/**
 * Key for passing to [NSURLHandle]'s <code>propertyForKey..</code> methods to
 * obtain status code.
 */
GS_EXPORT NSString * const NSHTTPPropertyStatusCodeKey;

/**
 * Key for passing to [NSURLHandle]'s <code>propertyForKey..</code> methods to
 * obtain status reason.
 */
GS_EXPORT NSString * const NSHTTPPropertyStatusReasonKey;

/**
 * Key for passing to [NSURLHandle]'s <code>propertyForKey..</code> methods to
 * obtain HTTP version supported by server.
 */
GS_EXPORT NSString * const NSHTTPPropertyServerHTTPVersionKey;

/**
 * Key for passing to [NSURLHandle]'s <code>propertyForKey..</code> methods to
 * obtain redirection headers.
 */
GS_EXPORT NSString * const NSHTTPPropertyRedirectionHeadersKey;

/**
 * Key for passing to [NSURLHandle]'s <code>propertyForKey..</code> methods to
 * obtain error page data.
 */
GS_EXPORT NSString * const NSHTTPPropertyErrorPageDataKey;

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)

/**
 * Key for passing to [NSURLHandle]'s <code>propertyForKey..</code> methods to
 * obtain local host.
 */
GS_EXPORT NSString * const GSHTTPPropertyLocalHostKey;

/**
 * Key for passing to [NSURLHandle]'s <code>propertyForKey..</code> methods to
 * obtain method (GET, POST, etc.).
 */
GS_EXPORT NSString * const GSHTTPPropertyMethodKey;

/**
 * Key for passing to [NSURLHandle]'s <code>propertyForKey..</code> methods to
 * obtain proxy host.
 */
GS_EXPORT NSString * const GSHTTPPropertyProxyHostKey;

/**
 * Key for passing to [NSURLHandle]'s <code>propertyForKey..</code> methods to
 * obtain proxy port.
 */
GS_EXPORT NSString * const GSHTTPPropertyProxyPortKey;

/**
 * Key for passing to [NSURLHandle]'s <code>propertyForKey..</code> methods to
 * specify the location of an SSL certificate file.
 */
GS_EXPORT NSString * const GSHTTPPropertyCertificateFileKey;

/**
 * Key for passing to [NSURLHandle]'s <code>propertyForKey..</code> methods to
 * specify the location of an SSL key file.
 */
GS_EXPORT NSString * const GSHTTPPropertyKeyFileKey;

/**
 * Key for passing to [NSURLHandle]'s <code>propertyForKey..</code> methods to
 * specify the password for an SSL key file.
 */
GS_EXPORT NSString * const GSHTTPPropertyPasswordKey;

#endif

/**
 * Enumerated type returned by [NSURLHandle-status]:
<example>
{
  NSURLHandleNotLoaded
  NSURLHandleLoadSucceeded,
  NSURLHandleLoadInProgress,
  NSURLHandleLoadFailed
}
</example>
 */
enum
{
  NSURLHandleNotLoaded = 0,
  NSURLHandleLoadSucceeded,
  NSURLHandleLoadInProgress,
  NSURLHandleLoadFailed
};
typedef NSUInteger NSURLHandleStatus;

/**
 * A protocol to which clients of a handle must conform in order to
 * receive notification of events on the handle.
 */
@protocol NSURLHandleClient

/**
 * Sent by the NSURLHandle object when some data becomes available
 * from the handle.  Note that this does not mean that all data has become
 * available, only that a chunk of data has arrived.
 */
- (void) URLHandle: (NSURLHandle*)sender
  resourceDataDidBecomeAvailable: (NSData*)newData;

/**
 * Sent by the NSURLHandle object on resource load failure.
 * Supplies a human readable failure reason.
 */
- (void) URLHandle: (NSURLHandle*)sender
  resourceDidFailLoadingWithReason: (NSString*)reason;

/**
 * Sent by the NSURLHandle object when it begins loading
 * resource data.
 */
- (void) URLHandleResourceDidBeginLoading: (NSURLHandle*)sender;

/**
 * Sent by the NSURLHandle object when resource loading is cancelled
 * by programmatic request (rather than by failure).
 */
- (void) URLHandleResourceDidCancelLoading: (NSURLHandle*)sender;

/**
 * Sent by the NSURLHandle object when it completes loading
 * resource data.
 */
- (void) URLHandleResourceDidFinishLoading: (NSURLHandle*)sender;
@end

GS_EXPORT_CLASS
@interface NSURLHandle : NSObject
{
#if	GS_EXPOSE(NSURLHandle)
@protected
  id			_data;
  NSMutableArray	*_clients;
  NSString		*_failure; 
  NSURLHandleStatus	_status;
#endif
}

+ (NSURLHandle*) cachedHandleForURL: (NSURL*)url;
+ (BOOL) canInitWithURL: (NSURL*)url;
+ (void) registerURLHandleClass: (Class)urlHandleSubclass;
+ (Class) URLHandleClassForURL: (NSURL*)url;

- (void) addClient: (id <NSURLHandleClient>)client;
- (NSData*) availableResourceData;
- (void) backgroundLoadDidFailWithReason: (NSString*)reason;
- (void) beginLoadInBackground;
- (void) cancelLoadInBackground;
- (void) didLoadBytes: (NSData*)newData
	 loadComplete: (BOOL)loadComplete;
- (void) endLoadInBackground;
- (NSString*) failureReason;
- (void) flushCachedData;
- (id) initWithURL: (NSURL*)url
	    cached: (BOOL)cached;
- (void) loadInBackground;
- (NSData*) loadInForeground;
- (id) propertyForKey: (NSString*)propertyKey;
- (id) propertyForKeyIfAvailable: (NSString*)propertyKey;
- (void) removeClient: (id <NSURLHandleClient>)client;
- (NSData*) resourceData;

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)

/** GNUstep extension to turn on debug logging for a handle.  Returns the
 * previous debug setting for the handle.  Implemented for http/https only.
 */
- (int) setDebug: (int)flag;

/** GNUstep extension to turn on returning of complete http/https response
 * even when the status code is not in the 200 to 299 success range.
 */
- (void) setReturnAll: (BOOL)flag;

/** GNUstep extension to change the URL that the handle sends requests to.
 * Implemented for http/https only.
 */
- (void) setURL: (NSURL*)newUrl;
#endif

- (NSURLHandleStatus) status;
- (BOOL) writeData: (NSData*)data;
- (BOOL) writeProperty: (id)propertyValue
		forKey: (NSString*)propertyKey;

@end

#if	defined(__cplusplus)
}
#endif

#endif

#endif /* __NSURLHandle_h_GNUSTEP_BASE_INCLUDE */

