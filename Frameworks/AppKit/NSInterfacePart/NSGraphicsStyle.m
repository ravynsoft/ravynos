/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSGraphicsStyle.h>
#import <AppKit/NSInterfacePartAttributedString.h>
#import <AppKit/NSInterfacePartDisabledAttributedString.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSGraphicsContextFunctions.h>
#import "NSInterfaceGraphics.h"

@implementation NSGraphicsStyle

static NSDictionary *sNormalMenuTextAttributes = nil;
static NSDictionary *sSelectedMenuTextAttributes = nil;
static NSDictionary *sDimmedMenuTextAttributes = nil;
static NSDictionary *sDimmedMenuTextShadowAttributes = nil;

+ (void)initialize
{
	if (sNormalMenuTextAttributes == nil)
	{
		NSFont *menuFont = [NSFont menuFontOfSize:0];
		sNormalMenuTextAttributes = [[NSDictionary dictionaryWithObjectsAndKeys:
									  menuFont,NSFontAttributeName,
									  [NSColor menuItemTextColor],NSForegroundColorAttributeName,
									  nil] retain];
		sSelectedMenuTextAttributes = [[NSDictionary dictionaryWithObjectsAndKeys:
									   menuFont,NSFontAttributeName,
									   [NSColor selectedMenuItemTextColor],NSForegroundColorAttributeName,
										nil] retain];
		
		sDimmedMenuTextAttributes = [[NSDictionary dictionaryWithObjectsAndKeys:
									  menuFont,NSFontAttributeName,
									  [NSColor grayColor],NSForegroundColorAttributeName,
									  nil] retain];

		sDimmedMenuTextShadowAttributes = [[NSDictionary dictionaryWithObjectsAndKeys:
											menuFont,NSFontAttributeName,
											[NSColor whiteColor],NSForegroundColorAttributeName,
											nil] retain];
	}
}

-initWithView:(NSView *)view {
   _view=[view retain];
   return self;
}

-(void)dealloc {
   [_view release];
   [super dealloc];
}

@end

@implementation NSGraphicsStyle (NSMenu)


#define TITLE_TOP_MARGIN 2
#define TITLE_BOTTOM_MARGIN 2
#define BRANCH_ARROW_LEFT_MARGIN 2
#define BRANCH_ARROW_RIGHT_MARGIN 2

-(NSInterfacePartAttributedString *)branchArrow
{
	static NSInterfacePartAttributedString *sBranchArrow = nil;

	if (sBranchArrow == nil)
		sBranchArrow = [[NSInterfacePartAttributedString alloc] initWithFACharacter:0xF054];
	return sBranchArrow;
}

- (NSInterfacePartAttributedString *)checkMark
{
	static NSInterfacePartAttributedString *sCheckMark = nil;
	
	if (sCheckMark == nil)
		sCheckMark = [[NSInterfacePartAttributedString alloc] initWithFACharacter:0xF00C];
	return sCheckMark;
}

-(NSSize)menuItemSeparatorSize {
	return NSMakeSize(0,9);
}

-(Margins)menuItemBranchArrowMargins {
	Margins result = [self menuItemTextMargins];
	
	result.left = BRANCH_ARROW_LEFT_MARGIN;
	result.right = BRANCH_ARROW_RIGHT_MARGIN;
	
	return result;
}

-(NSSize)menuItemBranchArrowSize {
   return NSMakeSize(5,9);
}

-(NSSize)menuItemCheckMarkSize {
	NSSize result = [[self checkMark] size];
	
	return result;
}

-(Margins)menuItemGutterMargins {
	Margins result;
	
	result.left = 0;
	result.right = 0;
	result.top = TITLE_TOP_MARGIN;
	result.bottom = TITLE_BOTTOM_MARGIN;

	return result;
}

