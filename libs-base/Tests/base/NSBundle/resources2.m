#import "Testing.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSString.h>
#import <Foundation/NSPathUtilities.h>

@interface NSObject (TestMock)
- (NSString*)test;
@end

@interface TestFramework: NSObject
@end

static void _testBundle(NSBundle* bundle, NSString* path, NSString* className)
{
  NSArray  *arr, *carr;
  NSString *localPath;

  PASS((bundle != nil),
    "bundle was found");
  PASS((path != nil),
    "path of bundle was found");
  arr = [bundle pathsForResourcesOfType: @"txt" inDirectory: nil];
  PASS((arr && [arr count]),
    "-pathsForResourcesOfType:inDirectory: returns an array");
  localPath = [path stringByAppendingPathComponent:
    @"Resources/NonLocalRes.txt"];
  PASS([arr containsObject: localPath],
    "Returned array contains non-localized resource");
  localPath = [path stringByAppendingPathComponent:
    @"Resources/English.lproj/TextRes.txt"];
  PASS([arr containsObject: localPath],
    "Returned array contains localized resource");

  /* --- [NSBundle +pathsForResourcesOfType:inDirectory:] --- */
  carr = [NSBundle pathsForResourcesOfType: @"txt" inDirectory: path];
  PASS([arr isEqual: carr],
    "+pathsForResourcesOfType:inDirectory: returns same array");

  /* --- [NSBundle -pathsForResourcesOfType:inDirectory:forLocalization:] --- */
  arr = [bundle pathsForResourcesOfType: @"txt" inDirectory: nil
    forLocalization: @"English"];
  PASS((arr && [arr count]),
    "-pathsForResourcesOfType:inDirectory:forLocalization returns an array");
  localPath = [path stringByAppendingPathComponent:
    @"Resources/NonLocalRes.txt"];
  PASS([arr containsObject: localPath],
    "Returned array contains non-localized resource");
  localPath = [path stringByAppendingPathComponent:
    @"Resources/English.lproj/TextRes.txt"];
  PASS([arr containsObject: localPath],
    "Returned array contains localized resource");

  /* --- [NSBundle -pathsForResourcesOfType:inDirectory:forLocalization:] --- */
  arr = [bundle pathsForResourcesOfType: @"txt" inDirectory: nil
    forLocalization: @"en"];
  PASS((arr && [arr count]),
    "-pathsForResources... returns an array for 'en'");
  localPath = [path stringByAppendingPathComponent:
    @"Resources/NonLocalRes.txt"];
  PASS([arr containsObject: localPath],
    "Returned array for 'en' contains non-localized resource");
  localPath = [path stringByAppendingPathComponent:
    @"Resources/English.lproj/TextRes.txt"];
  PASS([arr containsObject: localPath],
    "Returned array for 'en' contains localized resource");

  /* --- [NSBundle -pathsForResourcesOfType:inDirectory:forLocalization:] --- */
  arr = [bundle pathsForResourcesOfType: @"txt" inDirectory: nil
    forLocalization: @"German"];
  PASS((arr && [arr count]),
    "-pathsForResources... returns an array for 'German'");
  localPath = [path stringByAppendingPathComponent:
    @"Resources/NonLocalRes.txt"];
  PASS([arr containsObject: localPath],
    "Returned array for 'German' contains non-localized resource");
  localPath = [path stringByAppendingPathComponent:
    @"Resources/de.lproj/TextRes.txt"];
  PASS([arr containsObject: localPath],
    "Returned array for 'German' contains localized resource");
  Class clz = [bundle classNamed: className];
  PASS(clz, "Class can be loaded from bundle");
  id obj = [clz new];
  PASS(obj, "Objects from bundle-loaded classes can be instantiated");
  PASS_EQUAL([obj test], @"Something", "Correct method called");
  [obj release];
}

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSString *path;
  NSBundle *bundle;

  START_SET("Bundle")
  path = [[[[[NSFileManager defaultManager] currentDirectoryPath]
    stringByStandardizingPath] stringByAppendingPathComponent: @"Resources"]
      stringByAppendingPathComponent: @"TestBundle.bundle"];
  bundle = [NSBundle bundleWithPath: path];
  _testBundle(bundle, path, @"TestBundle");
  END_SET("Bundle")

  START_SET("Framework")
  /* This method call is required to ensure that the linker does not decide to
   * elide the framework linkage.
   */
  [TestFramework class];
  bundle = [NSBundle bundleForClass: NSClassFromString(@"TestFramework")];
  path = [bundle bundlePath];
  _testBundle(bundle, path, @"TestFramework");
  PASS(0 == [bundle bundleVersion], "bundleVersion is zero");
  END_SET("Framework");

  [arp release]; arp = nil;
  return 0;
}
