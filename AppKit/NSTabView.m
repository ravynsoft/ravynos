/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSTabView.h>
#import <AppKit/NSTabViewItem.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSMatrix.h>
#import <AppKit/NSPopUpButton.h>	// for indexOfSelectedItem definition
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSGraphicsStyle.h>
#import <AppKit/NSRaise.h>

@interface NSTabViewItem(NSTabViewItem_private)
-(void)setTabView:(NSTabView *)tabView;
-(void)setTabState:(NSTabState)tabState;
@end

@implementation NSTabView

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];

   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    unsigned flags=[keyed decodeIntForKey:@"NSTvFlags"];

    _items=[[NSMutableArray alloc] initWithArray:[keyed decodeObjectForKey:@"NSTabViewItems"]];
    _selectedItem=[keyed decodeObjectForKey:@"NSSelectedTabViewItem"];
    _allowsTruncatedLabels=[keyed decodeBoolForKey:@"NSAllowTruncatedLabels"];
    _drawsBackground=[keyed decodeBoolForKey:@"NSDrawsBackground"];
    _controlSize=(flags&0x18000000)>>27;
    if (_font==nil)
     _font=[[NSFont boldSystemFontOfSize:13-_controlSize*2] retain];
    _type=(flags&0x7);

   switch (_type) {
	   case NSTopTabsBezelBorder:
	   case NSLeftTabsBezelBorder:
	   case NSBottomTabsBezelBorder:
	   case NSRightTabsBezelBorder: {
    // adjust the layout rectangle
    NSRect frame=[self frame];
    frame.origin.x += 8;
    frame.size.width -= 15;
    switch(_controlSize)
    {
     case NSRegularControlSize:
      frame.origin.y += 12;
      frame.size.height -= 16;
      break;
     case NSSmallControlSize:
      frame.origin.y += 9;
      frame.size.height -= 13;
      break;
     case NSMiniControlSize:
      frame.origin.y += 8;
      frame.size.height -= 12;
      break;
    }
    [self setFrame:frame];
   }
		   
	   case NSNoTabsBezelBorder:
	   case NSNoTabsLineBorder:
	   case NSNoTabsNoBorder:
	   default:
		   break;
   }
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   }

   return self;
}

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithFrame:(NSRect)frame {
   [super initWithFrame:frame];
   _items=[NSMutableArray new];
   _selectedItem=nil;
// should probably be boldUserFont
   _font=[[NSFont boldSystemFontOfSize:0] retain];
   _type=NSTopTabsBezelBorder;
   _allowsTruncatedLabels=NO;
   return self;
}

-(void)dealloc {
   [_items release];
   _selectedItem=nil;
   [_font release];
   [super dealloc];
}

-delegate {
   return _delegate;
}

-(NSSize)minimumSize {
    switch (_type) {
        case NSTopTabsBezelBorder:
        case NSLeftTabsBezelBorder:
        case NSBottomTabsBezelBorder:
        case NSRightTabsBezelBorder: {
            NSSize finalSize;
            int i, count=[_items count];

            if (count == 0)
                break;
            
            finalSize = [[_items objectAtIndex:0] sizeOfLabel:_allowsTruncatedLabels];
            for (i = 1; i < count; ++i)
                finalSize.width += [[_items objectAtIndex:i] sizeOfLabel:_allowsTruncatedLabels].width;

            return finalSize;
        }

        case NSNoTabsBezelBorder:
        case NSNoTabsLineBorder:
        case NSNoTabsNoBorder:
        default:
            break;
    }

    return NSMakeSize(0,0);	// correct?
}

-(NSRect)rectForBorder {
    NSRect result=[self bounds];

    switch (_type) {
        case NSTopTabsBezelBorder:
        case NSLeftTabsBezelBorder:
        case NSBottomTabsBezelBorder:
        case NSRightTabsBezelBorder:
            result.size.height-=24;
            break;

        case NSNoTabsBezelBorder:
        case NSNoTabsLineBorder:
        case NSNoTabsNoBorder:
        default:
            break;
    }

    return result;
}

