/* Copyright (c) 2008 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <CoreText/CTFont.h>
#import "KTFont.h"

CTFontRef CTFontCreateWithGraphicsFont(CGFontRef cgFont,CGFloat size,CGAffineTransform *xform,id attributes) {
   return [[KTFont alloc] initWithFont:cgFont size:size];
}

CTFontRef CTFontCreateUIFontForLanguage(CTFontUIFontType uiFontType,CGFloat size,NSString *language ) {
   return [[KTFont alloc] initWithUIFontType:uiFontType size:size language:language];
}

CFStringRef CTFontCopyFullName(CTFontRef self) {
   return [self copyName];
}

CGFloat CTFontGetSize(CTFontRef self) {
   return [self pointSize];
}

size_t CTFontGetGlyphCount(CTFontRef self) {
   return [self numberOfGlyphs];
}

BOOL CTFontGetGlyphsForCharacters(CTFontRef self,const unichar *characters,CGGlyph *glyphs,size_t count) {
   [self getGlyphs:glyphs forCharacters:characters length:count];
   // FIXME: change getGlyphs: to return a BOOL
   return YES;
}

CGRect CTFontGetBoundingBox(CTFontRef self) {
   return [self boundingRect];
}

void CTFontGetAdvancesForGlyphs(CTFontRef self,int orientation,const CGGlyph *glyphs,CGSize *advances,size_t count) {
   [self getAdvancements:advances forGlyphs:glyphs count:count];
}

CGFloat CTFontGetUnderlinePosition(CTFontRef self) {
   return [self underlinePosition];
}

CGFloat CTFontGetUnderlineThickness(CTFontRef self) {
   return [self underlineThickness];
}

CGFloat CTFontGetAscent(CTFontRef self) {
   return [self ascender];
}

CGFloat CTFontGetDescent(CTFontRef self) {
   return [self descender];
}

CGFloat CTFontGetLeading(CTFontRef self) {
   return [self leading];
}

CGFloat CTFontGetSlantAngle(CTFontRef self) {
   return [self italicAngle];
}

CGFloat CTFontGetXHeight(CTFontRef self) {
   return [self xHeight];
}

CGFloat CTFontGetCapHeight(CTFontRef self) {
   return [self capHeight];
}

CGPathRef CTFontCreatePathForGlyph(CTFontRef self,CGGlyph glyph,CGAffineTransform *xform) {
   return (CGPathRef)[self createPathForGlyph:glyph transform:xform];
}


