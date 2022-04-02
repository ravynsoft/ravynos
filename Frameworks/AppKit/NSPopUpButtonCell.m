/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSPopUpButtonCell.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSGraphicsStyle.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSPopUpWindow.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSRaise.h>

@implementation NSPopUpButtonCell

-init {
   self = [super init];
   _pullsDown = NO;
   _menu = [[[NSMenu alloc] init] retain];
   _selectedIndex=-1;
   _arrowPosition = NSPopUpArrowAtCenter;
   _preferredEdge = NSMaxYEdge;
   return self;
}

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];
   if([coder allowsKeyedCoding]){
    _pullsDown=[coder decodeBoolForKey:@"NSPullDown"];
    _menu=[[coder decodeObjectForKey:@"NSMenu"] retain];
    _selectedIndex=[coder decodeIntForKey:@"NSSelectedIndex"];
    _arrowPosition = [coder decodeIntForKey: @"NSArrowPosition"];
    _preferredEdge = [coder decodeIntForKey: @"NSPreferredEdge"];
    _usesItemFromMenu=[coder decodeBoolForKey:@"NSUsesItemFromMenu"];

    [self synchronizeTitleAndSelectedItem];
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"%@ can not initWithCoder:%@",isa,[coder class]];
   }
   return self;
}

-copyWithZone:(NSZone *)zone {
   NSPopUpButtonCell *copy = [super copyWithZone:zone];

   copy->_menu = [_menu copy];

   return copy;
}

-initTextCell:(NSString *)string pullsDown:(BOOL)pullDown {
   [super initTextCell:string];
   _menu = [[NSMenu alloc] initWithTitle:string];
   [_menu addItemWithTitle:string action:[self action] keyEquivalent:@""];
   _arrowPosition = NSPopUpArrowAtCenter;
   _preferredEdge = NSMaxYEdge;
   _pullsDown=pullDown;
   return self;
}

-(void)dealloc {
   [_menu release];
   [super dealloc];
}

-(BOOL)isOpaque {
   return NO;
}

-(BOOL)pullsDown {
   return _pullsDown;
}

-(NSMenu *)menu {
   return _menu;
}

-(BOOL)autoenablesItems {
   return _autoenablesItems;
}

-(NSPopUpArrowPosition)arrowPosition {
    return _arrowPosition;
}

-(NSRectEdge)preferredEdge {
   return _preferredEdge;
}

-(NSArray *)itemArray {
   return [_menu itemArray];
}

-(NSInteger)numberOfItems {
   return [_menu numberOfItems];
}

-(NSMenuItem *)itemAtIndex:(NSInteger)index {
   return [_menu itemAtIndex:index];
}

-(NSMenuItem *)itemWithTitle:(NSString *)title {
   return [_menu itemWithTitle:title];
}

-(NSMenuItem *)lastItem {
   if([_menu numberOfItems]==0)
    return nil;
    
   return [_menu itemAtIndex:[_menu numberOfItems]-1];
}

-(NSInteger)indexOfItem:(NSMenuItem *)item {
   return [_menu indexOfItem:item];
}

-(NSInteger)indexOfItemWithTitle:(NSString *)title {
   return [_menu indexOfItemWithTitle:title];
}

-(NSInteger)indexOfItemWithTag:(NSInteger)tag {
   return [_menu indexOfItemWithTag:tag];
}

-(NSInteger)indexOfItemWithRepresentedObject:object {
   return [_menu indexOfItemWithRepresentedObject:object];
}

-(NSInteger)indexOfItemWithTarget:target andAction:(SEL)action {
   return [_menu indexOfItemWithTarget:target andAction:action];
}

-(NSMenuItem *)selectedItem {
  if(_selectedIndex<0)
   return nil;
   
	if (_selectedIndex >= [_menu numberOfItems]) {
		return nil;
	}
	
  return [_menu itemAtIndex:_selectedIndex];
}

-(NSString *)titleOfSelectedItem {
   return [[self selectedItem] title];
}

-(NSInteger)indexOfSelectedItem {
   return _selectedIndex;
}

-(void)setPullsDown:(BOOL)flag {
    _pullsDown = flag;
}