-(NSRect)contentRect {
   NSRect result=[self rectForBorder];

    switch (_type) {
        case NSTopTabsBezelBorder:
        case NSLeftTabsBezelBorder:
        case NSBottomTabsBezelBorder:
        case NSRightTabsBezelBorder:
        case NSNoTabsBezelBorder:
            return NSInsetRect(result,2,2);

        case NSNoTabsLineBorder:
            return NSInsetRect(result,1,1);

        case NSNoTabsNoBorder:
        default:
            return result;
    }
}

-(NSFont *)font {
   return _font;
}

-(NSTabViewType)tabViewType {
    return _type;
}

-(BOOL)drawsBackground {
    return _drawsBackground;
}

-(BOOL)allowsTruncatedLabels {
    return _allowsTruncatedLabels;
}

-(void)setDelegate:delegate {
   _delegate=delegate;
}

-(void)setFont:(NSFont *)font {
   font=[font retain];
   [_font release];
   _font=font;
}

-(void)setTabViewType:(NSTabViewType)type {
    _type = type;
    [self setNeedsDisplay:YES];
}

-(void)setDrawsBackground:(BOOL)flag {
    _drawsBackground = flag;
    [self setNeedsDisplay:YES];
}

-(void)setAllowsTruncatedLabels:(BOOL)flag {
    _allowsTruncatedLabels = flag;
    [self setNeedsDisplay:YES];
}

-(int)numberOfTabViewItems {
    return [_items count];
}

-(NSArray *)tabViewItems {
    return _items;
}
	
-(NSTabViewItem *)tabViewItemAtIndex:(int)index {
    return [_items objectAtIndex:index];
}

-(NSRect)rectForItemLabelAtIndex:(unsigned)index {
   int            i,count=[_items count];
    NSPoint        base=NSMakePoint(0,[self bounds].size.height-22);
   NSSize         size;

    base.x+=6;
    for(i=0;i<count && i<index;i++){
     size=[[_items objectAtIndex:i] sizeOfLabel:NO];

     base.x+=size.width;
     base.x+=8;
    }

    size=[[_items objectAtIndex:index] sizeOfLabel:NO];

    return NSMakeRect(base.x,base.y,size.width,size.height);
}

-(NSTabViewItem *)tabViewItemAtPoint:(NSPoint)point {
   int i,count=[_items count];

   for(i=0;i<count;i++){
    NSTabViewItem *item=[_items objectAtIndex:i];
    NSRect         rect=[self rectForItemLabelAtIndex:i];

    if(NSMouseInRect(point,rect,[self isFlipped]))
     return item;
   }

   return nil;
}

-(int)indexOfTabViewItem:(NSTabViewItem *)item {
    return [_items indexOfObject:item];
}

-(int)indexOfTabViewItemWithIdentifier:identifier {
    int i, count=[_items count];

    for (i = 0; i < count; ++i)
        if ([[(NSTabViewItem *)[_items objectAtIndex:i] identifier] isEqual:identifier])
            return i;

    return NSNotFound;
}

-(void)addTabViewItem:(NSTabViewItem *)item {
   [_items addObject:item];
   if(_selectedItem==nil)
    [self selectTabViewItem:item];
   [item setTabView:self];

   if ([_delegate respondsToSelector:@selector(tabViewDidChangeNumberOfTabViewItems:)])
       [_delegate tabViewDidChangeNumberOfTabViewItems:self];

   [self setNeedsDisplay:YES];
}

