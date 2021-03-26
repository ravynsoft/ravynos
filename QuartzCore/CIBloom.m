#import "CIBloom.h"
#import <Foundation/NSCoder.h>
#import <Foundation/NSException.h>
#import <Foundation/NSString.h>

@implementation CIBloom

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
#if 0
// Raising keyed unarchiving exceptions, investigate
    _inputRadius=[coder decodeDoubleForKey:@"CI_inputRadius"];
    _inputIntensity=[coder decodeDoubleForKey:@"CI_inputIntensity"];
    _enabled=[coder decodeBoolForKey:@"CI_inputRadius"];
#endif
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] does not support non-keyed coding",isa,_cmd];
   }
   return self;
}

@end
