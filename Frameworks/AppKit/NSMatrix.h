/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSControl.h>

typedef enum {
    NSRadioModeMatrix,
    NSHighlightModeMatrix,
    NSListModeMatrix,
    NSTrackModeMatrix
} NSMatrixMode;

@interface NSMatrix : NSControl {
    id _delegate;
    id _target;
    SEL _action;
    SEL _doubleAction;
    NSFont *_font;
    NSColor *_backgroundColor;
    NSColor *_cellBackgroundColor;

    NSMutableArray *_cells;
    int _numberOfRows;
    int _numberOfColumns;
    int _selectedIndex;
    int _keyCellIndex;
    NSSize _cellSize;
    NSSize _intercellSpacing;
    id _prototype;
    Class _cellClass;
    int _mode;
    BOOL _selectionByRect;
    BOOL _allowsEmptySelection;
    BOOL _tabKeyTraversesCells;
    BOOL _isAutoscroll;
    BOOL _isEnabled;
    BOOL _autosizesCells;
    BOOL _drawsBackground;
    BOOL _drawsCellBackground;
    BOOL _refusesFirstResponder;
}

- initWithFrame:(NSRect)frame mode:(int)mode prototype:(NSCell *)prototype numberOfRows:(int)rows numberOfColumns:(int)columns;
- initWithFrame:(NSRect)frame mode:(int)mode cellClass:(Class)cls numberOfRows:(int)rows numberOfColumns:(int)columns;

- delegate;
- (SEL)doubleAction;

- (Class)cellClass;
- prototype;

- (NSArray *)cells;
- cellWithTag:(int)tag;
- cellAtRow:(int)row column:(int)column;
- (NSRect)cellFrameAtRow:(int)row column:(int)column;
- (BOOL)getRow:(int *)row column:(int *)column ofCell:(NSCell *)cell;
- (BOOL)getRow:(int *)row column:(int *)column forPoint:(NSPoint)point;

- (int)numberOfRows;
- (int)numberOfColumns;
- (void)getNumberOfRows:(int *)rows columns:(int *)columns;

- (NSString *)toolTipForCell:(NSCell *)cell;

- keyCell;

- (NSMatrixMode)mode;
- (BOOL)allowsEmptySelection;
- (BOOL)tabKeyTraversesCells;

- (BOOL)autosizesCells;
- (NSSize)cellSize;
- (NSSize)intercellSpacing;

- (BOOL)drawsBackground;
- (NSColor *)backgroundColor;

- (BOOL)drawsCellBackground;
- (NSColor *)cellBackgroundColor;

- (BOOL)isAutoscroll;

- (int)selectedRow;
- (int)selectedColumn;
- (NSArray *)selectedCells;

- (void)setDelegate:delegate;
- (void)setDoubleAction:(SEL)action;
- (void)setCellClass:(Class)aClass;
- (void)setPrototype:(NSCell *)cell;

- (void)renewRows:(int)rows columns:(int)columns;
- (NSCell *)makeCellAtRow:(int)row column:(int)col;
- (void)putCell:(NSCell *)cell atRow:(int)row column:(int)column;

- (void)addRow;
- (void)insertRow:(int)row;
- (void)removeRow:(int)row;
- (void)insertRow:(int)row withCells:(NSArray *)cells;

- (void)addColumn;
- (void)removeColumn:(int)col;

- (void)setToolTip:(NSString *)tip forCell:(NSCell *)cell;

- (void)setKeyCell:cell;

- (void)setMode:(NSMatrixMode)mode;
- (void)setAllowsEmptySelection:(BOOL)flag;
- (void)setTabKeyTraversesCells:(BOOL)flag;

- (void)setAutosizesCells:(BOOL)flag;
- (void)setCellSize:(NSSize)size;
- (void)setIntercellSpacing:(NSSize)size;

- (void)setDrawsBackground:(BOOL)flag;
- (void)setBackgroundColor:(NSColor *)color;
- (void)setDrawsCellBackground:(BOOL)flag;
- (void)setCellBackgroundColor:(NSColor *)color;

- (void)setAutoscroll:(BOOL)flag;

- (void)selectCellAtRow:(int)row column:(int)column;
- (void)selectCell:(NSCell *)cell;
- (BOOL)selectCellWithTag:(int)tag;
- (void)selectAll:sender;
- (void)setSelectionFrom:(int)from to:(int)to anchor:(int)anchor highlight:(BOOL)highlight;
- (void)deselectAllCells;
- (void)deselectSelectedCell;

- (void)sizeToCells;

- (void)setState:(int)state atRow:(int)row column:(int)column;
- (void)highlightCell:(BOOL)highlight atRow:(int)row column:(int)column;

- (void)drawCellAtRow:(int)row column:(int)column;

- (void)scrollCellToVisibleAtRow:(int)row column:(int)column;

- (BOOL)sendAction;

@end
