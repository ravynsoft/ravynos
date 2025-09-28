#import <Onyx2D/O2Font_ttf.h>
#import <Onyx2D/O2TTFDecoder.h>


@implementation O2Font_ttf

-initWithDataProvider:(O2DataProviderRef)provider {
   O2TTFDecoderRef decoder=O2TTFDecoderCreate(provider);
   
   _nameToGlyph=O2TTFDecoderGetPostScriptNameMapTable(decoder,&_numberOfGlyphs);
   _glyphLocations=O2TTFDecoderGetGlyphLocations(decoder,_numberOfGlyphs);
   return self;
}

-(O2Glyph)glyphWithGlyphName:(NSString *)name {
   return (O2Glyph)(int)NSMapGet(_nameToGlyph,name);
}

O2FontRef O2FontCreateWithFontName_platform(NSString *name) {
   return nil;
}

O2FontRef O2FontCreateWithDataProvider_platform(O2DataProviderRef provider) {
   return nil;
}

@end
