/* Definition of class NSPDFPanel
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory Casamento <greg.casamento@gmail.com>
   Date: Sat Nov 16 21:21:00 EST 2019

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#ifndef _NSPDFPanel_h_GNUSTEP_GUI_INCLUDE
#define _NSPDFPanel_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_9, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSString, NSPDFInfo, NSWindow, NSViewController;

enum {
  NSPDFPanelShowsPaperSize = 1 << 2,
  NSPDFPanelShowsOrientation = 1 << 3,
  NSPDFPanelRequestsParentDirectory = 1 << 24,
};
typedef NSUInteger NSPDFPanelOptions;

DEFINE_BLOCK_TYPE(GSPDFPanelCompletionHandler, void, NSInteger);
  
@interface NSPDFPanel : NSObject
{
  NSViewController *_accessoryController;
  NSPDFPanelOptions _options;
  NSString *_defaultFileName;
}

+ (NSPDFPanel *) panel;

- (NSViewController *) accessoryController;
- (void) setAccessoryController: (NSViewController *)accessoryController;

- (NSPDFPanelOptions) options;
- (void) setOptions: (NSPDFPanelOptions)opts;

- (NSString *) defaultFileName;
- (void) setDefaultFileName: (NSString *)fileName;

- (void) beginSheetWithPDFInfo: (NSPDFInfo *)pdfInfo
                modalForWindow: (NSWindow *)window
             completionHandler: (GSPDFPanelCompletionHandler)handler;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSPDFPanel_h_GNUSTEP_GUI_INCLUDE */

