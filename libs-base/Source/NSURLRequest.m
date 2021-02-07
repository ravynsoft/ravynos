/* Implementation for NSURLRequest for GNUstep
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

#define	EXPOSE_NSURLRequest_IVARS	1
#import "GSURLPrivate.h"
#import "GSPrivate.h"

#import "Foundation/NSCoder.h"


// Internal data storage
typedef struct {
  NSData			*body;
  NSInputStream			*bodyStream;
  NSString			*method;
  NSMutableDictionary		*headers;
  BOOL				shouldHandleCookies;
  BOOL                          debug;
  id<GSLogDelegate>             ioDelegate;
  NSURL				*URL;
  NSURL				*mainDocumentURL;
  NSURLRequestCachePolicy	cachePolicy;
  NSTimeInterval		timeoutInterval;
  NSMutableDictionary		*properties;
} Internal;
 
/* Defines to get easy access to internals from mutable/immutable
 * versions of the class and from categories.
 */
#define	this	((Internal*)(self->_NSURLRequestInternal))
#define	inst	((Internal*)(((NSURLRequest*)o)->_NSURLRequestInternal))

@interface	_GSMutableInsensitiveDictionary : NSMutableDictionary
@end

@implementation	NSURLRequest

+ (id) allocWithZone: (NSZone*)z
{
  NSURLRequest	*o = [super allocWithZone: z];

  if (o != nil)
    {
      o->_NSURLRequestInternal = NSZoneCalloc(z, 1, sizeof(Internal));
    }
  return o;
}

+ (instancetype) requestWithURL: (NSURL *)URL
{
  return [self requestWithURL: URL
		  cachePolicy: NSURLRequestUseProtocolCachePolicy
	      timeoutInterval: 60.0];
}

+ (instancetype) requestWithURL: (NSURL *)URL
                    cachePolicy: (NSURLRequestCachePolicy)cachePolicy
                timeoutInterval: (NSTimeInterval)timeoutInterval
{
  NSURLRequest	*o = [[self class] allocWithZone: NSDefaultMallocZone()];

  o = [o initWithURL: URL
	 cachePolicy: cachePolicy
     timeoutInterval: timeoutInterval];
  return AUTORELEASE(o);
}

- (NSURLRequestCachePolicy) cachePolicy
{
  return this->cachePolicy;
}

- (id) copyWithZone: (NSZone*)z
{
  NSURLRequest	*o;

  if (NSShouldRetainWithZone(self, z) == YES
    && [self isKindOfClass: [NSMutableURLRequest class]] == NO)
    {
      o = RETAIN(self);
    }
  else
    {
      o = [[self class] allocWithZone: z];
      o = [o initWithURL: [self URL]
	     cachePolicy: [self cachePolicy]
	 timeoutInterval: [self timeoutInterval]];
      if (o != nil)
        {
	  inst->properties = [this->properties mutableCopy];
	  ASSIGN(inst->mainDocumentURL, this->mainDocumentURL);
	  ASSIGN(inst->body, this->body);
	  ASSIGN(inst->bodyStream, this->bodyStream);
	  ASSIGN(inst->method, this->method);
	  inst->shouldHandleCookies = this->shouldHandleCookies;
	  inst->debug = this->debug;
	  inst->ioDelegate = this->ioDelegate;
          inst->headers = [this->headers mutableCopy];
	}
    }
  return o;
}

- (void) dealloc
{
  if (this != 0)
    {
      RELEASE(this->body);
      RELEASE(this->bodyStream);
      RELEASE(this->method);
      RELEASE(this->URL);
      RELEASE(this->mainDocumentURL);
      RELEASE(this->properties);
      RELEASE(this->headers);
      NSZoneFree([self zone], this);
    }
  [super dealloc];
}

