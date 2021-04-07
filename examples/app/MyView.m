#import <AppKit/AppKit.h>
#import <CoreGraphics/CGContext.h>
#import "MyView.h"

@implementation MyView

-(void)drawRect:(NSRect)dirtyRect {

    NSRect rect = NSMakeRect(20, 20, 100, 100);
    [[NSColor purpleColor] set];
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
}

@end


