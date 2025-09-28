#import <Foundation/NSAffineTransform.h>

@interface CIAffineTransform : NSObject <NSCoding> {
    NSAffineTransform *_transform;
    BOOL _ciEnabled;
}

- (NSAffineTransform *)affineTransform;

@end
