/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSControl.h>

@class NSMatrix, NSScroller;

@interface NSBrowser : NSControl {
    id _target;
    SEL _action;
    SEL _doubleAction;

    id _delegate;

    NSMutableArray *_explicitTitles;
    NSMutableArray *_titles;
    NSMutableArray *_matrices;
    NSMutableArray *_scrollViews;
    NSScroller *_horizontalScroller;
    NSColor *_backgroundColor;

    Class _matrixClass;
    Class _cellClass;
    id _cellPrototype;
    int _numberOfVisibleColumns;
    int _minColumnWidth;
    int _firstVisibleColumn;
    int _maxVisibleColumns;
    int _selectedColumn;
    BOOL _reusesColumns;
    BOOL _hasHorizontalScroller;
    BOOL _separatesColumns;
    BOOL _isTitled;
    BOOL _allowsMultipleSelection;
    BOOL _allowsBranchSelection;
    BOOL _allowsEmptySelection;
    BOOL _takesTitleFromPreviousColumn;
    BOOL _acceptsArrowKeys;
    BOOL _sendsActionOnArrowKeys;
}

- delegate;
- (NSColor *)backgroundColor;
- (SEL)doubleAction;
- (Class)matrixClass;
- (NSInteger)maxVisibleColumns;
- (BOOL)hasHorizontalScroller;
- (BOOL)separatesColumns;
- (BOOL)isTitled;
- (BOOL)takesTitleFromPreviousColumn;
- (BOOL)allowsMultipleSelection;
- (BOOL)allowsBranchSelection;
- (BOOL)allowsEmptySelection;
- (BOOL)acceptsArrowKeys;
- (BOOL)sendsActionOnArrowKeys;
- (BOOL)reusesColumns;

- (NSInteger)lastColumn;
- (NSInteger)lastVisibleColumn;
- (NSInteger)firstVisibleColumn;

- (NSMatrix *)matrixInColumn:(NSInteger)column;
- (NSInteger)columnOfMatrix:(NSMatrix *)matrix;
- (NSString *)titleOfColumn:(NSInteger)column;

- (NSArray *)selectedCells;
- selectedCell;
- selectedCellInColumn:(NSInteger)column;
- (NSInteger)selectedColumn;
- (NSInteger)selectedRowInColumn:(NSInteger)column;

- (float)titleHeight;
- (NSRect)titleFrameOfColumn:(NSInteger)column;
- (NSRect)frameOfColumn:(NSInteger)column;
- (NSRect)frameOfInsideOfColumn:(NSInteger)column;

- (void)setDelegate:delegate;
- (void)setBackgroundColor:(NSColor *)color;
- (void)setDoubleAction:(SEL)action;
- (void)setMatrixClass:(Class)aClass;
- (void)setCellClass:(Class)aClass;
- (void)setMaxVisibleColumns:(NSInteger)count;
- (void)setHasHorizontalScroller:(BOOL)flag;
- (void)setSeparatesColumns:(BOOL)flag;
- (void)setTitled:(BOOL)flag;
- (void)setTakesTitleFromPreviousColumn:(BOOL)flag;
- (void)setAllowsMultipleSelection:(BOOL)flag;
- (void)setAllowsBranchSelection:(BOOL)flag;
- (void)setAllowsEmptySelection:(BOOL)flag;
- (void)setAcceptsArrowKeys:(BOOL)flag;
- (void)setSendsActionOnArrowKeys:(BOOL)flag;
- (void)setReusesColumns:(BOOL)flag;

- (void)setTitle:(NSString *)title ofColumn:(NSInteger)column;

- (void)selectRow:(NSInteger)row inColumn:(NSInteger)column;
- (void)setPath:(NSString *)path;

- (BOOL)sendAction;
- (void)doClick:sender;
- (void)doDoubleClick:sender;

- (void)loadColumnZero;
- (void)reloadColumn:(NSInteger)column;
- (void)addColumn;
- (void)setLastColumn:(NSInteger)column;
- (void)validateVisibleColumns;

- (void)scrollColumnsLeftBy:(NSInteger)delta;
- (void)scrollColumnsRightBy:(NSInteger)delta;
- (void)scrollColumnToVisible:(NSInteger)column;
- (void)scrollViaScroller:(NSScroller *)sender;
- (void)updateScroller;

- (void)drawTitleOfColumn:(NSInteger)column inRect:(NSRect)rect;

- (void)tile;

@end

@interface NSObject (NSBrowser_delegate)
- (BOOL)browser:(NSBrowser *)browser isColumnValid:(NSInteger)column;

- (NSInteger)browser:(NSBrowser *)browser numberOfRowsInColumn:(NSInteger)column;
- (void)browser:(NSBrowser *)browser createRowsForColumn:(NSInteger)column inMatrix:(NSMatrix *)matrix;

- (BOOL)browser:(NSBrowser *)browser selectRow:(NSInteger)row inColumn:(NSInteger)column;
- (BOOL)browser:(NSBrowser *)browser selectCellWithString:(NSString *)title inColumn:(NSInteger)column;

- (NSString *)browser:(NSBrowser *)browser titleOfColumn:(NSInteger)column;

- (void)browserWillScroll:(NSBrowser *)browser;
- (void)browserDidScroll:(NSBrowser *)browser;

- (void)browser:(NSBrowser *)browser willDisplayCell:cell atRow:(NSInteger)row column:(NSInteger)column;

@end
