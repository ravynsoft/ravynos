/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSBrowser.h>
#import <AppKit/NSBrowserCell.h>
#import <AppKit/NSScrollView.h>
#import <AppKit/NSClipView.h>
#import <AppKit/NSScroller.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSGraphicsStyle.h>
#import <AppKit/NSStringDrawing.h>
#import <AppKit/NSMatrix.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSStringDrawer.h>
#import <AppKit/NSWindow.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSRaise.h>

@interface NSBrowser(Private)
-(NSRect)frameOfScroller;
@end

@implementation NSBrowser

+(Class)cellClass {
   return [NSBrowserCell class];
}

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];

   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    unsigned           flags=[keyed decodeIntForKey:@"NSBrFlags"];
    NSString          *firstTitle=[keyed decodeObjectForKey:@"NSFirstColumnTitle"];
    
    _explicitTitles=[NSMutableArray new];
    if(firstTitle!=nil)
     [_explicitTitles addObject:firstTitle];
    _titles=[NSMutableArray new];

    _matrices=[NSMutableArray new];
    _scrollViews=[NSMutableArray new];

    _backgroundColor=[[NSColor whiteColor] copy];
    
    _matrixClass=[NSMatrix class];
    _cellClass=[[self class] cellClass];
    _cellPrototype=nil;
    
    _numberOfVisibleColumns=[keyed decodeIntForKey:@"NSNumberOfVisibleColumns"];
    _selectedColumn=-1;
    
    _allowsMultipleSelection=(flags&0x80000000)?YES:NO;
    _allowsEmptySelection=(flags&0x00020000)?NO:YES;
    _allowsBranchSelection=(flags&0x40000000)?YES:NO;
    _separatesColumns=(flags&0x04000000)?YES:NO;
    _isTitled=(flags&0x10000000)?YES:NO;
    _hasHorizontalScroller=(flags&0x00010000)?YES:NO;
    _horizontalScroller=nil; // not stored
    [self setHasHorizontalScroller:_hasHorizontalScroller];
    _acceptsArrowKeys=(flags&0x00100000)?YES:NO;
    _sendsActionOnArrowKeys=(flags&0x00040000)?YES:NO;

    _takesTitleFromPreviousColumn=YES;

    [self tile];
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   }
   return self;
}

-initWithFrame:(NSRect)frame {
   [super initWithFrame:frame];

   _explicitTitles=[NSMutableArray new];
   [_explicitTitles addObject:@"Browser"];
   _titles=[NSMutableArray new];

   _matrices=[NSMutableArray new];
   _scrollViews=[NSMutableArray new];

   _horizontalScroller=nil;
   _backgroundColor=[[NSColor whiteColor] copy];
   _matrixClass=[NSMatrix class];
   _cellClass=[[self class] cellClass];
   _cellPrototype=nil;

   _numberOfVisibleColumns=1;
   _allowsMultipleSelection=NO;
   _allowsEmptySelection=NO;
   _allowsBranchSelection=NO;
   _separatesColumns=NO;
   _isTitled=YES;
   _hasHorizontalScroller=NO;

   _takesTitleFromPreviousColumn=YES;

   return self;
}

-(void)dealloc {
   [_explicitTitles release];
   [_titles release];
   [_matrices release];
   [_scrollViews release];
   [_horizontalScroller release];
   [super dealloc];
}

-(Class)cellClass {
   return _cellClass;
}

-target {
   return _target;
}

-(SEL)action {
   return _action;
}

-(void)setTarget:target {
   _target=target;
}

-(void)setAction:(SEL)action {
   _action=action;
}

-delegate {
   return _delegate;
}

-(NSColor *)backgroundColor {
   return _backgroundColor;
}

-(SEL)doubleAction {
   return _doubleAction;
}

-(Class)matrixClass {
   return _matrixClass;
}

-(NSInteger)maxVisibleColumns {
   return _maxVisibleColumns;
}

