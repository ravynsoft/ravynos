/* -*-objc-*-
   NSShadow.h

   GUI implementation of a shadow effect.

   Copyright (C) 2009 Free Software Foundation, Inc.

   Author: Eric Wasylishen <ewasylishen@gmail.com>
   Date: Dec 2009

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _GNUstep_H_NSShadow
#define _GNUstep_H_NSShadow
#import <GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
#import <Foundation/NSGeometry.h>
#import <Foundation/NSObject.h>

@class NSColor;

@interface NSShadow : NSObject <NSCopying, NSCoding>
{
    NSSize _offset;
    CGFloat _radius;
    NSColor *_color;
}

- (NSSize) shadowOffset; 
- (void) setShadowOffset: (NSSize)offset;
- (CGFloat) shadowBlurRadius;  
- (void) setShadowBlurRadius: (CGFloat)radius;
- (NSColor *) shadowColor; 
- (void) setShadowColor: (NSColor *)color;
- (void) set;

@end

#endif
#endif
