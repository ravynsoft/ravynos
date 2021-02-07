/* Implementation of class NSScrubberLayout
   Copyright (C) 2019 Free Software Foundation, Inc.
   
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

#import "AppKit/NSScrubberLayout.h"

@implementation NSScrubberLayoutAttributes 

+ (NSScrubberLayoutAttributes *) layoutAttributesForItemAtIndex: (NSInteger)index
{
  return nil;
}

- (CGFloat) alpha
{
  return 0.0;
}

- (NSRect) frame
{
  return NSZeroRect;
}

- (NSInteger) itemIndex
{
  return 0;
}

- (instancetype) copyWithZone: (NSZone *)z
{
  return nil;
}

@end

@implementation NSScrubberLayout
// Configuring
- (Class) layoutAttributesClass
{
  return nil;
}

- (NSScrubber *) scrubber
{
  return nil;
}

- (NSRect) visibleRect
{
  return NSZeroRect;
}

- (void) invalidateLayout
{
}

// Subclassing layout
- (void) prepareLayout
{
}

- (NSSize) scrubberContentSize
{
  return NSZeroSize;
}

- (NSScrubberLayoutAttributes *) layoutAttributesForItemAtIndex: (NSInteger)index
{
  return nil;
}

- (NSScrubberLayoutAttributes *) layoutAttributesForItemsInRect: (NSRect)rect
{
  return nil;
}

- (BOOL) shouldInvalidateLayoutForHighlightChange
{
  return NO;
}

- (BOOL) shouldInvalidateLayoutForSelectionChange
{
  return NO;
}

- (BOOL) shouldInvalidateLayoutForChangeFromVisibleRect: (NSRect)fromRect
                                          toVisibleRect: (NSRect)toRect
{
  return NO;
}

- (BOOL) automaticallyMirrorsInRightToLeftLayout
{
  return NO;
}

- (instancetype) init
{
  self = [super init];
  return self;
}

- (instancetype) initWithCoder: (NSCoder *)coder
{
  self = [super init];
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
}

@end

