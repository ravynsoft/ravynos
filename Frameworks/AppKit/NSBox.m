/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSBox.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSBezierPath.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSStringDrawing.h>
#import <AppKit/NSParagraphStyle.h>
#import <AppKit/NSStringDrawer.h>
#import <AppKit/NSCell.h>
#import <AppKit/NSTextFieldCell.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSGraphicsStyle.h>
#import <AppKit/NSRaise.h>


@implementation NSBox

-(void)encodeWithCoder:(NSCoder *)coder {
   [super encodeWithCoder:coder];
   [coder encodeInt:_borderType forKey:@"NSBox borderType"];
   [coder encodeInt:_titlePosition forKey:@"NSBox titlePosition"];
   [coder encodeSize:_contentViewMargins forKey:@"NSBox contentViewMargins"];
   [coder encodeObject:_titleCell forKey:@"NSBox titleCell"];
}

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];

   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    
    _boxType=[keyed decodeIntForKey:@"NSBoxType"];
    _borderType=[keyed decodeIntForKey:@"NSBorderType"];
    _titlePosition=[keyed decodeIntForKey:@"NSTitlePosition"];
    _contentViewMargins=[keyed decodeSizeForKey:@"NSOffsets"];
    _titleCell=[[keyed decodeObjectForKey:@"NSTitleCell"] retain];
    // There is a key NSTransparent, but as far as I can tell it is always NO
    _isTransparent=[keyed decodeBoolForKey:@"NSFullyTransparent"];
    
    [[_subviews lastObject] setAutoresizingMask: NSViewWidthSizable| NSViewHeightSizable];
    [[_subviews lastObject] setAutoresizesSubviews:YES];
	   
    if (_boxType == NSBoxCustom)
	{
		id obj;
		_customData = [[NSMutableDictionary alloc] init];
		
		obj = [keyed decodeObjectForKey:@"NSBorderWidth2"];
		if (obj == nil) obj = [NSNumber numberWithDouble:1];
		[_customData setObject:obj forKey:@"NSBorderWidth2"];
		
		obj = [keyed decodeObjectForKey:@"NSCornerRadius2"];
		if (obj == nil) obj = [NSNumber numberWithDouble:0];

		[_customData setObject:obj forKey:@"NSCornerRadius2"];
		
		obj = [keyed decodeObjectForKey:@"NSBorderColor2"];
		if (obj == nil) obj = [NSColor colorWithCalibratedWhite:0.000 alpha:0.420];
		[_customData setObject:obj forKey:@"NSBorderColor2"];
		
		obj = [keyed decodeObjectForKey:@"NSFillColor2"];
		if (obj == nil) obj = [NSColor colorWithCalibratedWhite:0.000 alpha:0.000];
		[_customData setObject:obj forKey:@"NSFillColor2"];
   }
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"%@ can not initWithCoder:%@",isa,[coder class]];
   }
   return self;
}

-initWithFrame:(NSRect) frame {
  NSView *contentview;

  [super initWithFrame: frame];
   
   _isTransparent = YES;
   _titleCell = [[NSTextFieldCell alloc] initTextCell:@""];
   _borderType = NSLineBorder;
   _titlePosition = NSAboveTop;
   _customData = [[NSMutableDictionary alloc] init];

  contentview = [[NSView alloc] initWithFrame:NSMakeRect(0,0,NSWidth(frame),NSHeight(frame))];
  [contentview  setAutoresizingMask: NSViewWidthSizable| NSViewHeightSizable];
  [self addSubview: contentview];
  [contentview release];
  [self setAutoresizesSubviews:YES];
  return self;
}
  
-(void)dealloc {
   [_customData release];
   [_titleCell release];
   [super dealloc];
}

-(NSBoxType)boxType {
   return _boxType;
}

-(NSBorderType)borderType {
   return _borderType;
}

-(NSString *)title {
   return [_titleCell stringValue];
}

-(NSFont *)titleFont {
   return [_titleCell font];
}

-contentView {
   return [[self subviews] lastObject];
}

-(NSSize)contentViewMargins {
   return _contentViewMargins;
}

