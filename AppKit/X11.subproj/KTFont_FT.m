/* Copyright (c) 2008 Johannes Fortmann
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "KTFont_FT.h"
#import <AppKit/KTFont.h>
#import <AppKit/NSRaise.h>
#import "O2Font_FT.h"
#import <AppKit/NSFontTypeface.h>

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
    case kCTFontMenuItemFontType:
     if(size==0)
      size=BASEFONT_SIZE;
     font=O2FontCreateWithFontName(BASEFONT_NS);
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


@end


