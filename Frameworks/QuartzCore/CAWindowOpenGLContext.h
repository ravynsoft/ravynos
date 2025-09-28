#import <QuartzCore/CARenderer.h>

@interface CAWindowOpenGLContext : NSObject {
    CGLContextObj _cglContext;
    unsigned int vbo;
    unsigned int vao;
}

- initWithCGLContext:(CGLContextObj)cglContext;

- (void)prepareViewportWidth:(int)width height:(int)height;

- (void)renderSurface:(O2Surface *)surface;

@end
