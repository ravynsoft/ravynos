/** Implementation for NSError for GNUStep
   Copyright (C) 2004 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Date: May 2004

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
#define	EXPOSE_NSError_IVARS	1
#import	"Foundation/NSDictionary.h"
#import	"Foundation/NSError.h"
#import	"Foundation/NSCoder.h"

NSString* const NSFilePathErrorKey = @"NSFilePath";
NSString* const NSLocalizedDescriptionKey = @"NSLocalizedDescriptionKey";
NSString* const NSStringEncodingErrorKey = @"NSStringEncodingErrorKey";
NSString* const NSURLErrorKey = @"NSURLErrorKey";
NSString* const NSUnderlyingErrorKey = @"NSUnderlyingErrorKey";

NSString* const NSLocalizedFailureReasonErrorKey
  = @"NSLocalizedFailureReasonErrorKey";
NSString* const NSLocalizedRecoveryOptionsErrorKey
  = @"NSLocalizedRecoveryOptionsErrorKey";
NSString* const NSLocalizedRecoverySuggestionErrorKey
  = @"NSLocalizedRecoverySuggestionErrorKey";
NSString* const NSRecoveryAttempterErrorKey
  = @"NSRecoveryAttempterErrorKey";

NSString* const NSURLErrorFailingURLErrorKey = @"NSErrorFailingURLKey";
NSString* const NSURLErrorFailingURLStringErrorKey = @"NSErrorFailingURLStringKey";

NSErrorDomain const NSMACHErrorDomain = @"NSMACHErrorDomain";
NSErrorDomain const NSOSStatusErrorDomain = @"NSOSStatusErrorDomain";
NSErrorDomain const NSPOSIXErrorDomain = @"NSPOSIXErrorDomain";
NSErrorDomain const NSCocoaErrorDomain = @"NSCocoaErrorDomain";

@implementation	NSError

+ (id) errorWithDomain: (NSErrorDomain)aDomain
		  code: (NSInteger)aCode
	      userInfo: (NSDictionary*)aDictionary
{
  NSError	*e = [self allocWithZone: NSDefaultMallocZone()];

  e = [e initWithDomain: aDomain code: aCode userInfo: aDictionary];
  return AUTORELEASE(e);
}

- (NSInteger) code
{
  return _code;
}

- (id) copyWithZone: (NSZone*)z
{
  NSError	*e = [[self class] allocWithZone: z];

  e = [e initWithDomain: _domain code: _code userInfo: _userInfo];
  return e;
}

- (void) dealloc
{
  DESTROY(_domain);
  DESTROY(_userInfo);
  [super dealloc];
}

- (NSString*) description
{
  return [self localizedDescription];
}

- (NSErrorDomain) domain
{
  return _domain;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeInt: _code forKey: @"NSCode"];
      [aCoder encodeObject: _domain forKey: @"NSDomain"];
      [aCoder encodeObject: _userInfo forKey: @"NSUserInfo"];
    }
  else
    {
      [aCoder encodeValueOfObjCType: @encode(int) at: &_code];
      [aCoder encodeValueOfObjCType: @encode(id) at: &_domain];
      [aCoder encodeValueOfObjCType: @encode(id) at: &_userInfo];
    }
}

- (id) init
{
  return [self initWithDomain: nil code: 0 userInfo: nil];
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      int	c;
      id	d;
      id	u;

      c = [aCoder decodeIntForKey: @"NSCode"];
      d = [aCoder decodeObjectForKey: @"NSDomain"];
      u = [aCoder decodeObjectForKey: @"NSUserInfo"];
      self = [self initWithDomain: d code: c userInfo: u];
    }
  else
    {
      [aCoder decodeValueOfObjCType: @encode(int) at: &_code];
      [aCoder decodeValueOfObjCType: @encode(id) at: &_domain];
      [aCoder decodeValueOfObjCType: @encode(id) at: &_userInfo];
    }
  return self;
}

- (id) initWithDomain: (NSErrorDomain)aDomain
		 code: (NSInteger)aCode
	     userInfo: (NSDictionary*)aDictionary
{
  if (aDomain == nil)
    {
      NSLog(@"[%@-%@] with nil domain",
	NSStringFromClass([self class]), NSStringFromSelector(_cmd));
      DESTROY(self);
    }
  else if ((self = [super init]) != nil)
    {
      ASSIGN(_domain, aDomain);
      _code = aCode;
      ASSIGN(_userInfo, aDictionary);
    }
  return self;
}

- (NSString *) localizedDescription
{
  NSString	*desc = [_userInfo objectForKey: NSLocalizedDescriptionKey];

  if (desc == nil)
    {
      desc = [NSString stringWithFormat: @"%@ %d", _domain, _code];
    }
  return desc;
}

- (NSString *) localizedFailureReason
{
  return [_userInfo objectForKey: NSLocalizedFailureReasonErrorKey];
}

- (NSArray *) localizedRecoveryOptions
{
  return [_userInfo objectForKey: NSLocalizedRecoveryOptionsErrorKey];
}

- (NSString *) localizedRecoverySuggestion
{
  return [_userInfo objectForKey: NSLocalizedRecoverySuggestionErrorKey];
}

- (id) recoveryAttempter
{
  return [_userInfo objectForKey: NSRecoveryAttempterErrorKey];
}

- (NSDictionary*) userInfo
{
  return _userInfo;
}
@end

