#import <AppKit/NSAffineTransform.h>
#import <AppKit/NSBezierPath.h>
#import <ApplicationServices/ApplicationServices.h>
#import <AppKit/NSGraphicsContext.h>

@implementation NSAffineTransform(AppKit)

-(void)concat {
   NSAffineTransformStruct atStruct=[self transformStruct];
   CGAffineTransform       cgMatrix;
   
   cgMatrix.a=atStruct.m11;
   cgMatrix.b=atStruct.m12;
   cgMatrix.c=atStruct.m21;
   cgMatrix.d=atStruct.m22;
   cgMatrix.tx=atStruct.tX;
   cgMatrix.ty=atStruct.tY;

   CGContextConcatCTM([[NSGraphicsContext currentContext] graphicsPort],cgMatrix);
}

-(void)set {
   NSAffineTransformStruct atStruct=[self transformStruct];
   CGAffineTransform       cgMatrix;
   
   cgMatrix.a=atStruct.m11;
   cgMatrix.b=atStruct.m12;
   cgMatrix.c=atStruct.m21;
   cgMatrix.d=atStruct.m22;
   cgMatrix.tx=atStruct.tX;
   cgMatrix.ty=atStruct.tY;

   CGContextSetCTM([[NSGraphicsContext currentContext] graphicsPort],cgMatrix);
}

-(NSBezierPath *)transformBezierPath:(NSBezierPath *)bezierPath {
   NSBezierPath *result=[[bezierPath copy] autorelease];
   
   [result transformUsingAffineTransform:self];
   
   return result;
}

@end
