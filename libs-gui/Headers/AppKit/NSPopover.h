/*
   NSPopover.h

   The popover class

   Copyright (C) 2013 Free Software Foundation, Inc.

   Author:  Gregory Casamento <greg.casamento@gmail.com>
   Date: 2013

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
#ifndef _GNUstep_H_NSPopover
#define _GNUstep_H_NSPopover

#import <Foundation/NSGeometry.h>
#import <AppKit/NSNibDeclarations.h>
#import <AppKit/NSResponder.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
/* Keys */
APPKIT_EXPORT NSString *NSPopoverCloseReasonKey;
APPKIT_EXPORT NSString *NSPopoverCloseReasonStandard;
APPKIT_EXPORT NSString *NSPopoverCloseReasonDetachToWindow;

/* Notifications */
APPKIT_EXPORT NSString *NSPopoverWillShowNotification;
APPKIT_EXPORT NSString *NSPopoverDidShowNotification;
APPKIT_EXPORT NSString *NSPopoverWillCloseNotification;
APPKIT_EXPORT NSString *NSPopoverDidCloseNotification;

/* Constants and enums */
enum {
   NSPopoverAppearanceMinimal = 0,
   NSPopoverAppearanceHUD = 1
};
typedef NSInteger NSPopoverAppearance;

enum {
   NSPopoverBehaviorApplicationDefined = 0,
   NSPopoverBehaviorTransient = 1,
   NSPopoverBehaviorSemitransient = 2
};
typedef NSInteger NSPopoverBehavior;

/* Forward declarations */
@class NSViewController, NSWindow, NSView, NSNotification;
@protocol NSPopoverDelegate;

/* Class */
@interface NSPopover : NSResponder
{
  BOOL _animates;
  NSPopoverAppearance _appearance;
  NSPopoverBehavior _behavior;
  NSSize _contentSize;
  IBOutlet NSViewController *_contentViewController;
  id _delegate;
  NSRect _positioningRect;
  BOOL _shown;

  NSWindow *_realWindow;
}

/* Properties */
- (void)setAnimates:(BOOL)flag;
- (BOOL)animates;
- (void)setAppearance: (NSPopoverAppearance)value;
- (NSPopoverAppearance)appearance;
- (void)setBehavior:(NSPopoverBehavior)value;
- (NSPopoverBehavior)behavior;
- (void)setContentSize:(NSSize)value;
- (NSSize)contentSize;
- (void)setContentViewController:(NSViewController *)controller;
- (NSViewController *)contentViewController;
- (void)setDelegate:(id)value;
- (id)delegate;
- (void)setPositioningRect:(NSRect)value;
- (NSRect)positioningRect;
- (BOOL)isShown;

/* Methods */
- (void)close;
- (IBAction)performClose:(id)sender;
- (void)showRelativeToRect:(NSRect)positioningRect
		    ofView:(NSView *)positioningView 
	     preferredEdge:(NSRectEdge)preferredEdge;
@end

/* Delegate */
@protocol NSPopoverDelegate
#if GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#else
@end
@interface NSObject (NSPopoverDelegate)
#endif
- (NSWindow *)detachableWindowForPopover:(NSPopover *)popover;
- (void)popoverDidClose:(NSNotification *)notification;
- (void)popoverDidShow:(NSNotification *)notification;
- (BOOL)popoverShouldClose:(NSPopover *)popover;
- (void)popoverWillClose:(NSNotification *)notification;
- (void)popoverWillShow:(NSNotification *)notification;
@end

#endif
#endif
