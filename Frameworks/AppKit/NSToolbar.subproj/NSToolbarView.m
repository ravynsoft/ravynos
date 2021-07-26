/* Copyright (c) 2006-2009 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/AppKitExport.h>
#import <AppKit/NSToolbarView.h>
#import <AppKit/NSToolbar.h>
#import <AppKit/NSToolbarItem.h>
#import <AppKit/NSToolbarCustomizationView.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSParagraphStyle.h>
#import <AppKit/NSText.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSStringDrawing.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSMenuWindow.h>
#import <AppKit/NSMenuView.h>

@interface NSToolbar (Private)
-(NSArray *)_itemsWithIdentifiers:(NSArray*)identifiers;
-(NSArray *)_allowedToolbarItems;
-(NSArray *)_defaultToolbarItems;
-(NSToolbarItem *)_itemForItemIdentifier:(NSString*)identifier willBeInsertedIntoToolbar:(BOOL)toolbar;
-(void)_setItemsWithIdentifiersFromArray:(NSArray *)identifiers;
-(NSArray *)itemIdentifiers;
@end

@interface NSToolbarItem(private)
-(NSSize)constrainedSize;
-(void)_setItemViewFrame:(NSRect)rect;
-(CGFloat)_expandWidth:(CGFloat)try;
-(NSView *)_enclosingView;
-(void)drawInRect:(NSRect)rect;
@end

@implementation NSToolbarView

-initWithFrame:(NSRect)frame {
   [super initWithFrame:frame];
   [self setAutoresizesSubviews:YES];
   [self setAutoresizingMask:NSViewWidthSizable|NSViewMinYMargin];
   _toolbar=nil;
   _minXMargin=6.0;
   _minYMargin=2.0;
   _visibleItems = [[NSMutableArray alloc] init];
   _overflow=NO;
               
   [self registerForDraggedTypes:[NSArray arrayWithObject:NSToolbarItemIdentifierPboardType]];
    
   return self;
}

-(void)dealloc {
   [[NSNotificationCenter defaultCenter] removeObserver:self];
   _toolbar=nil;
   [_visibleItems release];
   [super dealloc];
}

-(NSToolbar *)toolbar {
   return _toolbar;
}

-(void)setToolbar:(NSToolbar *)toolbar {
   _toolbar = toolbar;
}

-(NSArray *)visibleItems {
   return _visibleItems;
}

-(NSImage *)overflowImage  {
   return [NSImage imageNamed:@"NSToolbarOverflowArrow"];
}

-(NSSize)overflowImageSizeWithMargins {
   NSSize size=[[self overflowImage] size];
   
   size.width+=8;
   size.height+=8;
   
   return size;
}

-(NSRect)overflowRect {
   NSRect bounds=[self bounds];
   NSRect result;
   
   result.size=[self overflowImageSizeWithMargins];

   result.size.height=MAX(result.size.height,bounds.size.height);
   
   result.origin.y=bounds.origin.y;
   result.origin.x=NSMaxX(bounds)-result.size.width;
   return result;
}

-(void)layoutViewsWithWidth:(CGFloat)desiredWidth setFrame:(BOOL)setFrame {
   NSArray  *items=[_toolbar items];
   NSInteger i,count=[items count];
   NSRect    frames[count];
   BOOL      isFlexible[count];
   BOOL      notSpace[count];
   BOOL      visible[count];
   NSSize    overflowSize=[self overflowImageSizeWithMargins];
   CGFloat   leftMargin=5,rightMargin=5;
   CGFloat   consumedWidth=leftMargin+rightMargin,resultHeight=overflowSize.height;
   NSInteger totalNonSpace=0,totalVisibleNonSpace=0;
   NSInteger totalVisible=0;
   
   NSInteger priorities[4]={
    NSToolbarItemVisibilityPriorityUser,
    NSToolbarItemVisibilityPriorityHigh,
    NSToolbarItemVisibilityPriorityStandard,
    NSToolbarItemVisibilityPriorityLow,
   };
   int       p,priorityCount=4;
   
   [_visibleItems removeAllObjects];
   
   for(i=0;i<count;i++){
    NSToolbarItem *item=[items objectAtIndex:i];
    NSString      *identifier=[item itemIdentifier];
    NSSize         size=[item constrainedSize];
    
    frames[i].origin=NSZeroPoint;
    frames[i].size=size;
    resultHeight=MAX(resultHeight,size.height);
    visible[i]=NO;
    isFlexible[i]=NO;
    notSpace[i]=NO;
    
    if([identifier isEqualToString:NSToolbarFlexibleSpaceItemIdentifier])
     isFlexible[i]=YES;
    else if(![identifier isEqualToString:NSToolbarSeparatorItemIdentifier] &&
            ![identifier isEqualToString:NSToolbarSpaceItemIdentifier]){
     notSpace[i]=YES;
     totalNonSpace++;
    }
   }
   
   // consume available space based on priority
   for(p=0;p<priorityCount;p++){
    NSInteger priority=priorities[p];
    
    for(i=0;i<count;i++){
     NSToolbarItem *item=[items objectAtIndex:i];
    
     if([item visibilityPriority]==priority){
      CGFloat availableWidth=desiredWidth-consumedWidth;
            
      // if there are still items which can go in the overflow menu
      // we need to accomodate the menu
      if(totalVisibleNonSpace+1<totalNonSpace)
       availableWidth-=overflowSize.width;
 
      if(frames[i].size.width<availableWidth){
       [_visibleItems addObject:item];
       visible[i]=YES;
       totalVisible++;
       consumedWidth+=frames[i].size.width;
       if(notSpace[i])
        totalVisibleNonSpace++;
      }
     }
    }
   }
   
   if(totalVisibleNonSpace<totalNonSpace){
    consumedWidth+=overflowSize.width;
    _overflow=YES;
   }
   else {
    _overflow=NO;
   }
   
   // distribute leftover
   NSInteger totalConsumers=totalVisible;
   BOOL      useFlexible=NO;
   
   while(consumedWidth<desiredWidth && totalConsumers>0){
    CGFloat availablePerItem=floor((desiredWidth-consumedWidth)/totalConsumers);
    
    if(availablePerItem<1)
     break;
    
    totalConsumers=0;
    for(i=0;i<count && consumedWidth<desiredWidth;i++){
     if(visible[i] && (!isFlexible[i] || useFlexible)){
      NSToolbarItem *item=[items objectAtIndex:i];
      CGFloat        attempt=frames[i].size.width+availablePerItem;
      CGFloat        final=[item _expandWidth:attempt];
      CGFloat        consumed=final-frames[i].size.width;
     
      if(consumed>=1){
       frames[i].size.width+=consumed;
       consumedWidth+=consumed;
       totalConsumers++;
      }
     }
    }
    if(totalConsumers==0 && !useFlexible){
     useFlexible=YES;
     totalConsumers=totalVisible;
    }
   }
   
   if(setFrame){
    NSRect frame=[self frame];
    
    [self setFrame:NSMakeRect(frame.origin.x,frame.origin.y,desiredWidth,resultHeight)];
   }
    
   CGFloat x=leftMargin;
   
   for(i=0;i<count;i++){
    NSToolbarItem *item=[items objectAtIndex:i];
    if(!visible[i])
     [[item _enclosingView] removeFromSuperview];
    else {
     
     frames[i].origin.x=x;
     [item _setItemViewFrame:frames[i]];
     if([item _enclosingView]!=self)
      [self addSubview:[item _enclosingView]];
	  	[[item _enclosingView] setToolTip:[item toolTip]];

     x+=frames[i].size.width;
    }
   }
   
   [self setNeedsDisplay:YES];
   
}


-(void)drawRect:(NSRect)rect {
   if(_overflow){
    NSSize imageSize=[[self overflowImage] size];
    NSRect rect=[self overflowRect];
    
    rect.origin.x+=(rect.size.width-imageSize.width)/2;
    rect.origin.y+=(rect.size.height-imageSize.height)/2;
    rect.size=imageSize;

    [[self overflowImage] drawInRect:rect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
   }
   
}

-(void)layoutViews {
   [self layoutViewsWithWidth:[self bounds].size.width setFrame:NO];
}

-(void)_insertItem:(NSToolbarItem *)item atIndex:(NSInteger)index {
   [self layoutViews];
}

-(void)_removeItemAtIndex:(NSInteger)index {
   [[[[self subviews] copy] autorelease] makeObjectsPerformSelector:@selector(removeFromSuperview)];
   [self layoutViews];
}

-(void)resizeWithOldSuperviewSize:(NSSize)oldSize  {
   [super resizeWithOldSuperviewSize:oldSize];
    
   [self layoutViews];
}

-(void)popUpOverflowMenu:sender {
}

// NSView dragging destination settings.
// - as drop target: if decoded object is an item, then insert; if NSArray, then replace all
- (unsigned)draggingEntered:(id <NSDraggingInfo>)sender {
    if ([_toolbar customizationPaletteIsRunning]) {
        id droppedObject = [NSUnarchiver unarchiveObjectWithData:[[NSPasteboard pasteboardWithName:NSDragPboard] dataForType:NSToolbarItemIdentifierPboardType]];
        
        if ([droppedObject isKindOfClass:[NSString class]]) {
            NSToolbarItem *item;
            
            item = [_toolbar _itemForItemIdentifier:droppedObject willBeInsertedIntoToolbar:NO];

            if ([[_toolbar itemIdentifiers] containsObject:droppedObject] &&
                [item allowsDuplicatesInToolbar] == NO)
                return NSDragOperationNone;
        }

        return NSDragOperationCopy;
    }
    
    return NSDragOperationNone;
}

// It'd be nice to do the OSX-style visual toolbar item insertion stuff
- (unsigned)draggingUpdated:(id <NSDraggingInfo>)sender {
    return NSDragOperationCopy;
}

- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender {
    return YES;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
    NSPasteboard *pasteboard = [sender draggingPasteboard];
    
    if ([_toolbar customizationPaletteIsRunning]) {
        if ([[pasteboard types] containsObject:NSToolbarItemIdentifierPboardType]) {
            id droppedObject = [NSUnarchiver unarchiveObjectWithData:[pasteboard dataForType:NSToolbarItemIdentifierPboardType]];
            
            if ([droppedObject isKindOfClass:[NSArray class]]) {
                [_toolbar _setItemsWithIdentifiersFromArray:droppedObject];
            }
            else {
                NSPoint location = [self convertPoint:[sender draggingLocation] fromView:nil];
                NSArray *subviews = [self subviews];
                int i, count = [subviews count];
                
                // Figure out what this drop "means". I figure:
                // [ Item 0 ] [ Item 1 ] [ Item 2 ]
                //  0 ] [ Insert 1 ] [ Insert 2 ] ... etc
                for (i = 0; i < count; ++i) {
                    NSRect frame = [[subviews objectAtIndex:i] frame];
                    
                    frame.origin.x -= floor(frame.size.width/2);
                    
                    if (NSPointInRect(location, frame))
                        break;
                }
                
                [_toolbar insertItemWithItemIdentifier:droppedObject atIndex:i];
            }
            
            return YES;
        }
    }
    
    return NO;
}

- (unsigned)draggingSourceOperationMaskForLocal:(BOOL)isLocal 
{
    return NSDragOperationCopy;
}

- (void)mouseDown:(NSEvent *)event
{
    if ([_toolbar customizationPaletteIsRunning]) {
        NSPasteboard *pasteboard = [NSPasteboard pasteboardWithName:NSDragPboard];
        NSView *subview = [self hitTest:[event locationInWindow]];
        int index = [[self subviews] indexOfObject:subview];
        NSToolbarItem *item = [[[_toolbar items] objectAtIndex:index] retain];
        NSRect frame = NSMakeRect(0, 0, [subview frame].size.width + 4, [subview frame].size.height + 4);
        NSImage *image = [[[NSImage alloc] initWithSize:frame.size] autorelease];
        NSData *data = [NSArchiver archivedDataWithRootObject:[item itemIdentifier]];
                                
        [image setCachedSeparately:YES];
        [image lockFocus];
        [item drawInRect:[self bounds]];
        [[NSColor blackColor] setStroke];
        NSFrameRect(frame);
        [image unlockFocus];
        
        [pasteboard declareTypes:[NSArray arrayWithObject:NSToolbarItemIdentifierPboardType] owner:nil];
        [pasteboard setData:data forType:NSToolbarItemIdentifierPboardType];
        
        [_toolbar removeItemAtIndex:index];
        
        [self dragImage:image 
                     at:NSMakePoint([[item image] size].width/2, [[item image] size].height/2)
                 offset:NSMakeSize(0,0)
                  event:event 
             pasteboard:pasteboard 
                 source:self 
              slideBack:YES];
        
        [item release];
        return;
    }

   if(!_overflow)
    return;
     
   if(!NSPointInRect([self convertPoint:[event locationInWindow] fromView:nil],[self overflowRect]))
   return;
     
   NSArray      *items=[_toolbar items];
   NSInteger     i,count=[items count];
   NSMenu       *menu=[[NSMenu alloc] initWithTitle: NSLocalizedStringFromTableInBundle(@"Overflow", nil, [NSBundle bundleForClass: [NSToolbarView class]], @"Describes the overflow area of the toolbar")];
   NSMenuWindow *window;
   NSMenuItem   *item;
   NSRect       menuFrame = [self frame];

   for(i=0;i<count;i++){
    NSToolbarItem *item=[items objectAtIndex:i];
    NSString      *identifier=[item itemIdentifier];
    
    if([identifier isEqualToString:NSToolbarFlexibleSpaceItemIdentifier] ||
       [identifier isEqualToString:NSToolbarSeparatorItemIdentifier] ||
       [identifier isEqualToString:NSToolbarSpaceItemIdentifier])
     continue;

    if([[item _enclosingView] superview]==nil)
     [menu addItem:[item menuFormRepresentation]];
   }
       
   for (i = 0; i < [menu numberOfItems]; ++i) {
    NSToolbarItem *item = [[menu itemAtIndex:i] representedObject];
        
    if ([[item label] sizeWithAttributes:nil].width > menuFrame.size.width)
     menuFrame.size.width = [[item label] sizeWithAttributes:nil].width + 20.0;  // argh
   }
    
   window=[[NSMenuWindow alloc] initWithMenu:menu];

   menuFrame.origin.x = NSMaxX([self frame]);
   menuFrame.origin.y = [self frame].origin.y + [self frame].size.height/2;
   menuFrame.origin = [[self window] convertBaseToScreen:menuFrame.origin];

   menuFrame.origin.y -= [[window menuView] frame].size.height;

   [window setFrameOrigin:menuFrame.origin];
   [window orderFront:nil];
    
   BOOL        didAccept=[window acceptsMouseMovedEvents];
   NSMenuItem *menuItem;
   
   [window setAcceptsMouseMovedEvents:YES];
   menuItem=[[window menuView] trackForEvent:event];
   [window setAcceptsMouseMovedEvents:didAccept];

   [window close];
    
   if(menuItem != nil)
    [NSApp sendAction:[menuItem action] to:[menuItem target] from:[menuItem representedObject]];
        
   [menu release];
}

@end
