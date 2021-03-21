#import "KTFont+PDF.h"
#import "KGPDFArray.h"
#import "KGPDFDictionary.h"
#import "KGPDFContext.h"
#import <Foundation/NSArray.h>

@implementation KTFont(PDF)

// FIXME: needs to do encoding properly

-(void)getBytes:(unsigned char *)bytes forGlyphs:(const CGGlyph *)glyphs length:(unsigned)length {
   unichar  characters[length];
   unsigned i;
   
   [self getCharacters:characters forGlyphs:glyphs length:length];
   for(i=0;i<length;i++)
    if(characters[i]<0xFF)
     bytes[i]=characters[i];
    else
     bytes[i]=' ';
}

-(KGPDFArray *)_pdfWidths {
   KGPDFArray   *result=[KGPDFArray pdfArray];
   unichar       characters[256];
   CGGlyph       glyphs[256];
   CGSize        advancements[256];
   int           i;
   
   for(i=0;i<256;i++)
    characters[i]=i;

// FIXME: needs to do encoding properly

   [self getGlyphs:glyphs forCharacters:characters length:256];
   [self getAdvancements:advancements forGlyphs:glyphs count:256];

// FIX, probably not entirely accurate, you can get precise widths out of the TrueType data
   for(i=0;i<255;i++){
    KGPDFReal width=(advancements[i].width/_size)*1000;
    
    [result addNumber:width];
   }
   
   return result;
}

-(const char *)pdfFontName {
   return [[[[self name] componentsSeparatedByString:@" "] componentsJoinedByString:@","] cString];
}

-(KGPDFDictionary *)_pdfFontDescriptor {
   KGPDFDictionary *result=[KGPDFDictionary pdfDictionary];

   [result setNameForKey:"Type" value:"FontDescriptor"];
   [result setNameForKey:"FontName" value:[self pdfFontName]];
   [result setIntegerForKey:"Flags" value:4];
   
   KGPDFReal bbox[4];
   CGRect    boundingRect=[self boundingRect];
   
   bbox[0]=boundingRect.origin.x;
   bbox[1]=boundingRect.origin.y;
   bbox[2]=boundingRect.size.width;
   bbox[3]=boundingRect.size.height;
   [result setObjectForKey:"FontBBox" value:[KGPDFArray pdfArrayWithNumbers:bbox count:4]];
   [result setIntegerForKey:"ItalicAngle" value:[self italicAngle]];
   [result setIntegerForKey:"Ascent" value:[self ascender]];
   [result setIntegerForKey:"Descent" value:[self descender]];
   [result setIntegerForKey:"CapHeight" value:[self capHeight]];
   [result setIntegerForKey:"StemV" value:0];
   [result setIntegerForKey:"StemH" value:0];
   
   return result;
}

-(KGPDFObject *)encodeReferenceWithContext:(KGPDFContext *)context {
   KGPDFObject *reference=[context referenceForFontWithName:[self name] size:_size];
   
   if(reference==nil){
    KGPDFDictionary *result=[KGPDFDictionary pdfDictionary];

    [result setNameForKey:"Type" value:"Font"];
    [result setNameForKey:"Subtype" value:"TrueType"];
    [result setNameForKey:"BaseFont" value:[self pdfFontName]];
    [result setIntegerForKey:"FirstChar" value:0];
    [result setIntegerForKey:"LastChar" value:255];
    [result setObjectForKey:"Widths" value:[context encodeIndirectPDFObject:[self _pdfWidths]]];
    [result setObjectForKey:"FontDescriptor" value:[context encodeIndirectPDFObject:[self _pdfFontDescriptor]]];

    [result setNameForKey:"Encoding" value:"WinAnsiEncoding"];

    reference=[context encodeIndirectPDFObject:result];
    [context setReference:reference forFontWithName:[self name] size:_size];
   }
   
   return reference;
}

@end
