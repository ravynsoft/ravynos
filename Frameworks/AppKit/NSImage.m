/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSImage.h>
#import <AppKit/NSImageRep.h>
#import <AppKit/NSBitmapImageRep.h>
#import <AppKit/NSCachedImageRep.h>
#import <AppKit/NSPDFImageRep.h>
#import <AppKit/NSEPSImageRep.h>
#import <AppKit/NSCustomImageRep.h>
#import <AppKit/NSPasteboard.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSGraphicsContextFunctions.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSRaise.h>

// Private class used so the context knows the flipped status of a locked image
// 10.4 does something like that - probably for more than just getting the flippiness - 10.6 uses some special NSSnapshotBitmapGraphicsContext
@interface NSImageCacheView : NSView {
	BOOL _flipped;
}
- (id)initWithFlipped:(BOOL)flipped;
@end
@implementation NSImageCacheView
- (id)initWithFlipped:(BOOL)flipped
{
	if ((self = [super init])) {
		_flipped = flipped;
	}
	return self;
}
-(BOOL)isFlipped
{
	return _flipped;
}
@end

@implementation NSImage

+(NSArray *)imageFileTypes {
   return [self imageUnfilteredFileTypes];
}

+(NSArray *)imageUnfilteredFileTypes {
   NSMutableArray *result=[NSMutableArray array];
   NSArray        *allClasses=[NSImageRep registeredImageRepClasses];
   int             i,count=[allClasses count];
   
   for(i=0;i<count;i++)
    [result addObjectsFromArray:[[allClasses objectAtIndex:i] imageUnfilteredFileTypes]];

   return result;
}

+(NSArray *)imagePasteboardTypes {
   return [self imageUnfilteredPasteboardTypes];
}

+(NSArray *)imageUnfilteredPasteboardTypes{
   NSMutableArray *result=[NSMutableArray array];
   NSArray        *allClasses=[NSImageRep registeredImageRepClasses];
   int             i,count=[allClasses count];
   
   for(i=0;i<count;i++)
    [result addObjectsFromArray:[[allClasses objectAtIndex:i] imageUnfilteredPasteboardTypes]];

   return result;
}

+(BOOL)canInitWithPasteboard:(NSPasteboard *)pasteboard {
   NSString *available=[pasteboard availableTypeFromArray:[self imageUnfilteredPasteboardTypes]];
   
   return (available!=nil)?YES:NO;
}

+(NSArray *)_checkBundles {
   return [NSArray arrayWithObjects:
           [NSBundle mainBundle], // Check the main bundle first according to the doc
           [NSBundle bundleForClass:self],
    nil];
}

+(NSMutableDictionary *)allImages {
   NSMutableDictionary *result=[[[NSThread currentThread] threadDictionary] objectForKey:@"__allImages"];

   if(result==nil){
    result=[NSMutableDictionary dictionary];
    [[[NSThread currentThread] threadDictionary] setObject:result forKey:@"__allImages"];
   }

   return result;
}

+imageNamed:(NSString *)name {
   NSImage *image=[[self allImages] objectForKey:name];
   if(image==nil){
    NSArray *bundles=[self _checkBundles];
    int      i,count=[bundles count];

    for(i=0;i<count;i++){
     NSBundle *bundle=[bundles objectAtIndex:i];
     NSString *path=[bundle pathForImageResource:name];

     if(path!=nil){
         image=[[[NSImage alloc] initWithContentsOfFile:path] autorelease];
         [image setName:name];
         if (image) {
             break;
         }
     }
    }
   }

  // Cocoa AppKit always returns the same shared cached image
   return image;
}

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    NSUInteger            length;
    const unsigned char  *tiff=[keyed decodeBytesForKey:@"NSTIFFRepresentation" returnedLength:&length];
    NSBitmapImageRep     *rep;
    
    if(tiff==NULL){
        [self release];
     return nil;
    }
   
    rep=[NSBitmapImageRep imageRepWithData:[NSData dataWithBytes:tiff length:length]];
    if(rep==nil){
        [self release];
     return nil;
    }
    
    _name=nil;
    _size=NSMakeSize(0,0);
    _representations=[NSMutableArray new];

    [_representations addObject:rep];
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   }
   return nil;
}

