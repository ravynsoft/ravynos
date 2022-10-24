#import <CoreGraphics/CoreGraphicsExport.h>
#import <CoreGraphics/CGGeometry.h>
#import <CoreGraphics/CGColorSpace.h>
#import <CoreFoundation/CFArray.h>

typedef struct CGGradient *CGGradientRef;

enum {
    kCGGradientDrawsBeforeStartLocation = 0x01,
    kCGGradientDrawsAfterEndLocation = 0x02
};

CGGradientRef CGGradientCreateWithColorComponents(CGColorSpaceRef colorSpace, const CGFloat components[], const CGFloat locations[], size_t count);
CGGradientRef CGGradientCreateWithColors(CGColorSpaceRef colorSpace, CFArrayRef colors, const CGFloat locations[]);

void CGGradientRelease(CGGradientRef self);
CGGradientRef CGGradientRetain(CGGradientRef self);