-(NSSize)menuItemGutterSize {
	NSSize result = NSZeroSize;
	Margins margins = [self menuItemGutterMargins];
	
	result = [self menuItemCheckMarkSize];
	
	result.height += (margins.top + margins.bottom);
	result.width += (margins.left + margins.right);
	
	return result;
}

-(Margins)menuItemTextMargins {
	Margins result;
	
	result.left = 0;
	result.right = 0;
	result.top = TITLE_TOP_MARGIN;
	result.bottom = TITLE_BOTTOM_MARGIN;
	
	return result;
}

-(NSSize)menuItemTextSize:(NSString *)title {
	NSSize result = NSZeroSize;
	Margins margins = [self menuItemTextMargins];
	
	result = [title sizeWithAttributes:sNormalMenuTextAttributes];
	
	result.height += (margins.top + margins.bottom);
	result.width += (margins.left + margins.right);
	
	return result;
}

-(NSSize)menuItemAttributedTextSize:(NSAttributedString *)title {
	NSSize result = NSZeroSize;
	Margins margins = [self menuItemTextMargins];
	
	result = [title size];
	
	result.height += (margins.top + margins.bottom);
	result.width += (margins.left + margins.right);
	
	return result;
}

-(float)menuBarHeight
{
    return 0.0;
#if 0
	NSDictionary *attributes=[NSDictionary dictionaryWithObjectsAndKeys:
							  [NSFont menuFontOfSize:0],NSFontAttributeName,nil];
	float         result=[@"Menu" sizeWithAttributes:attributes].height;
	
	result+=2; // border top/bottom margin
	result+=4; // border
	result+=1; // sunken title baseline
	
	return result;
#endif
}

-(float)menuItemGutterGap
{
	return 0;
}

-(void)drawMenuSeparatorInRect:(NSRect)rect 
{
	NSPoint point = NSMakePoint(rect.origin.x + 1, rect.origin.y + 3);
	float   width = rect.size.width - 2;
	
	[[NSColor grayColor] setFill];
	NSRectFill(NSMakeRect(point.x,point.y,width,1));
	[[NSColor whiteColor] setFill];
	NSRectFill(NSMakeRect(point.x,point.y+1,width,1));
}

-(void)drawMenuGutterInRect:(NSRect)rect
{
	// Nothing to do.
}

-(void)drawMenuItemText:(NSString *)string inRect:(NSRect)rect enabled:(BOOL)enabled selected:(BOOL)selected
{
    // Ensure we have enough width - fractional widths give float comparison trouble
    rect.size.width = ceilf(rect.size.width);
    
	Margins margins=[self menuItemTextMargins];
	
	rect.origin.x += margins.left;
	rect.origin.y += margins.top;
	rect.size.width -= (margins.left + margins.right);
	rect.size.height -= (margins.top + margins.bottom);
	
	if (enabled)
	{
		if (selected)
		{
			[string drawInRect:rect withAttributes:sSelectedMenuTextAttributes];
		}
		else
		{
			[string drawInRect:rect withAttributes:sNormalMenuTextAttributes];
		}
	}
	else
	{
		if (!selected)
		{
			NSRect offsetRect = rect;
			offsetRect.origin.x += 1;
			offsetRect.origin.y += 1;
			[string drawInRect:offsetRect withAttributes:sDimmedMenuTextShadowAttributes];
		}
		[string drawInRect:rect withAttributes:sDimmedMenuTextAttributes];
	}
}

