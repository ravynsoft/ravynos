/** NSURLHandle.m - Class NSURLHandle
   Copyright (C) 1999 Free Software Foundation, Inc.

   Written by: 	Manuel Guesdon <mguesdon@sbuilders.com>
   Date: 	Jan 1999
   Rewrite by:	Richard Frith-Macdonald <rfm@gnu.org>
   Date:	Sep 2000, June 2002

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

   <title>NSURLHandle class reference</title>
   $Date$ $Revision$
*/

#import "common.h"

#define	EXPOSE_NSURLHandle_IVARS	1
#import "GSURLPrivate.h"

#import "Foundation/NSURLHandle.h"
#import "Foundation/NSRunLoop.h"
#import "Foundation/NSFileManager.h"


@class	GSFTPURLHandle;
@interface GSFTPURLHandle : NSObject	// Help the compiler
@end
@class	GSHTTPURLHandle;
@interface GSHTTPURLHandle : NSObject	// Help the compiler
@end

@interface	GSFileURLHandle : NSURLHandle
{
  NSString		*_path;
  NSMutableDictionary	*_attributes;
}
@end

/*
 * Keys for NSURLHandle
 */
NSString * const NSHTTPPropertyStatusCodeKey
  = @"NSHTTPPropertyStatusCodeKey";

NSString * const NSHTTPPropertyStatusReasonKey
  = @"NSHTTPPropertyStatusReasonKey";

NSString * const NSHTTPPropertyServerHTTPVersionKey
  = @"NSHTTPPropertyServerHTTPVersionKey";

NSString * const NSHTTPPropertyRedirectionHeadersKey
  = @"NSHTTPPropertyRedirectionHeadersKey";

NSString * const NSHTTPPropertyErrorPageDataKey
  = @"NSHTTPPropertyErrorPageDataKey";

/* These are GNUstep extras */
NSString * const GSHTTPPropertyMethodKey
  = @"GSHTTPPropertyMethodKey";

NSString * const GSHTTPPropertyLocalHostKey
  = @"GSHTTPPropertyLocalHostKey";

NSString * const GSHTTPPropertyProxyHostKey
  = @"GSHTTPPropertyProxyHostKey";

NSString * const GSHTTPPropertyProxyPortKey
  = @"GSHTTPPropertyProxyPortKey";

NSString * const GSHTTPPropertyCertificateFileKey
  = @"GSHTTPPropertyCertificateFileKey";

NSString * const GSHTTPPropertyKeyFileKey
  = @"GSHTTPPropertyKeyFileKey";

NSString * const GSHTTPPropertyPasswordKey
  = @"GSHTTPPropertyPasswordKey";


/**
 * <p>
 *   An NSURLHandle instance is used to manage the resource data
 *   corresponding to an NSURL object. A single NSURLHandle can
 *   be used to manage multiple NSURL objects as long as those
 *   objects all refer to the same resource.
 * </p>
 * <p>
 *   Different NSURLHandle subclasses are used to manage different
 *   types of URL (usually based on the scheme of the URL), and you
 *   can register new subclasses to extend (or replace) the
 *   standard ones.
 * </p>
 * <p>
 *   GNUstep comes with private subclasses to handle the common
 *   URL schemes -
 * </p>
 * <list>
 *   <item>
 *     <code>file:</code> (local file I/O)
 *   </item>
 *   <item>
 *     <code>http:</code> and <code>https:</code> (webserver) access.
 *   </item>
 *   <item>
 *     <code>ftp:</code> (FTP server) access.
 *   </item>
 * </list>
 */
@implementation NSURLHandle

static NSLock		*registryLock = nil;
static NSMutableArray	*registry = nil;
static Class		NSURLHandleClass = 0;

/**
 * Return a handle for the specified URL from the cache if possible.
 * If the cache does not contain a matching handle, returns nil.
 */
+ (NSURLHandle*) cachedHandleForURL: (NSURL*)url
{
  /*
   * Each subclass is supposed to do its own caching, so we must
   * find the correct subclass and ask it for its cached handle.
   */
  if (self == NSURLHandleClass)
    {
      Class	c = [self URLHandleClassForURL: url];

      if (c == self || c == 0)
	{
	  return nil;
	}
      else
	{
	  return [c cachedHandleForURL: url];
	}
    }
  else
    {
      [self subclassResponsibility: _cmd];
      return nil;
    }
}