-(void)setMenu:(NSMenu *)menu {
   menu=[menu retain];
   [_menu release];
   _menu = menu;
   
   if([_menu numberOfItems]>0)
    _selectedIndex=0;
   else
    _selectedIndex=-1;

   [self synchronizeTitleAndSelectedItem];
}

-(void)setAutoenablesItems:(BOOL)value {
   _autoenablesItems=value?YES:NO;
}

-(void)setArrowPosition:(NSPopUpArrowPosition)position {
    _arrowPosition = position;
}

-(void)setPreferredEdge:(NSRectEdge)edge {
   edge=_preferredEdge;
}

-(void)_addItemWithTitle:(NSString *)title {
    
    NSMenuItem *duplicate=[_menu itemWithTitle: title];
    if (duplicate != nil) {
        // don't allow items with duplicate titles by default
        [_menu removeItem: duplicate];
    }
    
   [_menu addItemWithTitle:title action:@selector(_popUpItemAction:) keyEquivalent:nil];
    NSMenuItem *item=[[_menu itemArray] lastObject];
    [item setTarget: self];
    
   if(_selectedIndex<0)
    _selectedIndex=0;
}

-(void)addItemWithTitle:(NSString *)title {
   [self _addItemWithTitle:title];
   [self synchronizeTitleAndSelectedItem];
}

-(void)addItemsWithTitles:(NSArray *)titles {
   NSInteger i,count=[titles count];

   for(i=0;i<count;i++)
    [self _addItemWithTitle:[titles objectAtIndex:i]];
    
   [self synchronizeTitleAndSelectedItem];
}

-(void)removeAllItems {
   [_menu removeAllItems];
	_selectedIndex = -1;
	[self synchronizeTitleAndSelectedItem];
}

-(void)removeItemAtIndex:(NSInteger)index {
   [_menu removeItemAtIndex:index];
	
	if (_selectedIndex >= [_menu numberOfItems]) {
		// Don't know what to select anymore...
		_selectedIndex = -1;
	}
	[self synchronizeTitleAndSelectedItem];
}

-(void)removeItemWithTitle:(NSString *)title {
   NSInteger index=[self indexOfItemWithTitle:title];
   [self removeItemAtIndex:index];
}

-(void)insertItemWithTitle:(NSString *)title atIndex:(NSInteger)index {
   [_menu insertItemWithTitle:title action:@selector(_popUpItemAction:) keyEquivalent:nil atIndex:index];
    NSMenuItem *check=[_menu itemAtIndex:index];
    [check setTarget: self];

   [self synchronizeTitleAndSelectedItem];
}

-(void)selectItem:(NSMenuItem *)item {

    NSInteger selectedIndex = _selectedIndex;
   if(item==nil)
    selectedIndex=-1;
   else {
    NSInteger check=[[_menu itemArray] indexOfObjectIdenticalTo:item];
    
    selectedIndex=(check==NSNotFound)?-1:check;
   }

    if (selectedIndex != _selectedIndex) {
        [self willChangeValueForKey:@"selectedItem"];
        _selectedIndex = selectedIndex;
        [self didChangeValueForKey:@"selectedItem"];
    }
   [self synchronizeTitleAndSelectedItem];
}

-(void)selectItemAtIndex:(NSInteger)index {
   NSMenuItem *item=(index<0)?nil:[self itemAtIndex:index];

   [self selectItem:item];
}

-(void)selectItemWithTitle:(NSString *)title {
   [self selectItemAtIndex:[self indexOfItemWithTitle:title]];
}

-(BOOL)selectItemWithTag:(NSInteger)tag {   
   NSInteger index=[self indexOfItemWithTag:tag];
   
   if(index<0)
    return NO;

   [self selectItemAtIndex:index];
   return YES;
}

-(NSString *)itemTitleAtIndex:(NSInteger)index {
   return [[self itemAtIndex:index] title];
}

-(NSArray *)itemTitles {
   NSMutableArray *result=[NSMutableArray array];
   NSArray *array=[self itemArray];
   NSInteger i,count=[array count];
   
   for(i=0;i<count;i++)
    [result addObject:[[array objectAtIndex:i] title]];
   
   return result;
}

