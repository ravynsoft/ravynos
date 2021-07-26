/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSPanel.h>
#import <AppKit/NSNibLoading.h>

@class NSButton, NSColorWell, NSMatrix, NSView, NSColorList, NSSplitView, NSTextField, NSSlider;

enum {
    NSColorPanelGrayModeMask = (1 << 0),
    NSColorPanelRGBModeMask = (1 << 1),
    NSColorPanelCMYKModeMask = (1 << 2),
    NSColorPanelHSBModeMask = (1 << 3),
    NSColorPanelCustomPaletteModeMask = (1 << 4),
    NSColorPanelColorListModeMask = (1 << 5),
    NSColorPanelWheelModeMask = (1 << 6),
    NSColorPanelCrayonModeMask = (1 << 7),
    NSColorPanelAllModesMask = 0xFFFF,
};

enum {
    NSNoModeColorPanel = -1,
    NSGrayModeColorPanel = 0,
    NSRGBModeColorPanel = 1,
    NSCMYKModeColorPanel = 2,
    NSHSBModeColorPanel = 3,
    NSCustomPaletteModeColorPanel = 4,
    NSColorListModeColorPanel = 5,
    NSWheelModeColorPanel = 6,
    NSCrayonModeColorPanel = 7,
};

typedef NSInteger NSColorPanelMode;

APPKIT_EXPORT NSString *const NSColorPanelColorDidChangeNotification;

@interface NSColorPanel : NSPanel {
    NSButton *setColorButton;
    NSColorWell *colorWell;
    NSMatrix *colorPickersMatrix;
    NSView *colorPickerView;
    NSView *currentColorPickerView;
    NSView *swatchView;
    NSSplitView *splitView;

    IBOutlet NSTextField *opacityTitle;
    IBOutlet NSSlider *opacitySlider;
    IBOutlet NSTextField *opacityTextField;
    IBOutlet NSTextField *opacityPercentLabel;

    BOOL _showsAlpha;
    BOOL _continuous;
    int _mode;
    NSMutableArray *_colorPickers;
    id _target;
    SEL _action;
}

+ (BOOL)sharedColorPanelExists;
+ (NSColorPanel *)sharedColorPanel;

+ (void)setPickerMask:(NSUInteger)mask;
+ (void)setPickerMode:(NSColorPanelMode)mode;

+ (BOOL)dragColor:(NSColor *)color withEvent:(NSEvent *)event fromView:(NSView *)view;

- (NSColor *)color;
- (CGFloat)alpha;
- (NSColorPanelMode)mode;
- (BOOL)showsAlpha;
- (BOOL)isContinuous;
- (NSView *)accessoryView;

- (void)setColor:(NSColor *)color;
- (void)setMode:(NSColorPanelMode)mode;
- (void)setShowsAlpha:(BOOL)flag;
- (void)setContinuous:(BOOL)flag;
- (void)setAccessoryView:(NSView *)view;

- (void)setAction:(SEL)action;
- (void)setTarget:target;

- (void)attachColorList:(NSColorList *)colorList;
- (void)detachColorList:(NSColorList *)colorList;

@end

@interface NSObject (NSColorPanel_responder)
- (void)changeColor:sender;
@end
