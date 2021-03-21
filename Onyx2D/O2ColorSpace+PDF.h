#import <Onyx2D/O2ColorSpace.h>

@class O2PDFObject, O2PDFContext;

@interface O2ColorSpace (PDF)
- (O2PDFObject *)encodeReferenceWithContext:(O2PDFContext *)context;
+ (O2ColorSpaceRef)createColorSpaceFromPDFObject:(O2PDFObject *)object;
@end