-(void)synchronizeTitleAndSelectedItem {
   NSArray    *itemArray=[_menu itemArray];
   NSMenuItem *item=nil;
   
   if(_selectedIndex<0 || _pullsDown){
    if([itemArray count]>0)
     item=[itemArray objectAtIndex:0];
   }
   else {
    item=[itemArray objectAtIndex:_selectedIndex];
   }
  [super setTitle:[item title]];
    // For a redraw of the control
  [(NSControl *)[self controlView] updateCell:self];
}


-(NSImage *)arrowImage {
   if(_pullsDown)
    return [NSImage imageNamed:@"NSPopUpButtonCellPullDown"];
   else
    return [NSImage imageNamed:@"NSPopUpButtonCellPopUp"];
}

-(NSRect)_arrowRectForRect:(NSRect)frame
{
	NSRect result = NSZeroRect;
	if( _arrowPosition != NSPopUpNoArrow ) {
		NSImage * arrowImage = ( _arrowPosition != NSPopUpNoArrow ) ? [self arrowImage] : NULL;
		
		if (arrowImage != nil) {
			
			// Scale down the arrows so they look proportional to the control size
			float sizeFactor = 0;
			
			switch ([self controlSize]) {
				case NSRegularControlSize:
					sizeFactor = 0;
					break;
				case NSSmallControlSize:
					sizeFactor = 1;
					break;
				case NSMiniControlSize:
					sizeFactor = 2;
					break;
			}
			NSRect otherFrame = frame;
			NSSize arrowSize = [arrowImage size];
			otherFrame.origin.x += otherFrame.size.width - ( arrowSize.width + (4 - sizeFactor) );
			otherFrame.origin.y += ( otherFrame.size.height - arrowSize.height ) / 2;
			otherFrame.size =  arrowSize;
			
			result = NSInsetRect(otherFrame, sizeFactor, sizeFactor);
		}
	}
	return result;
}

-(NSRect)titleRectForBounds:(NSRect)rect 
{
	rect = [super titleRectForBounds:rect];
	if (_arrowPosition != NSPopUpNoArrow) {
		// Don't use the room used by the arrow, with some margin
		NSRect arrowRect = [self _arrowRectForRect:rect];
		rect.size.width = NSMinX(arrowRect) - NSMinX(rect) - 5.;
	}
	return rect;
}

-(void)drawBezelWithFrame:(NSRect)frame inView:(NSView *)controlView {

	[super drawBezelWithFrame: frame inView: controlView];

	// Now draw the arrow
    if( _arrowPosition != NSPopUpNoArrow )
	{
		NSImage * arrowImage = ( _arrowPosition != NSPopUpNoArrow ) ? [self arrowImage] : NULL;
		
		if (arrowImage == NULL) return;
				
		NSRect arrowFrame = [self _arrowRectForRect:frame];
		[[controlView graphicsStyle] drawButtonImage:arrowImage inRect:arrowFrame enabled:YES mixed:YES];
	}
}

-(NSSize)cellSize  {
   NSSize result=[super cellSize];
   
   switch([self controlSize]){
   
    case NSRegularControlSize:
     result.height=22;
     break;
			
    case NSSmallControlSize:
     result.height=19;
     break;
			
    case NSMiniControlSize:
     result.height=15;
     break;
   }

   return result;
}

-(void)setTitle:(NSString *)title {
   
   if(_pullsDown){
    // Doc.s for pulls down behavior are not correct, it just sets the title to the argument and doesn't affect the selection
    [super setTitle:title];
   }
   else {
    NSMenuItem *item=[_menu itemWithTitle:title];
   
    if(item==nil)
        [self addItemWithTitle:title];

    [self selectItemWithTitle:title];
   }
}

// From Cocoa doc :
// This method has no effect.
// The image displayed in a pop up button is taken from the selected menu item (in the case of a pop up menu) or
// from the first menu item (in the case of a pull-down menu).
-(void)setImage:(NSImage *)image
{
    
}

