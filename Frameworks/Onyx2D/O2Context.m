/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2Context.h>
#import <Onyx2D/O2BitmapContext.h>
#import <Onyx2D/O2GraphicsState.h>
#import <Onyx2D/O2Color.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2MutablePath.h>
#import <Onyx2D/O2Layer.h>
#import <Onyx2D/O2PDFPage.h>
#import <Onyx2D/O2ClipPhase.h>
#import <Onyx2D/O2Exceptions.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSArray.h>
#import "O2Encoding.h"
#import "O2PDFCharWidths.h"

void O2ContextDefaultShowText(O2ContextRef self,const char *text,unsigned length);

@implementation O2Context

static NSMutableArray *possibleContextClasses=nil;

+(void)initialize {
   if(possibleContextClasses==nil){
    possibleContextClasses=[NSMutableArray new];
    
    [possibleContextClasses addObject:@"O2Context_gdi"];
    [possibleContextClasses addObject:@"O2Context_builtin"];
    [possibleContextClasses addObject:@"O2Context_builtin_gdi"];
    [possibleContextClasses addObject:@"O2Context_cairo"];
    [possibleContextClasses addObject:@"O2Context_builtin_FT"];
    
    NSArray *allPaths=[[NSBundle bundleForClass:self] pathsForResourcesOfType:@"cgContext" inDirectory:nil];
    int      i,count=[allPaths count];
    
    for(i=0;i<count;i++){
     NSString *path=[allPaths objectAtIndex:i];
     NSBundle *check=[NSBundle bundleWithPath:path];
     Class     cls=[check principalClass];
     
     if(cls!=Nil)
      [possibleContextClasses addObject:NSStringFromClass([check principalClass])];
    }
   }
}

+(NSArray *)allContextClasses {
   NSMutableArray *result=[NSMutableArray array];
   int             i,count=[possibleContextClasses count];
   
   for(i=0;i<count;i++){
    Class check=NSClassFromString([possibleContextClasses objectAtIndex:i]);
    
    if(check!=Nil)
     [result addObject:check];
   }

	return result;
}

+(O2Context *)createContextWithSize:(O2Size)size window:(CGWindow *)window {
   NSArray *array=[self allContextClasses];
   int      count=[array count];

	while(--count>=0){
    Class check=[array objectAtIndex:count];
    if([check canInitWithWindow:window]){
     O2Context *result=[[check alloc] initWithSize:size window:window];

	if(result!=nil)
      return result;
    }
   }
   
   return nil;
}

+(O2Context *)createBackingContextWithSize:(O2Size)size context:(O2Context *)context deviceDictionary:(NSDictionary *)deviceDictionary {
   NSArray *array=[self allContextClasses];
   int      count=[array count];
   
   while(--count>=0){
    Class check=[array objectAtIndex:count];

    if([check canInitBackingWithContext:context deviceDictionary:deviceDictionary]){
     O2Context *result=[[check alloc] initWithSize:size context:context];
     if(result!=nil)
      return result;
    }
   }
   
   return nil;
}

-initWithBytes:(void *)bytes width:(size_t)width height:(size_t)height bitsPerComponent:(size_t)bitsPerComponent bytesPerRow:(size_t)bytesPerRow colorSpace:(O2ColorSpaceRef)colorSpace bitmapInfo:(O2BitmapInfo)bitmapInfo releaseCallback:(O2BitmapContextReleaseDataCallback)releaseCallback releaseInfo:(void *)releaseInfo {
   return nil;
}

+(O2Context *)createWithBytes:(void *)bytes width:(size_t)width height:(size_t)height bitsPerComponent:(size_t)bitsPerComponent bytesPerRow:(size_t)bytesPerRow colorSpace:(O2ColorSpaceRef)colorSpace bitmapInfo:(O2BitmapInfo)bitmapInfo releaseCallback:(O2BitmapContextReleaseDataCallback)releaseCallback releaseInfo:(void *)releaseInfo {
   NSArray *array=[self allContextClasses];
   int      count=[array count];
   
   while(--count>=0){
    Class check=[array objectAtIndex:count];
    
    if([check canInitBitmap]){
     O2Context *result=[[check alloc] initWithBytes:bytes width:width height:height bitsPerComponent:bitsPerComponent bytesPerRow:bytesPerRow colorSpace:colorSpace bitmapInfo:bitmapInfo releaseCallback:releaseCallback releaseInfo:releaseInfo];

     if(result!=nil)
      return result;
    }
   }
   
   return nil;
}

+(BOOL)canInitWithWindow:(CGWindow *)window {
   return NO;
}

+(BOOL)canInitBackingWithContext:(O2Context *)context deviceDictionary:(NSDictionary *)deviceDictionary {
   return NO;
}

+(BOOL)canInitBitmap {
   return NO;
}

-initWithSize:(O2Size)size window:(CGWindow *)window {
   O2InvalidAbstractInvocation();
   return nil;
}

-initWithSize:(O2Size)size context:(O2Context *)context {
   O2InvalidAbstractInvocation();
   return nil;
}

-initWithGraphicsState:(O2GState *)state {
   _userToDeviceTransform=state->_deviceSpaceTransform;
   _layerStack=[NSMutableArray new];
   _stateStack=[NSMutableArray new];
   [_stateStack addObject:state];
   _currentState=state;
   _path=[[O2MutablePath alloc] init];
   _allowsAntialiasing=YES;
   _textMatrix=O2AffineTransformIdentity;
   _showTextFunction=O2ContextDefaultShowText;
   _showGlyphsFunction=(O2ContextShowGlyphsFunction)[self methodForSelector:@selector(showGlyphs:advances:count:)];
   return self;
}

-init {
   return [self initWithGraphicsState:[[[O2GState alloc] init] autorelease]];
}

-(void)dealloc {
   [_layerStack release];
   [_stateStack release];
   [_path release];
   [super dealloc];
}

-(O2Surface *)surface {
   return nil;
}

