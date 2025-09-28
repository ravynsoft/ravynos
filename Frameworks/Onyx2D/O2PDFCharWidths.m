#import "O2PDFCharWidths.h"
#import <Onyx2D/O2PDFArray.h>

@implementation O2PDFCharWidths

-initWithArray:(O2PDFArray *)array firstChar:(int)firstChar lastChar:(int)lastChar missingWidth:(CGFloat)missingWidth {
   int i,arrayIndex;
   
   for(i=0;i<firstChar;i++)
    _widths[i]=missingWidth;
    
   for(arrayIndex=0;i<=lastChar;i++,arrayIndex++){
    O2PDFReal real=0;
    
    [array getNumberAtIndex:arrayIndex value:&real];
    
    _widths[i]=real/1000.0;
   }
   for(;i<256;i++)
    _widths[i]=missingWidth;
    
   return self;
}

void O2PDFCharWidthsGetAdvances(O2PDFCharWidths *self,O2Size *advances,const uint8_t *bytes,int length) {
   int i;
   
   for(i=0;i<length;i++){
    advances[i].width=self->_widths[bytes[i]];
    advances[i].height=0;
   }
}

@end