/** <override-subclass />
 * Implemented by subclasses to say which URLs they can handle.
 * This method is used to determine which subclasses can be used
 * to handle a particular URL.
 */
+ (BOOL) canInitWithURL: (NSURL*)url
{
  /*
   * The semi-abstract base class can't handle ANY scheme
   */
  return NO;
}

+ (void) initialize
{
  if (self == [NSURLHandle class])
    {
      NSURLHandleClass = self;
      registry = [NSMutableArray new];
      [[NSObject leakAt: &registry] release];
      registryLock = [NSLock new];
      [[NSObject leakAt: &registryLock] release];
      [self registerURLHandleClass: [GSFileURLHandle class]];
      [self registerURLHandleClass: [GSFTPURLHandle class]];
      [self registerURLHandleClass: [GSHTTPURLHandle class]];
    }
}

/**
 * Used to register a subclass as being available to handle URLs.
 */
+ (void) registerURLHandleClass: (Class)urlHandleSubclass
{
  /*
   * Maintain a registry of classes that handle various schemes
   * Re-adding a class moves it to the end of the registry - so it will
   * be used in preference to any class added earlier.
   */
  [registryLock lock];
  NS_DURING
    {
      [registry removeObjectIdenticalTo: urlHandleSubclass];
      [registry addObject: urlHandleSubclass];
    }
  NS_HANDLER
    {
      [registryLock unlock];
      [localException raise];
    }
  NS_ENDHANDLER
  [registryLock unlock];
}

/**
 * Returns the most recently registered NSURLHandle subclass that
 * responds to +canInitWithURL: with YES.
 * If there is no such subclass, returns nil.
 */
+ (Class) URLHandleClassForURL: (NSURL*)url
{
  unsigned	count;
  Class		c = 0;

  [registryLock lock];
  NS_DURING
    {
      count = [registry count];

      /*
       * Find a class to handle the URL, try most recently registered first.
       */
      while (count-- > 0)
	{
	  id	found = [registry objectAtIndex: count];

	  if ([found canInitWithURL: url] == YES)
	    {
	      c = (Class)found;
	      break;		// Found it.
	    }
	}
    }
  NS_HANDLER
    {
      [registryLock unlock];
      [localException raise];
    }
  NS_ENDHANDLER
  [registryLock unlock];
  return c;
}

/**
 * Add a client object, making sure that it doesn't occur more than once.<br />
 * The client object will receive messages notifying it of events on the handle.
 */
- (void) addClient: (id <NSURLHandleClient>)client
{
  id	o = client;

  IF_NO_GC([o retain];)
  [_clients removeObjectIdenticalTo: o];
  [_clients addObject: o];
  IF_NO_GC([o release];)
}

/**
 * Returns the resource data that is currently available for the
 * handle.  This may be a partially loaded resource or may be
 * empty if no data has been loaded yet or the last load failed.
 */
- (NSData*) availableResourceData
{
  return AUTORELEASE([_data copy]);
}

/**
 * This method should be called when a background load fails.<br />
 * The method passes the failure notification to the clients of
 * the handle - so subclasses should call super's implementation
 * at the end of their implementation of this method.
 */
- (void) backgroundLoadDidFailWithReason: (NSString*)reason
{
  NSEnumerator			*enumerator = [_clients objectEnumerator];
  id <NSURLHandleClient>	client;

  _status = NSURLHandleLoadFailed;
  DESTROY(_data);
  ASSIGNCOPY(_failure, reason);

  while ((client = [enumerator nextObject]) != nil)
    {
      [client URLHandle: self resourceDidFailLoadingWithReason: _failure];
    }
}

/**
 * This method is called by when a background load begins.
 * Subclasses should call super's implementation at
 * the end of their implementation of this method.
 */
- (void) beginLoadInBackground
{
  _status = NSURLHandleLoadInProgress;
  DESTROY(_data);
  _data = [NSMutableData new];
  [_clients makeObjectsPerformSelector:
    @selector(URLHandleResourceDidBeginLoading:)
    withObject: self];
}

/**
 * This method should be called to cancel a load currently in
 * progress.  The method calls -endLoadInBackground
 * Subclasses should call super's implementation at
 * the end of their implementation of this method.
 */
- (void) cancelLoadInBackground
{
  IF_NO_GC([self retain];)
  [_clients makeObjectsPerformSelector:
    @selector(URLHandleResourceDidCancelLoading:)
    withObject: self];
  [self endLoadInBackground];
  IF_NO_GC(RELEASE(self);)
}

