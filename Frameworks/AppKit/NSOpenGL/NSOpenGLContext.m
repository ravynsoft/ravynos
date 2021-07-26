/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSOpenGLContext.h>
#import <AppKit/NSOpenGLPixelFormat.h>
#import <AppKit/NSGraphicsContext.h>
#import <AppKit/NSRaise.h>
#import <AppKit/NSView.h>
#import <AppKit/NSWindow-Private.h>
#import <OpenGL/OpenGL.h>
#import <Foundation/NSThread-Private.h>

@interface NSOpenGLContext(private)
-(void)_clearCurrentContext;
@end

@implementation NSOpenGLContext

static inline NSOpenGLContext *_currentContext(){
   return (NSOpenGLContext *)NSThreadSharedInstanceDoNotCreate(@"NSOpenGLContext");
}

static void _setCurrentContext(NSOpenGLContext *context){
   [NSCurrentThread() setSharedObject:context forClassName:@"NSOpenGLContext"];
}

static inline void _clearCurrentContext(){
   [_currentContext() _clearCurrentContext];
   _setCurrentContext(nil);
}

+(NSOpenGLContext *)currentContext {
   return _currentContext();
}

+(void)clearCurrentContext {
   _clearCurrentContext();
}

-initWithFormat:(NSOpenGLPixelFormat *)pixelFormat shareContext:(NSOpenGLContext *)shareContext {
   CGLError error;

    if(_pixelFormat!=nil){
        // Cocoa's NSOpenGLContext can withstand a double init and I know of at least one app that does it
        // Maybe Cocoa just leaks, we don't
        [_pixelFormat release];
        _pixelFormat==nil;
        CGLReleaseContext(_glContext);
        _glContext=NULL;
    }
    
   _pixelFormat=[pixelFormat retain];
   if((error=CGLCreateContext([_pixelFormat CGLPixelFormatObj],[shareContext CGLContextObj],(CGLContextObj *)&_glContext))!=kCGLNoError)
    NSLog(@"CGLCreateContext failed with %d in %s %d",error,__FILE__,__LINE__);
        
   return self;
}

-(void)dealloc {
// FIXME: this doesn't actually work because if we are the current context we're retained by the thread shared object dict
   if(_currentContext()==self)
      _clearCurrentContext();
   [_pixelFormat release];
   _view=nil;
   
   
   CGLReleaseContext(_glContext);
   [super dealloc];
}

-(NSView *)view {
   return _view;
}

-(NSOpenGLPixelBuffer *)pixelBuffer {
   NSUnimplementedMethod();
   return nil;
}

-(unsigned long)pixelBufferCubeMapFace {
   NSUnimplementedMethod();
   return 0;
}

-(long)pixelBufferMipMapLevel {
   NSUnimplementedMethod();
   return 0;
}

-(void *)CGLContextObj {
   return _glContext;
}

-(void)getValues:(GLint *)vals forParameter:(NSOpenGLContextParameter)parameter {   
   CGLGetParameter(_glContext,parameter,vals);
}

-(void)setValues:(const GLint *)vals forParameter:(NSOpenGLContextParameter)parameter {   
   CGLSetParameter(_glContext,parameter,vals);
}

-(void)updateViewParameters {
    NSRect rect=[_view bounds];
    
    if([_view window]!=nil)
        rect=[_view convertRect:rect toView:nil];
   
   GLint size[2]={
    rect.size.width,
    rect.size.height };
   GLint origin[2]={
    rect.origin.x,
    rect.origin.y };
   GLint hidden[1]= {
    [_view isHidden] ? 1 : 0
   };
   
   CGLSetParameter(_glContext,kCGLCPSurfaceBackingSize,size);
   CGLSetParameter(_glContext,kCGLCPSurfaceBackingOrigin,origin);
   CGLSetParameter(_glContext,kCGLCPSurfaceHidden,hidden);
}

-(void)setView:(NSView *)view {
   if(_view!=view)
    _hasPrepared=NO;
    
   _view=view;
      
   CGLLockContext(_glContext);
   
   GLint num[1]={[[_view window] windowNumber]};
   
   CGLSetParameter(_glContext,kCGLCPSurfaceWindowNumber,num);
   
   [self update];

   CGLUnlockContext(_glContext);
}

-(void)makeCurrentContext {
   CGLError error;
   
   if((error=CGLSetCurrentContext(_glContext))!=kCGLNoError)
    NSLog(@"CGLSetCurrentContext failed with %d in %s %d",error,__FILE__,__LINE__);
    
   _setCurrentContext(self);

#if 0
/*
   We need to reload the view values when becoming current because it may
   have moved windows since the last make current
 */
 // Possible this shouldnt be done here, especially on a non-main thread
 
 // Don't do this here due to threading reason. Figure out where to do this when moving windows, view _setWindow ?
   [self updateViewParameters];
#endif

   if(!_hasPrepared){
    _hasPrepared=YES;

// NSOpenGLContext will call prepareOpenGL on any view, not just NSOpenGLView    
    if([_view respondsToSelector:@selector(prepareOpenGL)])
     [_view performSelector:@selector(prepareOpenGL)];
   }
   
}

-(void)_clearCurrentContext {
   CGLError error;
   
   if((error=CGLSetCurrentContext(NULL))!=kCGLNoError)
    NSLog(@"CGLSetCurrentContext failed with %d in %s %d",error,__FILE__,__LINE__);
}

-(int)currentVirtualScreen {
   NSUnimplementedMethod();
   return 0;
}

-(void)setCurrentVirtualScreen:(int)screen {
   NSUnimplementedMethod();
}

-(void)setFullScreen {
   if(_view!=nil)
    NSLog(@"NSOpenGLContext has view, full-screen not allowed");
    
   NSUnimplementedMethod();
}

-(void)setOffscreen:(void *)bytes width:(long)width height:(long)height rowbytes:(long)rowbytes {
   NSUnimplementedMethod();
}

-(void)setPixelBuffer:(NSOpenGLPixelBuffer *)pixelBuffer cubeMapFace:(unsigned long)cubeMapFace mipMapLeve:(long)mipMapLevel currentVirtualScreen:(int)screen {
   NSUnimplementedMethod();
}

-(void)setTextureImageToPixelBuffer:(NSOpenGLPixelBuffer *)pixelBuffer colorBuffer:(unsigned long)source {
   NSUnimplementedMethod();
}

-(void)update {
    [self updateViewParameters];
}

-(void)clearDrawable {
    [self setView:nil];
}

-(void)copyAttributesFromContext:(NSOpenGLContext *)context withMask:(unsigned long)mask {
   NSUnimplementedMethod();
}

-(void)createTexture:(unsigned long)identifier fromView:(NSView *)view internalFormat:(unsigned long)internalFormat {
   NSUnimplementedMethod();
}

-(void)flushBuffer {
   CGLFlushDrawable(_glContext);
   
}

@end
