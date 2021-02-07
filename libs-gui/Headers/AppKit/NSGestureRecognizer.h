/*
   NSGestureRecognizer.h
  
   Abstract base class for monitoring user events

   Copyright (C) 2017 Free Software Foundation, Inc.

   Author: Daniel Ferreira <dtf@stanford.edu>
   Date: 2017
   Editor: Gregory John Casamento
   Date: Thu Dec  5 12:54:49 EST 2019

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

#ifndef _GNUstep_H_NSGestureRecognizer
#define _GNUstep_H_NSGestureRecognizer

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_0, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSView, NSEvent;

@interface NSGestureRecognizer : NSObject
- (NSPoint)locationInView:(NSView *)view;
@end

@protocol NSGestureRecognizerDelegate <NSObject>
#if GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#else
@end
@interface NSObject (NSGestureRecognizerDelegate)
#endif
- (BOOL)gestureRecognizer:(NSGestureRecognizer *)gestureRecognizer
shouldAttemptToRecognizeWithEvent:(NSEvent *)event;
- (BOOL)gestureRecognizerShouldBegin:(NSGestureRecognizer *)gestureRecognizer;
- (BOOL)gestureRecognizer:(NSGestureRecognizer *)gestureRecognizer
shouldRecognizeSimultaneouslyWithGestureRecognizer:(NSGestureRecognizer *)otherGestureRecognizer;
- (BOOL)gestureRecognizer:(NSGestureRecognizer *)gestureRecognizer
shouldRequireFailureOfGestureRecognizer:(NSGestureRecognizer *)otherGestureRecognizer;
- (BOOL)gestureRecognizer:(NSGestureRecognizer *)gestureRecognizer
shouldBeRequiredToFailByGestureRecognizer:(NSGestureRecognizer *)otherGestureRecognizer;
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _GNUstep_H_NSGestureRecognizer */

