/*
 * Copyright (c) 2008 Johannes Fortmann
 * Copyright (C) 2022 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#import <Foundation/NSException.h>
#import <AppKit/NSDisplay.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSPanel.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#import "WLWindow.h"
#import "O2Context_builtin_FT.h"
#import <Onyx2D/O2Surface.h>
#import <Onyx2D/O2ImageSource_PNG.h>
#import <Onyx2D/O2Image.h>
#import <QuartzCore/CAWindowOpenGLContext.h>


CGL_EXPORT CGLError CGLCreateContextForWindow(CGLPixelFormatObj pixelFormat,
    CGLContextObj share, CGLContextObj *resultp, unsigned long window);

void CGNativeBorderFrameWidthsForStyle(unsigned styleMask,CGFloat *top,CGFloat *left,
                                       CGFloat *bottom,CGFloat *right)
{
    switch(styleMask) {
        case NSBorderlessWindowMask:
            *top=0;
            *left=0;
            *bottom=0;
            *right=0;
            break;
        // FIXME: tool window style?
        default:
            *top=44;
            *left=0;
            *bottom=0;
            *right=0;
    }
}

static void handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
    WLWindow *win = (__bridge WLWindow *)data;

    // FIXME: most of this should be part of WLDisplay, not WLWindow
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        [win set_compositor:wl_registry_bind(registry, name, &wl_compositor_interface, 1)];
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        [win set_wm_base:wl_registry_bind(registry, name, &xdg_wm_base_interface, 1)];
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        struct wl_seat *seat = wl_registry_bind(registry, name, &wl_seat_interface, 7);
        WLDisplay *display = [NSDisplay currentDisplay];
        [display setSeat:seat];
    }
}

static void handle_global_remove(void *data, struct wl_registry *registry,
		uint32_t name) {
	// Who cares?
}

static const struct wl_registry_listener registry_listener = {
	.global = handle_global,
	.global_remove = handle_global_remove,
};


static void xdg_surface_handle_configure(void *data,
		struct xdg_surface *xdg_surface, uint32_t serial) {
    WLWindow *win = (__bridge WLWindow *)data;
    @synchronized(win) {
        CGRect frame = CGOutsetRectForNativeWindowBorder([win frame],[win styleMask]);
        xdg_surface_ack_configure(xdg_surface, serial);
        [win setFrame:frame];
        [win setReady:YES];
        [win flushBuffer];

        [win frameChanged];
        [[win delegate] platformWindow:win frameChanged:frame didSize:YES];
    }
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_handle_configure,
};

static void xdg_toplevel_handle_configure(void *data,
		struct xdg_toplevel *xdg_toplevel, int32_t w, int32_t h,
		struct wl_array *states) {
    if(w > 0 && h > 0) {
        WLWindow *win = (__bridge WLWindow *)data;
        @synchronized(win) {
            NSRect frame = [win frame];
            frame.size.width = w;
            frame.size.height = h;
            [win setFrame:frame];
        }
    }
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_handle_configure,
};

static void
xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

static void renderCallback(void *data, struct wl_callback *cb, uint32_t time);
static const struct wl_callback_listener frame_listener = {
    .done = renderCallback,
};

static void renderCallback(void *data, struct wl_callback *cb, uint32_t time) {
    if(cb != NULL)
        wl_callback_destroy(cb);

    WLWindow *win = (__bridge WLWindow *)data;
    [win flushBuffer];
    cb = wl_surface_frame([win wl_surface]);
    wl_callback_add_listener(cb, &frame_listener, (__bridge void *)win);
}

@implementation WLWindow

- initWithFrame:(O2Rect)frame styleMask:(unsigned)styleMask isPanel:(BOOL)isPanel
    backingType:(NSUInteger)backingType
{
    _level = kCGNormalWindowLevel;
    _backingType = backingType;
    _deviceDictionary = [NSMutableDictionary new];

    /* FIXME: this is because wayland doesn't give us position info */
    _frame.origin = NSMakePoint(0,0);
    _frame.size = frame.size;

    _context = nil;
    _styleMask = styleMask;
    _ready = NO;

    _display = (WLDisplay *)[NSDisplay currentDisplay];
    struct wl_display *display = [_display display];
    if(display == NULL) {
        NSLog(@"WLWindow: Failed to connect to display");
        return nil;
    }

    registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, (__bridge void *)self);
    wl_display_roundtrip(display);

    if(compositor == NULL) {
        NSLog(@"WLWindow: compositor not available");
        return nil;
    }
    if(wm_base == NULL) {
        NSLog(@"WLWindow: xdg-shell not available");
        return nil;
    }

    wl_surface = wl_compositor_create_surface(compositor);
    xdg_surface = xdg_wm_base_get_xdg_surface(wm_base, wl_surface);
    xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);

    wl_display_roundtrip(display);

    xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, (__bridge void *)self);
    xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, (__bridge void *)self);

    wl_surface_commit(wl_surface);

    if(isPanel && (styleMask & NSDocModalWindowMask))
        _styleMask=NSBorderlessWindowMask;
      
    [_display setWindow:self forID:(uintptr_t)wl_surface];
    return self;
}

