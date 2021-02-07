#import "ObjectTesting.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSCell.h>
#import <AppKit/NSImage.h>

int main()
{
  NSAutoreleasePool *arp = [NSAutoreleasePool new];
  id testObject;
  id testObject1;
  id testObject2;
  NSArray *testObjects;

  START_SET("NSCell GNUstep basic")

  NS_DURING
  {
    [NSApplication sharedApplication];
  }
  NS_HANDLER
  {
    if ([[localException name] isEqualToString: NSInternalInconsistencyException ])
      SKIP("It looks like GNUstep backend is not yet installed")
  }
  NS_ENDHANDLER

  test_alloc(@"NSCell");

  testObject = [NSCell new];
  testObject1 = [[NSCell alloc] initImageCell: [NSImage imageNamed: @"GNUstep"]];
  testObject2 = [[NSCell alloc] initTextCell: @"GNUstep"];

  testObjects = [NSArray arrayWithObjects: testObject, testObject1, testObject2, nil];
  test_NSObject(@"NSCell", testObjects);
  test_NSCoding(testObjects);
  test_keyed_NSCoding(testObjects);
  test_NSCopying(@"NSCell",
                 @"NSCell",
		 testObjects, NO, NO);

  END_SET("NSCell GNUstep basic")
  [arp release];
  return 0;
}

@implementation NSCell (Testing)

- (BOOL) isEqual: (id)anObject
{
  if (self == anObject)
    return YES;
  if (![anObject isKindOfClass: [NSCell class]])
    return NO;
  if (![[(NSCell *)anObject stringValue] isEqual: [self stringValue]])
    return NO;
  if (![[(NSCell *)anObject title] isEqual: [self title]])
    return NO;
  if (!([(NSCell *)anObject image] == [self image])
      && ![[(NSCell *)anObject image] isEqual: [self image]])
    {
      NSLog(@"image differ %@ %@", [self image], [anObject image]);
      return NO;
    }
  if ([(NSCell *)anObject type] != [self type])
    return NO;
  if ([(NSCell *)anObject tag] != [self tag])
    return NO;
  return YES;
}

@end
