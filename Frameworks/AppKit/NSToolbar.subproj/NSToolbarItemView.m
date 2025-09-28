/* Copyright (c) 2009 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import "NSToolbarItemView.h"
#import <AppKit/NSToolbar.h>
#import <AppKit/NSToolbarItem.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSStringDrawer.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSGradient.h>
#import <AppKit/NSGraphicsContext.h>

@interface NSToolbarItem(private)
-(void)drawInRect:(NSRect)bounds highlighted:(BOOL)highlighted;
-(NSSize)_labelSize;
@end

@interface NSToolbar (private)
- (void)didSelectToolbarItem:(NSString*)identifier;
@end

@implementation NSToolbarItemView

-initWithFrame:(NSRect)frame {
   [super initWithFrame:frame];
   _toolbarItem=nil;
   return self;
}

-(void)dealloc {
   _toolbarItem=nil;
   [super dealloc];
}

-(void)setToolbarItem:(NSToolbarItem *)item {
   _toolbarItem=item;
}

-(void)setSubview:(NSView *)view {
   while([[self subviews] count])
    [[[self subviews] lastObject] removeFromSuperview];
   
   if(view!=nil)
    [self addSubview:view];
}

-(NSRect)availableRectForSubview
{
    NSRect rect=[self bounds];
    CGFloat labelHeight=[_toolbarItem _labelSize].height;
    rect.size.height -= labelHeight;
    rect.origin.y += labelHeight + 4; // 4 is Cocotron magic padding (see NSToolbarItem drawInRect:
    return rect;
}

-(void)setFrame:(NSRect)frame {
    [super setFrame:frame];
    if ([_toolbarItem view] != nil) {
        // Fix up the item viewWe 
        NSView *view = [_toolbarItem view];
        // Center the subview within the bounds
        NSRect availableRect = [self availableRectForSubview];
        NSRect viewRect = [view frame];
        
        // Center the subview within the available bounds
        viewRect.origin.x=NSMinX(availableRect) + (NSWidth(availableRect) - NSWidth(viewRect)) / 2.f;
        // Align top
        viewRect.origin.y=NSMinY(availableRect);
        [view setFrame: viewRect];
    }
}
-(NSView *)view {
   return [[self subviews] lastObject];
}

typedef struct {
 CGFloat _C0[4];
 CGFloat _C1[4];
} gradientColors;

static void evaluate(void *info,const float *in, float *output) {
   float         x=in[0];
   gradientColors *colors=info;
   int           i;
   
    for(i=0;i<4;i++)
     output[i]=colors->_C0[i]+x*(colors->_C1[i]-colors->_C0[i]);
}

const float kFillGrayLevel = 0.75;
const float kEdgeGrayLevel = 0.5;
const float kEdgeThickness = 2.f;

-(void)drawRect:(NSRect)rect {
   if([[_toolbarItem itemIdentifier] isEqual:[[_toolbarItem toolbar] selectedItemIdentifier]]){

       NSArray *colors = [NSArray arrayWithObjects: [NSColor colorWithCalibratedWhite: kFillGrayLevel alpha: 0],
                          [NSColor colorWithCalibratedWhite: kFillGrayLevel alpha: 0.5],
                          [NSColor colorWithCalibratedWhite: kFillGrayLevel alpha: 1],
                          [NSColor colorWithCalibratedWhite: kFillGrayLevel alpha: 0.0],
                          nil];
       NSGradient *gradient = [[[NSGradient alloc] initWithColors: colors] autorelease];
       [gradient drawInRect: [self bounds] angle: 270];
       
       // fill the edges
       colors = [NSArray arrayWithObjects: [NSColor colorWithCalibratedWhite: kEdgeGrayLevel alpha: 0],
                          [NSColor colorWithCalibratedWhite: kEdgeGrayLevel alpha: 1],
                          [NSColor colorWithCalibratedWhite: kEdgeGrayLevel alpha: 0.0],
                          nil];
       gradient = [[[NSGradient alloc] initWithColors: colors] autorelease];
       NSRect edgeRect = [self bounds];
       edgeRect.size.width = kEdgeThickness;
       [gradient drawInRect: edgeRect angle: 270];
       edgeRect.origin.x = NSMaxX([self bounds]) - kEdgeThickness;
       [gradient drawInRect: edgeRect angle: 270];
   }
   [_toolbarItem drawInRect:[self bounds] highlighted:_isHighlighted];  
}

-(void)mouseDown:(NSEvent *)event {
   BOOL sendAction=NO;

   if(![_toolbarItem isEnabled])
    return;

   do {
    NSPoint point=[self convertPoint:[event locationInWindow] fromView:nil];

    _isHighlighted=NSMouseInRect(point,[self bounds],[self isFlipped]);

    [self setNeedsDisplay:YES];
    event=[[self window] nextEventMatchingMask:NSLeftMouseUpMask| NSLeftMouseDraggedMask];
   }while([event type]!=NSLeftMouseUp);

   if(_isHighlighted){
    _isHighlighted=NO;
       [[_toolbarItem toolbar] didSelectToolbarItem: [_toolbarItem itemIdentifier]];
    [NSApp sendAction:[_toolbarItem action] to:[_toolbarItem target] from:_toolbarItem];
    [self setNeedsDisplay:YES];
   }
}

@end

