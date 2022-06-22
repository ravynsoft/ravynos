/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
   Copyright (c) 2022 Zoe Knox <zoe@pixin.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/NSSubmenuView.h>
#import <AppKit/NSMenuWindow.h>
#import <AppKit/NSGraphicsStyle.h>

@implementation NSSubmenuView

#define MIN_TITLE_KEY_GAP 8
#define WINDOW_BORDER_THICKNESS 3
#define IMAGE_TITLE_GAP 8

-(NSSize)sizeForMenuItemImage:(NSMenuItem *)item {
	NSSize result=NSZeroSize;
	
	if([item image]!=nil)
		result=[[item image] size];
	
	return result;
}

-(NSSize)contentSizeForItem:(NSMenuItem *)item {
	if ([item isSeparatorItem])	{
		return [[self graphicsStyle] menuItemSeparatorSize];
	}
	else {
#define ITEM_MAX(size) height = MAX(height,size.height); width += size.width;
		float width = [[self graphicsStyle] menuItemGutterGap];
		float height = 0.0f;
		NSSize size;
		
		size = [[self graphicsStyle] menuItemGutterSize];
		ITEM_MAX(size);
		size = [self sizeForMenuItemImage:item];
		if (size.width > 0.0f) 
		{
			ITEM_MAX(size);
			width += IMAGE_TITLE_GAP;
		}
		size = [[self graphicsStyle] menuItemTextSize:[item title]];
		ITEM_MAX(size);
		if ([[item keyEquivalent] length] == 0)
		{
			size = [[self graphicsStyle] menuItemTextSize:[item _keyEquivalentDescription]];
			ITEM_MAX(size);
		}
		if ([item hasSubmenu])
		{
			size = [[self graphicsStyle] menuItemBranchArrowSize];
			ITEM_MAX(size);
		}
		
		return NSMakeSize(width,height);
	}
}

-(NSSize)sizeForMenuItems:(NSArray *)items {
	NSSize   result=NSZeroSize;
	float    maxTitleWidth = 0.0f;
	BOOL     anItemHasAnImage = NO;
	float    maxKeyWidth = 0.0f; 
	float    totalHeight = 0.0f;
	NSSize   gutterSize = [[self graphicsStyle] menuItemGutterSize];
	NSSize   rightArrowSize = [[self graphicsStyle] menuItemBranchArrowSize];
	unsigned i,count=[items count];
	
	for (i = 0;i<count;i++)
	{
		NSMenuItem *item = [items objectAtIndex:i];
		if ([item isSeparatorItem])
		{
			totalHeight += [[self graphicsStyle] menuItemSeparatorSize].height;
		}
		else
		{
			NSSize     size = NSZeroSize;
			float      height = 0.0f;
			float      titleAndIconWidth = 0.0f;
			
			size = [self sizeForMenuItemImage:item];
			height = MAX(height,size.height);
			if ((titleAndIconWidth = size.width) > 0.0f)
				anItemHasAnImage = YES;

			size = [[self graphicsStyle] menuItemTextSize:[item title]];
			titleAndIconWidth += size.width;
			maxTitleWidth = MAX(maxTitleWidth,titleAndIconWidth);
			height = MAX(height,size.height);
            
			if ([[item keyEquivalent] length] != 0)
			{
				size = [[self graphicsStyle] menuItemTextSize:[item _keyEquivalentDescription]];
				maxKeyWidth=MAX(maxKeyWidth,size.width);
				height = MAX(height,size.height);
			}
			height = MAX(height,gutterSize.height);
			height = MAX(height,rightArrowSize.height);
			
			totalHeight += height;
        }
	}
	
	result.height = totalHeight;
	result.width = gutterSize.width;
	result.width += [[self graphicsStyle] menuItemGutterGap];
	if (anItemHasAnImage)
		result.width += IMAGE_TITLE_GAP;
	result.width += maxTitleWidth;
	if (maxKeyWidth > 0.0f)
	{
		result.width += MIN_TITLE_KEY_GAP;
		result.width += maxKeyWidth;
	}
	result.width += rightArrowSize.width;
	
	// border. Magic constants that may not be right for Win7 vs XP
	result.width += WINDOW_BORDER_THICKNESS*2;
	result.height += WINDOW_BORDER_THICKNESS*2;
	
	return result;
}
	
-initWithMenu:(NSMenu *)menu {
   NSRect frame=NSZeroRect;

   [self initWithFrame:frame];

   _menu=[menu retain];
   _selectedItemIndex=NSNotFound;

   frame.size=[self sizeForMenuItems:[self visibleItemArray]];
   [self setFrame:frame];

   return self;
}

-(BOOL)isFlipped {
   return YES;
}

static NSRect boundsToTitleAreaRect(NSRect rect){
   return NSInsetRect(rect, WINDOW_BORDER_THICKNESS, WINDOW_BORDER_THICKNESS);
}

