/* Copyright (c) 2008 Johannes Fortmann
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSFont.h>
#import "KTFont_FT.h"
#import <AppKit/KTFont.h>
#import <AppKit/NSRaise.h>
#import "O2Font_FT.h"
#import <AppKit/NSFontTypeface.h>
#import <AppKit/NSGraphicsContext.h>

@implementation KTFont(KTFont_FT)
+(id)allocWithZone:(NSZone*)zone
{
   return NSAllocateObject([KTFont_FT class], 0, NULL);
}
@end

@implementation KTFont_FT


-initWithUIFontType:(CTFontUIFontType)uiFontType size:(CGFloat)size language:(NSString *)language {
   O2Font *font=nil;
   
   switch(uiFontType){
  
    case kCTFontMenuTitleFontType:
     if(size==0)
      size=14.0;
     font=O2FontCreateWithFontName(@"Nimbus Sans-Regular");
     break;
    case kCTFontMenuItemFontType:
    case kCTFontSystemFontType:
     if(size==0)
      size=12.0;
     font=O2FontCreateWithFontName(@"Nimbus Sans-Regular");
     break;
 
    default:
     return nil;
   }
   
   self=[self initWithFont:font size:size];
   
   [font release];
   
   return self;
}

-(void)getGlyphs:(CGGlyph *)glyphs forCharacters:(const unichar *)characters length:(unsigned)length {
   O2Font_FT *o2Font=(O2Font_FT *)_font;
   FT_Face    face=[o2Font face];
   
   int i;
   for(i=0; i<length; i++)
   {
      glyphs[i]=FT_Get_Char_Index(face, characters[i]);
   }
}

-(void)getAdvancements:(CGSize *)advancements forGlyphs:(const CGGlyph *)glyphs count:(unsigned)count {
   O2Font_FT *o2Font=(O2Font_FT *)_font;
   FT_Face    face=[o2Font face];

   int i;
   FT_Set_Pixel_Sizes(face, _size, _size);

   for(i=0;i<count;i++){
    FT_Load_Glyph(face, glyphs[i], FT_LOAD_DEFAULT);
      advancements[i]= CGSizeMake(face->glyph->bitmap_left,
                                  0);
   }
}

-(NSSize)advancementForGlyph:(NSGlyph)glyph {
   CGSize adv[1];
   [self getAdvancements:&adv forGlyphs:(CGGlyph *)&glyph count:1];
   return adv[0];
}

-(CGPoint)positionOfGlyph:(CGGlyph)current precededByGlyph:(CGGlyph)previous isNominal:(BOOL *)isNominalp {
   O2Font_FT *o2Font=(O2Font_FT *)_font;
   FT_Face    face=[o2Font face];

   *isNominalp=YES;
 
   if(!current)
      return NSZeroPoint;

   FT_Set_Pixel_Sizes(face, _size, _size);

   FT_Load_Glyph(face, current, FT_LOAD_DEFAULT);
   return NSMakePoint(face->glyph->advance.x/(float)(2<<5),face->glyph->advance.y/(float)(2<<5));
}

-(NSString *)description {
   return [NSString stringWithFormat:@"<%@ %@ %f>",isa,_font,_size];
}

// FIXME: I feel like all of this below should be inherited from NSFont but these are
// all unrecognized selectors unless implemented here. WHY?

-(NSCharacterSet *)coveredCharacterSet {
	return [_font coveredCharacterSet];
}

-(CGFloat)defaultLineHeightForFont {
   return round(O2FontGetAscent(_font) + O2FontGetDescent(_font) + O2FontGetLeading(_font));
}

-(void)setInContext:(NSGraphicsContext *)context {
   CGContextRef cgContext=[context graphicsPort];
   
   CGContextSetFont(cgContext,_font);
   CGContextSetFontSize(cgContext,_size);

   CGAffineTransform textMatrix;
   
// FIX, should check the focusView in the context instead of NSView's
   if([[NSGraphicsContext currentContext] isFlipped])
    textMatrix=(CGAffineTransform){1,0,0,-1,0,0};
   else
    textMatrix=CGAffineTransformIdentity;

   CGContextSetTextMatrix(cgContext,textMatrix);
}

@end


