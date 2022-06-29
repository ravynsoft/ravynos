/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/NSMainMenuView.h>
#import <AppKit/NSMenuWindow.h>
#import <AppKit/NSSubmenuView.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSGraphicsStyle.h>

/*
   - implement raised border on mouse over
   - use system menu font
 */

@implementation NSMainMenuView

+(NSFont *)menuFont {
   return [NSFont menuBarFontOfSize:15.0];
}

+(float)menuHeight {
   NSDictionary *attributes=[NSDictionary dictionaryWithObjectsAndKeys:
     [self menuFont],NSFontAttributeName,nil];
   float         result=[@"Menu" sizeWithAttributes:attributes].height;

   result+=3; // border top/bottom margin
   result+=4; // border
   result+=1; // sunken title baseline

	return result;
}

-initWithFrame:(NSRect)frame menu:(NSMenu *)menu {
   [super initWithFrame:frame];

   _menu=[menu retain];
   _font=[[[self class] menuFont] retain];
   _selectedItemIndex=NSNotFound;
   [self sizeToFit];

   return self;
}

-(void)dealloc {
   // [_menu release]; NSView does this for us
   [_font release];
   [super dealloc];
}

-(BOOL)isFlipped {
   return YES;
}

-(BOOL)isOpaque {
   return YES;
}

-(NSMenu *)menu {
   return _menu;
}

-(void)setMenu:(NSMenu *)menu {
   [super setMenu:menu];
   [self sizeToFit];
}

-(NSRect)titleRectForItem:(NSMenuItem *)item previousBorderRect:(NSRect)previousBorderRect
{
	NSRect result;
	NSSize titleSize = NSZeroSize;
        if([item hasSubmenu] && [[[item submenu] _name] isEqualToString:@"NSAppleMenu"])
            titleSize = [[self graphicsStyle] menuItemAttributedTextSize:[item attributedTitle]];
        else
            titleSize = [[self graphicsStyle] menuItemTextSize:[item title]];
        NSImage *img = [item image];
        if(img) {
            titleSize.width += [img size].width;
            if(titleSize.height < 22)
                titleSize.height = 22; // whee, magic numbers (height of menu bar)
        }

	result.origin = NSMakePoint(NSMaxX(previousBorderRect)+6,floor(([self bounds].size.height-titleSize.height)/2));
	result.size = titleSize;

   return result;
}

-(NSRect)borderRectFromTitleRect:(NSRect)titleRect {
   NSRect result=NSInsetRect(titleRect,-6,0);

   result.size.height=[self bounds].size.height;
   result.origin.y=0;

   return result;
}

-(void)sizeToFit {
#if 1
   [self setFrameSize:NSMakeSize([self frame].size.width,[[self class] menuHeight])];
#else
   NSArray      *items=[[self menu] itemArray];
   int           i,count=[items count];
   float         height=0;

   if(count==0){
    [self setFrameSize:NSMakeSize([self frame].size.width,0)];
    return;
   }

   for(i=0;i<count;i++){
    NSMenuItem *item=[items objectAtIndex:i];
    NSString   *title=[item title];
    NSSize      size=[title sizeWithAttributes:[self titleAttributes]];

    height=MAX(height,size.height);
   }

   height+=2; // border top/bottom margin
   height+=4; // border
   height+=1; // sunken title baseline

   [self setFrameSize:NSMakeSize([self frame].size.width,height)];
#endif
}

static void drawSunkenBorder(NSRect rect){
   NSRect   rects[5];
   NSColor *colors[5];

	rects[0]=rect;
   rects[0].size.width=1;
   colors[0]=[NSColor darkGrayColor];
   rects[1]=rect;
   rects[1].size.height=1;
   colors[1]=[NSColor darkGrayColor];
   rects[2]=rect;
   rects[2].origin.x=NSMaxX(rect)-1;
   rects[2].size.width=1;
   colors[2]=[NSColor whiteColor];
   rects[3]=rect;
   rects[3].origin.y=NSMaxY(rect)-1;
   rects[3].size.height=1;
   colors[3]=[NSColor whiteColor];
   rects[4]=NSInsetRect(rect,1,1);
   colors[4]=[NSColor controlColor];

   NSRectFillListWithColors(rects,colors,5);
}

