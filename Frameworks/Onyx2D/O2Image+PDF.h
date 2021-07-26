#import <Onyx2D/O2Image.h>

@class O2ColorSpace, O2DataProvider, O2PDFObject, O2PDFContext;

@interface O2Image (PDF)

- (O2PDFObject *)encodeReferenceWithContext:(O2PDFContext *)context;
+ (O2Image *)imageWithPDFObject:(O2PDFObject *)object;

@end