-(void)dealloc
{
    if(xdg_toplevel)
        xdg_toplevel_destroy(xdg_toplevel);
    if(xdg_surface)
        xdg_surface_destroy(xdg_surface);
    if(wl_surface)
        wl_surface_destroy(wl_surface);
    if(registry)
        wl_registry_destroy(registry);
    [super dealloc];
}

-(struct wl_surface *)wl_surface
{
    return wl_surface;
}

-(void) set_wm_base:(struct xdg_wm_base *)base
{
    wm_base = base;
}

-(void) set_compositor:(struct wl_compositor *)comp
{
    compositor = comp;
}

-(unsigned) styleMask
{
    return _styleMask;
}

-(void) setDelegate:delegate
{
    _delegate=delegate;
}

- delegate
{
    return _delegate;
}

-(void) invalidate
{
    [_delegate platformWindowDidInvalidateCGContext:self];
    _delegate=nil;
    _context=nil;

    [_display setWindow:nil forID:(uintptr_t)wl_surface];
}


-(void)createCGLContextObjIfNeeded {
   if(_cglContext==NULL){
    CGLError error;
    
    if((error=CGLCreateContextForWindow(NULL,NULL,&_cglContext,(uintptr_t)self))!=kCGLNoError)
     NSLog(@"CGLCreateContextForWindow failed at %s %d with error %d",__FILE__,__LINE__,error);
   }
   if(_cglContext!=NULL && _caContext==NULL){
    _caContext=[[CAWindowOpenGLContext alloc] initWithCGLContext:_cglContext];
   }
}