-(O2Surface *)createSurfaceWithWidth:(size_t)width height:(size_t)height {
   return nil;
}

-(void)beginTransparencyLayerWithInfo:(NSDictionary *)unused {
}

-(void)endTransparencyLayer {
}

O2ColorRef O2ContextStrokeColor(O2ContextRef self) {
   return O2GStateStrokeColor(O2ContextCurrentGState(self));
}

O2ColorRef O2ContextFillColor(O2ContextRef self) {
   return O2GStateFillColor(O2ContextCurrentGState(self));
}

-(void)setStrokeAlpha:(O2Float)alpha {
   O2ColorRef color=O2ColorCreateCopyWithAlpha(O2ContextStrokeColor(self),alpha);
   O2ContextSetStrokeColorWithColor(self,color);
   O2ColorRelease(color);
}

-(void)setGrayStrokeColor:(O2Float)gray {
   O2Float alpha=O2ColorGetAlpha(O2ContextStrokeColor(self));
   
   O2ContextSetGrayStrokeColor(self,gray,alpha);
}

-(void)setStrokeColorRed:(O2Float)r green:(O2Float)g blue:(O2Float)b {
   O2Float alpha=O2ColorGetAlpha(O2ContextStrokeColor(self));
   O2ContextSetRGBStrokeColor(self,r,g,b,alpha);
}

-(void)setStrokeColorC:(O2Float)c m:(O2Float)m y:(O2Float)y k:(O2Float)k {
   O2Float alpha=O2ColorGetAlpha(O2ContextStrokeColor(self));
   O2ContextSetCMYKStrokeColor(self,c,m,y,k,alpha);
}

-(void)setFillAlpha:(O2Float)alpha {
   O2ColorRef color=O2ColorCreateCopyWithAlpha(O2ContextFillColor(self),alpha);
   O2ContextSetFillColorWithColor(self,color);
   O2ColorRelease(color);
}

-(void)setGrayFillColor:(O2Float)gray {
   O2Float alpha=O2ColorGetAlpha(O2ContextFillColor(self));
   O2ContextSetGrayFillColor(self,gray,alpha);
}

-(void)setFillColorRed:(O2Float)r green:(O2Float)g blue:(O2Float)b {
   O2Float alpha=O2ColorGetAlpha(O2ContextFillColor(self));
   O2ContextSetRGBFillColor(self,r,g,b,alpha);
}

-(void)setFillColorC:(O2Float)c m:(O2Float)m y:(O2Float)y k:(O2Float)k {
   O2Float alpha=O2ColorGetAlpha(O2ContextFillColor(self));
   O2ContextSetCMYKFillColor(self,c,m,y,k,alpha);
}

-(void)setAlpha:(float)alpha
{
	O2GStateSetAlpha(O2ContextCurrentGState(self), alpha);
	if ([self supportsGlobalAlpha] == NO) {
		[self setStrokeAlpha:alpha];
		[self setFillAlpha:alpha];
	}
}

-(BOOL)supportsGlobalAlpha
{
	return NO;
}

-(BOOL)isBitmapContext
{
    return YES;
}

-(void)drawPath:(O2PathDrawingMode)pathMode {
   O2InvalidAbstractInvocation();
// reset path in subclass
}

-(void)replacePathWithStrokedPath {
	O2UnimplementedFunction();
}

-(void)drawShading:(O2Shading *)shading {
   O2InvalidAbstractInvocation();
}

-(void)drawImage:(O2Image *)image inRect:(O2Rect)rect {
   O2InvalidAbstractInvocation();
}

-(void)drawLayer:(O2LayerRef)layer inRect:(O2Rect)rect {
   O2InvalidAbstractInvocation();
}
   
-(void)flush {
   // do nothing
}

-(void)synchronize {
   // do nothing
}

-(BOOL)resizeWithNewSize:(O2Size)size {
   return NO;
}

-(void)beginPage:(const O2Rect *)mediaBox {
   // do nothing
}

-(void)endPage {
   // do nothing
}

-(void)close {
   // do nothing
}

-(O2Size)size {
   return O2SizeMake(0,0);
}

-(O2ContextRef)createCompatibleContextWithSize:(O2Size)size unused:(NSDictionary *)unused {
   return [[isa alloc] initWithSize:size context:self];
}

-(BOOL)getImageableRect:(O2Rect *)rect {
   return NO;
}

// temporary

-(void)setAntialiasingQuality:(int)value {
   [O2ContextCurrentGState(self) setAntialiasingQuality:value];
}

-(void)copyBitsInRect:(O2Rect)rect toPoint:(O2Point)point gState:(int)gState {
   O2InvalidAbstractInvocation();
}

-(NSData *)captureBitmapInRect:(NSRect)rect {
   O2InvalidAbstractInvocation();
   return nil;
}

/*
  Notes: OSX generates a clip mask at fill time using the current aliasing setting. Once the fill is complete the clip mask persists with the next clip/fill. This means you can turn off AA, create a clip path, fill, turn on AA, create another clip path, fill, and edges from the first path will be aliased, and ones from the second will not. PDF does not dictate aliasing behavior, so while this is not really expected behavior, it is not a bug. 
    
 */
 
-(void)clipToState:(O2ClipState *)clipState {
   O2InvalidAbstractInvocation();
}

O2ContextRef O2ContextRetain(O2ContextRef self) {
   return (self!=NULL)?(O2ContextRef)CFRetain(self):NULL;
}

void O2ContextRelease(O2ContextRef self) {
   if(self!=NULL)
    CFRelease(self);
}

// context state
void O2ContextSetAllowsAntialiasing(O2ContextRef self,BOOL yesOrNo) {
   if(self==nil)
    return;

   self->_allowsAntialiasing=yesOrNo;
}

// layers
void O2ContextBeginTransparencyLayer(O2ContextRef self,NSDictionary *unused) {
   if(self==nil)
    return;

   [self beginTransparencyLayerWithInfo:unused];
}

