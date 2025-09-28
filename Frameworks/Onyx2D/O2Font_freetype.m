#import <Onyx2D/O2Font_freetype.h>
#ifdef FREETYPE_PRESENT
#import <Onyx2D/O2Encoding.h>

@implementation O2Font_freetype

FT_Library freeTypeLibrary;

+(void)initialize {
  if(self==[O2Font_freetype class]){
   int error = FT_Init_FreeType(&freeTypeLibrary);
  
   if ( error ) {
     NSLog(@"error initializing FreeType %d",error);
   }
  }
}


-initWithDataProvider:(O2DataProviderRef)provider {
   if([super initWithDataProvider:provider]==nil)
    return nil;

   _platformType=O2FontPlatformTypeFreeType;
   
   const void *bytes=[provider bytes];
   size_t      length=[provider length];

   int error=FT_New_Memory_Face(freeTypeLibrary,bytes,length,0,&_face);
   
   if(error!=0){
    NSLog(@"FT_New_Memory_Face=%d",error);
    [self dealloc];
    return nil;
   }
   
   int i,numberOfCharMaps=_face->num_charmaps;
   bool hasUnicode=FALSE;
   bool hasMacRoman=FALSE;
   
   for(i=0;i<numberOfCharMaps;i++){
    
    if(_face->charmaps[i]->encoding==FT_ENCODING_UNICODE)
     hasUnicode=TRUE;
    if(_face->charmaps[i]->encoding==FT_ENCODING_APPLE_ROMAN)
     hasMacRoman=TRUE;
   }
   
   if(hasUnicode){
    _ftEncoding=FT_ENCODING_UNICODE;
   }
   else if(hasMacRoman){
    _ftEncoding=FT_ENCODING_APPLE_ROMAN;
   }
   else {
    NSLog(@"encoding=%c %c %c %c",_face->charmaps[i]->encoding>>24,_face->charmaps[i]->encoding>>16,_face->charmaps[i]->encoding>>8,_face->charmaps[i]->encoding);
    _ftEncoding=_face->charmaps[0]->encoding;
   }
   
   if(FT_Select_Charmap(_face,_ftEncoding)!=0)
    NSLog(@"FT_Select_Charmap(%d) failed",_ftEncoding);
   
   if(!(_face->face_flags&FT_FACE_FLAG_SCALABLE))
    NSLog(@"FreeType font face is not scalable");
    
   _unitsPerEm=(float)_face->units_per_EM;
   _ascent=_face->ascender;
   _descent=_face->descender;
   _leading=0;
   _capHeight=_face->height;
   _xHeight=_face->height;
   _italicAngle=0;
   _stemV=0;
   _bbox.origin.x=_face->bbox.xMin;
   _bbox.origin.y=_face->bbox.yMin;
   _bbox.size.width=_face->bbox.xMax-_face->bbox.xMin;
   _bbox.size.height=_face->bbox.yMax-_face->bbox.yMin;
   _numberOfGlyphs=_face->num_glyphs;
   _advances=NULL;
   return self;
}

-(void)dealloc {
   FT_Done_Face(_face);
   [_macRomanEncoding release];
   [_macExpertEncoding release];
   [_winAnsiEncoding release];
   [super dealloc];
}

FT_Face O2FontFreeTypeFace(O2Font_freetype *self) {
   return self->_face;
}

-(void)fetchAdvances {
   O2Glyph glyph;

   FT_Set_Char_Size(_face,0,_unitsPerEm*64,72,72);

   _advances=NSZoneMalloc(NULL,sizeof(int)*_numberOfGlyphs);

   for(glyph=0;glyph<_numberOfGlyphs;glyph++){
    FT_Load_Glyph(_face, glyph, FT_LOAD_DEFAULT);
    
    _advances[glyph]=_face->glyph->advance.x/(float)(2<<5);
   }
}

-(O2Glyph)glyphWithGlyphName:(NSString *)name {
   return FT_Get_Name_Index(_face,(char *)[name cString]);
}

-(void)getGlyphsForCodePoints:(uint16_t *)codes:(O2Glyph *)glyphs:(int)length {
   int i;
   
   for(i=0;i<length;i++)
    glyphs[i]=FT_Get_Char_Index(_face,codes[i]);
}

-(O2Encoding *)unicode_createEncodingForTextEncoding:(O2TextEncoding)encoding {
   unichar unicode[256];
   O2Glyph glyphs[256];
   
   switch(encoding){
    case kO2EncodingFontSpecific:
    case kO2EncodingMacRoman:
     if(_macRomanEncoding==nil){
      O2EncodingGetMacRomanUnicode(unicode);
      [self getGlyphsForCodePoints:unicode:glyphs:256];
      _macRomanEncoding=[[O2Encoding alloc] initWithGlyphs:glyphs unicode:unicode];
     }
     return [_macRomanEncoding retain];
     
    case kO2EncodingMacExpert:
     if(_macExpertEncoding==nil){
      O2EncodingGetMacExpertUnicode(unicode);
      [self getGlyphsForCodePoints:unicode:glyphs:256];
      _macExpertEncoding=[[O2Encoding alloc] initWithGlyphs:glyphs unicode:unicode];
     }
     return [_macExpertEncoding retain];

    case kO2EncodingWinAnsi:
     if(_winAnsiEncoding==nil){
      O2EncodingGetWinAnsiUnicode(unicode);
      [self getGlyphsForCodePoints:unicode:glyphs:256];
      _winAnsiEncoding=[[O2Encoding alloc] initWithGlyphs:glyphs unicode:unicode];
     }
     return [_winAnsiEncoding retain];
    
    default:
     return nil;
   }
   return nil;
}

-(O2Encoding *)MacRoman_createEncodingForTextEncoding:(O2TextEncoding)encoding {
   uint16_t codes[256];
   O2Glyph glyphs[256];
   unichar unicode[256];

   if(_macRomanEncoding==nil){
    int i;
   
    if(encoding!=kO2EncodingMacRoman && encoding!=kO2EncodingFontSpecific){
     NSLog(@"font encoding is MacRoman, requesting encoding %d failed",encoding);
    }
   
    for(i=0;i<256;i++)
     codes[i]=i;
    
    [self getGlyphsForCodePoints:codes:glyphs:256];
    
    O2EncodingGetMacExpertUnicode(unicode);

    _macRomanEncoding=[[O2Encoding alloc] initWithGlyphs:glyphs unicode:unicode];
   }
   
   return [_macRomanEncoding retain];
}

-(O2Encoding *)createEncodingForTextEncoding:(O2TextEncoding)encoding {
   if(_ftEncoding==FT_ENCODING_APPLE_ROMAN)
    return [self MacRoman_createEncodingForTextEncoding:encoding];
    
   return [self unicode_createEncodingForTextEncoding:encoding];
}

@end
#endif
