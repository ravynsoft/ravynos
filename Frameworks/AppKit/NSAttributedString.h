/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>
#import <AppKit/AppKitExport.h>
#import <AppKit/NSStringDrawing.h>

@class NSFont, NSColor, NSParagraphStyle, NSTextAttachment, NSFileWrapper, NSTextList, NSTextBlock, NSTextTable;

APPKIT_EXPORT NSString *const NSFontAttributeName;
APPKIT_EXPORT NSString *const NSParagraphStyleAttributeName;
APPKIT_EXPORT NSString *const NSForegroundColorAttributeName;
APPKIT_EXPORT NSString *const NSBackgroundColorAttributeName;
APPKIT_EXPORT NSString *const NSUnderlineStyleAttributeName;
APPKIT_EXPORT NSString *const NSUnderlineColorAttributeName;
APPKIT_EXPORT NSString *const NSAttachmentAttributeName;
APPKIT_EXPORT NSString *const NSKernAttributeName;
APPKIT_EXPORT NSString *const NSLigatureAttributeName;
APPKIT_EXPORT NSString *const NSStrikethroughStyleAttributeName;
APPKIT_EXPORT NSString *const NSStrikethroughColorAttributeName;
APPKIT_EXPORT NSString *const NSObliquenessAttributeName;
APPKIT_EXPORT NSString *const NSStrokeWidthAttributeName;
APPKIT_EXPORT NSString *const NSStrokeColorAttributeName;
APPKIT_EXPORT NSString *const NSBaselineOffsetAttributeName;
APPKIT_EXPORT NSString *const NSSuperscriptAttributeName;
APPKIT_EXPORT NSString *const NSLinkAttributeName;
APPKIT_EXPORT NSString *const NSShadowAttributeName;
APPKIT_EXPORT NSString *const NSExpansionAttributeName;
APPKIT_EXPORT NSString *const NSCursorAttributeName;
APPKIT_EXPORT NSString *const NSToolTipAttributeName;
APPKIT_EXPORT NSString *const NSBackgroundColorDocumentAttribute;

APPKIT_EXPORT NSString *const NSSpellingStateAttributeName;

enum {
    NSSpellingStateSpellingFlag = 0x01,
    NSSpellingStateGrammarFlag = 0x02,
};

enum {
    NSUnderlineStyleNone,
    NSUnderlineStyleSingle,
    NSUnderlineStyleThick,
    NSUnderlineStyleDouble,
};

// Deprecated constants
enum {
    NSNoUnderlineStyle = NSUnderlineStyleNone,
    NSSingleUnderlineStyle = NSUnderlineStyleSingle,
};

enum {
    NSUnderlinePatternSolid = 0x000,
    NSUnderlinePatternDot = 0x100,
    NSUnderlinePatternDash = 0x200,
    NSUnderlinePatternDashDot = 0x300,
    NSUnderlinePatternDashDotDot = 0x400,
};

@interface NSAttributedString (NSAttributedString_AppKit)

#pragma mark -
#pragma mark Creating an NSAttributedString

+ (NSAttributedString *)attributedStringWithAttachment:(NSTextAttachment *)attachment;

- initWithData:(NSData *)data options:(NSDictionary *)options documentAttributes:(NSDictionary **)attributes error:(NSError **)error;
- initWithDocFormat:(NSData *)werd documentAttributes:(NSDictionary **)attributes;

- initWithHTML:(NSData *)html baseURL:(NSURL *)url documentAttributes:(NSDictionary **)attributes;
- initWithHTML:(NSData *)html documentAttributes:(NSDictionary **)attributes;
- initWithHTML:(NSData *)html options:(NSDictionary *)options documentAttributes:(NSDictionary **)attributes;
- initWithPath:(NSString *)path documentAttributes:(NSDictionary **)attributes;

- initWithRTF:(NSData *)rtf documentAttributes:(NSDictionary **)attributes;

- initWithRTFD:(NSData *)rtfd documentAttributes:(NSDictionary **)attributes;
- initWithRTFDFileWrapper:(NSFileWrapper *)wrapper documentAttributes:(NSDictionary **)attributes;

- initWithURL:(NSURL *)url documentAttributes:(NSDictionary **)attributes;
- initWithURL:(NSURL *)url options:(NSDictionary *)options documentAttributes:(NSDictionary **)attributes error:(NSError **)error;

#pragma mark -
#pragma mark Retrieving Font Attribute Information

- (BOOL)containsAttachments;
- (NSDictionary *)fontAttributesInRange:(NSRange)range;
- (NSDictionary *)rulerAttributesInRange:(NSRange)range;

#pragma mark -
#pragma mark Calculating Linguistic Units

- (NSRange)doubleClickAtIndex:(unsigned)index;
- (unsigned)lineBreakBeforeIndex:(unsigned)index withinRange:(NSRange)range;
- (unsigned)lineBreakByHyphenatingBeforeIndex:(unsigned)index withinRange:(NSRange)range;
- (unsigned)nextWordFromIndex:(unsigned)index forward:(BOOL)forward;

#pragma mark -
#pragma mark Calculating Ranges

- (int)itemNumberInTextList:(NSTextList *)list atIndex:(unsigned)index;
- (NSRange)rangeOfTextBlock:(NSTextBlock *)block atIndex:(unsigned)index;
- (NSRange)rangeOfTextList:(NSTextList *)list atIndex:(unsigned)index;
- (NSRange)rangeOfTextTable:(NSTextTable *)table atIndex:(unsigned)index;

#pragma mark -
#pragma mark Generating Data

- (NSData *)dataFromRange:(NSRange)range documentAttributes:(NSDictionary *)attributes error:(NSError **)error;
- (NSFileWrapper *)fileWrapperFromRange:(NSRange)range documentAttributes:(NSDictionary *)attributes error:(NSError **)error;
- (NSData *)docFormatFromRange:(NSRange)range documentAttributes:(NSDictionary *)attributes;
- (NSData *)RTFFromRange:(NSRange)range documentAttributes:(NSDictionary *)attributes;
- (NSData *)RTFDFromRange:(NSRange)range documentAttributes:(NSDictionary *)attributes;
- (NSFileWrapper *)RTFDFileWrapperFromRange:(NSRange)range documentAttributes:(NSDictionary *)attributes;

#pragma mark -
#pragma mark Drawing the String

- (void)drawAtPoint:(NSPoint)point;
- (void)drawInRect:(NSRect)rect;
- (void)drawWithRect:(NSRect)rect options:(NSStringDrawingOptions)options;
- (NSSize)size;

#pragma mark -
#pragma mark Getting the Bounding Rectangle of Rendered Strings

- (NSRect)boundingRectWithSize:(NSSize)size options:(NSStringDrawingOptions)options;

#pragma mark -
#pragma mark Testing String Data Sources

+ (NSArray *)textTypes;
+ (NSArray *)textUnfilteredTypes;

#pragma mark -
#pragma mark Deprecated in 10.5

+ (NSArray *)textFileTypes;
+ (NSArray *)textPasteboardTypes;

+ (NSArray *)textUnfilteredFileTypes;
+ (NSArray *)textUnfilteredPasteboardTypes;

@end

#pragma mark -
#pragma mark Private

NSFont *NSFontAttributeInDictionary(NSDictionary *dictionary);
NSColor *NSForegroundColorAttributeInDictionary(NSDictionary *dictionary);
NSParagraphStyle *NSParagraphStyleAttributeInDictionary(NSDictionary *dictionary);

#import <AppKit/NSMutableAttributedString.h>
