#import <AppKit/NSOpenGLDrawable.h>

@implementation NSOpenGLDrawable 

-initWithPixelFormat:(NSOpenGLPixelFormat *)pixelFormat view:(NSView *)view {
   return self;
}

-(CGLContextObj)createGLContext {
   return NULL;
}

-(void)invalidate {
}

-(void)updateWithView:(NSView *)view {
}

-(void)makeCurrentWithGLContext:(CGLContextObj)glContext {
}

-(void)clearCurrentWithGLContext:(CGLContextObj)glContext {
}

-(void)swapBuffers {
}

@end
