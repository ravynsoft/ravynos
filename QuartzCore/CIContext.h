#import <Foundation/NSObject.h>
#import <CoreGraphics/CoreGraphics.h>

@class CIImage, NSDictionary;

@interface CIContext : NSObject {
    CGContextRef _cgContext;
}

+ (CIContext *)contextWithCGContext:(CGContextRef)cgContext options:(NSDictionary *)options;

- (void)drawImage:(CIImage *)image atPoint:(CGPoint)atPoint fromRect:(CGRect)fromRect;
- (void)drawImage:(CIImage *)image inRect:(CGRect)inRect fromRect:(CGRect)fromRect;

@end
