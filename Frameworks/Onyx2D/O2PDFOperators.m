/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2PDFOperators.h>
#import <Onyx2D/O2PDFOperatorTable.h>
#import <Onyx2D/O2PDFScanner.h>
#import <Onyx2D/O2PDFContentStream.h>
#import <Onyx2D/O2PDFObject.h>
#import <Onyx2D/O2PDFObject_Name.h>
#import <Onyx2D/O2PDFArray.h>
#import <Onyx2D/O2PDFDictionary.h>
#import <Onyx2D/O2PDFStream.h>
#import <Onyx2D/O2PDFString.h>
#import <Onyx2D/O2DataProvider.h>
#import <Onyx2D/O2PDFFunction_Type2.h>
#import <Onyx2D/O2PDFFunction_Type3.h>
#import "O2PDFCharWidths.h"
#import "O2PDFFont.h"

#import <Onyx2D/O2Context.h>
#import <Onyx2D/O2Color.h>
#import <Onyx2D/O2ColorSpace+PDF.h>
#import <Onyx2D/O2Image+PDF.h>
#import <Onyx2D/O2Function+PDF.h>
#import <Onyx2D/O2Shading+PDF.h>
#import <Foundation/NSData.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>

#import <stddef.h>

static O2Context *kgContextFromInfo(void *info) {
   return (O2Context *)info;
}

// closepath, fill, stroke
void O2PDF_render_b(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   
   O2ContextClosePath(context);
   O2ContextDrawPath(context,kO2PathFillStroke);
}

// fill, stroke
void O2PDF_render_B(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   
   O2ContextDrawPath(context,kO2PathFillStroke);
}

// closepath, eofill, stroke
void O2PDF_render_b_star(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   
   O2ContextClosePath(context);
   O2ContextDrawPath(context,kO2PathEOFillStroke);
}

// eofill, stroke
void O2PDF_render_B_star(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   
   O2ContextDrawPath(context,kO2PathEOFillStroke);
}

// Begin marked-content sequence with property list
void O2PDF_render_BDC(O2PDFScanner *scanner,void *info) {
   O2PDFObject *properties=NULL;
   const char      *tag=NULL;
   
   if(!O2PDFScannerPopObject(scanner,&properties)){
    O2PDFError(__FILE__,__LINE__,@"popDictionary failed");
    return;
}

   if(!O2PDFScannerPopName(scanner,&tag)){
    O2PDFError(__FILE__,__LINE__,@"popName failed");
    return;
   }
}

// Begin inline image object
void O2PDF_render_BI(O2PDFScanner *scanner,void *info) {
// do nothing, all the values are pushed on the stack and ID handles them
}

// Begin marked-content sequence
void O2PDF_render_BMC(O2PDFScanner *scanner,void *info) {
   const char *tag=NULL;
   
   if(!O2PDFScannerPopName(scanner,&tag)){
    O2PDFError(__FILE__,__LINE__,@"popName failed");
    return;
}
}

// Begin text object
void O2PDF_render_BT(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   
   O2ContextSetTextMatrix(context,O2AffineTransformIdentity);
   O2ContextSetTextLineMatrix(context,O2AffineTransformIdentity);
}

// Begin compatibility section
void O2PDF_render_BX(O2PDFScanner *scanner,void *info) {
   O2PDFFix(__FILE__,__LINE__,@"BX unimplemented");
}

