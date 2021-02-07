/* 
   NSRulerMarker.h

   Displays a symbol in a NSRulerView.

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Michael Hanni <mhanni@sprintmail.com>
   Date: Feb 1999
   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: Sept 2001
   
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

#ifndef _GNUstep_H_NSRulerMarker
#define _GNUstep_H_NSRulerMarker

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

@class NSRulerView;
@class NSImage;
@class NSEvent;

@interface NSRulerMarker : NSObject <NSCopying, NSCoding>
{
  NSRulerView *_rulerView;
  NSImage *_image;
  id <NSCopying> _representedObject;
  NSPoint _imageOrigin;
  CGFloat _location;
  BOOL _isMovable;
  BOOL _isRemovable;
  BOOL _isDragging;
}

- (id)initWithRulerView:(NSRulerView *)aRulerView
         markerLocation:(CGFloat)location
		  image:(NSImage *)anImage
	    imageOrigin:(NSPoint)imageOrigin; 

- (NSRulerView *)ruler; 

- (void)setImage:(NSImage *)anImage; 
- (NSImage *)image;

- (void)setImageOrigin:(NSPoint)aPoint; 
- (NSPoint)imageOrigin; 
- (NSRect)imageRectInRuler; 
- (CGFloat)thicknessRequiredInRuler; 

- (void)setMovable:(BOOL)flag;
- (BOOL)isMovable; 
- (void)setRemovable:(BOOL)flag; 
- (BOOL)isRemovable; 

- (void)setMarkerLocation:(CGFloat)location; 
- (CGFloat)markerLocation; 

- (void)setRepresentedObject:(id <NSCopying>)anObject; 
- (id <NSCopying>)representedObject;

- (void)drawRect:(NSRect)aRect;
- (BOOL)isDragging; 
- (BOOL)trackMouse:(NSEvent *)theEvent adding:(BOOL)adding; 

@end

#endif /* _GNUstep_H_NSRulerMarker */