void O2ContextEndTransparencyLayer(O2ContextRef self) {
   if(self==nil)
    return;

   [self endTransparencyLayer];
}

// path
BOOL O2ContextIsPathEmpty(O2ContextRef self) {
   if(self==nil)
    return YES;

   return (self->_path==nil)?YES:O2PathIsEmpty(self->_path);
}

O2Point O2ContextGetPathCurrentPoint(O2ContextRef self) {
   if(self==nil)
    return O2PointZero;

   return (self->_path==nil)?O2PointZero:O2PathGetCurrentPoint(self->_path);
}

O2Rect  O2ContextGetPathBoundingBox(O2ContextRef self) {
   if(self==nil)
    return O2RectZero;

   return (self->_path==nil)?O2RectZero:O2PathGetBoundingBox(self->_path);
}

BOOL    O2ContextPathContainsPoint(O2ContextRef self,O2Point point,O2PathDrawingMode pathMode) {
   if(self==nil)
    return NO;

   O2AffineTransform ctm=O2ContextCurrentGState(self)->_deviceSpaceTransform;

// FIX  evenOdd
   return O2PathContainsPoint(self->_path,&ctm,point,NO);
}

void O2ContextBeginPath(O2ContextRef self) {
   if(self==nil)
    return;

   O2PathReset(self->_path);
}

void O2ContextClosePath(O2ContextRef self) {
   if(self==nil)
    return;

   O2PathCloseSubpath(self->_path);
}

/* Path building is affected by the CTM, we transform them here into base coordinates (first quadrant, no transformation)

   A backend can then convert them to where it needs them, base,  current, or device space.
 */

void O2ContextMoveToPoint(O2ContextRef self,O2Float x,O2Float y) {
   if(self==nil)
    return;

   O2AffineTransform ctm=O2GStateUserSpaceTransform(O2ContextCurrentGState(self));

   O2PathMoveToPoint(self->_path,&ctm,x,y);
}

void O2ContextAddLineToPoint(O2ContextRef self,O2Float x,O2Float y) {
   if(self==nil)
    return;

   O2AffineTransform ctm=O2GStateUserSpaceTransform(O2ContextCurrentGState(self));

   O2PathAddLineToPoint(self->_path,&ctm,x,y);
}

void O2ContextAddCurveToPoint(O2ContextRef self,O2Float cx1,O2Float cy1,O2Float cx2,O2Float cy2,O2Float x,O2Float y) {
   if(self==nil)
    return;

   O2AffineTransform ctm=O2GStateUserSpaceTransform(O2ContextCurrentGState(self));

   O2PathAddCurveToPoint(self->_path,&ctm,cx1,cy1,cx2,cy2,x,y);
}

void O2ContextAddQuadCurveToPoint(O2ContextRef self,O2Float cx1,O2Float cy1,O2Float x,O2Float y) {
   if(self==nil)
    return;

   O2AffineTransform ctm=O2GStateUserSpaceTransform(O2ContextCurrentGState(self));

   O2PathAddQuadCurveToPoint(self->_path,&ctm,cx1,cy1,x,y);
}

void O2ContextAddLines(O2ContextRef self,const O2Point *points,unsigned count) {
   if(self==nil)
    return;

   O2AffineTransform ctm=O2GStateUserSpaceTransform(O2ContextCurrentGState(self));

   O2PathAddLines(self->_path,&ctm,points,count);
}

void O2ContextAddRect(O2ContextRef self,O2Rect rect) {
   if(self==nil)
    return;

   O2AffineTransform ctm=O2GStateUserSpaceTransform(O2ContextCurrentGState(self));

   O2PathAddRect(self->_path,&ctm,rect);
}

void O2ContextAddRects(O2ContextRef self,const O2Rect *rects,unsigned count) {
   if(self==nil)
    return;

   O2AffineTransform ctm=O2GStateUserSpaceTransform(O2ContextCurrentGState(self));

   O2PathAddRects(self->_path,&ctm,rects,count);
}

void O2ContextAddArc(O2ContextRef self,O2Float x,O2Float y,O2Float radius,O2Float startRadian,O2Float endRadian,BOOL clockwise) {
   if(self==nil)
    return;

   O2AffineTransform ctm=O2GStateUserSpaceTransform(O2ContextCurrentGState(self));

   O2PathAddArc(self->_path,&ctm,x,y,radius,startRadian,endRadian,clockwise);
}

void O2ContextAddArcToPoint(O2ContextRef self,O2Float x1,O2Float y1,O2Float x2,O2Float y2,O2Float radius) {
   if(self==nil)
    return;

   O2AffineTransform ctm=O2GStateUserSpaceTransform(O2ContextCurrentGState(self));

   O2PathAddArcToPoint(self->_path,&ctm,x1,y1,x2,y2,radius);
}

void O2ContextAddEllipseInRect(O2ContextRef self,O2Rect rect) {
   if(self==nil)
    return;

   O2AffineTransform ctm=O2GStateUserSpaceTransform(O2ContextCurrentGState(self));

   O2PathAddEllipseInRect(self->_path,&ctm,rect);
}

void O2ContextAddPath(O2ContextRef self,O2PathRef path) {
   if(self==nil)
    return;

   O2AffineTransform ctm=O2GStateUserSpaceTransform(O2ContextCurrentGState(self));

   O2PathAddPath(self->_path,&ctm,path);
}

void O2ContextReplacePathWithStrokedPath(O2ContextRef self) {
   if(self==nil)
    return;

	[self replacePathWithStrokedPath];
}

O2Path* O2ContextCopyPath(O2ContextRef self) {
    if(self==nil)
        return nil;
    
    return [self->_path copy];
}

// gstate

void O2ContextSaveGState(O2ContextRef self) {
   if(self==nil){
    return;
   }

   O2GState *current=O2ContextCurrentGState(self),*next;

   next=O2GStateCopyWithZone(current,NULL);
   [self->_stateStack addObject:next];
   self->_currentState=next;
   [next release];
}

