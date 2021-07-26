#import "NSDecimalNumberPlaceholder.h"
#import <Foundation/NSRaise.h>
#import <Foundation/NSCoder.h>

@implementation NSDecimalNumberPlaceholder


- initWithCoder:(NSCoder *)coder
{
    if ([coder allowsKeyedCoding]) {
        //unused
        //NSInteger exponent=[coder decodeIntegerForKey:@"NS.exponent"];
        //NSInteger length=[coder decodeIntegerForKey:@"NS.length"];
        //BOOL negative=[coder decodeBoolForKey:@"NS.negative"];
        //BOOL compact=[coder decodeBoolForKey:@"NS.compact"];
        //NSInteger mantissaByteOrder=[coder decodeIntegerForKey:@"NS.mantissa.bo"]; // byte order??
        //NSUInteger byteLength=0;
        //const uint8_t *mantissa=[coder decodeBytesForKey:@"NS.mantissa" returnedLength:&byteLength];
    }

// We should warn here, but this needs to be ignored for an app, maybe at least construct a float?
//   NSUnimplementedMethod();
    [self dealloc];
    return (NSDecimalNumberPlaceholder *)[[NSNumber alloc] initWithInteger:0]; // NSNumber is implemented
}


@end
