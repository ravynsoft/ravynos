/* Copyright (c) 2008 Johannes Fortmann
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "O2Context_cairo.h"
#import <AppKit/X11Display.h>
#import <Onyx2D/O2MutablePath.h>
#import <Onyx2D/O2Color.h>
#import <Foundation/NSException.h>
#import <Onyx2D/O2GraphicsState.h>
#import <Onyx2D/O2ClipState.h>
#import <Onyx2D/O2ClipPhase.h>
#import <AppKit/KTFont_FT.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2Surface.h>
#import <Foundation/NSException.h>
#import <cairo/cairo-ft.h>
#import <cairo/cairo.h>
#import "O2FontState_cairo.h"
#import "O2Surface_cairo.h"
#import "O2Context_builtin_FT.h"

@implementation O2Context_cairo

+(BOOL)canInitWithWindow:(CGWindow *)window {
   return YES;
}

+(BOOL)canInitBackingWithContext:(O2Context *)context deviceDictionary:(NSDictionary *)deviceDictionary {
   NSString *name=[deviceDictionary objectForKey:@"O2Context"];

   if(name==nil || [name isEqual:@"cairo"])
    return YES;
    
   return NO;
}

-initWithSize:(O2Size)size window:(CGWindow *)cgWindow {
   X11Window *window=(X11Window *)cgWindow;
   O2Rect frame=[window frame];

   O2GState  *initialState=[[[O2GState alloc] initWithDeviceTransform:O2AffineTransformIdentity] autorelease];
   
   if(self=[super initWithGraphicsState:initialState]){
      Display *dpy=[(X11Display*)[NSDisplay currentDisplay] display];
      _surface = cairo_xlib_surface_create(dpy, [window drawable], [window visual], frame.size.width, frame.size.height);
     _context = cairo_create(_surface);
   }
   return self;
}

-initWithSize:(O2Size)size context:(O2Context *)context {
   O2GState  *initialState=[[[O2GState alloc] initWithDeviceTransform:O2AffineTransformIdentity] autorelease];
   
   if(self=[super initWithGraphicsState:initialState]){
      _surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size.width, size.height);
      _context = cairo_create(_surface);
   }
   return self;
}

-(void)dealloc {
   cairo_surface_destroy(_surface);
   cairo_destroy(_context);
   [super dealloc];
}

-(O2Surface *)createSurfaceWithWidth:(size_t)width height:(size_t)height {
   return [[O2Surface_cairo alloc] initWithWidth:width height:height compatibleWithContext:self];
}

-(void)setCurrentColor:(O2Color*)color
{
   const float *c=O2ColorGetComponents(color);
   int count=O2ColorGetNumberOfComponents(color);

	switch(count)
   {
      case 1:
         cairo_set_source_rgba(_context,
                            c[0],
                            c[0],
                            c[0],
                            1.0);
         break;
      case 2:
         cairo_set_source_rgba(_context,
                               c[0],
                               c[0],
                               c[0],
                               c[1]);
         break;
         
      case 3:
         cairo_set_source_rgba(_context,
                               c[0],
                               c[1],
                               c[2],
                               1.0);
         break;
      case 4:
         cairo_set_source_rgba(_context,
                               c[0],
                               c[1],
                               c[2],
                               c[3]);
         break;
      default:
         NSLog(@"color with %i components", count);
         cairo_set_source_rgba(_context,
                               1.0,
                               0.0,
                               1.0,
                               1.0);
         break;
	}   
}


-(void)appendCTM
{
	O2AffineTransform ctm=O2ContextGetCTM(self);
	cairo_matrix_t matrix={ctm.a, ctm.b, ctm.c, ctm.d, ctm.tx, ctm.ty};
   

	cairo_transform(_context,&matrix);
}

-(void)synchronizeFontCTM
{
	O2AffineTransform ctm = O2ContextGetTextMatrix(self);
    O2Float size = O2GStatePointSize(O2ContextCurrentGState(self));

	ctm = O2AffineTransformScale(ctm, size, -size);
	
	cairo_matrix_t matrix={ctm.a, ctm.b, ctm.c, ctm.d, ctm.tx, ctm.ty};

	cairo_set_font_matrix(_context, &matrix);
}


-(void)appendFlip
{
   cairo_matrix_t matrix={1, 0, 0, -1, 0, [self size].height};

	cairo_transform(_context,&matrix);
}

-(void)synchronizeLineAttributes
{
   O2GState *gState=O2ContextCurrentGState(self);
	int i;
   
	cairo_set_line_width(_context, gState->_lineWidth);
	cairo_set_line_cap(_context, gState->_lineCap);
	cairo_set_line_join(_context, gState->_lineJoin);
	cairo_set_miter_limit(_context, gState->_miterLimit);
	
	double dashLengths[gState->_dashLengthsCount];
	double totalLength=0.0;
	for(i=0; i<gState->_dashLengthsCount; i++)
	{
		dashLengths[i]=(double)gState->_dashLengths[i];
		totalLength=(double)gState->_dashLengths[i];
	}
	cairo_set_dash (_context, dashLengths, gState->_dashLengthsCount, gState->_dashPhase/totalLength);
}



-(void)setCurrentPath:(O2Path*)path
{
	unsigned             opCount=O2PathNumberOfElements(path);
	const unsigned char *operators=O2PathElements(path);
	unsigned             pointCount=O2PathNumberOfPoints(path);
	const NSPoint       *points=O2PathPoints(path);
	unsigned             i,pointIndex;
	cairo_identity_matrix(_context);
	cairo_new_path(_context);
   [self appendFlip];
	
	pointIndex=0;
	for(i=0;i<opCount;i++){
		switch(operators[i]){
            
			case kCGPathElementMoveToPoint:{
				NSPoint point=points[pointIndex++];

				cairo_move_to(_context,point.x,point.y);
			}
				break;
				
			case kCGPathElementAddLineToPoint:{
				NSPoint point=points[pointIndex++];
				
				cairo_line_to(_context,point.x,point.y);
			}
				break;
				
			case kCGPathElementAddCurveToPoint:{
				NSPoint cp1=points[pointIndex++];
				NSPoint cp2=points[pointIndex++];
				NSPoint end=points[pointIndex++];
				
				cairo_curve_to(_context,cp1.x,cp1.y,
                           cp2.x,cp2.y,
                           end.x,end.y);
			}
				break;

			case kCGPathElementAddQuadCurveToPoint:{
				NSPoint cp1=points[pointIndex++];
				NSPoint end=points[pointIndex++];
				
				cairo_curve_to(_context,cp1.x,cp1.y,
                           cp1.x,cp1.y,
                           end.x,end.y);
			}
				break;
				
			case kCGPathElementCloseSubpath:
				cairo_close_path(_context);
				break;
		}
	}
}

-(void)clipToState:(O2ClipState *)clipState {
   NSArray *phases=[O2GStateClipState(O2ContextCurrentGState(self)) clipPhases];
   int      i,count=[phases count];
   
   cairo_reset_clip(_context);  
   
   for(i=0;i<count;i++){
    O2ClipPhase *phase=[phases objectAtIndex:i];
    
    switch(O2ClipPhasePhaseType(phase)){
    
     case O2ClipPhaseNonZeroPath:;
      O2Path *path=O2ClipPhaseObject(phase);
	[self setCurrentPath:path];
	cairo_set_fill_rule(_context, CAIRO_FILL_RULE_WINDING);
	cairo_clip(_context);
      break;
      
     case O2ClipPhaseEOPath:{
       O2Path *path=O2ClipPhaseObject(phase);
// not handled
}
      break;

     case O2ClipPhaseMask:
      break;
    }

   }
}

-(void)drawPath:(O2PathDrawingMode)mode
{
	[self setCurrentPath:(O2Path*)_path];
   

	switch(mode)
	{
		case kCGPathStroke:
         [self setCurrentColor:O2ContextStrokeColor(self)];
			[self synchronizeLineAttributes];
			cairo_stroke_preserve(_context);
			break;
			
		case kCGPathFill:	
         [self setCurrentColor:O2ContextFillColor(self)];
			cairo_set_fill_rule(_context, CAIRO_FILL_RULE_WINDING);
			cairo_fill_preserve(_context);
			break;
			
		case kCGPathEOFill:
         [self setCurrentColor:O2ContextFillColor(self)];
			cairo_set_fill_rule(_context, CAIRO_FILL_RULE_EVEN_ODD);
			cairo_fill_preserve(_context);
			break;
			
			
		case kCGPathFillStroke:
         [self setCurrentColor:O2ContextFillColor(self)];
			cairo_set_fill_rule(_context, CAIRO_FILL_RULE_WINDING);
			cairo_fill_preserve(_context);
         [self setCurrentColor:O2ContextStrokeColor(self)];
			[self synchronizeLineAttributes];
			cairo_stroke_preserve(_context);
			break;
			
		case kCGPathEOFillStroke:
         [self setCurrentColor:O2ContextFillColor(self)];
			cairo_set_fill_rule(_context, CAIRO_FILL_RULE_EVEN_ODD);
			cairo_fill_preserve(_context);
         [self setCurrentColor:O2ContextStrokeColor(self)];
			[self synchronizeLineAttributes];
			cairo_stroke_preserve(_context);
			break;
	}
         
   cairo_new_path(_context);
   O2PathReset(_path);
}

-(BOOL)resizeWithNewSize:(O2Size)size {
   switch(cairo_surface_get_type(_surface)){
    case CAIRO_SURFACE_TYPE_XLIB:
  //   if(_context!=NULL)
    //  cairo_destroy(_context);
     cairo_xlib_surface_set_size(_surface, size.width, size.height);
    // _context = cairo_create(_surface);
     return YES;
         
    default:
    return NO;
   }
}

-(NSSize)size {
   switch(cairo_surface_get_type(_surface))
   {
      case CAIRO_SURFACE_TYPE_XLIB:
         return NSMakeSize(cairo_xlib_surface_get_width(_surface), cairo_xlib_surface_get_height(_surface));
      case CAIRO_SURFACE_TYPE_IMAGE:
         return NSMakeSize(cairo_image_surface_get_width(_surface), cairo_image_surface_get_height(_surface));
      default:
         return NSZeroSize;
   }
}

-(void)drawShading:(O2Shading *)shading {
   if([shading isAxial]) {
      cairo_pattern_t *pat;
      pat = cairo_pattern_create_linear (0.0, 0.0,  0.0, 256.0);

   
      
      cairo_pattern_destroy(pat);
   }
}

-(void)drawImage:(O2Image *)image inRect:(O2Rect)rect {
   cairo_surface_t *surface=NULL;
   
   if([image isKindOfClass:[O2Surface_cairo class]])
    surface=cairo_surface_reference([(O2Surface_cairo *)image cairo_surface]);
   else {
    int width=O2ImageGetWidth(image);
    int height=O2ImageGetHeight(image);
    
    surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);

    unsigned char *data=cairo_image_surface_get_data(surface);
    int            bytesPerRow=cairo_image_surface_get_stride(surface);
    int i;
    
    for(i=0; i<height; i++) {
     image->_read_argb8u(image, 0, i, (O2argb8u *)(data+i*bytesPerRow), width);
    }
   }
   
   cairo_identity_matrix(_context);
   [self appendFlip];
   [self appendCTM];
   
   cairo_new_path(_context);
   
   cairo_translate(_context, rect.origin.x, rect.origin.y);
   cairo_rectangle(_context,0, 0, rect.size.width, rect.size.height);  
   
   cairo_clip(_context);

   cairo_set_source_surface(_context,surface, 0.0, 0.0);

   cairo_paint(_context);
   
   cairo_surface_destroy(surface);
}

-(void)establishFontStateInDeviceIfDirty {
   O2GState *gState=O2ContextCurrentGState(self);
   
   if(gState->_fontIsDirty){
    O2GStateClearFontIsDirty(gState);

    O2Font_FT *cgFont=(O2Font_FT *)O2GStateFont(gState);
    KTFont *fontState=[[O2FontState_cairo alloc] initWithFreeTypeFont:cgFont size:O2GStatePointSize(gState)];
   
    [gState setFontState:fontState];
    [fontState release];
   }
}


-(void)showGlyphs:(const O2Glyph *)glyphs advances:(const O2Size *)advancesIn count:(unsigned)count {
// FIXME: use advancesIn if not NULL

   [self establishFontStateInDeviceIfDirty];
   
   O2GState          *gState=O2ContextCurrentGState(self);
   O2Font            *font=O2GStateFont(gState);
   O2FontState_cairo *fontState=[gState fontState];
   cairo_font_face_t *face=[fontState cairo_font_face];
   cairo_glyph_t     *cg=alloca(sizeof(cairo_glyph_t)*count);
   int                i,advances[count];
   O2Float            unitsPerEm=O2FontGetUnitsPerEm(font);
   
   O2FontGetGlyphAdvances(font,glyphs,count,advances);

   float x=0, y=0;
   for(i=0; i<count; i++){      
    cg[i].x=x;
    cg[i].y=y;
    cg[i].index=glyphs[i];
    x+=((CGFloat)advances[i]/(CGFloat)unitsPerEm)*gState->_pointSize;
   }
   
   cairo_set_font_face(_context, face);
   cairo_set_font_size(_context, gState->_pointSize);
   
   cairo_identity_matrix(_context);

   [self appendFlip];

   [self appendCTM];
   [self synchronizeFontCTM];
   [self setCurrentColor:O2ContextFillColor(self)];
   cairo_move_to(_context, 0, 0);
   
   cairo_show_glyphs(_context, cg, count);
   
}

-(void)flush {
   cairo_surface_flush(_surface);
}

-(cairo_surface_t *)cairo_surface {
   return _surface;
}


cairo_status_t writeToData(void		  *closure,
                           const unsigned char *data,
                           unsigned int	   length) {
   id obj=(id)closure;
   [obj appendBytes:data length:length];
   return CAIRO_STATUS_SUCCESS;
}

-(void)drawBackingContext:(O2Context *)other size:(NSSize)size {
   cairo_surface_t *otherSurface=NULL;
   
   if([other isKindOfClass:[O2Context_cairo class]])
    otherSurface=[(O2Context_cairo *)other cairo_surface];
   else if([other isKindOfClass:[O2Context_builtin_FT class]]){
    O2Surface *surface=[(O2Context_builtin_FT *)other surface];
    
    if([surface isKindOfClass:[O2Surface_cairo class]])
     otherSurface=[(O2Surface_cairo *)surface cairo_surface];
   }
   
   if(otherSurface==NULL){
    NSLog(@"unable to draw backing context %@",other);
    return;
   }
      
   if(size.width==0 || size.height==0)
      return;
   
   cairo_identity_matrix(_context);
   cairo_reset_clip(_context);

   cairo_rectangle(_context,0,0,size.width,size.height);
   cairo_set_source_surface(_context, otherSurface, 0, 0);
   
   cairo_paint(_context);
   cairo_surface_flush(_surface);
}

@end