void O2ContextRestoreGState(O2ContextRef self) {
   if(self==nil){
    return;
   }
    
   [self->_stateStack removeLastObject];
   self->_currentState=[self->_stateStack lastObject];

   O2GState *gState=O2ContextCurrentGState(self);

// FIXME, this could be conditional by comparing to previous font
   gState->_fontIsDirty=YES;
   gState->_fillColorIsDirty=YES;
   
   [self clipToState:O2GStateClipState(gState)];
      }
      
O2AffineTransform      O2ContextGetUserSpaceToDeviceSpaceTransform(O2ContextRef self) {
   if(self==nil)
    return O2AffineTransformIdentity;

   return O2ContextCurrentGState(self)->_deviceSpaceTransform;
}

O2AffineTransform      O2ContextGetCTM(O2ContextRef self){
   if(self==nil)
    return O2AffineTransformIdentity;

   return O2GStateUserSpaceTransform(O2ContextCurrentGState(self));
}

O2Rect                 O2ContextGetClipBoundingBox(O2ContextRef self) {
   if(self==nil)
    return O2RectZero;

   return [O2ContextCurrentGState(self) clipBoundingBox];
}

O2AffineTransform      O2ContextGetTextMatrix(O2ContextRef self) {
   if(self==nil)
    return O2AffineTransformIdentity;

   return self->_textMatrix;
}

O2InterpolationQuality O2ContextGetInterpolationQuality(O2ContextRef self) {
   if(self==nil)
    return 0;

   return [O2ContextCurrentGState(self) interpolationQuality];
}

O2Point O2ContextGetTextPosition(O2ContextRef self){
   if(self==nil)
    return O2PointZero;

   O2Point result={self->_textMatrix.tx,self->_textMatrix.ty};
   return result;
}

O2Point O2ContextConvertPointToDeviceSpace(O2ContextRef self,O2Point point) {
   if(self==nil)
    return O2PointZero;

   return [O2ContextCurrentGState(self) convertPointToDeviceSpace:point];
}

O2Point O2ContextConvertPointToUserSpace(O2ContextRef self,O2Point point) {
   if(self==nil)
    return O2PointZero;

   return [O2ContextCurrentGState(self) convertPointToUserSpace:point];
}

O2Size  O2ContextConvertSizeToDeviceSpace(O2ContextRef self,O2Size size) {
   if(self==nil)
    return O2SizeZero;

   return [O2ContextCurrentGState(self) convertSizeToDeviceSpace:size];
}

O2Size  O2ContextConvertSizeToUserSpace(O2ContextRef self,O2Size size) {
   if(self==nil)
    return O2SizeZero;

   return [O2ContextCurrentGState(self) convertSizeToUserSpace:size];
}

O2Rect  O2ContextConvertRectToDeviceSpace(O2ContextRef self,O2Rect rect) {
   if(self==nil)
    return O2RectZero;

   return [O2ContextCurrentGState(self) convertRectToDeviceSpace:rect];
}

O2Rect  O2ContextConvertRectToUserSpace(O2ContextRef self,O2Rect rect) {
   if(self==nil)
    return O2RectZero;

   return [O2ContextCurrentGState(self) convertRectToUserSpace:rect];
}

void O2ContextConcatCTM(O2ContextRef self,O2AffineTransform matrix) {
   if(self==nil)
    return;

   O2GStateConcatCTM(O2ContextCurrentGState(self),matrix);
}

void O2ContextTranslateCTM(O2ContextRef self,O2Float translatex,O2Float translatey) {
   if(self==nil)
    return;

   O2ContextConcatCTM(self,O2AffineTransformMakeTranslation(translatex,translatey));
}

void O2ContextScaleCTM(O2ContextRef self,O2Float scalex,O2Float scaley) {
   if(self==nil)
    return;

   O2ContextConcatCTM(self,O2AffineTransformMakeScale(scalex,scaley));
}

void O2ContextRotateCTM(O2ContextRef self,O2Float radians) {
   if(self==nil)
    return;

   O2ContextConcatCTM(self,O2AffineTransformMakeRotation(radians));
}

void O2ContextClip(O2ContextRef self) {
   if(self==nil)
    return;

   if(O2PathIsEmpty(self->_path))
    return;
   
   O2GState *gState=O2ContextCurrentGState(self);

   O2PathApplyTransform(self->_path,O2AffineTransformInvert(gState->_userSpaceTransform));
   O2PathApplyTransform(self->_path,gState->_deviceSpaceTransform);

   O2GStateAddClipToPath(gState,self->_path);
   
   O2PathReset(self->_path);
   
   [self clipToState:O2GStateClipState(gState)];
}

void O2ContextEOClip(O2ContextRef self) {
   if(self==nil)
    return;

   if(O2PathIsEmpty(self->_path))
    return;

   O2GState *gState=O2ContextCurrentGState(self);

   O2PathApplyTransform(self->_path,O2AffineTransformInvert(gState->_userSpaceTransform));
   O2PathApplyTransform(self->_path,gState->_deviceSpaceTransform);

   O2GStateAddEvenOddClipToPath(gState,self->_path);
   
   O2PathReset(self->_path);
   
   [self clipToState:O2GStateClipState(gState)];
}

void O2ContextClipToMask(O2ContextRef self,O2Rect rect,O2ImageRef image) {
   if(self==nil)
    return;

   O2GState *gState=O2ContextCurrentGState(self);
   
   O2GStateAddClipToMask(gState,image,rect);
   
   [self clipToState:O2GStateClipState(gState)];
}

void O2ContextClipToRect(O2ContextRef self,O2Rect rect) {
   if(self==nil)
    return;
    
   O2AffineTransform ctm=O2GStateUserSpaceTransform(O2ContextCurrentGState(self));

   O2PathReset(self->_path);
   O2PathAddRect(self->_path,&ctm,rect);
   O2ContextClip(self);
}

