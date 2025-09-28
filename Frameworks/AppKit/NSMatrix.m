/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSMatrix.h>
#import <AppKit/NSCell.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSWindow.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSRaise.h>

@implementation NSMatrix

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];

   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    int flags=[keyed decodeIntForKey:@"NSMatrixFlags"];
    NSString *name;
    
    _numberOfRows=[keyed decodeIntForKey:@"NSNumRows"];
    _numberOfColumns=[keyed decodeIntForKey:@"NSNumCols"];
    _cellSize=[keyed decodeSizeForKey:@"NSCellSize"];
    _intercellSpacing=[keyed decodeSizeForKey:@"NSIntercellSpacing"];
    _tabKeyTraversesCells=(flags&0x00200000)?YES:NO;
    _autosizesCells=(flags&0x00800000)?YES:NO;
    _drawsBackground=(flags&0x01000000)?YES:NO;
    _drawsCellBackground=(flags&0x02000000)?YES:NO;
    _selectionByRect=(flags&0x04000000)?YES:NO;
    _isAutoscroll=(flags&0x08000000)?YES:NO;
    _allowsEmptySelection=(flags&0x10000000)?YES:NO;
    _mode=NSTrackModeMatrix;
    if(flags&0x20000000)
     _mode=NSListModeMatrix;
    if(flags&0x40000000)
     _mode=NSRadioModeMatrix;
    if(flags&0x80000000)
     _mode=NSHighlightModeMatrix;
    _backgroundColor=[[keyed decodeObjectForKey:@"NSBackgroundColor"] retain];
    _cellBackgroundColor=[[keyed decodeObjectForKey:@"NSCellBackgroundColor"] retain];
    name=[keyed decodeObjectForKey:@"NSCellClass"];
    if((_cellClass=NSClassFromString(name))==Nil){
        if (name) {
            NSLog(@"Unknown cell class '%@' in NSMatrix, using NSCell",name);
        }
     _cellClass=[NSCell class];
    }
    _prototype=[[keyed decodeObjectForKey:@"NSProtoCell"] retain];
    _cells=[[NSMutableArray new] initWithArray:[keyed decodeObjectForKey:@"NSCells"]];
    id selectedCell=[keyed decodeObjectForKey:@"NSSelectedCell"];
    if ((_selectedIndex=[_cells indexOfObjectIdenticalTo:selectedCell]) != NSNotFound)
      [self selectCell:selectedCell];
    else
      _selectedIndex=-1;
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   }
   return self;
}

-initWithFrame:(NSRect)frame {
   [super initWithFrame:frame];
   _target=nil;
   _action=NULL;
   _doubleAction=NULL;
   _font=[[NSFont userFontOfSize:0] retain];
   _cells=[NSMutableArray new];
   _numberOfRows=0;
   _numberOfColumns=1;
   _selectedIndex=-1;
   _keyCellIndex=-1;
   _cellSize=NSMakeSize(frame.size.width,20);
   _intercellSpacing=NSMakeSize(0,0);
   _prototype=nil;
   _cellClass=[NSCell class];
   _mode=NSListModeMatrix;
   _tabKeyTraversesCells=YES;

   return self;
}

-initWithFrame:(NSRect)frame mode:(int)mode prototype:(NSCell *)prototype numberOfRows:(int)rows numberOfColumns:(int)columns {
   int i;
   
   [self initWithFrame:frame];
   
   _mode=mode;
   _prototype=[prototype copy];
   _numberOfRows=rows;
   _numberOfColumns=columns;
   for(i=0;i<rows*columns;i++)
    [_cells addObject:[[_prototype copy] autorelease]];
    
   return self;
}

-initWithFrame:(NSRect)frame mode:(int)mode cellClass:(Class)cls numberOfRows:(int)rows numberOfColumns:(int)columns {
   int i;
   
   [self initWithFrame:frame];
   
   _mode=mode;
   _cellClass=cls;
   _numberOfRows=rows;
   _numberOfColumns=columns;
   for(i=0;i<rows*columns;i++)
    [_cells addObject:[[[cls alloc] init] autorelease]];
    
   return self;
}

-(void)dealloc {
   [_font release];
   [_backgroundColor release];
   [_cellBackgroundColor release];
   [_cells release];
   [_prototype release];
   [super dealloc];
}

-(BOOL)resignFirstResponder {
   [self setNeedsDisplay:YES];
   return [super resignFirstResponder];
}

-(void)setRefusesFirstResponder:(BOOL)flag {
   _refusesFirstResponder=flag;
}

-(BOOL)refusesFirstResponder {
   return _refusesFirstResponder;
}

