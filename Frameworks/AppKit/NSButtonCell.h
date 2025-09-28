/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSActionCell.h>
#import <AppKit/NSImageCell.h>

@class NSSound;

enum {
    NSNoCellMask = 0x00,
    NSContentsCellMask = 0x01,
    NSPushInCellMask = 0x02,
    NSChangeGrayCellMask = 0x04,
    NSChangeBackgroundCellMask = 0x08,
};

typedef enum {
    NSMomentaryLightButton = 0,
    NSPushOnPushOffButton = 1,
    NSToggleButton = 2,
    NSSwitchButton = 3,
    NSRadioButton = 4,
    NSMomentaryChangeButton = 5,
    NSOnOffButton = 6,
    NSMomentaryPushInButton = 7,
    // deprecated values
    NSMomentaryPushButton = 0,
    NSMomentaryLight = 7
} NSButtonType;

typedef enum {
    NSRoundedBezelStyle = 1,
    NSRegularSquareBezelStyle = 2,
    NSThickSquareBezelStyle = 3,
    NSThickerSquareBezelStyle = 4,
    NSDisclosureBezelStyle = 5,
    NSShadowlessSquareBezelStyle = 6,
    NSCircularBezelStyle = 7,
    NSTexturedSquareBezelStyle = 8,
    NSHelpButtonBezelStyle = 9,
    NSSmallSquareBezelStyle = 10,
    NSTexturedRoundedBezelStyle = 11,
    NSRoundRectBezelStyle = 12,
    NSRecessedBezelStyle = 13,
    NSRoundedDisclosureBezelStyle = 14,
} NSBezelStyle;

typedef enum {
    NSGradientNone = 0,
    NSGradientConcaveWeak = 1,
    NSGradientConcaveStrong = 2,
    NSGradientConvexWeak = 3,
    NSGradientConvexStrong = 4,
} NSGradientType;

@interface NSButtonCell : NSActionCell {
    NSImage *_normalImage;
    NSString *_alternateTitle;
    NSImage *_alternateImage;
    int _imagePosition;
    unsigned _highlightsBy : 4;
    unsigned _showsStateBy : 4;
    NSBezelStyle _bezelStyle;
    BOOL _isTransparent;
    BOOL _imageDimsWhenDisabled;
    NSString *_keyEquivalent;
    unsigned _keyEquivalentModifierMask;
    BOOL _showsBorderOnlyWhileMouseInside;
    NSSound *_sound;
    NSGradientType _gradientType;
    NSImageScaling _imageScaling;
    NSFont *_keyEquivalentFont;
    NSColor *_backgroundColor;
    float _periodicDelay, _periodicInterval;
}

- (BOOL)isTransparent;
- (NSString *)keyEquivalent;
- (NSCellImagePosition)imagePosition;
- (NSString *)title;
- (NSString *)alternateTitle;
- (NSImage *)alternateImage;
- (NSAttributedString *)attributedTitle;
- (NSAttributedString *)attributedAlternateTitle;
- (int)highlightsBy;
- (int)showsStateBy;
- (BOOL)imageDimsWhenDisabled;
- (unsigned)keyEquivalentModifierMask;
- (NSBezelStyle)bezelStyle;
- (BOOL)showsBorderOnlyWhileMouseInside;
- (NSSound *)sound;
- (NSGradientType)gradientType;
- (NSImageScaling)imageScaling;
- (BOOL)isOpaque;
- (NSFont *)keyEquivalentFont;
- (NSColor *)backgroundColor;
- (void)getPeriodicDelay:(float *)delay interval:(float *)interval;

- (void)setTransparent:(BOOL)flag;
- (void)setKeyEquivalent:(NSString *)keyEquivalent;
- (void)setImagePosition:(NSCellImagePosition)position;
- (void)setTitle:(NSString *)title;
- (void)setAlternateTitle:(NSString *)title;
- (void)setAlternateImage:(NSImage *)image;
- (void)setAttributedTitle:(NSAttributedString *)title;
- (void)setAttributedAlternateTitle:(NSAttributedString *)title;
- (void)setHighlightsBy:(int)type;
- (void)setShowsStateBy:(int)type;
- (void)setImageDimsWhenDisabled:(BOOL)flag;
- (void)setKeyEquivalentModifierMask:(unsigned)mask;
- (void)setBezelStyle:(NSBezelStyle)bezelStyle;
- (void)setButtonType:(NSButtonType)buttonType;
- (void)setShowsBorderOnlyWhileMouseInside:(BOOL)show;
- (void)setSound:(NSSound *)sound;
- (void)setGradientType:(NSGradientType)value;
- (void)setBackgroundColor:(NSColor *)value;
- (void)setImageScaling:(NSImageScaling)value;
- (void)setKeyEquivalentFont:(NSFont *)value;
- (void)setKeyEquivalentFont:(NSString *)value size:(CGFloat)size;
- (void)setPeriodicDelay:(float)delay interval:(float)interval;

- (void)performClick:sender;

- (void)drawBezelWithFrame:(NSRect)rect inView:(NSView *)view;
- (void)drawImage:(NSImage *)image withFrame:(NSRect)rect inView:(NSView *)view;
- (NSRect)drawTitle:(NSAttributedString *)title withFrame:(NSRect)rect inView:(NSView *)view;

- (void)mouseEntered:(NSEvent *)event;
- (void)mouseExited:(NSEvent *)event;

@end