void O2ContextClipToRects(O2ContextRef self,const O2Rect *rects,unsigned count) {
   if(self==nil)
    return;

   O2AffineTransform ctm=O2GStateUserSpaceTransform(O2ContextCurrentGState(self));

   O2PathReset(self->_path);
   O2PathAddRects(self->_path,&ctm,rects,count);
   O2ContextClip(self);
}

void O2ContextSetStrokeColorSpace(O2ContextRef self,O2ColorSpaceRef colorSpace) {
   if(self==nil)
    return;

   int   i,length=O2ColorSpaceGetNumberOfComponents(colorSpace);
   O2Float components[length+1];
   
   for(i=0;i<length;i++)
    components[i]=0;
   components[i]=1;

   O2ColorRef color=O2ColorCreate(colorSpace,components);
   
   O2ContextSetStrokeColorWithColor(self,color);
   
   O2ColorRelease(color);
}

void O2ContextSetFillColorSpace(O2ContextRef self,O2ColorSpaceRef colorSpace) {
   if(self==nil)
    return;

   int   i,length=O2ColorSpaceGetNumberOfComponents(colorSpace);
   O2Float components[length+1];
   
   for(i=0;i<length;i++)
    components[i]=0;
   components[i]=1;

   O2ColorRef color=O2ColorCreate(colorSpace,components);

   O2ContextSetFillColorWithColor(self,color);
   
   O2ColorRelease(color);
}

void O2ContextSetStrokeColor(O2ContextRef self,const O2Float *components) {
   if(self==nil)
    return;

   O2ColorSpaceRef colorSpace=O2ColorGetColorSpace(O2ContextStrokeColor(self));
   O2ColorRef color=O2ColorCreate(colorSpace,components);
   
   O2ContextSetStrokeColorWithColor(self,color);
   
   O2ColorRelease(color);
}

void O2ContextSetStrokeColorWithColor(O2ContextRef self,O2ColorRef color) {
   if(self==nil)
    return;

   O2GStateSetStrokeColor(O2ContextCurrentGState(self),color);
}

void O2ContextSetGrayStrokeColor(O2ContextRef self,O2Float gray,O2Float alpha) {
   if(self==nil)
    return;

   O2ColorSpaceRef colorSpace=O2ColorSpaceCreateDeviceGray();
   O2Float         components[2]={gray,alpha};
   O2ColorRef color=O2ColorCreate(colorSpace,components);
   
   O2ContextSetStrokeColorWithColor(self,color);
   
   O2ColorRelease(color);
   O2ColorSpaceRelease(colorSpace);
}

void O2ContextSetRGBStrokeColor(O2ContextRef self,O2Float r,O2Float g,O2Float b,O2Float alpha) {
   if(self==nil)
    return;

   O2ColorSpaceRef colorSpace=O2ColorSpaceCreateDeviceRGB();
   O2Float         components[4]={r,g,b,alpha};
   O2ColorRef color=O2ColorCreate(colorSpace,components);
   
   O2ContextSetStrokeColorWithColor(self,color);
   
   O2ColorRelease(color);
   O2ColorSpaceRelease(colorSpace);
}

void O2ContextSetCMYKStrokeColor(O2ContextRef self,O2Float c,O2Float m,O2Float y,O2Float k,O2Float alpha) {
   if(self==nil)
    return;

   O2ColorSpaceRef colorSpace=O2ColorSpaceCreateDeviceCMYK();
   O2Float         components[5]={c,m,y,k,alpha};
   O2ColorRef color=O2ColorCreate(colorSpace,components);
   
   O2ContextSetStrokeColorWithColor(self,color);
   
   O2ColorRelease(color);
   O2ColorSpaceRelease(colorSpace);
}

void O2ContextSetFillColor(O2ContextRef self,const O2Float *components) {
   if(self==nil)
    return;

   O2ColorSpaceRef colorSpace=O2ColorGetColorSpace(O2ContextFillColor(self));
   O2ColorRef color=O2ColorCreate(colorSpace,components);
   
   O2ContextSetFillColorWithColor(self,color);
   
   O2ColorRelease(color);
}

void O2ContextSetFillColorWithColor(O2ContextRef self,O2ColorRef color) {
   if(self==nil)
    return;

   O2GStateSetFillColor(O2ContextCurrentGState(self),color);
}

void O2ContextSetGrayFillColor(O2ContextRef self,O2Float gray,O2Float alpha) {
   if(self==nil)
    return;

   O2ColorSpaceRef colorSpace=O2ColorSpaceCreateDeviceGray();
   O2Float         components[2]={gray,alpha};
   O2ColorRef color=O2ColorCreate(colorSpace,components);
   
   O2ContextSetFillColorWithColor(self,color);
   
   O2ColorRelease(color);
   O2ColorSpaceRelease(colorSpace);
}

void O2ContextSetRGBFillColor(O2ContextRef self,O2Float r,O2Float g,O2Float b,O2Float alpha) {
   if(self==nil)
    return;

   O2ColorSpaceRef colorSpace=O2ColorSpaceCreateDeviceRGB();
   O2Float         components[4]={r,g,b,alpha};
   O2ColorRef color=O2ColorCreate(colorSpace,components);
   
   O2ContextSetFillColorWithColor(self,color);
   
   O2ColorRelease(color);
   O2ColorSpaceRelease(colorSpace);
}

void O2ContextSetCMYKFillColor(O2ContextRef self,O2Float c,O2Float m,O2Float y,O2Float k,O2Float alpha) {
   if(self==nil)
    return;

   O2ColorSpaceRef colorSpace=O2ColorSpaceCreateDeviceCMYK();
   O2Float         components[5]={c,m,y,k,alpha};
   O2ColorRef color=O2ColorCreate(colorSpace,components);
   
   O2ContextSetFillColorWithColor(self,color);
   
   O2ColorRelease(color);
   O2ColorSpaceRelease(colorSpace);
}

void O2ContextSetAlpha(O2ContextRef self,O2Float alpha) {
   if(self==nil)
    return;

	[self setAlpha:alpha];
}

