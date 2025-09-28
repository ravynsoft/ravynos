#import <Foundation/CFUID.h>

@implementation CFUID

-initWithUnsignedLongLong:(unsigned long long)value {
   _value=value;
   return self;
}

-(unsigned long long)unsignedLongLongValue {
   return _value;
}

-(NSInteger)integerValue {
   return _value;
}

@end