-(BOOL)hasHorizontalScroller {
   return _hasHorizontalScroller;
}

-(BOOL)separatesColumns {
   return _separatesColumns;
}

-(BOOL)isTitled {
   return _isTitled;
}

-(BOOL)takesTitleFromPreviousColumn {
   return _takesTitleFromPreviousColumn;
}

-(BOOL)allowsMultipleSelection {
   return _allowsMultipleSelection;
}

-(BOOL)allowsBranchSelection {
   return _allowsBranchSelection;
}

-(BOOL)allowsEmptySelection {
   return _allowsEmptySelection;
}

-(BOOL)acceptsArrowKeys {
   return _acceptsArrowKeys;
}

-(BOOL)sendsActionOnArrowKeys {
   return _sendsActionOnArrowKeys;
}

-(BOOL)reusesColumns {
   return _reusesColumns;
}

-(NSInteger)lastColumn {
   return [_matrices count]-1;
}

-(NSInteger)lastVisibleColumn {
   return [self firstVisibleColumn]+(_numberOfVisibleColumns-1);
}

-(NSInteger)firstVisibleColumn {
   return _firstVisibleColumn;
}

-(NSMatrix *)matrixInColumn:(NSInteger)column {
   if(column<[_matrices count])
    return [_matrices objectAtIndex:column];

   return nil;
}

-(NSInteger)columnOfMatrix:(NSMatrix *)matrix {
   return [_matrices indexOfObjectIdenticalTo:matrix];
}

-(NSString *)titleOfColumn:(NSInteger)column {
   id result;

   if(column>=[_titles count])
    result=@"";
   else {
    result=[_titles objectAtIndex:column];

    if(result==[NSNull null]){
     if(column<[_explicitTitles count])
      result=[_explicitTitles objectAtIndex:column];
     else
      result=@"";
    }
   }

   return result;
}

-(NSArray *)selectedCells {
    return [[self matrixInColumn:_selectedColumn] selectedCells];
}

-selectedCell {
    return [self selectedCellInColumn:_selectedColumn];
}

-selectedCellInColumn:(NSInteger)column {
    return [[self matrixInColumn:column] selectedCell];
}

// This needs to be computed, manipulating individual cell state bypasses _selectedColumn
// We should get rid of _selectedColumn eventually
-(NSInteger)selectedColumn {
   NSInteger i,count=[_matrices count];

   for(i=0;i<count;i++){
    NSMatrix *matrix=[_matrices objectAtIndex:i];

    if([matrix selectedCell]==nil)
     break;
   }

   return i-1;
}

-(NSInteger)selectedRowInColumn:(NSInteger)column {
   return [[self matrixInColumn:column] selectedRow];
}

-(float)seperatorWidth {
   return 2;
}

-(float)titleHeight {
   return 20;
}

-(NSRect)titleFrameOfColumn:(NSInteger)column {
   NSRect result;
   float  columnGap=_separatesColumns?[self seperatorWidth]:0;

   column-=[self firstVisibleColumn];

   result.size.width=([self bounds].size.width-(_numberOfVisibleColumns-1)* columnGap)/_numberOfVisibleColumns;
   result.size.height=[self titleHeight];
   result.origin.x=column*(result.size.width+ columnGap);
   result.origin.y=[self bounds].size.height-result.size.height;

   return result;
}

-(NSRect)frameOfScrollerBorder {
   NSRect result=[self bounds];

   result=NSInsetRect(result,2,2);
   result.size.height=[NSScroller scrollerWidth];
   result=NSInsetRect(result,-2,-2);
   return result;
}

-(NSRect)frameOfScroller {
   NSRect result=[self frameOfScrollerBorder];

   return NSInsetRect(result,2,2);
}