-initWithSize:(NSSize)size {
   _name=nil;
   _size=size;
   _representations=[NSMutableArray new];
   return self;
}

-init {
   return [self initWithSize:NSMakeSize(0,0)];
}

-initWithData:(NSData *)data {
	Class repClass = [NSImageRep imageRepClassForData:data];
    NSArray *reps=nil;
    
    if([repClass respondsToSelector:@selector(imageRepsWithData:)])
     reps=[repClass performSelector:@selector(imageRepsWithData:) withObject:data];
    else if([repClass respondsToSelector:@selector(imageRepWithData:)]){
     NSImageRep *rep=[repClass performSelector:@selector(imageRepWithData:) withObject:data];
     
     if(rep!=nil)
      reps=[NSArray arrayWithObject:rep];
    }

	if([reps count]==0){
       [self release];
		return nil;
	}
	
	_name=nil;
    _size=NSMakeSize(0,0);
	_representations=[NSMutableArray new];
	
	[_representations addObjectsFromArray:reps];
	
	return self;
}

-initWithContentsOfFile:(NSString *)path {
   NSArray *reps=[NSImageRep imageRepsWithContentsOfFile:path];
    
   if([reps count]==0){
       [self release];
    return nil;
   }

   _name=nil;
   _size=NSMakeSize(0,0);
   _representations=[NSMutableArray new];

   [_representations addObjectsFromArray:reps];

   return self;
}

-initWithContentsOfURL:(NSURL *)url {
   NSData *data=[NSData dataWithContentsOfURL:url];
   
   if(data==nil){
    [self release];
    return nil;
}

   return [self initWithData:data];
}

-initWithCGImage:(CGImageRef)cgImage size:(NSSize)size;
{
    if (self = [self initWithSize:size]) {
        NSBitmapImageRep *rep = [[[NSBitmapImageRep alloc] initWithCGImage:cgImage] autorelease];
        [_representations addObject:rep];
    }
    return self;
}

-initWithPasteboard:(NSPasteboard *)pasteboard {

	NSString *available=[pasteboard availableTypeFromArray:[[self class] imageUnfilteredPasteboardTypes]];
	NSData *data = [pasteboard dataForType:available];
	if (data == nil) {
		[self release];
		return nil;
	}
	return [self initWithData:data];
}

-initByReferencingFile:(NSString *)path {
   return [self initWithContentsOfFile:path];
}

-initByReferencingURL:(NSURL *)url {
	// Better than nothing
	return [self initWithContentsOfURL:url];
}

-(void)dealloc {
   [_name release];
   [_backgroundColor release];
   [_representations release];
   [super dealloc];
}

-copyWithZone:(NSZone *)zone {
   NSImage *result=NSCopyObject(self,0,zone);
   
   result->_name=[_name copy];
   result->_backgroundColor=[_backgroundColor copy];
   result->_representations=[_representations mutableCopy];
   
   return result;
}

-(NSString *)name {
   return _name;
}

-(NSSize)size {
   if(_size.width==0.0 && _size.height==0.0){
    int i,count=[_representations count];
    NSSize  largestSize=NSMakeSize(0,0);
    
    for(i=0;i<count;i++){
     NSImageRep *check=[_representations objectAtIndex:i];
     NSSize      checkSize=[check size];
    
     if(checkSize.width*checkSize.height>largestSize.width*largestSize.height)
      largestSize=checkSize;
    }

    return largestSize;
   }
   
   return _size;
}

-(NSColor *)backgroundColor {
   return _backgroundColor;
}

-(BOOL)isFlipped {
   return _isFlipped;
}

-(BOOL)isTemplate {
   return _isTemplate;
}

