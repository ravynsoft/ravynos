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

#import <Foundation/NSByteOrder.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSError.h>
#import <Foundation/NSStream.h>
#import <Foundation/NSString.h>

@class GSSocksParser;

@protocol GSSocksParserDelegate <NSObject>

- (void) parser: (GSSocksParser *)aParser
 needsMoreBytes: (NSUInteger)aLength;
- (void) parser: (GSSocksParser *)aParser
  formedRequest: (NSData *)aRequest;
- (void) parser: (GSSocksParser *)aParser
finishedWithAddress: (NSString *)anAddress
port: (NSUInteger)aPort;

- (void) parser: (GSSocksParser *)aParser
encounteredError: (NSError *)anError;

@end

@interface GSSocksParser : NSObject
{
  NSDictionary                *configuration;
  NSString                    *address;
  id<GSSocksParserDelegate>   delegate;
  NSUInteger                  port;
}

- (id) initWithConfiguration: (NSDictionary *)aConfiguration
                     address: (NSString *)anAddress
                        port: (NSUInteger)aPort;

- (id<GSSocksParserDelegate>) delegate;
- (void) setDelegate: (id<GSSocksParserDelegate>)aDelegate;

- (NSString *) address;
- (NSUInteger) port;

- (void) start;
- (void) parseNextChunk: (NSData *)aChunk;

@end