-(void)drawAttributedMenuItemText:(NSAttributedString *)string inRect:(NSRect)rect enabled:(BOOL)enabled selected:(BOOL)selected
{
    // Ensure we have enough width - fractional widths give float comparison trouble
    rect.size.width = ceilf(rect.size.width);

	NSMutableAttributedString* mutableString = [string mutableCopy];
	
	Margins margins=[self menuItemTextMargins];
	
	rect.origin.x += margins.left;
	rect.origin.y += margins.top;
	rect.size.width -= (margins.left + margins.right);
	rect.size.height -= (margins.top + margins.bottom);
	
	NSRange range = NSMakeRange(0, [string length]);
	
	if (enabled)
	{
		if (!selected) {
			[mutableString addAttributes: [NSDictionary dictionaryWithObject: [NSColor menuItemTextColor] forKey: NSForegroundColorAttributeName] range: range];
		}
		[mutableString drawInRect:rect];
	}
	else
	{
		if (!selected)
		{
			[mutableString addAttributes: [NSDictionary dictionaryWithObject: [NSColor grayColor] forKey: NSForegroundColorAttributeName] range: range];
			NSRect offsetRect = rect;
			offsetRect.origin.x += 1;
			offsetRect.origin.y += 1;
			[mutableString drawInRect:offsetRect];
		}
		[mutableString drawInRect:rect];
	}
}

-(void)drawMenuCheckmarkInRect:(NSRect)rect enabled:(BOOL)enabled selected:(BOOL)selected {
	NSColor                         *color;
	NSInterfacePartAttributedString *checkMark;
	Margins                         margins=[self menuItemTextMargins];
	
	if (enabled)
		color = selected ? [NSColor selectedControlTextColor] : [NSColor menuItemTextColor];
	else
		color = [NSColor disabledControlTextColor];
	
	checkMark = [[NSInterfacePartAttributedString alloc] initWithCharacter:0xF00C
																  fontName:@"Font Awesome 5 Free-Solid"
																 pointSize:10
																	 color:color];
	rect.origin.x += margins.left;
	rect.origin.y += margins.top;

	[checkMark drawAtPoint:rect.origin];
}


-(void)drawMenuBranchArrowInRect:(NSRect)rect enabled:(BOOL)enabled selected:(BOOL)selected {
	CGContextRef context = [[NSGraphicsContext currentContext] graphicsPort];
	NSColor      *color;
	Margins      margins=[self menuItemBranchArrowMargins];
	NSRect       themeRect = rect;
	
	if (enabled)
		color = selected ? [NSColor selectedControlTextColor] : [NSColor menuItemTextColor];
	else
		color = [NSColor disabledControlTextColor];
	
	[color set];
	
	themeRect.origin.x += margins.left;
	themeRect.origin.y += margins.top;
	themeRect.size.width -= (margins.left + margins.right);
	themeRect.size.height -= (margins.top + margins.bottom);
	
	CGContextBeginPath(context);
	CGContextMoveToPoint(context,NSMinX(themeRect),NSMaxY(themeRect));
	CGContextAddLineToPoint(context,NSMaxX(themeRect),NSMidY(themeRect));
	CGContextAddLineToPoint(context,NSMinX(themeRect),NSMinY(themeRect));
	CGContextClosePath(context);
	CGContextFillPath(context);
}

-(void)drawMenuSelectionInRect:(NSRect)rect enabled:(BOOL)enabled
{
	if (enabled)
	{
		[[NSColor selectedMenuItemColor] setFill];
		NSRectFill(rect);
	}
}

-(void)drawMenuWindowBackgroundInRect:(NSRect)rect {
    [[NSColor menuBackgroundColor] setFill];
	NSRectFill(rect);
	[[NSColor windowFrameColor] set];
	NSFrameRect(rect);
}

-(void)drawMenuBarItemBorderInRect:(NSRect)rect hover:(BOOL)hovering selected:(BOOL)selected
{
	if (selected || hovering)
	{
		[[NSColor selectedMenuItemColor] setFill];
		NSRectFill(rect);
	}
}

-(void)drawMenuBarBackgroundInRect:(NSRect)rect {
	[[NSColor mainMenuBarColor] setFill];
	NSRectFill(rect);
}

@end

@implementation NSGraphicsStyle (NSButton)

-(void)drawUnborderedButtonInRect:(NSRect)rect defaulted:(BOOL)defaulted {
   if(defaulted){
    [[NSColor blackColor] setFill];
    NSRectFill(rect);
   }
}

