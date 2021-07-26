/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSFormCell.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSStringDrawing.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSParagraphStyle.h>
#import <AppKit/NSStringDrawer.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSTextFieldCell.h>
#import <AppKit/NSRaise.h>

@implementation NSFormCell

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];

   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    
    _titleWidth=[keyed decodeFloatForKey:@"NSTitleWidth"];
    _titleCell=[[keyed decodeObjectForKey:@"NSTitleCell"] retain];
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   }
   return self;
}

-copyWithZone:(NSZone *)zone {
    NSFormCell *copy = [super copyWithZone:zone];

    copy->_titleCell=[_titleCell copy];

    return copy;
}

-initTextCell:(NSString *)value {
   [super initTextCell:value];
   NSUnimplementedMethod();
   return self;
}

-(void)dealloc {
   [_titleCell release];
   [super dealloc];
}

-(NSCellType)type {
   return NSTextCellType;
}

-(BOOL)isOpaque {
   if(_titleWidth==0 && [self isBezeled])
    return YES;
   
   return NO;
}

-(float)titleWidth {
   return _titleWidth;
}

-(float)titleWidth:(NSSize)size {
// FIX,wrong
   return _titleWidth;
}

-(NSString *)title {
   return [_titleCell stringValue];
}

-(NSAttributedString *)attributedTitle {
   return [_titleCell attributedStringValue];
}

-(NSFont *)titleFont {
   return [_titleCell font];
}

-(NSTextAlignment)titleAlignment {
   return [_titleCell alignment];
}

-(NSWritingDirection)titleBaseWritingDirection {
   return [_titleCell baseWritingDirection];
}

-(NSString *)placeholderString {
   if([_placeholder isKindOfClass:[NSString class]])
    return _placeholder;

   return nil;
}

-(NSAttributedString *)placeholderAttributedString {
   if([_placeholder isKindOfClass:[NSAttributedString class]])
    return _placeholder;

   return nil;
}

-(void)setTitleWidth:(float)value {
   _titleWidth=value;
}

-(void)setTitle:(NSString *)value {
   [_titleCell setStringValue:value];
}

-(void)setAttributedTitle:(NSAttributedString *)value {
   [_titleCell setAttributedStringValue:value];
}

-(void)setTitleFont:(NSFont *)value {
   [_titleCell setFont:value];
}

-(void)setTitleAlignment:(NSTextAlignment)value {
   [_titleCell setAlignment:value];
}

-(void)setTitleBaseWritingDirection:(NSWritingDirection)value {
   [_titleCell setBaseWritingDirection:value];
}

-(void)setPlaceholderString:(NSString *)value {
   value=[value copy];
   [_placeholder release];
   _placeholder=value;
}

-(void)setPlaceholderAttributedString:(NSAttributedString *)value {
   value=[value copy];
   [_placeholder release];
   _placeholder=value;
}

-(void)setTitleWithMnemonic:(NSString *)value {
// FIX, wrong
   [_titleCell setStringValue:value];
}

-(void)setEnabled:(BOOL)enabled {
   [super setEnabled:enabled];
   [_titleCell setEnabled:enabled];
}

-(NSAttributedString *)attributedStringValue {
   NSMutableDictionary *attributes=[NSMutableDictionary dictionary];
   NSMutableParagraphStyle *paraStyle=[[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
   NSFont              *font=[self font];

   if(font!=nil)
    [attributes setObject:font forKey:NSFontAttributeName];

   [attributes setObject:[self isEnabled]?[NSColor controlTextColor]:[NSColor disabledControlTextColor]
                  forKey:NSForegroundColorAttributeName];

   [attributes setObject:[NSColor whiteColor]
                  forKey:NSBackgroundColorAttributeName];

   [paraStyle setLineBreakMode:_lineBreakMode];
   [paraStyle setAlignment:_textAlignment];
   [attributes setObject:paraStyle forKey:NSParagraphStyleAttributeName];

   return [[[NSAttributedString alloc] initWithString:[self stringValue] attributes:attributes] autorelease];
}

-(void)drawInteriorWithFrame:(NSRect)frame inView:(NSView *)control {
   NSAttributedString *title=[self attributedTitle];
   NSAttributedString *value=[self attributedStringValue];
   NSSize              titleSize=[title size];
   NSRect              titleRect;
   NSRect              valueRect;

   titleRect.origin.x=frame.origin.x;
   titleRect.origin.y=frame.origin.y+floor((frame.size.height-titleSize.height)/2);
   titleRect.size.width=_titleWidth;
   titleRect.size.height=titleSize.height;

   [title _clipAndDrawInRect:titleRect];

   valueRect=frame;
   valueRect.origin.x+=(_titleWidth+1);
   valueRect.size.width-=(_titleWidth+1);

   if([self isBezeled]){
    NSDrawWhiteBezel(valueRect,valueRect);
   }

   valueRect=[self titleRectForBounds:frame];

   [value _clipAndDrawInRect:valueRect];
}

-(void)drawWithFrame:(NSRect)frame inView:(NSView *)control {
   _controlView=control;

   [self drawInteriorWithFrame:frame inView:control];
}

-(NSRect)titleRectForBounds:(NSRect)rect {
   rect.origin.x+=(_titleWidth+1);
   rect.size.width-=(_titleWidth+1);

   if([self isBezeled])
    rect=NSInsetRect(rect,3,3);

   return rect;
}

@end