-(NSImage *)image
{
    NSArray    *itemArray=[_menu itemArray];
    NSMenuItem *item=nil;
    if(_selectedIndex<0 || _pullsDown){
        if([itemArray count]>0)
            item=[itemArray objectAtIndex:0];
    }
    else {
        item=[itemArray objectAtIndex:_selectedIndex];
    }
    return [item image];
}


-(NSCellImagePosition)imagePosition
{
    // It seems Cocoa popup buttons never ignore the image - they are drawn at Left position if set to None
    NSCellImagePosition imagePosition = [super imagePosition];
    return imagePosition==NSNoImage?NSImageLeft:imagePosition;
}

-(NSInteger)tag {
   return [[self selectedItem] tag];
}

-(BOOL)trackMouse:(NSEvent *)event inRect:(NSRect)cellFrame ofView:(NSView *)controlView untilMouseUp:(BOOL)flag {
   NSRect bounds = [controlView bounds];
   NSPoint        origin=bounds.origin;
   
#if 0
   // Note: the min options don't mean much unless we don't have room for the menu, so either way we just pop
   // up over the button itself. However, maxX and maxY *do* have special meanings
   switch( _preferredEdge )
   {
      case NSMinXEdge:
      case NSMinYEdge:
	     break;
	  case NSMaxXEdge:
		 origin.x += [controlView bounds].size.width;
	     break;
	  case NSMaxYEdge: 
         // Remember, our Y axis is flipped in Cocoa. Also, not sure why we need the -4 offset here, 
		 // can't figure out where the offset comes from, but it works			
		 origin.y -= [controlView bounds].size.height - 4;
	     break;
   }
#endif
    origin=[controlView convertPoint:origin toView:nil];
//    origin=[[controlView window] convertBaseToScreen:origin];

    [[_menu delegate] menuNeedsUpdate: _menu];
//	[[_menu delegate] menuWillOpen: _menu];
    NSMenu *menu = _menu;
    if (_pullsDown && [_menu numberOfItems]) {
        // Don't display the first item for pullDowns controls
        menu = [[_menu copy] autorelease];
        [menu removeItemAtIndex:0];
    }
    [menu update];

    NSPopUpWindow *window=[[NSPopUpWindow alloc] initWithFrame:NSMakeRect(origin.x,origin.y,
        cellFrame.size.width,cellFrame.size.height)];
    [window setPullsDown:_pullsDown];
    [window setMenu:menu];
   if([self font]!=nil)
    [window setFont:[self font]];

   if(_pullsDown)
    [window selectItemAtIndex:-1]; // No initial selection for pullsDown
   else
    [window selectItemAtIndex:_selectedIndex];

    [window setParent:[controlView window]];
    [window orderFront:nil];
    NSInteger itemIndex=[window runTrackingWithEvent:event];
    if(itemIndex!=NSNotFound) {
        if (_pullsDown) {
            // remember that thing we did with the first menu item?
            itemIndex++;
        }
        // We can be embedded in controls other than a PopUpButton - so don't
        // assume selectItemAtIndex: is available
        if ([controlView respondsToSelector: @selector(selectItemAtIndex:)]) {
            [(id)controlView selectItemAtIndex:itemIndex];
        }
    }
    _selectedIndex = (itemIndex == NSNotFound) ? -1 : itemIndex;
    [window close]; // release when closed=YES
//	[[_menu delegate] menuDidClose: _menu];

   return itemIndex!=NSNotFound;
}

-(void)moveUp:sender {
   NSInteger index = [self indexOfSelectedItem];
    
   if (index > 0)
    [self selectItemAtIndex:index-1];
}

-(void)moveDown:sender {
   NSInteger index = [self indexOfSelectedItem];
    
   if (index < [self numberOfItems]-1)
    [self selectItemAtIndex:index+1];
}


/* That's the action the menu items are connected to in nibs */
- (void)_popUpItemAction:(id)sender
{
    NSUInteger itemIndex = [_menu indexOfItem: sender];
    if (itemIndex != NSNotFound) {
        [self selectItemAtIndex: itemIndex];
    }
    [NSApp sendAction: [self action] to: [self target] from: _controlView];
}
@end