-(NSTitlePosition)titlePosition {
   return _titlePosition;
}

-(BOOL)isTransparent {
   return _isTransparent;
}

-(void)setBoxType:(NSBoxType)value {
   _boxType=value;
   [self setNeedsDisplay:YES];
}

-(void)setBorderType:(NSBorderType)value {
   _borderType=value;
   [self setNeedsDisplay:YES];
}

-(void)setTitle:(NSString *)title {
   [_titleCell setStringValue:title];
   [self setNeedsDisplay:YES];
}

-(void)setTitleFont:(NSFont *)font {
   [_titleCell setFont:font];
}

-(void)setContentView:(NSView *)view {
   if(![[self subviews] containsObject:view]){
    [[self subviews] makeObjectsPerformSelector:@selector(removeFromSuperview)];
    // FIX, adjust size
    [self addSubview:view];
   }
}

-(void)setContentViewMargins:(NSSize)value {
   _contentViewMargins=value;
   [self setNeedsDisplay:YES];
}

-(void)setTitlePosition:(NSTitlePosition)value {
   _titlePosition=value;
   [self setNeedsDisplay:YES];
}

-(void)setTransparent:(BOOL)value {
   _isTransparent=value;
   [self setNeedsDisplay:YES];
}

-(void)setTitleWithMnemonic:(NSString *)value {
   NSUnimplementedMethod();
}

