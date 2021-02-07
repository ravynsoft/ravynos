/*
   GMAppKit.h

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: November 1997
   
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

#ifndef _GNUstep_H_GMAppKit
#define _GNUstep_H_GMAppKit

#ifndef GNUSTEP
#import <AppKit/AppKit.h>
#else
#import <AppKit/NSApplication.h>
#import <AppKit/NSBox.h>
#import <AppKit/NSButton.h>
#import <AppKit/NSCell.h>
#import <AppKit/NSClipView.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSControl.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSMatrix.h>
#import <AppKit/NSMenuItem.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSPopUpButton.h>
#import <AppKit/NSResponder.h>
#import <AppKit/NSTextField.h>
#import <AppKit/NSScrollView.h>
#import <AppKit/NSSecureTextField.h>
#import <AppKit/NSView.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSPanel.h>
#import <AppKit/NSSavePanel.h>
#import <AppKit/NSOpenPanel.h>
#import <AppKit/NSBrowser.h>
#import <AppKit/NSColorWell.h>
#import <AppKit/NSImageView.h>
#import <AppKit/NSSliderCell.h>
#import <AppKit/NSTextFieldCell.h>
#import <AppKit/NSFormCell.h>
#import <AppKit/NSText.h>
#import <AppKit/NSTextView.h>
#endif
#import <GNUstepGUI/GMArchiver.h>

@interface NSApplication (GMArchiverMethods) <ModelCoding>
@end

@interface NSBox (GMArchiverMethods) <ModelCoding>
@end

@interface NSButton (GMArchiverMethods) <ModelCoding>
@end

@interface NSCell (GMArchiverMethods) <ModelCoding>
@end

@interface NSClipView (GMArchiverMethods) <ModelCoding>
@end

@interface NSColor (GMArchiverMethods) <ModelCoding>
@end

@interface NSControl (GMArchiverMethods) <ModelCoding>
@end

@interface NSFont (GMArchiverMethods) <ModelCoding>
@end

@interface NSImage (GMArchiverMethods) <ModelCoding>
@end

@interface NSMatrix (GMArchiverMethods) <ModelCoding>
@end

@interface NSMenuItem (GMArchiverMethods) <ModelCoding>
@end

@interface NSMenu (GMArchiverMethods) <ModelCoding>
@end

@interface NSPopUpButton (GMArchiverMethods) <ModelCoding>
@end

@interface NSResponder (GMArchiverMethods) <ModelCoding>
@end

@interface NSTextField (GMArchiverMethods) <ModelCoding>
@end

@interface NSScrollView (GMArchiverMethods) <ModelCoding>
@end

@interface NSSecureTextFieldCell (GMArchiverMethods) <ModelCoding>
@end

@interface NSView (GMArchiverMethods) <ModelCoding>
@end

@interface NSWindow (GMArchiverMethods) <ModelCoding>
@end

@interface NSPanel (GMArchiverMethods) <ModelCoding>
@end

@interface NSSavePanel (GMArchiverMethods) <ModelCoding>
@end

@interface NSOpenPanel (GMArchiverMethods) <ModelCoding>
@end

@interface NSBrowser (GMArchiverMethods) <ModelCoding>
@end

@interface NSColorWell (GMArchiverMethods) <ModelCoding>
@end

@interface NSImageView (GMArchiverMethods) <ModelCoding>
@end

@interface NSSliderCell (GMArchiverMethods) <ModelCoding>
@end

@interface NSTextFieldCell (GMArchiverMethods) <ModelCoding>
@end

@interface NSFormCell (GMArchiverMethods) <ModelCoding>
@end

@interface NSText (GMArchiverMethods) <ModelCoding>
@end

@interface NSTextView (GMArchiverMethods) <ModelCoding>
@end

#endif /* _GNUstep_H_GMAppKit */
