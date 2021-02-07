/* Interface of class NSLayoutGuide
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

#ifndef _NSLayoutGuide_h_GNUSTEP_GUI_INCLUDE
#define _NSLayoutGuide_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>
#import <AppKit/NSUserInterfaceItemIdentification.h>
#import <AppKit/NSLayoutConstraint.h>
#import <AppKit/NSView.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSView, NSLayoutXAxisAnchor, NSLayoutYAxisAnchor, NSLayoutDimension;
  
@interface NSLayoutGuide : NSObject <NSCoding, NSUserInterfaceItemIdentification>
{
  NSRect _frame;
  NSView *_owningView;
  NSUserInterfaceItemIdentifier _identifier;
  NSLayoutXAxisAnchor *_leadingAnchor;
  NSLayoutXAxisAnchor *_trailingAnchor;
  NSLayoutXAxisAnchor *_leftAnchor;
  NSLayoutXAxisAnchor *_rightAnchor;
  NSLayoutYAxisAnchor *_topAnchor;
  NSLayoutYAxisAnchor *_bottomAnchor;
  NSLayoutDimension *_widthAnchor;
  NSLayoutDimension *_heightAnchor;
  NSLayoutXAxisAnchor *_centerXAnchor;
  NSLayoutYAxisAnchor *_centerYAnchor;

  BOOL _hasAmbiguousLayout;
}

- (NSRect) frame;

- (NSView *) owningView;
- (void) setOwningView: (NSView *)owningView;

- (NSUserInterfaceItemIdentifier) identifier;
- (void) setIdentifier: (NSUserInterfaceItemIdentifier)identifier;

- (NSLayoutXAxisAnchor *) leadingAnchor;
- (NSLayoutXAxisAnchor *) trailingAnchor;
- (NSLayoutXAxisAnchor *) leftAnchor;
- (NSLayoutXAxisAnchor *) rightAnchor;
- (NSLayoutYAxisAnchor *) topAnchor;
- (NSLayoutYAxisAnchor *) bottomAnchor;
- (NSLayoutDimension *) widthAnchor;
- (NSLayoutDimension *) heightAnchor;
- (NSLayoutXAxisAnchor *) centerXAnchor;
- (NSLayoutYAxisAnchor *) centerYAnchor;

- (BOOL) hasAmbiguousLayout;
  
- (NSArray *) constraintsAffectingLayoutForOrientation: (NSLayoutConstraintOrientation)orientation;
  
@end

@interface NSView (NSLayoutGuideSupport)

- (void) addLayoutGuide: (NSLayoutGuide *)guide;
- (void) removeLayoutGuide: (NSLayoutGuide *)guide;

- (NSArray *) layoutGuides;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSLayoutGuide_h_GNUSTEP_GUI_INCLUDE */

