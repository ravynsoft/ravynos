#import "ObjectTesting.h"
#import <GNUstepBase/NSHashTable+GNUstepBase.h> 
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>


int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSHashTable *obj = [[NSHashTable new] autorelease];
  NSString *strA = @"a";
  NSString *strB = @"b";
  NSArray *a = [NSArray arrayWithObjects: strA, strB, strA, nil];
  [obj addObjectsFromArray: a];
  PASS([obj count] == 2, "-addObjectsFromArray: adds objects ignoring duplicates");
  PASS([obj containsObject: strA] && [obj containsObject: strB], "Table contains correct objects");

  [arp release]; arp = nil;
  return 0;
} 

