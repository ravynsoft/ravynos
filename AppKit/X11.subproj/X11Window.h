/* Copyright (c) 2008 Johannes Fortmann
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <CoreGraphics/CGWindow.h>
#import <Onyx2D/O2Geometry.h>
#import <X11/Xlib.h>
#import <GL/glx.h>
#import <OpenGL/OpenGL.h>

@class O2Context_cairo, X11Display, CAWindowOpenGLContext;

@interface X11Window : CGWindow {
    int _level; //TODO: care about this value
    Display *_display;
    XVisualInfo *_visualInfo;
    Window _window;
    CGLContextObj _cglContext;
    CAWindowOpenGLContext *_caContext;

    id _delegate;
    CGSBackingStoreType _backingType;
    O2Context *_backingContext;
    O2Context *_context;

    NSMutableDictionary *_deviceDictionary;
    O2Rect _frame;
    BOOL _mapped;
}
+ (void)removeDecorationForWindow:(Window)w onDisplay:(Display *)dpy;
- initWithFrame:(NSRect)frame styleMask:(unsigned)styleMask isPanel:(BOOL)isPanel backingType:(NSUInteger)backingType;
- (O2Rect)frame;
- (Visual *)visual;
- (Drawable)drawable;
- (NSPoint)transformPoint:(NSPoint)pos;
- (O2Rect)transformFrame:(O2Rect)frame;
- (BOOL)setProperty:(NSString *)property toValue:(NSString *)value;

- (Window)windowHandle;

- (void)frameChanged;

@end

void CGNativeBorderFrameWidthsForStyle(unsigned styleMask, CGFloat *top, CGFloat *left, CGFloat *bottom, CGFloat *right);
