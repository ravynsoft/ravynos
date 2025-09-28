#import "CoreText/KTFont.h"

@class KGPDFObject, KGPDFContext;

@interface KTFont (PDF)
- (void)getBytes:(unsigned char *)bytes forGlyphs:(const CGGlyph *)glyphs length:(unsigned)length;
- (KGPDFObject *)encodeReferenceWithContext:(KGPDFContext *)context;
@end