-(void)removeTabViewItem:(NSTabViewItem *)item {
	int selectedIndex = [self indexOfTabViewItem:_selectedItem];
	if (item == _selectedItem) {
        int newIndex = selectedIndex - 1;
		
        if (newIndex < 0) {
            newIndex = selectedIndex + 1;
			
            if (newIndex >= [self numberOfTabViewItems]) {
                newIndex = NSNotFound;
            }
        }
		
        if (newIndex == NSNotFound) {
            [[_selectedItem view] removeFromSuperview];
            _selectedItem = nil;
        } else {
            [self selectTabViewItemAtIndex:newIndex];
        }
    }
	
	[_items removeObject:item];

    if ([_delegate respondsToSelector:@selector(tabViewDidChangeNumberOfTabViewItems:)])
        [_delegate tabViewDidChangeNumberOfTabViewItems:self];

    [self setNeedsDisplay:YES];
}

-(void)insertTabViewItem:(NSTabViewItem *)item atIndex:(int)index {
    [_items insertObject:item atIndex:index];
    if (_selectedItem==nil)
        [self selectTabViewItem:item];
    [item setTabView:self];

    if ([_delegate respondsToSelector:@selector(tabViewDidChangeNumberOfTabViewItems:)])
        [_delegate tabViewDidChangeNumberOfTabViewItems:self];

    [self setNeedsDisplay:YES];
}

-(NSTabViewItem *)selectedTabViewItem {
   return _selectedItem;
}

-(void)setFrame:(NSRect)frame {
/* A tab view will autoresize the selected view regardless of whether autoresizesSubviews is enabled
   We do it here because resizeSubviewsWithOldSize: won't be called if autoresizesSubviews is off. */

   [super setFrame:frame];
   if(_selectedItem!=nil)
    [[_selectedItem view] setFrame:[self contentRect]];
}

- (void)_itemViewDidChange:(NSTabViewItem *)item
{
    if(item == _selectedItem){
        NSView  *itemView = [item view];
        
        if(itemView != nil){
            [self addSubview:itemView];
            [itemView setFrame:[self contentRect]];
        }
     [self setNeedsDisplay:YES];
    }
}

// delegate methods go here
-(void)selectTabViewItem:(NSTabViewItem *)item {
    if(item!=_selectedItem) {
        BOOL selectItem=YES;

        if ([_delegate respondsToSelector:@selector(tabView:shouldSelectTabViewItem:)])
            selectItem=[_delegate tabView:self shouldSelectTabViewItem:item];        

        if (selectItem) {
            if ([_delegate respondsToSelector:@selector(tabView:willSelectTabViewItem:)])
                [_delegate tabView:self willSelectTabViewItem:item];
            
            [[_selectedItem view] removeFromSuperview];
            if([item view]!=nil){
             [self addSubview:[item view]];
             [[item view] setFrame:[self contentRect]];
            }
           [self setNeedsDisplay:YES];
            _selectedItem=item;

            if ([item initialFirstResponder])
                [[self window] makeFirstResponder:[item initialFirstResponder]];

            if ([_delegate respondsToSelector:@selector(tabView:didSelectTabViewItem:)])
                [_delegate tabView:self didSelectTabViewItem:item];
        }
// Since we're not opaque, we need to redraw the superview too
// Shouldn't the view machinery do this automatically?? it might now
       [[self superview] setNeedsDisplay:YES];
    }
}

-(void)selectTabViewItemAtIndex:(int)index {
    [self selectTabViewItem:[_items objectAtIndex:index]];
}

-(void)selectTabViewItemWithIdentifier:identifier {
    [self selectTabViewItemAtIndex:[self indexOfTabViewItemWithIdentifier:identifier]];
}

-(void)selectFirstTabViewItem:sender {
   [self selectTabViewItem:([_items count]==0)?nil:[_items objectAtIndex:0]];
}

-(void)selectLastTabViewItem:sender {
   [self selectTabViewItem:[_items lastObject]];
}

