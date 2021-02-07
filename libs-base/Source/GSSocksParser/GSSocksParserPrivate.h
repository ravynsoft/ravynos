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

#import "GSSocksParser.h"

typedef enum GSSocksAddressType {
  GSSocksAddressTypeIPv4     = 0x1,
  GSSocksAddressTypeIPv6     = 0x4,
  GSSocksAddressTypeDomain   = 0x3,
} GSSocksAddressType;

@interface GSSocksParser (Private)

- (NSError *) errorWithCode: (NSInteger)aCode
                description: (NSString *)aDescription;


- (GSSocksAddressType) addressType;

- (NSData *) addressData;
- (NSString *) addressFromData: (NSData *)aData
                      withType: (GSSocksAddressType)anAddressType;

@end