-(NSRect)frameOfColumn:(NSInteger)column {
   NSRect result;
   NSRect titleFrame=[self titleFrameOfColumn:column];

   result.origin.x=titleFrame.origin.x;
   result.size.width=titleFrame.size.width;
   result.origin.y=[self bounds].origin.y;
   result.size.height=[self bounds].size.height;

   if([self isTitled])
    result.size.height-=titleFrame.size.height+[self seperatorWidth];

   if([self hasHorizontalScroller]){
    result.origin.y+=[self frameOfScrollerBorder].size.height;
    result.size.height-=[self frameOfScrollerBorder].size.height;
   }

   return result;
}

-(NSRect)frameOfInsideOfColumn:(NSInteger)column {
   NSScrollView *scrollView=[_scrollViews objectAtIndex:0];
   NSSize        size=[scrollView contentSize];

   return NSMakeRect(0,0,size.width,size.height);
}

-(void)setDelegate:delegate {
    // check delegate validity
    if(![delegate respondsToSelector:@selector(browser:createRowsForColumn:inMatrix:)])
        if (![delegate respondsToSelector:@selector(browser:numberOfRowsInColumn:)] &&
            ![delegate respondsToSelector:@selector(browser:willDisplayCell:atRow:column:)])
            [NSException raise:NSInternalInconsistencyException
                        format:@"-[NSBrowser setDelegate:] doesn't implement delegate methods"];

    _delegate=delegate;
}

-(void)setBackgroundColor:(NSColor *)color {
   color=[color retain];
   [_backgroundColor release];
   _backgroundColor=color;
}

-(void)setDoubleAction:(SEL)action {
   _doubleAction=action;
}

-(void)setMatrixClass:(Class)class {
   _matrixClass=class;
}

-(void)setCellClass:(Class)class {
   _cellClass=class;
}

-(void)setMaxVisibleColumns:(NSInteger)count {
   _maxVisibleColumns=count;
   _numberOfVisibleColumns=count;
   [self tile];
   [self setNeedsDisplay:YES];
}

-(void)setHasHorizontalScroller:(BOOL)flag {
   _hasHorizontalScroller=flag;
   if(flag) {
    if(_horizontalScroller==nil){
       _horizontalScroller = [[NSScroller alloc] initWithFrame:[self frameOfScroller]];
       [_horizontalScroller setTarget:self];
       [_horizontalScroller setAction:@selector(scrollViaScroller:)];
     }
   }
   else {
       [_horizontalScroller release];
       _horizontalScroller = nil;
   }
}

-(void)setSeparatesColumns:(BOOL)flag {
   _separatesColumns=flag;
   [self tile];
   [self setNeedsDisplay:YES];
}

-(void)setTitled:(BOOL)flag {
   _isTitled=flag;
   [self tile];
   [self setNeedsDisplay:YES];
}

-(void)setTakesTitleFromPreviousColumn:(BOOL)flag {
   _takesTitleFromPreviousColumn=flag;
}

-(void)setAllowsMultipleSelection:(BOOL)flag {
   _allowsMultipleSelection=flag;
}

-(void)setAllowsBranchSelection:(BOOL)flag {
   _allowsBranchSelection=flag;
}

-(void)setAllowsEmptySelection:(BOOL)flag {
   _allowsEmptySelection=flag;
}

-(void)setAcceptsArrowKeys:(BOOL)flag {
   _acceptsArrowKeys=flag;
}

-(void)setSendsActionOnArrowKeys:(BOOL)flag {
   _sendsActionOnArrowKeys=flag;
}

-(void)setReusesColumns:(BOOL)flag {
    _reusesColumns=flag?YES:NO;
}

-(void)setTitle:(NSString *)title ofColumn:(NSInteger)column {
   while([_explicitTitles count]<=column)
    [_explicitTitles addObject:@""];

   [_explicitTitles replaceObjectAtIndex:column withObject:title];
   [self setNeedsDisplay:YES];
}

-(void)_unloadAfterColumn:(NSInteger)column {
    while([_matrices count]>column+1){
        NSMatrix *matrix=[_matrices lastObject];

        [matrix removeFromSuperview];
        [_matrices removeLastObject];
        [_titles removeLastObject];
    }

    [self setNeedsDisplay:YES];
    [self updateScroller];
}

