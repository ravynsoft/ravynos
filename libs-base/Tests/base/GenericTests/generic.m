#import "generic.h"

@interface NSObject (PretendToBeNSString)
- (NSUInteger)length;
@end

@implementation NSObject(TestAdditions)
-(BOOL)testEquals:(id)anObject
{
  return ([self isEqual:anObject] && [anObject isEqual:self]);
}
- (NSUInteger) length
{
  return 0;
}
-(BOOL)testForString
{
  return ([self isKindOfClass:[NSString class]] && [self length]);
}
@end
