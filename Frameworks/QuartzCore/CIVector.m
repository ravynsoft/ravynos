#import <QuartzCore/CIVector.h>
#import <Foundation/NSException.h>
#import <Foundation/NSString.h>

@implementation CIVector

+(CIVector *)vectorWithValues:(const CGFloat *)values count:(size_t)count {
   return [[[self alloc] initWithValues:values count:count] autorelease];
}

+(CIVector *)vectorWithX:(CGFloat)x {
   return [[[self alloc] initWithX:x] autorelease];
}

+(CIVector *)vectorWithX:(CGFloat)x Y:(CGFloat)y {
   return [[[self alloc] initWithX:x Y:y] autorelease];
}

+(CIVector *)vectorWithX:(CGFloat)x Y:(CGFloat)y Z:(CGFloat)z {
   return [[[self alloc] initWithX:x Y:y Z:z] autorelease];
}

+(CIVector *)vectorWithX:(CGFloat)x Y:(CGFloat)y Z:(CGFloat)z W:(CGFloat)w {
   return [[[self alloc] initWithX:x Y:y Z:z W:w] autorelease];
}

-initWithValues:(const CGFloat *)values count:(size_t)count {
   _count=count;
   _values=NSZoneMalloc(NULL,sizeof(CGFloat)*count);
   
   int i;
   for(i=0;i<count;i++)
    _values[i]=values[i];
    
   return self;
}

-(void)dealloc {
   NSZoneFree(NULL,_values);
   [super dealloc];
}

-initWithX:(CGFloat)x {
   return [self initWithValues:&x count:1];
}

-initWithX:(CGFloat)x Y:(CGFloat)y {
   CGFloat values[2]={x,y};
   return [self initWithValues:values count:2];
}

-initWithX:(CGFloat)x Y:(CGFloat)y Z:(CGFloat)z {
   CGFloat values[3]={x,y,z};
   return [self initWithValues:values count:3];
}

-initWithX:(CGFloat)x Y:(CGFloat)y Z:(CGFloat)z W:(CGFloat)w {
   CGFloat values[4]={x,y,z,w};
   return [self initWithValues:values count:4];
}

-(size_t)count {
   return _count;
}

-(CGFloat)valueAtIndex:(size_t)index {
   if(index>=_count){
    [NSException raise:NSInvalidArgumentException format:@"index %d is beyond count %d",index,_count];
    return 0;
   }
   
   return _values[index];
}

-(CGFloat)X {
   return [self valueAtIndex:0];
}

-(CGFloat)Y {
   return [self valueAtIndex:1];
}

-(CGFloat)Z {
   return [self valueAtIndex:2];
}

-(CGFloat)W {
   return [self valueAtIndex:3];
}

@end