-(NSImage *)overflowImage {
   return [[self window] isKeyWindow]?[NSImage imageNamed:@"NSMenuViewDoubleRightArrow"]:
     [NSImage imageNamed:@"NSMenuViewDoubleRightArrowGray"];
}

-(NSRect)overflowRect {
   NSRect   bounds=[self bounds];
   NSImage *image=[self overflowImage];
   NSSize   size=[image size];
   NSRect   rect=NSInsetRect(NSMakeRect(0,0,size.width,bounds.size.height),-3,0);

   rect.origin.x=NSMaxX(bounds)-rect.size.width;

   return rect;
}

-(void)drawRect:(NSRect)rect {
	NSRect        bounds=[self bounds];
	NSArray      *items=[[self menu] itemArray];
	int           i,count=[items count];
	NSRect        previousBorderRect=NSMakeRect(0,0,0,0);
	BOOL          overflow=NO;
	NSPoint       mouseLoc = [[NSApp currentEvent] locationInWindow];
	    
	mouseLoc = [self convertPoint:mouseLoc fromView:nil];
	
	[[self graphicsStyle] drawMenuBarBackgroundInRect:bounds];
	
	for(i=0;i<count;i++){
		NSMenuItem *item=[items objectAtIndex:i];
		NSString   *title=[item title];
		NSRect      titleRect=[self titleRectForItem:item previousBorderRect:previousBorderRect];
		NSRect      borderRect=[self borderRectFromTitleRect:titleRect];
		
		[[self graphicsStyle] drawMenuBarItemBorderInRect:borderRect hover:(i==_selectedItemIndex)/*NSPointInRect(mouseLoc,borderRect)*/ selected:(i==_selectedItemIndex)];
		
		titleRect.origin.x = borderRect.origin.x + (NSWidth(borderRect) - NSWidth(titleRect)) / 2;
		titleRect.origin.y = borderRect.origin.y + (NSHeight(borderRect) - NSHeight(titleRect)) / 2;

                NSImage *img = [item image];
                if(img) {
                    NSPoint pt = titleRect.origin;
                    pt.y += ([img size].height);
                    [[item image] compositeToPoint:pt operation:NSCompositeSourceOver];
                }

                if([item hasSubmenu] && [[[item submenu] _name] isEqualToString:@"NSAppleMenu"]) 
                    [[self graphicsStyle] drawAttributedMenuItemText:[item attributedTitle]
                        inRect:titleRect enabled:YES selected:(i==_selectedItemIndex)];
                else
                    [[self graphicsStyle] drawMenuItemText:title inRect:titleRect
                        enabled:YES selected:(i==_selectedItemIndex)];
		
		previousBorderRect=borderRect;
		
		if(NSMaxX(borderRect)>NSMaxX(bounds)){
			overflow=YES;
			break;
		}
	}
	
	if(overflow){
		NSImage *image=[self overflowImage];
		NSSize   size=[image size];
		NSRect   rect=[self overflowRect];
		NSPoint  origin;
		NSRect   fill=rect;
		
		[[NSColor controlColor] setFill];
		fill.origin.x-=4;
		fill.size.width+=4;
		NSRectFill(fill);
		
		if(_selectedItemIndex==count)
			drawSunkenBorder(rect);
		
		origin=rect.origin;
		origin.x+=3;
		origin.y+=floor((rect.size.height-size.height)/2);
		[image compositeToPoint:origin operation:NSCompositeSourceOver];
	}
}

-(unsigned)overflowIndex {
   NSRect        bounds=[self bounds];
   NSArray      *items=[[self menu] itemArray];
   unsigned      i,count=[items count];
   NSRect        previousBorderRect=NSMakeRect(0,0,0,0);

   for(i=0;i<count;i++){
    NSMenuItem *item=[items objectAtIndex:i];
    NSRect      titleRect=[self titleRectForItem:item previousBorderRect:previousBorderRect];
    NSRect      borderRect=[self borderRectFromTitleRect:titleRect];

    if(NSMaxX(borderRect)>NSMaxX(bounds))
     return i;

    previousBorderRect=borderRect;
   }

   return NSNotFound;
}