- (void) dealloc
{
  RELEASE(_data);
  RELEASE(_failure);
  RELEASE(_clients);
  [super dealloc];
}

/**
 * Method called by subclasses during process of loading a resource.
 * The base class maintains a copy of the data being read in and
 * accumulates separate parts of the data.
 */
- (void) didLoadBytes: (NSData*)newData
	 loadComplete: (BOOL)loadComplete
{
  NSEnumerator			*enumerator;
  id <NSURLHandleClient>	client;

  /*
   * Let clients know we are starting loading (unless this has already been
   * done).
   */
  if (_status != NSURLHandleLoadInProgress)
    {
      _status = NSURLHandleLoadInProgress;
      DESTROY(_data);
      _data = [NSMutableData new];
      [_clients makeObjectsPerformSelector:
	@selector(URLHandleResourceDidBeginLoading:)
	withObject: self];
    }

  /*
   * If we have been given nil data, there must have been a failure!
   */
  if (newData == nil)
    {
      [self backgroundLoadDidFailWithReason: @"nil data"];
      return;
    }

  /*
   * Let clients know we have read some data.
   */
  enumerator = [_clients objectEnumerator];
  while ((client = [enumerator nextObject]) != nil)
    {
      [client URLHandle: self resourceDataDidBecomeAvailable: newData];
    }

  /*
   * Accumulate data in cache.
   */
  [_data appendData: newData];

  if (loadComplete == YES)
    {
      id	tmp = _data;

      _data = [tmp copy];
      RELEASE(tmp);
      /*
       * Let clients know we have finished loading.
       */
      _status = NSURLHandleLoadSucceeded;
      [_clients makeObjectsPerformSelector:
	@selector(URLHandleResourceDidFinishLoading:)
	withObject: self];
    }
}

/**
 * This method is called to stop any background loading process.
 * -cancelLoadInBackground uses this method to cancel loading.
 * Subclasses should call super's implementation at
 * the end of their implementation of this method.
 */
- (void) endLoadInBackground
{
  _status = NSURLHandleNotLoaded;
  DESTROY(_data);
}

/**
 * Returns the failure reason for the last failure to load
 * the resource data.
 */
- (NSString*) failureReason
{
  if (_status == NSURLHandleLoadFailed)
    return _failure;
  else
    return nil;
}

/**
 * Flushes any cached resource data.
 */
- (void) flushCachedData
{
  DESTROY(_data);
}

- (id) init
{
  return [self initWithURL: nil cached: NO];
}

/** <init />
 * Initialises a handle with the specified URL.<br />
 * The flag determines whether the handle will cache resource data
 * and respond to requests from equivalent URLs for the cached data.
 */
- (id) initWithURL: (NSURL*)url
	    cached: (BOOL)cached
{
  _status = NSURLHandleNotLoaded;
  _clients = [NSMutableArray new];
  return self;
}

/**
 * Starts (or queues) loading of the handle's resource data
 * in the background (asynchronously).<br />
 * The default implementation uses loadInForeground -
 * if this method is not overridden, loadInForeground MUST be.
 */
- (void) loadInBackground
{
  NSData	*d;

  [self beginLoadInBackground];
  d = [self loadInForeground];
  if (d == nil)
    {
      [self backgroundLoadDidFailWithReason: @"foreground load returned nil"];
    }
  else
    {
      [self didLoadBytes: d loadComplete: YES];
    }
}

/**
 * Loads the handle's resource data in the foreground (synchronously).<br />
 * The default implementation starts a background load and waits for
 * it to complete -
 * if this method is not overridden, loadInBackground MUST be.
 */
- (NSData*) loadInForeground
{
  NSRunLoop	*loop = [NSRunLoop currentRunLoop];

  [self loadInBackground];
  while ([self status] == NSURLHandleLoadInProgress)
    {
      NSDate	*limit;

      limit = [[NSDate alloc] initWithTimeIntervalSinceNow: 1.0];
      [loop runMode: NSDefaultRunLoopMode beforeDate: limit];
      RELEASE(limit);
    }
  return _data;
}

/** <override-subclass />
 * Returns the property for the specified key, or nil if the
 * key does not exist.
 */
- (id) propertyForKey: (NSString*)propertyKey
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/** <override-subclass />
 * Returns the property for the specified key, but only if the
 * handle does not need to do any work to retrieve it.
 */
