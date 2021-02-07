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
#import "GSSocks4Parser.h"
#import "GSSocks5Parser.h"
#import "Foundation/NSException.h"

@interface NSObject (SubclassResponsibility)
- (id) subclassResponsibility: (SEL)aSelector;
@end

@implementation GSSocksParser

- (id) init
{
  if (nil != (self = [super init]))
    {
      configuration = nil;
      address = nil;
      delegate = nil;
      port = 0;
    }
  return self;
}

- (id) initWithConfiguration: (NSDictionary *)aConfiguration
                     address: (NSString *)anAddress
                        port: (NSUInteger)aPort
{
  NSString *version;
  Class concreteClass;

  version = [aConfiguration objectForKey: NSStreamSOCKSProxyVersionKey];
  version = version ? version : NSStreamSOCKSProxyVersion5;
  
  [self release];
  
  if ([version isEqualToString: NSStreamSOCKSProxyVersion5])
    {
      concreteClass = [GSSocks5Parser class];
    }
  else if ([version isEqualToString: NSStreamSOCKSProxyVersion4])
    {
      concreteClass = [GSSocks4Parser class];
    }
  else
    {
      [NSException raise: NSInternalInconsistencyException
                  format: @"Unsupported socks version: %@", version];
      return nil;       // Avoid spurious compiler warning
    }
  return [[concreteClass alloc] initWithConfiguration: aConfiguration
                                              address: anAddress 
                                                 port: aPort];
}

- (void) dealloc
{
  [delegate release];
  [address release];
  [configuration release];
  [super dealloc];
}

- (id<GSSocksParserDelegate>) delegate
{
  return delegate;
}

- (void) setDelegate: (id<GSSocksParserDelegate>)aDelegate
{
  id previous = delegate;
  delegate = [aDelegate retain];
  [previous release];
}

- (NSString *) address
{
  return address;
}

- (NSUInteger) port
{
  return port;
}

- (void) start
{
  [self subclassResponsibility:_cmd];
}

- (void) parseNextChunk: (NSData *)aChunk
{
  [self subclassResponsibility: _cmd];
}

@end