-(unsigned)itemIndexAtPoint:(NSPoint)point {
   unsigned      result=NSNotFound;
   NSRect        bounds=[self bounds];
   NSArray      *items=[[self menu] itemArray];
   unsigned      i,count=[items count];
   NSRect        previousBorderRect=NSMakeRect(0,0,0,0);
   BOOL          overflow=NO;

   for(i=0;i<count;i++){
    NSMenuItem *item=[items objectAtIndex:i];
    NSRect      titleRect=[self titleRectForItem:item previousBorderRect:previousBorderRect];
    NSRect      borderRect=[self borderRectFromTitleRect:titleRect];

    if(NSMaxX(borderRect)>NSMaxX(bounds))
     overflow=YES;

    if(NSMouseInRect(point,borderRect,[self isFlipped]))
     result=i;

    previousBorderRect=borderRect;
   }

   if(overflow){
    if(NSMouseInRect(point,[self overflowRect],[self isFlipped]))
     return count;
   }

   return result;
}

-(void)positionBranchForSelectedItem:(NSWindow *)branch screen:(NSScreen *)screen {
   NSRect        branchFrame=[branch frame];
   NSRect        screenVisible=[screen visibleFrame];
   NSArray      *items=[[self menu] itemArray];
   unsigned      i,count=[items count];
   NSRect        previousBorderRect=NSMakeRect(0,0,0,0);
   NSRect        itemRect=NSZeroRect;
   NSPoint       topLeft=NSZeroPoint;

   if(_selectedItemIndex==count){
    itemRect=[self overflowRect];
    topLeft=NSMakePoint(itemRect.origin.x,NSMaxY(itemRect));
   } else {
    for(i=0;i<count;i++){
     NSMenuItem *item=[items objectAtIndex:i];
     NSRect      titleRect=[self titleRectForItem:item previousBorderRect:previousBorderRect];
     itemRect=[self borderRectFromTitleRect:titleRect];
 
     if(i==_selectedItemIndex){
      topLeft=NSMakePoint(itemRect.origin.x,NSMaxY(itemRect));
      break;
     }

     previousBorderRect=itemRect;
    }
   }

   topLeft=[self convertPoint:topLeft toView:nil];
   topLeft=[[self window] convertBaseToScreen:topLeft];

   if(topLeft.y-branchFrame.size.height<NSMinY(screenVisible)){
    topLeft=NSMakePoint(itemRect.origin.x,NSMinY(itemRect));

    topLeft=[self convertPoint:topLeft toView:nil];
    topLeft=[[self window] convertBaseToScreen:topLeft];

    topLeft.y+=branchFrame.size.height;
   }

   if(topLeft.x+branchFrame.size.width>NSMaxX(screenVisible))
    topLeft.x=NSMaxX(screenVisible)-branchFrame.size.width;
   if(topLeft.x<NSMinX(screenVisible))
    topLeft.x=NSMinX(screenVisible);

    // Wayland hack: our WLWindowPopUp is a subsurface that is attached
    // to the parent surface, so its x,y are relative to that, not the
    // screen frame. FIXME: find a better way to do this.
    topLeft.x -= screenVisible.origin.x;
    topLeft.y = -1; // this is the bottom of the menubar frame

  [branch setFrameTopLeftPoint:topLeft];
}

-(NSMenuView *)viewAtSelectedIndexPositionOnScreen:(NSScreen *)screen {
   NSArray *items=[self visibleItemArray];

   if(_selectedItemIndex==[items count]){
    NSMenuWindow *branch=[[NSMenuWindow alloc] initWithMenu:[self menu] overflowAtIndex:[self overflowIndex]];

    [self positionBranchForSelectedItem:branch screen:screen];

    [branch setParent:[self window]];
    [branch orderFront:nil];
    return [branch menuView];
   }

   if(_selectedItemIndex<[items count]){
    NSMenuItem *item=[items objectAtIndex:_selectedItemIndex];

    if([item hasSubmenu]){
     if([[[item submenu] itemArray] count]>0){
      NSMenuWindow *branch=[[NSMenuWindow alloc] initWithMenu:[item submenu]];

      [self positionBranchForSelectedItem:branch screen:screen];

      [branch setParent:[self window]];
      [branch orderFront:nil];
      return [branch menuView];
     }
    }
   }
   return nil;
}


@end
