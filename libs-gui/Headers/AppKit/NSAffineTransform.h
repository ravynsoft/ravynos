/*
   NSAffineTransform.h

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: August 1997
   Rewrite for MacOS-X compatibility: Richard Frith-Macdonald, 1999
   
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

#ifndef _GNUstep_H_NSAffineTransform
#define _GNUstep_H_NSAffineTransform
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSAffineTransform.h>

@class NSBezierPath;

@interface NSAffineTransform (GUIAdditions)

- (void) concat;
- (void) set;
- (NSBezierPath*) transformBezierPath: (NSBezierPath*)aPath;
@end

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
#if GS_API_VERSION(GS_API_NONE, 011500)
@interface NSAffineTransform (GNUstep)
- (void) translateToPoint: (NSPoint)point;
- (void) scaleTo: (CGFloat)sx : (CGFloat)sy;
- (void) makeIdentityMatrix;
- (CGFloat) rotationAngle;
- (void) setFrameOrigin: (NSPoint)point;
- (void) setFrameRotation: (CGFloat)angle;

/* Deprecated: use -invert  */
- (void) inverse;

- (BOOL) isRotated;

- (void) boundingRectFor: (NSRect)rect result: (NSRect*)newRect;

/* Returns anotherMatrix * self */
- (void) concatenateWith: (NSAffineTransform*)anotherMatrix;
- (void) concatenateWithMatrix: (const float[6])anotherMatrix;

- (NSPoint) pointInMatrixSpace: (NSPoint)point;
- (NSPoint) deltaPointInMatrixSpace: (NSPoint)point;
- (NSSize) sizeInMatrixSpace: (NSSize)size;
- (NSRect) rectInMatrixSpace: (NSRect)rect;

/* Deprecated: use -setTransformStruct: */
- (void) setMatrix: (const float[6])replace;
/* Deprecated: use -transformStruct */
- (void) getMatrix: (float[6])replace;

- (void) takeMatrixFromTransform: (NSAffineTransform *)aTransform;

@end
#endif
#endif

#endif /* _GNUstep_H_NSAffineTransform */
