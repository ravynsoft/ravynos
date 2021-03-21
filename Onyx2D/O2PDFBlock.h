#import <Onyx2D/O2PDFObject.h>

@class NSMutableArray;

@interface O2PDFBlock : O2PDFObject {
    NSMutableArray *_objects;
}

+ pdfBlock;

- (NSArray *)objects;
- (void)addObject:object;

@end
