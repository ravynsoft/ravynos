/* 
 NSViewController.h

 Copyright (C) 2010 Free Software Foundation, Inc.

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

#ifndef _GNUstep_H_NSViewController
#define _GNUstep_H_NSViewController
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSNibDeclarations.h>
#import <AppKit/NSResponder.h>
#import <AppKit/NSSeguePerforming.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)

@class NSArray, NSBundle, NSPointerArray, NSView, NSMapTable, NSStoryboard;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)
enum
  {
    NSViewControllerTransitionNone                  =    0x0,
    NSViewControllerTransitionCrossfade             =    0x1,   
    NSViewControllerTransitionSlideUp               =   0x10,   
    NSViewControllerTransitionSlideDown             =   0x20,   
    NSViewControllerTransitionSlideLeft             =   0x40,   
    NSViewControllerTransitionSlideRight            =   0x80,   
    NSViewControllerTransitionSlideForward          =  0x140,   
    NSViewControllerTransitionSlideBackward         =  0x180,   
    NSViewControllerTransitionAllowUserInteraction  = 0x1000,   
  };
typedef NSUInteger NSViewControllerTransitionOptions;
#endif

@interface NSViewController : NSResponder <NSSeguePerforming>
{
@private
  NSString            *_nibName;
  NSBundle            *_nibBundle;
  id                   _representedObject;
  NSString            *_title;
  IBOutlet NSView     *view;
  NSArray             *_topLevelObjects;
  NSPointerArray      *_editors;
  id                   _autounbinder;
  NSString            *_designNibBundleIdentifier;
  NSMapTable          *_segueMap;
  NSStoryboard        *_storyboard; // a weak reference to the origin storyboard.
  struct ___vcFlags 
    {
      unsigned int nib_is_loaded:1;
      unsigned int RESERVED:31;
    } _vcFlags;
  id                   _reserved;
}

- (id)initWithNibName:(NSString *)nibNameOrNil 
               bundle:(NSBundle *)nibBundleOrNil;

- (void)setRepresentedObject:(id)representedObject;
- (id)representedObject;

- (void)setTitle:(NSString *)title;
- (NSString *)title;

- (void)setView:(NSView *)aView;
- (NSView *)view;
- (void)loadView;

- (NSString *)nibName;
- (NSBundle *)nibBundle;
@end

#endif // OS_API_VERSION
#endif /* _GNUstep_H_NSViewController */