void O2ContextSetPatternPhase(O2ContextRef self,O2Size phase) {
   if(self==nil)
    return;

   O2GStateSetPatternPhase(O2ContextCurrentGState(self),phase);
}

void O2ContextSetStrokePattern(O2ContextRef self,O2PatternRef pattern,const O2Float *components) {
   if(self==nil)
    return;

   [O2ContextCurrentGState(self) setStrokePattern:pattern components:components];
}

void O2ContextSetFillPattern(O2ContextRef self,O2PatternRef pattern,const O2Float *components) {
   if(self==nil)
    return;

   [O2ContextCurrentGState(self) setFillPattern:pattern components:components];
}

void O2ContextSetTextMatrix(O2ContextRef self,O2AffineTransform matrix) {
   if(self==nil)
    return;

   self->_textMatrix=matrix;
   O2ContextCurrentGState(self)->_fontIsDirty=YES;
}

void O2ContextSetTextPosition(O2ContextRef self,O2Float x,O2Float y) {
   if(self==nil)
    return;

   self->_textMatrix.tx=x;
   self->_textMatrix.ty=y;
}

void O2ContextSetCharacterSpacing(O2ContextRef self,O2Float spacing) {
   if(self==nil)
    return;

   O2GStateSetCharacterSpacing(O2ContextCurrentGState(self),spacing);
}

void O2ContextSetTextDrawingMode(O2ContextRef self,O2TextDrawingMode textMode) {
   if(self==nil)
    return;

   [O2ContextCurrentGState(self) setTextDrawingMode:textMode];
}

void O2ContextSetFont(O2ContextRef self,O2FontRef font) {
   if(self==nil)
    return;

   O2GStateSetFont(O2ContextCurrentGState(self),font);
}

void O2ContextSetFontSize(O2ContextRef self,O2Float size) {
   if(self==nil)
    return;

   O2GStateSetFontSize(O2ContextCurrentGState(self),size);
}

void O2ContextSelectFont(O2ContextRef self,const char *name,O2Float size,O2TextEncoding encoding) {
   if(self==nil)
    return;

   [O2ContextCurrentGState(self) selectFontWithName:name size:size encoding:encoding];
}

void O2ContextSetShouldSmoothFonts(O2ContextRef self,BOOL yesOrNo) {
   if(self==nil)
    return;

   [O2ContextCurrentGState(self) setShouldSmoothFonts:yesOrNo];
}

void O2ContextSetLineWidth(O2ContextRef self,O2Float width) {
   if(self==nil)
    return;

   O2GStateSetLineWidth(O2ContextCurrentGState(self),width);
}

void O2ContextSetLineCap(O2ContextRef self,O2LineCap lineCap) {
   if(self==nil)
    return;

   O2GStateSetLineCap(O2ContextCurrentGState(self),lineCap);
}

void O2ContextSetLineJoin(O2ContextRef self,O2LineJoin lineJoin) {
    if(self==nil)
    return;

   O2GStateSetLineJoin(O2ContextCurrentGState(self),lineJoin);
}

void O2ContextSetMiterLimit(O2ContextRef self,O2Float miterLimit) {
   if(self==nil)
    return;

   O2GStateSetMiterLimit(O2ContextCurrentGState(self),miterLimit);
}

void O2ContextSetLineDash(O2ContextRef self,O2Float phase,const O2Float *lengths,unsigned count) {
   if(self==nil)
    return;

   O2GStateSetLineDash(O2ContextCurrentGState(self),phase,lengths,count);
}

void O2ContextSetRenderingIntent(O2ContextRef self,O2ColorRenderingIntent renderingIntent) {
   if(self==nil)
    return;

   [O2ContextCurrentGState(self) setRenderingIntent:renderingIntent];
}

void O2ContextSetBlendMode(O2ContextRef self,O2BlendMode blendMode) {
   if(self==nil)
    return;

   O2GStateSetBlendMode(O2ContextCurrentGState(self),blendMode);
}

void O2ContextSetFlatness(O2ContextRef self,O2Float flatness) {
   if(self==nil)
    return;

   [O2ContextCurrentGState(self) setFlatness:flatness];
}

void O2ContextSetInterpolationQuality(O2ContextRef self,O2InterpolationQuality quality) {
   if(self==nil)
    return;

   [O2ContextCurrentGState(self) setInterpolationQuality:quality];
}

void O2ContextSetShadowWithColor(O2ContextRef self,O2Size offset,O2Float blur,O2ColorRef color) {
   if(self==nil)
    return;

   [O2ContextCurrentGState(self) setShadowOffset:offset blur:blur color:color];
}

void O2ContextSetShadow(O2ContextRef self,O2Size offset,O2Float blur) {
   if(self==nil)
    return;

   [O2ContextCurrentGState(self) setShadowOffset:offset blur:blur];
}

void O2ContextSetShouldAntialias(O2ContextRef self,BOOL yesOrNo) {
   if(self==nil)
    return;

   [O2ContextCurrentGState(self) setShouldAntialias:yesOrNo];
}

// drawing
void O2ContextStrokeLineSegments(O2ContextRef self,const O2Point *points,unsigned count) {
   if(self==nil)
    return;

   int i;
   
   O2ContextBeginPath(self);
   for(i=0;i<count;i+=2){
    O2ContextMoveToPoint(self,points[i].x,points[i].y);
    O2ContextAddLineToPoint(self,points[i+1].x,points[i+1].y);
   }
   O2ContextStrokePath(self);
}

void O2ContextStrokeRect(O2ContextRef self,O2Rect rect) {
   if(self==nil)
    return;

   O2ContextBeginPath(self);
   O2ContextAddRect(self,rect);
   O2ContextStrokePath(self);
}