-(void)_reloadSelectionInColumn:(NSInteger)column {
   NSMatrix *matrix=[self matrixInColumn:column];
   NSArray  *selectedCells=[matrix selectedCells];

   [_cell autorelease];
   _cell=[[matrix selectedCell] retain];
   _selectedColumn=column;

    // if multiple cells are selected, the display of that branch is invalid and must be unloaded.
   if([selectedCells count]>1)
    [self _unloadAfterColumn:_selectedColumn];
   else if(_cell!=nil && ![_cell isLeaf]){
    [self reloadColumn:_selectedColumn+1];
    [self _unloadAfterColumn:_selectedColumn+1];

    if(_selectedColumn==[self lastVisibleColumn])
     [self scrollColumnsRightBy:1];
   }
   else if(_cell!=nil)	// valid cell, no following branch
    [self _unloadAfterColumn:_selectedColumn];

   [self scrollColumnToVisible:_selectedColumn];
}

-(void)selectRow:(NSInteger)row inColumn:(NSInteger)column {
   _selectedColumn = column;

   if(column<[_matrices count]){
    NSMatrix *matrix=[self matrixInColumn:column];

#if 0
    if([self allowsMultipleSelection]){
     NSBrowserCell *theLuckyCell = [matrix cellAtRow:row column:0];

     // branch selection logic. "whether the user can 
     // select branch items when multiple selection is enabled"
     if ([self selectedCells] > 0 && theLuckyCell != [self selectedCell])
      if (![theLuckyCell isLeaf])
       if (![self allowsBranchSelection])
        return;
    }
#endif

    [matrix selectCellAtRow:row column:0];       
    [matrix scrollRectToVisible:[matrix cellFrameAtRow:row column:0]];

    [self _reloadSelectionInColumn:column];
   }
}

-(void)setPath:(NSString *)path {
   NSUnimplementedMethod();
}

-(BOOL)sendAction {
   return [self sendAction:[self action] to:[self target]];
}

-(void)doClick:sender {
    [self _reloadSelectionInColumn:[self columnOfMatrix:sender]];

    if ([[[self window] currentEvent] type] != NSKeyDown || _sendsActionOnArrowKeys == YES)
        [self sendAction];
}

-(void)doDoubleClick:sender {
   [self sendAction:[self doubleAction] to:[self target]];
}

-(void)unloadLastColumn {
   NSMatrix *matrix=[_matrices lastObject];

   [(NSClipView *)[matrix superview] setDocumentView:nil];
   [_matrices removeLastObject];
   [_titles removeLastObject];
   [self setNeedsDisplay:YES];
   [self updateScroller];
}

-(void)loadColumnZero {
   while([_matrices count]>1)
    [self unloadLastColumn];

   [self reloadColumn:0];
}

-(void)viewWillDraw {
    // This should be a flag really
    
    if([_matrices count]==0 || [[_matrices objectAtIndex:0] numberOfRows]==0){
    [self loadColumnZero];
   }
   
   [super viewWillDraw];
}