-(BOOL)scalesWhenResized {
   return _scalesWhenResized;
}

-(BOOL)matchesOnMultipleResolution {
   return _matchesOnMultipleResolution;
}

-(BOOL)usesEPSOnResolutionMismatch {
   return _usesEPSOnResolutionMismatch;
}

-(BOOL)prefersColorMatch {
   return _prefersColorMatch;
}

-(NSImageCacheMode)cacheMode {
   return _cacheMode;
}

-(BOOL)isCachedSeparately {
   return _isCachedSeparately;
}

-(BOOL)cacheDepthMatchesImageDepth {
   return _cacheDepthMatchesImageDepth;
}

-(BOOL)isDataRetained {
   return _isDataRetained;
}

-delegate {
   return _delegate;
}

-(BOOL)setName:(NSString *)name {
   if(_name!=nil && [[NSImage allImages] objectForKey:_name]==self)
    [[NSImage allImages] removeObjectForKey:_name];

   name=[name copy];
   [_name release];
   _name=name;

   if([[NSImage allImages] objectForKey:_name]!=nil)
    return NO;

   [[NSImage allImages] setObject:self forKey:_name];
   return YES;
}

-(void)setSize:(NSSize)size {
   _size=size;
   [self recache];
}

-(void)setBackgroundColor:(NSColor *)value {
   value=[value copy];
   [_backgroundColor release];
   _backgroundColor=value;
}

-(void)setFlipped:(BOOL)value {
   _isFlipped=value;
}

-(void)setTemplate:(BOOL)value {
   _isTemplate=value;
}

-(void)setScalesWhenResized:(BOOL)value {
   _scalesWhenResized=value;
}

-(void)setMatchesOnMultipleResolution:(BOOL)value {
   _matchesOnMultipleResolution=value;
}

-(void)setUsesEPSOnResolutionMismatch:(BOOL)value {
   _usesEPSOnResolutionMismatch=value;
}

-(void)setPrefersColorMatch:(BOOL)value {
   _prefersColorMatch=value;
}

-(void)setCacheMode:(NSImageCacheMode)value {
   _cacheMode=value;
}

-(void)setCachedSeparately:(BOOL)value {
   _isCachedSeparately=value;
}

-(void)setCacheDepthMatchesImageDepth:(BOOL)value {
   _cacheDepthMatchesImageDepth=value;
}

-(void)setDataRetained:(BOOL)value {
   _isDataRetained=value;
}

-(void)setDelegate:delegate {
   _delegate=delegate;
}

-(BOOL)isValid {
   NSUnimplementedMethod();
   return 0;
}

-(NSArray *)representations {
   return _representations;
}

-(void)addRepresentation:(NSImageRep *)representation {
   if(representation!=nil)
    [_representations addObject:representation];
}

-(void)addRepresentations:(NSArray *)array {
   int i,count=[array count];
   
   for(i=0;i<count;i++)
    [self addRepresentation:[array objectAtIndex:i]];
}

-(void)removeRepresentation:(NSImageRep *)representation {
   [_representations removeObjectIdenticalTo:representation];
}

-(NSCachedImageRep *)_cachedImageRepCreateIfNeeded {
   int count=[_representations count];

   while(--count>=0){
    NSCachedImageRep *check=[_representations objectAtIndex:count];

    if([check isKindOfClass:[NSCachedImageRep class]]){
    
     if(_cacheIsValid)
     return check;

     [_representations removeObjectAtIndex:count];
   }
   }
   
   NSCachedImageRep *cached=[[NSCachedImageRep alloc] initWithSize:[self size] depth:0 separate:_isCachedSeparately alpha:YES];
   [self addRepresentation:cached];
   [cached release];
   return cached;
}

-(NSImageRep *)_bestUncachedRepresentationForDevice:(NSDictionary *)device {
   int i,count=[_representations count];

   for(i=0;i<count;i++){
    NSImageRep *check=[_representations objectAtIndex:i];
      
    if(![check isKindOfClass:[NSCachedImageRep class]]){
     return check;
    }
   }
   
   return nil;
   
}

