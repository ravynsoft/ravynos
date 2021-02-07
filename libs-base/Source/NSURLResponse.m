/* Implementation for NSURLResponse for GNUstep
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

#define	EXPOSE_NSURLResponse_IVARS	1
#import "GSURLPrivate.h"
#import "GSPrivate.h"

#import "Foundation/NSCoder.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSScanner.h"
#import "NSCallBacks.h"
#import "GNUstepBase/GSMime.h"


// Internal data storage
typedef struct {
  long long		expectedContentLength;
  NSURL			*URL;
  NSString		*MIMEType;
  NSString		*textEncodingName;
  NSString		*statusText;
  NSMutableDictionary	*headers; /* _GSMutableInsensitiveDictionary */
  int			statusCode;
} Internal;
 
#define	this	((Internal*)(self->_NSURLResponseInternal))
#define	inst	((Internal*)(o->_NSURLResponseInternal))


@interface	_GSMutableInsensitiveDictionary : NSMutableDictionary
@end

@implementation	NSURLResponse (Private)

- (void) _checkHeaders
{
  if (NSURLResponseUnknownLength == this->expectedContentLength)
    {
      NSString	*s= [self _valueForHTTPHeaderField: @"content-length"];

      if ([s length] > 0)
	{
	  this->expectedContentLength = [s intValue];
	}
    }

  if (nil == this->MIMEType)
    {
      GSMimeHeader	*c;
      GSMimeParser	*p;
      NSScanner		*s;
      NSString		*v;

      v = [self _valueForHTTPHeaderField: @"content-type"];
      if (v == nil)
        {
	  v = @"text/plain";	// No content type given.
	}
      s = [NSScanner scannerWithString: v];
      p = [GSMimeParser new];
      c = AUTORELEASE([[GSMimeHeader alloc] initWithName: @"content-type"
                                                   value: nil]);
      /* We just set the header body, so we know it will scan and don't need
       * to check the retrurn type.
       */
      (void)[p scanHeaderBody: s into: c];
      RELEASE(p);
      ASSIGNCOPY(this->MIMEType, [c value]);
      v = [c parameterForKey: @"charset"];
      ASSIGNCOPY(this->textEncodingName, v);
    }
}

- (void) _setHeaders: (id)headers
{
  NSEnumerator	*e;
  NSString	*v;

  if ([headers isKindOfClass: [NSDictionary class]] == YES)
    {
      NSString		*k;

      e = [(NSDictionary*)headers keyEnumerator];
      while ((k = [e nextObject]) != nil)
	{
	  v = [(NSDictionary*)headers objectForKey: k];
	  [self _setValue: v forHTTPHeaderField: k];
	}
    }
  else if ([headers isKindOfClass: [NSArray class]] == YES)
    {
      GSMimeHeader	*h;

      /* Remove existing headers matching the ones we are setting.
       */
      e = [(NSArray*)headers objectEnumerator];
      while ((h = [e nextObject]) != nil)
	{
	  NSString	*n = [h namePreservingCase: YES];

	  [this->headers removeObjectForKey: n];
	}
      /* Set new headers, joining values where we have multiple headers
       * with the same name.
       */
      e = [(NSArray*)headers objectEnumerator];
      while ((h = [e nextObject]) != nil)
        {
	  NSString	*n = [h namePreservingCase: YES];
	  NSString	*o = [this->headers objectForKey: n];
	  NSString	*v = [h fullValue];

	  if (nil != o)
	    {
	      n = [NSString stringWithFormat: @"%@, %@", o, n];
	    }
	  [self _setValue: v forHTTPHeaderField: n];
	}
    }
  [self _checkHeaders];
}
- (void) _setStatusCode: (NSInteger)code text: (NSString*)text
{
  this->statusCode = code;
  ASSIGNCOPY(this->statusText, text);
}
- (void) _setValue: (NSString *)value forHTTPHeaderField: (NSString *)field
{
  if (this->headers == 0)
    {
      this->headers = [_GSMutableInsensitiveDictionary new];
    }
  [this->headers setObject: value forKey: field];
}
- (NSString *) _valueForHTTPHeaderField: (NSString *)field
{
  return [this->headers objectForKey: field];
}
@end


@implementation	NSURLResponse

+ (id) allocWithZone: (NSZone*)z
{
  NSURLResponse	*o = [super allocWithZone: z];

  if (o != nil)
    {
      o->_NSURLResponseInternal = NSZoneCalloc(z, 1, sizeof(Internal));
    }
  return o;
}