-(void)drawPushButtonNormalInRect:(NSRect)rect defaulted:(BOOL)defaulted {
   if(defaulted){
    [[NSColor blackColor] setFill];
    NSRectFill(rect);
    rect = NSInsetRect(rect,1,1);
   }

   NSDrawButton(rect,rect);
}

-(void)drawPushButtonPressedInRect:(NSRect)rect {
   NSInterfaceDrawDepressedButton(rect,rect);
}

-(void)drawPushButtonHighlightedInRect:(NSRect)rect {
   NSInterfaceDrawHighlightedButton(rect,rect);
}

-(NSSize)sizeOfButtonImage:(NSImage *)image enabled:(BOOL)enabled mixed:(BOOL)mixed {
   return [image size];
}

-(void)drawButtonImage:(NSImage *)image inRect:(NSRect)rect enabled:(BOOL)enabled mixed:(BOOL)mixed {
   float fraction=enabled?1.0:0.5;
   
   [image drawInRect:rect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:fraction];
}

@end

@implementation NSGraphicsStyle (NSBrowser)

-(void)drawBrowserTitleBackgroundInRect:(NSRect)rect {
   NSInterfaceDrawBrowserHeader(rect,rect);
}

-(void)drawBrowserHorizontalScrollerWellInRect:(NSRect)rect clipRect:(NSRect)clipRect {
   NSDrawGrayBezel(rect,clipRect);
}

@end

@implementation NSGraphicsStyle (NSColorWell)

-(NSRect)drawColorWellBorderInRect:(NSRect)rect enabled:(BOOL)enabled bordered:(BOOL)bordered active:(BOOL)active {
   if(bordered){
    if(active)
     NSInterfaceDrawHighlightedButton(rect,rect);
    else
     NSInterfaceDrawButton(rect,rect);
    
    rect=NSInsetRect(rect,6,6);
     
    if(enabled)
     NSDrawGrayBezel(rect,rect);
   }
   else {
    if(enabled){
     NSDrawGrayBezel(rect,rect);
    }
   }
   
   return NSInsetRect(rect,2,2);
}

@end

@implementation NSGraphicsStyle (NSPopUpButton)

-(void)drawPopUpButtonWindowBackgroundInRect:(NSRect)rect {
	[[NSColor menuBackgroundColor] setFill];
	NSRectFill(rect);
	[[NSColor blackColor] setStroke];
	NSFrameRect(rect);
}

@end

@implementation NSGraphicsStyle (NSOutlineView)

-(void)drawOutlineViewGridInRect:(NSRect)rect {
   NSInterfaceDrawOutlineGrid(rect,NSCurrentGraphicsPort());
}

@end

@implementation NSGraphicsStyle (NSProgressIndicator)

-(NSRect)drawProgressIndicatorBackground:(NSRect)rect clipRect:(NSRect)clipRect bezeled:(BOOL)bezeled {
   if(bezeled){
    NSInterfaceDrawProgressIndicatorBezel(rect,clipRect);
    return NSInsetRect(rect,2,2);
   }
   else {
    [[[_view window] backgroundColor] setFill];
    NSRectFill(rect);
    return rect;
   }
}

-(void)drawProgressIndicatorChunk:(NSRect)rect {
   [[NSColor selectedControlColor] setFill];
   NSRectFill(rect);
}

// rough estimates
#define BLOCK_WIDTH	8.0
#define BLOCK_SPACING	2.0

