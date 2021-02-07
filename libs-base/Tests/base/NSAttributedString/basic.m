#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSAttributedString.h>
#import "ObjectTesting.h"
int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSArray *arr = [NSArray arrayWithObject:[NSAttributedString new]];
  
  test_alloc(@"NSAttributedString");
  test_NSObject(@"NSAttributedString", arr);
  test_NSCoding(arr);
  test_keyed_NSCoding(arr);
  test_NSCopying(@"NSAttributedString",@"NSMutableAttributedString",arr,NO, NO);
  test_NSMutableCopying(@"NSAttributedString",@"NSMutableAttributedString",arr);

  [arp release]; arp = nil;
  return 0;
}

