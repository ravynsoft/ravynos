/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
   Copyright (C) 2023 Zoe Knox <zoe@ravynsoft.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>
#import <CoreFoundation/CFBase.h>
#ifdef WIN32
#import <windows.h>
#endif

typedef NSString *NSStatusItemAutosaveName; // 10.13+
typedef enum NSStatusItemBehavior : NSUInteger {
    NSStatusItemBehaviorRemovalAllowed = (1 << 1),
    NSStatusItemBehaviorTerminationOnRemoval = (1 << 2)
} NSStatusItemBehavior;

@class NSStatusBar, NSImage, NSAttributedString, NSMenu, NSView, NSWindow;
@interface NSStatusItem : NSObject <NSCopying> {
    CGFloat _length;
    NSMenu *_menu;

    // ravynOS internal: reference to this item in our global status bar 
    uint32_t _handle; 

    // These were deprecated in macOS 10.15
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
    NSInteger _actionMask;
#ifdef WIN32
    int _trayIconID;
    HICON _trayIcon;
    HMENU _win32Menu;
#endif
}

@property(assign, getter=isVisible) BOOL visible;
@property(copy) NSStatusItemAutosaveName autosaveName;
@property(assign) enum NSStatusItemBehavior behavior;
@property(copy) NSString *toolTip;

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