-(NSImageRep *)_bestUncachedFallbackCachedRepresentationForDevice:(NSDictionary *)device size:(NSSize)size {
   int i,count=[_representations count];
   NSImageRep *best=nil;

   size.width=ABS(size.width);
   size.height=ABS(size.height);
      
   for(i=0;i<count;i++){
    NSImageRep *check=[_representations objectAtIndex:i];
      
    if(![check isKindOfClass:[NSCachedImageRep class]]){
     if(best==nil)
      best=check;
     else {
      NSSize checkSize=[check size];
      NSSize bestSize=[best size];
      float  checkArea=checkSize.width*checkSize.height;
      float  bestArea=bestSize.width*bestSize.height;
      float  desiredArea=size.width*size.height;
      
      // downsampling is better than upsampling
      if(bestArea<desiredArea && checkArea>=desiredArea)
       best=check;
      // downsampling a closer image is better
      if(checkArea<bestArea && checkArea>=desiredArea)
       best=check;
      // if we have to upsample, biggest is better
      if(checkArea>bestArea && bestArea<desiredArea)
       best=check;
    }
   }
   }
   
   if(best!=nil)
    return best;
    
   for(i=0;i<count;i++){
    NSImageRep *check=[_representations objectAtIndex:i];
      
    if([check isKindOfClass:[NSCachedImageRep class]]){
     return check;
    }
   }
   
   return nil;   
}

-(NSImageRep *)bestRepresentationForDevice:(NSDictionary *)device {
   if(device==nil)    
    device=[[NSGraphicsContext currentContext] deviceDescription];
   
   if([device objectForKey:NSDeviceIsPrinter]!=nil){
    int i,count=[_representations count];
     
    for(i=0;i<count;i++){
     NSImageRep *check=[_representations objectAtIndex:i];
      
     if(![check isKindOfClass:[NSCachedImageRep class]])
      return check;
    }
   }
   
   if([device objectForKey:NSDeviceIsScreen]!=nil){
    NSImageRep      *uncached=[self _bestUncachedRepresentationForDevice:device];
    NSImageCacheMode caching=_cacheMode;
    
    if(caching==NSImageCacheDefault){
     if([uncached isKindOfClass:[NSBitmapImageRep class]])
      caching=NSImageCacheBySize;
     else if([uncached isKindOfClass:[NSPDFImageRep class]])
      caching=NSImageCacheAlways;
     else if([uncached isKindOfClass:[NSEPSImageRep class]])
      caching=NSImageCacheAlways;
     else if([uncached isKindOfClass:[NSCustomImageRep class]])
      caching=NSImageCacheAlways;
    }
     
    switch(caching){
    
     case NSImageCacheDefault:
     case NSImageCacheAlways:
      break;
     
     case NSImageCacheBySize:
      if([[uncached colorSpaceName] isEqual:[device objectForKey:NSDeviceColorSpaceName]]){
       NSSize size=[self size];
       
       if((size.width==[uncached pixelsWide]) && (size.height==[uncached pixelsHigh])){
        int deviceBPS=[[device objectForKey:NSDeviceBitsPerSample] intValue];
       
        if(deviceBPS==[uncached bitsPerSample])
         return uncached;
       }
      }
      break;
      
     case NSImageCacheNever:
      return uncached;
    }
        
    NSCachedImageRep *cached=[self _cachedImageRepCreateIfNeeded];
    
    if(!_cacheIsValid){
     [self lockFocusOnRepresentation:cached];
     NSRect rect;
     rect.origin.x=0;
     rect.origin.y=0;
     rect.size=[self size];
     
     if([self scalesWhenResized]){
      [self drawRepresentation:uncached inRect:rect];
     }
     else
      [uncached drawAtPoint:rect.origin];
      
     [self unlockFocus];
     _cacheIsValid=YES;
    }
    
    return cached;
   }
   
   return [_representations lastObject];
}

