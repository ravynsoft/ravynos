#import <Onyx2D/O2PDFObject.h>

@class O2PDFDictionary, O2Encoding, O2PDFCharWidths, O2Font;

@interface O2PDFFont : O2PDFObject {
    O2PDFDictionary *_info; // If we get rid of this, make sure to copy/retain any values in it
    const char *_baseFont;
    O2Font *_resourceFont;
    O2Font *_graphicsFont;
    O2Encoding *_encoding;
    O2PDFCharWidths *_pdfCharWidths;
}

+ (O2PDFFont *)createWithPDFDictionary:(O2PDFDictionary *)info;

- (O2Encoding *)encoding;
- (O2PDFCharWidths *)pdfCharWidths;

- (O2Font *)graphicsFont;

@end