-(void)resizeWithOldSuperviewSize:(NSSize)oldSize {
   [super resizeWithOldSuperviewSize:oldSize];

   if(_autosizesCells){
    NSRect frame=[self frame];

    if(_autoresizingMask&NSViewWidthSizable){
     if(_numberOfColumns==0)
      _cellSize.width=frame.size.width;
     else
      _cellSize.width=(frame.size.width-
         (_numberOfColumns-1)*_intercellSpacing.width)/_numberOfColumns;
    }

    if(_autoresizingMask& NSViewHeightSizable){
     if(_numberOfRows==0)
      _cellSize.height= frame.size.height;
     else
      _cellSize.height=(frame.size.height-
        (_numberOfRows-1)*_intercellSpacing.height)/_numberOfRows;
    }
   }
}

-(void)sizeToFit {
   NSSize size;

   size.width=_cellSize.width*_numberOfColumns+
       _intercellSpacing.width*(_numberOfColumns-1);
   size.height=_cellSize.height*_numberOfRows+
       _intercellSpacing.height*(_numberOfRows-1);

   [self setFrameSize:size];
}

-(BOOL)isFlipped {
   return YES;
}

-(void)resetCursorRects {
   NSRect visibleRect=[self visibleRect];
   int    row,col;

   for(row=0;row<_numberOfRows;row++)
    for(col=0;col<_numberOfColumns;col++){
     NSRect frame=[self cellFrameAtRow:row column:col];

     if(NSIntersectsRect(frame,visibleRect)){
      NSCell *cell=[self cellAtRow:row column:col];

      [cell resetCursorRect:frame inView:self];
     }
    } 
}

-(NSFont *)font {
   return _font;
}

-(void)setFont:(NSFont *)font {
   font=[font retain];
   [_font release];
   _font=font;
}

-target {
   return _target;
}

-(SEL)action {
   return _action;
}

-delegate {
   return _delegate;
}

-(SEL)doubleAction {
   return _doubleAction;
}

-(Class)cellClass {
   return _cellClass;
}

-prototype {
   return _prototype;
}

-(NSArray *)cells {
   return _cells;
}

-cellWithTag:(int)tag {
   int i,count=[_cells count];

   for(i=0;i<count;i++){
    NSCell *cell=[_cells objectAtIndex:i];
    if([cell tag]==tag)
     return cell;
   }

   return nil;
}

-cellAtRow:(int)row column:(int)column {
   unsigned index=row*_numberOfColumns+column;

   if(index>=[_cells count])
    return nil;

   // bounds checking added for keyboard spatial movement, hopefully doesn't blow up anything
   // which relies on broken behavior
   if (row < 0 || row >= _numberOfRows)
       return nil;
   if (column < 0 || column >= _numberOfColumns)
       return nil;

   return [_cells objectAtIndex:index];
}

-(NSRect)cellFrameAtRow:(int)row column:(int)column {
   NSRect result;

   result.origin.x=column*_cellSize.width+column*_intercellSpacing.width;
   result.origin.y=row*_cellSize.height+row*_intercellSpacing.height;
   result.size=_cellSize;

   return result;
}

-(BOOL)getRow:(int *)row column:(int *)column ofCell:(NSCell *)cell {
   int index=[_cells indexOfObjectIdenticalTo:cell];

   if(index!=NSNotFound){
    *row=index/_numberOfColumns;
    *column=index%_numberOfColumns;
    return YES;
   }

   return NO;
}

-(BOOL)getRow:(int *)row column:(int *)column forPoint:(NSPoint)point {
   NSRect cellFrame;

   *row=point.y/(_cellSize.height+_intercellSpacing.height);
   *column=point.x/(_cellSize.width+_intercellSpacing.width);

   cellFrame=[self cellFrameAtRow:*row column:*column];

   return NSMouseInRect(point,cellFrame,[self isFlipped]);
}

-(int)numberOfRows {
   return _numberOfRows;
}

-(int)numberOfColumns {
   return _numberOfColumns;
}

-(void)getNumberOfRows:(int *)rows columns:(int *)columns {
   *rows=_numberOfRows;
   *columns=_numberOfColumns;
}

-(NSString *)toolTipForCell:(NSCell *)cell {
   NSUnimplementedMethod();
   return nil;
}

-keyCell {
   if(_keyCellIndex==-1)
    return nil;

   return [_cells objectAtIndex:_keyCellIndex];
}

-(NSMatrixMode)mode {
   return _mode;
}

-(BOOL)allowsEmptySelection {
   return _allowsEmptySelection;
}

-(BOOL)tabKeyTraversesCells {
   return _tabKeyTraversesCells;
}

-(BOOL)autosizesCells {
   return _autosizesCells;
}

-(NSSize)cellSize {
   return _cellSize;
}

-(NSSize)intercellSpacing {
   return _intercellSpacing;
}

-(BOOL)drawsBackground {
   return _drawsBackground;
}

