#if	defined(GNUSTEP_BASE_LIBRARY)
/*
   Copyright (C) 2005 Free Software Foundation, Inc.

   Written by: David Ayers <d.ayers@inode.at>
   Date: November 2005
   
   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA.
  
*/
#import "Testing.h"
#import "ObjectTesting.h"
#import <Foundation/Foundation.h>

#define GST_PORT @"32329"

NSFileHandle *rFH = nil;

@interface Handler : NSObject
@end
@implementation Handler
- (id)init
{
  if ((self = [super init]))
    {
      NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
      [nc addObserver: self
	  selector: @selector(connect:)
	  name: NSFileHandleConnectionAcceptedNotification
	  object: nil];
    }
  return self;
}
- (void)connect:(NSNotification *)notif
{
  NSDictionary *d = [notif userInfo];
NSLog(@"%@", notif);
  rFH = [[d objectForKey: NSFileHandleNotificationFileHandleItem] retain];
}
@end

int main()
{
  NSAutoreleasePool     *arp = [NSAutoreleasePool new];
  Handler               *h;
  NSFileHandle          *sFH;
  NSFileHandle          *cFH;
  NSData                *wData;
  NSData                *rData;
  NSDate                *limit;

  wData = [@"Socket Test" dataUsingEncoding:NSASCIIStringEncoding];
  /* Note that the above data should be short enough to fit into the
     socket send buffer otherwise we risk being blocked in this single
     threaded process.  */

  h = [[Handler new] autorelease];

  sFH = [NSFileHandle fileHandleAsServerAtAddress: @"127.0.0.1"
		       service: GST_PORT
		       protocol: @"tcp"];
  PASS([sFH isKindOfClass:[NSFileHandle class]],
       "NSFileHandle understands +fileHandleAsServerAtAddress:");

  [sFH acceptConnectionInBackgroundAndNotify];

  cFH = [NSFileHandle fileHandleAsClientAtAddress: @"127.0.0.1"
		      service: GST_PORT
		      protocol: @"tcp"];
  PASS([cFH isKindOfClass:[NSFileHandle class]],
       "NSFileHandle understands +fileHandleAsClientAtAddress:");

  [cFH writeData: wData];
  limit = [NSDate dateWithTimeIntervalSinceNow: 2.0];
  while (nil == rFH && [limit timeIntervalSinceNow] > 0.0)
    {
      [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode
                               beforeDate: limit];
    }
  PASS(rFH != nil, "NSFileHandle connection was made");

  rData = [rFH availableData];
  PASS([wData isEqual: rData],
       "NSFileHandle -writeData:/-availableData match with socket");

  [arp release]; arp = nil;
  return 0;
}
#else
int main()
{
  return 0;
}
#endif