- (NSString*) description
{
  return [NSString stringWithFormat: @"<%@ %@>",
    NSStringFromClass([self class]), [[self URL] absoluteString]];
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
// FIXME
  if ([aCoder allowsKeyedCoding])
    {
    }
  else
    {
    }
}

- (id) initWithCoder: (NSCoder*)aCoder
{
// FIXME
  if ([aCoder allowsKeyedCoding])
    {
    }
  else
    {
    }
  return self;
}

- (NSUInteger) hash
{
  return [this->URL hash];
}

- (id) init
{
  return [self initWithURL: nil];
}

- (instancetype) initWithURL: (NSURL *)URL
{
  return [self initWithURL: URL
	       cachePolicy: NSURLRequestUseProtocolCachePolicy
	   timeoutInterval: 60.0];
}

- (instancetype) initWithURL: (NSURL *)URL
                 cachePolicy: (NSURLRequestCachePolicy)cachePolicy
             timeoutInterval: (NSTimeInterval)timeoutInterval
{
  if ([URL isKindOfClass: [NSURL class]] == NO)
    {
      URL = nil;
    }
  if ((self = [super init]) != nil)
    {
      this->URL = RETAIN(URL);
      this->cachePolicy = cachePolicy;
      this->timeoutInterval = timeoutInterval;
      this->mainDocumentURL = nil;
      this->method = @"GET";
      this->shouldHandleCookies = YES;
    }
  return self;
}

- (BOOL) isEqual: (id)o
{
  if ([o isKindOfClass: [NSURLRequest class]] == NO)
    {
      return NO;
    }
  if (this->URL != inst->URL
    && [this->URL isEqual: inst->URL] == NO)
    {
      return NO;
    }
  if (this->mainDocumentURL != inst->mainDocumentURL
    && [this->mainDocumentURL isEqual: inst->mainDocumentURL] == NO)
    {
      return NO;
    }
  if (this->method != inst->method
    && [this->method isEqual: inst->method] == NO)
    {
      return NO;
    }
  if (this->body != inst->body
    && [this->body isEqual: inst->body] == NO)
    {
      return NO;
    }
  if (this->bodyStream != inst->bodyStream
    && [this->bodyStream isEqual: inst->bodyStream] == NO)
    {
      return NO;
    }
  if (this->properties != inst->properties
    && [this->properties isEqual: inst->properties] == NO)
    {
      return NO;
    }
  if (this->headers != inst->headers
    && [this->headers isEqual: inst->headers] == NO)
    {
      return NO;
    }
  return YES;
}

- (NSURL *) mainDocumentURL
{
  return this->mainDocumentURL;
}

- (id) mutableCopyWithZone: (NSZone*)z
{
  NSMutableURLRequest	*o;

  o = [NSMutableURLRequest allocWithZone: z];
  o = [o initWithURL: [self URL]
	 cachePolicy: [self cachePolicy]
     timeoutInterval: [self timeoutInterval]];
  if (o != nil)
    {
      [o setMainDocumentURL: this->mainDocumentURL];
      inst->properties = [this->properties mutableCopy];
      ASSIGN(inst->mainDocumentURL, this->mainDocumentURL);
      ASSIGN(inst->body, this->body);
      ASSIGN(inst->bodyStream, this->bodyStream);
      ASSIGN(inst->method, this->method);
      inst->shouldHandleCookies = this->shouldHandleCookies;
      inst->debug = this->debug;
      inst->headers = [this->headers mutableCopy];
    }
  return o;
}

- (int) setDebug: (int)flag
{
  int   old = this->debug;

  this->debug = flag ? YES : NO;
  return old;
}

- (id<GSLogDelegate>) setDebugLogDelegate: (id<GSLogDelegate>)d
{
  id<GSLogDelegate>     old = this->ioDelegate;

  NSAssert(nil == d || [d conformsToProtocol: @protocol(GSLogDelegate)],
    NSInvalidArgumentException);
  this->ioDelegate = d;
  return old;
}

