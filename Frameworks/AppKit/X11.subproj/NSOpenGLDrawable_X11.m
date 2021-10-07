//
//  NSOpenGLDrawable_X11.m
//  AppKit
//
//  Created by Johannes Fortmann on 02.01.09.
//  Copyright 2009 -. All rights reserved.
//

#import <AppKit/NSOpenGLDrawable_X11.h>
#import <AppKit/X11Display.h>
#import <AppKit/X11Window.h>
#import <AppKit/NSWindow-Private.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#import <OpenGL/OpenGL.h>
#include <GL/glx.h>

//CGL_EXPORT CGLError CGLCreateContext(CGLPixelFormatObj pixelFormat,Display *dpy,XVisualInfo *vis,Window window,CGLContextObj *resultp);

@implementation NSOpenGLDrawable(X11)

+allocWithZone:(NSZone *)zone {
   return NSAllocateObject([NSOpenGLDrawable_X11 class],0,NULL);
}

@end

@implementation NSOpenGLDrawable_X11

-initWithPixelFormat:(NSOpenGLPixelFormat *)pixelFormat view:(NSView *)view {
   if(self = [super init]) {
   
      _format=[pixelFormat retain];
      _display=[(X11Display*)[NSDisplay currentDisplay] display];
      
      GLint att[] = {
       GLX_RGBA,
       GLX_DOUBLEBUFFER,
       GLX_RED_SIZE, 4,
       GLX_GREEN_SIZE, 4,
       GLX_BLUE_SIZE, 4,
       GLX_DEPTH_SIZE, 4,
       None
      };
      
      int screen = DefaultScreen(_display);
      
      if((_visualInfo=glXChooseVisual(_display,screen,att))==NULL){
       NSLog(@"glXChooseVisual failed");
       [self dealloc];
       return nil;;
      }
      
      Colormap cmap = XCreateColormap(_display, RootWindow(_display, _visualInfo->screen), _visualInfo->visual, AllocNone);

      if(cmap<0){
       NSLog(@"XCreateColormap failed");
       [self dealloc];
       return nil;
      }
      
      XSetWindowAttributes xattr;
      
      bzero(&xattr,sizeof(xattr));
      
      xattr.colormap=cmap;
      xattr.border_pixel = 0;                                                           
      xattr.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask;
                                            
      NSRect frame=[view frame];
    
       Window parent=[(X11Window*)[[view window] platformWindow] drawable];
       
       if(parent==0)
        parent=RootWindow(_display, _visualInfo->screen);
        
      _window = XCreateWindow(_display,parent, frame.origin.x, frame.origin.y, frame.size.width,frame.size.height, 0, _visualInfo->depth, InputOutput, _visualInfo->visual, CWBorderPixel | CWColormap | CWEventMask, &xattr);
      
      
     // XSetWindowBackgroundPixmap(_display, _window, None);
     // [X11Window removeDecorationForWindow:_window onDisplay:_display];
      
      XMapWindow(_display, _window);

   }
   return self;
}

-(void)dealloc {
   if(_window)
      XDestroyWindow(_display, _window);
   [_format release];
   [super dealloc];
}

-(CGLContextObj)createGLContext {
   CGLContextObj result=NULL;
   CGLError error;

   if((error=CGLCreateContextForWindow(NULL,NULL,&result,_display,_visualInfo,_window))!=kCGLNoError)
    NSLog(@"CGLCreateContext failed with %d in %s %d",error,__FILE__,__LINE__);

   return result;
}

-(void)invalidate {
}

-(void)updateWithView:(NSView *)view {

   NSRect frame=[view frame];
   frame=[[view superview] convertRect:frame toView:nil];

   X11Window *wnd=(X11Window*)[[view window] platformWindow];
   NSRect wndFrame=[wnd frame];
   
   frame.origin.y=wndFrame.size.height-(frame.origin.y+frame.size.height);
   
   XMoveResizeWindow(_display, _window, frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
   Window viewWindow=[(X11Window*)[[view window] platformWindow] drawable];
   if(_lastParent!=viewWindow) {
      XReparentWindow(_display, _window, viewWindow, frame.origin.x, frame.origin.y);
      _lastParent=viewWindow;
   }
}

-(void)makeCurrentWithGLContext:(CGLContextObj)glContext {
   CGLError error;
   
   if((error=CGLSetCurrentContext(glContext))!=kCGLNoError)
    NSLog(@"CGLSetCurrentContext failed with %d in %s %d",error,__FILE__,__LINE__);
}

-(void)clearCurrentWithGLContext:(CGLContextObj)glContext {   
   CGLError error;
   
   if((error=CGLSetCurrentContext(NULL))!=kCGLNoError)
    NSLog(@"CGLSetCurrentContext failed with %d in %s %d",error,__FILE__,__LINE__);
}

-(void)swapBuffers {
   glXSwapBuffers(_display, _window);
}

@end
