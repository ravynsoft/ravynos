/* Definition of class NSPageController
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: 27-07-2020

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

#ifndef _NSPageController_h_GNUSTEP_GUI_INCLUDE
#define _NSPageController_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSGeometry.h>
#import <AppKit/NSViewController.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_8, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSView, NSArray, NSMutableArray;
@protocol NSPageControllerDelegate;

typedef NSString* NSPageControllerObjectIdentifier;

enum
{
 NSPageControllerTransitionStyleStackHistory,
 NSPageControllerTransitionStyleStackBook,
 NSPageControllerTransitionStyleHorizontalStrip
};
typedef NSUInteger NSPageControllerTransitionStyle;

@interface NSPageController : NSViewController 
{
  NSPageControllerTransitionStyle _transitionStyle;
  id _delegate;
  NSMutableArray *_arrangedObjects;
  NSInteger _selectedIndex;
  NSViewController *_selectedViewController;
}
  
// Set/Get properties
- (NSPageControllerTransitionStyle) transitionStyle;
- (void) setTransitionStyle: (NSPageControllerTransitionStyle)style;

- (id) delegate;
- (void) setDelegate: (id)delegate;

- (NSArray *) arrangedObjects;
- (void) setArrangedObjects: (NSArray *)array;

- (NSInteger) selectedIndex;
- (void) setSelectedIndex: (NSInteger)index;

- (NSViewController *) selectedViewController;

// Handle page transitions
- (void) navigateForwardToObject: (id)object;

- (void) completeTransition;

- (IBAction) navigateBack: (id)sender;
- (IBAction) navigateForward: (id)sender;
- (IBAction) takeSelectedIndexFrom: (id)sender; // uses integerValue from sender
@end

@protocol NSPageControllerDelegate
- (NSPageControllerObjectIdentifier) pageController: (NSPageController *)pageController identifierForObject: (id)object;
- (NSViewController *) pageController: (NSPageController *)pageController viewControllerForIdentifier: (NSPageControllerObjectIdentifier)identifier;
- (NSRect) pageController: (NSPageController *)pageController frameForObject: (id)object;
- (void) pageController: (NSPageController *)pageController prepareViewController: (NSViewController *)viewController withObject: (id)object;
- (void) pageController: (NSPageController *)pageController didTransitionToObject: (id)object;
- (void) pageControllerWillStartLiveTransition: (NSPageController *)pageController;
- (void) pageControllerDidEndLiveTransition: (NSPageController *)pageController;
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSPageController_h_GNUSTEP_GUI_INCLUDE */

