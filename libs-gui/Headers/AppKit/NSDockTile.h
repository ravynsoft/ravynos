/* Definition of class NSDockTile
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory Casamento <greg.casamento@gmail.com>
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

#ifndef _NSDockTile_h_GNUSTEP_GUI_INCLUDE
#define _NSDockTile_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSView, NSString;

@interface NSDockTile : NSObject
{
  NSView   *_contentView;
  NSSize    _size;
  id        _owner;
  BOOL      _showsApplicationBadge;
  NSString *_badgeLabel;
}

- (NSView *) contentView;
- (void) setContentView: (NSView *)contentView;
  
- (NSSize) size;

- (id) owner;
- (void) setOwner: (id)owner;

- (BOOL) showsApplicationBadge;
- (void) setShowsApplicationBadge: (BOOL)flag;

- (NSString *) badgeLabel;
- (void) setBadgeLabel: (NSString *)label;

- (void) display;  
  
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSDockTile_h_GNUSTEP_GUI_INCLUDE */

