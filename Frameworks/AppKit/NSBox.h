/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSView.h>

@class NSFont;

typedef enum {
    NSNoTitle,
    NSAboveTop,
    NSAtTop,
    NSBelowTop,
    NSAboveBottom,
    NSAtBottom,
    NSBelowBottom,
} NSTitlePosition;

typedef enum {
    NSBoxPrimary,
    NSBoxSecondary,
    NSBoxSeparator,
    NSBoxOldStyle,
    NSBoxCustom
} NSBoxType;

@interface NSBox : NSView {
    NSBoxType _boxType;
    NSBorderType _borderType;
    NSTitlePosition _titlePosition;
    id _titleCell;
    NSSize _contentViewMargins;
    BOOL _isTransparent;
    id _customData;
}

- (NSBoxType)boxType;
- (NSBorderType)borderType;
- (NSString *)title;
- (NSFont *)titleFont;
- contentView;
- (NSSize)contentViewMargins;
- (NSTitlePosition)titlePosition;
- (BOOL)isTransparent;

- (void)setBoxType:(NSBoxType)value;
- (void)setBorderType:(NSBorderType)value;
- (void)setTitle:(NSString *)title;
- (void)setTitleFont:(NSFont *)font;
- (void)setContentView:(NSView *)view;
- (void)setContentViewMargins:(NSSize)value;
- (void)setTitlePosition:(NSTitlePosition)value;
- (void)setTransparent:(BOOL)value;
- (void)setTitleWithMnemonic:(NSString *)value;

- (NSRect)titleRect;
- (NSRect)borderRect;
- titleCell;

- (void)setFrameFromContentFrame:(NSRect)content;
- (void)sizeToFit;

- (CGFloat)borderWidth;
- (CGFloat)cornerRadius;
- (NSColor *)borderColor;
- (NSColor *)fillColor;

- (void)setBorderWidth:(CGFloat)value;
- (void)setCornerRadius:(CGFloat)value;
- (void)setBorderColor:(NSColor *)value;
- (void)setFillColor:(NSColor *)value;

@end
