/* 
   NSCachedImageRep.h

   Cached image representation.

   Copyright (C) 1996 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@colorado.edu>
   Date: Feb 1996
   
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

#ifndef _GNUstep_H_NSCachedImageRep
#define _GNUstep_H_NSCachedImageRep
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSImageRep.h>
#import <AppKit/NSGraphics.h>

@class NSWindow;

@interface NSCachedImageRep : NSImageRep
{
  // Attributes
  NSRect      _rect;
  NSWindow*   _window;
}

//
// Initializing an NSCachedImageRep 
//

/**
 * GNUstep extension
 */
- (id) initWithSize: (NSSize)aSize
	 pixelsWide: (NSInteger)pixelsWide
	 pixelsHigh: (NSInteger)pixelsHigh
	      depth: (NSWindowDepth)aDepth
	   separate: (BOOL)separate
	      alpha: (BOOL)alpha;

- (id)initWithSize:(NSSize)aSize
	     depth:(NSWindowDepth)aDepth
	  separate:(BOOL)separate
	     alpha:(BOOL)alpha;
- (id)initWithWindow:(NSWindow *)aWindow
		rect:(NSRect)aRect;

//
// Getting the Representation 
//
- (NSRect)rect;
- (NSWindow *)window;

@end

#endif // _GNUstep_H_NSCachedImageRep
