/* Implementation for NSURLConnection for GNUstep
   Copyright (C) 2006 Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
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

#define	EXPOSE_NSURLConnection_IVARS	1
#import "Foundation/NSError.h"
#import "Foundation/NSURLError.h"
#import "Foundation/NSRunLoop.h"
#import "GSURLPrivate.h"

@interface _NSURLConnectionDataCollector : NSObject
{
  NSURLConnection	*_connection;	// Not retained
  NSMutableData		*_data;
  NSError		*_error;
  NSURLResponse		*_response;
  BOOL			_done;
}

- (NSData*) data;
- (BOOL) done;
- (NSError*) error;
- (NSURLResponse*) response;
- (void) setConnection: (NSURLConnection *)c;

@end

@implementation _NSURLConnectionDataCollector

- (void) dealloc
{
  [_data release];
  [_error release];
  [_response release];
  [super dealloc];
}

- (BOOL) done
{
  return _done;
}

- (NSData*) data
{
  return _data;
}

- (id) init
{
  if (nil != (self = [super init]))
    {
      _data = [NSMutableData new];      // Empty data unless we get an error
    }
  return self;
}

- (NSError*) error
{
  return _error;
}

- (NSURLResponse*) response
{
  return _response;
}

- (void) setConnection: (NSURLConnection*)c
{
  _connection = c;	// Not retained ... the connection retains us
}

- (void) connection: (NSURLConnection *)connection
   didFailWithError: (NSError *)error
{
  ASSIGN(_error, error);
  DESTROY(_data);       // On error, we make the data nil
  _done = YES;
}

- (void) connection: (NSURLConnection *)connection
 didReceiveResponse: (NSURLResponse*)response
{
  ASSIGN(_response, response);
}

- (void) connectionDidFinishLoading: (NSURLConnection *)connection
{
  _done = YES;
}

- (void) connection: (NSURLConnection *)connection
     didReceiveData: (NSData *)data
{
  [_data appendData: data];
}

@end

typedef struct
{
  NSMutableURLRequest		*_request;
  NSURLProtocol			*_protocol;
  id				_delegate;
  BOOL				_debug;
} Internal;
 
#define	this	((Internal*)(self->_NSURLConnectionInternal))
#define	inst	((Internal*)(o->_NSURLConnectionInternal))

@implementation	NSURLConnection

+ (id) allocWithZone: (NSZone*)z
{
  NSURLConnection	*o = [super allocWithZone: z];

  if (o != nil)
    {
      o->_NSURLConnectionInternal = NSZoneCalloc([self zone],
	1, sizeof(Internal));
    }
  return o;
}

+ (BOOL) canHandleRequest: (NSURLRequest *)request
{
  return ([NSURLProtocol _classToHandleRequest: request] != nil);
}

+ (NSURLConnection *) connectionWithRequest: (NSURLRequest *)request
				   delegate: (id)delegate
{
  NSURLConnection	*o = [self alloc];

  o = [o initWithRequest: request delegate: delegate];
  return AUTORELEASE(o);
}

- (void) start
{
  [this->_protocol startLoading];
}

- (void) cancel
{
  [this->_protocol stopLoading];
  DESTROY(this->_protocol);
  DESTROY(this->_delegate);
}

- (id) delegate
{
  return this->_delegate;
}

- (void) scheduleInRunLoop: (NSRunLoop *)aRunLoop 
                   forMode: (NSRunLoopMode)mode
{
  NSArray *modes = [NSArray arrayWithObject: mode];
  [aRunLoop performSelector: @selector(start)
                     target: self
                   argument: nil
                      order: 0
                      modes: modes];
}

- (void) unscheduleFromRunLoop: (NSRunLoop *)aRunLoop 
                       forMode: (NSRunLoopMode)mode
{
  [aRunLoop cancelPerformSelector: @selector(start)
                           target: self
                         argument: nil];
}

- (void) dealloc
{
  if (this != 0)
    {
      [self cancel];
      DESTROY(this->_request);
      DESTROY(this->_delegate);
      NSZoneFree([self zone], this);
      _NSURLConnectionInternal = 0;
    }
  [super dealloc];
}

- (void) finalize
{
  if (this != 0)
    {
      [self cancel];
    }
}

- (id) initWithRequest: (NSURLRequest *)request
              delegate: (id)delegate
      startImmediately: (BOOL)startImmediately
{
  if ((self = [super init]) != nil)
    {
      this->_request = [request mutableCopyWithZone: [self zone]];

      /* Enrich the request with the appropriate HTTP cookies,
       * if desired.
       */
      if ([this->_request HTTPShouldHandleCookies] == YES)
	{
	  NSArray *cookies;

	  cookies = [[NSHTTPCookieStorage sharedHTTPCookieStorage]
	    cookiesForURL: [this->_request URL]];
	  if ([cookies count] > 0)
	    {
	      NSDictionary	*headers;
	      NSEnumerator	*enumerator;
	      NSString		*header;

	      headers = [NSHTTPCookie requestHeaderFieldsWithCookies: cookies];
	      enumerator = [headers keyEnumerator];
	      while (nil != (header = [enumerator nextObject]))
		{
		  [this->_request addValue: [headers valueForKey: header]
			forHTTPHeaderField: header];
		}
	    }
	}

      /* According to bug #35686, Cocoa has a bizarre deviation from the
       * convention that delegates are retained here.
       * For compatibility we retain the delegate and release it again
       * when the operation is over.
       */
      this->_delegate = [delegate retain];
      this->_protocol = [[NSURLProtocol alloc]
	initWithRequest: this->_request
	cachedResponse: nil
	client: (id<NSURLProtocolClient>)self];
      if (startImmediately == YES)
        {
          [this->_protocol startLoading];
        }
      this->_debug = GSDebugSet(@"NSURLConnection");
    }
  return self;
}

