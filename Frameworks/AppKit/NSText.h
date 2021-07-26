/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSView.h>
#import <AppKit/NSSpellProtocol.h>

@class NSColor, NSFont;

enum {
    NSEnterCharacter = 0x0003,
    NSBackspaceCharacter = 0x0008,
    NSTabCharacter = 0x0009,
    NSNewlineCharacter = 0x000A,
    NSFormFeedCharacter = 0x000C,
    NSCarriageReturnCharacter = 0x000D,
    NSBackTabCharacter = 0x0019,
    NSDeleteCharacter = 0x007F,

    NSLineSeparatorCharacter = 0x2028,
    NSParagraphSeparatorCharacter = 0x2029
};

typedef enum {
    NSLeftTextAlignment,
    NSRightTextAlignment,
    NSCenterTextAlignment,
    NSJustifiedTextAlignment,
    NSNaturalTextAlignment
} NSTextAlignment;

enum {
    NSIllegalTextMovement = 0x00,
    NSReturnTextMovement = 0x10,
    NSTabTextMovement = 0x11,
    NSBacktabTextMovement = 0x12,
    NSLeftTextMovement = 0x13,
    NSRightTextMovement = 0x14,
    NSUpTextMovement = 0x15,
    NSDownTextMovement = 0x16,
    NSCancelTextMovement = 0x17,
    NSOtherTextMovement = 0
};

typedef enum {
    NSWritingDirectionNatural = -1,
    NSWritingDirectionLeftToRight,
    NSWritingDirectionRightToLeft,
} NSWritingDirection;

APPKIT_EXPORT NSString *const NSTextDidBeginEditingNotification;
APPKIT_EXPORT NSString *const NSTextDidEndEditingNotification;
APPKIT_EXPORT NSString *const NSTextDidChangeNotification;

@interface NSText : NSView <NSChangeSpelling, NSIgnoreMisspelledWords>

- delegate;
- (NSString *)string;
- (NSData *)RTFFromRange:(NSRange)range;
- (NSData *)RTFDFromRange:(NSRange)range;

- (BOOL)isEditable;
- (BOOL)isSelectable;
- (BOOL)isRichText;
- (BOOL)isFieldEditor;
- (BOOL)usesFontPanel;
- (BOOL)importsGraphics;

- (NSFont *)font;
- (NSTextAlignment)alignment;

- (NSColor *)textColor;

- (BOOL)drawsBackground;
- (NSColor *)backgroundColor;

- (BOOL)isHorizontallyResizable;
- (BOOL)isVerticallyResizable;
- (NSSize)maxSize;
- (NSSize)minSize;

- (NSRange)selectedRange;

- (void)setDelegate:delegate;
- (void)setString:(NSString *)string;

- (void)replaceCharactersInRange:(NSRange)range withString:(NSString *)string;

- (BOOL)readRTFDFromFile:(NSString *)path;
- (void)replaceCharactersInRange:(NSRange)range withRTF:(NSData *)rtf;
- (void)replaceCharactersInRange:(NSRange)range withRTFD:(NSData *)rtfd;

- (void)setEditable:(BOOL)flag;
- (void)setSelectable:(BOOL)flag;
- (void)setRichText:(BOOL)flag;
- (void)setFieldEditor:(BOOL)flag;
- (void)setUsesFontPanel:(BOOL)value;
- (void)setImportsGraphics:(BOOL)value;

- (void)setFont:(NSFont *)font;
- (void)setFont:(NSFont *)font range:(NSRange)range;
- (void)setAlignment:(NSTextAlignment)alignment;

- (void)setTextColor:(NSColor *)color;
- (void)setTextColor:(NSColor *)color range:(NSRange)range;

- (void)setDrawsBackground:(BOOL)flag;
- (void)setBackgroundColor:(NSColor *)color;

- (void)setHorizontallyResizable:(BOOL)flag;
- (void)setVerticallyResizable:(BOOL)flag;
- (void)setMaxSize:(NSSize)size;
- (void)setMinSize:(NSSize)size;

- (void)setSelectedRange:(NSRange)range;

- (void)sizeToFit;

- (void)scrollRangeToVisible:(NSRange)range;

- (void)changeFont:(id)sender;
- (void)alignCenter:sender;
- (void)alignLeft:sender;
- (void)alignRight:sender;
- (void)underline:sender;
- (void)selectAll:sender;
- (void) delete:sender;
- (void)toggleRuler:sender;
- (void)copyRuler:sender;
- (void)pasteRuler:sender;

- (void)showGuessPanel:sender;
- (void)checkSpelling:sender;

@end

@interface NSObject (NSText_delegate)
- (BOOL)textShouldBeginEditing:(NSText *)text;
- (BOOL)textShouldEndEditing:(NSText *)text;

- (void)textDidBeginEditing:(NSNotification *)note;
- (void)textDidChange:(NSNotification *)note;
- (void)textDidEndEditing:(NSNotification *)note;
@end
