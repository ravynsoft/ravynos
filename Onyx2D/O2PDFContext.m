/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2PDFContext.h>
#import <Onyx2D/O2PDFArray.h>
#import <Onyx2D/O2PDFDictionary.h>
#import <Onyx2D/O2PDFPage.h>
#import <Onyx2D/O2PDFContext.h>
#import <Onyx2D/O2PDFxref.h>
#import <Onyx2D/O2PDFxrefEntry.h>
#import <Onyx2D/O2PDFObject_R.h>
#import <Onyx2D/O2PDFObject_Name.h>
#import <Onyx2D/O2PDFStream.h>
#import <Onyx2D/O2PDFString.h>
#import <Onyx2D/O2Shading+PDF.h>
#import <Onyx2D/O2Image+PDF.h>
#import <Onyx2D/O2Font+PDF.h>
#import <Onyx2D/O2MutablePath.h>
#import <Onyx2D/O2Color.h>
#import <Onyx2D/O2ColorSpace+PDF.h>
#import <Onyx2D/O2GraphicsState.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSData.h>
#import <Foundation/NSPathUtilities.h>
#import <Onyx2D/O2Exceptions.h>
#import <Onyx2D/O2ClipState.h>
#import <Onyx2D/O2ClipPhase.h>
#import <Onyx2D/O2DataConsumer.h>

const NSString *kO2PDFContextTitle=@"kO2PDFContextTitle";

@implementation O2PDFContext

-(void)clipToState:(O2ClipState *)clipState
{
}

-initWithConsumer:(O2DataConsumer *)consumer mediaBox:(const O2Rect *)mediaBox auxiliaryInfo:(NSDictionary *)auxiliaryInfo {
   [super init];

   _dataConsumer=[consumer retain];
   _fontCache=[NSMutableDictionary new];
   _objectToRef=NSCreateMapTable(NSObjectMapKeyCallBacks,NSObjectMapValueCallBacks,0);
   _indirectObjects=[NSMutableArray new];
   _indirectEntries=[NSMutableArray new];
   _nextNumber=1;
   _xref=[[O2PDFxref alloc] initWithData:nil];
   [_xref setTrailer:[O2PDFDictionary pdfDictionary]];

   [self appendCString:"%PDF-1.3\n"];

   _info=[[O2PDFDictionary pdfDictionary] retain];
   [_info setObjectForKey:"Author" value:[O2PDFString pdfObjectWithString:NSFullUserName()]];
   [_info setObjectForKey:"Creator" value:[O2PDFString pdfObjectWithString:[[NSProcessInfo processInfo] processName]]];
   [_info setObjectForKey:"Producer" value:[O2PDFString pdfObjectWithCString:"THE COCOTRON http://www.cocotron.org O2PDFContext"]];
   [[_xref trailer] setObjectForKey:"Info" value:_info];

   _catalog=[[O2PDFDictionary pdfDictionary] retain];
   [[_xref trailer] setObjectForKey:"Root" value:_catalog];

   _pages=[[O2PDFDictionary pdfDictionary] retain];
   [_catalog setNameForKey:"Type" value:"Catalog"];
   [_catalog setObjectForKey:"Pages" value:_pages];
   [_pages setIntegerForKey:"Count" value:0];

   _kids=[[O2PDFArray pdfArray] retain];
   [_pages setNameForKey:"Type" value:"Pages"];
   [_pages setObjectForKey:"Kids" value:_kids];

   _page=nil;
   _categoryToNext=[NSMutableDictionary new];
   _contentStreamStack=[NSMutableArray new];

   NSString *title=[auxiliaryInfo objectForKey:kO2PDFContextTitle];

   if(title==nil)
    title=@"Untitled";

   [_info setObjectForKey:"Title" value:[O2PDFString pdfObjectWithString:title]];

   [self referenceForObject:_catalog];
   [self referenceForObject:_info];

   return self;
}