-(NSAttributedString *)_attributedTitle {
   NSMutableDictionary *attributes=[NSMutableDictionary dictionary];
   NSMutableParagraphStyle *paraStyle=[[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
   NSFont              *font=[_titleCell font];

   if(font!=nil)
    [attributes setObject:font forKey:NSFontAttributeName];

   [paraStyle setLineBreakMode:NSLineBreakByClipping];
   [paraStyle setAlignment:NSLeftTextAlignment];
   [attributes setObject:paraStyle forKey:NSParagraphStyleAttributeName];

   [attributes setObject:[NSColor controlTextColor]
                  forKey: NSForegroundColorAttributeName];

   [attributes setObject:[NSColor windowBackgroundColor]
                  forKey:NSBackgroundColorAttributeName];

   return [[[NSAttributedString alloc] initWithString:[_titleCell stringValue] attributes:attributes] autorelease];
}

#define TEXTGAP 4

-(NSRect)titleRect {
   // Obtain the size the title cell prefers
   NSSize size = [_titleCell cellSize]; 
   NSRect bounds=[self bounds];
   NSRect result=NSZeroRect;
   
   result.origin.x=10+TEXTGAP;
   result.size.height=ceil(size.height);
    
    //result.size.width=ceil(size.width); // // NSTextField cell must be bugged we get too low values for the width here
    result.size.width = bounds.size.width - result.origin.x;  // use the whole width until the text field cell size is fixed
    
   switch(_titlePosition){

    case NSNoTitle:
     break;

    case NSAboveTop:
     result.origin.y=bounds.size.height-result.size.height;
     break;

    case NSAtTop:
     result.origin.y=bounds.size.height-result.size.height;
     break;

    case NSBelowTop:
     result.origin.y=bounds.size.height-(result.size.height+TEXTGAP);
     break;

    case NSAboveBottom:
     result.origin.y=TEXTGAP;
     break;

    case NSAtBottom:
     break;

    case NSBelowBottom:
     result.origin.y=0;
     break;
   }


   return result;
}

-(NSRect)borderRect {
   NSUnimplementedMethod();
   return NSMakeRect(0,0,0,0);
}

-titleCell {
   return _titleCell;
}

-(void)setFrameFromContentFrame:(NSRect)content {
// FIX, adjust content frame size to accomodate border/title
   [self setFrame:content];
}

-(void)sizeToFit {
   NSUnimplementedMethod();
}

-(void)drawRect:(NSRect)rect {
   NSRect              grooveRect=_bounds;
   NSRect              titleRect=[self titleRect];
   BOOL                drawTitle=YES;

   if([self isTransparent])
    return;
    
   switch(_titlePosition){

    case NSNoTitle:
     drawTitle=NO;
     break;

    case NSAboveTop:
     grooveRect.size.height-=titleRect.size.height+TEXTGAP;
     break;

    case NSAtTop:
     grooveRect.size.height-=floor(titleRect.size.height/2);
     break;

    case NSBelowTop:
     break;

    case NSAboveBottom:
     break;

    case NSAtBottom:
     grooveRect.origin.y+=floor(titleRect.size.height/2);
     grooveRect.size.height-=floor(titleRect.size.height/2);
     break;

    case NSBelowBottom:
     grooveRect.origin.y+=titleRect.size.height+TEXTGAP;
     grooveRect.size.height-=titleRect.size.height+TEXTGAP;
     break;
   }

	if (_boxType != NSBoxSeparator) {
		// Separator are transparent except for drawing a line
		[[NSColor controlColor] setFill];
		NSRectFill(grooveRect);
	}

	if (_boxType == NSBoxCustom){
		
		// Ignoring corner radius for now.
		[[self fillColor] set];
		NSRectFill(rect);
		
		if (_borderType != NSNoBorder)
		{
			[[self borderColor] set];
			NSFrameRectWithWidth(_bounds,[self borderWidth]);
		}
	} else if (_boxType == NSBoxSeparator) {
		// These are just the simple divider lines
		[[NSColor grayColor] set];
		NSBezierPath* line = [NSBezierPath bezierPath];
		if (NSWidth(grooveRect) > NSHeight(grooveRect)) {
			[line moveToPoint: NSMakePoint(NSMinX(grooveRect), NSMidY(grooveRect))];
			[line lineToPoint: NSMakePoint(NSMaxX(grooveRect), NSMidY(grooveRect))];
		} else {
			[line moveToPoint: NSMakePoint(NSMidX(grooveRect), NSMinY(grooveRect))];
			[line lineToPoint: NSMakePoint(NSMidX(grooveRect), NSMaxY(grooveRect))];
		}
		[line stroke];
	}
	else{
   switch(_borderType){
    case NSNoBorder:
     break;

    case NSLineBorder:
     [[self graphicsStyle] drawBoxWithLineInRect:grooveRect];
     break;

    case NSBezelBorder:
     [[self graphicsStyle] drawBoxWithBezelInRect:grooveRect clipRect:rect];
     break;

    case NSGrooveBorder:
     [[self graphicsStyle] drawBoxWithGrooveInRect:grooveRect clipRect:rect];
     break;
   }
	}
   if(drawTitle){
#if 0
    [[NSColor windowBackgroundColor] setFill];
    titleRect.origin.x-=TEXTGAP;
    titleRect.size.width+=TEXTGAP*2;
    NSRectFill(titleRect);
#endif
    titleRect.origin.x+=TEXTGAP;

	// Ask the cell to draw itself now
	// TODO: Should we be doing some sort of clipping setup here?
    [_titleCell setControlView:self];
    [_titleCell drawWithFrame: titleRect inView: self];
   }
}

- (CGFloat)borderWidth
{
	return [[_customData objectForKey:@"NSBorderWidth2"] doubleValue];
}

- (CGFloat)cornerRadius;
{
	return [[_customData objectForKey:@"NSCornerRadius2"] doubleValue];
}

- (NSColor *)borderColor;
{
   return [_customData objectForKey:@"NSBorderColor2"];
}

- (NSColor *)fillColor;
{
   return [_customData objectForKey:@"NSFillColor2"];
}


- (void)setBorderWidth:(CGFloat)value
{
	[_customData setObject:[NSNumber numberWithDouble:value] forKey:@"NSBorderWidth2"];
}

- (void)setCornerRadius:(CGFloat)value
{
	[_customData setObject:[NSNumber numberWithDouble:value] forKey:@"NSCornerRadius2"];
}

- (void)setBorderColor:(NSColor *)value
{
   [_customData setObject:value forKey:@"NSBorderColor2"];
}

- (void)setFillColor:(NSColor *)value
{
   [_customData setObject:value forKey:@"NSFillColor2"];
}

-(void)updateCell:(NSCell *)cell
{
    [self setNeedsDisplay:YES];
}

@end
