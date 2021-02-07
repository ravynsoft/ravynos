/* Implementation of extension methods to base additions

   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.

*/

#import "common.h"
#import "GNUstepBase/NSObject+GNUstepBase.h"
#import "GNUstepBase/NSStream+GNUstepBase.h"

/* Code used in GNUstepBase
 */
NSString * const GSStreamLocalAddressKey
  = @"GSStreamLocalAddressKey";
NSString * const GSStreamLocalPortKey
  = @"GSStreamLocalPortKey";
NSString * const GSStreamRemoteAddressKey
  = @"GSStreamRemoteAddressKey";
NSString * const GSStreamRemotePortKey
  = @"GSStreamRemotePortKey";


/* The remaining code is specific to the Apple Foundation
 */
#if	!defined(GNUSTEP)

@implementation NSStream (GNUstepBase)

+ (void) getLocalStreamsToPath: (NSString *)path 
		   inputStream: (NSInputStream **)inputStream 
		  outputStream: (NSOutputStream **)outputStream
{
  [self notImplemented: _cmd];
}

+ (void) pipeWithInputStream: (NSInputStream **)inputStream 
                outputStream: (NSOutputStream **)outputStream
{
  [self notImplemented: _cmd];
}

@end

@implementation GSServerStream

+ (id) allocWithZone: (NSZone*)z
{
  return [self notImplemented: _cmd];
}

+ (id) serverStreamToAddr: (NSString*)addr port: (NSInteger)port
{
  return [self notImplemented: _cmd];
}

+ (id) serverStreamToAddr: (NSString*)addr
{
  return [self notImplemented: _cmd];
}

- (void) acceptWithInputStream: (NSInputStream **)inputStream 
                  outputStream: (NSOutputStream **)outputStream
{
  [self notImplemented: _cmd];
}

- (id) initToAddr: (NSString*)addr port: (NSInteger)port
{
  return [self notImplemented: _cmd];
}

- (id) initToAddr: (NSString*)addr
{
  return [self notImplemented: _cmd];
}

@end
#endif

