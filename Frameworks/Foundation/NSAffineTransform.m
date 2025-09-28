/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSAffineTransform.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSKeyedUnarchiver.h>
#import <CoreFoundation/CFByteOrder.h>
#include <math.h>

@implementation NSAffineTransform

static NSAffineTransformStruct identity={1,0,0,1,0,0};

// Instead of doing start * append - this imp is doing append * start - which is quite different in matrix multiplication
static inline NSAffineTransformStruct multiplyStruct(NSAffineTransformStruct start, NSAffineTransformStruct append){
   NSAffineTransformStruct result;

   result.m11=append.m11*start.m11+append.m12*start.m21;
   result.m12=append.m11*start.m12+append.m12*start.m22;
   result.m21=append.m21*start.m11+append.m22*start.m21;
   result.m22=append.m21*start.m12+append.m22*start.m22;
   result.tX=append.tX*start.m11+append.tY*start.m21+start.tX;
   result.tY=append.tX*start.m12+append.tY*start.m22+start.tY;

   return result;
}

static inline NSAffineTransformStruct invertStruct(NSAffineTransformStruct matrix){
   NSAffineTransformStruct result;
   CGFloat determinant;

   determinant=matrix.m11*matrix.m22-matrix.m21*matrix.m12;
   if(determinant == 0.)
       [NSException raise:NSGenericException format:@"NSAffineTransform: Transform has no inverse"];

   result.m11=matrix.m22/determinant;
   result.m12=-matrix.m12/determinant;
   result.m21=-matrix.m21/determinant;
   result.m22=matrix.m11/determinant;
   result.tX=(-matrix.m22*matrix.tX+matrix.m21*matrix.tY)/determinant;
   result.tY=(matrix.m12*matrix.tX-matrix.m11*matrix.tY)/determinant;

   return result;
}

-init {
   _matrix=identity;
   return self;
}

-(void)encodeWithCoder:(NSCoder *)coder {
   	if ([coder allowsKeyedCoding]) {
		NSKeyedArchiver *keyed=(NSKeyedArchiver *)coder;
		CFSwappedFloat32 *words = alloca(sizeof(CFSwappedFloat32) * 6);
		words[0] = CFConvertFloat32HostToSwapped(_matrix.m11);
		words[1] = CFConvertFloat32HostToSwapped(_matrix.m12);
		words[2] = CFConvertFloat32HostToSwapped(_matrix.m21);
		words[3] = CFConvertFloat32HostToSwapped(_matrix.m22);
		words[4] = CFConvertFloat32HostToSwapped(_matrix.tX);
		words[5] = CFConvertFloat32HostToSwapped(_matrix.tY);
		[keyed encodeBytes:(void*)words length:(sizeof(CFSwappedFloat32) * 6) forKey:@"NSTransformStruct"];
	}
}

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    
    NSUInteger    length;
    const uint8_t *bytes=[keyed decodeBytesForKey:@"NSTransformStruct" returnedLength:&length];
    
    if(length!=24)
     _matrix=identity;
    else {
     CFSwappedFloat32 *words=(CFSwappedFloat32 *)bytes;
     
     _matrix.m11=CFConvertFloat32SwappedToHost(words[0]);
     _matrix.m12=CFConvertFloat32SwappedToHost(words[1]);
     _matrix.m21=CFConvertFloat32SwappedToHost(words[2]);
     _matrix.m22=CFConvertFloat32SwappedToHost(words[3]);
     _matrix.tX=CFConvertFloat32SwappedToHost(words[4]);
     _matrix.tY=CFConvertFloat32SwappedToHost(words[5]);
    }
   }
   return self;
}

-initWithTransform:(NSAffineTransform *)other {
    // Cocoa doesn't raise when 'other' is nil
   _matrix=[other transformStruct];
   return self;
}

-copyWithZone:(NSZone *)zone {
   return [[[self class] allocWithZone:zone] initWithTransform:self];
}

+(NSAffineTransform *)transform {
   return [[self new] autorelease];
}

-(NSAffineTransformStruct)transformStruct {
   return _matrix;
}

-(void)setTransformStruct:(NSAffineTransformStruct)matrix {
   _matrix=matrix;
}

-(void)invert {
   _matrix=invertStruct(_matrix);
}

-(void)appendTransform:(NSAffineTransform *)other {
    // Cocoa doesn't raise when 'other' is nil
   _matrix=multiplyStruct([other transformStruct], _matrix);
}

-(void)prependTransform:(NSAffineTransform *)other {
    // Cocoa doesn't raise when 'other' is nil
   _matrix=multiplyStruct(_matrix, [other transformStruct]);
}

-(void)translateXBy:(CGFloat)xby yBy:(CGFloat)yby {
   NSAffineTransformStruct translate={1,0,0,1,xby,yby};
   _matrix=multiplyStruct(_matrix, translate);
}

-(NSPoint)transformPoint:(NSPoint)point {
   NSPoint result;

   result.x=_matrix.m11*point.x+_matrix.m21*point.y+_matrix.tX;
   result.y=_matrix.m12*point.x+_matrix.m22*point.y+_matrix.tY;

   return result;
}

-(NSSize)transformSize:(NSSize)value {
    NSSize result;
    
    result.width  = _matrix.m11 * value.width + _matrix.m21 * value.height;
    result.height = _matrix.m12 * value.width + _matrix.m22 * value.height;
    
    return result;
}

-(void)rotateByDegrees:(CGFloat)angle
{
	[self rotateByRadians:M_PI*angle/180.0];
}

-(void)rotateByRadians:(CGFloat)radians
{
	NSAffineTransformStruct rotate={cos(radians),sin(radians),-sin(radians),cos(radians),0,0};
	_matrix=multiplyStruct(_matrix, rotate);
}

-(void)scaleBy:(CGFloat)value {
	NSAffineTransformStruct scale={value,0,0,value,0,0};
	_matrix=multiplyStruct(_matrix, scale);
}

-(void)scaleXBy:(CGFloat)xvalue yBy:(CGFloat)yvalue {
	NSAffineTransformStruct scale={xvalue,0,0,yvalue,0,0};
	_matrix=multiplyStruct(_matrix, scale);
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ 0x%p> {%g, %g, %g, %g, %g, %g}", NSStringFromClass([self class]), self, _matrix.m11, _matrix.m12, _matrix.m21, _matrix.m22, _matrix.tX, _matrix.tY];
}

@end
