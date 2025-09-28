/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSMatrix.h>

@class NSFormCell;

@interface NSForm : NSMatrix

- cellAtIndex:(int)index;
- (int)indexOfCellWithTag:(int)tag;
- (int)indexOfSelectedItem;

- (void)setBordered:(BOOL)value;
- (void)setBezeled:(BOOL)value;

- (void)setEntryWidth:(float)value;
- (void)setInterlineSpacing:(float)value;

- (void)setTitleAlignment:(NSTextAlignment)value;
- (void)setTitleFont:(NSFont *)value;
- (void)setTitleBaseWritingDirection:(NSWritingDirection)value;

- (void)setTextAlignment:(NSTextAlignment)value;
- (void)setTextFont:(NSFont *)value;
- (void)setTextBaseWritingDirection:(NSWritingDirection)value;

- (NSFormCell *)addEntry:(NSString *)title;
- (NSFormCell *)insertEntry:(NSString *)title atIndex:(int)index;
- (void)removeEntryAtIndex:(int)index;

- (void)selectTextAtIndex:(int)index;

- (void)drawCellAtIndex:(int)index;

@end
