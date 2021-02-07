/** <title>NSPrintOperation</title>

   <abstract>Controls generation of EPS, PDF or PS print jobs.</abstract>

   Copyright (C) 1996,2001,2004 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: November 2000
   Updated to new specification
   Author: Adam Fedor <fedor@gnu.org>
   Date: Oct 2001
   Modified for Printing Backend Support
   Author: Chad Hardin <cehardin@mac.com>
   Date: June 2004

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

#ifndef _GNUstep_H_NSPrintOperation
#define _GNUstep_H_NSPrintOperation
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

@class NSString;
@class NSData;
@class NSMutableData;

@class NSView;
@class NSWindow;
@class NSPrintInfo;
@class NSPrintPanel;
@class NSGraphicsContext;

typedef enum _NSPrintingPageOrder {
  NSDescendingPageOrder,
  NSSpecialPageOrder,
  NSAscendingPageOrder,
  NSUnknownPageOrder
} NSPrintingPageOrder;

@interface NSPrintOperation : NSObject
{
  // Attributes
  NSPrintInfo *_print_info;
  NSView *_view;
  NSRect _rect;
  NSMutableData *_data;
  NSString *_path;
  NSGraphicsContext *_context;
  NSPrintPanel *_print_panel;
  NSView *_accessory_view;
  NSString *_job_style_hint;
  NSPrintingPageOrder _page_order;
  struct __Flags {
      unsigned int show_print_panel:1;
      unsigned int show_progress_panel:1;
      unsigned int can_spawn_separate_thread:1;
      unsigned int RESERVED:29;
  } _flags;
  int  _currentPage;
}

//
// Creating and Initializing an NSPrintOperation Object
//
+ (NSPrintOperation *)EPSOperationWithView:(NSView *)aView
                                insideRect:(NSRect)rect
                                    toData:(NSMutableData *)data;
+ (NSPrintOperation *)EPSOperationWithView:(NSView *)aView	
                                insideRect:(NSRect)rect
                                    toData:(NSMutableData *)data
                                 printInfo:(NSPrintInfo *)aPrintInfo;
+ (NSPrintOperation *)EPSOperationWithView:(NSView *)aView	
                                insideRect:(NSRect)rect
                                    toPath:(NSString *)path
                                 printInfo:(NSPrintInfo *)aPrintInfo;

+ (NSPrintOperation *)printOperationWithView:(NSView *)aView;
+ (NSPrintOperation *)printOperationWithView:(NSView *)aView
                                   printInfo:(NSPrintInfo *)aPrintInfo;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
+ (NSPrintOperation *)PDFOperationWithView:(NSView *)aView 
                                insideRect:(NSRect)rect 
                                    toData:(NSMutableData *)data;
+ (NSPrintOperation *)PDFOperationWithView:(NSView *)aView 
                                insideRect:(NSRect)rect 
                                    toData:(NSMutableData *)data 
                                 printInfo:(NSPrintInfo*)aPrintInfo;
+ (NSPrintOperation *)PDFOperationWithView:(NSView *)aView 
                                insideRect:(NSRect)rect 
                                    toPath:(NSString *)path 
                                 printInfo:(NSPrintInfo*)aPrintInfo;
#endif

- (id)initEPSOperationWithView:(NSView *)aView
                    insideRect:(NSRect)rect
                        toData:(NSMutableData *)data
                     printInfo:(NSPrintInfo *)aPrintInfo;
- (id)initWithView:(NSView *)aView
         printInfo:(NSPrintInfo *)aPrintInfo;

//
// Setting the Print Operation
//
+ (NSPrintOperation *)currentOperation;
+ (void)setCurrentOperation:(NSPrintOperation *)operation;

//
// Determining the Type of Operation
//
- (BOOL)isEPSOperation;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (BOOL)isCopyingOperation;
#endif

//
// Controlling the User Interface
//
- (NSPrintPanel *)printPanel;
- (BOOL)showPanels;
- (void)setPrintPanel:(NSPrintPanel *)panel;
- (void)setShowPanels:(BOOL)flag;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (BOOL)showsPrintPanel;
- (void)setShowsPrintPanel:(BOOL)flag;
- (BOOL)showsProgressPanel;
- (void)setShowsProgressPanel:(BOOL)flag;
#endif

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (NSView *)accessoryView;
- (void)setAccessoryView:(NSView *)aView;
#endif

//
// Managing the DPS Context
//
- (NSGraphicsContext *)createContext;
- (NSGraphicsContext *)context;
- (void)destroyContext;

//
// Page Information
//
- (int)currentPage;
- (NSPrintingPageOrder)pageOrder;
- (void)setPageOrder:(NSPrintingPageOrder)order;

//
// Running a Print Operation
//
- (void)cleanUpOperation;
- (BOOL)deliverResult;
- (BOOL)runOperation;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void)runOperationModalForWindow:(NSWindow *)docWindow 
                          delegate:(id)delegate 
                    didRunSelector:(SEL)didRunSelector 
                       contextInfo:(void *)contextInfo;
- (BOOL)canSpawnSeparateThread;
- (void)setCanSpawnSeparateThread:(BOOL)flag;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
- (NSString *) jobStyleHint;
- (void)setJobStyleHint:(NSString *)hint;
#endif

//
// Getting the NSPrintInfo Object
//
- (NSPrintInfo *)printInfo;
- (void)setPrintInfo:(NSPrintInfo *)aPrintInfo;

//
// Getting the NSView Object
//
- (NSView *)view;

@end


//
// Private method used by the NSPrintOperation subclasses 
// such as GSEPSPrintOperation, GSPDFPrintOperation.  This
// also includes GSPrintOperation, which is used in the printing
// backend system.  Printing bundles subclass GSPrintOperation
// and use that to interface with the native printing system.
//
@interface NSPrintOperation (Private)

- (id) initWithView:(NSView *)aView
         insideRect:(NSRect)rect
             toData:(NSMutableData *)data
          printInfo:(NSPrintInfo *)aPrintInfo;

@end


#endif // _GNUstep_H_NSPrintOperation
