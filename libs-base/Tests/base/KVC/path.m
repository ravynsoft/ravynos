#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSDecimalNumber.h>
#import <Foundation/NSDictionary.h>

@interface Tester : NSObject
{
  int num1;
  double num2;
  id child;
  NSMutableDictionary * dict;
}

@end

@implementation Tester
@end

@interface CustomKVC : NSObject
{
  int num1;
  NSString * string;
}
@end

@implementation CustomKVC
- (void)dealloc
{
  [string release];
  [super dealloc];
}

- (void)setValue:(id)value forKey:(NSString *)key
{
  if ([key isEqualToString:@"num1"]) {
    num1 = [value intValue];
  } else if ([key isEqualToString:@"Lücke"]) {
    [string release];
    string = [value copy];
  }
}

- (id)valueForKey:(NSString *)key
{
  if ([key isEqualToString:@"num1"]) {
    return [NSNumber numberWithInt:num1];
  } else if ([key isEqualToString:@"Lücke"]) {
    return string;
  }
  return nil;
}

@end

@interface DeprecatedCustomKVC : NSObject
{
  NSMutableDictionary * storage;
}

- (id)init;
- (id)valueForKey:(NSString *)key;
- (void)takeValue:(id)value forKey:(NSString *)key;
@end

@implementation DeprecatedCustomKVC
- (id)init
{
  self = [super init];
  storage = [[NSMutableDictionary alloc] init];

  return self;
}

- (id)valueForKey:(NSString *)key
{
  if ([key isEqualToString:@"dict"]) {
    return storage;
  }
  return nil;
}

- (void)takeValue:(id)value forKey:(NSString *)key
{
  if ([key isEqualToString:@"dict"]) {
    [storage release];
    storage = [value retain];
  }
}

@end

int main(void) {
  NSAutoreleasePool * arp = [NSAutoreleasePool new];

  NSString * string = @"GNUstep";

  Tester * tester = [[[Tester alloc] init] autorelease];
  [tester setValue:[NSMutableDictionary dictionary] forKey:@"dict"];
  [tester setValue:[[[Tester alloc] init] autorelease] forKey:@"child"];
  [tester setValue:[[CustomKVC new] autorelease]
        forKeyPath:@"child.child"];
  DeprecatedCustomKVC * deprecated = [[[DeprecatedCustomKVC alloc] init]
    autorelease];
  NSNumber * n = [NSNumber numberWithInt:8];
  NSNumber * n2 = [NSNumber numberWithDouble:87.999];

  [tester setValue:n2 forKeyPath:@"child.num2"];
  PASS([[tester valueForKeyPath:@"child.num2"] isEqualToNumber:n2],
      "KVC works with simple paths");

  [deprecated takeValue:[NSDictionary dictionaryWithObject:@"test"
                          forKey:@"key"]
                forKey:@"dict"];

  PASS_RUNS(
      [tester setValue:n forKeyPath:@"child.child.num1"],
      "KVC appears to work with key path");
  PASS([[tester valueForKeyPath:@"child.child.num1"] isEqualToNumber:n],
      "KVC works with key paths");

  NSLog(@"tester.child.child = %@", [tester valueForKeyPath:
      @"child.child"]);
  PASS_RUNS(
      [tester setValue:string forKeyPath:@"child.child.Lücke"],
      "KVC appears to work with a unicode key path");
  PASS([[tester valueForKeyPath:@"child.child.Lücke"] isEqualToString:string],
      "KVC works with unicode path");

  PASS_RUNS(
      [tester setValue:string forKeyPath:@"dict.Lücke"],
      "KVC appears to work with a unicode key path (test2)");
  PASS([[tester valueForKeyPath:@"dict.Lücke"] isEqualToString:string],
      "KVC works with unicode path (test2)");

  [arp release];
  return 0;
}