void O2ContextStrokeRectWithWidth(O2ContextRef self,O2Rect rect,O2Float width) {
   if(self==nil)
    return;

   O2ContextSaveGState(self);
   O2ContextSetLineWidth(self,width);
   O2ContextBeginPath(self);
   O2ContextAddRect(self,rect);
   O2ContextStrokePath(self);
   O2ContextRestoreGState(self);
}

void O2ContextStrokeEllipseInRect(O2ContextRef self,O2Rect rect) {
   if(self==nil)
    return;

   O2ContextBeginPath(self);
   O2ContextAddEllipseInRect(self,rect);
   O2ContextStrokePath(self);
}

void O2ContextFillRect(O2ContextRef self,O2Rect rect) {
   if(self==nil)
    return;

   O2ContextFillRects(self,&rect,1);
}

void O2ContextFillRects(O2ContextRef self,const O2Rect *rects,unsigned count) {
   if(self==nil)
    return;

   O2ContextBeginPath(self);
   O2ContextAddRects(self,rects,count);
   O2ContextFillPath(self);
}

void O2ContextFillEllipseInRect(O2ContextRef self,O2Rect rect) {
   if(self==nil)
    return;

   O2ContextBeginPath(self);
   O2ContextAddEllipseInRect(self,rect);
   O2ContextFillPath(self);
}

void O2ContextDrawPath(O2ContextRef self,O2PathDrawingMode pathMode) {
   if(self==nil)
    return;

   [self drawPath:pathMode];
}

void O2ContextStrokePath(O2ContextRef self) {
   if(self==nil)
    return;

   O2ContextDrawPath(self,kO2PathStroke);
}

void O2ContextFillPath(O2ContextRef self) {
   if(self==nil)
    return;

   O2ContextDrawPath(self,kO2PathFill);
}

void O2ContextEOFillPath(O2ContextRef self) {
   if(self==nil)
    return;

   O2ContextDrawPath(self,kO2PathEOFill);
}

void O2ContextClearRect(O2ContextRef self,O2Rect rect) {
   if(self==nil)
    return;

// doc.s are not clear. tests say O2ContextClearRect resets the path and does not affect gstate color
   O2ContextSaveGState(self);
   O2ContextSetBlendMode(self,kO2BlendModeCopy); // does it set the blend mode?
   O2ContextSetGrayFillColor(self,0,0);
   O2ContextFillRect(self,rect);
   O2ContextRestoreGState(self);
}

void O2ContextShowGlyphs(O2ContextRef self,const O2Glyph *glyphs,unsigned count) {
   if(self==nil)
    return;

   self->_showGlyphsFunction(self,NULL,glyphs,NULL,count);
}

void O2ContextShowGlyphsAtPoint(O2ContextRef self,O2Float x,O2Float y,const O2Glyph *glyphs,unsigned count) {
   O2ContextSetTextPosition(self,x,y);
   O2ContextShowGlyphs(self,glyphs,count);
}

-(void)showGlyphs:(const O2Glyph *)glyphs advances:(const O2Size *)advances count:(unsigned)count {
#if 1
   O2InvalidAbstractInvocation();
#else
   O2AffineTransform textMatrix=O2ContextGetTextMatrix(self);
   O2Float             x=textMatrix.tx;
   O2Float             y=textMatrix.ty;
   int i;
   
   for(i=0;i<count;i++){
    [self showGlyphs:glyphs+i count:1];
    
    O2Size advance=O2SizeApplyAffineTransform(advances[i],textMatrix);
    
    x+=advance.width;
    y+=advance.height;
    O2ContextSetTextPosition(self,x,y);
   }
#endif
}

void O2ContextShowGlyphsWithAdvances(O2ContextRef self,const O2Glyph *glyphs,const O2Size *advances,unsigned count) {
   if(self==nil)
    return;

   self->_showGlyphsFunction(self,NULL,glyphs,advances,count);
}

void O2ContextDefaultShowText(O2ContextRef self,const char *text,unsigned length) {
   O2GState        *gState=O2ContextCurrentGState(self);
   O2Encoding      *encoding=O2GStateEncoding(gState);
   O2PDFCharWidths *widths=O2GStateCharWidths(gState);
   O2Glyph          glyphs[length];
   
   O2EncodingGetGlyphsForBytes(encoding,glyphs,(const uint8_t *)text,length);
   if(widths==nil && gState->_characterSpacing==0)
    self->_showGlyphsFunction(self,NULL,glyphs,NULL,length);
   else {
    O2Size advances[length];
    int    i;
    
    if(widths!=nil)
     O2PDFCharWidthsGetAdvances(widths,advances,(const uint8_t *)text,length);
    else
     O2ContextGetDefaultAdvances(self,glyphs,advances,length);
    
    for(i=0;i<length;i++){
     advances[i].width+=gState->_characterSpacing;
    }
    
    self->_showGlyphsFunction(self,NULL,glyphs,advances,length);
   }
}


void O2ContextShowText(O2ContextRef self,const char *text,unsigned length) {
   if(self==nil)
    return;

   self->_showTextFunction(self,text,length);
}

void O2ContextShowTextAtPoint(O2ContextRef self,O2Float x,O2Float y,const char *text,unsigned count) {
   if(self==nil)
    return;

   O2ContextSetTextPosition(self,x,y);
   O2ContextShowText(self,text,count);
}

void O2ContextDrawShading(O2ContextRef self,O2ShadingRef shading) {
   if(self==nil)
    return;

   [self drawShading:shading];
}

void O2ContextDrawImage(O2ContextRef self,O2Rect rect,O2ImageRef image) {
   if(self==nil)
    return;

   [self drawImage:image inRect:rect];
}

void O2ContextDrawLayerAtPoint(O2ContextRef self,O2Point point,O2LayerRef layer) {
   if(self==nil)
    return;

   O2Size size=O2LayerGetSize(layer);
   O2Rect rect={point,size};
   
   O2ContextDrawLayerInRect(self,rect,layer);
}

void O2ContextDrawLayerInRect(O2ContextRef self,O2Rect rect,O2LayerRef layer) {
   if(self==nil)
    return;

   [self drawLayer:layer inRect:rect];
}

