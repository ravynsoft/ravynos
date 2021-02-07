/** <title>NSPrintInfo</title>

   <abstract>Stores information used in printing.</abstract>

   Copyright (C) 1996,1997,2004 Free Software Foundation, Inc.

   Author: Simon Frankau <sgf@frankau.demon.co.uk>
   Date: July 1997
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

#ifndef _GNUstep_H_NSPrintInfo
#define _GNUstep_H_NSPrintInfo

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/AppKitDefines.h>

@class NSString;
@class NSDictionary;
@class NSMutableDictionary;

@class NSPrinter;

typedef enum _NSPrintingOrientation {
  NSPortraitOrientation,
  NSLandscapeOrientation
} NSPrintingOrientation;

typedef enum _NSPaperOrientation {
  NSPaperOrientationPortrait,
  NSPaperOrientationLandscape
} NSPaperOrientation;

typedef enum _NSPrintingPaginationMode {
  NSAutoPagination,
  NSFitPagination,
  NSClipPagination
} NSPrintingPaginationMode;

@interface NSPrintInfo : NSObject <NSCoding, NSCopying>
{
  NSMutableDictionary *_info;
}


//
// Creating and Initializing an NSPrintInfo Instance 
//
- (id)initWithDictionary:(NSDictionary *)aDict;

//
// Managing the Shared NSPrintInfo Object 
//
+ (void)setSharedPrintInfo:(NSPrintInfo *)printInfo;
+ (NSPrintInfo *)sharedPrintInfo;

//
// Managing the Printing Rectangle 
//
+ (NSSize)sizeForPaperName:(NSString *)name;
- (CGFloat)bottomMargin;
- (CGFloat)leftMargin;
- (NSPrintingOrientation)orientation;
- (NSString *)paperName;
- (NSSize)paperSize;
- (CGFloat)rightMargin;
- (void)setBottomMargin:(CGFloat)value;
- (void)setLeftMargin:(CGFloat)value;
- (void)setOrientation:(NSPrintingOrientation)mode;
- (void)setPaperName:(NSString *)name;
- (void)setPaperSize:(NSSize)size;
- (void)setRightMargin:(CGFloat)value;
- (void)setTopMargin:(CGFloat)value;
- (CGFloat)topMargin;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
- (NSRect)imageablePageBounds;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (NSString *)localizedPaperName;
#endif

//
// Pagination 
//
- (NSPrintingPaginationMode)horizontalPagination;
- (void)setHorizontalPagination:(NSPrintingPaginationMode)mode;
- (void)setVerticalPagination:(NSPrintingPaginationMode)mode;
- (NSPrintingPaginationMode)verticalPagination;

//
// Positioning the Image on the Page 
//
- (BOOL)isHorizontallyCentered;
- (BOOL)isVerticallyCentered;
- (void)setHorizontallyCentered:(BOOL)flag;
- (void)setVerticallyCentered:(BOOL)flag;

//
// Specifying the Printer 
//
+ (NSPrinter *)defaultPrinter;
+ (void)setDefaultPrinter:(NSPrinter *)printer;
- (NSPrinter *)printer;
- (void)setPrinter:(NSPrinter *)aPrinter;

//
// Controlling Printing
//
- (NSString *)jobDisposition;
- (void)setJobDisposition:(NSString *)disposition;
- (void)setUpPrintOperationDefaultValues;

//
// Accessing the NSPrintInfo Object's Dictionary 
//
- (NSMutableDictionary *)dictionary;

@end


//
// Printing Information Dictionary Keys 
//
APPKIT_EXPORT NSString *NSPrintAllPages;
APPKIT_EXPORT NSString *NSPrintBottomMargin;
APPKIT_EXPORT NSString *NSPrintCopies;
APPKIT_EXPORT NSString *NSPrintFaxCoverSheetName;
APPKIT_EXPORT NSString *NSPrintFaxHighResolution;
APPKIT_EXPORT NSString *NSPrintFaxModem;
APPKIT_EXPORT NSString *NSPrintFaxReceiverNames;
APPKIT_EXPORT NSString *NSPrintFaxReceiverNumbers;
APPKIT_EXPORT NSString *NSPrintFaxReturnReceipt;
APPKIT_EXPORT NSString *NSPrintFaxSendTime;
APPKIT_EXPORT NSString *NSPrintFaxTrimPageEnds;
APPKIT_EXPORT NSString *NSPrintFaxUseCoverSheet;
APPKIT_EXPORT NSString *NSPrintFirstPage;
APPKIT_EXPORT NSString *NSPrintHorizontalPagination;
APPKIT_EXPORT NSString *NSPrintHorizontallyCentered;
APPKIT_EXPORT NSString *NSPrintJobDisposition;
APPKIT_EXPORT NSString *NSPrintJobFeatures;
APPKIT_EXPORT NSString *NSPrintLastPage;
APPKIT_EXPORT NSString *NSPrintLeftMargin;
APPKIT_EXPORT NSString *NSPrintManualFeed;
APPKIT_EXPORT NSString *NSPrintMustCollate;
APPKIT_EXPORT NSString *NSPrintOrientation;
APPKIT_EXPORT NSString *NSPrintPackageException;
APPKIT_EXPORT NSString *NSPrintPagesPerSheet;
APPKIT_EXPORT NSString *NSPrintPaperFeed;
APPKIT_EXPORT NSString *NSPrintPaperName;
APPKIT_EXPORT NSString *NSPrintPaperSize;
APPKIT_EXPORT NSString *NSPrintPrinter;
APPKIT_EXPORT NSString *NSPrintReversePageOrder;
APPKIT_EXPORT NSString *NSPrintRightMargin;
APPKIT_EXPORT NSString *NSPrintSavePath;
APPKIT_EXPORT NSString *NSPrintScalingFactor;
APPKIT_EXPORT NSString *NSPrintTopMargin;
APPKIT_EXPORT NSString *NSPrintVerticalPagination;
APPKIT_EXPORT NSString *NSPrintVerticallyCentered;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
APPKIT_EXPORT NSString *NSPrintPagesAcross;
APPKIT_EXPORT NSString *NSPrintPagesDown;
APPKIT_EXPORT NSString *NSPrintTime;
APPKIT_EXPORT NSString *NSPrintDetailedErrorReporting;
APPKIT_EXPORT NSString *NSPrintFaxNumber;
APPKIT_EXPORT NSString *NSPrintPrinterName;
APPKIT_EXPORT NSString *NSPrintHeaderAndFooter;
#endif

//
// Additional (GNUstep) keys
//
/** Set to <code>Rows</code> to print row by row, set to <code>Columns</code>
    to print column by column */
APPKIT_EXPORT NSString *NSPrintPageDirection;

//
// Print Job Disposition Values 
//
APPKIT_EXPORT NSString *NSPrintCancelJob;
APPKIT_EXPORT NSString *NSPrintFaxJob;
APPKIT_EXPORT NSString *NSPrintPreviewJob;
APPKIT_EXPORT NSString *NSPrintSaveJob;
APPKIT_EXPORT NSString *NSPrintSpoolJob;

#endif // _GNUstep_H_NSPrintInfo
