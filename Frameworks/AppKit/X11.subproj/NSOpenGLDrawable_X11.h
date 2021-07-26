//
//  NSOpenGLDrawable_X11.h
//  AppKit
//
//  Created by Johannes Fortmann on 02.01.09.
//  Copyright 2009 -. All rights reserved.
//

#import <AppKit/NSOpenGLDrawable.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

@class NSView, NSOpenGLPixelFormat;

@interface NSOpenGLDrawable_X11 : NSOpenGLDrawable {
    NSOpenGLPixelFormat *_format;
    Display *_display;
    XVisualInfo *_visualInfo;
    Window _window;
    Window _lastParent;
}

@end
