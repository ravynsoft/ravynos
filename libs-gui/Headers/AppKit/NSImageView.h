/* -*-objc-*-
   NSImageView.h

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: January 1998
   
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

#ifndef _GNUstep_H_NSImageView
#define _GNUstep_H_NSImageView
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSControl.h>
#import <AppKit/NSImageCell.h>

@interface NSImageView : NSControl
{
  id _target;
  SEL _action;
  struct GSImageViewFlagsType { 
    // total 32 bits.  30 bits left.
    unsigned allowsCutCopyPaste: 1;
    unsigned initiatesDrag: 1;
  } _ivflags;
}

- (NSImage *)image;
- (void)setImage:(NSImage *)image;

- (NSImageAlignment)imageAlignment;
- (void)setImageAlignment:(NSImageAlignment)align;
- (NSImageScaling)imageScaling;
- (void)setImageScaling:(NSImageScaling)scaling;
- (NSImageFrameStyle)imageFrameStyle;
- (void)setImageFrameStyle:(NSImageFrameStyle)style;
- (void)setEditable:(BOOL)flag;
- (BOOL)isEditable;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (BOOL)animates;
- (void)setAnimates:(BOOL)flag;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (BOOL)allowsCutCopyPaste;
- (void)setAllowsCutCopyPaste:(BOOL)flag;
#endif

@end

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
// 
// Methods that are GNUstep extensions
//
@interface NSImageView (GNUstep)
- (BOOL)initiatesDrag;
- (void)setInitiatesDrag: (BOOL)flag;
@end
#endif
#endif /* _GNUstep_H_NSImageView */
