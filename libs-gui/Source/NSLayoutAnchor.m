/* Implementation of class NSLayoutAnchor
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory Casamento <greg.casamento@gmail.com>
   Date: Sat May  9 16:30:52 EDT 2020

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

#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>

#import "AppKit/NSLayoutAnchor.h"
#import "AppKit/NSLayoutConstraint.h"

@implementation NSLayoutAnchor

- (NSLayoutConstraint *) constraintEqualToAnchor: (NSLayoutAnchor *)anchor
{
  return [NSLayoutConstraint constraintWithItem: _item
                                      attribute: NSLayoutAttributeLeft
                                      relatedBy: NSLayoutRelationEqual
                                         toItem: [anchor item]
                                      attribute: NSLayoutAttributeLeft
                                     multiplier: 1.0
                                       constant: 0.0];
}

- (NSLayoutConstraint *) constraintGreaterThanOrEqualToAnchor: (NSLayoutAnchor *)anchor
{
  return [NSLayoutConstraint constraintWithItem: _item
                                      attribute: NSLayoutAttributeLeft
                                      relatedBy: NSLayoutRelationGreaterThanOrEqual
                                         toItem: [anchor item]
                                      attribute: NSLayoutAttributeLeft
                                     multiplier: 1.0
                                       constant: 0.0];
}

- (NSLayoutConstraint *) constraintLessThanOrEqualToAnchor: (NSLayoutAnchor *)anchor
{
  return [NSLayoutConstraint constraintWithItem: _item
                                      attribute: NSLayoutAttributeLeft
                                      relatedBy: NSLayoutRelationLessThanOrEqual
                                         toItem: [anchor item]
                                      attribute: NSLayoutAttributeLeft
                                     multiplier: 1.0
                                       constant: 0.0];
}

- (NSLayoutConstraint *) constraintEqualToAnchor: (NSLayoutAnchor *)anchor constant: (CGFloat)c
{
  return [NSLayoutConstraint constraintWithItem: _item
                                      attribute: NSLayoutAttributeLeft
                                      relatedBy: NSLayoutRelationEqual
                                         toItem: [anchor item]
                                      attribute: NSLayoutAttributeLeft
                                     multiplier: 1.0
                                       constant: 0.0];
}

- (NSLayoutConstraint *) constraintGreaterThanOrEqualToAnchor: (NSLayoutAnchor *)anchor constant: (CGFloat)c
{
  return [NSLayoutConstraint constraintWithItem: _item
                                      attribute: NSLayoutAttributeLeft
                                      relatedBy: NSLayoutRelationGreaterThanOrEqual
                                         toItem: [anchor item]
                                      attribute: NSLayoutAttributeLeft
                                     multiplier: 1.0
                                       constant: c];
}

- (NSLayoutConstraint *) constraintLessThanOrEqualToAnchor: (NSLayoutAnchor *)anchor constant: (CGFloat)c;
{
  return [NSLayoutConstraint constraintWithItem: _item
                                      attribute: NSLayoutAttributeLeft
                                      relatedBy: NSLayoutRelationLessThanOrEqual
                                         toItem: [anchor item]
                                      attribute: NSLayoutAttributeLeft
                                     multiplier: 1.0
                                       constant: c];
}

- (instancetype) init
{
  self = [super init];
  if (self != nil)
    {
      _name = nil;
      _item = nil;
      _hasAmbiguousLayout = NO;
      _constraintsAffectingLayout = [[NSMutableArray alloc] init];
    }
  return self;
}

- (void) dealloc
{
  RELEASE(_name);
  RELEASE(_item);
  RELEASE(_constraintsAffectingLayout);
  [super dealloc];
}

- (NSString *) name
{
  return _name;
}

- (id) item
{
  return _item;
}

- (BOOL) hasAmbiguousLayout
{
  return _hasAmbiguousLayout;
}

- (NSArray *) constraintsAffectingLayout
{
  return _constraintsAffectingLayout;
}

- (id) initWithCoder: (NSCoder *)c
{
  self = [super init];
  return self;
}

- (void) encodeWithCoder: (NSCoder *)c
{
}

- (id) copyWithZone: (NSZone *)z
{
  return nil;
}

@end

@implementation NSLayoutDimension

- (NSLayoutConstraint *) constraintEqualToConstant: (CGFloat)c
{
  return [NSLayoutConstraint constraintWithItem: _item
                                      attribute: NSLayoutAttributeLeft
                                      relatedBy: NSLayoutRelationEqual
                                         toItem: _item
                                      attribute: NSLayoutAttributeLeft
                                     multiplier: 1.0
                                       constant: c];
}

- (NSLayoutConstraint *) constraintGreaterThanOrEqualToConstant: (CGFloat)c
{
  return [NSLayoutConstraint constraintWithItem: _item
                                      attribute: NSLayoutAttributeLeft
                                      relatedBy: NSLayoutRelationGreaterThanOrEqual
                                         toItem: _item
                                      attribute: NSLayoutAttributeLeft
                                     multiplier: 1.0
                                       constant: c];
}

- (NSLayoutConstraint *) constraintLessThanOrEqualToConstant: (CGFloat)c
{
  return [NSLayoutConstraint constraintWithItem: _item
                                      attribute: NSLayoutAttributeLeft
                                      relatedBy: NSLayoutRelationLessThanOrEqual
                                         toItem: _item
                                      attribute: NSLayoutAttributeLeft
                                     multiplier: 1.0
                                       constant: c];
}

- (NSLayoutConstraint *) constraintEqualToAnchor: (NSLayoutDimension *)anchor multiplier: (CGFloat)m
{
  return [NSLayoutConstraint constraintWithItem: _item
                                      attribute: NSLayoutAttributeLeft
                                      relatedBy: NSLayoutRelationEqual
                                         toItem: [anchor item]
                                      attribute: NSLayoutAttributeLeft
                                     multiplier: m
                                       constant: 0.0];
}

- (NSLayoutConstraint *) constraintGreaterThanOrEqualToAnchor: (NSLayoutDimension *)anchor multiplier: (CGFloat)m
{
  return [NSLayoutConstraint constraintWithItem: _item
                                      attribute: NSLayoutAttributeLeft
                                      relatedBy: NSLayoutRelationGreaterThanOrEqual
                                         toItem: [anchor item]
                                      attribute: NSLayoutAttributeLeft
                                     multiplier: m
                                       constant: 0.0];
}

- (NSLayoutConstraint *) constraintLessThanOrEqualToAnchor: (NSLayoutDimension *)anchor multiplier: (CGFloat)m
{
  return [NSLayoutConstraint constraintWithItem: _item
                                      attribute: NSLayoutAttributeLeft
                                      relatedBy: NSLayoutRelationLessThanOrEqual
                                         toItem: [anchor item]
                                      attribute: NSLayoutAttributeLeft
                                     multiplier: m
                                       constant: 0.0];
}

- (NSLayoutConstraint *) constraintEqualToAnchor: (NSLayoutDimension *)anchor multiplier: (CGFloat)m constant: (CGFloat)c
{
  return [NSLayoutConstraint constraintWithItem: _item
                                      attribute: NSLayoutAttributeLeft
                                      relatedBy: NSLayoutRelationEqual
                                         toItem: [anchor item]
                                      attribute: NSLayoutAttributeLeft
                                     multiplier: m
                                       constant: c];
}

- (NSLayoutConstraint *) constraintGreaterThanOrEqualToAnchor: (NSLayoutDimension *)anchor multiplier: (CGFloat)m constant: (CGFloat)c
{
  return [NSLayoutConstraint constraintWithItem: _item
                                      attribute: NSLayoutAttributeLeft
                                      relatedBy: NSLayoutRelationGreaterThanOrEqual
                                         toItem: [anchor item]
                                      attribute: NSLayoutAttributeLeft
                                     multiplier: m
                                       constant: c];
}

- (NSLayoutConstraint *) constraintLessThanOrEqualToAnchor: (NSLayoutDimension *)anchor multiplier: (CGFloat)m constant: (CGFloat)c
{
  return [NSLayoutConstraint constraintWithItem: _item
                                      attribute: NSLayoutAttributeLeft
                                      relatedBy: NSLayoutRelationLessThanOrEqual
                                         toItem: [anchor item]
                                      attribute: NSLayoutAttributeLeft
                                     multiplier: m
                                       constant: c];
}

- (id) initWithCoder: (NSCoder *)coder
{
  self = [super init];
  if (self != nil)
    {
      NSLog(@"Decoding");
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  [super encodeWithCoder: coder];
}
@end

@implementation NSLayoutXAxisAnchor

- (NSLayoutDimension *) anchorWithOffsetToAnchor: (NSLayoutXAxisAnchor *)anchor
{
  return nil;
}

@end

@implementation NSLayoutYAxisAnchor

- (NSLayoutDimension *) anchorWithOffsetToAnchor: (NSLayoutYAxisAnchor *)anchor
{
  return nil;
}
  
@end

