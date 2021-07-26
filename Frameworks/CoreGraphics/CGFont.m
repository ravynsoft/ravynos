/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <CoreGraphics/CGFont.h>
#import <Onyx2D/O2Font.h>

CGFontRef CGFontCreateWithFontName(CFStringRef name) {
   return O2FontCreateWithFontName((NSString *)name);
}

CGFontRef CGFontRetain(CGFontRef self) {
   return O2FontRetain(self);
}

void      CGFontRelease(CGFontRef self) {
   O2FontRelease(self);
}

CFStringRef CGFontCopyFullName(CGFontRef self) {
   return (CFStringRef)O2FontCopyFullName(self);
}

int       CGFontGetUnitsPerEm(CGFontRef self) {
   return O2FontGetUnitsPerEm(self);
}

int       CGFontGetAscent(CGFontRef self) {
   return O2FontGetAscent(self);
}

int       CGFontGetDescent(CGFontRef self) {
   return O2FontGetDescent(self);
}

int       CGFontGetLeading(CGFontRef self) {
   return O2FontGetLeading(self);
}

int       CGFontGetCapHeight(CGFontRef self) {
   return O2FontGetCapHeight(self);
}

int       CGFontGetXHeight(CGFontRef self) {
   return O2FontGetXHeight(self);
}

CGFloat   CGFontGetItalicAngle(CGFontRef self) {
   return O2FontGetItalicAngle(self);
}

CGFloat   CGFontGetStemV(CGFontRef self) {
   return O2FontGetStemV(self);
}

CGRect    CGFontGetFontBBox(CGFontRef self) {
   return O2FontGetFontBBox(self);
}

size_t    CGFontGetNumberOfGlyphs(CGFontRef self) {
   return O2FontGetNumberOfGlyphs(self);
}

bool      CGFontGetGlyphAdvances(CGFontRef self,const CGGlyph *glyphs,size_t count,int *advances) {
   return O2FontGetGlyphAdvances(self,glyphs,count,advances);
}

CGGlyph   CGFontGetGlyphWithGlyphName(CGFontRef self,CFStringRef name) {
   return O2FontGetGlyphWithGlyphName(self,name);
}

CFStringRef CGFontCopyGlyphNameForGlyph(CGFontRef self,CGGlyph glyph) {
   return (CFStringRef)O2FontCopyGlyphNameForGlyph(self,glyph);
}

CFDataRef CGFontCopyTableForTag(CGFontRef self,uint32_t tag) {
   return (CFDataRef)O2FontCopyTableForTag(self,tag);
}
