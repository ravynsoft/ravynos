/* Copyright (c) 2008 Johannes Fortmann
   Copyright (c) 2009 Christopher J. W. Lloyd - <cjwl@objc.net>
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import "O2Font_FT.h"
#import <Onyx2D/O2Font_freetype.h>

O2FontRef O2FontCreateWithFontName_platform(NSString *name) {
    return [[O2Font_FT alloc] initWithFontName:name];
}

O2FontRef O2FontCreateWithDataProvider_platform(O2DataProviderRef provider) {
#ifdef FREETYPE_PRESENT
    return [[O2Font_freetype alloc] initWithDataProvider:provider];
#else
    return nil;
#endif
    
}

@implementation O2Font(FreeType)

+allocWithZone:(NSZone *)zone {
   return NSAllocateObject([O2Font_FT class],0,NULL);
}

@end

@implementation O2Font_FT

FT_Library O2FontSharedFreeTypeLibrary(){
   static FT_Library library=NULL;
   
   if(library==NULL){
    if(FT_Init_FreeType(&library)!=0)
     NSLog(@"FT_Init_FreeType failed");
   }
        
   return library;
}

FcConfig *O2FontSharedFontConfig() {
   static FcConfig *fontConfig=NULL;
   
   if(fontConfig==NULL){
    fontConfig=FcInitLoadConfigAndFonts();
   }
   
   return fontConfig;
}

+(NSString*)filenameForPattern:(NSString *)pattern {
   int i;
   pattern = [self nativeFontNameForPostscriptName:pattern];
   FcPattern *pat=FcNameParse((unsigned char*)[pattern UTF8String]);

   FcObjectSet *props=FcObjectSetBuild(FC_FILE, NULL);

   FcFontSet *set = FcFontList (O2FontSharedFontConfig(), pat, props);
   NSString* ret=NULL;
   for(i = 0; i < set->nfont && !ret; i++) {
      FcChar8 *filename;

      if (FcPatternGetString (set->fonts[i], FC_FILE, 0, &filename) == FcResultMatch) {
         ret=[NSString stringWithUTF8String:(char*)filename];
      }
   }

   FcPatternDestroy(pat);
   FcObjectSetDestroy(props);
   FcFontSetDestroy(set);
   return ret;
}

-initWithFontName:(NSString *)name {
   [super initWithFontName:name];

   NSString *filename=[isa filenameForPattern:name];
   if(filename==nil) {
    filename=[isa filenameForPattern:@""];
    
    if(filename==nil) {
      filename=@"/System/Library/Fonts/TTF/NimbusSans-Regular.ttf";
    }
   }
      
   FT_Error ret=FT_New_Face(O2FontSharedFreeTypeLibrary(),[filename fileSystemRepresentation],0,&_face);

   if(ret!=0)
    NSLog(@"FT_New_Face returned %d",ret);

   FT_Select_Charmap(_face, FT_ENCODING_UNICODE);
 //  FT_Set_Char_Size(_face,0,2048*64,72,72);

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
   [super dealloc];
}

-(FT_Face)face {
   return _face;
}

-(O2Glyph)glyphWithGlyphName:(NSString *)name {
// FIXME: implement
   return 0;
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

-(NSCharacterSet *)coveredCharacterSet {
    if(_coveredCharSet == nil) {
        NSMutableCharacterSet *set = [[NSMutableCharacterSet alloc] init];
        uint32_t code, first, last, index;

        code = first = FT_Get_First_Char(_face, &index);
        while(index != 0) {
            last = code;
            code = FT_Get_Next_Char(_face, code, &index);
            if(code > (last+1)) {
                [set addCharactersInRange:NSMakeRange(first, last - first)];
                first = code;
            }
        }
        [set addCharactersInRange: NSMakeRange(first, last - first)];
        _coveredCharSet = set;
    }
    return _coveredCharSet;
}

@end

@implementation O2Font(FT)

+ (NSString *)nativeFontNameForPostscriptName:(NSString *)name
{
    if([name rangeOfString:@"-"].length == NSNotFound)
        return [NSString stringWithFormat:@"%@:style=Regular", name];
    return [name stringByReplacingOccurrencesOfString:@"-" withString:@":style="];
}

+ (NSString *)postscriptNameForNativeName:(NSString *)name
{
	return [[name stringByReplacingOccurrencesOfString:@":style="
        withString:@"-"] stringByReplacingOccurrencesOfString:@"-Regular"
        withString:@""];
}

+ (NSString *)postscriptNameForDisplayName:(NSString *)name
{
	return name;
}

+ (NSString *)displayNameForPostscriptName:(NSString *)name
{
	return name;
}

+ (NSString *)postscriptNameForFontName:(NSString *)name
{
	return name;
}

@end
