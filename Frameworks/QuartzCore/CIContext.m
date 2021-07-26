#import <QuartzCore/CIContext.h>
#import <QuartzCore/CIImage.h>
#import <QuartzCore/CIFilter.h>
#import <QuartzCore/CIColor.h>
#import <QuartzCore/CIVector.h>
#import <AppKit/NSRaise.h>
#import <Foundation/NSNumber.h>

@interface CIImage(private)
-(CIFilter *)filter;
-(CGImageRef)CGImage;
@end

@implementation CIContext

-initWithCGContext:(CGContextRef)cgContext options:(NSDictionary *)options {
   _cgContext=CGContextRetain(cgContext);
   return self;
}

-(void)dealloc {
   CGContextRelease(_cgContext);
   [super dealloc];
}

+(CIContext *)contextWithCGContext:(CGContextRef)cgContext options:(NSDictionary *)options {
   return [[[self alloc] initWithCGContext:cgContext options:options] autorelease];
}

-(void)drawImage:(CIImage *)image atPoint:(CGPoint)atPoint fromRect:(CGRect)fromRect {
   CGRect inRect={{atPoint.x,atPoint.y},{fromRect.size.width,fromRect.size.height}};
   
   [self drawImage:image inRect:inRect fromRect:fromRect];
}

typedef struct {
 CGFloat _C0[4];
 CGFloat _C1[4];
} CIGradientColors;

static void evaluate(void *info,const float *in, float *output) {
   float         x=in[0];
   CIGradientColors *colors=info;
   int           i;
   
    for(i=0;i<4;i++)
     output[i]=colors->_C0[i]+x*(colors->_C1[i]-colors->_C0[i]);
}

-(void)drawImage:(CIImage *)image inRect:(CGRect)inRect fromRect:(CGRect)fromRect {
   CIFilter *filter=[image filter];
   NSString *filterName=[filter valueForKey:@"kCIAttributeFilterName"];
   
   if([filterName isEqualToString:@"CILinearGradient"]){
    CIColor *startColor=[filter valueForKey:@"inputColor0"];
    CIColor *endColor=[filter valueForKey:@"inputColor1"];
    CIVector *startVector=[filter valueForKey:@"inputPoint0"];
    CIVector *endVector=[filter valueForKey:@"inputPoint1"];
    float         domain[2]={0,1};
    float         range[8]={0,1,0,1,0,1,0,1};
    CGFunctionCallbacks callbacks={0,evaluate,NULL};
    CIGradientColors colors;
    const CGFloat *startComponents=[startColor components];
    const CGFloat *endComponents=[endColor components];
        
    colors._C0[0]=startComponents[0];
    colors._C0[1]=startComponents[1];
    colors._C0[2]=startComponents[2];
    colors._C0[3]=startComponents[3];
    colors._C1[0]=endComponents[0];
    colors._C1[1]=endComponents[1];
    colors._C1[2]=endComponents[2];
    colors._C1[3]=endComponents[3];
    
    CGFunctionRef function=CGFunctionCreate(&colors,1,domain,4,range,&callbacks);
    CGColorSpaceRef colorSpace=CGColorSpaceCreateDeviceRGB();
    CGShadingRef  shading=CGShadingCreateAxial(colorSpace,CGPointMake([startVector X],[startVector Y]),CGPointMake([endVector X],[endVector Y]),function,NO,NO);
    
    CGContextSaveGState(_cgContext);
    CGContextTranslateCTM(_cgContext,inRect.origin.x,inRect.origin.y);
    CGContextDrawShading(_cgContext,shading);
    CGContextRestoreGState(_cgContext);

    CGFunctionRelease(function);
    CGColorSpaceRelease(colorSpace);
    CGShadingRelease(shading);
   }
   
   if([filterName isEqualToString:@"CIBoxBlur"]){
    CIImage   *inputImage=[filter valueForKey:@"inputImage"];
    NSNumber  *inputRadius=[filter valueForKey:@"inputRadius"];
    CGImageRef cgImage=[inputImage CGImage];
// FIXME: implement

    if(cgImage!=NULL)
     CGContextDrawImage(_cgContext,inRect,cgImage);
   }
   
}

@end