-(void)dealloc {
   [_dataConsumer release];
   [_fontCache release];
   NSFreeMapTable(_objectToRef);
   [_indirectObjects release];
   [_indirectEntries release];
   [_xref release];
   [_info release];
   [_catalog release];
   [_pages release];
   [_kids release];
   [_page release];
   [_categoryToNext release];
   [_textStateStack release];
   [_contentStreamStack release];
   [super dealloc];
}

-(unsigned)length {
    return _length;
}

-(BOOL)isBitmapContext
{
    return NO;
}

-(void)appendBytes:(const void *)ptr length:(unsigned)length {
    _length += length;
    O2DataConsumerPutBytes(_dataConsumer, ptr, length);
}

-(void)appendData:(NSData *)data {
    [self appendBytes:[data bytes] length:[data length]];
}

-(void)appendCString:(const char *)cString {
    [self appendBytes:cString length:strlen(cString)];
}

-(void)appendString:(NSString *)string {
   NSData *data=[string dataUsingEncoding:NSASCIIStringEncoding];
   [self appendData:data];
}

-(void)appendFormat:(NSString *)format,... {
   NSString *string;
   va_list   arguments;

   va_start(arguments,format);

   string=[[NSString alloc] initWithFormat:format arguments:arguments];
   [self appendString:string];
   [string release];

   va_end(arguments);
}

-(void)appendPDFStringWithBytes:(const void *)bytesV length:(unsigned)length toObject:(id)data {
   const unsigned char *bytes=bytesV;
   BOOL hex=NO;
   int  i;

   for(i=0;i<length;i++)
    if(bytes[i]<' ' || bytes[i]>=127 || bytes[i]=='(' || bytes[i]==')'){
     hex=YES;
     break;
    }

   if(hex){
    const char *hex="0123456789ABCDEF";
    int         i,bufCount,bufSize=256;
    char        buf[bufSize];

    [data appendBytes:"<" length:1];
    bufCount=0;
    for(i=0;i<length;i++){
     buf[bufCount++]=hex[bytes[i]>>4];
     buf[bufCount++]=hex[bytes[i]&0xF];

     if(bufCount==bufSize){
      [data appendBytes:buf length:bufCount];
      bufCount=0;
     }
    }
    [data appendBytes:buf length:bufCount];
    [data appendBytes:"> " length:2];
   }
   else {
    [data appendBytes:"(" length:1];
    [data appendBytes:bytes length:length];
    [data appendBytes:") " length:2];
   }
}

-(void)appendPDFStringWithBytes:(const void *)bytes length:(unsigned)length {
   [self appendPDFStringWithBytes:bytes length:length toObject:self];
}

-(BOOL)hasReferenceForObject:(O2PDFObject *)object {
   O2PDFObject *result=NSMapGet(_objectToRef,object);

   return (result==nil)?NO:YES;
}

-(O2PDFObject *)referenceForObject:(O2PDFObject *)object {
   O2PDFObject *result=NSMapGet(_objectToRef,object);

   if(result==nil){
    O2PDFxrefEntry *entry=[O2PDFxrefEntry xrefEntryWithPosition:0 number:_nextNumber generation:0];

    result=[O2PDFObject_R pdfObjectWithNumber:_nextNumber generation:0 xref:_xref];
    NSMapInsert(_objectToRef,object,result);

    [_xref addEntry:entry object:object];
    [_indirectObjects addObject:object];
    [_indirectEntries addObject:entry];

    _nextNumber++;
   }

   return result;
}

-(void)encodePDFObject:(O2PDFObject *)object {
   if(![object isByReference] && ![self hasReferenceForObject:object])
    [object encodeWithPDFContext:self];
   else {
    O2PDFObject *ref=[self referenceForObject:object];

    [ref encodeWithPDFContext:self];
   }
}

-(O2PDFObject *)encodeIndirectPDFObject:(O2PDFObject *)object {
   O2PDFObject *result=[self referenceForObject:object];

   return result;
}