-(float)heightOfMenuItem:(NSMenuItem *)item {
	NSSize titleSize=[self contentSizeForItem:item];
	
	return titleSize.height;
}

-(void)drawRect:(NSRect)rect 
{
	NSRect   bounds = [self bounds];
	NSRect   itemArea=boundsToTitleAreaRect(bounds);
	NSArray *items=[self visibleItemArray];
	unsigned i,count=[items count];
	NSPoint  origin=itemArea.origin;
	
	[[self graphicsStyle] drawMenuWindowBackgroundInRect:rect];
	
	for(i=0;i<count;i++)
	{
		NSMenuItem *item=[items objectAtIndex:i];
		
		if ([item isSeparatorItem])
		{
			NSRect separatorRect = NSMakeRect(origin.x,origin.y,NSWidth(itemArea),[[self graphicsStyle] menuItemSeparatorSize].height);

			[[self graphicsStyle] drawMenuSeparatorInRect:separatorRect];
			
			origin.y += NSHeight(separatorRect);
		}
		else 
		{
#define CENTER_PART_RECT_VERTICALLY(partSize)                          \
{                                                                      \
	NSSize __partSize = (partSize);                                    \
	partRect.origin.y = origin.y + (itemHeight - __partSize.height) / 2; \
	partRect.size.height = __partSize.height;                            \
	partRect.size.width = __partSize.width;                              \
}
			NSImage      *image = [item image];
			BOOL         selected = (i ==_selectedItemIndex) ? YES : NO;
			float        itemHeight = [self heightOfMenuItem:item];
			NSRect       partRect;
			NSSize       partSize;
			BOOL         showsEnabled = ([item isEnabled] || [item hasSubmenu]);
			
			partRect = NSMakeRect(origin.x,origin.y,itemArea.size.width,itemHeight);

			if (selected)
				[[self graphicsStyle] drawMenuSelectionInRect:partRect enabled:showsEnabled];

			// Draw the gutter and checkmark (if any)
			CENTER_PART_RECT_VERTICALLY([[self graphicsStyle] menuItemGutterSize]);
			if ([item state])
			{
				[[self graphicsStyle] drawMenuGutterInRect:partRect];			
				[[self graphicsStyle] drawMenuCheckmarkInRect:partRect enabled:showsEnabled selected:selected];
			}
			
			partRect.origin.x += [[self graphicsStyle] menuItemGutterGap];;
			
			// Draw the image
			if (image)
			{
				NSRect imageRect;
				
				partRect.origin.x += partRect.size.width;
				CENTER_PART_RECT_VERTICALLY([image size]);
				
                CGContextRef ctx=[[NSGraphicsContext currentContext] graphicsPort];
                CGContextSaveGState(ctx);
                CGContextTranslateCTM(ctx,partRect.origin.x,partRect.origin.y);
                if([self isFlipped]){
                    CGContextTranslateCTM(ctx,0,partRect.size.height);
                    CGContextScaleCTM(ctx,1,-1);
                }
                NSRect drawingRect = partRect;
                drawingRect.origin = NSZeroPoint;
                [[self graphicsStyle] drawButtonImage:image inRect:drawingRect enabled:showsEnabled mixed:NO];
                CGContextRestoreGState(ctx);

				partRect.origin.x += IMAGE_TITLE_GAP;
			}
			
			// Draw the title
			partRect.origin.x += partRect.size.width;

			NSAttributedString     *atitle = [item attributedTitle];
			if (atitle != nil && [atitle length] > 0) {
				CENTER_PART_RECT_VERTICALLY([atitle size]);
				[[self graphicsStyle] drawAttributedMenuItemText:atitle inRect:partRect enabled:showsEnabled selected:selected];
			} else {
				NSString     *title = [item title];
				CENTER_PART_RECT_VERTICALLY([[self graphicsStyle] menuItemTextSize:title]);
				[[self graphicsStyle] drawMenuItemText:title inRect:partRect enabled:showsEnabled selected:selected];
			}
			
			// Draw the key equivalent
			if ([[item keyEquivalent] length] != 0)
			{
				NSString *keyString = [item _keyEquivalentDescription];
				NSSize   branchArrowSize = [[self graphicsStyle] menuItemBranchArrowSize];
				NSSize   keyEquivalentSize = [[self graphicsStyle] menuItemTextSize:keyString];
				
				partRect.origin.x = origin.x + NSWidth(itemArea) - branchArrowSize.width - keyEquivalentSize.width;
				CENTER_PART_RECT_VERTICALLY(keyEquivalentSize);
				[[self graphicsStyle] drawMenuItemText:keyString inRect:partRect enabled:showsEnabled selected:selected];
			}
			
			// Draw the submenu arrow
			if([item hasSubmenu])
			{
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

-(unsigned)itemIndexAtPoint:(NSPoint)point {
   NSArray *items=[[self menu] itemArray];
   unsigned i,count=[items count];
   NSRect   check=boundsToTitleAreaRect([self bounds]);

   for(i=0;i<count;i++){
    NSMenuItem *item=[items objectAtIndex:i];

    check.size.height=[self heightOfMenuItem:item];
    check.size.height+=2;
    check.origin.y-=2;

    if(NSMouseInRect(point,check,[self isFlipped]))
     return i;

    check.origin.y+=check.size.height;
   }

   return NSNotFound;
}

-(void)positionBranchForSelectedItem:(NSWindow *)branch window:(NSWindow *)window screen:(NSScreen *)screen {
   NSRect   branchFrame=[branch frame];
   NSRect   windowFrame=[window frame];
   NSRect   screenVisible=[screen visibleFrame];
   NSArray *items=[[self menu] itemArray];
   unsigned i,count=[items count];
   NSRect   itemRect=boundsToTitleAreaRect([self bounds]);
   NSPoint  topLeft=NSZeroPoint;


   for(i=0;i<count;i++){
    NSMenuItem *item=[items objectAtIndex:i];

    itemRect.size.height=[self heightOfMenuItem:item];

    if(i==_selectedItemIndex){
     topLeft=itemRect.origin;

     topLeft.x+=itemRect.size.width;
     //topLeft.x-=WINDOW_BORDER_THICKNESS;
     topLeft.y-=WINDOW_BORDER_THICKNESS;

     break;
    }

    itemRect.origin.y+=itemRect.size.height;
   }

   topLeft=[self convertPoint:topLeft toView:nil];
   topLeft=[[self window] convertBaseToScreen:topLeft];

   if(topLeft.y-branchFrame.size.height<NSMinY(screenVisible)){
    topLeft=itemRect.origin;

    topLeft.x+=itemRect.size.width;
    topLeft.y+=itemRect.size.height;
    topLeft.y+=WINDOW_BORDER_THICKNESS;

    topLeft=[self convertPoint:topLeft toView:nil];
    topLeft=[[self window] convertBaseToScreen:topLeft];

    topLeft.y+=branchFrame.size.height;
   }

   if(topLeft.x+branchFrame.size.width>NSMaxX(screenVisible)){
    NSPoint redo=itemRect.origin;

    redo=[self convertPoint:redo toView:nil];
    redo=[[self window] convertBaseToScreen:redo];
    redo.x-=branchFrame.size.width;

    topLeft.x=redo.x;
   }

    topLeft.y -= windowFrame.origin.y;
    topLeft.x -= windowFrame.origin.x;

   [branch setFrameTopLeftPoint:topLeft];
}

-(NSMenuView *)viewAtSelectedIndexPositionOnScreen:(NSScreen *)screen {
   NSArray *items=[self visibleItemArray];

   if(_selectedItemIndex<[items count]){
    NSMenuItem *item=[items objectAtIndex:_selectedItemIndex];

    if([item hasSubmenu]){
     NSMenuWindow *branch=[[NSMenuWindow alloc] initWithMenu:[item submenu]];

     [self positionBranchForSelectedItem:branch window:[self window] screen:screen];

     [branch setParent:[self window]];
     [branch orderFront:nil];
     return [branch menuView];
    }
   }
   return nil;
}

@end

/*@implementation MenuMetrics

-(id)initWithStyle:(NSGraphicsStyle *)style
{
	checkBoxMargins = [style menuItemCheckBoxMargins];
	checkBoxBackgroundMargins = [style menuItemCheckBoxBackgroundMargins];
	itemMargins = [style menuItemMargins];
    keyEquivalentMargins = [style menuItemKeyEquivalentMargins];
	
	checkMarkSize = [style menuItemCheckMarkSize];
	separatorSize = [style menuItemSeparatorSize];
	
	minTitleKeyEquivalentWidth = [style minimumTitleKeyEquivalentWidth];
	extraTitleGutterMargin = [style extraTitleGutterMargin];
	
	titleMargins = itemMargins;
	titleMargins.left = [style menuBorderBackgroundSize].width;
	titleMargins.right = [style menuBorderSize].width;
	
	extraCheckMarkHeight = checkBoxBackgroundMargins.top + checkBoxBackgroundMargins.bottom;
}

-(Margins)checkMarkMargins
{
	return checkMarkMargins;
}

-(Margins)checkMarkBackgroundMargins
{
	return checkMarkBackgroundMargins;
}

-(Margins)itemMargins
{
	return itemMargins;
}

-(Margins)titleMargins
{
	return titleMargins;
}

-(Margins)keyEquivalentMargins
{
	return keyEquivalentMargins;
}

-(NSSize)checkMarkSize
{
	return checkMarkSize;
}

-(NSSize)separatorSize
{
	return separatorSize;
}

-(float)minTitleKeyEquivalentWidth
{
	return minTitleKeyEquivalentWidth;
}

-(float)extraTitleGutterMargin
{
	return extraTitleGutterMargin;
}

-(float)extraCheckMarkHeight
{
	return extraCheckMarkHeight;
}


@end
*/
