#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSFileManager.h>
#import "ObjectTesting.h"

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSString *path;
  NSBundle *bundle;
  
  path = [[[[NSFileManager defaultManager] currentDirectoryPath] 
    stringByStandardizingPath] stringByAppendingPathComponent: @"Resources"];
  
  PASS([NSBundle mainBundle] != nil, 
    "+mainBundle returns non-nil if the tool has no bundle");
  
  bundle = [NSBundle bundleWithPath: path];

  TEST_FOR_CLASS(@"NSBundle", bundle, "+bundleWithPath returns a bundle");
  
  TEST_STRING([bundle bundlePath],"a bundle has a path");
  
  PASS([path isEqual:[bundle bundlePath]] &&
       [[bundle bundlePath] isEqual: path],
       "bundlePath returns the correct path");
  
  TEST_FOR_CLASS(@"NSDictionary",[bundle infoDictionary],
                 "a bundle has an infoDictionary");
  
  PASS([NSBundle bundleWithPath:
	 [path stringByAppendingPathComponent: @"nonexistent"]] == nil,
       "+bundleWithPath returns nil for a non-existing path"); 
  
  {
    NSArray *arr = [NSBundle allBundles];
    PASS(arr != nil && [arr isKindOfClass: [NSArray class]] && [arr count] != 0,
         "+allBundles returns an array");
  }
  [arp release]; arp = nil;
  return 0;
}
