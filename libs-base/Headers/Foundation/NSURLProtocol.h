/* Interface for NSURLProtocol for GNUstep
   Copyright (C) 2006 Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <frm@gnu.org>
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

#ifndef __NSURLProtocol_h_GNUSTEP_BASE_INCLUDE
#define __NSURLProtocol_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2,GS_API_LATEST) && GS_API_VERSION( 11300,GS_API_LATEST)

#import	<Foundation/NSObject.h>

#if	defined(__cplusplus)
extern "C" {
#endif

#import	<Foundation/NSURLCache.h>

@class NSCachedURLResponse;
@class NSError;
@class NSMutableURLRequest;
@class NSURLAuthenticationChallenge;
@class NSURLConnection;
@class NSURLProtocol;
@class NSURLRequest;
@class NSURLResponse;
@class NSURLSessionTask;


/**
 * Defines the API for NSURLProtocol loading
 */
@protocol NSURLProtocolClient <NSObject>

/**
 * Informs a client that a cached response is valid.
 */
- (void) URLProtocol: (NSURLProtocol *)protocol
  cachedResponseIsValid: (NSCachedURLResponse *)cachedResponse;

/**
 * Informs a client that loading of a request has failed.
 */
- (void) URLProtocol: (NSURLProtocol *)protocol
    didFailWithError: (NSError *)error;

/**
 * Informs a client that data has been loaded.  Only new data since the
 * last call to this method must be provided.
 */
- (void) URLProtocol: (NSURLProtocol *)protocol
	 didLoadData: (NSData *)data;

/**
 * Informs a client that an authentication challenge has been received.
 */
- (void) URLProtocol: (NSURLProtocol *)protocol
  didReceiveAuthenticationChallenge: (NSURLAuthenticationChallenge *)challenge;

/**
 * Informs a client that a response for the current load has been created.<br />
 * Also supplies the policy to be used for caching the response.
 */
- (void) URLProtocol: (NSURLProtocol *)protocol
  didReceiveResponse: (NSURLResponse *)response
  cacheStoragePolicy: (NSURLCacheStoragePolicy)policy;

/**
 * Informs a client that a redirect has occurred.<br />
 */
- (void) URLProtocol: (NSURLProtocol *)protocol
  wasRedirectedToRequest: (NSURLRequest *)request
  redirectResponse: (NSURLResponse *)redirectResponse;


/**
 * Informs a client that loading of a request has successfully finished.
 */
- (void) URLProtocolDidFinishLoading: (NSURLProtocol *)protocol;

/**
 * Informs a client that an authentication challenge has been cancelled.
 */
- (void) URLProtocol: (NSURLProtocol *)protocol
  didCancelAuthenticationChallenge: (NSURLAuthenticationChallenge *)challenge;

@end


/**
 * <p>Subclasses of NSURLProtocol implement basic handling of URL
 * loading for specific protocols.  The NSURLProtocol class
 * itsself is a semi-abstract class giving the essential
 * structure for the subclasses.
 * </p>
 * <p>You never instantiate NSURLProtocol yourself ... it should only
 * ever be done by other classes within the URL loading system.
 * </p>
 */
GS_EXPORT_CLASS
@interface NSURLProtocol : NSObject
{
#if	GS_EXPOSE(NSURLProtocol)
  void *_NSURLProtocolInternal;
#endif
}

/**
 * Allows subclasses to provide access to proptocol specific
 * properties, returning the property of request stored by the
 * name key or nil if no property had been stored using that
 * key in the request.
 */
+ (id) propertyForKey: (NSString *)key inRequest: (NSURLRequest *)request;

/**
 * Registers the specified class so that it can be used to load requests.<br />
 * When the system is determining which class to use to handle a
 * request it examines them in a most recently registered first order.<br />
 * The +canInitWithRequest: method is used to determine whether a class
 * may be used to handle a particular request or not.
 * Returns YES if registered (ie the class is an NSURLProtocol subclass),
 * NO otherwise.
 */
+ (BOOL) registerClass: (Class)protocolClass;

/**
 * Allows subclasses to provide a way to set protocol specific properties,
 * setting the property named key to value in the request.
 */
+ (void) setProperty: (id)value
	      forKey: (NSString *)key
	   inRequest: (NSMutableURLRequest *)request;

/**
 * Unregisters a class which was previously registered using the
 * +registerClass: method.
 */
+ (void) unregisterClass: (Class)protocolClass;

/**
 * Returns the cachedResponse of the receiver.
 */
- (NSCachedURLResponse *) cachedResponse;

/**
 * Returns the client associated with the receiver.
 */
- (id <NSURLProtocolClient>) client;

/**
 * Initialises the receiver with request, cachedResponse and client.<br />
 * The cachedResponse may be the result of a previous load of the
 * request (in which case the protocol may validate and use it).<br />
 * The client is the object which receives messages about the progress
 * of the load.  This is retained by the protocl instance and is released
 * once the last message has been sent to it.
 */
- (id) initWithRequest: (NSURLRequest *)request
	cachedResponse: (NSCachedURLResponse *)cachedResponse
		client: (id <NSURLProtocolClient>)client;

- (instancetype) initWithTask: (NSURLSessionTask*)task 
               cachedResponse: (NSCachedURLResponse*)cachedResponse 
                       client: (id<NSURLProtocolClient>)client;

/**
 * Returns the request handled by the receiver.
 */
- (NSURLRequest *) request;

/**
 * Returns the task handled by the receiver.
 */
- (NSURLSessionTask *) task;

@end

/**
 * This category lists the methods which a subclass must implement
 * in order to produce a working protocol.
 */
@interface	NSURLProtocol (Subclassing)

/** <override-subclass />
 * This method is called to decide whether a class can deal with
 * the specified request. The abstract class implementation
 * raises an exception.
 */
+ (BOOL) canInitWithRequest: (NSURLRequest *)request;

/** <override-subclass />
 * Returns the 'canonical' version of the request.<br />
 * The canonical form is used to look up requests in the cache by
 * checking for equality.<br />
 * The abnstract class implementation simply returns request.
 */
+ (NSURLRequest *) canonicalRequestForRequest: (NSURLRequest *)request;

/** <override-subclass />
 * Compares two requests for equivalence for caching purposes.<br />
 * The abstract class implementaton just uses [NSObject-isEqual:]
 */
+ (BOOL) requestIsCacheEquivalent: (NSURLRequest *)a
			toRequest: (NSURLRequest *)b;

/** <override-subclass />
 * Starts loading of a request.
 */
- (void) startLoading;

/** <override-subclass />
 * Stops loading of a request (eg when the load is cancelled).
 */
- (void) stopLoading;

@end

#if	defined(__cplusplus)
}
#endif

#endif

#endif
