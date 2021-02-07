#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSString.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSString *path;
  NSBundle *bundle;
  Class aClass;
  id    anObj;

  path = RETAIN([[[[[NSFileManager defaultManager] currentDirectoryPath]
           stringByDeletingLastPathComponent]
            stringByAppendingPathComponent:@"Resources"]
              stringByAppendingPathComponent: @"TestBundle.bundle"]);
  
  bundle = RETAIN([NSBundle bundleWithPath: path]);
  
  pass([bundle isKindOfClass:[NSBundle class]],
       "+bundleWithPath returns a bundle");
  
  pass([[bundle bundlePath] testForString],"the bundle has a path");
  
  pass([path testEquals: [bundle bundlePath]], 
       "bundlePath returns the correct path"); 
  
  pass([[bundle resourcePath] testForString],"a bundle has a resource path"); 
  pass([[bundle infoDictionary] isKindOfClass:[NSDictionary class]],
       "a bundle has an infoDictionary");

  pass([bundle load],"bundle -load returns YES");
  aClass = NSClassFromString(@"TestBundle");
  pass(aClass != Nil,"-load actually loaded the class");
  anObj = [aClass new];
  pass(anObj != nil, "we can instantiate a loaded class");
  pass([bundle principalClass] != nil, "-principalClass is not nil");
  pass([[[bundle principalClass] description] testEquals:@"TestBundle"],
       "-principalClass works");
  pass([[bundle principalClass] new] != nil, "we can instantiate -principalClass");
  pass([[bundle classNamed:@"TestBundle"] testEquals:[bundle principalClass]],
       "-classNamed works");
  pass([bundle testEquals: [NSBundle bundleForClass: NSClassFromString(@"TestBundle")]],
       "+bundleForClass works");
  [bundle setBundleVersion:42];
  pass([bundle bundleVersion] == 42, "we can set and get a bundle version");
  pass([[NSBundle allBundles] containsObject:bundle],
       "+allBundles contains a bundle after we loaded it");
  
  RELEASE(bundle);
  RELEASE(path);
  [arp release]; arp = nil;
  return 0;
}