- (id) initWithRequest: (NSURLRequest *)request delegate: (id)delegate
{
  return [self initWithRequest: request
                      delegate: delegate
              startImmediately: YES];
}

@end



@implementation NSObject (NSURLConnectionDelegate)

- (void) connection: (NSURLConnection *)connection
  didCancelAuthenticationChallenge: (NSURLAuthenticationChallenge *)challenge
{
  return;
}

- (void) connection: (NSURLConnection *)connection
   didFailWithError: (NSError *)error
{
  return;
}

- (void) connectionDidFinishLoading: (NSURLConnection *)connection
{
  return;
}

- (void) connection: (NSURLConnection *)connection
  didReceiveAuthenticationChallenge: (NSURLAuthenticationChallenge *)challenge
{
  if ([challenge proposedCredential] == nil
    || [challenge previousFailureCount] > 0)
    {
      /* continue without a credential if there is no proposed credential
       * at all or if an authentication failure has already happened.
       */
      [[challenge sender]
        continueWithoutCredentialForAuthenticationChallenge: challenge];
    }
}

- (void) connection: (NSURLConnection *)connection
     didReceiveData: (NSData *)data
{
  return;
}

- (void) connection: (NSURLConnection *)connection
 didReceiveResponse: (NSURLResponse *)response
{
  return;
}

- (NSCachedURLResponse *) connection: (NSURLConnection *)connection
  willCacheResponse: (NSCachedURLResponse *)cachedResponse
{
  return cachedResponse;
}

- (NSURLRequest *) connection: (NSURLConnection *)connection
	      willSendRequest: (NSURLRequest *)request
	     redirectResponse: (NSURLResponse *)response
{
  return request;
}

@end



@implementation NSURLConnection (NSURLConnectionSynchronousLoading)

