/*
   OpalSurface.h

   Copyright (C) 2013 Free Software Foundation, Inc.

   Author: Ivan Vucica <ivan@vucica.net>
   Date: June 2013

   This file is part of GNUstep.

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

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

@interface OpalSurface : NSObject
{
  struct _gswindow_device_t * _gsWindowDevice;
  CGContextRef _backingCGContext;
  CGContextRef _x11CGContext;
}

- (id) initWithDevice: (void *)device context: (CGContextRef)ctx;
- (void *)device;
- (CGContextRef) CGContext;
- (NSSize) size;

- (CGContextRef) backingCGContext;
- (CGContextRef) x11CGContext;
- (void) handleExposeRect: (NSRect)rect;
- (BOOL) isDrawingToScreen;
@end