-(void)contentWithString:(NSString *)string {
   NSData *data=[string dataUsingEncoding:NSASCIIStringEncoding];

   [[[_contentStreamStack lastObject] mutableData] appendData:data];
}

-(void)contentWithFormat:(NSString *)format,... {
   NSString *string;
   va_list   arguments;

   va_start(arguments,format);

   string=[[NSString alloc] initWithFormat:format arguments:arguments];
   [self contentWithString:string];
   [string release];

   va_end(arguments);
}

-(void)contentPDFStringWithBytes:(const void *)bytes length:(unsigned)length {
   [self appendPDFStringWithBytes:bytes length:length toObject:[[_contentStreamStack lastObject] mutableData]];
}

-(O2PDFObject *)referenceForFontWithName:(NSString *)name size:(float)size {
   return [(NSDictionary *)[_fontCache objectForKey:name] objectForKey:[NSNumber numberWithFloat:size]];
}

-(void)setReference:(O2PDFObject *)reference forFontWithName:(NSString *)name size:(float)size {
   NSMutableDictionary *sizes=[_fontCache objectForKey:name];

   if(sizes==nil){
    sizes=[NSMutableDictionary dictionary];
    [_fontCache setObject:sizes forKey:name];
   }

   [sizes setObject:reference forKey:[NSNumber numberWithFloat:size]];
}

-(O2PDFObject *)nameForResource:(O2PDFObject *)pdfObject inCategory:(const char *)categoryName {
   O2PDFDictionary *resources;
   O2PDFDictionary *category;

   if(![_page getDictionaryForKey:"Resources" value:&resources]){
    resources=[O2PDFDictionary pdfDictionary];
    [_page setObjectForKey:"Resources" value:resources];
   }

   if(![resources getDictionaryForKey:categoryName value:&category]){
    category=[O2PDFDictionary pdfDictionary];
    [resources setObjectForKey:categoryName value:category];
   }

   NSString *key=[NSString stringWithCString:categoryName encoding:NSISOLatin1StringEncoding];
   NSNumber *next=[_categoryToNext objectForKey:key];

   next=[NSNumber numberWithInt:(next==nil)?0:[next intValue]+1];
   [_categoryToNext setObject:next forKey:key];

   const char *objectName=[[NSString stringWithFormat:@"%s%d",categoryName,[next intValue]] UTF8String];
   [category setObjectForKey:objectName value:pdfObject];

    return [O2PDFObject_Name pdfObjectWithCString:objectName];
}

-(void)emitPath:(O2PathRef)path {
   int                  i,numberOfElements=O2PathNumberOfElements(path);
   const unsigned char *elements=O2PathElements(path);
   const O2Point       *points=O2PathPoints(path);
   int                  pi=0;
   O2AffineTransform    invertUserSpaceTransform=O2AffineTransformInvert(O2GStateUserSpaceTransform(O2ContextCurrentGState(self)));

   for(i=0;i<numberOfElements;i++){
    switch(elements[i]){

     case kO2PathElementMoveToPoint:{
       O2Point point=O2PointApplyAffineTransform(points[pi++],invertUserSpaceTransform);

       [self contentWithFormat:@"%g %g m ",point.x,point.y];
      }
      break;

     case kO2PathElementAddLineToPoint:{
       O2Point point=O2PointApplyAffineTransform(points[pi++],invertUserSpaceTransform);

       [self contentWithFormat:@"%g %g l ",point.x,point.y];
      }
      break;

     case kO2PathElementAddQuadCurveToPoint:{
       O2Point c1=O2PointApplyAffineTransform(points[pi++],invertUserSpaceTransform);
       O2Point end=O2PointApplyAffineTransform(points[pi++],invertUserSpaceTransform);

       [self contentWithFormat:@"%g %g %g %g v ",c1.x,c1.y,end.x,end.y];
      }
      break;

     case kO2PathElementAddCurveToPoint:{
       O2Point c1=O2PointApplyAffineTransform(points[pi++],invertUserSpaceTransform);
       O2Point c2=O2PointApplyAffineTransform(points[pi++],invertUserSpaceTransform);
       O2Point end=O2PointApplyAffineTransform(points[pi++],invertUserSpaceTransform);

       [self contentWithFormat:@"%g %g %g %g %g %g c ",c1.x,c1.y,c2.x,c2.y,end.x,end.y];
      }
      break;

     case kO2PathElementCloseSubpath:
      [self contentWithString:@"h "];
      break;

    }
   }
}

