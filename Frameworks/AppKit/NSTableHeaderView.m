/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSTableHeaderView.h>
#import <AppKit/NSTableView.h>
#import <AppKit/NSTableColumn.h>
#import <AppKit/NSCell.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSCursor.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSScrollView.h>
#import <AppKit/NSClipView.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSRaise.h>

@interface NSTableView(NSTableView_private)
-(void)noteColumnDidResizeWithOldWidth:(float)oldWidth;
@end

@implementation NSTableHeaderView

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];
   
   if([coder allowsKeyedCoding]){
    // NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    // nothing
   }
   return self;
}

-(NSTableView *)tableView {
    return _tableView;
}

-(int)resizedColumn {
    return _resizedColumn;
}

-(int)draggedColumn {
    NSUnimplementedMethod();
    return -1;
}

-(float)draggedDistance {
    NSUnimplementedMethod();
    return -1;
}

-(void)setTableView:(NSTableView *)tableView {
    _tableView = tableView;
}

-(NSRect)headerRectOfColumn:(int)column {
    NSArray *tableColumns = [_tableView tableColumns];
    NSRect headerRect = _bounds;
    NSSize spacing = [_tableView intercellSpacing];

    if (column < 0 || column >= [[_tableView tableColumns] count]) {
        [NSException raise:NSInternalInconsistencyException
                    format:@"headerRectOfColumn: invalid index %d (valid {%d, %d})",
            column, 0, [tableColumns count]];
    }

    headerRect.size.width = [[tableColumns objectAtIndex:column--] width] + spacing.width;
    while (column >= 0)
        headerRect.origin.x += [[tableColumns objectAtIndex:column--] width] + spacing.width;
    
    return headerRect;
}

-(int)columnAtPoint:(NSPoint)point {
    int i, count = [[_tableView tableColumns] count];

    for (i = 0; i < count; ++i) {
        if (NSMouseInRect(point, [self headerRectOfColumn:i],[self isFlipped]))
            return i;
    }

    return NSNotFound;
}

-(void)drawRect:(NSRect)rect {
    NSArray *tableColumns = [_tableView tableColumns];
    int i, count = [tableColumns count];
    NSRect columnRect = _bounds;
    NSSize spacing = [_tableView intercellSpacing];

    // fixes a bizarre drawing bug i couldn't otherwise track down;
    // uncomment to notice a 1-pixel wide white line just inside
    // the NSDrawButton bezel of the rightmost NSTableHeaderCell
   // NSDrawButton(_bounds, _bounds);

    for (i = 0; i < count; ++i) {
        NSTableColumn *column = [tableColumns objectAtIndex:i];
        
        columnRect.size.width = [column width] + spacing.width;
        [[column headerCell] setHighlighted:[_tableView isColumnSelected:[[_tableView tableColumns] indexOfObject:column]]];
        [[column headerCell] setControlView:self];
        [[column headerCell] drawWithFrame:columnRect inView:self];        
        columnRect.origin.x += [column width] + spacing.width;
    }
}

-(NSRect)_resizeRectBeforeColumn:(int)column {
    NSRect rect=[self headerRectOfColumn:column];

    rect.origin.x -= 2;
    rect.size.width = 6;

    return rect;
}

-(void)mouseDown:(NSEvent *)theEvent {
    NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    int clickedColumn = [self columnAtPoint:location];

    if ([[_tableView delegate] respondsToSelector:@selector(tableView:mouseDownInHeaderOfTableColumn:)])
        [[_tableView delegate] tableView:_tableView
          mouseDownInHeaderOfTableColumn:[[_tableView tableColumns] objectAtIndex:clickedColumn]];

    if ([_tableView allowsColumnResizing]) {
        int i, count=[[_tableView tableColumns] count];

        // if there is any editing going on, we have to end it. Apple ends editing, sends the
        // setObjectValue, but does NOT select any following cells. sending nil to textDidEndEditing
        // will cause a 0 or NSIllegalTextMovement, so no editing chain stuff will happen.
        if ([_tableView editedColumn] != -1 || [_tableView editedRow] != -1)
            [[self window] endEditingFor:nil];

        for (i = 1; i < count; ++i) {
            if (NSMouseInRect(location, [self _resizeRectBeforeColumn:i],[self isFlipped])) {
                NSTableColumn *resizingColumn = [[_tableView tableColumns] objectAtIndex:i-1];
                float resizedColumnWidth = [resizingColumn width];

                if (![resizingColumn isResizable])
                    return;

                _resizedColumn = i - 1;
                location=[self convertPoint:[theEvent locationInWindow] fromView:nil];
                do { // greatly simplified code...
                    NSPoint newPoint;
                    NSRect newRect;
                    int q;
                    float newWidth=newPoint.x;
                    
                    theEvent=[[self window] nextEventMatchingMask:NSLeftMouseUpMask|NSLeftMouseDraggedMask];
                    newPoint=[self convertPoint:[theEvent locationInWindow] fromView:nil];

                    newWidth=newPoint.x;
                    for (q = 0; q < _resizedColumn; ++q) {
                        newWidth -= [[[_tableView tableColumns] objectAtIndex:q] width];
                        newWidth -= [_tableView intercellSpacing].width;
                    }

                    [resizingColumn setWidth:newWidth];

                    [_tableView tile];
                    newRect.origin=[_tableView convertPoint:newPoint fromView:self];
                    newRect.size=NSMakeSize(10,10);
                    [_tableView scrollRectToVisible:newRect];
                    
                    location=newPoint;
                } while ([theEvent type] != NSLeftMouseUp);

                [[self window] invalidateCursorRectsForView:self];

                [_tableView noteColumnDidResizeWithOldWidth:resizedColumnWidth];
         
                _resizedColumn = -1;
                return;
            }
        }
    }
	
    if ([_tableView allowsColumnSelection]) {
        if ([theEvent modifierFlags] & NSAlternateKeyMask) {		// extend/change selection
            if ([_tableView isColumnSelected:clickedColumn])		// deselect previously selected?
                [_tableView deselectColumn:clickedColumn];
            else if ([_tableView allowsMultipleSelection] == YES) {
                [_tableView selectColumn:clickedColumn byExtendingSelection:YES];	// add to selection
            }
        }
        else if ([theEvent modifierFlags] & NSShiftKeyMask) {
            int startColumn = [_tableView selectedColumn];
            int endColumn = clickedColumn;

            if (startColumn == -1)
                startColumn = 0;
            if (startColumn > endColumn) {
                endColumn = startColumn;
                startColumn = clickedColumn;
            }

            [_tableView deselectAll:nil];
            while (startColumn <= endColumn)
                [_tableView selectColumn:startColumn++ byExtendingSelection:YES];
        }
        else
            [_tableView selectColumn:clickedColumn byExtendingSelection:NO];
    }
	
	[[[_tableView tableColumns] objectAtIndex:clickedColumn] _sort];
}

-(void)resetCursorRects {
    int i;

    for (i = 1; i < [[_tableView tableColumns] count]; ++i)
        [self addCursorRect:[self _resizeRectBeforeColumn:i] cursor:[NSCursor resizeLeftRightCursor]];
}

-(void)resizeWithOldSuperviewSize:(NSSize)size {
    [self resetCursorRects];
}

@end
