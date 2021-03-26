#import <Onyx2D/O2Context_distill.h>
#import <Onyx2D/O2GraphicsState.h>
#import <Onyx2D/O2Encoding.h>
#import <Onyx2D/O2PDFCharWidths.h>

@implementation O2Context_distill

static void O2ContextDistillShowText(O2ContextRef self,const char *text,unsigned length) {
   O2GState        *gState=O2ContextCurrentGState(self);
   O2AffineTransform Trm=O2ContextGetTextRenderingMatrix(self);
   NSPoint           point=O2PointApplyAffineTransform(NSMakePoint(0,0),Trm);
   O2Size            fontSize=O2SizeApplyAffineTransform(O2SizeMake(0,O2GStatePointSize(gState)),Trm);
   O2Encoding      *encoding=[gState encoding];
   O2PDFCharWidths *widths=[gState pdfCharWidths];
   O2Glyph          glyphs[length];
   uint16_t         unicode[length];
      
   O2EncodingGetGlyphsForBytes(encoding,glyphs,text,length);
   O2EncodingGetUnicodeForBytes(encoding,unicode,text,length);
   
   O2Size advances[length];
   int    i;
    
   if(widths!=nil)
    O2PDFCharWidthsGetAdvances(widths,advances,text,length);
   else
    O2ContextGetDefaultAdvances(self,glyphs,advances,length);
    
   for(i=0;i<length;i++){
    advances[i].width+=gState->_characterSpacing;
   }
       

// FIXME: Trm includes the device transform which in this case is identity, producing the right result, however the matrix
// used here should not use the device transform, just the CTM because we want the results officially in default user space, not device

   O2Rect  glyphRects[length];
   O2Font *font=O2GStateFont(gState);
   O2Float descent=((CGFloat)O2FontGetDescent(font)/(CGFloat)O2FontGetUnitsPerEm(font))*O2GStatePointSize(gState);
   
   descent=O2SizeApplyAffineTransform(O2SizeMake(0,descent),Trm).height;
   
   for(i=0;i<length;i++){
    O2Size advance=O2SizeApplyAffineTransform(advances[i],Trm);
    
    glyphRects[i].origin=point;
    glyphRects[i].origin.y+=descent;
    glyphRects[i].size.width=advance.width;
    glyphRects[i].size.height=fontSize.height;
    
    point.x+=advance.width;
    point.y+=advance.height;
   }
   
   O2ContextConcatAdvancesToTextMatrix(self,advances,length);

   [((O2Context_distill *)self)->_delegate distiller:(O2Context_distill *)self unicode:unicode rects:glyphRects count:length];
}

-(void)drawPath:(O2PathDrawingMode)pathMode {
}

-(void)showGlyphs:(const O2Glyph *)glyphs advances:(const O2Size *)advances count:(unsigned)count {
}

-(void)drawShading:(O2Shading *)shading {
}

-(void)drawImage:(O2Image *)image inRect:(O2Rect)rect {
}

-(void)clipToState:(O2ClipState *)clipState {
}

-init {
   [super init];
   _showTextFunction=O2ContextDistillShowText;
   return self;
}

-delegate {
   return _delegate;
}

-(void)setDelegate:delegate {
   _delegate=delegate;
}

@end
