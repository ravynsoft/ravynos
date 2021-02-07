/* Definition of class NSScrubberLayout
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Wed Apr  8 09:20:18 EDT 2020

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#ifndef _NSScrubberLayout_h_GNUSTEP_GUI_INCLUDE
#define _NSScrubberLayout_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSGeometry.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_12, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSScrubber; 
  
@interface NSScrubberLayoutAttributes : NSObject <NSCopying>

+ (NSScrubberLayoutAttributes *) layoutAttributesForItemAtIndex: (NSInteger)index;

- (CGFloat) alpha;

- (NSRect) frame;

- (NSInteger) itemIndex;

@end
  
@interface NSScrubberLayout : NSObject <NSCoding>

// Configuring
- (Class) layoutAttributesClass;

- (NSScrubber *) scrubber;

- (NSRect) visibleRect;

- (void) invalidateLayout;

// Subclassing layout
- (void) prepareLayout;

- (NSSize) scrubberContentSize;

- (NSScrubberLayoutAttributes *) layoutAttributesForItemAtIndex: (NSInteger)index;

- (NSScrubberLayoutAttributes *) layoutAttributesForItemsInRect: (NSRect)rect;

- (BOOL) shouldInvalidateLayoutForHighlightChange;

- (BOOL) shouldInvalidateLayoutForSelectionChange;

- (BOOL) shouldInvalidateLayoutForChangeFromVisibleRect: (NSRect)fromRect
                                          toVisibleRect: (NSRect)toRect;

- (BOOL) automaticallyMirrorsInRightToLeftLayout;
  
@end



  
#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSScrubberLayout_h_GNUSTEP_GUI_INCLUDE */