-(NSColor *)backgroundColor {
   return _backgroundColor;
}

-(BOOL)drawsCellBackground {
   return _drawsCellBackground;
}

-(NSColor *)cellBackgroundColor {
   return _cellBackgroundColor;
}

-(BOOL)isAutoscroll {
   return _isAutoscroll;
}

-(int)selectedRow {
   if(_selectedIndex<0 || _numberOfRows==0 || _numberOfColumns==0)
    return -1;

   return _selectedIndex/_numberOfColumns;
}

-(int)selectedColumn {
   if(_selectedIndex<0 || _numberOfRows==0 || _numberOfColumns==0)
    return -1;

   return _selectedIndex%_numberOfColumns;
}

-selectedCell {
   if(_selectedIndex<0)
    return nil;

// FIX shouldn't need this check if _selectedIndex is kept in sync
   if(_selectedIndex>=[_cells count])
    return nil;

   return [_cells objectAtIndex:_selectedIndex];
}

-(NSArray *)selectedCells {
   NSMutableArray *result=[NSMutableArray array];
   int i,count=[_cells count];

   for(i=0;i<count;i++){
    NSCell *cell=[_cells objectAtIndex:i];

    if([cell isHighlighted])
     [result addObject:cell];
   }

   return result;
}

-(void)setDelegate:delegate {
   if(_delegate==delegate)
    return;
    
   NSNotificationCenter *center=[NSNotificationCenter defaultCenter];
   struct {
    SEL       selector;
    NSString *name;
   } notes[]={
    { @selector(controlTextDidBeginEditing:), NSControlTextDidBeginEditingNotification },
    { @selector(controlTextDidChange:), NSControlTextDidChangeNotification },
    { @selector(controlTextDidEndEditing:), NSControlTextDidEndEditingNotification },
    { NULL, nil }
   };
   int i;

   if(_delegate!=nil)
    for(i=0;notes[i].selector!=NULL;i++)
     [center removeObserver:_delegate name:notes[i].name object:self];

   _delegate=delegate;

   for(i=0;notes[i].selector!=NULL;i++)
    if([_delegate respondsToSelector:notes[i].selector])
     [center addObserver:_delegate selector:notes[i].selector name:notes[i].name object:self];
}

-(void)setTarget:target {
   _target=target;
}

-(void)setAction:(SEL)action {
   _action=action;
}

-(void)setDoubleAction:(SEL)action {
   _doubleAction=action;
}

-(void)setCellClass:(Class)class {
   _cellClass=class;
}

-(void)setPrototype:(NSCell *)cell {
   cell=[cell retain];
   [_prototype release];
   _prototype=cell;
}

-(void)_deselectAllCells {
   int i,count=[_cells count];

   _selectedIndex=-1;
   _keyCellIndex=-1;

   for(i=0;i<count;i++)
    [[_cells objectAtIndex:i] setState:NSOffState];
}

-(void)renewRows:(int)rows columns:(int)columns {
   while(_numberOfRows<rows)
    [self addRow];
   while(_numberOfRows>rows)
    [self removeRow:_numberOfRows-1];
   while(_numberOfColumns<columns)
    [self addColumn];
   while(_numberOfColumns>columns)
    [self removeColumn:_numberOfColumns-1];

   [self _deselectAllCells];
}

-(NSCell *)makeCellAtRow:(int)row column:(int)column {
   NSCell *result=[[_prototype copy] autorelease];

   if(result==nil)
    result=[[[_cellClass alloc] init] autorelease];

   return result;
}

-(void)putCell:(NSCell *)cell atRow:(int)row column:(int)column {
   unsigned index=row*_numberOfColumns+column;

   [_cells replaceObjectAtIndex:index withObject:cell];
   [self setNeedsDisplayInRect:[self cellFrameAtRow:row column:column]];
}

-(void)addRow {
   int i;

   for(i=0;i<_numberOfColumns;i++)
    [_cells addObject:[self makeCellAtRow:_numberOfRows column:i]];

   _numberOfRows++;
}

-(void)insertRow:(int)row {
   int i;

   for(i=0;i<_numberOfColumns;i++){
    [_cells insertObject:[self makeCellAtRow:row column:i]
                 atIndex:row*_numberOfColumns];
   }
   _numberOfRows++;
}

-(void)insertRow:(int)row withCells:(NSArray *)cells {
   int i;

   for(i=0;i<_numberOfColumns;i++)
    [_cells insertObject:[cells objectAtIndex:i] atIndex:row*_numberOfColumns+i];
    
   _numberOfRows++;
}

-(void)removeRow:(int)row {
   int i;

   for(i=0;i<_numberOfColumns;i++){
    [_cells removeObjectAtIndex:row*_numberOfColumns];
   }

   _numberOfRows--;
}

