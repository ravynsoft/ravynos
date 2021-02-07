/* Implementation for NSCachedURLResponse for GNUstep
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
#define	EXPOSE_NSCachedURLResponse_IVARS	1
#import "GSURLPrivate.h"
#import "Foundation/NSCoder.h"

// Internal data storage
typedef struct {
  NSData			*data;
  NSURLResponse			*response;
  NSDictionary			*userInfo;
  NSURLCacheStoragePolicy	storagePolicy;
} Internal;
 
#define	this	((Internal*)(self->_NSCachedURLResponseInternal))


@implementation	NSCachedURLResponse

+ (id) allocWithZone: (NSZone*)z
{
  NSCachedURLResponse	*o = [super allocWithZone: z];

  if (o != nil)
    {
      o->_NSCachedURLResponseInternal = NSZoneMalloc(z, sizeof(Internal));
      memset(o->_NSCachedURLResponseInternal, '\0', sizeof(Internal));
    }
  return o;
}

- (id) copyWithZone: (NSZone*)z
{
  NSCachedURLResponse	*o;

  if (NSShouldRetainWithZone(self, z) == YES)
    {
      o = RETAIN(self);
    }
  else
    {
      o = [[self class] allocWithZone: z];
      o = [o initWithResponse: [self response]
			 data: [self data]
		     userInfo: [self userInfo]
		storagePolicy: [self storagePolicy]];
    }
  return o;
}

- (void) dealloc
{
  if (this != 0)
    {
      RELEASE(this->data);
      RELEASE(this->response);
      RELEASE(this->userInfo);
      NSZoneFree([self zone], this);
    }
  [super dealloc];
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

- (NSData *) data
{
  return this->data;
}

- (id) initWithResponse: (NSURLResponse *)response data: (NSData *)data
{
  return [self initWithResponse: response
			   data: data
		       userInfo: nil
		  storagePolicy: NSURLCacheStorageAllowed];
}

- (id) initWithResponse: (NSURLResponse *)response
		   data: (NSData *)data
	       userInfo: (NSDictionary *)userInfo
	  storagePolicy: (NSURLCacheStoragePolicy)storagePolicy;
{
  if ((self = [super init]) != nil)
    {
      ASSIGNCOPY(this->data, data);
      ASSIGNCOPY(this->response, response);
      ASSIGNCOPY(this->userInfo, userInfo);
      this->storagePolicy = storagePolicy;
    }
  return self;
}

- (NSURLResponse *) response
{
  return this->response;
}

- (NSURLCacheStoragePolicy) storagePolicy
{
  return this->storagePolicy;
}

- (NSDictionary *) userInfo
{
  return this->userInfo;
}

@end

