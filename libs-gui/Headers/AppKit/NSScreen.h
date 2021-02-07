/* 
   NSScreen.h

   Class representing monitors

   Copyright (C) 1996, 2000 Free Software Foundation, Inc.

   Author: Scott Christley <scottc@net-community.com>
   Date: 1996

   Fixes and updates made by
   Author: Gregory John Casamento <greg.casamento@gmail.com>
   Date: 2000
   
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

#ifndef _GNUstep_H_NSScreen
#define _GNUstep_H_NSScreen
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <AppKit/NSGraphics.h>

@class NSArray;
@class NSDictionary;

@interface NSScreen : NSObject
{
@private
  NSWindowDepth        _depth;
  NSRect               _frame;
  int                  _screenNumber;
  NSWindowDepth		*_supportedWindowDepths;
  void			*_reserved;
}

/*
 * Creating NSScreen Instances
 */
+ (NSScreen*) mainScreen;
+ (NSScreen*) deepestScreen;
+ (NSArray*) screens;
+ (void) resetScreens;

/*
 * Reading Screen Information
 */
- (NSWindowDepth) depth;
- (NSRect) frame;
- (NSDictionary*) deviceDescription;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (const NSWindowDepth*) supportedWindowDepths;
- (NSRect) visibleFrame;
#endif
#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
- (int) screenNumber;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (CGFloat) userSpaceScaleFactor;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
- (CGFloat) backingScaleFactor;
#endif

@end
#endif // _GNUstep_H_NSScreen