-(void)addColumn {
   int i=_numberOfRows;
   for(;i>0;i--){
    NSCell *cell=[self makeCellAtRow:i-1 column:_numberOfColumns];

    [_cells insertObject:cell atIndex:i*_numberOfColumns];
   }

   _numberOfColumns++;
}

-(void)removeColumn:(int)col {
   int i;

   for(i=_numberOfRows;i>=1;i--)
    [_cells removeObjectAtIndex:(i*col)];

   _numberOfColumns--;
}

-(void)setToolTip:(NSString *)tip forCell:(NSCell *)cell {
}

-(void)setKeyCell:cell {
   _keyCellIndex=[_cells indexOfObjectIdenticalTo:cell];
}

-(void)setMode:(NSMatrixMode)mode {
   _mode=mode;
}

-(void)setAllowsEmptySelection:(BOOL)flag {
   _allowsEmptySelection=flag;
}

-(void)setTabKeyTraversesCells:(BOOL)flag {
   _tabKeyTraversesCells=flag;
}

-(void)setAutosizesCells:(BOOL)flag {
   _autosizesCells=flag;
}

-(void)setCellSize:(NSSize)size {
   _cellSize=size;
}

-(void)setIntercellSpacing:(NSSize)size {
   _intercellSpacing=size;
}

-(void)setDrawsBackground:(BOOL)flag {
   _drawsBackground=flag;
}

-(void)setBackgroundColor:(NSColor *)color {
   color=[color retain];
   [_backgroundColor release];
   _backgroundColor=color;
   [self setNeedsDisplay:YES];
}

-(void)setDrawsCellBackground:(BOOL)flag {
   _drawsCellBackground=flag;
}

-(void)setCellBackgroundColor:(NSColor *)color {
   color=[color retain];
   [_cellBackgroundColor release];
   _cellBackgroundColor=color;
   [self setNeedsDisplay:YES];
}

-(void)setAutoscroll:(BOOL)flag {
   _isAutoscroll=flag;
}

-(void)selectCellAtRow:(int)row column:(int)column {
   NSCell *cell;

   if(row<0 || row>=_numberOfRows)
    cell=nil;
   else if(column<0 || column>=_numberOfColumns)
    cell=nil;
   else 
    cell=[_cells objectAtIndex:row*_numberOfColumns+column];

   [self selectCell:cell];
}

-(void)_setSelectedIndexFromCell:(NSCell *)select {
	[self willChangeValueForKey:@"selectedTag"];
	[self willChangeValueForKey:@"selectedIndex"];
   if(select==nil)
    _selectedIndex=-1;
   else
    _selectedIndex=[_cells indexOfObjectIdenticalTo:select];

   _keyCellIndex=_selectedIndex;
	[self didChangeValueForKey:@"selectedIndex"];
	[self didChangeValueForKey:@"selectedTag"];
}

-(void)_selectCell:(NSCell *)select deselectOthers:(BOOL)deselectOthers {
   if([self mode]==NSRadioModeMatrix){
    if(![self allowsEmptySelection] && select==nil)
     return;
   }

   if(deselectOthers)
    [self _deselectAllCells];

   [self _setSelectedIndexFromCell:select];
   [select setState:NSOnState];
   if([self mode]==NSListModeMatrix)
    [select setHighlighted:YES];

   [self setNeedsDisplay:YES];
}

-(void)selectCell:(NSCell *)select {
   [self _selectCell:select deselectOthers:YES];
}

-(BOOL)selectCellWithTag:(int)tag {
   NSCell *cell=[self cellWithTag:tag];

   [self selectCell:cell];

   return (cell!=nil)?YES:NO;
}

-(void)selectAll:sender {
    int i, count = [_cells count];

    for (i=0; i < count; ++i)
        [self _selectCell:[_cells objectAtIndex:i] deselectOthers:NO];
}

-(void)setSelectionFrom:(int)from to:(int)to anchor:(int)anchor highlight:(BOOL)highlight {
    if (anchor != -1) {	// no anchor, i.e., no selected cell
        if (anchor < from)
            from = anchor;
        if (anchor > to)
            to = anchor;
    }

    [self _deselectAllCells];
    while (from < to) {
        NSCell *cell = [_cells objectAtIndex:from];
        
        [self _selectCell:cell deselectOthers:NO];
        if (highlight)
            [cell setHighlighted:YES];
        from++;
    }
}

-(void)_deselectAllCellsInLockFocus {
   int row,column;

   for(row=0;row<_numberOfRows;row++)
    for(column=0;column<_numberOfColumns;column++){
     NSCell *cell=[self cellAtRow:row column:column];
     NSRect  cellFrame=[self cellFrameAtRow:row column:column];

     [cell setControlView:self];
     [cell setState:NSOffState];
     [cell setHighlighted:NO];
     [cell drawWithFrame:cellFrame inView:self];
    }
}

