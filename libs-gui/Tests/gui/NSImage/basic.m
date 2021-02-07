#import "ObjectTesting.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSImage.h>

int main()
{
  NSAutoreleasePool *arp = [NSAutoreleasePool new];
  id testObject;
  id testObject1;
  id testObject2;
  NSArray *testObjects;

  START_SET("NSImage GNUstep basic")

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

  test_alloc(@"NSImage");

  testObject = [NSImage new];
  testObject1 = [NSImage imageNamed: @"GNUstep"];
  testObject2 = [[NSImage alloc] initWithData: nil];

  testObjects = [NSArray arrayWithObjects: testObject, testObject1, testObject2, nil];
  RELEASE(testObject);

  test_NSObject(@"NSImage", testObjects);
  test_NSCoding(testObjects);
  test_keyed_NSCoding(testObjects);
  test_NSCopying(@"NSImage",
                 @"NSImage",
		 testObjects, NO, NO);

  END_SET("NSImage GNUstep basic")

  [arp release];
  return 0;
}

@implementation NSImage (Testing)

- (BOOL) isEqual: (id)anObject
{
  if (self == anObject)
    return YES;
  if (![anObject isKindOfClass: [NSImage class]])
    return NO;
  if (![[anObject backgroundColor] isEqual: [self backgroundColor]])
    return NO;
  if ([anObject isFlipped] != [self isFlipped])
    return NO;
  if (!NSEqualSizes([anObject size], [self size]))
    return NO;

  // FIXME
  return YES;
}

@end
