#import <Foundation/NSObject.h>
#import <Onyx2D/O2Geometry.h>

@class O2PDFArray;

@interface O2PDFCharWidths : NSObject {
    O2Float _widths[256];
}

- initWithArray:(O2PDFArray *)array firstChar:(int)firstChar lastChar:(int)lastChar missingWidth:(CGFloat)missingWidth;

void O2PDFCharWidthsGetAdvances(O2PDFCharWidths *self, O2Size *advances, const uint8_t *bytes, int length);

@end