-(O2Context *) createCGContextIfNeeded
{
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

-(O2Context *) createBackingCGContextIfNeeded
{
    return nil;
}

-(O2Context *) cgContext
{
    return [self createCGContextIfNeeded];
}

-(void) invalidateContextsWithNewSize:(NSSize)size forceRebuild:(BOOL)forceRebuild
{
    if(!NSEqualSizes(_frame.size,size) || forceRebuild) {
        NSSize oldSize = _frame.size;
        _frame.size = size;

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
        //[self decorateWindow];
        [_delegate platformWindowDidInvalidateCGContext:self];
        CGLSurfaceResize(_cglContext, size.width, size.height);
        currentContext = nil;
    }
}

-(void) invalidateContextsWithNewSize:(NSSize)size
{
    [self invalidateContextsWithNewSize:size forceRebuild:NO];
}

-(void) setTitle:(NSString *)title
{
    xdg_toplevel_set_title(xdg_toplevel, [title UTF8String]);
}

-(BOOL) setProperty:(NSString *)property toValue:(NSString *)value
{
    return YES;
}

-(void) setFrame:(O2Rect)frame
{
    [self invalidateContextsWithNewSize:frame.size];
    // move window
    _frame = frame;
}

-(void) setLevel:(int)value
{
    _level = value;
}

-(void) showWindowForAppActivation:(O2Rect)frame
{
    NSUnimplementedMethod();
}

-(void) hideWindowForAppDeactivation:(O2Rect)frame
{
    NSUnimplementedMethod();
}

-(void) hideWindow
{
    _mapped=NO;
}

-(void) placeAboveWindow:(int)otherNumber
{
    // map and stack order
}

-(void) placeBelowWindow:(int)otherNumber
{
    // map and stack order
}

-(void) makeKey
{
    // map and stack order
}

-(void) makeMain
{
    // map and stack order
}

-(void) captureEvents
{
    // FIXME: find out what this is supposed to do
}

-(void) miniaturize
{
    NSUnimplementedMethod();
}

-(void) deminiaturize
{
    NSUnimplementedMethod();
}

-(BOOL) isMiniaturized
{
    return NO;
}


-(void) openGLFlushBuffer
{
    if(! _ready)
        return;

    CGLError error;
    CGLContextObj prevContext = CGLGetCurrentContext();
   
    [self createCGLContextObjIfNeeded];
    if(_caContext == NULL)
        return;

    O2Surface *surface = [_context surface];
    size_t width = _frame.size.width;
    size_t height = _frame.size.height;
    size_t stride = O2ImageGetBytesPerRow(surface);
    size_t size = stride * height;

    [_caContext prepareViewportWidth:width height:height];
    [_caContext renderSurface:surface];
    CGLFlushDrawable(_cglContext);

    CGLSetCurrentContext(prevContext);
}

-(void) flushBuffer
{
    O2ContextFlush(_context);
    [self openGLFlushBuffer];
}

// This seems wrong but it's exactly what was done in the Win32 version
-(NSPoint) mouseLocationOutsideOfEventStream
{
#if notyet
    Window window;
    int rootX, rootY, winX, winY;
    unsigned int mask;

    BOOL result = XQueryPointer(_display, DefaultRootWindow(_display),
        &window, &window, &rootX, &rootY, &winX, &winY, &mask);
    if(result == YES) {
        return [self transformPoint:NSMakePoint(rootX, rootY)];
    }
#endif
    NSLog(@"-[WLWindow mouseLocationOutsideOfEventStream] unable to locate mouse pointer");
    return NSMakePoint(0,0);
}


-(O2Rect) frame
{
    CGRect rect = CGInsetRectForNativeWindowBorder(_frame,_styleMask);
    return rect;
}

-(void) addEntriesToDeviceDictionary:(NSDictionary *)entries
{
    [_deviceDictionary addEntriesFromDictionary:entries];
}

- (NSPoint)transformPoint:(NSPoint)pos
{
    return pos;
}

- (O2Rect)transformFrame:(O2Rect)frame
{
    return frame;
}

- (void)frameChanged
{
    [self invalidateContextsWithNewSize:_frame.size];
}

- (void)setReady:(BOOL)ready
{
    _ready = ready;
    if(_ready) {
        struct wl_callback *cb = wl_surface_frame(wl_surface);
        wl_callback_add_listener(cb, &frame_listener, (__bridge void *)self);
    }
}

- (BOOL)isReady
{
    return _ready;
}

@end

CGRect CGInsetRectForNativeWindowBorder(CGRect frame,unsigned styleMask)
{
    CGFloat top,left,bottom,right;
    
    CGNativeBorderFrameWidthsForStyle(styleMask,&top,&left,&bottom,&right);
    
    frame.origin.x+=left;
    frame.origin.y+=bottom;
    frame.size.width-=left+right;
    frame.size.height-=top+bottom;
    
    return frame;
}

CGRect CGOutsetRectForNativeWindowBorder(CGRect frame,unsigned styleMask)
{
    CGFloat top,left,bottom,right;
    
    CGNativeBorderFrameWidthsForStyle(styleMask,&top,&left,&bottom,&right);
    
    frame.origin.x-=left;
    frame.origin.y-=bottom;
    frame.size.width+=left+right;
    frame.size.height+=top+bottom;
    
    return frame;
}
