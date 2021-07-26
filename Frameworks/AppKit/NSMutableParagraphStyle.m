/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSMutableParagraphStyle.h>

@interface NSParagraphStyle(NSParagraphStyle_private)
-initWithParagraphStyle:(NSParagraphStyle *)other;
@end

@implementation NSMutableParagraphStyle

+(NSParagraphStyle *)defaultParagraphStyle {
   static NSParagraphStyle *shared=nil;

   if(shared==nil)
    shared=[self new];

   return shared;
}

-copyWithZone:(NSZone *)zone {
   return [[NSParagraphStyle alloc] initWithParagraphStyle:self];
}

-(void)setParagraphStyle:(NSParagraphStyle *)other {
   _writingDirection=other->_writingDirection;
   _paragraphSpacing=other->_paragraphSpacing;
   _paragraphSpacingBefore=other->_paragraphSpacingBefore;
   [self setTextBlocks:[other textBlocks]];
   [self setTextLists:[other textLists]];
   _headerLevel=other->_headerLevel;
   _firstLineHeadIndent=other->_firstLineHeadIndent;
   _headIndent=other->_headIndent;
   _tailIndent=other->_tailIndent;
   _alignment=other->_alignment;
   _lineBreakMode=other->_lineBreakMode;
   _minimumLineHeight=other->_minimumLineHeight;
   _maximumLineHeight=other->_maximumLineHeight;
   _lineHeightMultiple=other->_lineHeightMultiple;
   _lineSpacing=other->_lineSpacing;
   _defaultTabInterval=other->_defaultTabInterval;
   [self setTabStops:[other tabStops]];
   _hyphenationFactor=other->_hyphenationFactor;
   _tighteningFactorForTruncation=other->_tighteningFactorForTruncation;
}

-(void)setBaseWritingDirection:(NSWritingDirection)direction {
   _writingDirection=direction;
}

-(void)setParagraphSpacing:(float)spacing {
   _paragraphSpacing=spacing;
}

-(void)setParagraphSpacingBefore:(float)spacing {
   _paragraphSpacingBefore=spacing;
}

-(void)setTextBlocks:(NSArray *)blocks {
   blocks=[blocks copy];
   [_textBlocks release];
   _textBlocks=blocks;
}

-(void)setTextLists:(NSArray *)lists {
   lists=[lists copy];
   [_textLists release];
   _textLists=lists;
}

-(void)setHeaderLevel:(int)level {
   _headerLevel=level;
}

-(void)setFirstLineHeadIndent:(float)indent {
   _firstLineHeadIndent=indent;
}

-(void)setHeadIndent:(float)indent {
   _headIndent=indent;
}

-(void)setTailIndent:(float)indent {
   _tailIndent=indent;
}

-(void)setAlignment:(NSTextAlignment)alignment {
   _alignment=alignment;
}

-(void)setLineBreakMode:(NSLineBreakMode)mode {
   _lineBreakMode=mode;
}

-(void)setMinimumLineHeight:(float)height {
   _minimumLineHeight=height;
}

-(void)setMaximumLineHeight:(float)height {
   _maximumLineHeight=height;
}

-(void)setLineHeightMultiple:(float)multiple {
   _lineHeightMultiple=multiple;
}

-(void)setLineSpacing:(float)spacing {
   _lineSpacing=spacing;
}

-(void)setDefaultTabInterval:(float)interval {
   _defaultTabInterval=interval;
}

-(void)setTabStops:(NSArray *)tabStops {
   tabStops=[tabStops copy];
   [_tabStops release];
   _tabStops=tabStops;
}

-(void)setHyphenationFactor:(float)factor {
   _hyphenationFactor=factor;
}

-(void)setTighteningFactorForTruncation:(float)factor {
   _tighteningFactorForTruncation=factor;
}

@end
