#import <Onyx2D/O2Shading.h>

@class O2PDFObject, O2PDFContext;

@interface O2Shading (PDF)
- (O2PDFObject *)encodeReferenceWithContext:(O2PDFContext *)context;
+ (O2Shading *)shadingWithPDFObject:(O2PDFObject *)object;
@end
