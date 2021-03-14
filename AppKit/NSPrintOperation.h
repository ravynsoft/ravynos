/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>

@class NSPrintInfo, NSPrintPanel, NSView, NSGraphicsContext;

@interface NSPrintOperation : NSObject {
    NSView *_view;
    NSPrintInfo *_printInfo;
    NSView *_accessoryView;
    NSPrintPanel *_printPanel;
    BOOL _showsPrintPanel;
    int _currentPage;
    NSGraphicsContext *_context;
    NSRect _insideRect;
    NSMutableData *_mutableData;
    int _type;
    BOOL _showsPrintProgressPanel;
}

+ (NSPrintOperation *)currentOperation;

+ (NSPrintOperation *)printOperationWithView:(NSView *)view;
+ (NSPrintOperation *)printOperationWithView:(NSView *)view printInfo:(NSPrintInfo *)printInfo;
+ (NSPrintOperation *)PDFOperationWithView:(NSView *)view insideRect:(NSRect)rect toData:(NSMutableData *)data;
+ (NSPrintOperation *)PDFOperationWithView:(NSView *)view insideRect:(NSRect)rect toData:(NSMutableData *)data printInfo:(NSPrintInfo *)printInfo;
+ (NSPrintOperation *)EPSOperationWithView:(NSView *)view insideRect:(NSRect)rect toData:(NSMutableData *)data;
+ (NSPrintOperation *)EPSOperationWithView:(NSView *)view insideRect:(NSRect)rect toData:(NSMutableData *)data printInfo:(NSPrintInfo *)printInfo;

- (BOOL)isCopyingOperation;

- (void)setAccessoryView:(NSView *)view;

- (NSView *)view;
- (NSPrintInfo *)printInfo;
- (NSPrintPanel *)printPanel;
- (BOOL)showsPrintPanel;
- (void)setShowsPrintPanel:(BOOL)flag;

- (BOOL)showsProgressPanel;
- (void)setShowsProgressPanel:(BOOL)flag;

- (int)currentPage;

- (NSGraphicsContext *)createContext;
- (void)destroyContext;

- (BOOL)runOperation;

@end