-(NSMatrix *)createMatrixInColumn:(NSInteger)column {
   while([_matrices count]<=column){
    NSRect    frame=[self frameOfInsideOfColumn:column];
    NSMatrix *matrix=[[[_matrixClass alloc] initWithFrame:frame] autorelease];

    [matrix setTarget:self];
    [matrix setAction:@selector(doClick:)];
    [matrix setDoubleAction:@selector(doDoubleClick:)];

    [matrix setCellClass:[self cellClass]];
    [matrix setCellSize:NSMakeSize(frame.size.width,16)];
    [matrix setAutoresizingMask:NSViewWidthSizable];
    [matrix setAutosizesCells:YES];
    [matrix setAllowsEmptySelection:[self allowsEmptySelection]];

    if([self allowsMultipleSelection])
     [matrix setMode:NSListModeMatrix];
    else
     [matrix setMode:NSRadioModeMatrix];

#if 0
    if(column>=[self firstVisibleColumn] && column<=[self lastVisibleColumn]){
     NSScrollView *scrollView=[_scrollViews objectAtIndex:column-[self firstVisibleColumn]];

     [scrollView setDocumentView:matrix];
    }
#endif
 
    [_titles addObject:[NSNull null]];
    [_matrices addObject:matrix];
   }

    if(column>=[self firstVisibleColumn] && column<=[self lastVisibleColumn]){
        NSScrollView *scrollView=[_scrollViews objectAtIndex:column-[self firstVisibleColumn]];
        
        [scrollView setDocumentView:[_matrices objectAtIndex:column]];
        [scrollView setLineScroll:[[_matrices objectAtIndex:column] cellSize].height];
        [scrollView setPageScroll:[[scrollView contentView] frame].size.height];
    }
    
   [self updateScroller];
   return [_matrices objectAtIndex:column];
}

-(NSMatrix *)_reloadColumn:(NSInteger)column preserveSelection:(BOOL)preserveSelection {
   NSMatrix        *matrix=[self createMatrixInColumn:column];
   NSInteger              selectedRow=[matrix selectedRow],selectedColumn=[matrix selectedColumn];
   id               title=[NSNull null];

   [matrix renewRows:0 columns:1];

   if([_delegate respondsToSelector:@selector(browser:createRowsForColumn:inMatrix:)])
    [_delegate browser:self createRowsForColumn:column inMatrix:matrix];
   else {
    NSInteger nrows=[_delegate browser:self numberOfRowsInColumn:column];
    NSInteger i;

    [matrix renewRows:nrows columns:1];

    for(i=0;i<nrows;i++){
     [_delegate browser:self willDisplayCell:[matrix cellAtRow:i column:0] atRow:i column:column];
    }
   }

   [matrix sizeToFit];

   if(preserveSelection)
    [matrix selectCellAtRow:selectedRow column:selectedColumn];
   else
    [self _unloadAfterColumn:column];

   [matrix setNeedsDisplay:YES];

   if([_delegate respondsToSelector:@selector(browser:titleOfColumn:)])
    title=[_delegate browser:self titleOfColumn:column];
   else if(_takesTitleFromPreviousColumn){
    if(column>0){
     title=[[[self matrixInColumn:column-1] selectedCell] stringValue];
    }
   }

   [_titles replaceObjectAtIndex:column withObject:title];

   return matrix;
}

-(void)reloadColumn:(NSInteger)column {
   [self _reloadColumn:column preserveSelection:NO];
}

-(void)addColumn {
   NSUnimplementedMethod();
}

-(void)setLastColumn:(NSInteger)column {
   [self _reloadColumn:column preserveSelection:YES];
   
   while([_matrices count]>column+1)
    [_matrices removeLastObject];
    
   [self tile];
}

-(void)validateVisibleColumns {
   NSInteger  i;

   if([_delegate respondsToSelector:@selector(browser:isColumnValid:)]){
    for(i=[self firstVisibleColumn];i<=[self lastVisibleColumn];i++){
     NSMatrix *matrix;

     if(![_delegate browser:self isColumnValid:i])
      [self _reloadColumn:i preserveSelection:YES];

     matrix=[self matrixInColumn:i];
     if([matrix selectedRow]<0){
      [self _unloadAfterColumn:i];
      break;
     }
    }
   }
}

-(void)scrollColumnsLeftBy:(NSInteger)offset {
    NSInteger i, scrollViewIndex = 0;
    
    if (_firstVisibleColumn - offset < 0)
        offset = _firstVisibleColumn;

    if (offset < 1)
        return;
    
    _firstVisibleColumn -= offset;

    for (i = _firstVisibleColumn; i <= [self lastVisibleColumn] && i<[_matrices count]; i++){
     NSView * matrix =(i<[_matrices count])?[_matrices objectAtIndex:i]:nil;

      [[_scrollViews objectAtIndex:scrollViewIndex++] setDocumentView:matrix];
    }

    [self updateScroller];
}