-(void)drawProgressIndicatorIndeterminate:(NSRect)rect clipRect:(NSRect)clipRect bezeled:(BOOL)bezeled animation:(double)animation {
   if(bezeled)
    rect=[self drawProgressIndicatorBackground:rect clipRect:clipRect bezeled:bezeled];

    NSRect progressRect = rect;
    NSRect blockRect = progressRect;
    int numBlocks;

    numBlocks = (animation * progressRect.size.width)/(BLOCK_WIDTH + BLOCK_SPACING);

    if (numBlocks > 0)
        numBlocks++;

    while (numBlocks-->=0) {
        blockRect.size.width = BLOCK_WIDTH;

        if (NSMaxX(blockRect) > NSMaxX(progressRect))
            blockRect.size.width -= (NSMaxX(blockRect) - NSMaxX(progressRect));

        if (blockRect.size.width > 0) {
            if (numBlocks < 2) {
               [self drawProgressIndicatorChunk:blockRect];
            }
            blockRect.origin.x += BLOCK_WIDTH + BLOCK_SPACING;
        }
    }
}

-(void)drawProgressIndicatorDeterminate:(NSRect)rect clipRect:(NSRect)clipRect bezeled:(BOOL)bezeled value:(double)value {
   if(bezeled)
    rect=[self drawProgressIndicatorBackground:rect clipRect:clipRect bezeled:bezeled];

    NSRect progressRect = rect;
    NSRect blockRect = progressRect;
    int numBlocks;
                
    numBlocks = (value * progressRect.size.width)/(BLOCK_WIDTH + BLOCK_SPACING);
    
    if (numBlocks > 0)
        numBlocks++;

    while (numBlocks-->=0) {
        blockRect.size.width = BLOCK_WIDTH;

        if (NSMaxX(blockRect) > NSMaxX(progressRect))
            blockRect.size.width -= (NSMaxX(blockRect) - NSMaxX(progressRect));

        if (blockRect.size.width > 0) {
            [self drawProgressIndicatorChunk:blockRect];
            blockRect.origin.x += BLOCK_WIDTH + BLOCK_SPACING;
        }
    }
}

@end

@implementation NSGraphicsStyle (NSScroller)

-(void)drawScrollerButtonInRect:(NSRect)rect enabled:(BOOL)enabled pressed:(BOOL)pressed vertical:(BOOL)vertical upOrLeft:(BOOL)upOrLeft {
   unichar code=vertical?(upOrLeft?0xF0D8:0xF0D7):(upOrLeft?0xF0D9:0xF0DA);
   Class   class;
   NSInterfacePart *arrow;
   
   if(enabled)
    class=[NSInterfacePartAttributedString class];
   else
    class=[NSInterfacePartDisabledAttributedString class];

   arrow=[[[class alloc] initWithFACharacter:code] autorelease];
   
   if(!NSIsEmptyRect(rect)){
    NSSize arrowSize=[arrow size];

    if(pressed)
     NSInterfaceDrawDepressedScrollerButton(rect,rect);
    else
     NSInterfaceDrawScrollerButton(rect,rect);

    if(rect.size.height>8 && rect.size.width>8){
     NSPoint point=rect.origin;

     point.x+=floor((rect.size.width-arrowSize.width)/2);
     point.y+=floor((rect.size.height-arrowSize.height)/2);
     [arrow drawAtPoint:point];
    }
   }
}

-(void)drawScrollerKnobInRect:(NSRect)rect vertical:(BOOL)vertical highlight:(BOOL)highlight {
   NSDrawButton(rect,rect);
}

-(void)drawScrollerTrackInRect:(NSRect)rect vertical:(BOOL)vertical upOrLeft:(BOOL)upOrLeft {
   [[NSColor colorWithCalibratedWhite:0.9 alpha:1] setFill];
   NSRectFill(rect);
}

-(void)drawScrollerTrackInRect:(NSRect)rect vertical:(BOOL)vertical {
   [self drawScrollerTrackInRect:rect vertical:vertical upOrLeft:NO];
}

@end

@implementation NSGraphicsStyle (NSSlider)