- (id) propertyForKeyIfAvailable: (NSString*)propertyKey
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/**
 * Removes an object from them list of clients notified of
 * resource loading events by the URL handle.
 */
- (void) removeClient: (id <NSURLHandleClient>)client
{
  [_clients removeObjectIdenticalTo: client];
}

/**
 * Returns the resource data belonging to the handle.
 * Calls -loadInForeground if necessary.
 * <p>
 *   The GNUstep implementation treats an <em>ftp:</em> request for a
 *   directory as a request to list the names of the directory contents.
 * </p>
 */
- (NSData*) resourceData
{
  NSData        *d = nil;

  if (NSURLHandleLoadSucceeded == _status)
    {
      d = [self availableResourceData];
    }
  if (nil == d
    && _status != NSURLHandleLoadSucceeded
    && _status != NSURLHandleLoadFailed)
    {
      if (_status == NSURLHandleLoadInProgress)
	{
	  return nil;
	}
      else
	{
	  d = [self loadInForeground];
	  if (d != nil)
	    {
	      ASSIGNCOPY(_data, d);
	    }
	}
    }
  return d;
}

/* Private method ... subclasses override this to enable debug to be
 * turned off and on.
 */
- (int) setDebug: (int)aFlag
{
  return 0;
}

- (void) setReturnAll: (BOOL)flag
{
  return;
}

- (void) setURL: (NSURL*)newUrl
{
  return;
}

/**
 * Returns the current status of the handle.
 */
- (NSURLHandleStatus) status
{
  return _status;
}

/**
 * <p>
 *   Writes resource data to the handle.  Returns YES on success,
 *   NO on failure.
 * </p>
 * <p>
 *   The GNUstep implementation for <em>file:</em> writes the data
 *   directly to the local filesystem, and the return status reflects
 *   the result of that write operation.
 * </p>
 * <p>
 *   The GNUstep implementation for <em>http:</em> and <em>https:</em>
 *   sets the specified data as information to be POSTed to the URL next
 *   time it is loaded - so the method always returns YES.
 * </p>
 * <p>
 *   The GNUstep implementation for <em>ftp:</em> sets the specified data
 *   as information to be written to the URL next time it is loaded - so
 *   the method always returns YES.
 * </p>
 */
- (BOOL) writeData: (NSData*)data
{
  [self subclassResponsibility: _cmd];
  return NO;
}

/**
 * <p>
 *   Sets a property for handle.
 *   Returns YES on success, NO on failure.
 * </p>
 * <p>
 *   The GNUstep implementation sets the property as a header
 *   to be sent the next time the URL is loaded, and recognizes
 *   some special property keys which control the behavior of
 *   the next load.
 * </p>
 */
- (BOOL) writeProperty: (id)propertyValue
		forKey: (NSString*)propertyKey
{
  [self subclassResponsibility: _cmd];
  return NO;
}

@end

/**
 * <p>
 *   This is a <em>PRIVATE</em> subclass of NSURLHandle.
 *   It is documented here in order to give you information about the
 *   default behavior of an NSURLHandle created to deal with a URL
 *   that has the FILE scheme.  The name and/or other
 *   implementation details of this class may be changed at any time.
 * </p>
 * <p>
 *   A GSFileURLHandle instance is used to manage files on the local
 *   file-system of your machine.
 * </p>
 */
@implementation	GSFileURLHandle

static NSMutableDictionary	*fileCache = nil;
static NSLock			*fileLock = nil;

+ (NSURLHandle*) cachedHandleForURL: (NSURL*)url
{
  NSURLHandle	*obj = nil;

  if ([url isFileURL] == YES)
    {
      NSString	*path = [url path];

      path = [path stringByStandardizingPath];
      [fileLock lock];
      NS_DURING
	{
	  obj = [fileCache objectForKey: path];
	  IF_NO_GC([[obj retain] autorelease];)
	}
      NS_HANDLER
	{
	  [fileLock unlock];
	  [localException raise];
	}
      NS_ENDHANDLER
      [fileLock unlock];
    }
  return obj;
}

+ (BOOL) canInitWithURL: (NSURL*)url
{
  if ([url isFileURL] == YES)
    {
      return YES;
    }
  return NO;
}

+ (void) initialize
{
  fileCache = [NSMutableDictionary new];
  [[NSObject leakAt: &fileCache] release];
  fileLock = [NSLock new];
  [[NSObject leakAt: &fileLock] release];
}