-(void)deselectAllCells {
   if([self mode]==NSRadioModeMatrix && [self allowsEmptySelection]==NO)
    return;

   [self lockFocus];
   [self _deselectAllCellsInLockFocus];
   [self unlockFocus];
   [[self window] flushWindow];

   _selectedIndex=-1;
   _keyCellIndex=-1;
}

-(void)deselectSelectedCell {
   int row,column;

   if ([self mode] == NSRadioModeMatrix && [self allowsEmptySelection] == NO)
       return;

   for(row=0;row<_numberOfRows;row++)
    for(column=0;column<_numberOfColumns;column++){
     NSCell *cell=[self cellAtRow:row column:column];

     [cell setState:NSOffState];
     [self drawCellAtRow:row column:column];
    }
    _selectedIndex=-1;
    _keyCellIndex=-1;
}

-(void)sizeToCells {
   [self sizeToFit];
}

-(void)setState:(int)state atRow:(int)row column:(int)column {
   NSCell *cell=[self cellAtRow:row column:column];

   if(cell!=nil){
    NSRect frame=[self cellFrameAtRow:row column:column];
    
    [cell setState:state];
    [self setNeedsDisplayInRect:frame];
   }
}

-(void)highlightCell:(BOOL)highlight atRow:(int)row column:(int)column {
   NSCell *cell=[self cellAtRow:row column:column];

   if(cell!=nil){
    NSRect frame=[self cellFrameAtRow:row column:column];

    [self lockFocus];
    [cell highlight:highlight withFrame:frame inView:self];
    [self unlockFocus];
    [[self window] flushWindow];
   }
}

-(void)drawCellAtRow:(int)row column:(int)column {
   NSCell *cell=[self cellAtRow:row column:column];

   [self drawCell:cell];
}

-(void)scrollCellToVisibleAtRow:(int)row column:(int)column {
   NSRect frame=[self cellFrameAtRow:row column:column];
   [self scrollRectToVisible:frame];
}

-(BOOL)sendAction {
   NSCell *cell=[self selectedCell];
   SEL     action=[cell action];
   id      target=[cell target];

   if(action==NULL){
    action=[self action];
    target=[self target];
   }
   else if(target==nil){
    target=[self target];
   }

   return [NSApp sendAction:action to:target from:self];
}

-(void)sendDoubleAction {
   SEL action=[self doubleAction];
   id  target=[self target];

   if(action==NULL){
    action=[[self selectedCell] action];
    target=nil;
   }

   if(action==NULL){
    action=[self action];
    target=[self target];
   }

   [NSApp sendAction:action to:target from:self];
}


-(void)setEnabled:(BOOL)enabled {
   int count=[_cells count];

   while(--count>=0){
    [[_cells objectAtIndex:count] setEnabled:enabled];
   }
   [self setNeedsDisplay:YES];
}


-(BOOL)isOpaque {
   return [self drawsBackground];
}

-(void)updateCell:(NSCell *)cell {
   int row,column;

   if([self getRow:&row column:&column ofCell:cell]){
    NSRect frame=[self cellFrameAtRow:row column:column];

    [self setNeedsDisplayInRect:frame];
   }
}

-(void)updateCellInside:(NSCell *)cell {
   int row,column;

   if([self getRow:&row column:&column ofCell:cell]){
    NSRect frame=[self cellFrameAtRow:row column:column];

    [self setNeedsDisplayInRect:frame];
   }
}




-(void)drawCell:(NSCell *)cell {
   int row,column;

   if([self getRow:&row column:&column ofCell:cell]){
    NSRect frame=[self cellFrameAtRow:row column:column];
    [self lockFocus];
    [cell setControlView:self];
    [cell drawWithFrame:frame inView:self];
    [self unlockFocus];
    [[self window] flushWindow];
   }
}



-(void)drawRect:(NSRect)rect {
   int row,col;

   if([self drawsBackground]){
    [[self backgroundColor] setFill];
    NSRectFill(rect);
   }

   for(row=0;row<_numberOfRows;row++)
    for(col=0;col<_numberOfColumns;col++){
     NSRect frame=[self cellFrameAtRow:row column:col];

     if(NSIntersectsRect(frame,rect)){
      NSCell *cell=[self cellAtRow:row column:col];
      [cell setControlView:self];
      [cell drawWithFrame:frame inView:self];
     }
    } 
}

-(void)_fieldEditCell:(NSCell *)cell row:(int)row column:(int)column {
   [self selectCell:cell];

   NSText* editor =[[self window] fieldEditor:YES forObject:self];
   _currentEditor=[[cell setUpFieldEditorAttributes: editor] retain];
}

