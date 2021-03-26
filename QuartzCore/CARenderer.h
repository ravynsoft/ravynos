#import <Foundation/NSObject.h>
#import <ApplicationServices/ApplicationServices.h>
#import <CoreVideo/CoreVideo.h>

@class CALayer, O2Surface;

@interface CARenderer : NSObject {
    void *_cglContext;
    CGRect _bounds;
    CALayer *_rootLayer;
}

@property(assign) CGRect bounds;
@property(retain) CALayer *layer;

+ (CARenderer *)rendererWithCGLContext:(void *)cglContext options:(NSDictionary *)options;

- (void)beginFrameAtTime:(CFTimeInterval)currentTime timeStamp:(CVTimeStamp *)timeStamp;

- (void)render;

- (void)endFrame;

@end