-(void)scrollColumnsRightBy:(NSInteger)offset {
    NSInteger i, scrollViewIndex = 0;
    
    if ([self lastVisibleColumn] + offset >= [_matrices count]) 
        offset = [_matrices count] - [self lastVisibleColumn] - 1;

    if (offset < 1)
        return;
       
    _firstVisibleColumn += offset;

    // NB here we swap in reverse, since a view doesn't display properly if it is
    // the docView of two scrollViews at once. bug or feature?
    scrollViewIndex = [_scrollViews count]-1;
    for (i = [self lastVisibleColumn]; i >= _firstVisibleColumn; --i){
        [[_scrollViews objectAtIndex:scrollViewIndex--] setDocumentView:[_matrices objectAtIndex:i]];
    }
        
    [self updateScroller];
}

-(void)scrollColumnToVisible:(NSInteger)column {
    if (column >= [self firstVisibleColumn] && column <= [self lastVisibleColumn])
        return;	// already visible

    if (column < _firstVisibleColumn)
        [self scrollColumnsLeftBy:_firstVisibleColumn-column];
    else
        [self scrollColumnsRightBy:column-[self lastVisibleColumn]];
}

-(void)scrollViaScroller:(NSScroller *)sender {
    switch ([sender hitPart]) {
        case NSScrollerDecrementLine:
        case NSScrollerDecrementPage:
            [self scrollColumnsLeftBy:1];
            break;

        case NSScrollerIncrementLine:
        case NSScrollerIncrementPage:
            [self scrollColumnsRightBy:1];
            break;

        case NSScrollerKnob:
        case NSScrollerKnobSlot:
            [self scrollColumnToVisible:[sender floatValue]*[_matrices count]];
            break;

        default:
            break;
    }

    [self setNeedsDisplay:YES];
}

-(void)updateScroller {
    if (_firstVisibleColumn+_numberOfVisibleColumns > [_matrices count])
        [self scrollColumnsLeftBy:1];

    if ([_matrices count] == 0 || [_matrices count] <= _numberOfVisibleColumns) {
        [_horizontalScroller setFloatValue:0.0 knobProportion:1.0];
        [_horizontalScroller setEnabled:NO];
    }
    else {
        float val = (float)_firstVisibleColumn/(float)([_matrices count]-_numberOfVisibleColumns);
        float knobP = (float)_numberOfVisibleColumns/(float)[_matrices count];

        [_horizontalScroller setFloatValue:val knobProportion:knobP];
        [_horizontalScroller setEnabled:YES];
    }

    [_horizontalScroller setNeedsDisplay:YES];
}

-(void)drawTitleOfColumn:(NSInteger)column inRect:(NSRect)rect {
   NSString *title=[self titleOfColumn:column];
   NSSize    titleSize=[title sizeWithAttributes:nil];
   NSRect    titleFrame=NSInsetRect(rect,2,0);

   titleFrame.size.height=titleSize.height;
   titleFrame.origin.y+=floor((rect.size.height-titleSize.height)/2);

   [[self titleOfColumn:column] _clipAndDrawInRect:titleFrame withAttributes:nil];
}

-(NSScrollView *)createScrollViewAtIndex:(NSInteger)index {
   while([_scrollViews count]<=index){
    NSInteger           column=_firstVisibleColumn+[_scrollViews count];
    NSRect        frame=[self frameOfColumn:column];
    NSScrollView *view=[[[NSScrollView alloc] initWithFrame:frame] autorelease];

    [view setBorderType:NSBezelBorder];
    [view setHasVerticalScroller:YES];
    [view setHasHorizontalScroller:NO];
    [view setAutoresizesSubviews:YES];

    [_scrollViews addObject:view];
    [self addSubview:view];
   }

   return [_scrollViews objectAtIndex:index];
}

