/*
 * Copyright (C) 2024 Zoe Knox <zoe@ravynsoft.com>
 * Contains code Copyright (c) 2006-2007 Christopher J. W. Lloyd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#import "SubmenuView.h"
#import "NSMenuWindow.h"

#define MIN_TITLE_KEY_GAP 8
#define WINDOW_BORDER_THICKNESS 1
#define IMAGE_TITLE_GAP 8

static NSRect boundsToTitleAreaRect(NSRect rect){
   return NSInsetRect(rect, WINDOW_BORDER_THICKNESS, WINDOW_BORDER_THICKNESS);
}

@implementation SubmenuView

-initWithMenu:(NSMenu *)menu {
    NSRect frame = NSZeroRect;
    self = [self initWithFrame:frame];

    _menu = menu;
    _selectedItemIndex = NSNotFound;

    frame.size = [self sizeForMenuItems:[self visibleItemArray]];
    [self setFrame:frame];

    return self;
}

-(BOOL)isFlipped {
    return YES;
}

-(BOOL)isOpaque {
    return YES;
}

-(NSArray *)itemArray {
   return [[self menu] itemArray];
}

-(NSArray *)visibleItemArray {
    NSArray *items = [[self menu] itemArray];
   
    // Construct a new array of just the visible items
    if(_visibleArray == nil)
        _visibleArray = [NSMutableArray new];
    [_visibleArray removeAllObjects];
   
    for(int i = 0; i < [items count]; ++i) {
        NSMenuItem *item = [items objectAtIndex:i];
        if(![item isHidden])
	     [_visibleArray addObject:item];
    }

    return _visibleArray;
}

-(float)heightOfMenuItem:(NSMenuItem *)item {
    NSSize titleSize = [self contentSizeForItem:item];
    return titleSize.height;
}

#define ITEM_MAX(size) height = MAX(height, size.height); width += size.width;
-(NSSize)contentSizeForItem:(NSMenuItem *)item {
    if([item isSeparatorItem])	{
        return [[self graphicsStyle] menuItemSeparatorSize];
    } else {
        float width = [[self graphicsStyle] menuItemGutterGap];
        float height = 0.0f;

        NSSize size = [[self graphicsStyle] menuItemGutterSize];
        ITEM_MAX(size);

        size = [self sizeForMenuItemImage:item];
        if(size.width > 0.0f) {
            ITEM_MAX(size);
            width += IMAGE_TITLE_GAP;
        }

        size = [[self graphicsStyle] menuItemTextSize:[item title]];
        ITEM_MAX(size);

        if([[item keyEquivalent] length] == 0) {
            size = [[self graphicsStyle] menuItemTextSize:[item _keyEquivalentDescription]];
            ITEM_MAX(size);
        }

        if([item hasSubmenu] && [[item submenu] numberOfItems] > 0) {
            size = [[self graphicsStyle] menuItemBranchArrowSize];
            ITEM_MAX(size);
        }
		
        return NSMakeSize(width, height);
    }
}

-(NSSize)sizeForMenuItemImage:(NSMenuItem *)item {
    if([item image] != nil)
        return [[item image] size];
    return NSZeroSize;
}

-(NSSize)sizeForMenuItems:(NSArray *)items {
    NSSize result = NSZeroSize;
    float maxTitleWidth = 0;
    BOOL anItemHasAnImage = NO;
    float maxKeyWidth = 0; 
    float totalHeight = 0;
    NSSize gutterSize = [[self graphicsStyle] menuItemGutterSize];
    NSSize rightArrowSize = [[self graphicsStyle] menuItemBranchArrowSize];
    NSSize separatorSize = [[self graphicsStyle] menuItemSeparatorSize];
	
    for(int i = 0; i < [items count]; ++i) {
        NSMenuItem *item = [items objectAtIndex:i];
        if([item isSeparatorItem]) {
            totalHeight += separatorSize.height;
        } else {
            NSSize size = NSZeroSize;
            float height = 0;
            float titleAndIconWidth = 0;
			
            size = [self sizeForMenuItemImage:item];
            height = MAX(height, size.height);
            if ((titleAndIconWidth = size.width) > 0.0f)
                anItemHasAnImage = YES;

            size = [[self graphicsStyle] menuItemTextSize:[item title]];
            titleAndIconWidth += size.width;
            maxTitleWidth = MAX(maxTitleWidth, titleAndIconWidth);
            height = MAX(height, size.height);
            
            if([[item keyEquivalent] length] != 0)
            {
                size = [[self graphicsStyle] menuItemTextSize:[item _keyEquivalentDescription]];
                maxKeyWidth = MAX(maxKeyWidth, size.width);
                height = MAX(height, size.height);
            }
            height = MAX(height, gutterSize.height);
            height = MAX(height, rightArrowSize.height);
			
            totalHeight += height;
        }
    }
	
    result.height = totalHeight;
    result.width = gutterSize.width;
    result.width += [[self graphicsStyle] menuItemGutterGap];

    if(anItemHasAnImage)
        result.width += IMAGE_TITLE_GAP;

    result.width += maxTitleWidth;
    
    if(maxKeyWidth > 0.0f) {
        result.width += MIN_TITLE_KEY_GAP;
        result.width += maxKeyWidth;
    }

    result.width += rightArrowSize.width;
	
    // border
    result.width += WINDOW_BORDER_THICKNESS*2;
    result.height += WINDOW_BORDER_THICKNESS*2;
    
    return result;
}

-(void)positionBranchForSelectedItem:(NSWindow *)branch screen:(NSScreen *)screen {
    NSRect branchFrame = [branch frame];
    //NSRect windowFrame = [window frame];
    NSRect screenVisible = [screen visibleFrame];
    NSArray *items = [[self menu] itemArray];
    unsigned i, count = [items count];
    NSRect itemRect = boundsToTitleAreaRect([self bounds]);
    NSPoint topLeft = NSZeroPoint;

    for(i = 0; i < count; i++) {
        NSMenuItem *item = [items objectAtIndex:i];

        itemRect.size.height = [self heightOfMenuItem:item];
    
        if(i == _selectedItemIndex) {
            topLeft = itemRect.origin;

            topLeft.x += itemRect.size.width;
            //topLeft.x -= WINDOW_BORDER_THICKNESS;
            topLeft.y -= WINDOW_BORDER_THICKNESS;

            break;
        }

        itemRect.origin.y += itemRect.size.height;
    }

    topLeft = [self convertPoint:topLeft toView:nil];
    //topLeft = [[self window] convertBaseToScreen:topLeft];

    if(topLeft.y - branchFrame.size.height < NSMinY(screenVisible)) {
        topLeft = itemRect.origin;

        topLeft.x += itemRect.size.width;
        topLeft.y += itemRect.size.height;
        topLeft.y += WINDOW_BORDER_THICKNESS;

        topLeft = [self convertPoint:topLeft toView:nil];
        //topLeft = [[self window] convertBaseToScreen:topLeft];

        topLeft.y += branchFrame.size.height;
    }

    if(topLeft.x + branchFrame.size.width > NSMaxX(screenVisible)) {
        NSPoint redo = itemRect.origin;

        redo = [self convertPoint:redo toView:nil];
        //redo = [[self window] convertBaseToScreen:redo];
        redo.x -= branchFrame.size.width;

        topLeft.x = redo.x;
    }

    //    topLeft.y -= windowFrame.origin.y;
    //    topLeft.x -= windowFrame.origin.x;

    [branch setFrameTopLeftPoint:topLeft];
    [self setNeedsDisplay:YES];
}

-(SubmenuView *)viewAtSelectedIndexPositionOnScreen:(NSScreen *)screen {
    NSArray *items = [self visibleItemArray];

#if 0
    if(_selectedItemIndex==[items count]) {
        NSMenuWindow *branch=[[NSMenuWindow alloc] initWithMenu:[self menu] overflowAtIndex:[self overflowIndex]];

        [self positionBranchForSelectedItem:branch screen:screen];

        [branch orderFront:nil];
        return [branch menuView];
    }
#endif

    if(_selectedItemIndex < [items count]) {
        NSMenuItem *item = [items objectAtIndex:_selectedItemIndex];

        if([item hasSubmenu]) {
            if([[[item submenu] itemArray] count] > 0) {
                NSMenuWindow *branch = [[NSMenuWindow alloc] initWithMenu:[item submenu]];

                [self positionBranchForSelectedItem:branch screen:screen];

                [branch orderFront:nil];
                return [branch menuView];
            }
        }
    }
    return nil;
}

-(NSRect)titleRectForItem:(NSMenuItem *)item previousBorderRect:(NSRect)previousBorderRect {
    NSRect result = NSZeroRect;
    NSSize titleSize = NSZeroSize;

    titleSize = [[self graphicsStyle] menuItemTextSize:[item title]];
    NSImage *img = [item image];
    if(img) {
        titleSize.width += [img size].width;
        if(titleSize.height < 22)
            titleSize.height = 22; // whee, magic numbers (height of menu bar)
    }

    result.origin = NSMakePoint(6, previousBorderRect.origin.y);
    result.size = titleSize;
    return result;
}

-(NSRect)borderRectFromTitleRect:(NSRect)titleRect {
    NSRect result = NSInsetRect(titleRect, -6, 0);
    result.size.width = [self bounds].size.width;
    return result;
}
 
-(NSImage *)overflowImage {
   return [NSImage imageNamed:@"NSMenuViewDoubleRightArrow"];
}

-(NSRect)overflowRect {
   NSRect bounds = [self bounds];
   NSImage *image = [self overflowImage];
   NSSize size = [image size];
   NSRect rect = NSInsetRect(NSMakeRect(0, 0, size.width, bounds.size.height), -3, 0);

   rect.origin.x = NSMaxX(bounds) - rect.size.width;
   return rect;
}


-(unsigned)itemIndexAtPoint:(NSPoint)point {
    NSInteger result = NSNotFound;
    NSRect bounds = [self bounds];
    NSArray *items = [[self menu] itemArray];
    unsigned i, count = [items count];
    NSRect previousBorderRect = NSMakeRect(0, 0, 0, 0);
    BOOL overflow = NO;

    for(i = 0; i < count; ++i) {
        NSMenuItem *item = [items objectAtIndex:i];
        NSRect titleRect = [self titleRectForItem:item previousBorderRect:previousBorderRect];
        NSRect borderRect = [self borderRectFromTitleRect:titleRect];

        if(NSMaxX(borderRect) > NSMaxX(bounds))
            overflow = YES;

        if(NSMouseInRect(point, borderRect, [self isFlipped]))
            result = i;

        previousBorderRect = borderRect;
        previousBorderRect.origin.y += [self heightOfMenuItem:item];
    }

    if(overflow) {
        if(NSMouseInRect(point, [self overflowRect], [self isFlipped]))
            return count;
    }

    return result;
}

-(NSUInteger)selectedItemIndex {
   return _selectedItemIndex;
}

-(void)setSelectedItemIndex:(NSUInteger)itemIndex {
    if (_selectedItemIndex != itemIndex) {
        _selectedItemIndex=itemIndex;
        [self setNeedsDisplay:YES];
    }
}

-(NSMenuItem *)itemAtSelectedIndex {
   NSArray *items=[self visibleItemArray];

   if(_selectedItemIndex<[items count])
    return [items objectAtIndex:_selectedItemIndex];

   return nil;
}

-(void)mouseDown:(NSEvent *)event {
    BOOL didAccept = [[self window] acceptsMouseMovedEvents];

    [[self window] setAcceptsMouseMovedEvents:YES];
    NSMenuItem *item = [self trackForEvent:event];
    [[self window] setAcceptsMouseMovedEvents:didAccept];
   
    if(item != nil)
        [NSApp sendAction:[item action] to:[item target] from:item];
}

-(NSMenuItem *)trackForEvent:(NSEvent *)event {
    NSLog(@"tracking for event");
    return nil;
}

// This exists because most of AppKit (incl NSView) is not built with ARC, and
// we need something that uses ARC to hold a ref to the window, or it will get
// deallocated prematurely!
-(void)setWindow:(NSWindow *)window {
    _menuWindow = window;
}

#define CENTER_PART_RECT_VERTICALLY(partSize)                                \
{                                                                            \
	NSSize __partSize = (partSize);                                      \
	partRect.origin.y = origin.y + (itemHeight - __partSize.height) / 2; \
	partRect.size.height = __partSize.height;                            \
	partRect.size.width = __partSize.width;                              \
}

-(void)drawRect:(NSRect)rect {
    NSRect bounds = [self bounds];
    NSRect itemArea = boundsToTitleAreaRect(bounds);
    NSArray *items = [self visibleItemArray];
    unsigned i, count = [items count];
    NSPoint origin = itemArea.origin;
    float separatorHeight = [[self graphicsStyle] menuItemSeparatorSize].height; 

    [[self graphicsStyle] drawMenuWindowBackgroundInRect:rect]; // FIXME: MenuBarBackground?	

    for(i = 0; i < count; i++) {
        NSMenuItem *item = [items objectAtIndex:i];
		
        if([item isSeparatorItem]) {
            NSRect separatorRect = NSMakeRect(origin.x, origin.y, NSWidth(itemArea), separatorHeight);
            [[self graphicsStyle] drawMenuSeparatorInRect:separatorRect];
            origin.y += NSHeight(separatorRect);
        } else {
            NSImage *image = [item image];
            BOOL selected = (i == _selectedItemIndex) ? YES : NO;
            float itemHeight = [self heightOfMenuItem:item];
            NSRect partRect;
            NSSize partSize;
            BOOL showsEnabled = ([item isEnabled] || [item hasSubmenu]);
			
            partRect = NSMakeRect(origin.x, origin.y, itemArea.size.width, itemHeight);
            if(selected)
                [[self graphicsStyle] drawMenuSelectionInRect:partRect enabled:showsEnabled];

            // Draw the gutter and checkmark (if any)
            CENTER_PART_RECT_VERTICALLY([[self graphicsStyle] menuItemGutterSize]);
            if ([item state]) {
                [[self graphicsStyle] drawMenuGutterInRect:partRect];			
                [[self graphicsStyle] drawMenuCheckmarkInRect:partRect enabled:showsEnabled selected:selected];
            }
			
            partRect.origin.x += [[self graphicsStyle] menuItemGutterGap];;
			
            // Draw the image
            if(image) {
                NSRect imageRect;
				
                partRect.origin.x += partRect.size.width;
                CENTER_PART_RECT_VERTICALLY([image size]);
				
                CGContextRef ctx = [[NSGraphicsContext currentContext] graphicsPort];
                CGContextSaveGState(ctx);
                CGContextTranslateCTM(ctx, partRect.origin.x, partRect.origin.y);
                if([self isFlipped]) {
                    CGContextTranslateCTM(ctx, 0, partRect.size.height);
                    CGContextScaleCTM(ctx, 1, -1);
                }

                NSRect drawingRect = partRect;
                drawingRect.origin = NSZeroPoint;
                [[self graphicsStyle] drawButtonImage:image inRect:drawingRect enabled:showsEnabled mixed:NO];
                CGContextRestoreGState(ctx);

                partRect.origin.x += IMAGE_TITLE_GAP;
            }
			
            // Draw the title
            partRect.origin.x += partRect.size.width;

            NSAttributedString *atitle = [item attributedTitle];
            if(atitle != nil && [atitle length] > 0) {
                CENTER_PART_RECT_VERTICALLY([atitle size]);
                [[self graphicsStyle] drawAttributedMenuItemText:atitle
                                                          inRect:partRect
                                                         enabled:showsEnabled
                                                        selected:selected];
            } else {
                NSString *title = [item title];
                CENTER_PART_RECT_VERTICALLY([[self graphicsStyle] menuItemTextSize:title]);
                [[self graphicsStyle] drawMenuItemText:title
                                                inRect:partRect
                                               enabled:showsEnabled
                                              selected:selected];
            }
			
            // Draw the key equivalent
            if([[item keyEquivalent] length] != 0) {
                NSString *keyString = [item _keyEquivalentDescription];
                NSSize branchArrowSize = [[self graphicsStyle] menuItemBranchArrowSize];
                NSSize keyEquivalentSize = [[self graphicsStyle] menuItemTextSize:keyString];
				
                partRect.origin.x = origin.x + NSWidth(itemArea) - branchArrowSize.width - keyEquivalentSize.width;
                CENTER_PART_RECT_VERTICALLY(keyEquivalentSize);
                [[self graphicsStyle] drawMenuItemText:keyString
                                                inRect:partRect
                                               enabled:showsEnabled
                                              selected:selected];
            }
			
            // Draw the submenu arrow
            if([item hasSubmenu] && [[item submenu] numberOfItems] > 0) {
                NSSize branchArrowSize = [[self graphicsStyle] menuItemBranchArrowSize];
                partRect.origin.x = origin.x + NSWidth(itemArea) - branchArrowSize.width;
                partRect.size.width = branchArrowSize.width;
                CENTER_PART_RECT_VERTICALLY(branchArrowSize);
                [[self graphicsStyle] drawMenuBranchArrowInRect:partRect enabled:showsEnabled selected:selected];
            }
			
            origin.y += itemHeight;
        }
    }
}

@end
