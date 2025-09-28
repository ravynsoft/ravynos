/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSTabViewItem.h>
#import <AppKit/NSTabView.h>
#import <AppKit/NSView.h>
#import <AppKit/NSStringDrawing.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSStringDrawer.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSRaise.h>

// not perfect; needs real ellipsis character, will fail to produce a small enough string if
// char. 1 + @"..." > rect.size.width
NSString *_NSTruncatedStringWithAttributesInRect(NSString *string, NSDictionary *attributes, NSRect rect) {
   unsigned length=[string length];

   while ([string length] > 1 && [string sizeWithAttributes:attributes].width > rect.size.width) {
       string = [string substringWithRange:NSMakeRange(0, [string length]-1)];
   }

   if (length==[string length])
       return string;
   else
       return [string stringByAppendingString:@"..."];
}

@implementation NSTabViewItem

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    
    _identifier=[[keyed decodeObjectForKey:@"NSIdentifier"] retain];
    _label=[[keyed decodeObjectForKey:@"NSLabel"] retain];
    _view=[[keyed decodeObjectForKey:@"NSView"] retain];

    _tabView=[keyed decodeObjectForKey:@"NSTabView"];
    _color=[[keyed decodeObjectForKey:@"NSColor"] retain];
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   }

   return self;
}

-initWithIdentifier:identifier {
   _identifier=[identifier retain];
   _label=@"";
   _view=nil;
   _color=[[NSColor controlColor] retain];
   _state=NSBackgroundTab;
   return self;
}

-(void)dealloc {
   [_identifier release];
   [_label release];
   [_view release];
   [_color release];
   [super dealloc];
}

-identifier {
   return _identifier;
}

-(NSString *)label {
   return _label;
}

-view {
   return _view;
}

-(NSColor *)color {
    return _color;
}

-(NSTabView *)tabView {
    return _tabView;
}

-(NSTabState)tabState {
    return _state;
}

-initialFirstResponder {
    return _initialFirstResponder;
}


-(void)setLabel:(NSString *)label {
   label=[label copy];
   [_label release];
   _label=label;
}

-(void)setView:(NSView *)view {
   view=[view retain];
   [_view release];
   _view=view;

   if (_initialFirstResponder==nil)
       _initialFirstResponder=view;
   [view removeFromSuperview];
   [_tabView _itemViewDidChange:self];
}

-(void)setColor:(NSColor *)color {
    color=[color retain];
    [_color release];
    _color=color;
}

// private; no retain
-(void)setTabView:(NSTabView *)tabView {
    _tabView=tabView;
}

// private
-(void)setTabState:(NSTabState)tabState {
    _state=tabState;
}

-(void)setInitialFirstResponder:responder {
    responder=[responder retain];
    [_initialFirstResponder release];
    _initialFirstResponder=responder;
}   

// This is private Apple API, BGHUDAppKit uses it, override for custom color
-_labelColor {
   return [NSColor blackColor];
}

-(NSDictionary *)labelAttributes {
   return [NSDictionary dictionaryWithObjectsAndKeys:
    [_tabView font],NSFontAttributeName,
    [self _labelColor],NSForegroundColorAttributeName,
    nil];
}

// our tabs draw too closely to the left mragin.
#define ORIGIN_PADDING 2.0
-(void)drawLabel:(BOOL)truncateLabel inRect:(NSRect)rect {
    _lastRect = rect;
    rect.origin.x+=ORIGIN_PADDING;
    if ([self sizeOfLabel:NO].width > rect.size.width && truncateLabel) {
        NSString *truncatedLabel = _NSTruncatedStringWithAttributesInRect(_label, [self labelAttributes], rect);
        [truncatedLabel _clipAndDrawInRect:rect withAttributes:[self labelAttributes]];
    }
    else
        [_label _clipAndDrawInRect:rect withAttributes:[self labelAttributes]];
}

// our tabs look too short.
#define SIZE_PADDING  8.0
-(NSSize)sizeOfLabel:(BOOL)truncateLabel {
    NSSize size = [_label sizeWithAttributes:[self labelAttributes]];

    // make sure that we don't return the uninitialized lastRect
    if (truncateLabel && _lastRect.size.width > 0) {
        NSString *truncatedLabel = _NSTruncatedStringWithAttributesInRect(_label, [self labelAttributes], _lastRect);
        size = [truncatedLabel sizeWithAttributes:[self labelAttributes]];
    }
    size.width+=SIZE_PADDING;

    return size;
}

@end