-(void)_selectTextCell:(NSCell *)cell {
   int    row,column;
   NSRect cellFrame;

   if(![cell isEditable])
    return;

   [self getRow:&row column:&column ofCell:cell];
   cellFrame=[self cellFrameAtRow:row column:column];

   [self _fieldEditCell:cell row:row column:column];
   [cell selectWithFrame:cellFrame inView:self editor:_currentEditor delegate:self start:0 length:[[cell stringValue] length]];
}

-(void)_editTextCell:(NSCell *)cell row:(int)row column:(int)column event:(NSEvent *)event {
   NSRect cellFrame=[self cellFrameAtRow:row column:column];

   [self _fieldEditCell:cell row:row column:column];
   [cell editWithFrame:cellFrame inView:self editor:_currentEditor delegate:self event:event];
}

-(NSCell *)_enabledCellAtPoint:(NSPoint)point row:(int *)row column:(int *)column {
   if([self getRow:row column:column forPoint:point]){
    NSCell *cell=[self cellAtRow:*row column:*column];

    if([cell isEnabled])
     return cell;
   }

   return nil;
}

-(BOOL)_mouseDownRadio:(NSEvent *)event {
   BOOL     result=NO;
   NSEvent *lastMouse=event;

   do {
    NSPoint point=[self convertPoint:[lastMouse locationInWindow] fromView:nil];
    int     row,column;
    NSCell *cell;

    [[self superview] autoscroll:lastMouse];
    [self lockFocus];
    if((cell=[self _enabledCellAtPoint:point row:&row column:&column])!=nil){
     result=YES;

     if([self selectedCell]!=cell)
      [self highlightCell:NO atRow:[self selectedRow] column:[self selectedColumn]];

     [self highlightCell:YES atRow:row column:column];
     [self selectCell:cell];
    }
    [self unlockFocus];
    [[self window] flushWindow];
    event=[[self window] nextEventMatchingMask:NSLeftMouseUpMask|
                          NSLeftMouseDraggedMask|NSPeriodicMask];
    if([event type]!=NSPeriodic)
     lastMouse=event;

   }while([event type]!=NSLeftMouseUp);

   [self highlightCell:NO atRow:[self selectedRow] column:[self selectedColumn]];

   return result;
}

-(BOOL)_mouseDownHighlight:(NSEvent *)event {
   BOOL     result=NO;
   NSEvent *lastMouse=event;
   do {
    NSPoint point=[self convertPoint:[lastMouse locationInWindow] fromView:nil];
    int     row,column;
    NSCell *cell;

    [[self superview] autoscroll:lastMouse];
    [self lockFocus];
    if((cell=[self _enabledCellAtPoint:point row:&row column:&column])!=nil){
     NSRect  cellFrame=[self cellFrameAtRow:row column:column];
     int     currentState=[cell state];
     int     nextState=[cell nextState];

     result=YES;
     [cell highlight:YES withFrame:cellFrame inView:self];
 
     [cell setState:nextState];

     if([cell trackMouse:lastMouse inRect:cellFrame ofView:self untilMouseUp:NO]){
      _selectedIndex=[_cells indexOfObjectIdenticalTo:cell];
      _keyCellIndex=_selectedIndex;
      [cell highlight:NO withFrame:cellFrame inView:self];
      [self unlockFocus];
      break;
     }
     else {
      [cell setState:currentState];
      [cell highlight:NO withFrame:cellFrame inView:self];
     }
    }
    [self unlockFocus];
    [[self window] flushWindow];
    event=[[self window] nextEventMatchingMask:NSLeftMouseUpMask|
                          NSLeftMouseDraggedMask|NSPeriodicMask];
    if([event type]!=NSPeriodic)
     lastMouse=event;
   }while([event type]!=NSLeftMouseUp);

   return result;
}