-(void)takeSelectedTabViewItemFromSender:sender {
    if ([sender respondsToSelector:@selector(indexOfSelectedItem)])
        [self selectTabViewItemAtIndex:[sender indexOfSelectedItem]];
    else if ([sender isKindOfClass:[NSMatrix class]])
        [self selectTabViewItemAtIndex:[[sender cells] indexOfObject:[sender selectedCell]]];
}

-(NSRect)rectForItemBorderAtIndex:(unsigned)index {
   NSRect result=[self rectForItemLabelAtIndex:index];

   result=NSInsetRect(result,-4,0);
   result.origin.y=[self bounds].size.height-24-_controlSize;
   result.size.height=24-_controlSize;

   return result;
}

-(void)drawRect:(NSRect)rect {
    NSGraphicsStyle *style=[self graphicsStyle];

    switch (_type) {
        case NSTopTabsBezelBorder:
        case NSLeftTabsBezelBorder:
        case NSBottomTabsBezelBorder:
        case NSRightTabsBezelBorder: {
            int    i,count=[_items count];

            for(i=0;i<count;i++){
                NSTabViewItem *item=[_items objectAtIndex:i];

                // defer selected item for overlap
                if (item != _selectedItem) {
                    [style drawTabInRect:[self rectForItemBorderAtIndex:i] clipRect:rect color:[item color] selected:NO];
                    [item drawLabel:_allowsTruncatedLabels inRect:[self rectForItemLabelAtIndex:i]];
                }
            }

            [style drawTabPaneInRect:[self rectForBorder]];

            if((i=[_items indexOfObjectIdenticalTo:_selectedItem])!=NSNotFound) {
               // now do selected item
               i = [_items indexOfObject:_selectedItem];
               [style drawTabInRect:[self rectForItemBorderAtIndex:i] clipRect:rect color:[_selectedItem color] selected:YES];
               [[_selectedItem view] setNeedsDisplay:YES];

               NSRect labelRect=[self rectForItemLabelAtIndex:i];
               labelRect.origin.y+=2;

               [_selectedItem drawLabel:_allowsTruncatedLabels inRect: labelRect];

               if ([[self window] firstResponder] == self)
                  NSDottedFrameRect(NSInsetRect(labelRect,-1,0));
            }
            break;
        }
            
        case NSNoTabsBezelBorder:
            NSDrawButton([self rectForBorder],rect);
            break;
            
        case NSNoTabsLineBorder:
            [[NSColor blackColor] setStroke];
            NSFrameRect([self rectForBorder]);
            break;
            
        case NSNoTabsNoBorder:
            if (_drawsBackground) {
                [[NSColor controlColor] setFill];
                NSRectFill([self rectForBorder]);
            }
            break;
            
        default:
            break;
    }
}

-(void)mouseDown:(NSEvent *)event {
    NSPoint point=[self convertPoint:[event locationInWindow] fromView:nil];
    NSTabViewItem *item=[self tabViewItemAtPoint:point];

    if(item!=nil) {
        [_selectedItem setTabState:NSBackgroundTab];
        [self selectTabViewItem:item];

        // item is "pressed" until mouse up. this correct? docs unclear
        // IB 4.x "selects" immediately (at least visually)
        [_selectedItem setTabState:NSPressedTab];

// Since we're not opaque, we need to redraw the superview too
// Shouldn't the view machinery do this automatically?? it might now
       [[self superview] setNeedsDisplay:YES];
        do {
            event=[[self window] nextEventMatchingMask:NSLeftMouseUpMask|NSLeftMouseDraggedMask];
        } while([event type]!=NSLeftMouseUp);
        
        [_selectedItem setTabState:NSSelectedTab];
    }
    [[self superview] setNeedsDisplay:YES];
}

@end


@implementation NSTabView (Bindings)

-(int)_selectedIndex {
    return [self indexOfTabViewItem:_selectedItem];
}
-(void)_setSelectedIndex:(int)selectedIndex {
    [self selectTabViewItemAtIndex:selectedIndex];
}

@end