- (void)translateCTM:(float)x:(float)y
{
    [self contentWithFormat:@"1 0 0 1 %f %f cm ", x, y];
}
- (void)scaleCTM:(float)x:(float)y
{
    [self contentWithFormat:@"%f 0 0 %f 0 0 cm ", x, y];
}

-(void)emitSaveGState {
   [self contentWithString:@"q "];
}

-(void)emitRestoreGState {
   [self contentWithString:@"Q "];
}

-(void)emitCurrentGState {
  O2GState *gState=O2ContextCurrentGState(self);

  {
  const float *components=O2ColorGetComponents(gState->_strokeColor);

   switch([O2ColorGetColorSpace(gState->_strokeColor) type]){

    case kO2ColorSpaceModelMonochrome:
     [self contentWithFormat:@"%f G ",components[0]];
     break;

    case kO2ColorSpaceModelRGB:
     [self contentWithFormat:@"%f %f %f RG ",components[0],components[1],components[2]];
     break;

    case kO2ColorSpaceModelCMYK:
     [self contentWithFormat:@"%f %f %f %f K ",components[0],components[1],components[2],components[3]];
     break;

    default:
     O2UnimplementedMethod();
     break;
   }
  }

  {
   const float *components=O2ColorGetComponents(gState->_fillColor);

   switch([O2ColorGetColorSpace(gState->_fillColor) type]){

    case kO2ColorSpaceModelMonochrome:
     [self contentWithFormat:@"%f g ",components[0]];
     break;

    case kO2ColorSpaceModelRGB:
     [self contentWithFormat:@"%f %f %f rg ",components[0],components[1],components[2]];
     break;

    case kO2ColorSpaceModelCMYK:
     [self contentWithFormat:@"%f %f %f %f k ",components[0],components[1],components[2],components[3]];
     break;

    default:
     O2UnimplementedMethod();
     break;
   }
  }

   [self contentWithFormat:@"%g Tc ",gState->_characterSpacing];
   [self contentWithFormat:@"%g w ",gState->_lineWidth];
   [self contentWithFormat:@"%d J ",gState->_lineCap];
   [self contentWithFormat:@"%d j ",gState->_lineJoin];
   [self contentWithFormat:@"%g M ",gState->_miterLimit];
   O2PDFArray *array=[O2PDFArray pdfArray];
   int         i,count=gState->_dashLengthsCount;

   for(i=0;i<count;i++)
    [array addNumber:gState->_dashLengths[i]];

   [self contentWithFormat:@"%@ %g d ",array,gState->_dashPhase];

   const char *name;

   switch(gState->_renderingIntent){

    case kO2RenderingIntentAbsoluteColorimetric:
     name="AbsoluteColorimetric";
     break;

    default:
    case kO2RenderingIntentRelativeColorimetric:
     name="RelativeColorimetric";
     break;

    case kO2RenderingIntentSaturation:
     name="Saturation";
     break;

    case kO2RenderingIntentPerceptual:
     name="Perceptual";
     break;
   }
   [self contentWithFormat:@"/%s ri ",name];

   [self contentWithFormat:@"%g i ",gState->_flatness];

   O2AffineTransform matrix=gState->_userSpaceTransform;
    [self contentWithFormat:@"%g %g %g %g %g %g cm ",matrix.a,matrix.b,matrix.c,matrix.d,matrix.tx,matrix.ty];

   NSArray *clipPhases=[O2GStateClipState(gState) clipPhases];

   for(O2ClipPhase *phase in clipPhases){
    switch(O2ClipPhasePhaseType(phase)){

     case O2ClipPhaseNonZeroPath:{
       O2Path *path=O2ClipPhaseObject(phase);
       [self emitPath:path];
       [self contentWithString:@"W "];
      }
      break;

     case O2ClipPhaseEOPath:{
       O2Path *path=O2ClipPhaseObject(phase);
       [self emitPath:path];
       [self contentWithString:@"W* "];
      }
      break;

        case O2ClipPhaseMask: {
#if 0
      O2PDFObject *pdfObject=[image encodeReferenceWithContext:self];
      O2PDFObject *name=[self nameForResource:pdfObject inCategory:"XObject"];

     [self contentWithString:@"q "];
     [self translateCTM:rect.origin.x:rect.origin.y];
     [self scaleCTM:rect.size.width:rect.size.height];
     [self contentWithFormat:@"%@ Do ",name];
     [self contentWithString:@"Q "];
#endif
        }
      break;
    }
   }
}