-(BOOL)_mouseDownList:(NSEvent *)event {
   BOOL     result=YES;
   NSEvent *lastMouse=event;
   int  firstRow=-1,firstColumn=-1;
   int  previousRow=-1,previousColumn=-1;
   BOOL firstHighlight=YES;

   if([event modifierFlags]&NSShiftKeyMask){
    firstRow=previousRow=[self selectedRow];
    firstColumn=previousColumn=[self selectedColumn];
    [self lockFocus];
    [self _deselectAllCellsInLockFocus];
    [self unlockFocus];
   }
   else if(!([event modifierFlags]&NSControlKeyMask))
    [self deselectAllCells];

   do {
    NSPoint point=[self convertPoint:[lastMouse locationInWindow] fromView:nil];
    int     row,column,minSelRow,maxSelRow,minSelColumn,maxSelColumn,r,c;

    [[self superview] autoscroll:lastMouse];

    if([self getRow:&row column:&column forPoint:point]){
     NSCell *cell=[self cellAtRow:row column:column];

     if(firstRow==-1){
      previousRow=firstRow=row;
      previousColumn=firstColumn=column;
      if([lastMouse modifierFlags]&NSControlKeyMask)
       firstHighlight=![cell isHighlighted];
    //  [self _selectCell:cell];
     }


     [self lockFocus];

     minSelRow=MIN(firstRow,row);
     maxSelRow=MAX(firstRow,row);
     minSelColumn=MIN(firstColumn,column);
     maxSelColumn=MAX(firstColumn,column);
     for(r=MIN(minSelRow,previousRow);r<=MAX(maxSelRow,previousRow);r++){
      for(c=MIN(minSelColumn,previousColumn);c<=MAX(maxSelColumn,previousColumn);c++){
       cell=[self cellAtRow:r column:c];

       if([cell isEnabled]){
        NSRect cellFrame=[self cellFrameAtRow:r column:c];
        BOOL   inSelection=(r>=minSelRow && r<=maxSelRow && c>=minSelColumn && c<=maxSelColumn);
        BOOL   highlight=inSelection?firstHighlight:!firstHighlight;

        [cell setState:highlight?NSOnState:NSOffState];
        [cell setHighlighted:highlight];
        [self _setSelectedIndexFromCell:cell];

        //[self setNeedsDisplay:YES];
        [cell setControlView:self];
        [cell drawWithFrame:cellFrame inView:self];
       }
      }
     }
     [self unlockFocus];
     [[self window] flushWindow];

     previousRow=row;
     previousColumn=column;
    }

    event=[[self window] nextEventMatchingMask:NSLeftMouseUpMask|NSLeftMouseDraggedMask|NSPeriodicMask];
    if([event type]!=NSPeriodic)
     lastMouse=event;
   }while([event type]!=NSLeftMouseUp);

   return result;
}

-(BOOL)_mouseDownTrack:(NSEvent *)event {
   BOOL    result=NO;
   NSPoint point=[self convertPoint:[event locationInWindow] fromView:nil];
   int     row,column;
   NSCell *cell;

   [self lockFocus];
    if((cell=[self _enabledCellAtPoint:point row:&row column:&column])!=nil){
     NSRect  cellFrame=[self cellFrameAtRow:row column:column];

     if([cell trackMouse:event inRect:cellFrame ofView:self untilMouseUp:YES]){
      [cell setState:[cell nextState]];
      [self selectCell:cell];
     }
     result=YES;
    }
   [self unlockFocus];

   return result;
}

-(void)mouseDown:(NSEvent *)event {
   NSPoint point=[self convertPoint:[event locationInWindow] fromView:nil];
   int     row,column;
   BOOL    sendAction=NO;

   if([self getRow:&row column:&column forPoint:point]){
    NSCell *cell=[self cellAtRow:row column:column];

    if([cell isEditable]){
     if([cell isEnabled])
      [self _editTextCell:cell row:row column:column event:event];
     return;
    }
   }

   [NSEvent startPeriodicEventsAfterDelay:0.1 withPeriod:0.2];
 
   switch([self mode]){
    case NSRadioModeMatrix:     sendAction=[self _mouseDownRadio:event]; break;
    case NSHighlightModeMatrix: sendAction=[self _mouseDownHighlight:event]; break;
    case NSListModeMatrix:      sendAction=[self _mouseDownList:event]; break;
    case NSTrackModeMatrix:     sendAction=[self _mouseDownTrack:event]; break;
   }

   [NSEvent stopPeriodicEvents];

   if(sendAction){
    if([event clickCount]==2)
     [self sendDoubleAction];
    else
     [self sendAction];
   }

   [[self window] flushWindow];
}

// n.b. now moves to next/previous view on last/first cell, should not wrap (according to spec)
- (void)insertTab:sender {
    BOOL selectNextKeyView = YES;
    
    if ([self tabKeyTraversesCells] && [self mode] != NSRadioModeMatrix){
        _keyCellIndex++;
        if(_keyCellIndex >= [_cells count])
            _keyCellIndex = [_cells count]-1;
        else
            selectNextKeyView = NO;
        
        [self setNeedsDisplay:YES];
    }

    if (selectNextKeyView)
        [[self window] selectNextKeyView:sender];
}

- (void)insertBacktab:sender {
    BOOL selectPreviousKeyView = YES;
    
    if ([self tabKeyTraversesCells] && [self mode] != NSRadioModeMatrix){
        _keyCellIndex--;
        if(_keyCellIndex < 0)
            _keyCellIndex = 0;
        else
            selectPreviousKeyView = NO;
        
        [self setNeedsDisplay:YES];
    }

    if (selectPreviousKeyView)
        [[self window] selectPreviousKeyView:sender];
}

