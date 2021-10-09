/* Copyright (c) 2008 Johannes Fortmann
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/X11Window.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSPanel.h>
#import <AppKit/X11Display.h>
#import <AppKit/NSRaise.h>
#import <DBusKit/DKMenu.h>
#import <DBusKit/DKConnection.h>
#import <X11/Xutil.h>
#import <Foundation/NSException.h>
#import "O2Context_cairo.h"
#import "O2Context_builtin_FT.h"
#import <Onyx2D/O2Surface.h>
#import <QuartzCore/CAWindowOpenGLContext.h>

void CGNativeBorderFrameWidthsForStyle(unsigned styleMask,CGFloat *top,CGFloat *left,CGFloat *bottom,CGFloat *right) {
   *top=0;
   *left=0;
   *bottom=0;
   *right=0;
}

@implementation X11Window

+(Visual*)visual {
   static Visual* ret=NULL;
   
   if(!ret) {
      int visuals_matched, i;
      XVisualInfo match={0};
      Display *dpy=[(X11Display*)[NSDisplay currentDisplay] display];
   
      XVisualInfo *info=XGetVisualInfo(dpy,0, &match, &visuals_matched);
      
      for(i=0; i<visuals_matched; i++) {
         if(info[i].depth == 32 &&
            (info[i].red_mask   == 0xff0000 &&
             info[i].green_mask == 0x00ff00 &&
             info[i].blue_mask  == 0x0000ff)) {
            ret=info[i].visual;
         }
      }
      XFree(info);
      if(!ret)
         ret=DefaultVisual(dpy, DefaultScreen(dpy));
   }
   
   return ret;
}


-initWithFrame:(O2Rect)frame styleMask:(unsigned)styleMask isPanel:(BOOL)isPanel backingType:(NSUInteger)backingType {
   _level=kCGNormalWindowLevel;
   _backingType=backingType;
   _deviceDictionary=[NSMutableDictionary new];
   _display=[(X11Display*)[NSDisplay currentDisplay] display];
   int s = DefaultScreen(_display);
   _frame=[self transformFrame:frame];
   if(isPanel && styleMask&NSDocModalWindowMask)
    styleMask=NSBorderlessWindowMask;
      
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
       NSLog(@"glXChooseVisual failed at %s %d",__FILE__,__LINE__);
      }

      Colormap cmap = XCreateColormap(_display, RootWindow(_display, _visualInfo->screen), _visualInfo->visual, AllocNone);

      if(cmap<0){
       NSLog(@"XCreateColormap failed");
       [self dealloc];
       return nil;
      }

   XSetWindowAttributes xattr;
   unsigned long xattr_mask;
   xattr.override_redirect = styleMask == NSBorderlessWindowMask ? True : False;
   xattr_mask = CWOverrideRedirect|CWColormap;
      xattr.colormap=cmap;

   _window = XCreateWindow(_display, DefaultRootWindow(_display),
                              _frame.origin.x, _frame.origin.y, _frame.size.width, _frame.size.height,
                              0, (_visualInfo==NULL)?CopyFromParent:_visualInfo->depth, InputOutput, 
                              (_visualInfo==NULL)?CopyFromParent:_visualInfo->visual,
                              xattr_mask, &xattr);

   XSelectInput(_display, _window, ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask |
    ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | VisibilityChangeMask | FocusChangeMask | SubstructureRedirectMask );

   Atom atm=XInternAtom(_display, "WM_DELETE_WINDOW", False);
   XSetWMProtocols(_display, _window, &atm , 1);

   [self setProperty:@"_KDE_NET_WM_APPMENU_SERVICE_NAME" toValue:[[NSApp dbusConnection] name]];
   [self setProperty:@"_KDE_NET_WM_APPMENU_OBJECT_PATH" toValue:[[NSApp dbusMenu] objectPath]];
   [[NSApp dbusMenu] registerWindow:_window];
      
   XSetWindowBackgroundPixmap(_display, _window, None);
      
   [(X11Display*)[NSDisplay currentDisplay] setWindow:self forID:_window];
      
   if(styleMask == NSBorderlessWindowMask){
     [isa removeDecorationForWindow:_window onDisplay:_display];
    }
   return self;
}

-(void)dealloc {
   [self invalidate];
   [_backingContext release];
   [_context release];
   [_deviceDictionary release];   
   [super dealloc];
}

+(void)removeDecorationForWindow:(Window)w onDisplay:(Display*)dpy
{
   return;
   struct {
      unsigned long flags;
      unsigned long functions;
      unsigned long decorations;
      long input_mode;
      unsigned long status;
   } hints = {
      2, 0, 0, 0, 0,
   };
   XChangeProperty (dpy, w,
                    XInternAtom (dpy, "_MOTIF_WM_HINTS", False),
                    XInternAtom (dpy, "_MOTIF_WM_HINTS", False),
                    32, PropModeReplace,
                    (const unsigned char *) &hints,
                    sizeof (hints) / sizeof (long));
}

-(void)ensureMapped {
   if(!_mapped){      
      XMapWindow(_display, _window);
      _mapped=YES;
   }
}


-(void)setDelegate:delegate {
   _delegate=delegate;
}

-delegate {
   return _delegate;
}

-(void)invalidate {
   [_delegate platformWindowDidInvalidateCGContext:self];
   _delegate=nil;
   [_context release];
   _context=nil;

   if(_window) {
      [(X11Display*)[NSDisplay currentDisplay] setWindow:nil forID:_window];
      XDestroyWindow(_display, _window);
      _window=0;
   }
}

-(Window)windowHandle {
   return _window;
}

-(O2Context *)createCGContextIfNeeded {
    if(_context == nil) {
        O2ColorSpaceRef colorSpace = O2ColorSpaceCreateDeviceRGB();
        O2Surface *surface = [[O2Surface alloc] initWithBytes:NULL
            width:_frame.size.width height:_frame.size.height
            bitsPerComponent:8 bytesPerRow:0 colorSpace:colorSpace
            bitmapInfo:kO2ImageAlphaPremultipliedFirst|kO2BitmapByteOrder32Little];
        O2ColorSpaceRelease(colorSpace);
        _context = [[O2Context_builtin_FT alloc] initWithSurface:surface flipped:NO];
    }
    return _context;
}

-(O2Context *)createBackingCGContextIfNeeded {
   if(_backingContext==nil){
    _backingContext=[O2Context createBackingContextWithSize:_frame.size context:[self createCGContextIfNeeded] deviceDictionary:_deviceDictionary];
   }

   return _backingContext;
}

-(O2Context *)cgContext {
    return [self createCGContextIfNeeded];
}

-(void)invalidateContextsWithNewSize:(NSSize)size forceRebuild:(BOOL)forceRebuild {
   if(!NSEqualSizes(_frame.size,size) || forceRebuild){
    NSSize oldSize = _frame.size;
    _frame.size=size;

    O2Context *currentContext = _context;
    O2ColorSpaceRef colorSpace = O2ColorSpaceCreateDeviceRGB();
    O2Surface *surface = [[O2Surface alloc] initWithBytes:NULL
        width:_frame.size.width height:_frame.size.height
        bitsPerComponent:8 bytesPerRow:0 colorSpace:colorSpace
        bitmapInfo:kO2ImageAlphaPremultipliedFirst|kO2BitmapByteOrder32Little];
    O2ColorSpaceRelease(colorSpace);
    _context = [[O2Context_builtin_FT alloc] initWithSurface:surface flipped:NO];
    [_context drawImage:[currentContext surface]
        inRect:NSMakeRect(0,0,oldSize.width,oldSize.height)];
    [_delegate platformWindowDidInvalidateCGContext:self];
    [currentContext release];
   }
}

-(void)invalidateContextsWithNewSize:(NSSize)size {
   [self invalidateContextsWithNewSize:size forceRebuild:NO];
}

-(void)setTitle:(NSString *)title {
   XTextProperty prop;
   const char* text=[title cString];
   XStringListToTextProperty((char**)&text, 1, &prop);
   XSetWMName(_display, _window, &prop);
}

-(BOOL)setProperty:(NSString *)property toValue:(NSString *)value {
   if(XChangeProperty(_display, _window, 
     XInternAtom (_display, [property UTF8String], False),
     XInternAtom (_display, "STRING", False), 8,
     PropModeReplace, (const unsigned char *)[value UTF8String],
     [value length]))
       return NO;
   return YES;
}

-(void)setFrame:(O2Rect)frame {
   frame=[self transformFrame:frame];
   XMoveResizeWindow(_display, _window, frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
   [self invalidateContextsWithNewSize:frame.size];
}

-(void)setLevel:(int)value {
    _level=value;
}

-(void)showWindowForAppActivation:(O2Rect)frame {
   NSUnimplementedMethod();
}

-(void)hideWindowForAppDeactivation:(O2Rect)frame {
   NSUnimplementedMethod();
}

-(void)hideWindow {
   XUnmapWindow(_display, _window);
   _mapped=NO;
}

-(void)placeAboveWindow:(int)otherNumber {
   X11Window *other=[X11Window windowWithWindowNumber:otherNumber];
   [self ensureMapped];

   if(!other) {
      XRaiseWindow(_display, _window);
   }
   else {
      Window w[2]={_window, other->_window};
      XRestackWindows(_display, w, 1);
   }
}

-(void)placeBelowWindow:(int)otherNumber {
   X11Window *other=[X11Window windowWithWindowNumber:otherNumber];
   [self ensureMapped];

   if(!other) {
      XLowerWindow(_display, _window);
   }
   else {
      Window w[2]={other->_window, _window};
      XRestackWindows(_display, w, 1);
   }
}

-(void)makeKey {
   [self ensureMapped];
   XRaiseWindow(_display, _window);
}

-(void)makeMain {
}

-(void)captureEvents {
   // FIXME: find out what this is supposed to do
}

-(void)miniaturize {
   NSUnimplementedMethod();

}

-(void)deminiaturize {
   NSUnimplementedMethod();
}

-(BOOL)isMiniaturized {
   return NO;
}

CGL_EXPORT CGLError CGLCreateContextForWindow(CGLPixelFormatObj pixelFormat,CGLContextObj share,CGLContextObj *resultp,Display *display,XVisualInfo *visualInfo,Window window);

-(void)createCGLContextObjIfNeeded {
   if(_cglContext==NULL){
    CGLError error;
    
    if((error=CGLCreateContextForWindow(NULL,NULL,&_cglContext,_display,_visualInfo,_window))!=kCGLNoError)
     NSLog(@"glXCreateContext failed at %s %d with error %d",__FILE__,__LINE__,error);
   }
   if(_cglContext!=NULL && _caContext==NULL){
    _caContext=[[CAWindowOpenGLContext alloc] initWithCGLContext:_cglContext];
   }
   
}

-(void)openGLFlushBuffer {
   CGLError error;

  CGLContextObj prevContext = CGLGetCurrentContext();
   
   [self createCGLContextObjIfNeeded];
   if(_caContext==NULL)
    return;

   O2Surface *surface=[_context surface]; // [_backingContext surface];
   size_t width=O2ImageGetWidth(surface);
   size_t height=O2ImageGetHeight(surface);

   [_caContext prepareViewportWidth:width height:height];
   [_caContext renderSurface:surface];
   
   glFlush();
   glXSwapBuffers(_display,_window);

  CGLSetCurrentContext(prevContext);
}

-(void)flushBuffer {
    O2ContextFlush(_context);
    [self openGLFlushBuffer];
}


-(NSPoint)mouseLocationOutsideOfEventStream {
   NSUnimplementedMethod();
   return NSZeroPoint;
}


-(O2Rect)frame
{
   return [self transformFrame:_frame];
}

static int ignoreBadWindow(Display* display,
                        XErrorEvent* errorEvent) {
   if(errorEvent->error_code==BadWindow)
      return 0;
   char buf[512];
   XGetErrorText(display, errorEvent->error_code, buf, 512);
   [NSException raise:NSInternalInconsistencyException format:@"X11 error: %s", buf];
   return 0;
}

-(void)frameChanged
{
   XErrorHandler previousHandler=XSetErrorHandler(ignoreBadWindow);
   @try {
      Window root, parent;
      Window window=_window;
      int x, y;
      unsigned int w, h, d, b, nchild;
      Window* children;
      O2Rect rect=NSZeroRect;
      // recursively get geometry to get absolute position
      BOOL success=YES;
      while(window && success) {
         XGetGeometry(_display, window, &root, &x, &y, &w, &h, &b, &d);
         success = XQueryTree(_display, window, &root, &parent, &children, &nchild);
         if(children)
            XFree(children);
         
         // first iteration: save our own w, h
         if(window==_window)
            rect=NSMakeRect(0, 0, w, h);
         rect.origin.x+=x;
         rect.origin.y+=y;
         window=parent;
      };
      
     [self invalidateContextsWithNewSize:rect.size];
     _frame = rect;
   }
   @finally {
      XSetErrorHandler(previousHandler);
   }
}

-(Visual*)visual {
   return DefaultVisual(_display, DefaultScreen(_display));
}

-(Drawable)drawable {
   return _window;
}

-(void)addEntriesToDeviceDictionary:(NSDictionary *)entries  {
   [_deviceDictionary addEntriesFromDictionary:entries];
}

-(O2Rect)transformFrame:(O2Rect)frame {
   return NSMakeRect(frame.origin.x, DisplayHeight(_display, DefaultScreen(_display)) - frame.origin.y - frame.size.height, fmax(frame.size.width, 1.0), fmax(frame.size.height, 1.0));
}

-(NSPoint)transformPoint:(NSPoint)pos;
{
   return NSMakePoint(pos.x, _frame.size.height-pos.y);
}


@end

CGRect CGInsetRectForNativeWindowBorder(CGRect frame,unsigned styleMask){
    CGFloat top,left,bottom,right;
    
    CGNativeBorderFrameWidthsForStyle(styleMask,&top,&left,&bottom,&right);
    
    frame.origin.x+=left;
    frame.origin.y+=bottom;
    frame.size.width-=left+right;
    frame.size.height-=top+bottom;
    
    return frame;
}

CGRect CGOutsetRectForNativeWindowBorder(CGRect frame,unsigned styleMask){
    CGFloat top,left,bottom,right;
    
    CGNativeBorderFrameWidthsForStyle(styleMask,&top,&left,&bottom,&right);
    
    frame.origin.x-=left;
    frame.origin.y-=bottom;
    frame.size.width+=left+right;
    frame.size.height+=top+bottom;
    
    return frame;
}