-(NSSize)sliderKnobSizeForControlSize:(NSControlSize)controlSize {

   switch(controlSize){
    default:
    case NSRegularControlSize: // aqua is 17x19
      return NSMakeSize(12,15); // this is Windows specific, uxtheme part size request was failing, hardcoded, sigh
     
    case NSSmallControlSize: // aqua is 13x15
      return NSMakeSize(9,13);
     
    case NSMiniControlSize: // aqua is 11x11
      return NSMakeSize(11,11);
   }
   
}

-(void)drawSliderKnobInRect:(NSRect)rect vertical:(BOOL)vertical highlighted:(BOOL)highlighted hasTickMarks:(BOOL)hasTickMarks tickMarkPosition:(NSTickMarkPosition)tickMarkPosition {
   NSDrawButton(rect,rect);

   if(highlighted) {
    [[NSColor whiteColor] setFill];
    NSRectFill(NSInsetRect(rect,1,1));
   }
}

-(void)drawSliderTrackInRect:(NSRect)rect vertical:(BOOL)vertical hasTickMarks:(BOOL)hasTickMarks {
   NSRect groove=rect;

   if(vertical){
    groove.size.width=4;
    groove.origin.x=floor(rect.origin.x+(rect.size.width-4)/2);
   }
   else {
    groove.size.height=4;
    groove.origin.y=floor(rect.origin.y+(rect.size.height-4)/2);
   }

   NSDrawGrayBezel(groove, rect);
}

-(void)drawSliderTickInRect:(NSRect)rect {
   [[NSColor blackColor] setFill];
   NSRectFill(rect);
}

@end

@implementation NSGraphicsStyle (NSStepper)

-(void)drawStepperButtonInRect:(NSRect)rect clipRect:(NSRect)clipRect enabled:(BOOL)enabled highlighted:(BOOL)highlighted upNotDown:(BOOL)upNotDown {
   unichar         code=upNotDown?0xF0D8:0xF0D7;
   NSInterfacePart *arrow;
     
   if(enabled)
    arrow=[[[NSInterfacePartAttributedString alloc] initWithFACharacter:code] autorelease];
   else
    arrow=[[[NSInterfacePartDisabledAttributedString alloc] initWithFACharacter:code] autorelease];
      
   if(highlighted)
    NSDrawWhiteBezel(rect,clipRect);
   else
    NSDrawButton(rect,clipRect);
      
   rect.origin.x += rect.size.width/2;
   rect.origin.x -= [arrow size].width/2;
   rect.origin.y += rect.size.height/2;
   rect.origin.y -= [arrow size].height/2;
   [arrow drawAtPoint:rect.origin];
}

@end

@implementation NSGraphicsStyle (NSTableView)

-(void)drawTableViewHeaderInRect:(NSRect)rect highlighted:(BOOL)highlighted {
   NSDrawButton(rect, rect);
    
   if(highlighted){
    [[NSColor darkGrayColor] setFill];
    NSRectFill(NSInsetRect(rect,2,2));
   }
}

-(void)drawTableViewCornerInRect:(NSRect)rect {
   NSDrawButton(rect,rect);
}

@end

@implementation NSGraphicsStyle (NSBox)

-(void)drawBoxWithLineInRect:(NSRect)rect {
   [[NSColor blackColor] setStroke];
   NSFrameRect(rect);
}

-(void)drawBoxWithBezelInRect:(NSRect)rect clipRect:(NSRect)clipRect {
   NSDrawGrayBezel(rect,clipRect);
}

-(void)drawBoxWithGrooveInRect:(NSRect)rect clipRect:(NSRect)clipRect {
   NSDrawGroove(rect,clipRect);
}

@end

@implementation NSGraphicsStyle (NSComboBox)

