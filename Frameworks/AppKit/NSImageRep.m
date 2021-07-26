/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSImageRep.h>
#import <AppKit/NSBitmapImageRep.h>
#import <AppKit/NSPDFImageRep.h>
#import <AppKit/NSPasteboard.h>
#import <AppKit/NSGraphicsContextFunctions.h>
#import <AppKit/NSRaise.h>

@implementation NSImageRep

static NSMutableArray *_registeredClasses=nil;

+(void)initialize {
   if(self==[NSImageRep class]){
    _registeredClasses=[NSMutableArray new];
    [_registeredClasses addObject:[NSBitmapImageRep class]];
/*
 * // PDF is not ready for primetime
 * [_registeredClasses addObject:[NSPDFImageRep class]];
 */
	   
   }
}

+(NSArray *)registeredImageRepClasses {
   return _registeredClasses;
}

+(void)registerImageRepClass:(Class)class {
   [_registeredClasses addObject:class];
}

+(void)unregisterImageRepClass:(Class)class {
   [_registeredClasses removeObjectIdenticalTo:class];
}

+(NSArray *)imageFileTypes {
   NSMutableSet *result=[NSMutableSet set];
   int           i,count=[_registeredClasses count];
   
   for(i=0;i<count;i++){
    Class cls=[_registeredClasses objectAtIndex:i];
    
    [result addObjectsFromArray:[cls imageUnfilteredFileTypes]];
   }
   
   return [result allObjects];
}

+(NSArray *)imageUnfilteredFileTypes {
   return [NSArray array];
}

+(NSArray *)imagePasteboardTypes {
   NSMutableSet *result=[NSMutableSet set];
   int           i,count=[_registeredClasses count];
   
   for(i=0;i<count;i++){
    Class cls=[_registeredClasses objectAtIndex:i];
    
    [result addObjectsFromArray:[cls imageUnfilteredPasteboardTypes]];
   }
   
   return [result allObjects];
}

+(NSArray *)imageUnfilteredPasteboardTypes {
   return [NSArray array];
}

+(BOOL)canInitWithData:(NSData *)data {
   return NO;
}

+(BOOL)canInitWithPasteboard:(NSPasteboard *)pasteboard {
   NSString *available=[pasteboard availableTypeFromArray:[self imageUnfilteredPasteboardTypes]];
   
   return (available!=nil)?YES:NO;
}

+(Class)imageRepClassForData:(NSData *)data {
   int count=[_registeredClasses count];
   
   while(--count>=0){
    Class check=[_registeredClasses objectAtIndex:count];
    
    if([check canInitWithData:data])
     return check;
   }
   
   return nil;
}

+(Class)imageRepClassForFileType:(NSString *)type {
   int count=[_registeredClasses count];
   
   while(--count>=0){
    Class    checkClass=[_registeredClasses objectAtIndex:count];
    NSArray *types=[checkClass imageUnfilteredFileTypes];
    
    for(NSString *checkType in types)
     if([checkType caseInsensitiveCompare:type]==NSOrderedSame)
      return checkClass;
   }
   
   return nil;
}

+(Class)imageRepClassForPasteboardType:(NSString *)type {
   int count=[_registeredClasses count];
   
   while(--count>=0){
    Class    checkClass=[_registeredClasses objectAtIndex:count];
    NSArray *types=[checkClass imageUnfilteredPasteboardTypes];
    
    if([types containsObject:type])
     return checkClass;
   }
   
   return nil;
}

+(NSArray *)imageRepsWithContentsOfFile:(NSString *)path {
	// Try to guess which class to use from the path extension
   NSString *type=[path pathExtension];
   Class     class=[self imageRepClassForFileType:type];
	if (class == nil) {
		// Else use the content of the file to guess the class to use
		NSData *data=[NSData dataWithContentsOfFile:path];
		
		if(data==nil) {
			return nil;
		}
		
		if(self==[NSImageRep class]){    
			class=[self imageRepClassForData:data];
		}
		return [class imageRepsWithData:data];
	}
   return [class imageRepsWithContentsOfFile:path];
}

+(NSArray *)imageRepsWithContentsOfURL:(NSURL *)url {
   Class   class=self;
   NSData *data=[NSData dataWithContentsOfURL:url];
   
   if(data==nil)
    return nil;

   if(self==[NSImageRep class]){    
    if((class=[self imageRepClassForData:data])==nil)
     return nil;
   }
   
   return [class imageRepsWithData:data];
}