- (void)performClick:sender {
    if([self mode]==NSHighlightModeMatrix){
        NSCell *cell=[self keyCell];
        int     nextState=[cell nextState];

        [self selectCell:cell];
        [cell setState:nextState];
        [self drawCell:cell];
        [self sendAction];
        return;
    }
}

- (void)keyDown:(NSEvent *)event {
    [self interpretKeyEvents:[NSArray arrayWithObject:event]];
}

// bounds checking is done at selectCellAtRow:column:
// revised to pass keyboard events down to cells if possible
// hmm. if we're in radio mode, movement == selection
// otherwise, movement = keyboard focus
-(void)moveUp:sender {
    NSCell *nextCell = nil;
    int row = -1, column = -1;

    if ([self mode] == NSRadioModeMatrix) {
        row = [self selectedRow];
        column = [self selectedColumn];
    }
    else
        [self getRow:&row column:&column ofCell:[self keyCell]];

    nextCell = [self cellAtRow:row-1 column:column];

    if (nextCell) {
        if ([self mode] == NSRadioModeMatrix)
            [self selectCell:nextCell];
        
        [self setKeyCell:nextCell];
    }
    else if ([[self keyCell] respondsToSelector:@selector(moveUp:)])
        [[self keyCell] moveUp:sender];

    [self setNeedsDisplay:YES];
}

-(void)moveDown:sender {
    NSCell *nextCell = nil;
    int row = -1, column = -1;

    if ([self mode] == NSRadioModeMatrix) {
        row = [self selectedRow];
        column = [self selectedColumn];
    }
    else
        [self getRow:&row column:&column ofCell:[self keyCell]];

    nextCell = [self cellAtRow:row+1 column:column];

    if (nextCell) {
        if ([self mode] == NSRadioModeMatrix)
            [self selectCell:nextCell];
        
        [self setKeyCell:nextCell];
    }
    else if ([[self keyCell] respondsToSelector:@selector(moveDown:)])
        [[self keyCell] moveDown:sender];

    [self setNeedsDisplay:YES];
}

-(void)moveLeft:sender {
    NSCell *nextCell = nil;
    int row = -1, column = -1;

    if ([self mode] == NSRadioModeMatrix) {
        row = [self selectedRow];
        column = [self selectedColumn];
    }
    else
        [self getRow:&row column:&column ofCell:[self keyCell]];

    nextCell = [self cellAtRow:row column:column-1];

    if (nextCell) {
        if ([self mode] == NSRadioModeMatrix)
            [self selectCell:nextCell];
        
        [self setKeyCell:nextCell];
    }
    else if ([[self keyCell] respondsToSelector:@selector(moveLeft:)])
        [[self keyCell] moveLeft:sender];

    [self setNeedsDisplay:YES];
}

-(void)moveRight:sender {
    NSCell *nextCell = nil;
    int row = -1, column = -1;

    if ([self mode] == NSRadioModeMatrix) {
        row = [self selectedRow];
        column = [self selectedColumn];
    }
    else
        [self getRow:&row column:&column ofCell:[self keyCell]];

    nextCell = [self cellAtRow:row column:column+1];

    if (nextCell) {
        if ([self mode] == NSRadioModeMatrix)
            [self selectCell:nextCell];
        
        [self setKeyCell:nextCell];
    }
    else if ([[self keyCell] respondsToSelector:@selector(moveRight:)])
        [[self keyCell] moveRight:sender];

    [self setNeedsDisplay:YES];
}

-(BOOL)becomeFirstResponder {
   [self _selectTextCell:[self keyCell]];

   return YES;
}

-(void)textDidEndEditing:(NSNotification *)note {
   int movement=[[[note userInfo] objectForKey:@"NSTextMovement"] intValue];

   [super textDidEndEditing:note];

   if(movement==NSReturnTextMovement || [[self selectedCell] sendsActionOnEndEditing])
    [self sendAction];

   if(movement==NSTabTextMovement){

    _keyCellIndex++;
    if(_keyCellIndex>=[_cells count]){
     NSView *next=[self nextValidKeyView];

     _keyCellIndex=0;

     if(next!=nil){
      [[self window] makeFirstResponder:next];
      return;
     }
    }
    if(_keyCellIndex>=[_cells count])
     _keyCellIndex=-1;

     [self _selectTextCell:[self keyCell]];
   }
}

@end


@implementation NSMatrix (Bindings)

- (int)_selectedIndex
{
	return _selectedIndex;
}

- (void)_setSelectedIndex:(int)index
{
	if (_selectedIndex != index) {
		if (index < [_cells count]) {
			NSCell* cell = [_cells objectAtIndex: index];
			[self selectCell: cell];
		}
	}
}

- (int) _selectedTag {
    return [[self selectedCell] tag];
}
- (void) _setSelectedTag:(int)selectedTag {
    [self selectCellWithTag:selectedTag];
}

@end
