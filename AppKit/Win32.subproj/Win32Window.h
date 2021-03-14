/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <CoreGraphics/CGWindow.h>
#import <OpenGL/OpenGL.h>
#import <Onyx2D/O2Geometry.h>

#undef WINVER
#define WINVER 0x501 // XP drop shadow constants
#define WM_NSTRAYACTIVATE 9001
#import <windows.h>

@class O2Context_gdi, O2Surface, O2Surface_DIBSection;

@interface Win32Window : CGWindow {
    CRITICAL_SECTION _lock; // This must be a recursive lock.
    CGRect _frame;
    int _level;
    BOOL _isOpaque;
    BOOL _hasShadow;
    CGFloat _alphaValue;
    HWND _handle;
    HGLRC _hglrc;
    GLint _backingTextureId;
    BOOL _hasRenderTexture;
    BOOL _hasMakeCurrentRead;
    BOOL _hasReadback;
    BOOL _hasSwapHintRect;
    BOOL _reloadBackingTexture;
    CGFloat _borderTop, _borderLeft, _borderBottom, _borderRight;
    O2Context_gdi *_cgContext;

    CGSBackingStoreType _backingType;
    O2Context *_backingContext;

    int _surfaceCount;
    void **_surfaces;

    int _textureIdCount;
    GLint *_textureIds;

    O2Surface_DIBSection *_overlayResult;
    NSMutableArray *_overlays;

    int _disableFlushWindow;
    NSString *_title;
    BOOL _isLayered;
    BOOL _ignoreMinMaxMessage;
    BOOL _sentBeginSizing;
    BOOL _isClosing;
    BOOL _disableDisplay;
    unsigned _styleMask;
    BOOL _isPanel;
    BOOL _isMiniaturized;

    id _delegate;
    NSMutableDictionary *_deviceDictionary;

    int     _dirtyRectCap, _dirtyRectCnt;
    NSRect *_dirtyRectSet;
}

- initWithFrame:(CGRect)frame styleMask:(unsigned)styleMask isPanel:(BOOL)isPanel backingType:(CGSBackingStoreType)backingType;

- (void)setDelegate:delegate;
- delegate;

- (void)invalidate;

- (void)lock;
- (void)unlock;

- (HWND)windowHandle;
- (CGRect)frame;

- (void)showWindowForAppActivation:(CGRect)frame;
- (void)hideWindowForAppDeactivation:(CGRect)frame;

- (void)hideWindow;
- (void)showWindowWithoutActivation;
- (void)bringToTop;
- (void)makeTransparent;
- (void)placeAboveWindow:(Win32Window *)other;
- (void)placeBelowWindow:(Win32Window *)other;

- (void)makeKey;
- (void)captureEvents;
- (void)miniaturize;
- (void)deminiaturize;
- (BOOL)isMiniaturized;

- (BOOL)isLayeredWindow;

- (void)flushBuffer;

- (NSPoint)convertPOINTLToBase:(POINTL)point;
- (NSPoint)mouseLocationOutsideOfEventStream;

- (void)adjustEventLocation:(NSPoint *)location childWindow:(BOOL)childWindow;

- (void)sendEvent:(CGEvent *)event;

- (int)WM_SETCURSOR_wParam:(WPARAM)wParam lParam:(LPARAM)lParam;
- (int)WM_APP1_wParam:(WPARAM)wParam lParam:(LPARAM)lParam;

@end

void CGNativeBorderFrameWidthsForStyle(unsigned styleMask, CGFloat *top, CGFloat *left, CGFloat *bottom, CGFloat *right);

static inline CGRect CGRectFromRECT(RECT rect) {
    CGRect result;

    result.origin.x = rect.left;
    result.origin.y = rect.top;
    result.size.width = rect.right - rect.left;
    result.size.height = rect.bottom - rect.top;

    return result;
}

static inline RECT RECTFromCGRect(CGRect rect) {
    RECT result;

    result.top = rect.origin.y;
    result.left = rect.origin.x;
    result.bottom = result.top + rect.size.height;
    result.right = result.left + rect.size.width;

    return result;
}