+(NSArray *)imageRepsWithPasteboard:(NSPasteboard *)pasteboard {
   NSArray *imageTypes=[self imagePasteboardTypes];
   NSArray *pbTypes=[pasteboard types];
   
   for(NSString *check in pbTypes){
    if([imageTypes containsObject:check]){
     NSData *data=[pasteboard dataForType:check];
     Class   class;
     
     if((class=[self imageRepClassForData:data])!=nil){
      NSArray *result;
     
      if((result=[class imageRepsWithData:data])!=nil)
       return result;
     }
    }
   }
   
   return nil;
}

+imageRepWithContentsOfFile:(NSString *)path {
   Class   class=self;
   NSData *data=[NSData dataWithContentsOfFile:path];
   
   if(data==nil)
    return nil;
    
   if(self==[NSImageRep class]){
    NSString *type=[path pathExtension];

    if((class=[self imageRepClassForFileType:type])==nil)
     return nil;
   }

   return [class imageRepWithData:data];
}

+imageRepWithContentsOfURL:(NSURL *)url {
   Class   class=self;
   NSData *data=[NSData dataWithContentsOfURL:url];
   
   if(data==nil)
    return nil;

   if(self==[NSImageRep class]){    
    if((class=[self imageRepClassForData:data])==nil)
     return nil;
   }
   
   return [class imageRepWithData:data];
}

+imageRepWithPasteboard:(NSPasteboard *)pasteboard {
   NSArray *imageTypes=[self imagePasteboardTypes];
   NSArray *pbTypes=[pasteboard types];
   
   for(NSString *check in pbTypes){
    if([imageTypes containsObject:check]){
     NSData *data=[pasteboard dataForType:check];
     Class   class;
     
     if((class=[self imageRepClassForData:data])!=nil){
      NSArray *result;
     
      if((result=[class imageRepWithData:data])!=nil)
       return result;
     }
    }
   }
   
   return nil;
}

-copyWithZone:(NSZone *)zone {
   NSImageRep *result=NSCopyObject(self,0,zone);
   result->_colorSpaceName=[_colorSpaceName copy];
   return result;
}

-(NSSize)size {
   return _size;
}

-(int)pixelsWide {
   return _pixelsWide;
}

-(int)pixelsHigh {
   return _pixelsHigh;
}

-(BOOL)isOpaque {
   return _isOpaque;
}

-(BOOL)hasAlpha {
   return _hasAlpha;
}

-(NSString *)colorSpaceName {
   return _colorSpaceName;
}

-(NSInteger)bitsPerSample {
   return _bitsPerSample;
}

-(void)setSize:(NSSize)value {
   _size=value;
}

-(void)setPixelsWide:(int)value {
   _pixelsWide=value;
}

-(void)setPixelsHigh:(int)value {
   _pixelsHigh=value;
}

-(void)setOpaque:(BOOL)value {
   _isOpaque=value;
}

-(void)setAlpha:(BOOL)value {
   _hasAlpha=value;
}

-(void)setColorSpaceName:(NSString *)value {
   value=[value copy];
   [_colorSpaceName release];
   _colorSpaceName=value;
}

-(void)setBitsPerSample:(int)value {
   _bitsPerSample=value;
}

-(BOOL)draw {
// do nothing
   return YES;
}

-(BOOL)drawAtPoint:(NSPoint)point {
   CGContextRef context=NSCurrentGraphicsPort();
   BOOL         result;
   
   CGContextSaveGState(context);
   CGContextTranslateCTM(context,point.x,point.y);
   result=[self draw];
   CGContextRestoreGState(context);
   
   return result;
}

-(BOOL)drawInRect:(NSRect)rect {
   CGContextRef context=NSCurrentGraphicsPort();
   NSSize       size=[self size];
   BOOL         result;
   
   CGContextSaveGState(context);
   CGContextTranslateCTM(context,rect.origin.x,rect.origin.y);
   CGContextScaleCTM(context,rect.size.width/size.width,rect.size.height/size.height);
   result=[self draw];
   CGContextRestoreGState(context);
   
   return result;
}

-(NSString *)description {
    return [NSString stringWithFormat:@"<%@[0x%lx] size: { %f, %f } colorSpace: %@ (%dx%d @ %d bps) alpha: %@ opaque: %@>", 
        [self class], self, _size.width, _size.height, _colorSpaceName, _pixelsWide, _pixelsHigh, _bitsPerSample,
        _hasAlpha ? @"YES" : @"NO", _isOpaque ? @"YES" : @"NO"];
}

@end
