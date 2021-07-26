#import <QuartzCore/CARenderer.h>
#import <OpenGL/OpenGL.h>

@interface CAWindowOpenGLContext : NSObject {
    CGLContextObj _cglContext;
}

- initWithCGLContext:(CGLContextObj)cglContext;

- (void)prepareViewportWidth:(int)width height:(int)height;

- (void)renderSurface:(O2Surface *)surface;

@end