-(void)drawPath:(O2PathDrawingMode)pathMode {
   [self emitSaveGState];
   [self emitCurrentGState]; // spool the graphic state before the actual path, because some old Adobe Software,
   [self emitPath:_path];    // e.g. AR 7 and AI CS2, complain about "Invalid operation inside a path" otherwise.

   switch(pathMode){

    case kO2PathFill:
     [self contentWithString:@"f "];
     break;

    case kO2PathEOFill:
     [self contentWithString:@"f* "];
     break;

    case kO2PathStroke:
     [self contentWithString:@"S "];
     break;

    case kO2PathFillStroke:
     [self contentWithString:@"B "];
     break;

    case kO2PathEOFillStroke:
     [self contentWithString:@"B* "];
     break;

   }
   O2PathReset(_path);
   [self emitRestoreGState];
}

-(void)showGlyphs:(const O2Glyph *)glyphs advances:(const O2Size *)advances count:(unsigned)count {
// FIXME: use advances if not null

   [self emitSaveGState];
   [self emitCurrentGState];
   [self contentWithString:@"BT "];

   O2GState *state=O2ContextCurrentGState(self);
   O2PDFObject *pdfObject=[O2GStateFont(state) encodeReferenceWithContext:self size:O2GStatePointSize(state)];
   O2PDFObject *name=[self nameForResource:pdfObject inCategory:"Font"];

   [self contentWithFormat:@"%@ %g Tf ",name,O2GStatePointSize(O2ContextCurrentGState(self))];

   O2AffineTransform matrix=O2ContextGetTextMatrix(self);
   [self contentWithFormat:@"%g %g %g %g %g %g Tm ",matrix.a,matrix.b,matrix.c,matrix.d,matrix.tx,matrix.ty];

   unsigned char text[count];

   [O2GStateFont(O2ContextCurrentGState(self)) getMacRomanBytes:text forGlyphs:glyphs length:count];

   [self contentPDFStringWithBytes:text length:count];
   [self contentWithString:@" Tj "];

   [self contentWithString:@"ET "];
   [self emitRestoreGState];
}

-(void)drawShading:(O2Shading *)shading {
    [self emitSaveGState];
    [self emitCurrentGState];
    O2PDFObject *pdfObject=[shading encodeReferenceWithContext:self];
    O2PDFObject *name=[self nameForResource:pdfObject inCategory:"Shading"];

    [self contentWithFormat:@"%@ sh ",name];
    [self emitRestoreGState];
}

