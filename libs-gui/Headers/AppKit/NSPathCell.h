/* Definition of class NSPathCell
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Wed Apr 22 18:19:07 EDT 2020

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

#ifndef _NSPathCell_h_GNUSTEP_GUI_INCLUDE
#define _NSPathCell_h_GNUSTEP_GUI_INCLUDE

#import <AppKit/NSActionCell.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

enum {
  NSPathStyleStandard,
  NSPathStyleNavigationBar,  // deprecated
  NSPathStylePopUp
};
typedef NSUInteger NSPathStyle;

@protocol NSPathCellDelegate;

@class NSEvent, NSView, NSArray, NSString, NSAttributeString, NSColor, NSPathComponentCell, NSOpenPanel, NSURL;    
  
@interface NSPathCell : NSActionCell
{
  NSPathStyle _pathStyle;
  NSColor *_backgroundColor;
  NSAttributedString *_placeholderAttributedString;
  NSArray *_allowedTypes;
  id<NSPathCellDelegate> _delegate;
  NSURL *_url;
  SEL _doubleAction;
  NSArray *_pathComponentCells;
  NSPathComponentCell *_clickedPathComponentCell;
  id _objectValue;
}
  
- (void)mouseEntered:(NSEvent *)event 
           withFrame:(NSRect)frame 
              inView:(NSView *)view;

- (void)mouseExited:(NSEvent *)event 
          withFrame:(NSRect)frame 
             inView:(NSView *)view;

- (void) setAllowedTypes: (NSArray *)types;
- (NSArray *) allowedTypes;

- (NSPathStyle) pathStyle;
- (void) setPathStyle: (NSPathStyle)pathStyle;

- (NSAttributedString *) placeholderAttributedString;
- (void) setPlaceholderAttributedString: (NSAttributedString *)string;

- (NSString *) placeholderString;
- (void) setPlaceholderString: (NSString *)string;

- (NSColor *) backgroundColor;
- (void) setBackgroundColor: (NSColor *)color;

+ (Class) pathComponentCellClass;
+ (void) setPathComponentCellClass: (Class)clz;

- (NSRect)rectOfPathComponentCell:(NSPathComponentCell *)cell 
                        withFrame:(NSRect)frame 
                           inView:(NSView *)view;

- (NSPathComponentCell *)pathComponentCellAtPoint:(NSPoint)point 
                                        withFrame:(NSRect)frame 
                                           inView:(NSView *)view;

- (NSPathComponentCell *) clickedPathComponentCell;

- (NSArray *) pathComponentCells;
- (void) setPathComponentCells: (NSArray *)cells;

- (SEL) doubleAction;
- (void) setDoubleAction: (SEL)action;

- (NSURL *) URL;
- (void) setURL: (NSURL *)url;

- (id<NSPathCellDelegate>) delegate;
- (void) setDelegate: (id<NSPathCellDelegate>)delegate;

@end

@protocol NSPathCellDelegate

- (void)pathCell:(NSPathCell *)pathCell 
        willDisplayOpenPanel:(NSOpenPanel *)openPanel;

- (void)pathCell:(NSPathCell *)pathCell 
   willPopUpMenu:(NSMenu *)menu;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSPathCell_h_GNUSTEP_GUI_INCLUDE */