+ (NSData *) sendSynchronousRequest: (NSURLRequest *)request
		  returningResponse: (NSURLResponse **)response
			      error: (NSError **)error
{
  NSData	*data = nil;

  if (0 != response)
    {
      *response = nil;
    }
  if (0 != error)
    {
      *error = nil;
    }
  if ([self canHandleRequest: request] == YES)
    {
      _NSURLConnectionDataCollector	*collector;
      NSURLConnection			*conn;

      collector = [_NSURLConnectionDataCollector new];
      conn = [[self alloc] initWithRequest: request delegate: collector];
      if (nil != conn)
        {
          NSRunLoop	*loop;
          NSDate	*limit;

          [collector setConnection: conn];
          loop = [NSRunLoop currentRunLoop];
          limit = [[NSDate alloc] initWithTimeIntervalSinceNow:
            [request timeoutInterval]];

          while ([collector done] == NO && [limit timeIntervalSinceNow] > 0.0)
            {
              [loop runMode: NSDefaultRunLoopMode beforeDate: limit];
            }
          [limit release];
          if (NO == [collector done])
            {
              data = nil;
              if (0 != response)
                {
                  *response = nil;
                }
              if (0 != error)
                {
                  *error = [NSError errorWithDomain: NSURLErrorDomain
                                               code: NSURLErrorTimedOut
                                           userInfo: nil];
                }
            }
          else
            {
              data = [[[collector data] retain] autorelease];
              if (0 != response)
                {
                  *response = [[[collector response] retain] autorelease];
                }
              if (0 != error)
                {
                  *error = [[[collector error] retain] autorelease];
                }
            }
	  /* Cancel to prevent the NSURLProtocol instance retaining us.
	   */
	  [conn cancel];
          [conn release];
        }
      [collector release];
    }
  return data;
}

@end


@implementation	NSURLConnection (URLProtocolClient)

- (void) URLProtocol: (NSURLProtocol *)protocol
  cachedResponseIsValid: (NSCachedURLResponse *)cachedResponse
{
  return;
}

- (void) URLProtocol: (NSURLProtocol *)protocol
    didFailWithError: (NSError *)error
{
  id    o = this->_delegate;

  this->_delegate = nil;
  [o connection: self didFailWithError: error];
  DESTROY(o);
}

- (void) URLProtocol: (NSURLProtocol *)protocol
	 didLoadData: (NSData *)data
{
  [this->_delegate connection: self didReceiveData: data];
}

- (void) URLProtocol: (NSURLProtocol *)protocol
  didReceiveAuthenticationChallenge: (NSURLAuthenticationChallenge *)challenge
{
  [this->_delegate connection: self
    didReceiveAuthenticationChallenge: challenge];
}

- (void) URLProtocol: (NSURLProtocol *)protocol
  didReceiveResponse: (NSURLResponse *)response
  cacheStoragePolicy: (NSURLCacheStoragePolicy)policy
{
  [this->_delegate connection: self didReceiveResponse: response];
  if (policy == NSURLCacheStorageAllowed
    || policy == NSURLCacheStorageAllowedInMemoryOnly)
    {
      // FIXME ... cache response here?
    }
}

- (void) URLProtocol: (NSURLProtocol *)protocol
  wasRedirectedToRequest: (NSURLRequest *)request
  redirectResponse: (NSURLResponse *)redirectResponse
{
  if (this->_debug)
    {
      NSLog(@"%@ tell delegate %@ about redirect to %@ as a result of %@",
        self, this->_delegate, request, redirectResponse);
    }
  request = [this->_delegate connection: self
			willSendRequest: request
		       redirectResponse: redirectResponse];
  if (this->_protocol == nil)
    {
      if (this->_debug)
	{
          NSLog(@"%@ delegate cancelled request", self);
	}
      /* Our protocol is nil, so we have been cancelled by the delegate.
       */
      return;
    }
  if (request != nil)
    {
      if (this->_debug)
	{
          NSLog(@"%@ delegate allowed redirect to %@", self, request);
	}
      /* Follow the redirect ... stop the old load and start a new one.
       */
      [this->_protocol stopLoading];
      DESTROY(this->_protocol);
      ASSIGNCOPY(this->_request, request);
      this->_protocol = [[NSURLProtocol alloc]
	initWithRequest: this->_request
	cachedResponse: nil
	client: (id<NSURLProtocolClient>)self];
      [this->_protocol startLoading];
    }
  else if (this->_debug)
    {
      NSLog(@"%@ delegate cancelled redirect", self);
    }
}

- (void) URLProtocolDidFinishLoading: (NSURLProtocol *)protocol
{
  id    o = this->_delegate;

  this->_delegate = nil;
  [o connectionDidFinishLoading: self];
  DESTROY(o);
}

- (void) URLProtocol: (NSURLProtocol *)protocol
  didCancelAuthenticationChallenge: (NSURLAuthenticationChallenge *)challenge
{
  [this->_delegate connection: self
    didCancelAuthenticationChallenge: challenge];
}

@end

