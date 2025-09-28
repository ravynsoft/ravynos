/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <OpenGL/gl.h>

@class NSOpenGLPixelFormat, NSOpenGLPixelBuffer, NSView;

typedef enum {
    NSOpenGLCPSwapRectangle = 200,
    NSOpenGLCPSwapRectangleEnable = 201,
    NSOpenGLCPRasterizationEnable = 221,
    NSOpenGLCPSwapInterval = 222,
    NSOpenGLCPSurfaceOrder = 235,
    NSOpenGLCPSurfaceOpacity = 236,
    NSOpenGLCPStateValidation = 301,
} NSOpenGLContextParameter;

@interface NSOpenGLContext : NSObject {
    NSOpenGLPixelFormat *_pixelFormat;
    NSView *_view;
    void *_glContext;
    id __remove;
    BOOL _hasPrepared;
}

+ (NSOpenGLContext *)currentContext;
+ (void)clearCurrentContext;

- initWithFormat:(NSOpenGLPixelFormat *)pixelFormat shareContext:(NSOpenGLContext *)shareContext;

- (NSView *)view;
- (NSOpenGLPixelBuffer *)pixelBuffer;
- (unsigned long)pixelBufferCubeMapFace;
- (long)pixelBufferMipMapLevel;
- (void *)CGLContextObj;

- (void)getValues:(GLint *)vals forParameter:(NSOpenGLContextParameter)parameter;
- (void)setValues:(const GLint *)vals forParameter:(NSOpenGLContextParameter)parameter;
- (void)setView:(NSView *)view;

- (void)makeCurrentContext;

- (int)currentVirtualScreen;
- (void)setCurrentVirtualScreen:(int)screen;
- (void)setFullScreen;
- (void)setOffscreen:(void *)bytes width:(long)width height:(long)height rowbytes:(long)rowbytes;
- (void)setPixelBuffer:(NSOpenGLPixelBuffer *)pixelBuffer cubeMapFace:(unsigned long)cubeMapFace mipMapLeve:(long)mipMapLevel currentVirtualScreen:(int)screen;
- (void)setTextureImageToPixelBuffer:(NSOpenGLPixelBuffer *)pixelBuffer colorBuffer:(unsigned long)source;

- (void)update;

- (void)clearDrawable;

- (void)copyAttributesFromContext:(NSOpenGLContext *)context withMask:(unsigned long)mask;
- (void)createTexture:(unsigned long)identifier fromView:(NSView *)view internalFormat:(unsigned long)internalFormat;

- (void)flushBuffer;

@end
