/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>
#import <CoreFoundation/CFBase.h>
#ifdef WIN32
#import <windows.h>
#endif
@class NSStatusBar, NSImage, NSAttributedString, NSMenu, NSView, NSWindow;
@interface NSStatusItem : NSObject {
    SEL _action;
    SEL _doubleAction;
    id _target;
    //Image-Based Item Vars
    NSImage *_image;
    NSImage *_alternateImage;
    //Text-Based Item Vars
    NSString *_title;
    NSAttributedString *_atrTitle;
    //View-Based Item Vars
    NSView *_view;
    //Other Vars
    BOOL _highlightMode;
    BOOL _enabled;
    CGFloat _length;
    NSMenu *_menu;
    NSInteger _actionMask;
#ifdef WIN32
    int _trayIconID;
    HICON _trayIcon;
    HMENU _win32Menu;
#endif
    uint32_t _handle; // reference to this item in our global status bar
}
- (NSStatusBar *)statusBar;

- (SEL)action;
- (void)setAction:(SEL)action;
- (SEL)doubleAction;
- (void)setDoubleAction:(SEL)action;
- (id)target;
- (void)setTarget:(id)target;

- (NSImage *)image;
- (void)setImage:(NSImage *)image;
- (NSImage *)alternateImage;
- (void)setAlternateImage:(NSImage *)image;

- (NSString *)title;
- (void)setTitle:(NSString *)title;
- (void)setToolTip:(NSString *)toolTip;
- (NSAttributedString *)attributedTitle;
- (void)setAttributedTitle:(NSAttributedString *)title;

- (NSView *)view;
- (void)setView:(NSView *)view;

- (BOOL)highlightMode;
- (void)setHighlightMode:(BOOL)flag;

- (BOOL)isEnabled;
- (void)setEnabled:(BOOL)flag;

- (CGFloat)length;
- (void)setLength:(CGFloat)len;

- (NSMenu *)menu;
- (void)setMenu:(NSMenu *)menu;

- (void)popUpStatusItemMenu:(NSMenu *)menu;
- (NSInteger)sendActionOn:(NSInteger)mask;

- (void)drawStatusBarBackgroundInRect:(NSRect)rect withHighlight:(BOOL)highlight;

@end
