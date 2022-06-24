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
#import <Foundation/NSRaise.h>
#import <CoreGraphics/CGWindow.h>
#import <Onyx2D/O2Geometry.h>
#import <Onyx2D/O2Path.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "xdg-output-management-unstable-v1-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"
#import "WLDisplay.h"

@class CAWindowOpenGLContext;

@interface WLWindow : CGWindow {
    int _level;
    CGLContextObj _cglContext;
    CAWindowOpenGLContext *_caContext;
    WLDisplay *_display;

    struct wl_compositor *compositor;
    struct wl_registry *registry;
    struct xdg_wm_base *wm_base;
    struct wl_surface *wl_surface;
    struct wl_seat *wl_seat;
    struct xdg_toplevel *xdg_toplevel;
    struct xdg_surface *xdg_surface; 
    struct wl_callback *cb;

    // subsurface support
    struct wl_subcompositor *subcompositor;
    struct wl_subsurface *wl_subsurface;
    id parentWindow;

    // wlroots layer shell support
    struct zwlr_layer_shell_v1 *layer_shell;
    struct zwlr_layer_surface_v1 *layer_surface;
    uint32_t layerType;
    uint32_t anchorType;
    NSRect margins;
    uint32_t exclusiveZone;
    double layerAlpha;

    // decorations support
    struct zxdg_decoration_manager_v1 *decorationManager;
    struct zxdg_toplevel_decoration_v1 *decoration;
    enum zxdg_toplevel_decoration_v1_mode preferredMode;
    enum zxdg_toplevel_decoration_v1_mode currentMode;

    id _delegate;
    CGSBackingStoreType _backingType;
    O2Context *_context;

    NSMutableDictionary *_deviceDictionary;
    O2Rect _frame;
    unsigned _styleMask;
    BOOL _mapped;
    BOOL _ready;
}

- initWithFrame:(NSRect)frame styleMask:(unsigned)styleMask
        isPanel:(BOOL)isPanel backingType:(NSUInteger)backingType;
- initWithFrame:(NSRect)frame styleMask:(unsigned)styleMask
        isPanel:(BOOL)isPanel backingType:(NSUInteger)backingType
        output:(struct wl_output *)wlo;
- (O2Rect)frame;
- (NSPoint)transformPoint:(NSPoint)pos;
- (O2Rect)transformFrame:(O2Rect)frame;
- (BOOL)setProperty:(NSString *)property toValue:(NSString *)value;
- (void)setExclusiveZone:(uint32_t)pixels;
- (void)setKeyboardInteractivity:(uint32_t)keyboardStyle;
- (O2Context *) createCGContextIfNeeded;
- (void) frameChanged;
- (struct wl_surface *)wl_surface;
- (void) set_wm_base:(struct xdg_wm_base *)base;
- (void) set_compositor:(struct wl_compositor *)comp;
- (void) set_subcompositor:(struct wl_subcompositor *)comp;
- (void) setReady:(BOOL)ready;
- (BOOL) isReady;

@end

void CGNativeBorderFrameWidthsForStyle(unsigned styleMask, CGFloat *top, CGFloat *left,
                                       CGFloat *bottom, CGFloat *right);