- (id) copyWithZone: (NSZone*)z
{
  NSURLResponse	*o;

  if (NSShouldRetainWithZone(self, z) == YES)
    {
      o = RETAIN(self);
    }
  else
    {
      o = [[self class] allocWithZone: z];
      o = [o initWithURL: [self URL]
	MIMEType: [self MIMEType]
	expectedContentLength: [self expectedContentLength]
	textEncodingName: [self textEncodingName]];
      if (o != nil)
	{
	  ASSIGN(inst->statusText, this->statusText);
	  inst->statusCode = this->statusCode;
	  if (this->headers == 0)
	    {
	      inst->headers = 0;
	    }
	  else
	    {
	      inst->headers = [this->headers mutableCopy];
	    }
	}
    }
  return o;
}

- (void) dealloc
{
  if (this != 0)
    {
      RELEASE(this->URL);
      RELEASE(this->MIMEType);
      RELEASE(this->textEncodingName);
      RELEASE(this->statusText);
      RELEASE(this->headers);
      NSZoneFree([self zone], this);
    }
  [super dealloc];
}

- (NSString*) description
{
  return [NSString stringWithFormat: @"%@ { URL: %@ } { Status Code: %d, Headers %@ }", [super description], this->URL, this->statusCode, this->headers];
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

- (long long) expectedContentLength
{
  return this->expectedContentLength;
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

/**
 * Initialises the receiver with the URL, MIMEType, expected length and
 * text encoding name provided.
 */
- (id) initWithURL: (NSURL *)URL
  MIMEType: (NSString *)MIMEType
  expectedContentLength: (NSInteger)length
  textEncodingName: (NSString *)name
{
  if ((self = [super init]) != nil)
    {
      ASSIGN(this->URL, URL);
      ASSIGNCOPY(this->MIMEType, MIMEType);
      ASSIGNCOPY(this->textEncodingName, name);
      this->expectedContentLength = length;
    }
  return self;
}

- (id) initWithURL: (NSURL*)URL
	statusCode: (NSInteger)statusCode
       HTTPVersion: (NSString*)HTTPVersion
      headerFields: (NSDictionary*)headerFields
{
  self = [self initWithURL: URL
		  MIMEType: nil
     expectedContentLength: NSURLResponseUnknownLength
	  textEncodingName: nil];
  if (nil != self)
    {
      this->statusCode = statusCode;
      this->headers = [headerFields copy];
      [self _checkHeaders];
    }
  return self;
}

- (NSString *) MIMEType
{
  return this->MIMEType;
}

/**
 * Returns a suggested file name for storing the response data, with
 * suggested names being found in the following order:<br />
 * <list>
 *   <item>content-disposition header</item>
 *   <item>last path component of URL</item>
 *   <item>host name from URL</item>
 *   <item>'unknown'</item>
 * </list>
 * If possible, an extension based on the MIME type of the response
 * is also appended.<br />
 * The result should always be a valid file name.
 */
- (NSString *) suggestedFilename
{
  NSString	*disp = [self _valueForHTTPHeaderField: @"content-disposition"];
  NSString	*name = nil;

  if (disp != nil)
    {
      GSMimeParser	*p;
      GSMimeHeader	*h;
      NSScanner		*sc;

      // Try to get name from content disposition header.
      p = AUTORELEASE([GSMimeParser new]);
      h = [[GSMimeHeader alloc] initWithName: @"content-displosition"
				       value: disp];
      IF_NO_GC([h autorelease];)
      sc = [NSScanner scannerWithString: [h value]];
      if ([p scanHeaderBody: sc into: h] == YES)
        {
	  name = [h parameterForKey: @"filename"];
	  name = [name stringByDeletingPathExtension];
	}
    }

  if ([name length] == 0)
    {
      name = [[[self URL] absoluteString] lastPathComponent];
      name = [name stringByDeletingPathExtension];
    }
  if ([name length] == 0)
    {
      name = [[self URL] host];
    }
  if ([name length] == 0)
    {
      name = @"unknown";
    }
// FIXME ... add type specific extension
  return name;
}

- (NSString *) textEncodingName
{
  return this->textEncodingName;
}

- (NSURL *) URL
{
  return this->URL;
}

@end


@implementation NSHTTPURLResponse

+ (NSString *) localizedStringForStatusCode: (NSInteger)statusCode
{
// FIXME ... put real responses in here
  return [NSString stringWithFormat: @"%"PRIdPTR, statusCode];
}

- (NSDictionary *) allHeaderFields
{
  return AUTORELEASE([this->headers copy]);
}

- (NSInteger) statusCode
{
  return this->statusCode;
}
@end

