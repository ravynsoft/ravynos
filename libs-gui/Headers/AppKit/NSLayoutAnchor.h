/* Interface of class NSLayoutAnchor
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

#ifndef _NSLayoutAnchor_h_GNUSTEP_GUI_INCLUDE
#define _NSLayoutAnchor_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSLayoutConstraint, NSString, NSArray;
  
@interface NSLayoutAnchor : NSObject <NSCoding, NSCopying>
{
  NSString *_name;
  id _item;
  BOOL _hasAmbiguousLayout;
  NSArray *_constraintsAffectingLayout;
}

- (NSLayoutConstraint *) constraintEqualToAnchor: (NSLayoutAnchor *)anchor;
- (NSLayoutConstraint *) constraintGreaterThanOrEqualToAnchor: (NSLayoutAnchor *)anchor;
- (NSLayoutConstraint *) constraintLessThanOrEqualToAnchor: (NSLayoutAnchor *)anchor;

- (NSLayoutConstraint *) constraintEqualToAnchor: (NSLayoutAnchor *)anchor constant: (CGFloat)c;
- (NSLayoutConstraint *) constraintGreaterThanOrEqualToAnchor: (NSLayoutAnchor *)anchor constant: (CGFloat)c;
- (NSLayoutConstraint *) constraintLessThanOrEqualToAnchor: (NSLayoutAnchor *)anchor constant: (CGFloat)c;

- (NSString *) name;

- (id) item;

- (BOOL) hasAmbiguousLayout;

- (NSArray *) constraintsAffectingLayout;
  
@end

@interface NSLayoutDimension : NSLayoutAnchor
{
}

- (NSLayoutConstraint *) constraintEqualToConstant: (CGFloat)c;
- (NSLayoutConstraint *) constraintGreaterThanOrEqualToConstant: (CGFloat)c;
- (NSLayoutConstraint *) constraintLessThanOrEqualToConstant: (CGFloat)c;

- (NSLayoutConstraint *) constraintEqualToAnchor: (NSLayoutDimension *)anchor multiplier: (CGFloat)m; 
- (NSLayoutConstraint *) constraintGreaterThanOrEqualToAnchor: (NSLayoutDimension *)anchor multiplier: (CGFloat)m;
- (NSLayoutConstraint *) constraintLessThanOrEqualToAnchor: (NSLayoutDimension *)anchor multiplier: (CGFloat)m;

- (NSLayoutConstraint *) constraintEqualToAnchor: (NSLayoutDimension *)anchor multiplier: (CGFloat)m constant: (CGFloat)c;
- (NSLayoutConstraint *) constraintGreaterThanOrEqualToAnchor: (NSLayoutDimension *)anchor multiplier: (CGFloat)m constant: (CGFloat)c;
- (NSLayoutConstraint *) constraintLessThanOrEqualToAnchor: (NSLayoutDimension *)anchor multiplier: (CGFloat)m constant: (CGFloat)c;
  
@end

@interface NSLayoutXAxisAnchor : NSLayoutAnchor

- (NSLayoutDimension *) anchorWithOffsetToAnchor: (NSLayoutXAxisAnchor *)anchor;
  
@end

@interface NSLayoutYAxisAnchor : NSLayoutAnchor

- (NSLayoutDimension *) anchorWithOffsetToAnchor: (NSLayoutYAxisAnchor *)anchor;
  
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSLayoutAnchor_h_GNUSTEP_GUI_INCLUDE */

