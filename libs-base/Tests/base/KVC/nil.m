#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSKeyValueCoding.h>

@interface DefaultNil : NSObject
{
  int num;
}
@end

@implementation DefaultNil
- (id)init
{
  num = 7;
  return self;
}
@end

@interface DeprecatedNil : DefaultNil
- (void)unableToSetNilForKey:(NSString *)key;
@end

@implementation DeprecatedNil
- (void)unableToSetNilForKey:(NSString *)key
{
  num = 0;
}
@end

@interface SetNil : DefaultNil
- (void)setNilValueForKey:(NSString *)key;
@end

@implementation SetNil
- (void)setNilValueForKey:(NSString *)key
{
  num = 0;
}
@end


int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];

  DefaultNil * defaultNil = [DefaultNil new];
  DeprecatedNil * deprecatedNil = [DeprecatedNil new];
  SetNil * setNil = [SetNil new];

  PASS_EXCEPTION([defaultNil setValue: nil forKey: @"num"],
    NSInvalidArgumentException, "KVC handles setting nil for a scalar")

  PASS_EXCEPTION([defaultNil takeValue: nil forKey: @"num"],
    NSInvalidArgumentException,
    "KVC handles setting nil for a scalar via takeValue:")

  [setNil setValue:nil forKey: @"num"];
  PASS([[setNil valueForKey: @"num"] intValue] == 0,
    "KVC uses setNilValueForKey:")

  [deprecatedNil setValue:nil forKey: @"num"];
  PASS([[deprecatedNil valueForKey: @"num"] intValue] == 0,
    "KVC uses deprecated unableToSetNilForKey:")

  [arp release]; arp = nil;
  return 0;
}
