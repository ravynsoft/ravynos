#import <Onyx2D/O2Font+PDF.h>
#import <Onyx2D/O2PDFArray.h>
#import <Onyx2D/O2PDFDictionary.h>
#import <Onyx2D/O2PDFContext.h>
#import <Foundation/NSArray.h>
#import "O2Encoding.h"

@implementation O2Font(PDF)

extern NSString *O2MacRomanGlyphNames[256];

uint8_t O2PDFMacRomanPositionOfGlyphName(NSString *name){
   static NSDictionary *map=nil;
   
   if(map==nil){
    NSMutableDictionary *mutmap=[NSMutableDictionary new];
    int i;
    for(i=0;i<256;i++)
     [mutmap setObject:[NSNumber numberWithInt:i] forKey:O2MacRomanGlyphNames[i]];
     map=mutmap;
   }
   
   return [[map objectForKey:name] intValue];
}

O2PDFArray *O2FontCreatePDFWidthsWithEncoding(O2FontRef self,O2Encoding *encoding){
   CGFloat       unitsPerEm=O2FontGetUnitsPerEm(self);
   O2PDFArray   *result=[[O2PDFArray alloc] init];
   O2Glyph       glyphs[256];
   int           widths[256];
   int           i;
   
   [encoding getGlyphs:glyphs];

   O2FontGetGlyphAdvances(self,glyphs,256,widths);

   for(i=32;i<256;i++){
    O2PDFReal width=(widths[i]/unitsPerEm)*1000; // normalized to 1000
    
    [result addNumber:width];
   }
   
    return result;
}

// this is overriden for GDI
-(void)getMacRomanBytes:(unsigned char *)bytes forGlyphs:(const O2Glyph *)glyphs length:(unsigned)length {
   int i;
   
   for(i=0;i<length;i++){
    NSString *name=O2FontCopyGlyphNameForGlyph(self,glyphs[i]);

    bytes[i]=O2PDFMacRomanPositionOfGlyphName(name);

    [name release];
   }
}

-(const char *)pdfFontName {
// possibly wrong encoding
   return [[[self->_name componentsSeparatedByString:@" "] componentsJoinedByString:@","] cStringUsingEncoding:NSISOLatin1StringEncoding];
}

-(O2PDFDictionary *)_pdfFontDescriptorWithSize:(CGFloat)size {
   O2PDFDictionary *result=[O2PDFDictionary pdfDictionary];

   [result setNameForKey:"Type" value:"FontDescriptor"];
   [result setNameForKey:"FontName" value:[self pdfFontName]];
   [result setIntegerForKey:"Flags" value:4];
   
   CGFloat   unitsPerEm=O2FontGetUnitsPerEm(self);
   O2PDFReal bbox[4];
   O2Rect    bRect=O2FontGetFontBBox(self);
   bRect.origin.x/=unitsPerEm*size;
   bRect.origin.y/=unitsPerEm*size;
   bRect.size.width/=unitsPerEm*size;
   bRect.size.height/=unitsPerEm*size;
   
   bbox[0]=bRect.origin.x;
   bbox[1]=bRect.origin.y;
   bbox[2]=bRect.size.width;
   bbox[3]=bRect.size.height;
   [result setObjectForKey:"FontBBox" value:[O2PDFArray pdfArrayWithNumbers:bbox count:4]];
   [result setIntegerForKey:"ItalicAngle" value:O2FontGetItalicAngle(self)/unitsPerEm*size];
   [result setIntegerForKey:"Ascent" value:O2FontGetAscent(self)/unitsPerEm*size];
   [result setIntegerForKey:"Descent" value:O2FontGetDescent(self)/unitsPerEm*size];
   [result setIntegerForKey:"CapHeight" value:O2FontGetCapHeight(self)/unitsPerEm*size];
   [result setIntegerForKey:"StemV" value:0];
   [result setIntegerForKey:"StemH" value:0];
   
   return result;
}

-(O2PDFObject *)encodeReferenceWithContext:(O2PDFContext *)context size:(CGFloat)size {
   O2PDFObject *reference=[context referenceForFontWithName:self->_name size:size];
   
   if(reference==nil){
    O2PDFDictionary *result=[O2PDFDictionary pdfDictionary];
    O2Encoding      *encoding=[self createEncodingForTextEncoding:kO2EncodingMacRoman];
     
    [result setNameForKey:"Type" value:"Font"];
    [result setNameForKey:"Subtype" value:"TrueType"];
    [result setNameForKey:"BaseFont" value:[self pdfFontName]];
    [result setIntegerForKey:"FirstChar" value:32];
    [result setIntegerForKey:"LastChar" value:255];
    O2PDFArray *widths=O2FontCreatePDFWidthsWithEncoding(self,encoding);
    [encoding release];
    [result setObjectForKey:"Widths" value:[context encodeIndirectPDFObject:widths]];
    [widths release];
    
    [result setObjectForKey:"FontDescriptor" value:[context encodeIndirectPDFObject:[self _pdfFontDescriptorWithSize:size]]];

    [result setNameForKey:"Encoding" value:"MacRomanEncoding"];

    reference=[context encodeIndirectPDFObject:result];
    [context setReference:reference forFontWithName:self->_name size:size];
   }
   
   return reference;
}

@end
