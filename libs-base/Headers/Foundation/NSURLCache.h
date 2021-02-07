/* Interface for NSURLCache for GNUstep
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

#ifndef __NSURLCache_h_GNUSTEP_BASE_INCLUDE
#define __NSURLCache_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2,GS_API_LATEST) && GS_API_VERSION( 11300,GS_API_LATEST)

#import	<Foundation/NSObject.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSData;
@class NSDictionary;
@class NSURLRequest;
@class NSURLRequest;
@class NSURLResponse;

/**
 * Specifies the cache storage policy.
 */
typedef enum
{
  NSURLCacheStorageAllowed,	/** Unrestricted caching */
  NSURLCacheStorageAllowedInMemoryOnly,	/** In memory caching only */
  NSURLCacheStorageNotAllowed /** No caching allowed */
} NSURLCacheStoragePolicy;


/**
 * Encapsulates a cached response to a URL load request.
 */
GS_EXPORT_CLASS
@interface NSCachedURLResponse : NSObject <NSCoding, NSCopying>
{
#if	GS_EXPOSE(NSCachedURLResponse)
  void *_NSCachedURLResponseInternal;
#endif
}

/**
 * Returns the data with which the receiver was initialised.
 */
- (NSData *) data;

/**
 * Uses the NSURLCacheStorageAllowed policy to cache the specified
 * response and data.<br />
 * Returns the cached response.
 */
- (id) initWithResponse: (NSURLResponse *)response data: (NSData *)data;

/**
 * Returns the receiver initialized with the provided parameters.
 */
- (id) initWithResponse: (NSURLResponse *)response
		   data: (NSData *)data
	       userInfo: (NSDictionary *)userInfo
	  storagePolicy: (NSURLCacheStoragePolicy)storagePolicy;

/**
 * Returns the response with which the receiver was initialised.
 */
- (NSURLResponse *) response;

/**
 * Returns the storage policy with which the receiver was initialised.
 */
- (NSURLCacheStoragePolicy) storagePolicy;

/**
 * Returns the user info dictionary with which the receiver was initialised
 * (if any).
 */
- (NSDictionary *) userInfo;

@end


GS_EXPORT_CLASS
@interface NSURLCache : NSObject
{
#if	GS_EXPOSE(NSURLCache)
  void *_NSURLCacheInternal;
#endif
}

/**
 * Sets the shared [NSURLCache] used throughout the process.<br />
 * If you are going to call this method to specify an alternative to
 * the default cache, you should do so before the shared cache is used
 * in order to avoid loss of data that was in the old cache.
 */
+ (void) setSharedURLCache: (NSURLCache *)cache;

/**
 * Returns the shared cache instance set by +setSharedURLCache: or,
 * if none has been set, returns an instance initialised with<br />
 * <deflist>
 *   <term>Memory capacity</term>
 *   <desc>4 megabytes</desc>
 *   <term>Disk capacity</term>
 *   <desc>20 megabytes</desc>
 *   <term>Disk path</term>
 *   <desc>user-library-path/Caches/current-app-name</desc>
 * </deflist>
 */
+ (NSURLCache *) sharedURLCache;

/**
 * Returns the [NSCachedURLResponse] cached for the specified request
 * or nil if there is no matching response in tthe cache.
 */
- (NSCachedURLResponse *) cachedResponseForRequest: (NSURLRequest *)request;

/**
 * Returns the current size (butes) of the data stored in the on-disk
 * cache.
 */
- (NSUInteger) currentDiskUsage;

/**
 * Returns the current size (butes) of the data stored in the in-memory
 * cache.
 */
- (NSUInteger) currentMemoryUsage;

/**
 * Returns the disk capacity (in bytes) of the cache.
 */
- (NSUInteger) diskCapacity;

/**
 * Returns the receiver initialised with the specified capacities
 * (in bytes) and using the specified location on disk for persistent
 * storage.
 */
- (id) initWithMemoryCapacity: (NSUInteger)memoryCapacity
		 diskCapacity: (NSUInteger)diskCapacity
		     diskPath: (NSString *)path;

/**
 * Returns the memory capacity (in bytes) of the cache.
 */
- (NSUInteger) memoryCapacity;

/**
 * Empties the cache.
 */
- (void) removeAllCachedResponses;

/**
 * Removes from the cache (if present) the [NSCachedURLResponse]
 * which was stored using the specified request.
 */
- (void) removeCachedResponseForRequest: (NSURLRequest *)request;

/**
 * Sets the disk capacity (in bytes) truncating cache contents if necessary.
 */
- (void) setDiskCapacity: (NSUInteger)diskCapacity;

/**
 * Sets the memory capacity (in bytes) truncating cache contents if necessary.
 */
- (void) setMemoryCapacity: (NSUInteger)memoryCapacity;

/**
 * Stores cachedResponse in the cache, keyed on request.<br />
 * Replaces any existing response with the same key.
 */
- (void) storeCachedResponse: (NSCachedURLResponse *)cachedResponse
		  forRequest: (NSURLRequest *)request;

@end

@class NSURLSessionDataTask;

@interface NSURLCache (NSURLSessionTaskAdditions)

- (void) storeCachedResponse: (NSCachedURLResponse*)cachedResponse 
                 forDataTask: (NSURLSessionDataTask*)dataTask;

- (NSCachedURLResponse*) cachedResponseForDataTask: (NSURLSessionDataTask*)dataTask;

- (void) removeCachedResponseForDataTask: (NSURLSessionDataTask*)dataTask;

@end

#if	defined(__cplusplus)
}
#endif

#endif

#endif
