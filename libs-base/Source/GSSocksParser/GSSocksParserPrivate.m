/*
 * Parsers of SOCKS protocol messages
 * Copyright (C) 2013 Free Software Foundation, Inc.
 *
 * Written by Marat Ibadinov <ibadinov@me.com>
 * Date: 2013
 *
 * This file is part of the GNUstep Base Library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110 USA.
 *
 * $Date$ $Revision$
 */

#import "GSSocksParserPrivate.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSBundle.h"
#import "Foundation/NSCharacterSet.h"
#import "Foundation/NSException.h"
#import <stdio.h>

@interface NSString (GSSocksParser)
- (NSString *) stringByRepeatingCurrentString: (NSUInteger)times;
@end

@implementation NSString (GSSocksParser)

- (NSString *) stringByRepeatingCurrentString: (NSUInteger)times
{
  return [@"" stringByPaddingToLength: times * [self length]
                           withString: self
                      startingAtIndex: 0];
}

@end

@implementation GSSocksParser (Private)

- (NSError *) errorWithCode: (NSInteger)aCode
                description: (NSString *)aDescription
{
    NSDictionary *userInfo;

    aDescription = NSLocalizedString(aDescription, @"");
    userInfo = [NSDictionary dictionaryWithObject: aDescription
                                           forKey: NSLocalizedDescriptionKey];
    return [NSError errorWithDomain: NSStreamSOCKSErrorDomain
                               code: aCode
                           userInfo: userInfo];
}

- (GSSocksAddressType) addressType
{
  const char *cAddress;
  NSUInteger index;
  BOOL hasAlpha;
  BOOL hasDot;
  char character;

  if ([address length] > 16)
    {
      return GSSocksAddressTypeDomain;
    }
  cAddress = [address UTF8String];
  index = 0;
  hasAlpha = NO;
  hasDot = NO;

  while (0 != (character = cAddress[index]))
    {
      BOOL isAlpha = character >= 'a' && character <= 'f';

      if (!(character >= '0' && character <= '9')
        && !isAlpha && character != '.' && character != ':')
        {
          return GSSocksAddressTypeDomain;
        }
      hasAlpha    = hasAlpha  || isAlpha;
      hasDot      = hasDot    || character == '.';
      ++index;
    }
  return hasAlpha && hasDot ? GSSocksAddressTypeDomain
    : (hasDot ? GSSocksAddressTypeIPv4 : GSSocksAddressTypeIPv6);
}

- (NSData *) addressData
{
  switch ([self addressType])
    {
      case GSSocksAddressTypeIPv4:
        {
          NSMutableData *result = [NSMutableData dataWithLength: 4];
          const char *cString = [address UTF8String];
 	  int	elements[4];
          uint8_t *bytes = [result mutableBytes];

          sscanf(cString, "%d.%d.%d.%d",
            &elements[0], &elements[1], &elements[2], &elements[3]);
          bytes[0] = (uint8_t)elements[0];
          bytes[1] = (uint8_t)elements[1];
          bytes[2] = (uint8_t)elements[2];
          bytes[3] = (uint8_t)elements[3];

          return result;
        }
      case GSSocksAddressTypeIPv6:
        {
          NSArray *components = [address componentsSeparatedByString: @"::"];
          NSMutableData *result;
          uint16_t *bytes;
            
          if ([components count] == 2)
            {
              NSString *leading;
              NSString *trailing;
              NSCharacterSet *charset;
              NSArray   *separated;
              NSUInteger leadingCount;
              NSUInteger trailingCount;

              leading = [components objectAtIndex: 0];
              trailing = [components objectAtIndex: 1];
              charset
                = [NSCharacterSet characterSetWithCharactersInString: @":"];
              separated
                = [leading componentsSeparatedByCharactersInSet: charset];
              leadingCount = [leading length] ? [separated count] : 0;
/* FIXME ... do we need to add this following statement?
              separated
                = [trailing componentsSeparatedByCharactersInSet: charset];
*/
              trailingCount = [trailing length] ? [separated count] : 0;
              
              if (leadingCount && trailingCount)
                {
                  NSString *middle;

                  middle = [@"0:" stringByRepeatingCurrentString:
                    8 - leadingCount - trailingCount];
                  address = [[[leading stringByAppendingString: @":"]
                    stringByAppendingString: middle]
                      stringByAppendingString: trailing];
                }
              else if (!leadingCount)
                {
                  NSString *start;

                  start = [@"0:" stringByRepeatingCurrentString:
                    8 - trailingCount];
                  address = [start stringByAppendingString: trailing];
                }
              else
                {
                  NSString *end;

                  end = [@":0" stringByRepeatingCurrentString:
                    8 - leadingCount];
                  address = [leading stringByAppendingString: end];        
              }
          }
            
          result = [NSMutableData dataWithLength:16];
          bytes = [result mutableBytes];
          sscanf([address UTF8String], "%hx:%hx:%hx:%hx:%hx:%hx:%hx:%hx",
            &bytes[0], &bytes[1], &bytes[2], &bytes[3],
            &bytes[4], &bytes[5], &bytes[6], &bytes[7]);
          return result;
        }
      case GSSocksAddressTypeDomain:
        {
          return [address dataUsingEncoding:NSUTF8StringEncoding];
        }
      default:
        [NSException raise: NSInternalInconsistencyException
                    format: @"Unknown address type"];
        return nil;
    }
}

- (NSString *) addressFromData: (NSData *)aData
                      withType: (GSSocksAddressType)anAddressType
{
  switch (anAddressType)
    {
      case GSSocksAddressTypeIPv4:
        {
          const uint8_t *bytes = [aData bytes];

          return [NSString stringWithFormat: @"%hhu.%hhu.%hhu.%hhu",
            bytes[0], bytes[1], bytes[2], bytes[3]];
        }
      case GSSocksAddressTypeIPv6:
        {
          const uint16_t *bytes = [aData bytes];

          return [NSString stringWithFormat: @"%hx:%hx:%hx:%hx:%hx:%hx:%hx:%hx",
            bytes[0], bytes[1], bytes[2], bytes[3],
            bytes[4], bytes[5], bytes[6], bytes[7]];
        }
      case GSSocksAddressTypeDomain:
        {
          return [[[NSString alloc] initWithData: aData
            encoding: NSUTF8StringEncoding] autorelease];
        }
      default:
        [NSException raise: NSInternalInconsistencyException
                    format: @"Unknown address type"];
        return nil;
    }
}

@end
