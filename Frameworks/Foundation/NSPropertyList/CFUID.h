#import <Foundation/Foundation.h>

@interface CFUID : NSObject {
    unsigned long long _value;
}

- initWithUnsignedLongLong:(unsigned long long)value;

- (unsigned long long)unsignedLongLongValue;
- (NSInteger)integerValue;

@end
