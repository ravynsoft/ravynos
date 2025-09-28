/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSControl.h>
#import <AppKit/NSButtonCell.h>

@interface NSButton : NSControl

- (BOOL)isTransparent;
- (NSString *)keyEquivalent;
- (NSUInteger)keyEquivalentModifierMask;
- (NSImage *)image;
- (NSCellImagePosition)imagePosition;
- (NSString *)title;
- (NSInteger)state;
- (BOOL)allowsMixedState;
- (NSSound *)sound;

- (NSBezelStyle)bezelStyle;
- (NSString *)alternateTitle;
- (NSImage *)alternateImage;
- (NSAttributedString *)attributedTitle;
- (NSAttributedString *)attributedAlternateTitle;
- (BOOL)showsBorderOnlyWhileMouseInside;
- (void)getPeriodicDelay:(float *)delay interval:(float *)interval;

- (void)setTransparent:(BOOL)value;
- (void)setKeyEquivalent:(NSString *)value;
- (void)setKeyEquivalentModifierMask:(NSUInteger)value;
- (void)setImage:(NSImage *)value;
- (void)setImagePosition:(NSCellImagePosition)value;
- (void)setTitle:(NSString *)value;
- (void)setState:(NSInteger)value;
- (void)setNextState;
- (void)setAllowsMixedState:(BOOL)value;
- (void)setSound:(NSSound *)value;

- (void)setBezelStyle:(NSBezelStyle)value;
- (void)setAlternateTitle:(NSString *)value;
- (void)setAlternateImage:(NSImage *)value;
- (void)setAttributedTitle:(NSAttributedString *)value;
- (void)setAttributedAlternateTitle:(NSAttributedString *)value;
- (void)setShowsBorderOnlyWhileMouseInside:(BOOL)value;
- (void)setPeriodicDelay:(float)delay interval:(float)interval;

- (void)setButtonType:(NSButtonType)value;
- (void)setTitleWithMnemonic:(NSString *)value;

- (void)highlight:(BOOL)value;
- (BOOL)performKeyEquivalent:(NSEvent *)event;
- (void)performClick:sender;

@end
