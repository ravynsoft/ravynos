/* Implementation of class NSLayoutGuide
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory Casamento <greg.casamento@gmail.com>
   Date: Sat May  9 16:30:36 EDT 2020

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

#import "AppKit/NSLayoutGuide.h"

@implementation NSLayoutGuide

- (NSRect) frame
{
  return _frame;
}

- (NSView *) owningView
{
  return _owningView;
}

- (void) setOwningView: (NSView *)owningView
{
  _owningView = owningView; // weak
}

- (NSUserInterfaceItemIdentifier) identifier
{
  return _identifier;
}

- (void) setIdentifier: (NSUserInterfaceItemIdentifier)identifier
{
  _identifier = identifier;
}

- (NSLayoutXAxisAnchor *) leadingAnchor
{
  return _leadingAnchor;
}

- (NSLayoutXAxisAnchor *) trailingAnchor
{
  return _trailingAnchor;
}

- (NSLayoutXAxisAnchor *) leftAnchor
{
  return _leftAnchor;
}

- (NSLayoutXAxisAnchor *) rightAnchor
{
  return _rightAnchor;
}

- (NSLayoutYAxisAnchor *) topAnchor
{
  return _topAnchor;
}

- (NSLayoutYAxisAnchor *) bottomAnchor
{
  return _bottomAnchor;
}

- (NSLayoutDimension *) widthAnchor
{
  return _widthAnchor;
}

- (NSLayoutDimension *) heightAnchor
{
  return _heightAnchor;
}

- (NSLayoutXAxisAnchor *) centerXAnchor
{
  return _centerXAnchor;
}

- (NSLayoutYAxisAnchor *) centerYAnchor
{
  return _centerYAnchor;
}

- (BOOL) hasAmbiguousLayout
{
  return _hasAmbiguousLayout;
}
  
- (NSArray *) constraintsAffectingLayoutForOrientation: (NSLayoutConstraintOrientation)orientation
{
  return [NSArray array];
}

- (instancetype) init
{
  self = [super init];
  if (self != nil)
    {
      _frame = NSZeroRect;
    }
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