// curveto, Append curved segment to path, three control points
void O2PDF_render_c(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    x1,y1,x2,y2,x3,y3;
   
   if(!O2PDFScannerPopNumber(scanner,&y3)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   if(!O2PDFScannerPopNumber(scanner,&x3)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   if(!O2PDFScannerPopNumber(scanner,&y2)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   if(!O2PDFScannerPopNumber(scanner,&x2)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   if(!O2PDFScannerPopNumber(scanner,&y1)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");   
    return;
   }
   if(!O2PDFScannerPopNumber(scanner,&x1)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   
   O2ContextAddCurveToPoint(context,x1,y1,x2,y2,x3,y3);
}

// concat, Concatenate matrix to current transformation matrix
void O2PDF_render_cm(O2PDFScanner *scanner,void *info) {
   O2Context        *context=kgContextFromInfo(info);
   O2AffineTransform matrix;
   
   if(!O2PDFScannerPopNumber(scanner,&matrix.ty)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   if(!O2PDFScannerPopNumber(scanner,&matrix.tx)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   if(!O2PDFScannerPopNumber(scanner,&matrix.d)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   if(!O2PDFScannerPopNumber(scanner,&matrix.c)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   if(!O2PDFScannerPopNumber(scanner,&matrix.b)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   if(!O2PDFScannerPopNumber(scanner,&matrix.a)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }

   O2ContextConcatCTM(context,matrix);
}

O2ColorSpaceRef createColorSpaceFromScanner(O2PDFScanner *scanner,void *info,const char *name) {
   O2ColorSpaceRef result=NULL;
   
   if(strcmp(name,"DeviceGray")==0)
    result=O2ColorSpaceCreateDeviceGray();
   else if(strcmp(name,"DeviceRGB")==0)
    result=O2ColorSpaceCreateDeviceRGB();
   else if(strcmp(name,"DeviceCMYK")==0)
    result=O2ColorSpaceCreateDeviceCMYK();
   else {
    O2PDFContentStream *content=[scanner contentStream];
    O2PDFObject        *object=[content resourceForCategory:"ColorSpace" name:name];
    
    if(object==nil){
     O2PDFError(__FILE__,__LINE__,@"Unable to find color space named %s",name);
     return NULL;
    }
   
    return [O2ColorSpace createColorSpaceFromPDFObject:object];
   }
   
   return result;
}

// setcolorspace, Set color space for stroking operations
void O2PDF_render_CS(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   const char     *name;
   O2ColorSpaceRef colorSpace;
   
   if(!O2PDFScannerPopName(scanner,&name))
    return;
    
   colorSpace=createColorSpaceFromScanner(scanner,info,name);
   
   if(colorSpace!=NULL){
    O2ContextSetStrokeColorSpace(context,colorSpace);
    O2ColorSpaceRelease(colorSpace);
   }
}

// setcolorspace, Set color space for nonstroking operations
void O2PDF_render_cs(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   const char     *name;
   O2ColorSpaceRef colorSpace;
   
   if(!O2PDFScannerPopName(scanner,&name))
    return;
    
   colorSpace=createColorSpaceFromScanner(scanner,info,name);
   
   if(colorSpace!=NULL){
    O2ContextSetFillColorSpace(context,colorSpace);
    O2ColorSpaceRelease(colorSpace);
   }
}

// setdash, Set line dash pattern
void O2PDF_render_d(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    phase;
   O2PDFArray  *array;
   int          i,count;
   
   if(!O2PDFScannerPopNumber(scanner,&phase))
    return;
   if(!O2PDFScannerPopArray(scanner,&array))
    return;
   count=[array count];
   {
    O2PDFReal lengths[count];
   
    for(i=0;i<count;i++)
     if(![array getNumberAtIndex:i value:lengths+i])
      return;
   
    O2ContextSetLineDash(context,phase,lengths,count);
   }
}

// setcharwidth, Set glyph with in Type 3 font
void O2PDF_render_d0(O2PDFScanner *scanner,void *info) {
   O2PDFFix(__FILE__,__LINE__,@"d0 unimplemented");
}

// setcachedevice, Set glyph width and bounding box in Type 3 font
void O2PDF_render_d1(O2PDFScanner *scanner,void *info) {
   O2PDFFix(__FILE__,__LINE__,@"d1 unimplemented");
}

// Invoke named XObject
void O2PDF_render_Do(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFContentStream *content=[scanner contentStream];
   const char         *name;
   O2PDFObject        *resource;
   O2PDFStream        *stream;
   O2PDFDictionary    *dictionary;
   const char         *subtype;
   
   if(!O2PDFScannerPopName(scanner,&name))
    return;

   if((resource=[content resourceForCategory:"XObject" name:name])==nil)
    return;
   
   if(![resource checkForType:kO2PDFObjectTypeStream value:&stream])
    return;
   
   dictionary=[stream dictionary];
   
   if([dictionary getNameForKey:"Type" value:&name])
    if(strcmp(name,"XObject")!=0)
     return;

   if(![dictionary getNameForKey:"Subtype" value:&subtype])
    return;
    
   if(strcmp(subtype,"Form")==0){
    O2PDFDictionary    *resources;
    O2PDFContentStream *contentStream;
    O2PDFOperatorTable *operatorTable;
    O2PDFScanner       *doScanner;
    O2PDFDictionary    *group;
    BOOL doIt=YES;
    
    if(![dictionary getDictionaryForKey:"Resources" value:&resources])
     resources=nil;
    
    if([dictionary getDictionaryForKey:"Group" value:&group]){
     const char *name;
     
     if([group getNameForKey:"S" value:&name]){
      if(strcmp(name,"Transparency")==0){
       ;//doIt=NO;
       //NSLog(@"dictionry=%@",dictionary);
      }
     }
    }
        
    contentStream=[[[O2PDFContentStream alloc] initWithStream:stream resources:resources parent:[scanner contentStream]] autorelease];
    operatorTable=[O2PDFOperatorTable renderingOperatorTable];
    doScanner=[[[O2PDFScanner alloc] initWithContentStream:contentStream operatorTable:operatorTable info:info] autorelease];

if(doIt)
    [doScanner scan];
   }
   else if(strcmp(subtype,"Image")==0){
    O2Image *image=[O2Image imageWithPDFObject:stream];
    
    if(image!=NULL){
     O2ContextDrawImage(context,O2RectMake(0,0,1,1),image);
    }

    if(image!=NULL)
     [image release];
   }
   else if(strcmp(subtype,"PS")==0){
    NSLog(@"PS");
   }
   else {
    O2PDFError(__FILE__,__LINE__,@"Unknown object subtype %s for Do",subtype);
   }
}

// Define marked-content point with property list
void O2PDF_render_DP(O2PDFScanner *scanner,void *info) {
   O2PDFObject *properties=NULL;
   const char  *tag=NULL;
   
   if(!O2PDFScannerPopObject(scanner,&properties)){
    O2PDFError(__FILE__,__LINE__,@"popDictionary failed");
    return;
}

   if(!O2PDFScannerPopName(scanner,&tag)){
    O2PDFError(__FILE__,__LINE__,@"popName failed");
    return;
   }
}

// End inline image object
void O2PDF_render_EI(O2PDFScanner *scanner,void *info) {
// do nothing, everything is implemented by ID, EI is a formality
}

// End marked-content sequence
void O2PDF_render_EMC(O2PDFScanner *scanner,void *info) {
// do nothing
}

// End text object
void O2PDF_render_ET(O2PDFScanner *scanner,void *info) {
// do nothing
}

// End compatibility section
void O2PDF_render_EX(O2PDFScanner *scanner,void *info) {
   O2PDFFix(__FILE__,__LINE__,@"EX unimplemented");
}

// fill, fill path using nonzero winding number rule
void O2PDF_render_f(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   
   O2ContextFillPath(context);
}

// fill, fill path using nonzero winding number rule (obsolete)
void O2PDF_render_F(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   
   O2ContextFillPath(context);
}

// eofill, fill path using even-odd rule
void O2PDF_render_f_star(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   
   O2ContextEOFillPath(context);
}

// setgray, set gray level for stroking operations
void O2PDF_render_G(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    gray;
   
   if(!O2PDFScannerPopNumber(scanner,&gray)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   
   O2ContextSetGrayStrokeColor(context,gray,O2ColorGetAlpha(O2ContextStrokeColor(context)));
}

// setgray, set gray level for nonstroking operations
void O2PDF_render_g(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    gray;
   
   if(!O2PDFScannerPopNumber(scanner,&gray)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   
   O2ContextSetGrayFillColor(context,gray,O2ColorGetAlpha(O2ContextFillColor(context)));
}

static void establishFontInContext(O2Context *context,O2PDFFont *pdfFont,O2PDFReal scale){
   O2ContextSetFont(context,[pdfFont graphicsFont]);
   O2ContextSetFontSize(context,scale);
   O2ContextSetEncoding(context,[pdfFont encoding]);
   O2ContextSetPDFCharWidths(context,[pdfFont pdfCharWidths]);
}

// Set parameters from graphics state parameter dictionary
void O2PDF_render_gs(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFContentStream *content=[scanner contentStream];
   O2PDFObject        *resource;
   O2PDFDictionary    *graphicsState;
   const char         *name;
   O2PDFReal           number;
   O2PDFInteger        integer;
   O2PDFArray         *array;
   O2PDFDictionary    *dictionary;
   O2PDFBoolean        boolean;
   
   if(!O2PDFScannerPopName(scanner,&name))
    return;
    
   if((resource=[content resourceForCategory:"ExtGState" name:name])==nil)
    return;
   
   if(![resource checkForType:kO2PDFObjectTypeDictionary value:&graphicsState])
    return;
   
   if([graphicsState getNameForKey:"Type" value:&name])
    if(strcmp(name,"ExtGState")!=0)
     return;

   if([graphicsState getNumberForKey:"LW" value:&number])
    O2ContextSetLineWidth(context,number);
    
   if([graphicsState getIntegerForKey:"LC" value:&integer])
    O2ContextSetLineCap(context,integer);
    
   if([graphicsState getIntegerForKey:"LJ" value:&integer])
    O2ContextSetLineJoin(context,integer);
    
   if([graphicsState getNumberForKey:"ML" value:&number])
    O2ContextSetMiterLimit(context,number);
    
   if([graphicsState getArrayForKey:"D" value:&array]){
    O2PDFArray  *dashesArray;
    O2PDFInteger phase;
    
    if([array getArrayAtIndex:0 value:&dashesArray] && [array getIntegerAtIndex:1 value:&phase]){
     unsigned   count;
     O2PDFReal *lengths;
     
     if([dashesArray getNumbers:&lengths count:&count]){
      O2ContextSetLineDash(context,phase,lengths,count);
      NSZoneFree(NULL,lengths);
   }
   }
    else {
     O2PDFError(__FILE__,__LINE__,@"D entry does not contain dashes and/or phase");
   }
   }
   
   if([graphicsState getNameForKey:"RI" value:&name]){
    if(strcmp(name,"AbsoluteColorimetric")==0)
     O2ContextSetRenderingIntent(context,kO2RenderingIntentAbsoluteColorimetric);
    else if(strcmp(name,"RelativeColorimetric")==0)
     O2ContextSetRenderingIntent(context,kO2RenderingIntentRelativeColorimetric);
    else if(strcmp(name,"Saturation")==0)
     O2ContextSetRenderingIntent(context,kO2RenderingIntentSaturation);
    else if(strcmp(name,"Perceptual")==0)
     O2ContextSetRenderingIntent(context,kO2RenderingIntentPerceptual);
   }
   
   // Skip, OP, op and OPM as they control overprinting and can be ignored
   
   if([graphicsState getArrayForKey:"Font" value:&array]){
    O2PDFDictionary *fontDictionary;
    O2PDFReal        scale;
    
    if([array getDictionaryAtIndex:0 value:&fontDictionary] && [array getNumberAtIndex:1 value:&scale]){
     O2PDFFont *font=[O2PDFFont createWithPDFDictionary:fontDictionary];
     
     establishFontInContext(context,font,scale);
     
     [font release];
   }
    else {
     O2PDFError(__FILE__,__LINE__,@"Font entry does not contain dictionary and/or scale");
   }
   }
   
   // Skip BG and BG2, black generation
   
   if([graphicsState getNameForKey:"BM" value:&name]){
    if(strcmp(name,"Normal")==0)
     O2ContextSetBlendMode(context,kO2BlendModeNormal);
    else if(strcmp(name,"Multiply")==0)
     O2ContextSetBlendMode(context,kO2BlendModeMultiply);
    else if(strcmp(name,"Screen")==0)
     O2ContextSetBlendMode(context,kO2BlendModeScreen);
    else if(strcmp(name,"Overlay")==0)
     O2ContextSetBlendMode(context,kO2BlendModeOverlay);
    else if(strcmp(name,"Darken")==0)
     O2ContextSetBlendMode(context,kO2BlendModeDarken);
    else if(strcmp(name,"Lighten")==0)
     O2ContextSetBlendMode(context,kO2BlendModeLighten);
    else if(strcmp(name,"ColorDodge")==0)
     O2ContextSetBlendMode(context,kO2BlendModeColorDodge);
    else if(strcmp(name,"ColorBurn")==0)
     O2ContextSetBlendMode(context,kO2BlendModeColorBurn);
    else if(strcmp(name,"HardLight")==0)
     O2ContextSetBlendMode(context,kO2BlendModeHardLight);
    else if(strcmp(name,"SoftLight")==0)
     O2ContextSetBlendMode(context,kO2BlendModeSoftLight);
    else if(strcmp(name,"Difference")==0)
     O2ContextSetBlendMode(context,kO2BlendModeDifference);
    else if(strcmp(name,"Exclusion")==0)
     O2ContextSetBlendMode(context,kO2BlendModeExclusion);
    else if(strcmp(name,"Hue")==0)
     O2ContextSetBlendMode(context,kO2BlendModeHue);
    else if(strcmp(name,"Saturation")==0)
     O2ContextSetBlendMode(context,kO2BlendModeSaturation);
    else if(strcmp(name,"Color")==0)
     O2ContextSetBlendMode(context,kO2BlendModeColor);
    else if(strcmp(name,"Luminosity")==0)
     O2ContextSetBlendMode(context,kO2BlendModeLuminosity);
    else
     O2PDFError(__FILE__,__LINE__,@"Unknown blend mode %s",name);
   }
   
   // Skip UCR and UCR2, undercolor removal
   
   // Skip TR and TR2, transfer function
   
   // Skip HT, halftone
   
   if([graphicsState getNumberForKey:"FL" value:&number]){
    O2ContextSetFlatness(context,number);
   }
   
   // Skip SM, shading smoothing tolerance
   
   // Skip SA, stroke adjustment
   

   if([graphicsState getNameForKey:"SMask" value:&name]){
    O2PDFFix(__FILE__,__LINE__,@"SMask not implemented, name=%s",name);
   }
   else if([graphicsState getDictionaryForKey:"SMask" value:&dictionary]){
    O2PDFFix(__FILE__,__LINE__,@"SMask not implemented");
#if 0   
    O2ImageRef mask=[O2Image imageWithPDFObject:dictionary];
    
    if(mask!=NULL)
     O2ContextClipToMask(context,O2RectMake(0,0,1,1),mask);
#endif
   }

   if([graphicsState getNumberForKey:"CA" value:&number]){
    [context setStrokeAlpha:number];
   }
   
   if([graphicsState getNumberForKey:"ca" value:&number]){
    [context setFillAlpha:number];
   }
   
   // Skip AIS, alpha source flag
   
   // Skip TK, text knockout
}

// closepath
void O2PDF_render_h(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   
   O2ContextClosePath(context);
}

// setflat, Set flatness tolerance
void O2PDF_render_i(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    flatness;
   
   if(!O2PDFScannerPopNumber(scanner,&flatness)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }

   O2ContextSetFlatness(context,flatness);
}

// Begin inline image data
void O2PDF_render_ID(O2PDFScanner *scanner,void *info) {
   O2PDFDictionary *dictionary=[O2PDFDictionary pdfDictionary];
   
   while(YES) {
    O2PDFObject *value;
    const char  *key;
    
    if(!O2PDFScannerPopObject(scanner,&value))
     break;
    
    if(!O2PDFScannerPopName(scanner,&key)){
     O2PDFError(__FILE__,__LINE__,@"popName failed");
     break;
    }
    
    [dictionary setObjectForKey:key value:value];
   }
   
   O2PDFInteger bitsPerComponent=0;
   O2PDFObject *colorSpace=NULL;
   O2PDFArray  *decode=NULL;
   O2PDFObject *decodeParms=NULL;
   O2PDFObject *filter=NULL;
   O2PDFInteger height=0;
   O2PDFBoolean imageMask=NO;
   O2PDFObject *intent=NULL;
   O2PDFBoolean interpolate=NO;
   O2PDFInteger width=0;
   
   if(![dictionary getIntegerForKey:"BPC" value:&bitsPerComponent])
    if(![dictionary getIntegerForKey:"BitsPerComponent" value:&bitsPerComponent]){
    }
    
   if(![dictionary getObjectForKey:"CS" value:&colorSpace])
    if(![dictionary getObjectForKey:"ColorSpace" value:&colorSpace]){
    }

   if(![dictionary getArrayForKey:"D" value:&decode])
    if(![dictionary getArrayForKey:"Decode" value:&decode]){
    }

   if(![dictionary getObjectForKey:"DP" value:&decodeParms])
    if(![dictionary getObjectForKey:"DecodeParms" value:&decodeParms]){
    }

   if(![dictionary getObjectForKey:"F" value:&filter])
    if(![dictionary getObjectForKey:"Filter" value:&filter]){
    }

   if(![dictionary getIntegerForKey:"H" value:&height])
    if(![dictionary getIntegerForKey:"Height" value:&height]){
    }

   if(![dictionary getBooleanForKey:"IM" value:&imageMask])
    if(![dictionary getBooleanForKey:"ImageMask" value:&imageMask]){
    }
#if 0
   if(![dictionary getObjectForKey:"Intent" value:&intent]){
   }
#endif

   if(![dictionary getBooleanForKey:"I" value:&interpolate])
    if(![dictionary getBooleanForKey:"Interpolate" value:&interpolate]){
    }

   if(![dictionary getIntegerForKey:"W" value:&width])
    if(![dictionary getIntegerForKey:"Width" value:&width]){
    }
  
   size_t bytesPerRow=(width*bitsPerComponent)/8;
   
   NSData *data=O2PDFScannerCreateDataWithLength(scanner,height*bytesPerRow);
   
}

// setlinejoin, Set line join style
void O2PDF_render_j(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFInteger linejoin;
   
   if(!O2PDFScannerPopInteger(scanner,&linejoin)){
    O2PDFFix(__FILE__,__LINE__,@"popInteger failed");
    return;
   }
   O2ContextSetLineJoin(context,linejoin);
}

// setlinecap, Set line cap style
void O2PDF_render_J(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFInteger linecap;
   
   if(!O2PDFScannerPopInteger(scanner,&linecap)){
    O2PDFFix(__FILE__,__LINE__,@"popInteger failed");
    return;
   }

   O2ContextSetLineCap(context,linecap);
}

// setcmykcolor, Set CMYK color for stroking operations
void O2PDF_render_K(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    c,m,y,k;
   
   if(!O2PDFScannerPopNumber(scanner,&k)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   if(!O2PDFScannerPopNumber(scanner,&y)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   if(!O2PDFScannerPopNumber(scanner,&m)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   if(!O2PDFScannerPopNumber(scanner,&c)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
    
   [context setStrokeColorC:c m:m y:y k:k];
}

// setcmykcolor, Set CMYK color for nonstroking operations
void O2PDF_render_k(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    c,m,y,k;
   
   if(!O2PDFScannerPopNumber(scanner,&k)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   if(!O2PDFScannerPopNumber(scanner,&y)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   if(!O2PDFScannerPopNumber(scanner,&m)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   if(!O2PDFScannerPopNumber(scanner,&c)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   
   [context setFillColorC:c m:m y:y k:k];
}

// lineto, Append straight line segment to path
void O2PDF_render_l(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    x,y;
   
   if(!O2PDFScannerPopNumber(scanner,&y)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   if(!O2PDFScannerPopNumber(scanner,&x)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   
   O2ContextAddLineToPoint(context,x,y);
}

// moveto, Begin new subpath
void O2PDF_render_m(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    x,y;
   
   if(!O2PDFScannerPopNumber(scanner,&y)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   if(!O2PDFScannerPopNumber(scanner,&x)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   
   O2ContextMoveToPoint(context,x,y);
}

// setmiterlimit, Set miter limit
void O2PDF_render_M(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    limit;
   
   if(!O2PDFScannerPopNumber(scanner,&limit)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }

   O2ContextSetMiterLimit(context,limit);
}

void O2PDF_render_MP(O2PDFScanner *scanner,void *info) {
   const char *tag=NULL;
   
   if(!O2PDFScannerPopName(scanner,&tag)){
    O2PDFError(__FILE__,__LINE__,@"popName failed");
    return;
}
}

// End path without filling or stroking
void O2PDF_render_n(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   
   O2ContextBeginPath(context);
}

// gsave
void O2PDF_render_q(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   
   O2ContextSaveGState(context);
}

// grestore
void O2PDF_render_Q(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   
   O2ContextRestoreGState(context);
}

// Append rectangle to path
void O2PDF_render_re(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2Rect     rect;

   if(!O2PDFScannerPopNumber(scanner,&rect.size.height))
    return;
   if(!O2PDFScannerPopNumber(scanner,&rect.size.width))
    return;
   if(!O2PDFScannerPopNumber(scanner,&rect.origin.y))
    return;
   if(!O2PDFScannerPopNumber(scanner,&rect.origin.x))
    return;

   O2ContextAddRect(context,rect);
}

// setrgbcolor, Set RGB color for stroking operations
void O2PDF_render_RG(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    r,g,b;

   if(!O2PDFScannerPopNumber(scanner,&b))
    return;
   if(!O2PDFScannerPopNumber(scanner,&g))
    return;
   if(!O2PDFScannerPopNumber(scanner,&r))
    return;
   
   [context setStrokeColorRed:r green:g blue:b];
}

// setrgbcolor, Set RGB color for nonstroking operations
void O2PDF_render_rg(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    r,g,b;

   if(!O2PDFScannerPopNumber(scanner,&b))
    return;

   if(!O2PDFScannerPopNumber(scanner,&g))
    return;

   if(!O2PDFScannerPopNumber(scanner,&r))
    return;
   
   [context setFillColorRed:r green:g blue:b];
}

// name ri, Set color rendering intent
void O2PDF_render_ri(O2PDFScanner *scanner,void *info) {
   O2Context  *context=kgContextFromInfo(info);
   const char *name;
   
   if(!O2PDFScannerPopName(scanner,&name))
    return;
   
   O2ContextSetRenderingIntent(context,O2ImageRenderingIntentWithName(name));
}

// closepath stroke
void O2PDF_render_s(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   
   O2ContextClosePath(context);
   O2ContextStrokePath(context);
}

// stroke
void O2PDF_render_S(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   
   O2ContextStrokePath(context);
}

// setcolor, Set color for stroking operations
void O2PDF_render_SC(O2PDFScanner *scanner,void *info) {
   O2Context    *context=kgContextFromInfo(info);
   O2ColorRef color=O2ContextStrokeColor(context);
   O2ColorSpaceRef colorSpace=O2ColorGetColorSpace(color);
   unsigned      numberOfComponents=O2ColorSpaceGetNumberOfComponents(colorSpace);
   int           count=numberOfComponents;
   float         components[count+1];
   
   components[count]=O2ColorGetAlpha(color);
   while(--count>=0)
    if(!O2PDFScannerPopNumber(scanner,components+count)){
     O2PDFError(__FILE__,__LINE__,@"underflow in SC, numberOfComponents=%d,count=%d",numberOfComponents,count);
     return;
    }
    
   O2ContextSetStrokeColor(context,components);
}

// setcolor, Set color for nonstroking operations
void O2PDF_render_sc(O2PDFScanner *scanner,void *info) {
   O2Context    *context=kgContextFromInfo(info);
   O2ColorRef color=O2ContextFillColor(context);
   O2ColorSpaceRef colorSpace=O2ColorGetColorSpace(color);
   unsigned      numberOfComponents=O2ColorSpaceGetNumberOfComponents(colorSpace);
   int           count=numberOfComponents;
   float         components[count+1];
   
   components[count]=O2ColorGetAlpha(color);
   while(--count>=0)
    if(!O2PDFScannerPopNumber(scanner,components+count)){
     O2PDFError(__FILE__,__LINE__,@"underflow in sc, numberOfComponents=%d,count=%d",numberOfComponents,count);
     return;
    }
    
   O2ContextSetFillColor(context,components);
}

// setcolor, Set color for stroking operations, ICCBased and special color spaces
void O2PDF_render_SCN(O2PDFScanner *scanner,void *info) {
   O2Context    *context=kgContextFromInfo(info);
   O2ColorRef color=O2ContextStrokeColor(context);
   O2ColorSpaceRef colorSpace=O2ColorGetColorSpace(color);
   unsigned      numberOfComponents=O2ColorSpaceGetNumberOfComponents(colorSpace);
   int           count=numberOfComponents;
   float         components[count+1];
   
   components[count]=O2ColorGetAlpha(color);
   while(--count>=0)
    if(!O2PDFScannerPopNumber(scanner,components+count)){
     O2PDFError(__FILE__,__LINE__,@"underflow in SCN, numberOfComponents=%d,count=%d",numberOfComponents,count);
     return;
    }
    
   O2ContextSetStrokeColor(context,components);
}

// setcolor, Set color for nonstroking operations, ICCBased and special color spaces
void O2PDF_render_scn(O2PDFScanner *scanner,void *info) {
   O2Context    *context=kgContextFromInfo(info);
   O2ColorRef color=O2ContextFillColor(context);
   O2ColorSpaceRef colorSpace=O2ColorGetColorSpace(color);
   unsigned      numberOfComponents=O2ColorSpaceGetNumberOfComponents(colorSpace);
   int           count=numberOfComponents;
   O2PDFReal     components[count+1];
   
   components[count]=O2ColorGetAlpha(color);
   while(--count>=0)
    if(!O2PDFScannerPopNumber(scanner,&components[count])){
     O2PDFError(__FILE__,__LINE__,@"underflow in scn, numberOfComponents=%d,count=%d",numberOfComponents,count);
     return;
    }

   O2ContextSetFillColor(context,components);
}

// shfill, Paint area defined by shading pattern
void O2PDF_render_sh(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFContentStream *content=[scanner contentStream];
   O2PDFObject        *resource;
   const char         *name;
   O2Shading *shading=NULL;
   
   if(!O2PDFScannerPopName(scanner,&name))
    return;
    
   if((resource=[content resourceForCategory:"Shading" name:name])==nil)
    return;
   
   shading=[O2Shading shadingWithPDFObject:resource];
   
   if(shading!=NULL){
    O2ContextDrawShading(context,shading);
    [shading release];
   }
}

// Move to start of next text line
void O2PDF_render_T_star(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);

   O2AffineTransform Tlm=O2ContextGetTextLineMatrix(context);
   O2AffineTransform result=O2AffineTransformTranslate(Tlm,0,-O2ContextGetTextLeading(context));
   
   O2ContextSetTextMatrix(context,result);
   O2ContextSetTextLineMatrix(context,result);
}

// Set character spacing
void O2PDF_render_Tc(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal  spacing;
   
   if(!O2PDFScannerPopNumber(scanner,&spacing)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   
   O2ContextSetCharacterSpacing(context,spacing);
}

// Move text position
void O2PDF_render_Td(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    x,y;
   
   if(!O2PDFScannerPopNumber(scanner,&y))
    return;
   if(!O2PDFScannerPopNumber(scanner,&x))
    return;
    
   O2AffineTransform Tlm=O2ContextGetTextLineMatrix(context);
   O2AffineTransform result=O2AffineTransformTranslate(Tlm,x,y);
   
   O2ContextSetTextMatrix(context,result);
   O2ContextSetTextLineMatrix(context,result);
}

// Move text position and set leading
void O2PDF_render_TD(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    x,y;
   
   if(!O2PDFScannerPopNumber(scanner,&y))
    return;
   if(!O2PDFScannerPopNumber(scanner,&x))
    return;
    
  // NSLog(@"O2ContextSetTextPosition %f %f, O2ContextSetTextLeading %f",x,y,-y);
   O2ContextSetTextLeading(context,-y);

   O2AffineTransform Tlm=O2ContextGetTextLineMatrix(context);
   O2AffineTransform result=O2AffineTransformTranslate(Tlm,x,y);

   O2ContextSetTextMatrix(context,result);
   O2ContextSetTextLineMatrix(context,result);
}

// Set text font and size
void O2PDF_render_Tf(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFContentStream *content=[scanner contentStream];
   O2PDFObject     *resource;
   O2PDFDictionary *dictionary;
   O2PDFFont          *pdfFont=nil;
   O2PDFReal           scale=1.0;
   const char         *name;

   if(!O2PDFScannerPopNumber(scanner,&scale)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
   if(!O2PDFScannerPopName(scanner,&name)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
   }
       
   if((resource=[content resourceForCategory:"Font" name:name])==nil){
    O2PDFError(__FILE__,__LINE__,@"Unable to locate font named %s",name);
    return;
   }
   
   if([resource objectType]==O2PDFObjectTypeCached)
    pdfFont=(O2PDFFont*)resource;
   else {
    
    if(![resource checkForType:kO2PDFObjectTypeDictionary value:&dictionary]){
     O2PDFError(__FILE__,__LINE__,@"Font resource is not a dictionary");
     return;
   }
    
    if((pdfFont=[O2PDFFont createWithPDFDictionary:dictionary])==nil){
     O2PDFError(__FILE__,__LINE__,@"Unable to create font object");
     return;
   }
    
    [content replaceResource:dictionary forCategory:"Font" name:name withObject:pdfFont];
    
    [pdfFont release];
   }
   
   establishFontInContext(context,pdfFont,scale);
}

// show
void O2PDF_render_Tj(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFString *string;
   
   if(!O2PDFScannerPopString(scanner,&string)){
    O2PDFFix(__FILE__,__LINE__,@"popString failed");
    return;
   }
   
   O2ContextShowText(context,(const char *)O2PDFStringGetBytePtr(string),O2PDFStringGetLength(string));
}

// Show text, alowing individual glyph positioning
void O2PDF_render_TJ(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFArray  *array;
   int          i,count;
   
   if(!O2PDFScannerPopArray(scanner,&array))
    return;
    
   count=[array count];
   for(i=0;i<count;i++){
    O2PDFObject    *object;
    O2PDFReal       real;
    O2PDFString    *string;

    if(![array getObjectAtIndex:i value:&object])
     return;

    if([object checkForType:kO2PDFObjectTypeReal value:&real]){
// The amount is expressed in thousands of a text space unit and subtracted 
     O2AffineTransform Tm=O2ContextGetTextMatrix(context);
     O2Size            advance=O2SizeApplyAffineTransform(O2SizeMake(real/1000.0,0),Tm);

     Tm.tx-=advance.width;
     Tm.ty-=advance.height;

     O2ContextSetTextMatrix(context,Tm);
    }
    else if([object checkForType:kO2PDFObjectTypeString value:&string]){
     O2ContextShowText(context,(const char *)O2PDFStringGetBytePtr(string),O2PDFStringGetLength(string));
    }
    else {
     O2PDFFix(__FILE__,__LINE__,@"Invalid object in TJ array");
   } 
}
}

// Set text leading
void O2PDF_render_TL(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal  value;
   
   if(!O2PDFScannerPopNumber(scanner,&value)){
    O2PDFError(__FILE__,__LINE__,@"popNumber failed");
    return;
}

  // NSLog(@"O2ContextSetTextLeading %f ",value);
   O2ContextSetTextLeading(context,value);
}

// Set text matrix and text line matrix
void O2PDF_render_Tm(O2PDFScanner *scanner,void *info) {
   O2Context        *context=kgContextFromInfo(info);
   O2AffineTransform matrix;
   
   if(!O2PDFScannerPopNumber(scanner,&matrix.ty))
    return;
   if(!O2PDFScannerPopNumber(scanner,&matrix.tx))
    return;
   if(!O2PDFScannerPopNumber(scanner,&matrix.d))
    return;
   if(!O2PDFScannerPopNumber(scanner,&matrix.c))
    return;
   if(!O2PDFScannerPopNumber(scanner,&matrix.b))
    return;
   if(!O2PDFScannerPopNumber(scanner,&matrix.a))
    return;
     
  // NSLog(@"O2ContextSetTextMatrix %f %f %f %f %f %f",matrix.a,matrix.b,matrix.c,matrix.d,matrix.tx,matrix.ty);
   O2ContextSetTextMatrix(context,matrix);
   O2ContextSetTextLineMatrix(context,matrix);
}

// Set text rendering mode
void O2PDF_render_Tr(O2PDFScanner *scanner,void *info) {
   O2Context   *context=kgContextFromInfo(info);
   O2PDFInteger mode;
   
   if(!O2PDFScannerPopInteger(scanner,&mode)){
    O2PDFFix(__FILE__,__LINE__,@"popInteger failed");
    return;
}

   //NSLog(@"O2ContextSetTextDrawingMode %d",mode);
   O2ContextSetTextDrawingMode(context,mode);
}

// Set text rise
void O2PDF_render_Ts(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal  value;
   
   if(!O2PDFScannerPopNumber(scanner,&value))
    return;

  // NSLog(@"O2ContextSetTextRise %f ",value);
   O2ContextSetTextRise(context,value);
}

// Set word spacing
void O2PDF_render_Tw(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    spacing;

   if(!O2PDFScannerPopNumber(scanner,&spacing))
    return;
   
   if(spacing!=0.0)
    O2PDFError(__FILE__,__LINE__,@"Word spacing is not zero %f",spacing);
    
   O2ContextSetWordSpacing(context,spacing);
}

// Set horizontal text scaling
void O2PDF_render_Tz(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    scaling;
 
   if(!O2PDFScannerPopNumber(scanner,&scaling))
    return;

   if(scaling!=100.0)
    O2PDFError(__FILE__,__LINE__,@"Horizontal scaling is not 100 %f",scaling);

   O2ContextSetTextHorizontalScaling(context,scaling);
}

// curveto, Append curved segment to path, initial point replicated
void O2PDF_render_v(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    x2,y2,x3,y3;
   
   if(!O2PDFScannerPopNumber(scanner,&y3))
    return;
   if(!O2PDFScannerPopNumber(scanner,&x3))
    return;
   if(!O2PDFScannerPopNumber(scanner,&y2))
    return;
   if(!O2PDFScannerPopNumber(scanner,&x2))
    return;
   
   O2ContextAddQuadCurveToPoint(context,x2,y2,x3,y3);
}

// setlinewidth, Set line width
void O2PDF_render_w(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    width;

   if(!O2PDFScannerPopNumber(scanner,&width))
    return;
   
   O2ContextSetLineWidth(context,width);
}

// clip, Set clipping path using nonzero winding number rule
void O2PDF_render_W(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   
   O2ContextClip(context);
}

// eoclip, Set clipping path using even-odd rule
void O2PDF_render_W_star(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   
   O2ContextEOClip(context);
}

// curveto, Append curved segment to path, final point replicated
void O2PDF_render_y(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFReal    x1,y1,x3,y3;
   
   if(!O2PDFScannerPopNumber(scanner,&y3))
    return;
   if(!O2PDFScannerPopNumber(scanner,&x3))
    return;
   if(!O2PDFScannerPopNumber(scanner,&y1))
    return;
   if(!O2PDFScannerPopNumber(scanner,&x1))
    return;
   
   O2ContextAddCurveToPoint(context,x1,y1,x3,y3,x3,y3);
}

// Move to next line and show text
// T*string Tj
void O2PDF_render_quote(O2PDFScanner *scanner,void *info) {   
   O2PDF_render_T_star(scanner,info);
   O2PDF_render_Tj(scanner,info);
}

// Set word and character spacing, move to next line, and show text
// same as w Tw c Tc string '
void O2PDF_render_dquote(O2PDFScanner *scanner,void *info) {
   O2Context *context=kgContextFromInfo(info);
   O2PDFString *string;
   O2PDFReal    cspacing;
   O2PDFReal    wspacing;
   
   if(!O2PDFScannerPopString(scanner,&string))
    return;
   if(!O2PDFScannerPopNumber(scanner,&cspacing))
    return;
   if(!O2PDFScannerPopNumber(scanner,&wspacing))
    return;
   
   O2ContextSetWordSpacing(context,wspacing);
   O2ContextSetCharacterSpacing(context,cspacing);
   O2PDF_render_T_star(scanner,info);
   O2ContextShowText(context,(const char *)O2PDFStringGetBytePtr(string),O2PDFStringGetLength(string));
}

void O2PDF_render_populateOperatorTable(O2PDFOperatorTable *table) {
   struct {
    const char           *name;
    O2PDFOperatorCallback callback;
   } ops[]={
    { "b", O2PDF_render_b },
    { "B", O2PDF_render_B },
    { "b*", O2PDF_render_b_star },
    { "B*", O2PDF_render_B_star },
    { "BDC", O2PDF_render_BDC },
    { "BI", O2PDF_render_BI },
    { "BMC", O2PDF_render_BMC },
    { "BT", O2PDF_render_BT },
    { "BX", O2PDF_render_BX },
    { "c", O2PDF_render_c },
    { "cm", O2PDF_render_cm },
    { "CS", O2PDF_render_CS },
    { "cs", O2PDF_render_cs },
    { "d", O2PDF_render_d },
    { "d0", O2PDF_render_d0 },
    { "d1", O2PDF_render_d1 },
    { "Do", O2PDF_render_Do },
    { "DP", O2PDF_render_DP },
    { "EI", O2PDF_render_EI },
    { "EMC", O2PDF_render_EMC },
    { "ET", O2PDF_render_ET },
    { "EX", O2PDF_render_EX },
    { "f", O2PDF_render_f },
    { "F", O2PDF_render_F },
    { "f*", O2PDF_render_f_star },
    { "G", O2PDF_render_G },
    { "g", O2PDF_render_g },
    { "gs", O2PDF_render_gs },
    { "h", O2PDF_render_h },
    { "i", O2PDF_render_i },
    { "ID", O2PDF_render_ID },
    { "j", O2PDF_render_j },
    { "J", O2PDF_render_J },
    { "K", O2PDF_render_K },
    { "k", O2PDF_render_k },
    { "l", O2PDF_render_l },
    { "m", O2PDF_render_m },
    { "M", O2PDF_render_M },
    { "MP", O2PDF_render_MP },
    { "n", O2PDF_render_n },
    { "q", O2PDF_render_q },
    { "Q", O2PDF_render_Q },
    { "re", O2PDF_render_re },
    { "RG", O2PDF_render_RG },
    { "rg", O2PDF_render_rg },
    { "ri", O2PDF_render_ri },
    { "s", O2PDF_render_s },
    { "S", O2PDF_render_S },
    { "SC", O2PDF_render_SC },
    { "sc", O2PDF_render_sc },
    { "SCN", O2PDF_render_SCN },
    { "scn", O2PDF_render_scn },
    { "sh", O2PDF_render_sh },
    { "T*", O2PDF_render_T_star },
    { "Tc", O2PDF_render_Tc },
    { "Td", O2PDF_render_Td },
    { "TD", O2PDF_render_TD },
    { "Tf", O2PDF_render_Tf },
    { "Tj", O2PDF_render_Tj },
    { "TJ", O2PDF_render_TJ },
    { "TL", O2PDF_render_TL },
    { "Tm", O2PDF_render_Tm },
    { "Tr", O2PDF_render_Tr },
    { "Ts", O2PDF_render_Ts },
    { "Tw", O2PDF_render_Tw },
    { "Tz", O2PDF_render_Tz },
    { "v", O2PDF_render_v },
    { "w", O2PDF_render_w },
    { "W", O2PDF_render_W },
    { "W*", O2PDF_render_W_star },
    { "y", O2PDF_render_y },
    { "\'", O2PDF_render_quote },
    { "\"", O2PDF_render_dquote },
    { NULL, NULL }
   };
   int i;
   
   for(i=0;ops[i].name!=NULL;i++)
    [table setCallback:ops[i].callback forName:ops[i].name];
}

