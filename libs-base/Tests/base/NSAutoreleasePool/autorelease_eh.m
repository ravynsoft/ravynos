#import "ObjectTesting.h"
#import <Foundation/Foundation.h>
#import <Foundation/NSException.h>

@interface TestClass : NSObject
- (void)runTest;
- (void)exceptionThrowingMethod;

@end

@implementation TestClass
- (void)runTest
{
  int c = 0;
  int i;

  for (i = 0; i < 10; i++)
    {
      NSAutoreleasePool *pool = [NSAutoreleasePool new];
      
      NS_DURING
          [self exceptionThrowingMethod];
      NS_HANDLER
          c++;
      NS_ENDHANDLER
      [pool release];
    }
  PASS(c == 10, "Caught the correct number of exceptions without breaking the autorelease pool\n");
}

- (void)exceptionThrowingMethod
{
  NSAutoreleasePool *pool = [NSAutoreleasePool new];
  [@"Hello" stringByAppendingString: @" something to create a autoreleased object"];
  NSLog(@"Throwing an exception");
  [[NSException exceptionWithName: @"MyFunException" reason: @"it was always meant to happen" userInfo: [NSDictionary dictionary]] raise];
  [pool release]; // Obviously this doesn't get run, but the [NSAutorelease new] at the top causes the problem
}

@end

int main(int argc, char** argv)
{
  NSAutoreleasePool *pool = [NSAutoreleasePool new];
  TestClass *testClass = [[TestClass new] autorelease];
  [testClass runTest];
  [pool release];
  PASS(1, "Destroying pools in the wrong order didn't break anything...");
  return 0;
}
