#import <QuartzCore/CATransition.h>

@implementation CATransition

-(NSString *)type {
   return _type;
}

-(void)setType:(NSString *)value {
   value=[value copy];
   [_type release];
   _type=value;
}

-(NSString *)subtype {
   return _subtype;
}

-(void)setSubtype:(NSString *)value {
   value=[value copy];
   [_subtype release];
   _subtype=value;
}

-(float)startProgress {
  return _startProgress;
}

-(void)setStartProgress:(float)value {
   _startProgress=value;
}

-(float)endProgress {
   return _endProgress;
}

-(void)setEndProgress:(float)value {
   _endProgress=value;
}

@end
