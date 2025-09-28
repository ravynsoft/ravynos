#import <QuartzCore/CAMediaTimingFunction.h>
#import <Foundation/NSString.h>
#import <Foundation/NSRaise.h>

NSString * const kCAMediaTimingFunctionLinear=@"kCAMediaTimingFunctionLinear";
NSString * const kCAMediaTimingFunctionEaseIn=@"kCAMediaTimingFunctionEaseIn";
NSString * const kCAMediaTimingFunctionEaseOut=@"kCAMediaTimingFunctionEaseOut";
NSString * const kCAMediaTimingFunctionEaseInEaseOut=@"kCAMediaTimingFunctionEaseInEaseOut";
NSString * const kCAMediaTimingFunctionDefault=@"kCAMediaTimingFunctionDefault";

@implementation CAMediaTimingFunction

-initWithControlPoints:(float)c1x :(float)c1y :(float)c2x :(float)c2y {
   _c1x=c1x;
   _c1y=c1y;
   _c2x=c2x;
   _c2y=c2y;
   return self;
}

+functionWithControlPoints:(float)c1x :(float)c1y :(float)c2x :(float)c2y {
   return [[[self alloc] initWithControlPoints:c1x:c1y:c2x:c2y] autorelease];
}

+functionWithName:(NSString *)name {
   if([name isEqualToString:kCAMediaTimingFunctionLinear])
    return [self functionWithControlPoints:0:0:1:1];
   if([name isEqualToString:kCAMediaTimingFunctionEaseIn])
    return [self functionWithControlPoints:0.5:0:1:1];
   if([name isEqualToString:kCAMediaTimingFunctionEaseOut])
    return [self functionWithControlPoints:0:0:0.5:1];
   if([name isEqualToString:kCAMediaTimingFunctionEaseInEaseOut])
    return [self functionWithControlPoints:0.5:0:0.5:1];
   if([name isEqualToString:kCAMediaTimingFunctionDefault])
    return [self functionWithControlPoints:0.25:0.1:0.25:1];
    
   return nil;
}

-(void)getControlPointAtIndex:(size_t)index values:(float[2])ptr {

   switch(index){
   
    default:
    case 0:
     ptr[0]=0;
     ptr[1]=0;
     break;
     
    case 1:
     ptr[0]=_c1x;
     ptr[1]=_c1y;
     break;

    case 2:
     ptr[0]=_c2x;
     ptr[1]=_c2y;
     break;

    case 3:
     ptr[0]=1;
     ptr[1]=1;
     break;
     
   }
}

@end
