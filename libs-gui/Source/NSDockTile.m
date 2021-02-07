/* Implementation of class NSDockTile
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory John Casamento <greg.casamento@gmail.com>
   Date: Sat Nov 16 21:11:06 EST 2019

   This file is part of the GNUstep Library.
   
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

#import <AppKit/NSDockTile.h>
#import <AppKit/NSView.h>

@implementation NSDockTile

- (instancetype) init
{
  self = [super init];
  if (self != nil)
    {
      NSRect rect = NSMakeRect(0,0,48,48);
      _size = rect.size;
      _contentView = [[NSView alloc] initWithFrame: rect];
      _badgeLabel = nil;
      _owner = nil;
    }
  return self;
}

- (oneway void) release
{
  RELEASE(_contentView);
  RELEASE(_badgeLabel);
  [super release];
}

- (NSView *) contentView
{
  return _contentView;
}

- (void) setContentView: (NSView *)contentView
{
  ASSIGN(_contentView, contentView);
}
  
- (NSSize) size 
{
  return _size;
}

- (id) owner
{
  return _owner;
}

- (void) setOwner: (id)owner
{
  _owner = owner; // weak...
}

- (BOOL) showsApplicationBadge
{
  return _showsApplicationBadge;
}

- (void) setShowsApplicationBadge: (BOOL)flag
{
  _showsApplicationBadge = flag;
}

- (NSString *) badgeLabel
{
  return _badgeLabel;
}

- (void) setBadgeLabel: (NSString *)label
{
  ASSIGNCOPY(_badgeLabel, label);
}

- (void) display
{
  [_contentView setNeedsDisplay: YES];
}

@end

