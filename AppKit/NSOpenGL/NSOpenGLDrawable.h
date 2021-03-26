#import <Foundation/NSObject.h>
#import <OpenGL/OpenGL.h>

@class NSOpenGLPixelFormat, NSView;

@interface NSOpenGLDrawable : NSObject

- initWithPixelFormat:(NSOpenGLPixelFormat *)pixelFormat view:(NSView *)view;

- (CGLContextObj)createGLContext;

- (void)invalidate;
- (void)updateWithView:(NSView *)view;
- (void)makeCurrentWithGLContext:(CGLContextObj)glContext;
- (void)clearCurrentWithGLContext:(CGLContextObj)glContext;
- (void)swapBuffers;

@end
