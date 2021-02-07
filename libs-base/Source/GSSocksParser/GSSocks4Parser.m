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

#import "GSSocks4Parser.h"
#import "GSSocksParserPrivate.h"

typedef enum GSSocks4InternalError {
  GSSocks4InternalErrorIPv6 = 0x4a
} GSSocks4InternalError;

typedef enum GSSocks4ResponseStatus {
  GSSocks4ResponseStatusAccessGranted     = 0x5a,
  GSSocks4ResponseStatusRequestRejected   = 0x5b,
  GSSocks4ResponseStatusIdentdFailed      = 0x5c,
  GSSocks4ResponseStatusUserNotConfirmed  = 0x5d,
} GSSocks4ResponseStatus;

#ifdef  __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#endif

@implementation GSSocks4Parser

- (id) initWithConfiguration: (NSDictionary *)aConfiguration
                     address: (NSString *)anAddress
                        port: (NSUInteger)aPort
{
  if (nil != (self = [super init]))
    {
      configuration = [aConfiguration retain];
      address = [anAddress retain];
      port = aPort;
    }
  return self;
}

- (void) start
{
  NSMutableData *data;
  uint8_t *bytes;
  uint8_t zero;
  NSString *user;
  GSSocksAddressType addressType;

  addressType = [self addressType];
  if (addressType == GSSocksAddressTypeIPv6)
    {
      NSError *error;

      error = [self errorWithCode: GSSocks4InternalErrorIPv6
        description: @"IPv6 addresses are not supported by SOCKS4 proxies"];
      [delegate parser: self encounteredError: error];
      return;
    }
  
  data = [NSMutableData dataWithLength: 8];
  bytes = [data mutableBytes];
  bytes[0] = 0x4;
  bytes[1] = 0x1;
  *(uint16_t *)(bytes + 2) = NSSwapHostShortToBig((uint16_t)port);
  if (addressType == GSSocksAddressTypeDomain)
    {
      bytes[4] = bytes[5] = bytes[6] = 0;
      bytes[7] = 1;
    }
  else
    {
      const uint32_t *addressBytes = [[self addressData] bytes];

      *(uint32_t *)(bytes + 4) = NSSwapHostLongToBig(*addressBytes);
    }
  zero = 0x0;
  user = [configuration objectForKey: NSStreamSOCKSProxyUserKey];
  if (user)
    {
      [data appendData: [user dataUsingEncoding: NSUTF8StringEncoding]];
      [data appendBytes: &zero length: 1];
    }
  if (addressType == GSSocksAddressTypeDomain)
    {
      [data appendData: [address dataUsingEncoding: NSUTF8StringEncoding]];
      [data appendBytes: &zero length: 1];
    }
  
  [delegate parser: self formedRequest: data];
  [delegate parser: self needsMoreBytes: 8];
}

- (NSError *) errorWithResponseStatus: (NSInteger)aStatus
{
  NSString *description;

  switch ((GSSocks4ResponseStatus)aStatus)
    {
      case GSSocks4ResponseStatusRequestRejected:
        description = @"request was rejected or the server failed to fulfil it";
        break;
      case GSSocks4ResponseStatusIdentdFailed:
        description = @"identd is not running or not reachable from the server";
        break;
      case GSSocks4ResponseStatusUserNotConfirmed:
        description
	  = @"identd could not confirm the user ID string in the request";
        break;
      default:
        description = @"unknown";
        break;
  }
  description = [NSString stringWithFormat:
    @"SOCKS4 connection failed, reason: %@", description];
  return [self errorWithCode: aStatus description: description];
}

- (void) parseNextChunk: (NSData *)aChunk
{
  NSUInteger  bndPort;
  uint32_t    addressBytes;
  NSData      *addressData;
  NSString    *bndAddress;
  const uint8_t *bytes;

  bytes = [aChunk bytes];
  if (bytes[1] != GSSocks4ResponseStatusAccessGranted)
    {
      NSError *error = [self errorWithResponseStatus:bytes[1]];
      [delegate parser:self encounteredError:error];
      return;
    }
  
  bndPort = NSSwapBigShortToHost(*(uint16_t *)(bytes + 2));
  addressBytes = NSSwapBigLongToHost(*(uint32_t *)(bytes + 4));
  addressData = [NSData dataWithBytesNoCopy: &addressBytes
                                     length: 4
                               freeWhenDone: NO];
  bndAddress = [self addressFromData: addressData
                            withType :GSSocksAddressTypeIPv4];
  
  [delegate parser: self finishedWithAddress: bndAddress port: bndPort];
}

@end

#ifdef  __clang__
#pragma GCC diagnostic pop
#endif