- (NSTimeInterval) timeoutInterval
{
  return this->timeoutInterval;
}

- (NSURL *) URL
{
  return this->URL;
}

@end


@implementation NSMutableURLRequest

- (void) setCachePolicy: (NSURLRequestCachePolicy)cachePolicy
{
  this->cachePolicy = cachePolicy;
}

- (void) setMainDocumentURL: (NSURL *)URL
{
  ASSIGN(this->mainDocumentURL, URL);
}

- (void) setTimeoutInterval: (NSTimeInterval)seconds
{
  this->timeoutInterval = seconds;
}

- (void) setURL: (NSURL *)URL
{
  ASSIGN(this->URL, URL);
}

@end

@implementation NSURLRequest (NSHTTPURLRequest)

- (NSDictionary *) allHTTPHeaderFields
{
  NSDictionary	*fields;

  if (this->headers == nil)
    {
      fields = [NSDictionary dictionary];
    }
  else
    {
      fields = [NSDictionary dictionaryWithDictionary: this->headers];
    }
  return fields;
}

- (NSData *) HTTPBody
{
  return this->body;
}

- (NSInputStream *) HTTPBodyStream
{
  return this->bodyStream;
}

- (NSString *) HTTPMethod
{
  return this->method;
}

- (BOOL) HTTPShouldHandleCookies
{
  return this->shouldHandleCookies;
}

- (NSString *) valueForHTTPHeaderField: (NSString *)field
{
  return [this->headers objectForKey: field];
}

@end



@implementation NSMutableURLRequest (NSMutableHTTPURLRequest)

- (void) addValue: (NSString *)value forHTTPHeaderField: (NSString *)field
{
  NSString	*old = [self valueForHTTPHeaderField: field];

  if (old != nil)
    {
      value = [old stringByAppendingFormat: @",%@", value];
    }
  [self setValue: value forHTTPHeaderField: field];
}

- (void) setAllHTTPHeaderFields: (NSDictionary *)headerFields
{
  NSEnumerator	*enumerator = [headerFields keyEnumerator];
  NSString	*field;

  while ((field = [enumerator nextObject]) != nil)
    {
      id	value = [headerFields objectForKey: field];

      if ([value isKindOfClass: [NSString class]] == YES)
        {
	  [self setValue: (NSString*)value forHTTPHeaderField: field];
	}
    }
}

- (void) setHTTPBodyStream: (NSInputStream *)inputStream
{
  DESTROY(this->body);
  ASSIGN(this->bodyStream, inputStream);
}

- (void) setHTTPBody: (NSData *)data
{
  DESTROY(this->bodyStream);
  ASSIGNCOPY(this->body, data);
}

- (void) setHTTPMethod: (NSString *)method
{
/* NB. I checked MacOS-X 4.2, and this method actually lets you set any
 * copyable value (including non-string classes), but setting nil is
 * equivalent to resetting to the default value of 'GET'
 */
  if (method == nil)
    {
      method = @"GET";
    }
  ASSIGNCOPY(this->method, method);
}

- (void) setHTTPShouldHandleCookies: (BOOL)should
{
  this->shouldHandleCookies = should;
}

- (void) setValue: (NSString *)value forHTTPHeaderField: (NSString *)field
{
  if (this->headers == nil)
    {
      this->headers = [_GSMutableInsensitiveDictionary new];
    }
  [this->headers setObject: value forKey: field];
}

@end

@implementation	NSURLRequest (Private)

- (BOOL) _debug
{
  return this->debug;
}

- (id<GSLogDelegate>) _debugLogDelegate
{
  return this->ioDelegate;
}

- (id) _propertyForKey: (NSString*)key
{
  return [this->properties objectForKey: key];
}

- (void) _setProperty: (id)value forKey: (NSString*)key
{
  if (this->properties == nil)
    {
      this->properties = [NSMutableDictionary new];
      [this->properties setObject: value forKey: key];
    }
}
@end
