/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/AppKitExport.h>

@class NSDictionary, NSMutableDictionary, NSPrinter;

typedef int NSPrintingPaginationMode;

typedef enum {
    NSPortraitOrientation,
    NSLandscapeOrientation,
} NSPrintingOrientation;

APPKIT_EXPORT NSString *const NSPrintPrinter;
APPKIT_EXPORT NSString *const NSPrintPrinterName;
APPKIT_EXPORT NSString *const NSPrintJobDisposition;
APPKIT_EXPORT NSString *const NSPrintDetailedErrorReporting;

APPKIT_EXPORT NSString *const NSPrintSpoolJob;
APPKIT_EXPORT NSString *const NSPrintPreviewJob;
APPKIT_EXPORT NSString *const NSPrintSaveJob;
APPKIT_EXPORT NSString *const NSPrintCancelJob;

APPKIT_EXPORT NSString *const NSPrintSavePath;

APPKIT_EXPORT NSString *const NSPrintCopies;
APPKIT_EXPORT NSString *const NSPrintAllPages;
APPKIT_EXPORT NSString *const NSPrintFirstPage;
APPKIT_EXPORT NSString *const NSPrintLastPage;

APPKIT_EXPORT NSString *const NSPrintPaperName;
APPKIT_EXPORT NSString *const NSPrintPaperSize;
APPKIT_EXPORT NSString *const NSPrintOrientation;

APPKIT_EXPORT NSString *const NSPrintHorizontalPagination;
APPKIT_EXPORT NSString *const NSPrintVerticalPagination;

APPKIT_EXPORT NSString *const NSPrintTopMargin;
APPKIT_EXPORT NSString *const NSPrintBottomMargin;
APPKIT_EXPORT NSString *const NSPrintLeftMargin;
APPKIT_EXPORT NSString *const NSPrintRightMargin;
APPKIT_EXPORT NSString *const NSPrintHorizontallyCentered;
APPKIT_EXPORT NSString *const NSPrintVerticallyCentered;

APPKIT_EXPORT NSString *const NSPrintHeaderAndFooter;

@interface NSPrintInfo : NSObject <NSCopying> {
    NSMutableDictionary *_attributes;
}

+ (NSPrintInfo *)sharedPrintInfo;

- initWithDictionary:(NSDictionary *)dictionary;

- (NSMutableDictionary *)dictionary;

- (NSPrinter *)printer;
- (NSString *)jobDisposition;

- (NSString *)paperName;
- (NSSize)paperSize;
- (NSPrintingOrientation)orientation;

- (NSPrintingPaginationMode)horizontalPagination;
- (NSPrintingPaginationMode)verticalPagination;

- (float)topMargin;
- (float)bottomMargin;
- (float)leftMargin;
- (float)rightMargin;

- (BOOL)isHorizontallyCentered;
- (BOOL)isVerticallyCentered;

- (NSString *)localizedPaperName;
- (NSRect)imageablePageBounds;

- (void)setPrinter:(NSPrinter *)printer;
- (void)setJobDisposition:(NSString *)value;

- (void)setPaperName:(NSString *)value;
- (void)setPaperSize:(NSSize)value;
- (void)setOrientation:(NSPrintingOrientation)value;

- (void)setHorizontalPagination:(NSPrintingPaginationMode)value;
- (void)setVerticalPagination:(NSPrintingPaginationMode)value;

- (void)setTopMargin:(float)value;
- (void)setBottomMargin:(float)value;
- (void)setLeftMargin:(float)value;
- (void)setRightMargin:(float)value;

- (void)setHorizontallyCentered:(BOOL)value;
- (void)setVerticallyCentered:(BOOL)value;

- (void)setUpPrintOperationDefaultValues;

@end
