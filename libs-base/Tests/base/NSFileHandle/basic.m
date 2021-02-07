/*
   Copyright (C) 2005 Free Software Foundation, Inc.

   Written by: David Ayers <d.ayers@inode.at>
   Date: February 2005
   
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

#import "ObjectTesting.h"
#import <Foundation/Foundation.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  test_NSObject(@"NSFileHandle", 
    [NSArray arrayWithObject:[NSFileHandle fileHandleWithStandardInput]]);
  [arp release]; arp = nil;
  return 0;
}