-(void)drawImage:(O2Image *)image inRect:(O2Rect)rect {
    [self emitSaveGState];

    O2PDFObject *pdfObject=[image encodeReferenceWithContext:self];
    O2PDFObject *name=[self nameForResource:pdfObject inCategory:"XObject"];
    [self emitCurrentGState];

    [self emitSaveGState];
    [self translateCTM:rect.origin.x:rect.origin.y];
    [self scaleCTM:rect.size.width:rect.size.height];
    [self contentWithFormat:@"%@ Do ",name];
    [self emitRestoreGState];

    [self emitRestoreGState];
}

-(void)drawLayer:(O2LayerRef)layer inRect:(O2Rect)rect {
}

-(void)beginPage:(const O2Rect *)mediaBox {
    O2PDFObject *stream;

    _page=[[O2PDFDictionary pdfDictionary] retain];

    [_page setNameForKey:"Type" value:"Page"];
    [_page setObjectForKey:"MediaBox" value:[O2PDFArray pdfArrayWithRect:*mediaBox]];

    stream=[O2PDFStream pdfStream];
    [_page setObjectForKey:"Contents" value:stream];
    [_contentStreamStack addObject:stream];

    [_page setObjectForKey:"Parent" value:[self referenceForObject:_pages]];

    [self referenceForObject:_page];
}

-(void)internIndirectObjects {
    int i;

    for(i=0;i<[_indirectObjects count];i++){ // do not cache 'count', can grow during encoding
        O2PDFObject    *object=[_indirectObjects objectAtIndex:i];
        O2PDFxrefEntry *entry=[_indirectEntries objectAtIndex:i];
        if (![object isKindOfClass:[NSNull class]]) {
            unsigned        position=[self length];
            [entry setPosition:position];

            [self appendFormat:@"%d %d obj\n",[entry number],[entry generation]];
            [object encodeWithPDFContext:self];
            [self appendFormat:@"endobj\n"];
        }
    }
    [_indirectObjects removeAllObjects];
    [_indirectEntries removeAllObjects];
}

-(void)internIndirectObjectsExcluding:(NSArray *)array {
    int i;

    for(i=0;i<[_indirectObjects count];i++){ // do not cache 'count', can grow during encoding
        O2PDFObject    *object=[[[_indirectObjects objectAtIndex:i] retain] autorelease];
        if (![array containsObject:object]) {
            O2PDFxrefEntry *entry=[_indirectEntries objectAtIndex:i];
            [_indirectObjects replaceObjectAtIndex:i withObject:[NSNull null]];
            if (![object isKindOfClass:[NSNull class]]) {
                unsigned        position=[self length];
                [entry setPosition:position];

                [self appendFormat:@"%d %d obj\n",[entry number],[entry generation]];
                [object encodeWithPDFContext:self];
                [self appendFormat:@"endobj\n"];

                // We're done with this object
                NSMapRemove(_objectToRef,object);
            }
        }
    }
}

-(void)endPage {
   O2PDFInteger pageCount=0;

   [_contentStreamStack removeLastObject];

   [_kids addObject:_page];

   [_pages getIntegerForKey:"Count" value:&pageCount];
   pageCount++;
   [_pages setIntegerForKey:"Count" value:pageCount];

   [_page release];
   _page=nil;

   // Encode any indirect object now, so they can be released - don't encode the catalog, kids or pages now
   // else subsequent pages are not saved, because the 'Kids' array has already been encoded and we get
   // some invalid PDF
   // FIXME: The following (i.e. per page spooling) does not work as supposed. If uncommented, the generated PDF
   //        would contain the Kids object more than once, and its structure would be spoiled.
   //        Leaving this commented, deactivates per page spooling, however the final PDF will be perfectly valid.
   // [self internIndirectObjectsExcluding:[NSArray arrayWithObjects:_kids, _pages, _catalog, nil]];
}

-(void)close {
    // Encode the remaining references
    [self internIndirectObjects];
    [self encodePDFObject:(id)_xref];
    [_dataConsumer release];
    _dataConsumer=nil;
}

@end
