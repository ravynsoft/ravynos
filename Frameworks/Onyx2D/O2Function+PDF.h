#import <Onyx2D/O2Function.h>

@class O2PDFArray, O2PDFDictionary, O2PDFObject, O2PDFContext, O2PDFStream;

@interface O2Function (PDF)
- initWithDomain:(O2PDFArray *)domain range:(O2PDFArray *)range;
- (O2PDFObject *)encodeReferenceWithContext:(O2PDFContext *)context;
+ (O2Function *)createFunctionWithDictionary:(O2PDFDictionary *)dictionary;
+ (O2Function *)createFunctionWithStream:(O2PDFStream *)stream;
@end
