#import <QuartzCore/CAValueFunction.h>
#import <Foundation/NSString.h>

NSString * const kCAValueFunctionTranslate=@"kCAValueFunctionTranslate";
NSString * const kCAValueFunctionTranslateX=@"kCAValueFunctionTranslateX";
NSString * const kCAValueFunctionTranslateY=@"kCAValueFunctionTranslateY";
NSString * const kCAValueFunctionTranslateZ=@"kCAValueFunctionTranslateZ";

NSString * const kCAValueFunctionScale=@"kCAValueFunctionScale";
NSString * const kCAValueFunctionScaleX=@"kCAValueFunctionScaleX";
NSString * const kCAValueFunctionScaleY=@"kCAValueFunctionScaleY";
NSString * const kCAValueFunctionScaleZ=@"kCAValueFunctionScaleZ";
 
NSString * const kCAValueFunctionRotateX=@"kCAValueFunctionRotateX";
NSString * const kCAValueFunctionRotateY=@"kCAValueFunctionRotateY";
NSString * const kCAValueFunctionRotateZ=@"kCAValueFunctionRotateZ";

@implementation CAValueFunction

-initWithName:(NSString *)name {
   _name=[name copy];
   return self;
}

-(void)dealloc {
   [_name release];
   [super dealloc];
}

+functionWithName:(NSString *)name {
   return [[[self alloc] initWithName:name] autorelease];
}

-(NSString *)name {
   return _name;
}

@end
