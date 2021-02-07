/* 
   NSTrackingArea.h

   Track mouse movements in and out of a rectangle on screen.

   Copyright (C) 2013 Free Software Foundation, Inc.

   Written by: Gregory Casamento <greg.casamento@gmail.com>
   Date: September 2013
   
   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/ 

#import <Foundation/NSDictionary.h>
#import "AppKit/NSTrackingArea.h"
#import "GNUstepGUI/GSTrackingRect.h"

@implementation NSTrackingArea 

- (id)initWithRect: (NSRect)rect
           options: (NSTrackingAreaOptions)options
             owner: (id)owner
          userInfo: (NSDictionary *)userInfo
{
  if ((self = [super init]) != nil)
    {
	BOOL flag = (BOOL)(options & NSTrackingAssumeInside);

	_userInfo = RETAIN(_userInfo);
	_options = options;
	_trackingRect = [[GSTrackingRect alloc] initWithRect:rect
							 tag:0
						       owner:owner
						    userData:(void *)userInfo
						      inside:flag];
    }
  return self;
}

- (void) dealloc
{
  [_userInfo release];
  [_trackingRect release];
  [super dealloc];
}

- (NSTrackingAreaOptions) options
{
  return _options;
}

- (id) owner
{
  return [_trackingRect owner];
}

- (NSRect) rect
{
  return [_trackingRect rectangle];
}

- (NSDictionary *) userInfo
{
  return _userInfo;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  // [super encodeWithCoder:coder];
}

- (id) initWithCoder: (NSCoder *)coder
{
  //[super initWithCoder:coder];
  return self;
}

- (id) copyWithZone: (NSZone *)zone
{
  NSRect rect = [self rect];
  NSTrackingAreaOptions options = [self options];
  id owner = [self owner];
  id info = [self userInfo];
  NSTrackingArea *newArea = [[NSTrackingArea allocWithZone:zone]
				  initWithRect: rect
				       options: options
					 owner: owner
				      userInfo: info];

  return newArea;
}

@end
