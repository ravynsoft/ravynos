#import "ObjectTesting.h"
#import <Foundation/Foundation.h>

@interface ArrayIVar : NSObject
{
  NSArray *_testArray;
}
- (void) setTestArray: (NSArray*)array;
- (NSArray*) testArray;
@end

@implementation ArrayIVar
- (void) setTestArray: (NSArray*)array
{
  [array retain];
  [_testArray release];
  _testArray = array;
}
- (NSArray*) testArray
{
  return _testArray;
}
@end

int main(int argc,char **argv)
{
  NSAutoreleasePool *pool = [NSAutoreleasePool new];
  volatile BOOL result = NO;
  NSDictionary  *root;
  NSArray       *array;
  NSArray       *ivar;
  ArrayIVar     *aiv;
  NSString      *plist;

  array = [@"({value=10;},{value=12;})" propertyList];
  plist = @"{displayGroup={allObjects=({detailArray=({value=4;},{value=2;});},{detailArray=({value=8;},{value=10;});});};}";
  root = [plist propertyList];

  result = [[array valueForKeyPath:@"@sum.value"] intValue] == 22;
  PASS(result, "-[NSArray valueForKeyPath: @\"@sum.value\"]");

  result = [[array valueForKeyPath:@"@count.value"] intValue] == 2;
  PASS(result, "-[NSArray valueForKeyPath: @\"@count.value\"]");

  result = [[array valueForKeyPath:@"@count"] intValue] == 2;
  PASS(result, "-[NSArray valueForKeyPath: @\"@count\"]");

  aiv = [ArrayIVar new];
  ivar = [NSArray arrayWithObjects: @"Joe", @"Foo", @"Bar", @"Cat", nil];
  [aiv setTestArray: ivar];

  PASS([aiv valueForKeyPath: @"testArray.@count"]
    == [ivar valueForKey: @"@count"], "valueForKey: matches valueForKeypath:");

  /* Advanced KVC */

  result = [[root valueForKeyPath:@"displayGroup.allObjects.@sum.detailArray.@avg.value"] intValue] == 12;
  PASS(result, "-[NSArray valueForKeyPath: @\"displayGroup.allObjects.@sum.detailArray.@avg.value\"]");

  result = [[root valueForKeyPath:@"displayGroup.allObjects.@sum.detailArray.@count.value"] intValue] == 4;
  PASS(result, "-[NSArray valueForKeyPath: @\"displayGroup.allObjects.@sum.detailArray.@count.value\"]");

  result = [[root valueForKeyPath:@"displayGroup.allObjects.@sum.detailArray.@count"] intValue] == 4;
  PASS(result, "-[NSArray valueForKeyPath: @\"displayGroup.allObjects.@sum.detailArray.@count\"]");

  [pool release];
  return (0);
}
