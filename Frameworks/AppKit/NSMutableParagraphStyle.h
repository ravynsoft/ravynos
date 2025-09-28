/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSParagraphStyle.h>

@interface NSMutableParagraphStyle : NSParagraphStyle

- (void)setParagraphStyle:(NSParagraphStyle *)other;

- (void)setBaseWritingDirection:(NSWritingDirection)direction;

- (void)setParagraphSpacing:(float)spacing;
- (void)setParagraphSpacingBefore:(float)spacing;

- (void)setTextBlocks:(NSArray *)blocks;
- (void)setTextLists:(NSArray *)lists;

- (void)setHeaderLevel:(int)level;

- (void)setFirstLineHeadIndent:(float)indent;
- (void)setHeadIndent:(float)indent;
- (void)setTailIndent:(float)indent;

- (void)setAlignment:(NSTextAlignment)alignment;
- (void)setLineBreakMode:(NSLineBreakMode)mode;

- (void)setMinimumLineHeight:(float)height;
- (void)setMaximumLineHeight:(float)height;
- (void)setLineHeightMultiple:(float)multiple;
- (void)setLineSpacing:(float)spacing;

- (void)setDefaultTabInterval:(float)interval;
- (void)setTabStops:(NSArray *)tabStops;

- (void)setHyphenationFactor:(float)factor;
- (void)setTighteningFactorForTruncation:(float)factor;
@end