-(void)drawComboBoxButtonInRect:(NSRect)rect enabled:(BOOL)enabled bordered:(BOOL)bordered pressed:(BOOL)pressed {
   NSImage *image=[NSImage imageNamed:@"NSComboBoxCellDown"];
   NSSize   imageSize=[image size];
   NSRect   imageRect;
   
   if(pressed)
    NSInterfaceDrawDepressedButton(rect,rect);
   else
    NSDrawButton(rect,rect);
    
   imageRect.origin.x=rect.origin.x+(rect.size.width-imageSize.width)/2+(pressed?1:0);
   imageRect.origin.y=rect.origin.y+(rect.size.height-imageSize.height)/2+(pressed?-1:0);
   imageRect.size=imageSize;
   
   [self drawButtonImage:image inRect:imageRect enabled:enabled mixed:NO];
}

@end

@implementation NSGraphicsStyle (NSTabView)

-(void)drawTabInRect:(NSRect)rect clipRect:(NSRect)clipRect color:(NSColor *)color selected:(BOOL)selected {
    NSRect originalRect=rect;
    NSRect rects[8];
    NSColor *colors[8];
    int i;

    if(selected){
       rect.origin.x-=2;
       rect.size.width+=3;
    }
    else {
     rect.size.height-=2;
    }

    for(i=0; i<8; i++)
        rects[i]=rect;

    colors[0]=[NSColor controlColor];
    if(selected){
     rects[0].origin.y-=1;
     rects[0].size.height+=1;
    }
    colors[1]=color;
    rects[1]=NSInsetRect(rect,1,1);
    colors[2]=[NSColor whiteColor];
    rects[2].size.width=1;
    rects[2].size.height-=2;
    if(selected){
     rects[2].origin.y-=1;
     rects[2].size.height+=1;
    }
    colors[3]=[NSColor whiteColor];
    rects[3].origin.x+=1;
    rects[3].origin.y+=rect.size.height-2;
    rects[3].size.width=1;
    rects[3].size.height=1;
    colors[4]=[NSColor whiteColor];
    rects[4].origin.x+=2;
    rects[4].origin.y+=rect.size.height-1;
    rects[4].size.width=rect.size.width-4;
    rects[4].size.height=1;
    colors[5]=[NSColor blackColor];
    rects[5].origin.x+=rect.size.width-2;
    rects[5].origin.y+=rect.size.height-2;
    rects[5].size.width=1;
    rects[5].size.height=1;
    colors[6]=[NSColor controlShadowColor];
    rects[6].origin.x+=rect.size.width-2;
    rects[6].size.width=1;
    rects[6].size.height-=2;
    colors[7]=[NSColor blackColor];
    rects[7].origin.x+=rect.size.width-1;
    rects[7].size.width=1;
    rects[7].size.height-=2;
    if(selected){
     rects[7].origin.y-=1;
     rects[7].size.height+=1;
    }

    for(i=0; i<8; i++) {
        [colors[i] setFill];
        NSRectFill(rects[i]);
    }
    
    if(selected){ // cleanup
     NSRect erase=originalRect;
     
     [[NSColor controlColor] setFill];
     erase.size.height=2;
     erase=NSInsetRect(erase,1,0);
     NSRectFill(erase);
    }
}

-(void)drawTabPaneInRect:(NSRect)rect {
   NSDrawButton(rect,rect);
}

-(void)drawTabViewBackgroundInRect:(NSRect)rect {
   // do nothing
}

@end

@implementation NSGraphicsStyle (NSTextField)

-(void)drawTextFieldBorderInRect:(NSRect)rect bezeledNotLine:(BOOL)bezeledNotLine {
   if(bezeledNotLine)
    NSDrawWhiteBezel(rect,rect);
   else {
    [[NSColor blackColor] setStroke];
    NSFrameRect(rect);
   }
}

-(void)drawTextViewInsertionPointInRect:(NSRect)rect color:(NSColor *)color {
   [color setFill];
   NSRectFill(rect);
}

@end

@implementation NSView(NSGraphicsStyle)

-(NSGraphicsStyle *)graphicsStyle {
   return [[[NSGraphicsStyle alloc] initWithView:self] autorelease];
}

@end
