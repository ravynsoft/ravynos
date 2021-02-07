/** <title>NSPageLayout</title>

   <abstract>Standard panel for querying user about page layout.</abstract>

   Copyright (C) 2001,2004 Free Software Foundation, Inc.

   Written By: Adam Fedor <fedor@gnu.org>
   Date: Oct 2001
   Author: Chad Hardin <cehardin@mac.com>
   Date: July 2004
   
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

#ifndef _GNUstep_H_NSPageLayout
#define _GNUstep_H_NSPageLayout
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSApplication.h>
#import <AppKit/NSPanel.h>

@class NSPrintInfo;
@class NSView;
@class NSWindow;

//
// Note: not all of these tags are currently used in the
// NSPageLayout panel, see below for details.
//
enum {
  NSPLImageButton       = 1,  //Used.
  NSPLTitleField        = 2,  //Used.
  NSPLPaperNameButton   = 3,  //Used, only for the standard paper size 
                              //NSPopUpButton though.  Not the 
					//NSPopUpButton used to select custom 
					//paper sizes.
  NSPLUnitsButton       = 4,  //NOT used.
  NSPLWidthField        = 5,  //NOT used.
  NSPLHeightField       = 6,  //NOT used.
  NSPLOrientationMatrix = 7,  //Used.
  NSPLCancelButton      = 8,  //Used.
  NSPLOKButton          = 9,  //Used.
  NSPLPageLayout        = 10,  //NOT used.
  NSPLScaleField        = 11  //Used.
};


@interface NSApplication (NSPageLayout)
- (void) runPageLayout: (id)sender;
@end

@interface NSPageLayout : NSPanel
{
  id _controller; 
}

//
// Creating an NSPageLayout Instance 
//
+ (NSPageLayout *)pageLayout;

//
// Running the Panel 
//
- (NSInteger)runModal;
- (NSInteger)runModalWithPrintInfo:(NSPrintInfo *)printInfo;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void)beginSheetWithPrintInfo:(NSPrintInfo *)printInfo
                 modalForWindow:(NSWindow *)docWindow
                       delegate:(id)delegate
                 didEndSelector:(SEL)didEndSelector
                    contextInfo:(void *)contextInfo;
#endif

//
// Customizing the Panel 
//
- (NSView *)accessoryView;
- (void)setAccessoryView:(NSView *)aView;

//
// Updating the Panel's Display 
//
- (void)convertOldFactor:(float *)old
               newFactor:(float *)newFactor;
- (void)pickedButton:(id)sender;
- (void)pickedOrientation:(id)sender;
- (void)pickedPaperSize:(id)sender;
- (void)pickedUnits:(id)sender;

//
// Communicating with the NSPrintInfo Object 
//
- (NSPrintInfo *)printInfo;
- (void)readPrintInfo;
- (void)writePrintInfo;

@end

#endif // _GNUstep_H_NSPageLayout
