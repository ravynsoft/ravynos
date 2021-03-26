#import <Foundation/NSObject.h>
#import <CoreGraphics/CoreGraphics.h>

@class CIFilter;

@interface CIImage : NSObject {
    CGImageRef _cgImage;
    CIFilter *_filter;
}

+ (CIImage *)emptyImage;

- initWithCGImage:(CGImageRef)cgImage;
- (CGRect)extent;

@end
