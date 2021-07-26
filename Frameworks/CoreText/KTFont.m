/* Copyright (c) 2006-2008 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import "KTFont.h"
#import <Onyx2D/O2Exceptions.h>
#import <Foundation/NSArray.h>

@implementation KTFont

-initWithFont:(CGFontRef)font size:(CGFloat)size {
   _font=CGFontRetain(font);
   _unitsPerEm=CGFontGetUnitsPerEm(_font);
   _size=size;
   return self;
}

-initWithUIFontType:(CTFontUIFontType)uiFontType size:(CGFloat)size language:(NSString *)language {
   O2InvalidAbstractInvocation();
   return nil;
}

-(void)dealloc {
   CGFontRelease(_font);
   [super dealloc];
}

-(CFStringRef)copyName {
   return CGFontCopyFullName(_font);
}

-(CGFloat)pointSize {
	return _size;
}

-(CGFloat)fontSize {
	return _size;
}

-(CGRect)boundingRect {
   O2InvalidAbstractInvocation();
   return CGRectZero;
}

-(CGFloat)ascender {
   CGFloat ascent=CGFontGetAscent(_font);
   
   return (ascent/_unitsPerEm)*_size;
}

// CT descenter is the opposite of the CG one on Cocoa
-(CGFloat)descender {
   CGFloat descent=CGFontGetDescent(_font);
   return -(descent/_unitsPerEm)*_size;
}

-(CGFloat)leading {
   CGFloat leading=CGFontGetLeading(_font);
   
   return (leading/_unitsPerEm)*_size;
}

-(CGFloat)underlineThickness {
   O2InvalidAbstractInvocation();
   return 0;
}

-(CGFloat)underlinePosition {
   O2InvalidAbstractInvocation();
   return 0;
}

-(CGFloat)italicAngle {
   return CGFontGetItalicAngle(_font);
}

-(CGFloat)xHeight {
   CGFloat xHeight=CGFontGetXHeight(_font);

   return (xHeight/_unitsPerEm)*_size;
}

-(CGFloat)capHeight {
   CGFloat capHeight=CGFontGetCapHeight(_font);
   
   return (capHeight/_unitsPerEm)*_size;
}

-(unsigned)numberOfGlyphs {
   return CGFontGetNumberOfGlyphs(_font);
}

-(CGPoint)positionOfGlyph:(CGGlyph)current precededByGlyph:(CGGlyph)previous isNominal:(BOOL *)isNominalp {
   int advancement;
   
   if(previous==0)
    return CGPointMake(0,0);

   *isNominalp=YES;
   CGFontGetGlyphAdvances(_font,&previous,1,&advancement);

   return CGPointMake(((CGFloat)advancement/_unitsPerEm)*_size,0);
}

-(void)getGlyphs:(CGGlyph *)glyphs forCharacters:(const unichar *)characters length:(unsigned)length {
   int i;
   
   for(i=0;i<length;i++){
    uint16_t code=characters[i];
    uint8_t  group=code>>8;
    uint8_t  index=code&0xFF;
    
    if(_twoLevel[group]==NULL)
     glyphs[i]=0;
    else
     glyphs[i]=_twoLevel[group][index];
   }
}

-(void)getAdvancements:(CGSize *)advancements forGlyphs:(const CGGlyph *)glyphs count:(unsigned)count {
   int advances[count];
   
   CGFontGetGlyphAdvances(_font,glyphs,count,advances);
   for(int i=0;i<count;i++){
    advancements[i].width=((CGFloat)advances[i]/(CGFloat)_unitsPerEm)*_size;
    advancements[i].height=0;
   }
}

-(CGPathRef)createPathForGlyph:(CGGlyph)glyph transform:(CGAffineTransform *)xform {
   O2InvalidAbstractInvocation();
   return nil;
}

@end