- (NSData*) availableResourceData
{
  if (_data != nil)
    {
      NSDictionary	*dict;

      dict = [[NSFileManager defaultManager] fileAttributesAtPath: _path
						     traverseLink: YES];
      if (dict == nil)
	{
	  // File no longer exists.
	  DESTROY(_data);
	  DESTROY(_attributes);
	}
      else
	{
	  NSDate	*original;
	  NSDate	*latest;

	  original = [_attributes fileModificationDate];
	  latest = [dict fileModificationDate];
	  if ([latest earlierDate: original] != latest)
	    {
	      // File has been modified
	      DESTROY(_data);
	      DESTROY(_attributes);
              _status = NSURLHandleNotLoaded;
	    }
	}
    }
  return [super availableResourceData];
}

- (void) dealloc
{
  RELEASE(_path);
  RELEASE(_attributes);
  [super dealloc];
}

- (id) initWithURL: (NSURL*)url
	    cached: (BOOL)cached
{
  NSString	*path;

  if ([url isFileURL] == NO)
    {
      NSLog(@"Attempt to init GSFileURLHandle with bad URL");
      DESTROY(self);
      return nil;
    }
  path = [url path];
  path = [path stringByStandardizingPath];

  if (cached == YES)
    {
      id	obj;

      [fileLock lock];
      NS_DURING
	{
	  obj = [fileCache objectForKey: path];
	  if (obj != nil)
	    {
	      DESTROY(self);
	      IF_NO_GC([obj retain];)
	    }
	}
      NS_HANDLER
	{
	  obj = nil;
	  [fileLock unlock];
	  [localException raise];
	}
      NS_ENDHANDLER
      [fileLock unlock];
      if (obj != nil)
	{
	  return obj;
	}
    }

  if ((self = [super initWithURL: url cached: cached]) != nil)
    {
      _path = [path copy];
      if (cached == YES)
	{
	  [fileLock lock];
	  NS_DURING
	    {
	      [fileCache setObject: self forKey: _path];
	    }
	  NS_HANDLER
	    {
	      [fileLock unlock];
	      [localException raise];
	    }
	  NS_ENDHANDLER
	  [fileLock unlock];
	}
    }
  return self;
}

- (NSData*) loadInForeground
{
  NSData	*d = [NSData dataWithContentsOfFile: _path];
  NSDictionary	*dict;

  dict = [[NSFileManager defaultManager] fileAttributesAtPath: _path
						 traverseLink: YES];
  RELEASE(_attributes);
  _attributes = [dict mutableCopy];
  [self didLoadBytes: d loadComplete: YES];
  return d;
}

/**
 * Gets file attribute information for the file represented by
 * the handle, using the same dictionary keys as the
 * <ref class="NSFileManager">NSFileManager</ref> class.
 */
- (id) propertyForKey: (NSString*)propertyKey
{
  NSDictionary	*dict;

  dict = [[NSFileManager defaultManager] fileAttributesAtPath: _path
						 traverseLink: YES];
  RELEASE(_attributes);
  _attributes = [dict mutableCopy];
  return [_attributes objectForKey: propertyKey];
}

- (id) propertyForKeyIfAvailable: (NSString*)propertyKey
{
  return [_attributes objectForKey: propertyKey];
}

- (int) setDebug: (int)flag
{
  return 0;
}

- (void) setReturnAll: (BOOL)flag
{
  return;
}

- (void) setURL: (NSURL*)newUrl
{
  return;
}

/**
 * Writes the specified data as the contents of the file
 * represented by the handle.
 */
- (BOOL) writeData: (NSData*)data
{
  if ([data writeToFile: _path atomically: YES] == YES)
    {
      ASSIGNCOPY(_data, data);
      return YES;
    }
  return NO;
}

/**
 * Changes the attributes of the file represented by this handle.
 * This method uses the same dictionary keys as the
 * <ref class="NSFileManager">NSFileManger</ref> class.
 */
- (BOOL) writeProperty: (id)propertyValue
		forKey: (NSString*)propertyKey
{
  if ([self propertyForKey: propertyKey] == nil)
    {
      return NO;	/* Not a valid file property key.	*/
    }
  [_attributes setObject: propertyValue forKey: propertyKey];
  return [[NSFileManager defaultManager] changeFileAttributes: _attributes
						       atPath: _path];
}

@end

