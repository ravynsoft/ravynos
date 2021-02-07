#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSValueTransformer.h>
#import <Foundation/NSValue.h>

@interface YesTransformer : NSValueTransformer
@end

@implementation YesTransformer

+ (BOOL) allowsReverseTransformation
{
  return NO;
}

+ (Class) transformedValueClass
{
  return [NSNumber class];
}

- (id) transformedValue: (id)value
{
  return [NSNumber numberWithBool: YES];
}

@end

@interface NestedTransformer : NSValueTransformer
{
  NSValueTransformer *_backingTransformer;
}
@end

@implementation NestedTransformer

- (id) init
{
  if (self = [super init]) {
    _backingTransformer = RETAIN([NSValueTransformer valueTransformerForName:
      NSStringFromClass([YesTransformer class])]);
  }

  return self;
}

- (void) dealloc
{
  RELEASE(_backingTransformer);
  DEALLOC
}

+ (BOOL) allowsReverseTransformation
{
  return NO;
}

+ (Class) transformedValueClass
{
  return [YesTransformer transformedValueClass];
}

- (id) transformedValue: (id)value
{
  return [_backingTransformer transformedValue: value];
}

@end

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSValueTransformer  *transformer;
  
  transformer = [NSValueTransformer valueTransformerForName:NSNegateBooleanTransformerName];
  PASS([[transformer transformedValue:[NSNumber numberWithBool:NO]] boolValue] == YES
    && [[transformer transformedValue:[NSNumber numberWithBool:YES]] boolValue] == NO,
    "NSNegateBooleanTransformer transforms correctly");
  PASS([[transformer reverseTransformedValue:[NSNumber numberWithBool:NO]] boolValue] == YES
    && [[transformer reverseTransformedValue:[NSNumber numberWithBool:YES]] boolValue] == NO,
    "NSNegateBooleanTransformer reverse transforms correctly");
  
  transformer = [NSValueTransformer valueTransformerForName:NSIsNilTransformerName];
  PASS([[transformer transformedValue:nil] boolValue] == YES
    && [[transformer transformedValue:@""] boolValue] == NO,
    "NSIsNilTransformer transforms correctly");
  
  transformer = [NSValueTransformer valueTransformerForName:NSIsNotNilTransformerName];
  PASS([[transformer transformedValue:@""] boolValue] == YES
    && [[transformer transformedValue:nil] boolValue] == NO,
    "NSIsNotNilTransformer transforms correctly");
  
  transformer = [NSValueTransformer valueTransformerForName:NSStringFromClass([NestedTransformer class])];
  PASS([[transformer transformedValue:nil] boolValue] == YES
    && [[transformer transformedValue:@""] boolValue] == YES,
    "Custom transformer transforms correctly");

  [arp release]; arp = nil;
  return 0;
}
