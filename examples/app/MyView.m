#import "MyView.h"
#import <CoreGraphics/CGContext.h>

@implementation MyView

-(void)setText:(NSString *)string {
    _text = string;
}

-(void)setFont:(NSFont *)font {
    _font = font;
}

-(void)drawRect:(NSRect)dirtyRect {

    NSRect rect = NSMakeRect(20, 20, 100, 100);
    [[NSColor blueColor] set];
    [NSBezierPath fillRect:rect];

    NSGraphicsContext *gc = [NSGraphicsContext currentContext];
    CGContextRef ref = [gc graphicsPort];
    [[NSColor yellowColor] set];

    CGContextBeginPath(ref);
    CGContextMoveToPoint(ref, 20, 20);
    CGContextAddLineToPoint(ref, 120, 120);
    CGContextAddLineToPoint(ref, 20, 120);
    CGContextClosePath(ref);
    CGContextFillPath(ref);

    NSDictionary *attr = @{
	NSFontAttributeName : _font
    };
    NSAttributedString *s = [[NSAttributedString alloc] initWithString:_text
	    attributes:attr];
    rect = NSMakeRect(38,38,142,142);
    [[NSColor whiteColor] set];
    [NSBezierPath fillRect:rect];

    rect = NSMakeRect(40,40,140,140);
    [s drawInRect:rect];
}

@end


