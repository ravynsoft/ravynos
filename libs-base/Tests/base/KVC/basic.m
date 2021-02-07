#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSDecimalNumber.h>
#import <Foundation/NSGarbageCollector.h>

typedef struct {
  int	i;
  char	c;
} myStruct;

@interface TestClass : NSObject
{
  NSString *name;
  NSDate *date;
  int num1;
  double num2;
  int num3;
  int num4;
  TestClass *child;
  myStruct s;
  bool b;
}

- (void) setNum3:(int) num;
- (int) num3;
- (void) _setNum4:(int) num;
- (int) _num4;
- (char) sc;
- (int) si;
- (myStruct) sv;
- (void) setSv: (myStruct)v;

@end

@implementation TestClass

- (id) init
{
  s.i = 1;
  s.c = 2;
  return self;
}

- (void) setNum3:(int) num
{
  num3 = num;
  if (num3 == 8) num3 = 7;
}

- (int) num3
{
  return num3;
}

- (void) _setNum4:(int) num
{
  num4 = num;
  if (num4 == 8) num4 = 7;
}

- (int) _num4
{
  return num4;
}

- (char) sc
{
  return s.c;
}

- (int) si
{
  return s.i;
}

- (myStruct) sv
{
  return s;
}

- (void) setSv: (myStruct)v
{
  s = v;
}
@end

@interface UndefinedKey : NSObject
{
  int num1;
  NSString *string;
}
@end

@implementation UndefinedKey
- (void) dealloc
{
  [string release];
  [super dealloc];
}

- (void) setValue:(id) value forUndefinedKey:(NSString *) key
{
  if ([key isEqualToString: @"Lxcke"]) {
    [string release];
    string = [value copy];
  }
}

- (id) valueForUndefinedKey:(NSString *) key
{
  if ([key isEqualToString: @"Lxcke"]) {
    return string;
  }
  return nil;
}

@end

@interface UndefinedKey2 : NSObject
{
  int num1;
  NSString *string;
}
@end

@implementation UndefinedKey2
- (void) dealloc
{
  [string release];
  [super dealloc];
}

- (void) handleTakeValue:(id) value forUnboundKey:(NSString *) key
{
  if ([key isEqualToString: @"Lxcke"]) {
    [string release];
    string = [value copy];
  }
}

- (id) handleQueryWithUnboundKey:(NSString *) key
{
  if ([key isEqualToString: @"Lxcke"]) {
    return string;
  }
  return nil;
}

@end

int main() 
{
  NSAutoreleasePool *arp = [NSAutoreleasePool new];
  NSMutableString *m = [NSMutableString string];
  TestClass *tester = [[[TestClass alloc] init] autorelease];
  NSUInteger rc;
  NSValue *v;
  myStruct s;

  [m appendString: @"testing"];

  [tester setValue:[[[TestClass alloc] init] autorelease] forKey: @"child"];
  UndefinedKey *undefinedKey = [[[UndefinedKey alloc] init] autorelease];
  UndefinedKey2 *undefinedKey2 = [[[UndefinedKey2 alloc] init] autorelease];

  NSNumber *n = [NSNumber numberWithInt:8];
  NSNumber *nb = [NSNumber numberWithBool:1];
  NSNumber *adjustedN = [NSNumber numberWithInt:7];
  NSNumber *n2 = [NSNumber numberWithDouble:87.999];

  [tester setValue: @"tester" forKey: @"name"];
  PASS([[tester valueForKey: @"name"] isEqualToString: @"tester"],
      "KVC works with strings");

  rc = [m retainCount];
  [tester setValue: m forKey: @"name"];
  PASS([tester valueForKey: @"name"] == m,
      "KVC works with mutable string");
  if (nil == [NSGarbageCollector defaultCollector])
    {
      PASS(rc + 1 == [m retainCount], "KVC retains object values");
    }

  [tester setValue:nb forKey: @"b"];
  PASS([[tester valueForKey: @"b"] isEqualToNumber:nb],
      "KVC works with bool");

  [tester setValue:n forKey: @"num1"];
  PASS([[tester valueForKey: @"num1"] isEqualToNumber:n],
      "KVC works with ints");

  [tester setValue:n2 forKey: @"num2"];
  PASS([[tester valueForKey: @"num2"] isEqualToNumber:n2],
      "KVC works with doubles");

  [tester setValue:n forKey: @"num3"];
  PASS([[tester valueForKey: @"num3"] isEqualToNumber:adjustedN],
      "KVC works with setKey:");

  [tester setValue:n forKey: @"num4"];
  PASS([[tester valueForKey: @"num4"] isEqualToNumber:adjustedN],
      "KVC works with _setKey:");

  v = [tester valueForKey: @"s"];
  s.i = 0;
  s.c = 0;
  [v getValue: &s];
  PASS(s.i == 1 && s.c == 2, "KVC valueForKey: works for a struct (direct)");

  v = [tester valueForKey: @"sv"];
  s.i = 0;
  s.c = 0;
  [v getValue: &s];
  PASS(s.i == 1 && s.c == 2, "KVC valueForKey: works for a struct (getter)");

  s.i = 3;
  s.c = 4;
  v = [NSValue valueWithBytes: &s objCType: @encode(myStruct)];
  [tester setValue: v forKey: @"s"];
  PASS([tester si] == s.i && [tester sc] == s.c,
    "KVC setValue:forKey: works for a struct (direct)");

  s.i = 5;
  s.c = 6;
  v = [NSValue valueWithBytes: &s objCType: @encode(myStruct)];
  [tester setValue: v forKey: @"sv"];
  PASS([tester si] == s.i && [tester sc] == s.c,
    "KVC setValue:forKey: works for a struct (setter)");

  [undefinedKey setValue: @"GNUstep" forKey: @"Lxcke"];
  PASS([[undefinedKey valueForKey: @"Lxcke"] isEqualToString: @"GNUstep"],
      "KVC works with undefined keys");

  [undefinedKey2 setValue: @"GNUstep" forKey: @"Lxcke"];
  PASS([[undefinedKey2 valueForKey: @"Lxcke"] isEqualToString: @"GNUstep"],
      "KVC works with undefined keys (using deprecated methods) ");

  PASS_EXCEPTION(
    [tester setValue: @"" forKey: @"nonexistent"],
    NSUndefinedKeyException,
    "KVC properly throws @\"NSUnknownKeyException\"");

  PASS_EXCEPTION(
    [tester setValue: @"" forKey: @"nonexistent"],
    NSUndefinedKeyException,
    "KVC properly throws NSUndefinedKeyException");

  PASS_EXCEPTION(
    [tester setValue: @"" forKeyPath: @"child.nonexistent"],
    @"NSUnknownKeyException",
    "KVC properly throws @\"NSUnknownKeyException\" with key paths");

  PASS_EXCEPTION(
    [tester setValue: @"" forKeyPath: @"child.nonexistent"],
    NSUndefinedKeyException,
    "KVC properly throws NSUndefinedKeyException with key paths");

  PASS(sel_getUid(0) == 0, "null string gives null selector");
  PASS(sel_registerName(0) == 0, "register null string gives null selector");
  PASS(strcmp(sel_getName(0) , "<null selector>") == 0, "null selector name");

  [arp release]; arp = nil;
  return 0;
}