-(void)tile {
   NSInteger i;

   for(i=0;i<_numberOfVisibleColumns;i++){
    NSInteger     column=_firstVisibleColumn+i;
    NSRect  frame=[self frameOfColumn:column];
    NSView *scrollView=[self createScrollViewAtIndex:i];

    [scrollView setFrame:frame];
   }

   while([_scrollViews count]>_firstVisibleColumn+i){
    [[_scrollViews lastObject] removeFromSuperview];
    [_scrollViews removeLastObject];
   }

   [_horizontalScroller setFrame:[self frameOfScroller]];
}

-(void)setEnabled:(BOOL)enabled {
   NSInteger count=[_matrices count];

   while(--count>=0){
    [[_matrices objectAtIndex:count] setEnabled:enabled];
   }
   [self setNeedsDisplay:YES];
}

-(void)drawRect:(NSRect)rect {
   NSInteger i;

   [_backgroundColor setFill];
   NSRectFill(rect);

   if([self isTitled]){    
    for(i=[self firstVisibleColumn];i<=[self lastVisibleColumn];i++){
     NSRect titleRect=[self titleFrameOfColumn:i];

     [[self graphicsStyle] drawBrowserTitleBackgroundInRect:titleRect];
     
     titleRect=NSInsetRect(titleRect,2,2);

     [self drawTitleOfColumn:i inRect:titleRect];
    }
   }

   if([self hasHorizontalScroller])
    [[self graphicsStyle] drawBrowserHorizontalScrollerWellInRect:[self frameOfScrollerBorder] clipRect:rect];
}

-(void)resizeSubviewsWithOldSize:(NSSize)oldSize {
   [self tile];
}

-(void)mouseDown:(NSEvent *)event {
   // do nothing, but override NSControl's implementation
}


-(void)keyDown:(NSEvent *)event {
    if (_acceptsArrowKeys == YES)
        [self interpretKeyEvents:[NSArray arrayWithObject:event]];
}

//  up arrow
-(void)moveBackward:sender {
    NSInteger selectedRow = [self selectedRowInColumn:_selectedColumn];

    if (selectedRow > 0) {
        if (!([[[self window] currentEvent] modifierFlags] & NSShiftKeyMask))
            [[self matrixInColumn:_selectedColumn] deselectAllCells];
        
        [self selectRow:selectedRow - 1 inColumn:_selectedColumn];
        [self doClick:[self matrixInColumn:_selectedColumn]];
    }
}

-(void)moveForward:sender {
    NSInteger selectedRow = [self selectedRowInColumn:_selectedColumn];
    if (selectedRow < [[self matrixInColumn:_selectedColumn] numberOfRows] - 1) {
        if (!([[[self window] currentEvent] modifierFlags] & NSShiftKeyMask))
            [[self matrixInColumn:_selectedColumn] deselectAllCells];

        [self selectRow:selectedRow + 1 inColumn:_selectedColumn];
        [self doClick:[self matrixInColumn:_selectedColumn]];
    }
}

-(void)moveUp:sender {
   [self moveBackward:sender];
}

-(void)moveDown:sender {
   [self moveForward:sender];
}

-(void)moveLeft:sender {
    if (_selectedColumn > 0) {
        if (![[self selectedCell] isLeaf])
            [self _unloadAfterColumn:_selectedColumn];
        if ([self allowsEmptySelection] == YES)
            [[self matrixInColumn:_selectedColumn] deselectAllCells];
        
//        [self selectRow:0 inColumn:_selectedColumn - 1];
        _selectedColumn--;
        //NSLog(@"selectedColumn %d count %d", _selectedColumn, [_matrices count]);
        [self doClick:[self matrixInColumn:_selectedColumn]];
    }
}

-(void)moveRight:sender {
    if (![[self selectedCell] isLeaf]) {
        [self selectRow:0 inColumn:_selectedColumn + 1];	// nb this changes _selectedColumn
        [self doClick:[self matrixInColumn:_selectedColumn]];
    }
}

@end