-(void)recache {
// This doesn't actually remove the cache, it just marks it as invalid
// This is important because you can change the size of a drawn image
// and it doesn't destroy the cache. It is recached next time it is drawn.
   _cacheIsValid=NO;
}

-(void)cancelIncrementalLoad {
   NSUnimplementedMethod();
}

-(NSData *)TIFFRepresentation {
   return [self TIFFRepresentationUsingCompression:NSTIFFCompressionNone factor:0.0];
}

-(NSData *)TIFFRepresentationUsingCompression:(NSTIFFCompression)compression factor:(float)factor {
   NSMutableArray *bitmaps=[NSMutableArray array];
   
	for(NSImageRep *check in _representations){
		if([check isKindOfClass:[NSBitmapImageRep class]]) {
			[bitmaps addObject:check];
		} else if([check isKindOfClass:[NSCachedImageRep class]]) {
			// We don't use the general case else we get flipped results for flipped images since lockFocusOnRepresentation is flipping and the Cache content
			// is already flipped
			NSRect r = { .origin = NSZeroPoint, .size = check.size };
			[self lockFocus];
			NSBitmapImageRep *image = [[NSBitmapImageRep alloc] initWithFocusedViewRect: r];
			[self unlockFocus];

			[bitmaps addObject:image];
			[image release];
		} else {
			NSSize size=[check size];
			NSBitmapImageRep *image=[[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:size.width pixelsHigh:size.height bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSDeviceRGBColorSpace bytesPerRow:0 bitsPerPixel:32];
			
			[self lockFocusOnRepresentation:image];
     // we should probably use -draw here but not all reps implement it, or not?
			[check draw];
			[self unlockFocus];
			
			[bitmaps addObject:image];
			[image release];
    }
    
   }
   
   return [NSBitmapImageRep TIFFRepresentationOfImageRepsInArray:bitmaps usingCompression:compression factor:factor];
}

-(void)lockFocus {
   [self lockFocusOnRepresentation:nil];
}

-(void)lockFocusOnRepresentation:(NSImageRep *)representation {
   NSGraphicsContext *context=nil;
   CGContextRef       graphicsPort;

   if(representation==nil){
// FIXME: Cocoa doesn't add the cached rep until the unlockFocus, it just creates the drawing context 
// then snaps the image during unlock and adds it
    representation=[self _cachedImageRepCreateIfNeeded];
      
      [self lockFocusOnRepresentation:representation];
      NSRect rect;
      id uncached=[self _bestUncachedRepresentationForDevice:nil];
      rect.origin.x=0;
      rect.origin.y=0;
      rect.size=[self size];
      
  //    if([self scalesWhenResized])
         [uncached drawInRect:rect];
         // drawAtPoint: is not working with NSPDFImageRep
         // Should probably ditch all the caching stuff anyway as it is deprecated
    //   else
      //   [uncached drawAtPoint:rect.origin];
         
      [self unlockFocus];
      _cacheIsValid=YES;
   }
   
   if([representation isKindOfClass:[NSCachedImageRep class]])
    context=[NSGraphicsContext graphicsContextWithWindow:[(NSCachedImageRep *)representation window]];
   else if([representation isKindOfClass:[NSBitmapImageRep class]])
    context=[NSGraphicsContext graphicsContextWithBitmapImageRep:(NSBitmapImageRep *)representation];
   
   if(context==nil){
    [NSException raise:NSInvalidArgumentException format:@"NSImageRep %@ can not be lockFocus'd"]; 
    return;
   }
   
   [NSGraphicsContext saveGraphicsState];
   [NSGraphicsContext setCurrentContext:context];

   graphicsPort=NSCurrentGraphicsPort();
   CGContextSaveGState(graphicsPort);
   CGContextClipToRect(graphicsPort,NSMakeRect(0,0,[representation size].width,[representation size].height));
   
	// Some fake view, just so the context knows if it's flipped or not
	NSView *view = [[[NSImageCacheView alloc] initWithFlipped:[self isFlipped]] autorelease];
    [[context focusStack] addObject:self];

	if([self isFlipped]){
    CGAffineTransform flip={1,0,0,-1,0,[self size].height};
    CGContextConcatCTM(graphicsPort,flip);
   }

}

-(void)unlockFocus {
	// Remove the pushed view
	[[[NSGraphicsContext currentContext] focusStack] removeLastObject];

	CGContextRef graphicsPort=NSCurrentGraphicsPort();

   CGContextRestoreGState(graphicsPort);

   [NSGraphicsContext restoreGraphicsState];
}

-(BOOL)drawRepresentation:(NSImageRep *)representation inRect:(NSRect)rect {
   NSColor *bg=[self backgroundColor];
   
   if(bg!=nil){
    [bg setFill];
   NSRectFill(rect);
   }
   
   return [representation drawInRect:rect];
}

-(void)compositeToPoint:(NSPoint)point fromRect:(NSRect)rect operation:(NSCompositingOperation)operation {
   [self compositeToPoint:point fromRect:rect operation:operation fraction:1.0];
}

-(void)compositeToPoint:(NSPoint)point fromRect:(NSRect)source operation:(NSCompositingOperation)operation fraction:(float)fraction {
   /* Compositing is a blitting operation. We simulate it using the draw operation.
   
      Compositing does not honor all aspects of the CTM, e.g. it will keep an image upright regardless of the orientation of CTM.
      To deal with that we use a negative height in a flipped coordinate system.
      There are probably other cases which are not right here.
    */
    
   NSSize size=[self size];
   NSRect rect=NSMakeRect(point.x,point.y,size.width,size.height);

   NSGraphicsContext *graphicsContext=[NSGraphicsContext currentContext];
   CGContextRef context=[graphicsContext graphicsPort];
   
   CGContextSaveGState(context);   
   if([[NSGraphicsContext currentContext] isFlipped]){
    rect.size.height=-rect.size.height;
   }
   
   [self drawInRect:rect fromRect:source operation:operation fraction:fraction];
   CGContextRestoreGState(context);   
}

-(void)compositeToPoint:(NSPoint)point operation:(NSCompositingOperation)operation {
   [self compositeToPoint:point operation:operation fraction:1.0];
}

-(void)compositeToPoint:(NSPoint)point operation:(NSCompositingOperation)operation fraction:(float)fraction {   
   [self compositeToPoint:point fromRect:NSZeroRect operation:operation fraction:1.0];
}

-(void)dissolveToPoint:(NSPoint)point fraction:(float)fraction {
   NSUnimplementedMethod();
}

-(void)dissolveToPoint:(NSPoint)point fromRect:(NSRect)rect fraction:(float)fraction {
   NSUnimplementedMethod();
}

-(void)drawAtPoint:(NSPoint)point fromRect:(NSRect)source operation:(NSCompositingOperation)operation fraction:(float)fraction {
   NSSize size=[self size];
   
   [self drawInRect:NSMakeRect(point.x,point.y,size.width,size.height) fromRect:source operation:operation fraction:fraction];
}

-(void)drawInRect:(NSRect)rect fromRect:(NSRect)source operation:(NSCompositingOperation)operation fraction:(float)fraction {

	// Keep a lid on any intermediate allocations while producing caches
	NSAutoreleasePool *pool=[NSAutoreleasePool new];
   NSImageRep        *any=[[[self _bestUncachedFallbackCachedRepresentationForDevice:nil size:rect.size] retain] autorelease];
   NSImageRep        *cachedRep=nil;
   CGContextRef       context;
	NSRect fullRect = { .origin = NSZeroPoint, .size = self.size };
	BOOL drawFullImage = (NSIsEmptyRect(source) || NSEqualRects(source, fullRect));
	BOOL canCache = drawFullImage && !_isFlipped;

	if (canCache) {
		// If we're drawing the full image unflipped then we can just draw from a cached rep or a bitmap rep (assuming we have one)
		if([any isKindOfClass:[NSCachedImageRep class]] ||
		   [any isKindOfClass:[NSBitmapImageRep class]]) {
			cachedRep=any;
		}
	}
    
	if(cachedRep==nil) {
		// Looks like we need to create a cached rep for this image
		NSImageRep       *uncached=any;
		NSSize            uncachedSize=[uncached size];
		BOOL              useSourceRect=NSIsEmptyRect(source)?NO:YES;
		NSSize            cachedSize=useSourceRect?source.size:uncachedSize;
	
		// Create a cached image rep to hold our image
		NSCachedImageRep *cached=[[[NSCachedImageRep alloc] initWithSize:cachedSize depth:0 separate:YES alpha:YES] autorelease]; // remember that pool we created earlier

		// a non-nil object passed here means we need to manually add the rep
		[self lockFocusOnRepresentation: cached];
   
		context=NSCurrentGraphicsPort();
		if(useSourceRect) {
			// move to the origin of the source rect - remember we've locked focus so we've got a fresh CTM to work with
			CGContextTranslateCTM(context,-source.origin.x,-source.origin.y);
		}
		if (_isFlipped) {
			// Flip the CTM so the image is drawn the right way up in the cache
			CGContextTranslateCTM(context, 0, uncachedSize.height);
			CGContextScaleCTM(context, 1, -1);
		}
		// Draw into the new cache rep
		[self drawRepresentation:uncached inRect:NSMakeRect(0,0,uncachedSize.width,uncachedSize.height)];

		[self unlockFocus];
    
		// And keep it if it makes sense
		if (canCache) {
			[self addRepresentation: cached];
		}
		
		cachedRep=cached;
	}
   
	// OK now we've got a rep we can draw
	
	context=NSCurrentGraphicsPort();

	CGContextSaveGState(context);
   
	if (CGContextSupportsGlobalAlpha(context) == NO) {
		// That should really be done by setting the context alpha - and the compositing done in the context implementation
		if(fraction!=1.0){
			// fraction is accomplished with a 1x1 alpha mask
			// FIXME: could use a float format image to completely preserve fraction
			uint8_t           bytes[1]={ MIN(MAX(0,fraction*255),255) };
			CGDataProviderRef provider=CGDataProviderCreateWithData(NULL,bytes,1,NULL);
			CGImageRef        mask=CGImageMaskCreate(1,1,8,8,1,provider,NULL,NO);
			
			CGContextClipToMask(context,rect,mask);
			CGImageRelease(mask);
			CGDataProviderRelease(provider);
		}
	} else {
		CGContextSetAlpha(context, fraction);
	}	
	[[NSGraphicsContext currentContext] setCompositingOperation:operation];
    
	[self drawRepresentation: cachedRep inRect:rect];
   
	CGContextRestoreGState(context);

	[pool release];
}

-(NSString *)description {
   NSSize size=[self size];
   
   return [NSString stringWithFormat:@"<%@[0x%lx] name: %@ size: { %f, %f } representations: %@>", [self class], self, _name, size.width, size.height, _representations];
}

@end

@implementation NSBundle(NSImage)

-(NSString *)pathForImageResource:(NSString *)name {
    NSString *extension = [name pathExtension];
    if (extension && extension.length) {
        NSString *baseName=[name stringByDeletingPathExtension];
        return [self pathForResource:baseName ofType:extension];
    }
   NSArray *types=[NSImage imageFileTypes];
   int      i,count=[types count];

   for(i=0;i<count;i++){
    NSString *type=[types objectAtIndex:i];
    NSString *path=[self pathForResource:name ofType:type];

    if(path!=nil)
     return path;
   }

   return [self pathForResource:[name stringByDeletingPathExtension] ofType:[name pathExtension]];
}

@end

