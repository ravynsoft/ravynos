#import <Foundation/NSObject.h>
#import <CoreGraphics/CoreGraphics.h>

@interface CIColor : NSObject {
    CGColorRef _cgColor;
}

+ (CIColor *)colorWithCGColor:(CGColorRef)cgColor;

+ (CIColor *)colorWithRed:(CGFloat)red green:(CGFloat)green blue:(CGFloat)blue;
+ (CIColor *)colorWithRed:(CGFloat)red green:(CGFloat)green blue:(CGFloat)blue alpha:(CGFloat)alpha;

- initWithCGColor:(CGColorRef)cgColor;

- (size_t)numberOfComponents;
- (CGColorSpaceRef)colorSpace;
- (const CGFloat *)components;

- (CGFloat)red;
- (CGFloat)green;
- (CGFloat)blue;
- (CGFloat)alpha;

@end
