/* -*-objc-*-
   NSGradient.h

   GUI implementation of a colour gradient.

   Copyright (C) 2009 Free Software Foundation, Inc.

   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: Oct 2009
   
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

#ifndef _GNUstep_H_NSGradient
#define _GNUstep_H_NSGradient
#import <GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
#import <Foundation/NSGeometry.h>
#import <Foundation/NSObject.h>

@class NSArray;
@class NSBezierPath;
@class NSColor;
@class NSColorSpace;

typedef NSUInteger NSGradientDrawingOptions;

enum {
  NSGradientDrawsBeforeStartingLocation = 1,
  NSGradientDrawsAfterEndingLocation = 2
};

@interface NSGradient : NSObject <NSCopying, NSCoding>
{
  NSColorSpace *_colorSpace;
  NSArray *_colors;
  CGFloat *_locations;
  NSInteger _numberOfColorStops;
}

- (NSColorSpace *) colorSpace; 
- (void) drawFromCenter: (NSPoint)startCenter 
                 radius: (CGFloat)startRadius
               toCenter: (NSPoint)endCenter 
                 radius: (CGFloat)endRadius
                options: (NSGradientDrawingOptions)options;
- (void) drawFromPoint: (NSPoint)startPoint 
               toPoint: (NSPoint)endPoint
               options: (NSGradientDrawingOptions)options;
- (void) drawInBezierPath: (NSBezierPath *)path angle: (CGFloat)angle;
- (void) drawInBezierPath: (NSBezierPath *)path 
   relativeCenterPosition: (NSPoint)relativeCenterPoint;
- (void) drawInRect: (NSRect)rect angle: (CGFloat)angle;
- (void) drawInRect: (NSRect)rect relativeCenterPosition: (NSPoint)relativeCenterPoint;
- (void) getColor: (NSColor **)color 
         location: (CGFloat *)locatuib
          atIndex: (NSInteger)index;
- (id) initWithColors: (NSArray *)colArray;
- (id) initWithColors: (NSArray *)colArray 
          atLocations: (const CGFloat *)locs 
           colorSpace: (NSColorSpace *)colSpace;
- (id) initWithColorsAndLocations: (NSColor *)color, ...;
- (id) initWithStartingColor: (NSColor *)startColor endingColor: (NSColor *)endColor;
- (NSColor *) interpolatedColorAtLocation: (CGFloat)location;
- (NSInteger) numberOfColorStops;

@end

#endif
#endif // _GNUstep_H_NSGradient
