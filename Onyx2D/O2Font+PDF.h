#import <Onyx2D/O2Font.h>

@class O2PDFObject, O2PDFContext;

@interface O2Font (PDF)
- (void)getMacRomanBytes:(unsigned char *)bytes forGlyphs:(const O2Glyph *)glyphs length:(unsigned)length;
- (O2PDFObject *)encodeReferenceWithContext:(O2PDFContext *)context size:(O2Float)size;
@end