void O2ContextDrawPDFPage(O2ContextRef self,O2PDFPageRef page) {
   if(self==nil)
    return;

// Per doc.s, these are initialized at the beginning of each page only

   O2ContextSetCharacterSpacing(self,0);
   O2ContextSetWordSpacing(self,0);
   O2ContextSetTextHorizontalScaling(self,100);
   O2ContextSetTextLeading(self,0);
   O2ContextSetTextDrawingMode(self,0);
   O2ContextSetTextRise(self,0);

   [page drawInContext:self];
   
   O2ContextSetTextLineMatrix(self,O2AffineTransformIdentity);
   O2ContextSetTextMatrix(self,O2AffineTransformIdentity);
}

void O2ContextFlush(O2ContextRef self) {
   if(self==nil)
    return;

   [self flush];
}

void O2ContextSynchronize(O2ContextRef self) {
   if(self==nil)
    return;

   [self synchronize];
}

// pagination

void O2ContextBeginPage(O2ContextRef self,const O2Rect *mediaBox) {
   if(self==nil)
    return;

   [self beginPage:mediaBox];
}

void O2ContextEndPage(O2ContextRef self) {
   if(self==nil)
    return;

   [self endPage];
}

// **PRIVATE** These are private in Apple's implementation as well as ours.

void O2ContextSetTextLineMatrix(O2ContextRef self,O2AffineTransform matrix) {
   self->_textLineMatrix=matrix;
}

O2AffineTransform O2ContextGetTextLineMatrix(O2ContextRef self) {
   return self->_textLineMatrix;
}

O2Float O2ContextGetTextLeading(O2ContextRef self) {
   return O2GStateTextLeading(O2ContextCurrentGState(self));
}

void O2ContextSetTextLeading(O2ContextRef self,O2Float value) {
   O2GStateSetTextLeading(O2ContextCurrentGState(self),value);
}

void O2ContextSetWordSpacing(O2ContextRef self,O2Float value) {
   O2GStateSetWordSpacing(O2ContextCurrentGState(self),value);
}

void O2ContextSetTextRise(O2ContextRef self,O2Float value) {
   O2GStateSetTextRise(O2ContextCurrentGState(self),value);
}

void O2ContextSetTextHorizontalScaling(O2ContextRef self,O2Float value) {
   O2GStateSetTextHorizontalScaling(O2ContextCurrentGState(self),value);
}

void O2ContextSetEncoding(O2ContextRef self,O2Encoding *value) {
   O2GStateSetFontEncoding(O2ContextCurrentGState(self),value);
}

void O2ContextSetPDFCharWidths(O2ContextRef self,O2PDFCharWidths *value) {
   [O2ContextCurrentGState(self) setPDFCharWidths:value];
}

void O2ContextSetCTM(O2ContextRef self,O2AffineTransform matrix) {
   if(self==nil)
    return;
    
   O2AffineTransform deviceTransform=self->_userToDeviceTransform;
   
   deviceTransform=O2AffineTransformConcat(matrix,deviceTransform);
   
   O2GStateSetDeviceSpaceCTM(O2ContextCurrentGState(self),deviceTransform);

   O2GStateSetUserSpaceCTM(O2ContextCurrentGState(self),matrix);
}

void O2ContextResetClip(O2ContextRef self) {
   if(self==nil)
    return;
    
   O2GState *gState=O2ContextCurrentGState(self);
   
   O2GStateResetClip(gState);
   
   [self clipToState:O2GStateClipState(gState)];
}

O2AffineTransform O2ContextGetTextRenderingMatrix(O2ContextRef self) {
   O2GState *gState=O2ContextCurrentGState(self);

   O2AffineTransform transformToDevice=gState->_deviceSpaceTransform;
   O2AffineTransform Tm=self->_textMatrix;

	return O2AffineTransformConcat(Tm,transformToDevice);
}

void O2ContextGetDefaultAdvances(O2ContextRef self,const O2Glyph *glyphs,O2Size *advances,size_t count) {
   O2GState         *gState=O2ContextCurrentGState(self);
   O2Font           *font=O2GStateFont(gState);
   int               intAdvances[count];
   O2Float           unitsPerEm=O2FontGetUnitsPerEm(font);
   O2Float           pointSize=O2GStatePointSize(gState);
   size_t            i;
   
   O2FontGetGlyphAdvances(font,glyphs,count,intAdvances);
    
    float scale = [font nativeSizeForSize:pointSize]/unitsPerEm;
    for(i=0;i<count;i++){
    advances[i].width=intAdvances[i]*scale;
    advances[i].height=0;
   }
}

void O2ContextConcatAdvancesToTextMatrix(O2ContextRef self,const O2Size *advances,size_t count){
   O2AffineTransform Tm=self->_textMatrix;
   O2Size            totalAdvance=O2SizeMake(0,0);
   size_t            i;
   
   for(i=0;i<count;i++){
    O2Size advance=O2SizeApplyAffineTransform(advances[i],Tm);
    
    totalAdvance.width+=advance.width;
    totalAdvance.height+=advance.height;
   }
      
   self->_textMatrix.tx+=totalAdvance.width;
   self->_textMatrix.ty+=totalAdvance.height;
}

O2GState *O2ContextCurrentGState(O2ContextRef self) {
   return self->_currentState;
}

// Temporary hacks

void O2ContextCopyBits(O2ContextRef self,O2Rect rect,O2Point point,int gState) {
   if(self==nil)
    return;

   [self copyBitsInRect:rect toPoint:point gState:gState];
}

bool O2ContextSupportsGlobalAlpha(O2ContextRef self)
{
	return [self supportsGlobalAlpha];
}

NSData *O2ContextCaptureBitmap(O2ContextRef self,O2Rect rect) {
   return [self captureBitmapInRect:rect];
}

bool O2ContextIsBitmapContext(O2ContextRef self)
{
	return [self isBitmapContext];
}
@end
