/* 
   NSBrowserCell.h

   Cell class for the NSBrowser

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   
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

#ifndef _GNUstep_H_NSBrowserCell
#define _GNUstep_H_NSBrowserCell
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSCell.h>

@class NSImage;

@interface NSBrowserCell : NSCell
{
  // Attributes
  NSImage *_alternateImage;
  // Think of the following ones as of two BOOL ivars
#define _browsercell_is_leaf    _cell.subclass_bool_one
#define _browsercell_is_loaded  _cell.subclass_bool_two
}

//
// Accessing Graphic Attributes 
//
+ (NSImage *)branchImage;
+ (NSImage *)highlightedBranchImage;
- (NSImage *)alternateImage;
- (void)setAlternateImage:(NSImage *)anImage;
- (NSColor *)highlightColorInView: (NSView *)controlView;

//
// Placing in the Browser Hierarchy 
//
- (BOOL)isLeaf;
- (void)setLeaf:(BOOL)flag;

//
// Determining Loaded Status 
//
- (BOOL)isLoaded;
- (void)setLoaded:(BOOL)flag;

//
// Setting State 
//
- (void)reset;
- (void)set;

@end

#endif // _GNUstep_H_NSBrowserCell
