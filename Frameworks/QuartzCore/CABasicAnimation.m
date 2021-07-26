#import <QuartzCore/CABasicAnimation.h>

@implementation CABasicAnimation

-fromValue {
   return _fromValue;
}

-(void)setFromValue:value {
   value=[value retain];
   [_fromValue release];
   _fromValue=value;
}

-toValue {
   return _toValue;
}

-(void)setToValue:value {
   value=[value retain];
   [_toValue release];
   _toValue=value;
}

-byValue {
   return _byValue;
}

-(void)setByValue:value {
   value=[value retain];
   [_byValue release];
   _byValue=value;
}

@end
